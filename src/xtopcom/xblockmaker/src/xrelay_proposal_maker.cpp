// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xblockmaker/xrelay_proposal_maker.h"

#include "xbasic/xmemory.hpp"
#include "xblockmaker/xblock_maker_para.h"
#include "xblockmaker/xblockmaker_error.h"
#include "xblockmaker/xrelay_proposal_maker.h"
#include "xdata/xblocktool.h"
#include "xmbus/xevent_behind.h"
#include "xstore/xtgas_singleton.h"

#include <string>

NS_BEG2(top, blockmaker)

// REG_XMODULE_LOG(chainbase::enum_xmodule_type::xmodule_type_xblockmaker, xblockmaker_error_to_string, xblockmaker_error_base+1, xblockmaker_error_max);

xrelay_proposal_maker_t::xrelay_proposal_maker_t(const std::string & account, const xblockmaker_resources_ptr_t & resources) {
    xdbg("xrelay_proposal_maker_t::xrelay_proposal_maker_t create,this=%p,account=%s", this, account.c_str());
    m_resources = resources;
    m_relay_maker = make_object_ptr<xrelay_maker_t>(account, resources);
}

xrelay_proposal_maker_t::~xrelay_proposal_maker_t() {
    xdbg("xrelay_proposal_maker_t::xrelay_proposal_maker_t destroy,this=%p", this);
}

bool xrelay_proposal_maker_t::can_make_proposal(data::xblock_consensus_para_t & proposal_para) {
    if (proposal_para.get_viewid() <= proposal_para.get_latest_cert_block()->get_viewid()) {
        xwarn("xrelay_proposal_maker_t::can_make_proposal fail-behind viewid. %s,latest_viewid=%" PRIu64 "",
              proposal_para.dump().c_str(),
              proposal_para.get_latest_cert_block()->get_viewid());
        return false;
    }

    if (!data::xblocktool_t::verify_latest_blocks(
            proposal_para.get_latest_cert_block().get(), proposal_para.get_latest_locked_block().get(), proposal_para.get_latest_committed_block().get())) {
        xwarn("xrelay_proposal_maker_t::can_make_proposal. fail-verify_latest_blocks fail.%s", proposal_para.dump().c_str());
        return false;
    }
    return true;
}

data::xtablestate_ptr_t xrelay_proposal_maker_t::get_target_tablestate(base::xvblock_t * block) {
    base::xauto_ptr<base::xvbstate_t> bstate = m_resources->get_xblkstatestore()->get_block_state(block, metrics::statestore_access_from_blkmaker_get_target_tablestate);
    if (bstate == nullptr) {
        xwarn("xrelay_proposal_maker_t::get_target_tablestate fail-get target state.block=%s", block->dump().c_str());
        return nullptr;
    }
    data::xtablestate_ptr_t tablestate = std::make_shared<data::xtable_bstate_t>(bstate.get());
    return tablestate;
}

xblock_ptr_t xrelay_proposal_maker_t::make_proposal(data::xblock_consensus_para_t & proposal_para, uint32_t min_tx_num) {
    auto & latest_cert_block = proposal_para.get_latest_cert_block();
    data::xtablestate_ptr_t tablestate = get_target_tablestate(latest_cert_block.get());
    if (nullptr == tablestate) {
        xwarn("xrelay_proposal_maker_t::make_proposal fail clone tablestate. %s,cert_height=%" PRIu64 "", proposal_para.dump().c_str(), latest_cert_block->get_height());
        return nullptr;
    }

    data::xtablestate_ptr_t tablestate_commit = get_target_tablestate(proposal_para.get_latest_committed_block().get());
    if (tablestate_commit == nullptr) {
        xwarn("xrelay_proposal_maker_t::make_proposal fail clone tablestate. %s,commit_height=%" PRIu64 "",
              proposal_para.dump().c_str(),
              proposal_para.get_latest_committed_block()->get_height());
        return nullptr;
    }

    xtablemaker_para_t table_para(tablestate, tablestate_commit);

    if (false == leader_set_consensus_para(latest_cert_block.get(), proposal_para)) {
        xwarn("xrelay_proposal_maker_t::make_proposal fail-leader_set_consensus_para.%s", proposal_para.dump().c_str());
        return nullptr;
    }

    xtablemaker_result_t table_result;
    xblock_ptr_t proposal_block = m_relay_maker->make_proposal(table_para, proposal_para, table_result);
    if (proposal_block == nullptr) {
        if (xblockmaker_error_no_need_make_table != table_result.m_make_block_error_code) {
            XMETRICS_GAUGE(metrics::cons_table_leader_make_proposal_succ, 0);
            xwarn("xrelay_proposal_maker_t::make_proposal fail-make_proposal.%s error_code=%s",
                  proposal_para.dump().c_str(),
                  chainbase::xmodule_error_to_str(table_result.m_make_block_error_code).c_str());
        } else {
            xinfo("xrelay_proposal_maker_t::make_proposal no need make table.%s", proposal_para.dump().c_str());
        }
        return nullptr;
    }
    auto & proposal_input = table_para.get_proposal();
    std::string proposal_input_str;
    proposal_input->serialize_to_string(proposal_input_str);
    proposal_block->get_input()->set_proposal(proposal_input_str);
    bool bret = proposal_block->reset_prev_block(latest_cert_block.get());
    xassert(bret);

    // add metrics of tx counts / table counts ratio
    xinfo("xrelay_proposal_maker_t::make_proposal succ.proposal_block=%s,proposal_input={size=%zu,txs=%zu,accounts=%zu}",
          proposal_block->dump().c_str(),
          proposal_input_str.size(),
          proposal_input->get_input_txs().size(),
          proposal_input->get_other_accounts().size());
    return proposal_block;
}

int xrelay_proposal_maker_t::verify_proposal(base::xvblock_t * proposal_block, base::xvqcert_t * bind_clock_cert) {
    xdbg("xrelay_proposal_maker_t::verify_proposal enter. proposal=%s", proposal_block->dump().c_str());
    // uint64_t gmtime = proposal_block->get_second_level_gmtime();
    xblock_consensus_para_t cs_para(
        get_account(), proposal_block->get_clock(), proposal_block->get_viewid(), proposal_block->get_viewtoken(), proposal_block->get_height(), 0);

    // verify gmtime valid
    // uint64_t now = (uint64_t)base::xtime_utl::gettimeofday();
    // if ((gmtime > (now + 60)) || (gmtime < (now - 60))) {  // the gmtime of leader should in +-60s with backup node
    //     xwarn("xrelay_proposal_maker_t::verify_proposal fail-gmtime not match. proposal=%s,leader_gmtime=%ld,backup_gmtime=%ld", proposal_block->dump().c_str(), gmtime, now);
    //     XMETRICS_GAUGE(metrics::cons_fail_verify_proposal_blocks_invalid, 1);
    //     XMETRICS_GAUGE(metrics::cons_table_backup_verify_proposal_succ, 0);
    //     return xblockmaker_error_proposal_outofdate;
    // }

    auto cert_block = m_resources->get_blockstore()->get_latest_cert_block(*m_relay_maker);
    if (proposal_block->get_height() < cert_block->get_height()) {
        xwarn(
            "xrelay_proposal_maker_t::verify_proposal fail-proposal height less than cert block. proposal=%s,cert=%s", proposal_block->dump().c_str(), cert_block->dump().c_str());
        return xblockmaker_error_proposal_cannot_connect_to_cert;
    }

    // TODO(jimmy) xbft callback and pass cert/lock/commit to us for performance
    // find matched cert block
    xblock_ptr_t proposal_prev_block = nullptr;
    if (proposal_block->get_last_block_hash() == cert_block->get_block_hash() && proposal_block->get_height() == cert_block->get_height() + 1) {
        proposal_prev_block = xblock_t::raw_vblock_to_object_ptr(cert_block.get());
    } else {
        auto _demand_cert_block = m_resources->get_blockstore()->load_block_object(
            *m_relay_maker, proposal_block->get_height() - 1, proposal_block->get_last_block_hash(), false, metrics::blockstore_access_from_blk_mk_proposer_verify_proposal);
        if (_demand_cert_block == nullptr) {
            xwarn("xrelay_proposal_maker_t::verify_proposal fail-find cert block. proposal=%s", proposal_block->dump().c_str());
            XMETRICS_GAUGE(metrics::cons_fail_verify_proposal_blocks_invalid, 1);
            XMETRICS_GAUGE(metrics::cons_table_backup_verify_proposal_succ, 0);
            return xblockmaker_error_proposal_cannot_connect_to_cert;
        }
        proposal_prev_block = xblock_t::raw_vblock_to_object_ptr(_demand_cert_block.get());
    }
    cs_para.update_latest_cert_block(proposal_prev_block);  // prev table block is key info

    // find matched lock block
    if (proposal_prev_block->get_height() > 0) {
        auto lock_block = m_resources->get_blockstore()->load_block_object(*m_relay_maker,
                                                              proposal_prev_block->get_height() - 1,
                                                              proposal_prev_block->get_last_block_hash(),
                                                              false,
                                                              metrics::blockstore_access_from_blk_mk_proposer_verify_proposal);
        if (lock_block == nullptr) {
            xwarn("xrelay_proposal_maker_t::verify_proposal fail-find lock block. proposal=%s", proposal_block->dump().c_str());
            return xblockmaker_error_proposal_cannot_connect_to_cert;
        }
        xblock_ptr_t prev_lock_block = xblock_t::raw_vblock_to_object_ptr(lock_block.get());
        cs_para.update_latest_lock_block(prev_lock_block);
    } else {
        cs_para.update_latest_lock_block(proposal_prev_block);
    }
    // find matched commit block
    if (cs_para.get_latest_locked_block()->get_height() > 0) {
        // XTODO get latest connected block which can also load commit block, and it will invoke to update latest connect height.
        auto connect_block = m_resources->get_blockstore()->get_latest_connected_block(*m_relay_maker);
        if (connect_block == nullptr) {
            xerror("xrelay_proposal_maker_t::verify_proposal fail-find connected block. proposal=%s", proposal_block->dump().c_str());
            return xblockmaker_error_proposal_cannot_connect_to_cert;
        }
        if (connect_block->get_height() != cs_para.get_latest_locked_block()->get_height() - 1) {
            xwarn("xrelay_proposal_maker_t::verify_proposal fail-connect not match commit block. proposal=%s,connect_height=%ld",
                  proposal_block->dump().c_str(),
                  connect_block->get_height());
            return xblockmaker_error_proposal_cannot_connect_to_cert;
        }
        xblock_ptr_t prev_commit_block = xblock_t::raw_vblock_to_object_ptr(connect_block.get());
        cs_para.update_latest_commit_block(prev_commit_block);
    } else {
        cs_para.update_latest_commit_block(cs_para.get_latest_locked_block());
    }

    xdbg_info(
        "xrelay_proposal_maker_t::verify_proposal. set latest_cert_block.proposal=%s, latest_cert_block=%s", proposal_block->dump().c_str(), proposal_prev_block->dump().c_str());

    // update txpool receiptid state
    const xblock_ptr_t & commit_block = cs_para.get_latest_committed_block();
    data::xtablestate_ptr_t commit_tablestate = get_target_tablestate(commit_block.get());
    if (commit_tablestate == nullptr) {
        xwarn("xrelay_proposal_maker_t::verify_proposal fail clone tablestate. %s,cert=%s", cs_para.dump().c_str(), proposal_prev_block->dump().c_str());
        return xblockmaker_error_proposal_table_state_clone;
    }

    // get tablestate related to latest cert block
    data::xtablestate_ptr_t tablestate = get_target_tablestate(proposal_prev_block.get());
    if (nullptr == tablestate) {
        xwarn("xrelay_proposal_maker_t::verify_proposal fail clone tablestate. %s,cert=%s", cs_para.dump().c_str(), proposal_prev_block->dump().c_str());
        return xblockmaker_error_proposal_table_state_clone;
    }

    xtablemaker_para_t table_para(tablestate, commit_tablestate);
    if (false == verify_proposal_input(proposal_block, table_para)) {
        xwarn("xrelay_proposal_maker_t::verify_proposal fail-proposal input invalid. proposal=%s", proposal_block->dump().c_str());
        return xblockmaker_error_proposal_bad_input;
    }

    if (false == backup_set_consensus_para(proposal_prev_block.get(), proposal_block, nullptr, cs_para)) {
        xwarn("xproposal_maker_t::verify_proposal fail-backup_set_consensus_para. proposal=%s",
            proposal_block->dump().c_str());
        XMETRICS_GAUGE(metrics::cons_fail_verify_proposal_consensus_para_get, 1);
        XMETRICS_GAUGE(metrics::cons_table_backup_verify_proposal_succ, 0);
        return xblockmaker_error_proposal_bad_consensus_para;
    }

    int32_t verify_ret = m_relay_maker->verify_proposal(proposal_block, table_para, cs_para);
    if (verify_ret != xsuccess) {
        xwarn("xrelay_proposal_maker_t::verify_proposal fail-verify_proposal. proposal=%s,error_code=%s",
              proposal_block->dump().c_str(),
              chainbase::xmodule_error_to_str(verify_ret).c_str());
        XMETRICS_GAUGE(metrics::cons_table_backup_verify_proposal_succ, 0);
        return verify_ret;
    }
    XMETRICS_GAUGE(metrics::cons_table_backup_verify_proposal_succ, 1);
    xdbg_info("xrelay_proposal_maker_t::verify_proposal succ. proposal=%s,latest_cert_block=%s", proposal_block->dump().c_str(), proposal_prev_block->dump().c_str());
    return xsuccess;
}

bool xrelay_proposal_maker_t::verify_proposal_input(base::xvblock_t * proposal_block, xtablemaker_para_t & table_para) {
    return true;
}

std::string xrelay_proposal_maker_t::calc_random_seed(base::xvblock_t* latest_cert_block,uint64_t viewtoken) {
    std::string random_str;
    uint64_t last_block_nonce = latest_cert_block->get_cert()->get_nonce();
    random_str = base::xstring_utl::tostring(last_block_nonce);
    random_str += base::xstring_utl::tostring(viewtoken);
    uint64_t seed = base::xhash64_t::digest(random_str);
    return base::xstring_utl::tostring(seed);
}

bool xrelay_proposal_maker_t::leader_set_consensus_para(base::xvblock_t * latest_cert_block, xblock_consensus_para_t & cs_para) {
    uint64_t now = (uint64_t)base::xtime_utl::gettimeofday();
    cs_para.set_timeofday_s(now);

    uint64_t total_lock_tgas_token = 0;
    uint64_t property_height = 0;
    bool ret = store::xtgas_singleton::get_instance().leader_get_total_lock_tgas_token(cs_para.get_clock(), total_lock_tgas_token, property_height);
    if (!ret) {
        xwarn("xrelay_proposal_maker_t::leader_set_consensus_para fail-leader_get_total_lock_tgas_token. %s", cs_para.dump().c_str());
        return ret;
    }
    std::string random_seed = calc_random_seed(latest_cert_block, cs_para.get_viewtoken());
    cs_para.set_parent_height(latest_cert_block->get_height() + 1);
    cs_para.set_tableblock_consensus_para(0, random_seed, total_lock_tgas_token, property_height);
    xdbg_info("xrelay_proposal_maker_t::leader_set_consensus_para %s random_seed=%s,tgas_token=%" PRIu64 ",tgas_height=%" PRIu64 " leader",
              cs_para.dump().c_str(),
              random_seed.c_str(),
              total_lock_tgas_token,
              property_height);
    return true;
}

bool xrelay_proposal_maker_t::backup_set_consensus_para(base::xvblock_t * latest_cert_block,
                                                        base::xvblock_t * proposal,
                                                        base::xvqcert_t * bind_drand_cert,
                                                        xblock_consensus_para_t & cs_para) {
    uint64_t now = (uint64_t)base::xtime_utl::gettimeofday();
    cs_para.set_timeofday_s(now);

    cs_para.set_parent_height(latest_cert_block->get_height() + 1);
    cs_para.set_common_consensus_para(proposal->get_cert()->get_clock(),
                                      proposal->get_cert()->get_validator(),
                                      proposal->get_cert()->get_auditor(),
                                      proposal->get_cert()->get_viewid(),
                                      proposal->get_cert()->get_viewtoken(),
                                      proposal->get_cert()->get_drand_height());
    return true;
}

NS_END2
