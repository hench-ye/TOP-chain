// Copyright (c) 2017-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include "xbasic/xmemory.hpp"
#include "xcontract_common/xcontract_face.h"
#include "xcontract_common/xcontract_fwd.h"
#include "xcontract_common/xcontract_state.h"
#include "xcontract_common/xproperties/xproperty_token.h"
#include "xstate_accessor/xtoken.h"

NS_BEG2(top, contract_common)

enum class xtop_contract_type: std::uint8_t {
    invalid,
    system,
    user,
};
using xcontract_type_t = xtop_contract_type;

struct xtop_contract_metadata {
    xtop_contract_metadata() = default;
    xtop_contract_metadata(xtop_contract_metadata const& meta) = default;
    xtop_contract_metadata(common::xaccount_address_t const& account, xcontract_type_t type = xcontract_type_t::system);

    xcontract_type_t m_type{ xcontract_type_t::invalid };
    common::xaccount_address_t m_account{};
};

class xtop_basic_contract : public xcontract_face_t {
protected:
    observer_ptr<xcontract_execution_context_t> m_associated_execution_context{nullptr};
    xcontract_metadata_t m_contract_meta;

    properties::xtoken_property_t m_balance{this};

protected:
    xtop_basic_contract() = default;

public:
    xtop_basic_contract(xtop_basic_contract const &) = delete;
    xtop_basic_contract & operator=(xtop_basic_contract const &) = delete;
    xtop_basic_contract(xtop_basic_contract &&) = default;
    xtop_basic_contract & operator=(xtop_basic_contract &&) = default;
    ~xtop_basic_contract() override = default;

    common::xaccount_address_t address() const override final;
    common::xaccount_address_t sender() const override final;
    common::xaccount_address_t recver() const override final;

    uint64_t balance() const override;
    state_accessor::xtoken_t withdraw(std::uint64_t amount) override;
    void deposit(state_accessor::xtoken_t token) override;

    observer_ptr<xcontract_state_t> contract_state() const noexcept;

    void reset_execution_context(observer_ptr<xcontract_execution_context_t> exe_ctx) override final;

    observer_ptr<xcontract_state_t> state() const noexcept;
    xcontract_type_t type() const;
    data::enum_xaction_type action_type() const;
    data::enum_xaction_type source_action_type() const;
    data::enum_xaction_type target_action_type() const;
    xbyte_buffer_t action_data() const;
    std::string source_action_data() const;
    std::string target_action_data() const;
    state_accessor::xtoken_t src_action_asset(std::error_code & ec) const;
    data::enum_xtransaction_type transaction_type() const;
    common::xlogic_time_t time() const;
    common::xlogic_time_t timestamp() const;
    uint64_t state_height(common::xaccount_address_t const & address = common::xaccount_address_t{}) const;
    bool block_exist(common::xaccount_address_t const & address, uint64_t height) const;
    std::vector<xfollowup_transaction_datum_t> followup_transaction() const;

protected:
    bool at_source_action_stage() const noexcept override final;
    bool at_target_action_stage() const noexcept override final;
    bool at_confirm_action_stage() const noexcept override final;

    xbyte_buffer_t const & receipt_data(std::string const & key, std::error_code & ec) const override final;
    void write_receipt_data(std::string const & key, xbyte_buffer_t value, std::error_code & ec) override final;
    void call(common::xaccount_address_t const & target_addr,
              std::string const & method_name,
              std::string const & method_params,
              xfollowup_transaction_schedule_type_t type) override;
    void transfer(common::xaccount_address_t const & target_addr, uint64_t amount, xfollowup_transaction_schedule_type_t type, std::error_code & ec);
};

NS_END2
