#pragma once

#include "xdata/xsystem_contract/xdata_structures.h"

#include <gtest/gtest.h>

#define private public
#include "xevm_contract_runtime/sys_contract/xevm_eth_bridge_contract.h"
#include "xdata/xtable_bstate.h"

namespace top {
namespace tests {

class xmock_statectx_t : public statectx::xstatectx_face_t {
public:
    xmock_statectx_t(data::xunitstate_ptr_t s) {
        ustate = s;
    }

    const data::xtablestate_ptr_t & get_table_state() const override {
        return table_state;
    }

    data::xunitstate_ptr_t load_unit_state(const base::xvaccount_t & addr) override {
        return ustate;
    }

    bool do_rollback() override {
        return false;
    }

    size_t do_snapshot() override {
        return 0;
    }

    const std::string & get_table_address() const override {
        return table_address;
    }

    bool is_state_dirty() const override {
        return true;
    }

    data::xtablestate_ptr_t table_state{nullptr};
    data::xunitstate_ptr_t ustate{nullptr};
    std::string table_address{eth_table_address.value()};
};

class xcontract_fixture_t : public testing::Test {
public:
    xcontract_fixture_t() {
    }

    void init() {
        auto bstate = make_object_ptr<base::xvbstate_t>(evm_eth_bridge_contract_address.value(), (uint64_t)0, (uint64_t)0, std::string(), std::string(), (uint64_t)0, (uint32_t)0, (uint16_t)0);
        auto canvas = make_object_ptr<base::xvcanvas_t>();
        bstate->new_string_map_var(data::system_contract::XPROPERTY_ETH_CHAINS_HEADER, canvas.get());
        bstate->new_string_map_var(data::system_contract::XPROPERTY_ETH_CHAINS_HASH, canvas.get());
        bstate->new_string_var(data::system_contract::XPROPERTY_ETH_CHAINS_HEIGHT, canvas.get());
        auto bytes = evm_common::toBigEndian(evm_common::u256(0));
        bstate->load_string_var(data::system_contract::XPROPERTY_ETH_CHAINS_HEIGHT)->reset({bytes.begin(), bytes.end()}, canvas.get());
        if (contract.m_contract_state == nullptr) {
            contract.m_contract_state = std::make_shared<data::xunit_bstate_t>(bstate.get(), false);
        }
        data::xunitstate_ptr_t ustate = std::make_shared<data::xunit_bstate_t>(bstate.get(), false);
        statectx = make_unique<xmock_statectx_t>(ustate);
        statectx_observer = make_observer<statectx::xstatectx_face_t>(statectx.get());
        context.address.build_from("ff00000000000000000000000000000000000002");
        context.caller.build_from("f8a1e199c49c2ae2682ecc5b4a8838b39bab1a38");
    }

    void SetUp() override {
        init();
    }

    void TearDown() override {
    }

    contract_runtime::evm::sys_contract::xevm_eth_bridge_contract_t contract;
    std::unique_ptr<statectx::xstatectx_face_t> statectx;
    observer_ptr<statectx::xstatectx_face_t> statectx_observer;
    contract_runtime::evm::sys_contract_context context;
};

}
}