#include <chrono>
#include <thread>
#include <gtest/gtest.h>
#define protected public
#define private public
#include "xsync/xchain_downloader.h"
#include "xsync/xsync_sender.h"
#include "tests/xvnetwork/xdummy_vhost.h"
#include "xdata/xnative_contract_address.h"
#include "xdata/xblock_cs_para.h"
#include "xdata/xblocktool.h"
#include "xsync/xsync_util.h"
#include "tests/mock/xmock_auth.hpp"
#include "xsync/xsync_message.h"
#include "xsync/xsync_store_shadow.h"
#include "xsync/xrole_xips_manager.h"
#include "xsync/xdownloader.h"
#include "../common.h"
#include "xmbus/xevent_executor.h"
#include "xsyncbase/xmessage_ids.h"
#include "tests/mock/xdatamock_table.hpp"
#include "tests/mock/xvchain_creator.hpp"

using namespace top;
using namespace top::sync;
using namespace top::mbus;
using namespace top::data;
using namespace top::mock;

static std::string reason = "test";

class xsync_mock_ratelimit_t : public xsync_ratelimit_face_t {
public:
    xsync_mock_ratelimit_t() {}
    void start() override {
    }
    void stop() override {
    }
    bool get_token(int64_t now) override {
        return true;
    }
    void feedback(uint32_t cost, int64_t now) override {
    }
    void resume() override {}
};

TEST(xsync_account, no_response) {

    std::string address = xdatautil::serialize_owner_str(sys_contract_beacon_table_block_addr, 0);


    mock::xvchain_creator creator;
    creator.create_blockstore_with_xstore();
    xobject_ptr_t<store::xstore_face_t> store;
    store.attach(creator.get_xstore());
    xobject_ptr_t<base::xvblockstore_t> blockstore;
    blockstore.attach(creator.get_blockstore());
    xsync_store_shadow_t shadow;
    xsync_store_t sync_store("", make_observer(blockstore), &shadow);

    xmessage_bus_t mbus;
    xmock_vhost_sync_t vhost;
    xsync_sender_t sync_sender("", make_observer(&vhost), nullptr);
    xsync_mock_ratelimit_t ratelimit;

    std::vector<base::xvblock_t*> block_vector;
    base::xvblock_t* genesis_block = data::xblocktool_t::create_genesis_empty_table(address);

    base::xvblock_t* prev_block = genesis_block;
    block_vector.push_back(prev_block);

    for (uint64_t i=1; i<=5; i++) {
        prev_block = data::xblocktool_t::create_next_emptyblock(prev_block);
        block_vector.push_back(prev_block);
    }

    //store.current_block = genesis_block;
    //sync_store.store_block();

    top::mock::xmock_auth_t auth{1};
    sync::xrole_xips_manager_t manager("");
    xrole_chains_mgr_t chain_manager("");
    xchain_downloader_face_ptr_t chain_downloader =
        std::make_shared<xchain_downloader_t>("", &sync_store, &manager, &chain_manager, make_observer(&mbus), make_observer(&auth), &sync_sender, &ratelimit, address);

    block_vector[4]->add_ref();
    base::xauto_ptr<base::xvblock_t> vblock4 = block_vector[4];
    xblock_ptr_t successor_block4 = autoptr_to_blockptr(vblock4);

    top::common::xnode_address_t network_self;
    top::common::xnode_address_t target_address;

    chain_downloader->on_behind(0, vblock4->get_height(), enum_chain_sync_policy_full, network_self, target_address, reason);
    {
        xmessage_t msg;
        xvnode_address_t src;
        xvnode_address_t dst;
        ASSERT_EQ(vhost.read_msg(msg, src, dst), false);
        vnetwork::xmessage_t::message_type msg_type = msg.id();
        //ASSERT_EQ(msg_type, xmessage_id_sync_get_blocks);
        ASSERT_EQ(vhost.read_msg(msg, src, dst), false);
    }

    std::this_thread::sleep_for(std::chrono::seconds(1));
    chain_downloader->on_behind(0, vblock4->get_height(), enum_chain_sync_policy_full, network_self, target_address, reason);
    {
        xmessage_t msg;
        xvnode_address_t src;
        xvnode_address_t dst;
        ASSERT_EQ(vhost.read_msg(msg, src, dst), false);
        vnetwork::xmessage_t::message_type msg_type = msg.id();
        //ASSERT_EQ(msg_type, xmessage_id_sync_get_blocks);
        ASSERT_EQ(vhost.read_msg(msg, src, dst), false);
    }
}

static xblock_ptr_t copy_block(base::xvblock_t *block) {
    base::xstream_t stream(base::xcontext_t::instance());
    {
        dynamic_cast<xblock_t*>(block)->full_block_serialize_to(stream);
    }

    xblock_ptr_t block_ptr = nullptr;
    {
        xblock_t* _data_obj = dynamic_cast<xblock_t*>(xblock_t::full_block_read_from(stream));
        block_ptr.attach(_data_obj);
    }

    block_ptr->reset_block_flags();
    block_ptr->set_verify_signature(std::string(1,0));
    block_ptr->get_header()->set_last_block_hash(std::string(1,0));
    block_ptr->set_block_flag(base::enum_xvblock_flag_authenticated);

    return block_ptr;
}

TEST(xsync_account, highqc_fork) {

    std::string address = xdatautil::serialize_owner_str(sys_contract_sharding_table_block_addr, 0);

    mock::xvchain_creator creator;
    creator.create_blockstore_with_xstore();
    xobject_ptr_t<store::xstore_face_t> store;
    store.attach(creator.get_xstore());
    xobject_ptr_t<base::xvblockstore_t> blockstore;
    blockstore.attach(creator.get_blockstore());

    xsync_store_shadow_t shadow;
    xsync_store_t sync_store("", make_observer(blockstore), &shadow);
    xmessage_bus_t mbus;
    xmock_vhost_sync_t vhost;
    xsync_sender_t sync_sender("", make_observer(&vhost), nullptr);
    xsync_mock_ratelimit_t ratelimit;

    top::mock::xmock_auth_t auth{1};
    sync::xrole_xips_manager_t manager("");
    xrole_chains_mgr_t chain_manager("");
    xchain_downloader_face_ptr_t chain_downloader =
        std::make_shared<xchain_downloader_t>("", &sync_store, &manager, &chain_manager, make_observer(&mbus), make_observer(&auth), &sync_sender, &ratelimit, address);

    std::vector<base::xvblock_t*> block_vector_1;
    std::vector<base::xvblock_t*> block_vector_2;
    base::xvblock_t* genesis_block = data::xblocktool_t::create_genesis_empty_table(address);

    base::xvblock_t* prev_block = genesis_block;
    block_vector_1.push_back(prev_block);
    block_vector_2.push_back(prev_block);

    for (uint64_t viewid=1; viewid<=19; viewid++) {
        data::xblock_consensus_para_t cs_para(prev_block->get_account(), RandomUint32(), viewid, prev_block->get_viewtoken(), prev_block->get_height() + 1, prev_block->get_second_level_gmtime());
        xvip2_t any_xip={static_cast<xvip_t>(-1),static_cast<uint64_t>(-1)};
        cs_para.set_validator(any_xip);
        prev_block = data::xblocktool_t::create_next_emptyblock(prev_block, cs_para);
        block_vector_1.push_back(prev_block);
        block_vector_2.push_back(prev_block);
    }

/*
                           /--block(h=20,view=20)
                          /
    block(h=19,view=19)--
                          \
                           \--block(h=20,view=21)---block(h=21,view=22)---block(h=22,view=23)
*/

    {
        data::xblock_consensus_para_t cs_para(prev_block->get_account(), prev_block->get_clock(), 20, prev_block->get_viewtoken(), prev_block->get_height() + 1, prev_block->get_second_level_gmtime());
        xvip2_t any_xip={static_cast<xvip_t>(-1),static_cast<uint64_t>(-1)};
        cs_para.set_validator(any_xip);
        base::xvblock_t* blk = data::xblocktool_t::create_next_emptyblock(block_vector_1[19], cs_para);
        block_vector_1.push_back(blk);
    }

    for (uint64_t viewid=21; viewid<=41; viewid++) {
        data::xblock_consensus_para_t cs_para(prev_block->get_account(), prev_block->get_clock(), viewid, prev_block->get_viewtoken(), prev_block->get_height() + 1, prev_block->get_second_level_gmtime());
        xvip2_t any_xip={static_cast<xvip_t>(-1),static_cast<uint64_t>(-1)};
        cs_para.set_validator(any_xip);
        prev_block = data::xblocktool_t::create_next_emptyblock(prev_block, cs_para);
        block_vector_2.push_back(prev_block);
    }

    // set local block  1-20
    {
        for (uint64_t h = 1; h<=20; h++) {
            base::xvblock_t* blk = block_vector_1[h];
            xblock_ptr_t block = copy_block(blk);
            sync_store.store_block(block.get());
        }
    }

    base::xauto_ptr<base::xvblock_t> auto_successor_block = block_vector_2[40];
    xblock_ptr_t successor_block = autoptr_to_blockptr(auto_successor_block);

    top::common::xnode_address_t network_self;
    top::common::xnode_address_t target_address;
    chain_downloader->on_behind(0, successor_block->get_height(), enum_chain_sync_policy_full, network_self, target_address, reason);

}

TEST(xsync_account, lockedqc_fork) {

    std::string address = xdatautil::serialize_owner_str(sys_contract_sharding_table_block_addr, 0);

    mock::xvchain_creator creator;
    creator.create_blockstore_with_xstore();
    xobject_ptr_t<store::xstore_face_t> store;
    store.attach(creator.get_xstore());
    xobject_ptr_t<base::xvblockstore_t> blockstore;
    blockstore.attach(creator.get_blockstore());

    xsync_store_shadow_t shadow;
    xsync_store_t sync_store("", make_observer(blockstore), &shadow);
    xmessage_bus_t mbus;
    xmock_vhost_sync_t vhost;
    xsync_sender_t sync_sender("", make_observer(&vhost), nullptr);
    xsync_mock_ratelimit_t ratelimit;

    top::mock::xmock_auth_t auth{1};
    xrole_xips_manager_t manager("");
    xrole_chains_mgr_t chain_manager("");
    xchain_downloader_face_ptr_t chain_downloader =
        std::make_shared<xchain_downloader_t>("", &sync_store, &manager, &chain_manager, make_observer(&mbus), make_observer(&auth), &sync_sender, &ratelimit, address);

    std::vector<base::xvblock_t*> block_vector_1;
    std::vector<base::xvblock_t*> block_vector_2;
    base::xvblock_t* genesis_block = data::xblocktool_t::create_genesis_empty_table(address);

    base::xvblock_t* prev_block = genesis_block;
    block_vector_1.push_back(prev_block);
    block_vector_2.push_back(prev_block);

    for (uint64_t viewid=1; viewid<=19; viewid++) {
        data::xblock_consensus_para_t cs_para(prev_block->get_account(), RandomUint32(), viewid, prev_block->get_viewtoken(), prev_block->get_height() + 1, prev_block->get_second_level_gmtime());
        xvip2_t any_xip={static_cast<xvip_t>(-1),static_cast<uint64_t>(-1)};
        cs_para.set_validator(any_xip);
        prev_block = data::xblocktool_t::create_next_emptyblock(prev_block, cs_para);
        block_vector_1.push_back(prev_block);
        block_vector_2.push_back(prev_block);
    }

/*
                           /--block(h=20,view=20)---block(h=21,view=21)
                          /
    block(h=19,view=19)--
                          \
                           \--block(h=20,view=22)---block(h=21,view=23)---block(h=22,view=24)
*/
    {
        data::xblock_consensus_para_t cs_para(prev_block->get_account(), RandomUint32(), 20, prev_block->get_viewtoken(), prev_block->get_height() + 1, prev_block->get_second_level_gmtime());
        xvip2_t any_xip={static_cast<xvip_t>(-1),static_cast<uint64_t>(-1)};
        cs_para.set_validator(any_xip);
        base::xvblock_t* blk = data::xblocktool_t::create_next_emptyblock(block_vector_1[19], cs_para);
        block_vector_1.push_back(blk);
    }
    {
        data::xblock_consensus_para_t cs_para(prev_block->get_account(), RandomUint32(), 21, prev_block->get_viewtoken(), prev_block->get_height() + 1, prev_block->get_second_level_gmtime());
        xvip2_t any_xip={static_cast<xvip_t>(-1),static_cast<uint64_t>(-1)};
        cs_para.set_validator(any_xip);
        base::xvblock_t* blk = data::xblocktool_t::create_next_emptyblock(block_vector_1[20], cs_para);
        block_vector_1.push_back(blk);
    }

    for (uint64_t viewid=22; viewid<=41; viewid++) {
        data::xblock_consensus_para_t cs_para(prev_block->get_account(), RandomUint32(), viewid, prev_block->get_viewtoken(), prev_block->get_height() + 1, prev_block->get_second_level_gmtime());
        xvip2_t any_xip={static_cast<xvip_t>(-1),static_cast<uint64_t>(-1)};
        cs_para.set_validator(any_xip);
        prev_block = data::xblocktool_t::create_next_emptyblock(prev_block, cs_para);
        block_vector_2.push_back(prev_block);
    }

    // set local block  1-21
    {
        for (uint64_t h = 1; h<=block_vector_1.size()-1; h++) {
            base::xvblock_t* blk = block_vector_1[h];
            xblock_ptr_t block = copy_block(blk);
            sync_store.store_block(block.get());
        }
    }

    base::xauto_ptr<base::xvblock_t> auto_successor_block = block_vector_2[38];
    xblock_ptr_t successor_block = autoptr_to_blockptr(auto_successor_block);

    top::common::xnode_address_t network_self;
    top::common::xnode_address_t target_address;
    chain_downloader->on_behind(0, successor_block->get_height(), enum_chain_sync_policy_full, network_self, target_address, reason);

}

TEST(xsync_account, chain_snapshot) {
    mock::xvchain_creator creator;
    creator.create_blockstore_with_xstore();
    xobject_ptr_t<store::xstore_face_t> store;
    store.attach(creator.get_xstore());
    xobject_ptr_t<base::xvblockstore_t> blockstore;
    blockstore.attach(creator.get_blockstore());

    xmessage_bus_t mbus;
    xmock_vhost_sync_t vhost;
    xsync_sender_t sync_sender("", make_observer(&vhost), nullptr);
    xsync_mock_ratelimit_t ratelimit;
    top::mock::xmock_auth_t auth{1};

    xsync_store_shadow_t shadow;
    xsync_store_t sync_store("", make_observer(blockstore), &shadow);
    std::vector<observer_ptr<base::xiothread_t>> thread_pool;
    xobject_ptr_t<base::xiothread_t> thread = make_object_ptr<base::xiothread_t>();
    thread_pool.push_back(make_observer(thread));

    xrole_xips_manager_t manager("");
    xrole_chains_mgr_t chain_manager("");
    xdownloader_t download("", &sync_store, make_observer(&mbus), make_observer(&auth), &manager, &chain_manager, &sync_sender, thread_pool, &ratelimit, &shadow);
    download.m_timer_list.clear();
    shadow.set_downloader(&download);
    
    uint64_t max_block_height = 200;
    xdatamock_table mocktable;
    mocktable.genrate_table_chain(max_block_height, blockstore.get());
    std::string address = mocktable.get_account();
    xchain_downloader_face_ptr_t chain_downloader =
        std::make_shared<xchain_downloader_t>("", &sync_store, &manager, &chain_manager, make_observer(&mbus), make_observer(&auth), &sync_sender, &ratelimit, address);
    const std::vector<xblock_ptr_t> & tables = mocktable.get_history_tables();
    xassert(tables.size() == max_block_height+1);
    // tables[101]->set_full_offstate(nullptr);
    for (uint64_t i = 0; i < 100; i++) {
        base::xvaccount_t _vaddress(tables[i]->get_account());
        ASSERT_TRUE(blockstore->store_block(_vaddress, tables[i].get()));
    }
    top::common::xnode_address_t network_self;
    top::common::xnode_address_t target_address;
    chain_downloader->on_behind(101, 140, enum_chain_sync_policy_fast,network_self, target_address, reason);

}
TEST(xsync_account, is_elect_chain)
{
    mock::xvchain_creator creator;
    creator.create_blockstore_with_xstore();
    xobject_ptr_t<store::xstore_face_t> store;
    store.attach(creator.get_xstore());
    xobject_ptr_t<base::xvblockstore_t> blockstore;
    blockstore.attach(creator.get_blockstore());
    xsync_store_shadow_t shadow;
    xsync_store_t sync_store("", make_observer(blockstore), &shadow);

    xmessage_bus_t mbus;
    xmock_vhost_sync_t vhost;
    xsync_sender_t sync_sender("", make_observer(&vhost), nullptr);
    xsync_mock_ratelimit_t ratelimit;

    top::mock::xmock_auth_t auth{1};
    sync::xrole_xips_manager_t manager("");
    xrole_chains_mgr_t chain_manager("");
    {
        std::string address("Ta0000@1");
        xchain_downloader_t chain_downloader("", &sync_store, &manager, &chain_manager, make_observer(&mbus), make_observer(&auth), &sync_sender, &ratelimit, address);
        EXPECT_EQ(chain_downloader.is_elect_chain(), false);
    }
    {
        std::string address("Ta0005@0");
        xchain_downloader_t chain_downloader("", &sync_store, &manager, &chain_manager, make_observer(&mbus), make_observer(&auth), &sync_sender, &ratelimit, address);
        EXPECT_EQ(chain_downloader.is_elect_chain(), true);
    }
}