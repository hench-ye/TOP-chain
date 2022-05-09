// Copyright (c) 2017-2020 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xtxpool_v2/xreceiptid_state_cache.h"

#include "xtxpool_v2/xtxpool_log.h"

NS_BEG2(top, xtxpool_v2)

// xreceiptid_state_cache_t::xreceiptid_state_cache_t() {
//     base::xtable_index_t tableindex(base::enum_chain_zone_zec_index, 2);
//     base::xreceiptid_state_ptr_t receiptid_state = std::make_shared<base::xreceiptid_state_t>();
//     m_receiptid_state_map[tableindex.to_table_shortid()] = receiptid_state;
// }

void xreceiptid_state_cache_t::update_table_receiptid_state(const base::xvproperty_prove_ptr_t & property_prove_ptr, const base::xreceiptid_state_ptr_t & receiptid_state) {
    auto table_id = receiptid_state->get_self_tableid();
    std::lock_guard<std::mutex> lck(m_mutex);
    auto iter = m_receiptid_state_map.find(table_id);
    if (iter != m_receiptid_state_map.end()) {
        auto & old_receiptid_state = iter->second.m_receiptid_state;
        if (receiptid_state->get_block_height() <= old_receiptid_state->get_block_height()) {
            return;
        }
    }
    xinfo("xreceiptid_state_cache_t::update_table_receiptid_state table:%d,height:%llu,pairs:%s",
          receiptid_state->get_self_tableid(),
          receiptid_state->get_block_height(),
          receiptid_state->get_all_receiptid_pairs()->dump().c_str());
    m_receiptid_state_map[table_id] = xreceiptid_state_and_prove(property_prove_ptr, receiptid_state);
}

uint64_t xreceiptid_state_cache_t::get_confirmid_max(base::xtable_shortid_t table_id, base::xtable_shortid_t peer_table_id) const {
    std::lock_guard<std::mutex> lck(m_mutex);
    auto iter = m_receiptid_state_map.find(table_id);
    if (iter != m_receiptid_state_map.end()) {
        auto & table_receiptid_state = iter->second.m_receiptid_state;
        base::xreceiptid_pair_t pair;
        table_receiptid_state->find_pair(peer_table_id, pair);
        return pair.get_confirmid_max();
    }
    return 0;
}

uint64_t xreceiptid_state_cache_t::get_recvid_max(base::xtable_shortid_t table_id, base::xtable_shortid_t peer_table_id) const {
    std::lock_guard<std::mutex> lck(m_mutex);
    auto iter = m_receiptid_state_map.find(table_id);
    if (iter != m_receiptid_state_map.end()) {
        auto & table_receiptid_state = iter->second.m_receiptid_state;
        base::xreceiptid_pair_t pair;
        table_receiptid_state->find_pair(peer_table_id, pair);
        return pair.get_recvid_max();
    }
    return 0;
}

uint64_t xreceiptid_state_cache_t::get_sendid_max(base::xtable_shortid_t table_id, base::xtable_shortid_t peer_table_id) const {
    std::lock_guard<std::mutex> lck(m_mutex);
    auto iter = m_receiptid_state_map.find(table_id);
    if (iter != m_receiptid_state_map.end()) {
        auto & table_receiptid_state = iter->second.m_receiptid_state;
        base::xreceiptid_pair_t pair;
        table_receiptid_state->find_pair(peer_table_id, pair);
        return pair.get_sendid_max();
    }
    return 0;
}

uint64_t xreceiptid_state_cache_t::get_height(base::xtable_shortid_t table_id) const {
    std::lock_guard<std::mutex> lck(m_mutex);
    auto iter = m_receiptid_state_map.find(table_id);
    if (iter != m_receiptid_state_map.end()) {
        auto & table_receiptid_state = iter->second.m_receiptid_state;
        return table_receiptid_state->get_block_height();
    }
    return 0;
}

base::xreceiptid_state_ptr_t xreceiptid_state_cache_t::get_table_receiptid_state(base::xtable_shortid_t table_id) const {
    std::lock_guard<std::mutex> lck(m_mutex);
    auto iter = m_receiptid_state_map.find(table_id);
    if (iter != m_receiptid_state_map.end()) {
        return iter->second.m_receiptid_state;
    }
    return nullptr;
}

// normal case for table A and table B
// section of A as sender pull from B：         (A confirm id, B recv id]
// section of A as sender for B pull from A：   (B recv id, A send id]
// left boundary for A as sender: if A have no unconfirm rsp id    ----> B recv id
//                                else                             ----> A confirm id
// section of A as receiver pull from B：       (A recv id, B send id]
// section of A as receiver for B pull from A： (B confirm id, A recv id]
// left boundary for A as receiver: if B have no unconfirm rsp id  ----> A recv id
//                                  else                           ----> B confirm id
void xreceiptid_state_cache_t::get_unconfirm_id_section_as_sender(base::xtable_shortid_t table_id,
                                                                  base::xtable_shortid_t peer_table_id,
                                                                  uint64_t & confirm_id,
                                                                  uint64_t & unconfirm_id_max,
                                                                  bool for_pull_lacking) const {
    std::lock_guard<std::mutex> lck(m_mutex);
    uint64_t sendid_max = 0;
    uint64_t confirmid_max = 0;
    uint64_t recvid_max = 0;

    auto iter_peer = m_receiptid_state_map.find(peer_table_id);
    if (iter_peer != m_receiptid_state_map.end()) {
        auto & table_receiptid_state = iter_peer->second.m_receiptid_state;
        base::xreceiptid_pair_t peer_pair;
        table_receiptid_state->find_pair(table_id, peer_pair);
        recvid_max = peer_pair.get_recvid_max();
    }

    auto iter_self = m_receiptid_state_map.find(table_id);
    if (iter_self != m_receiptid_state_map.end()) {
        auto & table_receiptid_state = iter_self->second.m_receiptid_state;
        base::xreceiptid_pair_t self_pair;
        table_receiptid_state->find_pair(peer_table_id, self_pair);
        sendid_max = self_pair.get_sendid_max();
        confirmid_max = self_pair.get_confirmid_max();

        if (self_pair.all_confirmed_as_sender()) {
            if (recvid_max > sendid_max) {
                // self state is fall behind
                confirm_id = sendid_max;
                unconfirm_id_max = sendid_max;
                return;
            } else if (recvid_max >= confirmid_max) {
                // normal
                confirm_id = recvid_max;
            } else {
                // peer state is fall behind
                confirm_id = confirmid_max;
            }
        } else {
            confirm_id = confirmid_max;
        }
    }

    if (for_pull_lacking) {
        unconfirm_id_max = (confirm_id > recvid_max) ? confirm_id : recvid_max;
    } else {
        unconfirm_id_max = sendid_max;
    }
}

void xreceiptid_state_cache_t::get_unconfirm_id_section_as_receiver(base::xtable_shortid_t table_id,
                                                                    base::xtable_shortid_t peer_table_id,
                                                                    uint64_t & confirm_id,
                                                                    uint64_t & unconfirm_id_max) const {
    std::lock_guard<std::mutex> lck(m_mutex);
    uint64_t recvid_max = 0;
    uint64_t confirmid_max = 0;
    auto iter_self = m_receiptid_state_map.find(table_id);
    if (iter_self != m_receiptid_state_map.end()) {
        auto & table_receiptid_state = iter_self->second.m_receiptid_state;
        base::xreceiptid_pair_t self_pair;
        table_receiptid_state->find_pair(peer_table_id, self_pair);
        recvid_max = self_pair.get_recvid_max();
    }

    auto iter_peer = m_receiptid_state_map.find(peer_table_id);
    if (iter_peer != m_receiptid_state_map.end()) {
        auto & table_receiptid_state = iter_peer->second.m_receiptid_state;
        base::xreceiptid_pair_t peer_pair;
        table_receiptid_state->find_pair(table_id, peer_pair);
        if (peer_pair.all_confirmed_as_sender()) {
            if (recvid_max < peer_pair.get_confirmid_max()) {
                // self state is fall behind
                confirm_id = peer_pair.get_confirmid_max();
                unconfirm_id_max = peer_pair.get_confirmid_max();
                return;
            } else if (peer_pair.get_sendid_max() >= recvid_max) {
                // normal
                confirm_id = recvid_max;
                unconfirm_id_max = recvid_max;
                return;                
            } else {
                // peer state is fall behind.
                confirm_id = peer_pair.get_sendid_max();
                unconfirm_id_max = recvid_max;
                return;
            }
        }
        confirmid_max = peer_pair.get_confirmid_max();
    }

    confirm_id = confirmid_max;
    unconfirm_id_max = recvid_max;
}

const xreceiptid_state_and_prove xreceiptid_state_cache_t::get_receiptid_state_and_prove(base::xtable_shortid_t self_table_id,
                                                                                         base::xtable_shortid_t peer_table_id,
                                                                                         uint64_t min_not_need_confirm_receiptid,
                                                                                         uint64_t max_not_need_confirm_receiptid) const {
    std::lock_guard<std::mutex> lck(m_mutex);

    auto iter_peer = m_receiptid_state_map.find(peer_table_id);
    if (iter_peer == m_receiptid_state_map.end()) {
        return {};
    }
    auto & peer_receiptid_info = iter_peer->second;
    auto & peer_receiptid_state = peer_receiptid_info.m_receiptid_state;

    base::xreceiptid_pair_t peer_pair;
    peer_receiptid_state->find_pair(self_table_id, peer_pair);
    auto recvid_max = peer_pair.get_recvid_max();

    if (recvid_max >= min_not_need_confirm_receiptid && recvid_max <= max_not_need_confirm_receiptid && peer_receiptid_info.m_property_prove_ptr != nullptr) {
        xtxpool_info("xreceiptid_state_cache_t::get_receiptid_state_and_prove self:%d,peer:%d,recvid_max:%llu,min:%llu,max:%llu",
                     self_table_id,
                     peer_table_id,
                     recvid_max,
                     min_not_need_confirm_receiptid,
                     max_not_need_confirm_receiptid);
        return peer_receiptid_info;
    }
    xtxpool_dbg("xreceiptid_state_cache_t::get_receiptid_state_and_prove fail self:%d,peer:%d,recvid_max:%llu,min:%llu,max:%llu",
                self_table_id,
                peer_table_id,
                recvid_max,
                min_not_need_confirm_receiptid,
                max_not_need_confirm_receiptid);
    return {};
}

NS_END2
