// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <string>
#include "xbasic/xhex.h"
#include "xconfig/xconfig_register.h"
#include "xconfig/xpredefined_configurations.h"
#include "xdata/xethheader.h"
#include "xcommon/xerror/xerror.h"

NS_BEG2(top, data)

void xeth_header_t::set_format(enum_eth_header_format format) {
    m_format = format;
}

void xeth_header_t::set_gaslimit(uint64_t gaslimit) {
    m_gaslimit = gaslimit;
}

void xeth_header_t::set_gasused(uint64_t gasused) {
    m_gasused = gasused;
}
void xeth_header_t::set_baseprice(const evm_common::u256 & price) {
    m_baseprice = price;
}
void xeth_header_t::set_logBloom(const evm_common::xbloom9_t & bloom) {
    m_logBloom = bloom;
}
void xeth_header_t::set_transactions_root(const evm_common::xh256_t & root) {
    m_transactions_root = root;
}
void xeth_header_t::set_receipts_root(const evm_common::xh256_t & root) {
    m_receipts_root = root;
}
void xeth_header_t::set_state_root(const evm_common::xh256_t & root) {
    m_state_root = root;
}

void xeth_header_t::streamRLP(evm_common::RLPStream& _s) const {
    // todo: use different way to serialize for nil block and full block to reduce storage cost.
    if (m_format == ETH_HEADER_fORMAT_NORMAL) {
        _s.appendList(8);
    } else {
        _s.appendList(2);
    }
    _s << m_gaslimit;
    _s << m_baseprice;
    if (m_format == ETH_HEADER_fORMAT_NORMAL) {
        _s << m_gasused;
        _s << m_logBloom.get_data();
        _s << m_transactions_root;
        _s << m_receipts_root;
        _s << m_state_root;
        _s << m_extra_data;
    }
}

void xeth_header_t::decodeRLP(evm_common::RLP const& _r, std::error_code & ec) {
    if ((!_r.isList()) || (_r.itemCount() != 8 && _r.itemCount() != 2) ) {
        ec = common::error::xerrc_t::invalid_rlp_stream;
        xerror("xeth_header_t::decodeRLP fail item count,%d", _r.itemCount());
        return;
    }

    int field = 0;
    try
    {
        m_gaslimit = _r[field = 0].toInt<uint64_t>();
        m_baseprice = _r[field = 1].toInt<evm_common::u256>();
        if (_r.itemCount() == 8) {
            m_format = ETH_HEADER_fORMAT_NORMAL;
            m_gasused = _r[field = 2].toInt<uint64_t>();
            xbytes_t logbloom_bytes = _r[field = 3].toBytes();
            m_logBloom = evm_common::xbloom9_t(logbloom_bytes);
            m_transactions_root = _r[field = 4].toHash<evm_common::xh256_t>();
            m_receipts_root = _r[field = 5].toHash<evm_common::xh256_t>();
            m_state_root = _r[field = 6].toHash<evm_common::xh256_t>();
            m_extra_data = _r[field = 7].toBytes();
        } else {
            m_format = ETH_HEADER_fORMAT_SIMPLE;
        }
    }
    catch (...)
    {
        xwarn("xeth_header_t::decodeRLP invalid,field=%d,%s", field, top::to_hex(_r[field].toString()).c_str());
        ec = common::error::xerrc_t::invalid_rlp_stream;
    }
}

//============= xeth_block_t ===============
// xeth_block_t::xeth_block_t(const xeth_receipts_t & receipts)
// : m_receipts(receipts) {
// }

// bool xeth_block_t::build_block() {
//     if (m_receipts.empty()) {
//         xassert(false);
//         return false;
//     }

//     evm_common::h256 _receipts_root = xeth_build_t::build_receipts_root(m_receipts);
//     m_header.set_receipts_root(_receipts_root);

//     evm_common::LogBloom _block_logbloom = xeth_build_t::build_block_logsbloom(m_receipts);
//     m_header.set_logBloom(_block_logbloom);

//     uint64_t block_gaslimit = XGET_ONCHAIN_GOVERNANCE_PARAMETER(block_gas_limit);
//     m_header.set_gaslimit(block_gaslimit);

    

//     return true;
// }

NS_END2
