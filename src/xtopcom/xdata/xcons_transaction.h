// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <string>

#include "xvledger/xtxreceipt.h"
#include "xvledger/xvaccount.h"
#include "xdata/xtransaction.h"
#include "xdata/xlightunit_info.h"

namespace top { namespace data {

class xcons_transaction_t : public xbase_dataunit_t<xcons_transaction_t, xdata_type_cons_transaction> {
 public:
    xcons_transaction_t();
    xcons_transaction_t(xtransaction_t* raw_tx);
    // xcons_transaction_t(xtransaction_t* tx, const base::xtx_receipt_ptr_t & receipt);
    xcons_transaction_t(const base::xfull_txreceipt_t & full_txreceipt);
 protected:
    virtual ~xcons_transaction_t();
 private:
    xcons_transaction_t & operator = (const xcons_transaction_t & other);

 protected:
    int32_t do_write(base::xstream_t & stream) override;
    int32_t do_read(base::xstream_t & stream) override;

 public:
    std::string                     dump(bool detail = false) const;
    std::string                     dump_execute_state() const {return m_execute_state.dump();}
    inline xtransaction_t*          get_transaction() const {return m_tx.get();}
    std::string                     get_tx_hash() const {return m_tx->get_digest_str();}
    uint256_t                       get_tx_hash_256() const {return m_tx->digest();}
    bool                            verify_cons_transaction();

    const std::string &     get_source_addr()const {return m_tx->get_source_addr();}
    const std::string &     get_account_addr() const {return is_recv_tx()? m_tx->get_target_addr() : m_tx->get_source_addr();}
    const std::string &     get_target_addr()const {return m_tx->get_target_addr();}
    uint64_t                get_tx_nonce()const {return m_tx->get_tx_nonce();}
    const std::string &     get_receipt_source_account()const;
    const std::string &     get_receipt_target_account()const;
    const xaction_t &       get_source_action()const {return m_tx->get_source_action();}
    const xaction_t &       get_target_action()const {return m_tx->get_target_action();}

    enum_transaction_subtype  get_tx_subtype() const {return (enum_transaction_subtype)m_tx->get_tx_subtype();}
    std::string             get_tx_subtype_str() const {return m_tx->get_tx_subtype_str();}
    std::string             get_tx_dump_key() const {return base::xvtxkey_t::transaction_hash_subtype_to_string(m_tx->get_digest_str(), get_tx_subtype());}
    bool                    is_self_tx() const {return m_tx->get_tx_subtype() == enum_transaction_subtype_self;}
    bool                    is_send_tx() const {return m_tx->get_tx_subtype() == enum_transaction_subtype_send;}
    bool                    is_recv_tx() const {return m_tx->get_tx_subtype() == enum_transaction_subtype_recv;}
    bool                    is_confirm_tx() const {return m_tx->get_tx_subtype() == enum_transaction_subtype_confirm;}
    std::string             get_digest_hex_str() const {return m_tx->get_digest_hex_str();}
    uint32_t                get_last_action_used_tgas() const;
    uint32_t                get_last_action_used_deposit() const;
    uint32_t                get_last_action_send_tx_lock_tgas() const;
    uint32_t                get_last_action_recv_tx_use_send_tx_tgas() const;
    enum_xunit_tx_exec_status   get_last_action_exec_status() const;
    uint64_t                get_last_action_receipt_id() const;
    base::xtable_shortid_t  get_last_action_receipt_id_tableid() const;

 public:
    const xtransaction_exec_state_t & get_tx_execute_state() const {return m_execute_state;}
    void                    set_current_used_disk(uint32_t disk) {m_execute_state.set_used_disk(disk);}
    uint32_t                get_current_used_disk() const {return m_execute_state.get_used_disk();}
    void                    set_current_used_tgas(uint32_t tgas) {m_execute_state.set_used_tgas(tgas);}
    uint32_t                get_current_used_tgas() const {return m_execute_state.get_used_tgas();}
    void                    set_current_used_deposit(uint32_t deposit) {m_execute_state.set_used_deposit(deposit);}
    uint32_t                get_current_used_deposit() const {return m_execute_state.get_used_deposit();}
    void                    set_current_send_tx_lock_tgas(uint32_t tgas) {m_execute_state.set_send_tx_lock_tgas(tgas);}
    uint32_t                get_current_send_tx_lock_tgas() const {return m_execute_state.get_send_tx_lock_tgas();}
    void                    set_current_recv_tx_use_send_tx_tgas(uint32_t tgas) {m_execute_state.set_recv_tx_use_send_tx_tgas(tgas);}
    uint32_t                get_current_recv_tx_use_send_tx_tgas() const {return m_execute_state.get_recv_tx_use_send_tx_tgas();}
    void                    set_current_exec_status(enum_xunit_tx_exec_status status) {m_execute_state.set_tx_exec_status(status);}
    enum_xunit_tx_exec_status   get_current_exec_status() const {return m_execute_state.get_tx_exec_status();}
    uint32_t                get_current_receipt_id() const {return m_execute_state.get_receipt_id();}
    void                    set_current_receipt_id(base::xtable_shortid_t tableid, uint64_t value) {m_execute_state.set_receipt_id(tableid, value);}

    void                    set_unit_height(uint64_t unit_height) {m_unit_height = unit_height;}
    uint64_t                get_unit_height() const noexcept {return m_unit_height;}

    uint64_t                get_receipt_clock() const {return get_prove_cert()->get_clock();}
    uint64_t                get_receipt_gmtime() const {return get_prove_cert()->get_gmtime();}
    bool                    is_receipt_valid() const {return m_receipt->is_valid();}

 public:
    bool                    get_receipt_prove_cert_and_account(const base::xvqcert_t* & cert, std::string & account) const;

 private:
    void                    update_transation();
    uint64_t                get_dump_receipt_id() const;
    const base::xvqcert_t*  get_prove_cert() const {return m_receipt->get_prove_cert();}

 private:
    xtransaction_ptr_t          m_tx{nullptr};
    base::xtx_receipt_ptr_t     m_receipt{nullptr};

 private:  // local member, should not serialize
    xtransaction_exec_state_t    m_execute_state;
    uint64_t                     m_unit_height{0};
};

using xcons_transaction_ptr_t = xobject_ptr_t<xcons_transaction_t>;

}  // namespace data
}  // namespace top
