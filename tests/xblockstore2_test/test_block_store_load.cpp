#include "gtest/gtest.h"

#include "xbase/xcontext.h"
#include "xbase/xdata.h"
#include "xbase/xobject.h"
#include "xbase/xmem.h"
#include "xbase/xcontext.h"
// TODO(jimmy) #include "xbase/xvledger.h"

#include "xdata/xdatautil.h"
#include "xdata/xemptyblock.h"
#include "xdata/xblocktool.h"
#include "xdata/xlightunit.h"
#include "xmbus/xevent_store.h"
#include "xmbus/xmessage_bus.h"

// #include "test_blockmock.hpp"
#include "xstore/xstore.h"
#include "xblockstore/xblockstore_face.h"
#include "tests/mock/xvchain_creator.hpp"
#include "tests/mock/xdatamock_table.hpp"

using namespace top;
using namespace top::base;
using namespace top::mbus;
using namespace top::store;
using namespace top::data;
using namespace top::mock;

class test_block_store_load : public testing::Test {
protected:
    void SetUp() override {
    }

    void TearDown() override {
    }
};

TEST_F(test_block_store_load, store_batch_tables) {
    mock::xvchain_creator creator;
    base::xvblockstore_t* blockstore = creator.get_blockstore();

    uint64_t max_block_height = 19;
    mock::xdatamock_table mocktable(1, 4);
    mocktable.genrate_table_chain(max_block_height);
    const std::vector<xblock_ptr_t> & tableblocks = mocktable.get_history_tables();
    xassert(tableblocks.size() == max_block_height + 1);

    for (auto & block : tableblocks) {
        ASSERT_TRUE(blockstore->store_block(mocktable, block.get()));
    }
}

TEST_F(test_block_store_load, load_unexsit_block_1) {
    mock::xvchain_creator creator;
    base::xvblockstore_t* blockstore = creator.get_blockstore();

    std::string _test_addr = xdatamock_address::make_user_address_random(1);
    auto _block = blockstore->load_block_object(base::xvaccount_t(_test_addr), 0, 0, false);
    ASSERT_NE(_block, nullptr);  // TODO(jimmy) blockstore always will return genesis block
}
TEST_F(test_block_store_load, load_unexsit_block_2) {
    mock::xvchain_creator creator;
    base::xvblockstore_t* blockstore = creator.get_blockstore();

    mock::xdatamock_table mocktable;
    mocktable.genrate_table_chain(5);
    {
        auto _block = blockstore->load_block_object(mocktable, 1, 0, false);
        ASSERT_EQ(_block, nullptr);
    }
    {
        const std::vector<xblock_ptr_t> & tableblocks = mocktable.get_history_tables();
        bool ret = blockstore->store_block(mocktable, tableblocks[1].get());
        ASSERT_EQ(ret, true);        
    }
    {
        auto _block = blockstore->load_block_object(mocktable, 1, 0, false);
        ASSERT_NE(_block, nullptr);
        ASSERT_EQ(_block->is_output_ready(true), false);
        ASSERT_EQ(_block->is_input_ready(true), false);
    }
    {
        auto _block = blockstore->load_block_object(mocktable, 1, 0, true);
        ASSERT_NE(_block, nullptr);
        ASSERT_EQ(_block->is_output_ready(true), true);
        ASSERT_EQ(_block->is_input_ready(true), true);
    }    
}
TEST_F(test_block_store_load, load_unexsit_block_3) {
    mock::xvchain_creator creator;
    base::xvblockstore_t* blockstore = creator.get_blockstore();

    mock::xdatamock_table mocktable;
    mocktable.genrate_table_chain(20);
    const std::vector<xblock_ptr_t> & tableblocks = mocktable.get_history_tables();
    const std::vector<xdatamock_unit> & mockunits = mocktable.get_mock_units();    
    {
        auto _block = blockstore->load_block_object(base::xvaccount_t(mockunits[0].get_account()), 1, 0, false);
        ASSERT_EQ(_block, nullptr);
    }
    {
        const std::vector<xblock_ptr_t> & unitblocks = mockunits[0].get_history_units();
        bool ret = blockstore->store_block(base::xvaccount_t(mockunits[0].get_account()), unitblocks[1].get());
        ASSERT_EQ(ret, true);        
    }
    {
        auto _block = blockstore->load_block_object(base::xvaccount_t(mockunits[0].get_account()), 1, 0, false);
        ASSERT_NE(_block, nullptr);
        ASSERT_EQ(_block->is_output_ready(true), false);
        ASSERT_EQ(_block->is_input_ready(true), false);
    }
    {
        auto _block = blockstore->load_block_object(base::xvaccount_t(mockunits[0].get_account()), 1, 0, true);
        ASSERT_NE(_block, nullptr);
        ASSERT_EQ(_block->is_output_ready(true), true);
        ASSERT_EQ(_block->is_input_ready(true), true);
    }    
}

TEST_F(test_block_store_load, load_units_BENCH) {
    mock::xvchain_creator creator;
    base::xvblockstore_t* blockstore = creator.get_blockstore();

    uint64_t max_block_height = 1000;
    uint32_t user_count = 20;
    uint16_t tableid = 1;
    mock::xdatamock_table mocktable(tableid, user_count);
    mocktable.genrate_table_chain(max_block_height);
    const std::vector<xblock_ptr_t> & tableblocks = mocktable.get_history_tables();
    const std::vector<xdatamock_unit> & mockunits = mocktable.get_mock_units();

    {
        auto start_time = std::chrono::system_clock::now();
        for (auto & block : tableblocks) {
            ASSERT_TRUE(blockstore->store_block(mocktable, block.get()));
        }
        auto end_time = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << " store all blocks milliseconds " << duration.count() << std::endl;
    }

    {
        auto start_time = std::chrono::system_clock::now();
        for (auto & mockunit : mockunits) {
            uint64_t unit_height = mockunit.get_cert_block()->get_height();
            for (uint64_t height = 1; height <= unit_height; height++) {
                base::xvaccount_t _vaddr(mockunit.get_account());
                auto _block = blockstore->load_block_object(_vaddr, height, 0, false);
                blockstore->load_block_input(_vaddr, _block.get());
                blockstore->load_block_output(_vaddr, _block.get());
            }
        }
        auto end_time = std::chrono::system_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        std::cout << " load all blocks milliseconds " << duration.count() << std::endl;
    }

}


TEST_F(test_block_store_load, mock_table_unit_1) {
    mock::xvchain_creator creator;
    base::xvblockstore_t* blockstore = creator.get_blockstore();

    std::string table_addr = xdatamock_address::make_consensus_table_address(1);
    std::vector<std::string> unit_addrs = xdatamock_address::make_multi_user_address_in_table(table_addr, 4);
    std::string from_addr = unit_addrs[0];
    std::string to_addr = unit_addrs[3];
    mock::xdatamock_table mocktable(table_addr, unit_addrs);

    std::vector<xcons_transaction_ptr_t> send_txs = mocktable.create_send_txs(from_addr, to_addr, 2);
    mocktable.push_txs(send_txs);
    xblock_ptr_t _tableblock1 = mocktable.generate_one_table();
    mocktable.generate_one_table();
    mocktable.generate_one_table();
    {
        xassert(_tableblock1->get_height() == 1);
        xassert(_tableblock1->get_block_class() == base::enum_xvblock_class_light);
        std::vector<xobject_ptr_t<base::xvblock_t>> sub_blocks;
        _tableblock1->extract_sub_blocks(sub_blocks);
        xassert(sub_blocks.size() == 1);
    }

    std::vector<xcons_transaction_ptr_t> recv_txs = mocktable.create_receipts(_tableblock1);
    xassert(recv_txs.size() == send_txs.size());
    for (auto & tx : recv_txs) {
        xassert(tx->is_recv_tx());
    }
    mocktable.push_txs(recv_txs);    
    xblock_ptr_t _tableblock2 = mocktable.generate_one_table();
    mocktable.generate_one_table();
    mocktable.generate_one_table();    
    {
        xassert(_tableblock2->get_height() == 4);
        xassert(_tableblock2->get_block_class() == base::enum_xvblock_class_light);
        std::vector<xobject_ptr_t<base::xvblock_t>> sub_blocks;
        _tableblock2->extract_sub_blocks(sub_blocks);
        xassert(sub_blocks.size() == 1);
    }

    std::vector<xcons_transaction_ptr_t> confirm_txs = mocktable.create_receipts(_tableblock2);
    xassert(confirm_txs.size() == send_txs.size());
    for (auto & tx : confirm_txs) {
        xassert(tx->is_confirm_tx());
    }
    mocktable.push_txs(confirm_txs); 
    xblock_ptr_t _tableblock3 = mocktable.generate_one_table();
    mocktable.generate_one_table();
    mocktable.generate_one_table();
    {
        xassert(_tableblock3->get_height() == 7);
        xassert(_tableblock3->get_block_class() == base::enum_xvblock_class_light);
        std::vector<xobject_ptr_t<base::xvblock_t>> sub_blocks;
        _tableblock3->extract_sub_blocks(sub_blocks);
        xassert(sub_blocks.size() == 1);
    }    
}

TEST_F(test_block_store_load, mock_table_unit_2) {
    mock::xvchain_creator creator;
    base::xvblockstore_t* blockstore = creator.get_blockstore();

    std::string table_addr = xdatamock_address::make_consensus_table_address(1);
    std::vector<std::string> unit_addrs = xdatamock_address::make_multi_user_address_in_table(table_addr, 4);
    std::string from_addr = unit_addrs[0];
    std::string to_addr = unit_addrs[3];
    mock::xdatamock_table mocktable(table_addr, unit_addrs);

    std::vector<xcons_transaction_ptr_t> send_txs = mocktable.create_send_txs(from_addr, to_addr, 2);
    mocktable.push_txs(send_txs);
    xblock_ptr_t _tableblock1 = mocktable.generate_one_table();
    mocktable.generate_one_table();
    mocktable.generate_one_table();

    std::vector<xcons_transaction_ptr_t> recv_txs = mocktable.create_receipts(_tableblock1);
    xassert(recv_txs.size() == send_txs.size());
    for (auto & tx : recv_txs) {
        xassert(tx->is_recv_tx());
    }
    mocktable.push_txs(recv_txs);    
    xblock_ptr_t _tableblock2 = mocktable.generate_one_table();
    mocktable.generate_one_table();
    mocktable.generate_one_table();    

    std::vector<xcons_transaction_ptr_t> confirm_txs = mocktable.create_receipts(_tableblock2);
    xassert(confirm_txs.size() == send_txs.size());
    for (auto & tx : confirm_txs) {
        xassert(tx->is_confirm_tx());
    }
    mocktable.push_txs(confirm_txs); 
    xblock_ptr_t _tableblock3 = mocktable.generate_one_table();
    mocktable.generate_one_table();
    mocktable.generate_one_table();
}
