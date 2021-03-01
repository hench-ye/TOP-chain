// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
#define private public
#include "tests/xsystem_contract/xrec_standby_algorithm/xtest_rec_standby_contract_fixture.h"

#include <gtest/gtest.h>

NS_BEG3(top, tests, rec_standby)

class xtest_rec_standby_contract_algorithm
  : public xtop_test_rec_standby_contract_fixture
  , public testing::Test {
public:
    XDECLARE_DEFAULTED_DEFAULT_CONSTRUCTOR(xtest_rec_standby_contract_algorithm);
    XDECLARE_DELETED_COPY_DEFAULTED_MOVE_SEMANTICS(xtest_rec_standby_contract_algorithm);
    XDECLARE_DEFAULTED_OVERRIDE_DESTRUCTOR(xtest_rec_standby_contract_algorithm);

protected:
    void SetUp() override {
        m_registration_data.clear();
        standby_result_store.m_results.clear();
    }

    void TearDown() override {
    }
};

TEST_F(xtest_rec_standby_contract_algorithm, test_TOP_3495) {
    std::string node_id{"test_node_id_1"};
    common::xnode_id_t xnode_id{node_id};
    std::string program_version_1{"verison_1"};
    top::xpublic_key_t pub_key_1{"test_pub_key_1"};

    xstake::xreg_node_info node_info;
    node_info.consensus_public_key = pub_key_1;
    node_info.m_account_mortgage = 1000000000000;
    node_info.m_registered_role = common::xrole_type_t::advance;
    node_info.m_account = node_id;
    node_info.m_genesis_node = false;
    node_info.m_network_ids = std::set<uint32_t>({255});
    add_reg_info(node_info);

    EXPECT_TRUE(rec_standby_contract.nodeJoinNetworkImpl1(node_id, program_version_1, node_info, standby_result_store));

    auto & standby_node_info = standby_result_store.result_of(common::xnetwork_id_t{255}).result_of(xnode_id);
    EXPECT_TRUE(standby_node_info.program_version == program_version_1);
    EXPECT_TRUE(standby_node_info.consensus_public_key == pub_key_1);
    EXPECT_TRUE(standby_node_info.is_genesis_node == false);

    // changed program_version and rejoin:
    // #TOP-3495:
    std::string program_version_2{"version_2"};
    EXPECT_TRUE(rec_standby_contract.nodeJoinNetworkImpl1(node_id, program_version_2, node_info, standby_result_store));  // return true. but program_version has not be updated.
    EXPECT_FALSE(standby_node_info.program_version == program_version_2);
    // after fork:
    EXPECT_TRUE(rec_standby_contract.nodeJoinNetworkImpl2(node_id, program_version_2, node_info, standby_result_store));
    EXPECT_TRUE(standby_node_info.program_version == program_version_2);
    EXPECT_FALSE(rec_standby_contract.nodeJoinNetworkImpl2(node_id, program_version_2, node_info, standby_result_store));  // rejoin shouldn't changed the result.

    top::xpublic_key_t pub_key_2{"test_pub_key_2"};
    node_info.consensus_public_key = pub_key_2;
    EXPECT_TRUE(rec_standby_contract.nodeJoinNetworkImpl2(node_id, program_version_2, node_info, standby_result_store));
    EXPECT_TRUE(standby_node_info.consensus_public_key == pub_key_2);
}

TEST_F(xtest_rec_standby_contract_algorithm, test_on_timer_update_pubkey_and_role) {
    std::string node_id{"test_node_id_1"};
    common::xnode_id_t xnode_id{node_id};
    std::string program_version_1{"verison_1"};
    top::xpublic_key_t pub_key_1{"test_pub_key_1"};

    xstake::xreg_node_info node_info;
    node_info.consensus_public_key = pub_key_1;
    node_info.m_account_mortgage = 1000000000000;
    node_info.m_vote_amount = 1000000000;
    node_info.m_registered_role = common::xrole_type_t::advance;
    node_info.m_account = node_id;
    node_info.m_genesis_node = false;
    node_info.m_network_ids = std::set<uint32_t>({255});
    add_reg_info(node_info);

    EXPECT_TRUE(rec_standby_contract.nodeJoinNetworkImpl1(node_id, program_version_1, node_info, standby_result_store));

    auto & standby_node_info = standby_result_store.result_of(common::xnetwork_id_t{255}).result_of(xnode_id);
    EXPECT_TRUE(standby_node_info.program_version == program_version_1);
    EXPECT_TRUE(standby_node_info.consensus_public_key == pub_key_1);
    EXPECT_TRUE(standby_node_info.is_genesis_node == false);

#define rec_standby_on_timer_update rec_standby_contract.update_standby_result_store(m_registration_data, standby_result_store, record)

    EXPECT_FALSE(rec_standby_on_timer_update);

    // changed pub_key in reg && rec_standby on_timer update:
    top::xpublic_key_t pub_key_2{"test_pub_key_2"};
    change_public_key(xnode_id, pub_key_2);
    EXPECT_TRUE(rec_standby_on_timer_update);
    EXPECT_TRUE(standby_node_info.consensus_public_key == pub_key_2);
    change_public_key(xnode_id, pub_key_1);
    EXPECT_TRUE(rec_standby_on_timer_update);

#define EXPECT_HAS(node_type) EXPECT_TRUE(standby_node_info.stake_container.find(node_type) != standby_node_info.stake_container.end())
#define EXPECT_HAS_NOT(node_type) EXPECT_TRUE(standby_node_info.stake_container.find(node_type) == standby_node_info.stake_container.end())

    EXPECT_HAS(common::xnode_type_t::consensus_auditor);
    EXPECT_HAS(common::xnode_type_t::archive);
    EXPECT_HAS(common::xnode_type_t::rec);
    EXPECT_HAS(common::xnode_type_t::zec);
    EXPECT_HAS(common::xnode_type_t::consensus_validator);
    EXPECT_HAS_NOT(common::xnode_type_t::edge);

    change_role_type(xnode_id, common::xrole_type_t::consensus);
    EXPECT_HAS(common::xnode_type_t::consensus_auditor);
    EXPECT_HAS(common::xnode_type_t::archive);
    EXPECT_HAS(common::xnode_type_t::rec);
    EXPECT_HAS(common::xnode_type_t::zec);
    EXPECT_HAS(common::xnode_type_t::consensus_validator);
    EXPECT_HAS_NOT(common::xnode_type_t::edge);
    EXPECT_TRUE(rec_standby_on_timer_update);
    EXPECT_HAS_NOT(common::xnode_type_t::consensus_auditor);
    EXPECT_HAS_NOT(common::xnode_type_t::archive);
    EXPECT_HAS_NOT(common::xnode_type_t::rec);
    EXPECT_HAS_NOT(common::xnode_type_t::zec);
    EXPECT_HAS(common::xnode_type_t::consensus_validator);
    EXPECT_HAS_NOT(common::xnode_type_t::edge);

    change_role_type(xnode_id, common::xrole_type_t::edge);
    EXPECT_HAS_NOT(common::xnode_type_t::consensus_auditor);
    EXPECT_HAS_NOT(common::xnode_type_t::archive);
    EXPECT_HAS_NOT(common::xnode_type_t::rec);
    EXPECT_HAS_NOT(common::xnode_type_t::zec);
    EXPECT_HAS(common::xnode_type_t::consensus_validator);
    EXPECT_HAS_NOT(common::xnode_type_t::edge);
    EXPECT_TRUE(rec_standby_on_timer_update);
    EXPECT_HAS_NOT(common::xnode_type_t::consensus_auditor);
    EXPECT_HAS_NOT(common::xnode_type_t::archive);
    EXPECT_HAS_NOT(common::xnode_type_t::rec);
    EXPECT_HAS_NOT(common::xnode_type_t::zec);
    EXPECT_HAS_NOT(common::xnode_type_t::consensus_validator);
    EXPECT_HAS(common::xnode_type_t::edge);

#undef rec_standby_on_timer_update
#undef EXPECT_HAS
#undef EXPECT_HAS_NOT
}

TEST_F(xtest_rec_standby_contract_algorithm, test_on_timer_update_stake) {
    std::string node_id{"test_node_id_1"};
    common::xnode_id_t xnode_id{node_id};
    std::string program_version_1{"verison_1"};
    top::xpublic_key_t pub_key_1{"test_pub_key_1"};

    xstake::xreg_node_info node_info;
    node_info.consensus_public_key = pub_key_1;
    node_info.m_account_mortgage = 1000000000000;
    node_info.m_vote_amount = 1;
    node_info.m_registered_role = common::xrole_type_t::advance;
    node_info.m_account = node_id;
    node_info.m_genesis_node = false;
    node_info.m_network_ids = std::set<uint32_t>({255});
    EXPECT_TRUE(add_reg_info(node_info));

    EXPECT_TRUE(rec_standby_contract.nodeJoinNetworkImpl1(node_id, program_version_1, node_info, standby_result_store));

    auto & standby_node_info = standby_result_store.result_of(common::xnetwork_id_t{255}).result_of(xnode_id);
    EXPECT_TRUE(standby_node_info.program_version == program_version_1);
    EXPECT_TRUE(standby_node_info.consensus_public_key == pub_key_1);
    EXPECT_TRUE(standby_node_info.is_genesis_node == false);
    
#define rec_standby_on_timer_update rec_standby_contract.update_standby_result_store(m_registration_data, standby_result_store, record)
#define EXPECT_HAS(node_type) EXPECT_TRUE(standby_node_info.stake_container.find(node_type) != standby_node_info.stake_container.end())
#define EXPECT_HAS_NOT(node_type) EXPECT_TRUE(standby_node_info.stake_container.find(node_type) == standby_node_info.stake_container.end())
    EXPECT_FALSE(node_info.auditor());
    EXPECT_HAS_NOT(common::xnode_type_t::consensus_auditor);
    EXPECT_HAS_NOT(common::xnode_type_t::archive);
    EXPECT_HAS(common::xnode_type_t::consensus_validator);
    EXPECT_HAS(common::xnode_type_t::rec);
    EXPECT_HAS(common::xnode_type_t::zec);

    node_info.m_vote_amount = 1000000000;
    EXPECT_TRUE(update_reg_info(node_info));

    EXPECT_TRUE(node_info.auditor());
    EXPECT_TRUE(rec_standby_on_timer_update);

    EXPECT_HAS(common::xnode_type_t::consensus_auditor);
    EXPECT_HAS(common::xnode_type_t::archive);
    EXPECT_HAS(common::xnode_type_t::consensus_validator);
    EXPECT_HAS(common::xnode_type_t::rec);
    EXPECT_HAS(common::xnode_type_t::zec);
#undef rec_standby_on_timer_update
#undef EXPECT_HAS
#undef EXPECT_HAS_NOT
}

NS_END3