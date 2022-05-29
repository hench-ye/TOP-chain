// Copyright (c) 2017-2022 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xgasfee/xgas_tx_operator.h"

#include "xconfig/xconfig_register.h"
#include "xconfig/xpredefined_configurations.h"
#include "xdata/xgenesis_data.h"
#include "xdata/xnative_contract_address.h"
#include "xgasfee/xerror/xerror.h"

#include <stdint.h>

namespace top {
namespace gasfee {

xtop_gas_tx_operator::xtop_gas_tx_operator(xobject_ptr_t<data::xcons_transaction_t> const & tx) : m_tx(tx) {
}

common::xaccount_address_t xtop_gas_tx_operator::sender() const {
    return common::xaccount_address_t{m_tx->get_source_addr()};
}

common::xaccount_address_t xtop_gas_tx_operator::recver() const {
    return common::xaccount_address_t{m_tx->get_target_addr()};
}

std::string xtop_gas_tx_operator::sender_str() const {
    return m_tx->get_source_addr();
}

std::string xtop_gas_tx_operator::recver_str() const {
    return m_tx->get_target_addr();
}

uint64_t xtop_gas_tx_operator::deposit() const {
    return m_tx->get_transaction()->get_deposit();
}

data::enum_xtransaction_type xtop_gas_tx_operator::tx_type() const {
    return static_cast<data::enum_xtransaction_type>(m_tx->get_tx_type());
}

base::enum_transaction_subtype xtop_gas_tx_operator::tx_subtype() const {
    return m_tx->get_tx_subtype();
}

uint64_t xtop_gas_tx_operator::tx_last_action_used_deposit() const {
    return m_tx->get_last_action_used_deposit();
}

data::enum_xtransaction_version xtop_gas_tx_operator::tx_version() const {
    return static_cast<data::enum_xtransaction_version>(m_tx->get_transaction()->get_tx_version());
}

evm_common::u256 xtop_gas_tx_operator::tx_eth_gas_limit() const {
    return m_tx->get_transaction()->get_gaslimit();
}

evm_common::u256 xtop_gas_tx_operator::tx_eth_fee_per_gas() const {
    return m_tx->get_transaction()->get_max_fee_per_gas();
}

evm_common::u256 xtop_gas_tx_operator::tx_eth_limited_gasfee() const {
    // 1Gwei = (ratio / 10^3)Utop
    // 1Utop = (10^3 / ratio)Gwei
    auto price = tx_eth_gas_limit();
    auto limit = tx_eth_fee_per_gas();
    auto wei_gasfee = limit * price;
    auto utop_gasfee = wei_to_utop(wei_gasfee);
    xdbg("[xtop_gas_tx_operator::tx_eth_limited_gasfee] eth_gas_price: %s, eth_gas_limit: %s, wei_gasfee: %s, utop_gasfee: %s",
         price.str().c_str(),
         limit.str().c_str(),
         wei_gasfee.str().c_str(),
         utop_gasfee.str().c_str());
    return utop_gasfee;
}

evm_common::u256 xtop_gas_tx_operator::wei_to_utop(const evm_common::u256 wei) const {
    auto utop = wei * XGET_ONCHAIN_GOVERNANCE_PARAMETER(eth_to_top_exchange_ratio) / evm_common::u256(1000000000000ULL);
    xdbg("[xtop_gas_tx_operator::wei_to_utop] exchange ratio: %lu, wei: %s, utop: %s",
         XGET_ONCHAIN_GOVERNANCE_PARAMETER(eth_to_top_exchange_ratio),
         wei.str().c_str(),
         utop.str().c_str());
    return utop;
}

evm_common::u256 xtop_gas_tx_operator::utop_to_wei(const evm_common::u256 utop) const {
    auto wei = utop * evm_common::u256(1000000000000ULL) / XGET_ONCHAIN_GOVERNANCE_PARAMETER(eth_to_top_exchange_ratio);
    xdbg("[xtop_gas_tx_operator::utop_to_wei] exchange ratio: %lu, utop: %s, wei: %s",
         XGET_ONCHAIN_GOVERNANCE_PARAMETER(eth_to_top_exchange_ratio),
         utop.str().c_str(),
         wei.str().c_str());
    return wei;
}

uint64_t xtop_gas_tx_operator::tx_fixed_tgas() const {
    uint64_t fixed_tgas{0};
#ifndef XENABLE_MOCK_ZEC_STAKE
    if (recver_str().empty()) {
        return 0;
    }
    if (!data::is_sys_contract_address(sender()) && data::is_beacon_contract_address(recver())) {
        fixed_tgas = balance_to_tgas(XGET_ONCHAIN_GOVERNANCE_PARAMETER(beacon_tx_fee));
    }
#endif
    return fixed_tgas;
}

uint64_t xtop_gas_tx_operator::tx_bandwith_tgas() const {
#ifdef ENABLE_SCALE
    uint16_t amplify = 5;
#else
    uint16_t amplify = 1;
#endif
    if (tx_type() != data::xtransaction_type_transfer) {
        amplify = 1;
    }
    evm_common::u256 multiple{3};
    evm_common::u256 bandwith_tgas = multiple * amplify * m_tx->get_transaction()->get_tx_len();
    return static_cast<uint64_t>(bandwith_tgas);
}

uint64_t xtop_gas_tx_operator::tx_disk_tgas() const {
    if (tx_type() == data::xtransaction_type_transfer) {
        return 0;
    }
    evm_common::u256 multiple{1};
    // evm deploy tx
    if (recver_str().empty() || recver() == evm_zero_address) {
        multiple = 1200000UL;
    }
    evm_common::u256 disk_tgas = multiple * m_tx->get_transaction()->get_tx_len();
    return static_cast<uint64_t>(disk_tgas);
}

bool xtop_gas_tx_operator::is_one_stage_tx() const {
    return (m_tx->is_self_tx() || m_tx->get_inner_table_flag());
}

uint64_t xtop_gas_tx_operator::balance_to_tgas(const uint64_t balance) const {
    xassert(XGET_ONCHAIN_GOVERNANCE_PARAMETER(tx_deposit_gas_exchange_ratio) > 0);
    return balance / XGET_ONCHAIN_GOVERNANCE_PARAMETER(tx_deposit_gas_exchange_ratio);
}

uint64_t xtop_gas_tx_operator::tgas_to_balance(const uint64_t tgas) const {
    return tgas * XGET_ONCHAIN_GOVERNANCE_PARAMETER(tx_deposit_gas_exchange_ratio);
}

evm_common::u256 xtop_gas_tx_operator::estimate_eth_base_price() const{
    return evm_common::u256{5000000000UL};
}

evm_common::u256 xtop_gas_tx_operator::estimate_eth_gas_price() const {
    return estimate_eth_base_price() * evm_common::u256(5U);
}

evm_common::u256 xtop_gas_tx_operator::estimate_eth_gas_limit(const evm_common::u256 evm_gas) const {
    evm_common::u256 total_tgas = tx_bandwith_tgas() + tx_disk_tgas() + evm_gas * XGET_ONCHAIN_GOVERNANCE_PARAMETER(eth_gas_to_tgas_exchange_ratio);
    evm_common::u256 total_utop = total_tgas * XGET_ONCHAIN_GOVERNANCE_PARAMETER(tx_deposit_gas_exchange_ratio);
    evm_common::u256 total_wei = utop_to_wei(total_utop);
    evm_common::u256 max_gas = total_wei / estimate_eth_base_price();
    return max_gas;
}

}  // namespace gasfee
}  // namespace top