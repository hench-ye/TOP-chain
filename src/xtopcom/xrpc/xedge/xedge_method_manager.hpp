// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once
#include "xbase/xcontext.h"
#include "xedge_local_method.hpp"
#include "xedge_rpc_handler.h"
#include "xmetrics/xmetrics.h"
#include "xrpc/xcluster/xcluster_query_manager.h"
#include "xrpc/xerror/xrpc_error.h"
#include "xrpc/xjson_proc.h"
#include "xrpc/xrpc_define.h"
#include "xrpc/xrpc_method.h"
#include "xrpc/xuint_format.h"
#include "xstore/xstore_face.h"
#include "xverifier/xwhitelist_verifier.h"
#include "xvnetwork/xvhost_face.h"
#include "xpbase/rlp/RLP.h"
#include "xpbase/rlp/Transaction.h"

NS_BEG2(top, xrpc)
using base::xcontext_t;

using tx_method_handler = std::function<void(const std::string& raw_tx, xjson_proc_t & json_proc, const std::string & ip)>;

#define EDGE_REGISTER_V1_ACTION(T, func_name)                                                                                                                                      \
    m_edge_tx_method_map.emplace(                                                                                                                                                  \
        pair<pair<string, string>, tx_method_handler>{pair<string, string>{"1.0", #func_name}, std::bind(&xedge_method_base<T>::func_name##_method, this, _1, _2, _3)})

template <class T>
class xedge_method_base {
public:
    typedef typename T::conn_type conn_type;
    xedge_method_base(shared_ptr<xrpc_edge_vhost> edge_vhost,
                      common::xip2_t xip2,
                      shared_ptr<asio::io_service> & ioc,
                      bool archive_flag = false,
                      observer_ptr<store::xstore_face_t> store = nullptr,
                      observer_ptr<base::xvblockstore_t> block_store = nullptr,
                      observer_ptr<elect::ElectMain> elect_main = nullptr,
                      observer_ptr<election::cache::xdata_accessor_face_t> const & election_cache_data_accessor = nullptr);
    virtual ~xedge_method_base() {
    }
    void do_method(shared_ptr<conn_type> & response, xjson_proc_t & json_proc, const std::string & ip);
    void sendTransaction_method(const std::string& tx, xjson_proc_t & json_proc, const std::string & ip);
    void sendTransactionV2_method(const std::string& tx, xjson_proc_t & json_proc, const std::string & ip);
    void forward_method(shared_ptr<conn_type> & response, xjson_proc_t & json_proc);
    shared_ptr<xrpc_msg_request_t> generate_request(const xvnode_address_t & source_address, const uint64_t uuid, const string account, xjson_proc_t & json_proc);
    virtual void write_response(shared_ptr<conn_type> & response, const string & content) = 0;
    T * get_edge_handler() {
        return m_edge_handler_ptr.get();
    }
    virtual enum_xrpc_type type() = 0;
    void reset_edge_method(shared_ptr<xrpc_edge_vhost> edge_vhost, common::xip2_t xip2) {
        m_edge_handler_ptr->reset_edge_handler(edge_vhost);
        m_edge_local_method_ptr->reset_edge_local_method(xip2);
    }
private:
    int process_trust_data(shared_ptr<conn_type> & response, xjson_proc_t & json_proc, const std::string & ip);
    int process_h5_data(shared_ptr<conn_type> & response, xjson_proc_t & json_proc, const std::string & ip);
protected:
    unique_ptr<T> m_edge_handler_ptr;
    unordered_map<pair<string, string>, tx_method_handler> m_edge_tx_method_map;
    unique_ptr<xedge_local_method<T>> m_edge_local_method_ptr;
    std::shared_ptr<xcluster_query_manager> m_cluster_query_mgr;
    bool m_archive_flag{false};  // for local query
};

class xedge_http_method : public xedge_method_base<xedge_http_handler> {
public:
    xedge_http_method(shared_ptr<xrpc_edge_vhost> edge_vhost,
                      common::xip2_t xip2,
                      shared_ptr<asio::io_service> & ioc,
                      bool archive_flag,
                      observer_ptr<store::xstore_face_t> store,
                      observer_ptr<base::xvblockstore_t> block_store,
                      observer_ptr<elect::ElectMain> elect_main,
                      observer_ptr<election::cache::xdata_accessor_face_t> const & election_cache_data_accessor)
      : xedge_method_base<xedge_http_handler>(edge_vhost, xip2, ioc, archive_flag, store, block_store, elect_main, election_cache_data_accessor) {
    }
    void write_response(shared_ptr<conn_type> & response, const string & content) override {
        response->write(content);
    }
    enum_xrpc_type type() override {
        return enum_xrpc_type::enum_xrpc_http_type;
    }
};

class xedge_ws_method : public xedge_method_base<xedge_ws_handler> {
public:
    xedge_ws_method(shared_ptr<xrpc_edge_vhost> edge_vhost,
                    common::xip2_t xip2,
                    shared_ptr<asio::io_service> & ioc,
                    bool archive_flag,
                    observer_ptr<store::xstore_face_t> store,
                    observer_ptr<base::xvblockstore_t> block_store,
                    observer_ptr<elect::ElectMain> elect_main,
                    observer_ptr<election::cache::xdata_accessor_face_t> const & election_cache_data_accessor)
      : xedge_method_base<xedge_ws_handler>(edge_vhost, xip2, ioc, archive_flag, store, block_store, elect_main, election_cache_data_accessor) {
    }
    void write_response(shared_ptr<conn_type> & response, const string & content) override {
        response->send(content, [](const error_code & ec) {
            if (ec) {
                xkinfo_rpc("Server: Error sending message. Error: %d, error message:%s", ec.value(), ec.message().c_str());
            }
        });
    }
    enum_xrpc_type type() override {
        return enum_xrpc_type::enum_xrpc_ws_type;
    }
};

template <class T>
xedge_method_base<T>::xedge_method_base(shared_ptr<xrpc_edge_vhost> edge_vhost,
                                        common::xip2_t xip2,
                                        shared_ptr<asio::io_service> & ioc,
                                        bool archive_flag,
                                        observer_ptr<store::xstore_face_t> store,
                                        observer_ptr<base::xvblockstore_t> block_store,
                                        observer_ptr<elect::ElectMain> elect_main,
                                        observer_ptr<election::cache::xdata_accessor_face_t> const & election_cache_data_accessor)
  : m_edge_local_method_ptr(top::make_unique<xedge_local_method<T>>(elect_main, xip2))
  , m_cluster_query_mgr(std::make_shared<xcluster_query_manager>(store, block_store, nullptr))
  , m_archive_flag(archive_flag)
{
    m_edge_handler_ptr = top::make_unique<T>(edge_vhost, ioc, election_cache_data_accessor);
    m_edge_handler_ptr->init();
    EDGE_REGISTER_V1_ACTION(T, sendTransaction);
    EDGE_REGISTER_V1_ACTION(T, sendTransactionV2);
}
template <class T>
int xedge_method_base<T>::process_h5_data(shared_ptr<conn_type> & response, xjson_proc_t & json_proc, const std::string & ip)
{
    auto & request = json_proc.m_request_json["params"];

    std::vector<std::string> vecTx;
    int i = 0;
    for (auto& tx_it : request)
    {
        std::string tx = tx_it.asString();
        vecTx.push_back(top::HexDecode(tx));
        // xinfo_rpc("trust data: %s", tx.c_str());
    }
    if (vecTx.size() < 2) {
        xinfo_rpc("recv h5 data error.");
        return 1;
    }

    utl::xkeyaddress_t key_address(json_proc.m_request_json["target_account_addr"].asString());
    utl::xecdsasig_t signature_obj((uint8_t *)vecTx[1].c_str());
    uint256_t hash = utl::xsha2_256_t::digest((const char*)vecTx[0].data(), vecTx[0].size());
    if (key_address.verify_signature(signature_obj, hash) == false)
    {
        xinfo_rpc("h5 data sign error.");
        return 1;
    }

    xjson_proc_t json_proc_temp;
    json_proc_temp.m_enable_sign = 0;
    json_proc_temp.parse_json(vecTx[0]);
    xfilter_manager filter;
    filter.filter(json_proc_temp);

    json_proc_temp.m_tx_type = enum_xrpc_tx_type::enum_xrpc_tx_type;
    sendTransaction_method("", json_proc_temp, ip);
    assert(json_proc_temp.m_account_set.size());
    forward_method(response, json_proc_temp);
    return 0;
}
template <class T>
int xedge_method_base<T>::process_trust_data(shared_ptr<conn_type> & response, xjson_proc_t & json_proc, const std::string & ip)
{
    auto & request = json_proc.m_request_json["params"];

    std::vector<std::string> vecTx;
    int i = 0;
    for (auto& tx_it : request)
    {
        std::string tx = tx_it.asString();
        vecTx.push_back(top::HexDecode(tx));
        // xinfo_rpc("trust data: %s", tx.c_str());
    }

    const string & version = json_proc.m_request_json["version"].asString();
    const string & method = json_proc.m_request_json["method"].asString();
    auto version_method = pair<string, string>(version, method);

    auto iter = m_edge_tx_method_map.find(version_method);
    if (iter == m_edge_tx_method_map.end())
        return 1;

    for (int i = 0; i < vecTx.size(); i++) {
        xjson_proc_t json_proc_temp = json_proc;
        json_proc_temp.m_tx_type = enum_xrpc_tx_type::enum_xrpc_tx_type;
        iter->second(vecTx[i], json_proc_temp, ip);
        assert(json_proc_temp.m_account_set.size());
        forward_method(response, json_proc_temp);
    }
    return 0;
}

template <class T>
void xedge_method_base<T>::do_method(shared_ptr<conn_type> & response, xjson_proc_t & json_proc, const std::string & ip) {
    // set account
    const string & version = json_proc.m_request_json["version"].asString();
    const string & method = json_proc.m_request_json["method"].asString();
    if (method == "sendTransactionV2")  // support multi transactions
    {
        process_trust_data(response, json_proc, ip);
        return;
    } else if (method == "sendTransactionV3")  // process h5 data
    {
        process_h5_data(response, json_proc, ip);
        return;
    }
    auto version_method = pair<string, string>(version, method);
    if (m_edge_local_method_ptr->do_local_method(version_method, json_proc)) {
        write_response(response, json_proc.get_response());
        return;
    }
    auto iter = m_edge_tx_method_map.find(version_method);
    if (iter != m_edge_tx_method_map.end()) {
        json_proc.m_tx_type = enum_xrpc_tx_type::enum_xrpc_tx_type;
        iter->second("", json_proc, ip);
    } else {
        if (m_archive_flag) {
            xdbg("local arc query method: %s", method.c_str());
            m_cluster_query_mgr->call_method(json_proc);
            json_proc.m_response_json[RPC_ERRNO] = RPC_OK_CODE;
            json_proc.m_response_json[RPC_ERRMSG] = RPC_OK_MSG;
            write_response(response, json_proc.get_response());
            return;
        } else {
            json_proc.m_tx_type = enum_xrpc_tx_type::enum_xrpc_query_type;
            json_proc.m_account_set.emplace(json_proc.m_request_json["params"]["account_addr"].asString());
        }
    }
    assert(json_proc.m_account_set.size());
    forward_method(response, json_proc);
}
template <class T>
void xedge_method_base<T>::sendTransactionV2_method(const std::string& raw_tx, xjson_proc_t & json_proc, const std::string & ip) {
    json_proc.m_tx_ptr = make_object_ptr<data::xtransaction_t>();
    auto & tx = json_proc.m_tx_ptr;

    Ethereum::RLP::DecodedItem decoded = Ethereum::RLP::decode(::data(raw_tx));

    std::vector<std::string> vecData;
    for (int i = 0; i < (int)decoded.decoded.size(); i++)
    {
        std::string str(decoded.decoded[i].begin(), decoded.decoded[i].end());
        vecData.push_back(str);
        xinfo_rpc("transaction data%d: %s", i, HexEncode(str).c_str());
    }
    if (decoded.decoded.size() < Transaction::TRANSACTION_SIZE)
        throw xrpc_error{enum_xrpc_error_code::rpc_param_param_lack, "transaction missing params"};
    Transaction transaction(::data(vecData[0]), ::data(vecData[1]), to_uint16(vecData[2]), to_uint64(vecData[3]), to_uint32(vecData[4]), to_uint32(vecData[5]), 
        to_uint16(vecData[6]), to_uint16(vecData[7]), to_uint64(vecData[8]), ::data(vecData[9]), ::data(vecData[10]));
    xinfo_rpc("transaction: %d,%d,%d,%d,%d,%d,%d", transaction.tx_type, transaction.amount, transaction.extra, transaction.tx_deposit,
        transaction.source_action_type, transaction.target_action_type, transaction.last_tx_nonce);
    
    tx->set_tx_version(2);
    tx->set_deposit(transaction.tx_deposit);
    tx->set_to_ledger_id(0);
    tx->set_from_ledger_id(0);
    tx->set_tx_type(transaction.tx_type);
    tx->set_tx_len(0);
    tx->set_expire_duration(100);
    tx->set_fire_timestamp(base::xtime_utl::gmttime());
    tx->set_random_nonce(0);
    tx->set_premium_price(0);
    tx->set_last_nonce(transaction.last_tx_nonce);
    std::string last_trans_hash = data_to_string(transaction.m_last_tx_hash);
    tx->set_last_hash(hex_to_uint64(last_trans_hash));
    tx->set_challenge_proof("");
    tx->set_memo(data_to_string(transaction.note));

    auto & source_action = tx->get_source_action();
    source_action.set_action_hash(0);
    source_action.set_action_type(static_cast<enum_xaction_type>(transaction.source_action_type));
    source_action.set_action_size(0);
    source_action.set_account_addr(data_to_string(transaction.from));
    source_action.set_action_name("");
    uint64_t amount = transaction.amount;
    if (transaction.tx_type == xtransaction_type_transfer)
    {
        string source_param(sizeof(uint32_t), 0);
        source_param += std::string((char *)&amount, sizeof(amount));
        source_action.set_action_param(std::move(source_param));
    }
    source_action.set_action_ext("");
    source_action.set_action_authorization("");

    auto & target_action = tx->get_target_action();
    target_action.set_action_hash(0);
    target_action.set_action_type(static_cast<enum_xaction_type>(transaction.target_action_type));
    target_action.set_action_size(0);
    target_action.set_account_addr(data_to_string(transaction.to));
    target_action.set_action_name("");

    string target_param;
    if (transaction.tx_type == xtransaction_type_transfer ||
        transaction.tx_type == xtransaction_type_pledge_token_tgas ||
        transaction.tx_type == xtransaction_type_redeem_token_tgas)
    {
        target_param = std::string(sizeof(uint32_t), 0);
        target_param += std::string((char*)&amount, sizeof(amount));
    } else if (transaction.tx_type == xtransaction_type_pledge_token_vote)
    {
        uint16_t lock_time = transaction.extra;
        target_param += std::string((char*)&amount, sizeof(amount));
        target_param += std::string((char*)&lock_time, sizeof(lock_time));
    } else if (transaction.tx_type == xtransaction_type_redeem_token_vote) {
        target_param += std::string((char*)&amount, sizeof(amount));
    }
    target_action.set_action_param(std::move(target_param));
    target_action.set_action_ext("");
    target_action.set_action_authorization("");

    tx->set_ext("");

    Data encoded = transaction.encode();
    xinfo_rpc("encoded: %s", top::HexEncode(std::string((char*)encoded.data(), encoded.size())).c_str());
    uint256_t hash = utl::xsha2_256_t::digest((const char*)encoded.data(), encoded.size());
    xinfo_rpc("hash: %s", top::HexEncode(std::string((char*)hash.data(), hash.size())).c_str());
    tx->set_digest(std::move(hash));
    tx->set_authorization(std::move(vecData[Transaction::TRANSACTION_SIZE - 1]));

    if (!(target_action.get_account_addr() == sys_contract_rec_standby_pool_addr && target_action.get_action_name() == "nodeJoinNetwork"))
    {
        if (!tx->sign_check())
        {
            throw xrpc_error{enum_xrpc_error_code::rpc_param_param_error, "transaction sign error"};
        }
    }

    tx->set_len();

    const auto & tx_hash = uint_to_str(json_proc.m_tx_ptr->digest().data(), json_proc.m_tx_ptr->digest().size());
    // xinfo_rpc("send tx hash:%s", tx_hash.c_str());
    XMETRICS_PACKET_INFO("rpc_tx_ip",
                         "tx_hash", tx_hash,
                         "client_ip", ip);
    json_proc.m_account_set.emplace(vecData[0]);
    json_proc.m_response_json["tx_hash"] = tx_hash;
    json_proc.m_response_json["tx_size"] = tx->get_tx_len();
    return;
}
template <class T>
void xedge_method_base<T>::sendTransaction_method(const std::string& raw_tx, xjson_proc_t & json_proc, const std::string & ip) {
    json_proc.m_tx_ptr = make_object_ptr<data::xtransaction_t>();
    auto & tx = json_proc.m_tx_ptr;
    auto & request = json_proc.m_request_json["params"];

    tx->set_tx_version(request["tx_structure_version"].asUInt());
    tx->set_deposit(request["tx_deposit"].asUInt());
    tx->set_to_ledger_id(static_cast<uint8_t>(request["to_ledger_id"].asUInt()));
    tx->set_from_ledger_id(static_cast<uint8_t>(request["from_ledger_id"].asUInt()));
    tx->set_tx_type(static_cast<uint16_t>(request["tx_type"].asUInt()));
    tx->set_tx_len(static_cast<uint16_t>(request["tx_len"].asUInt()));
    tx->set_expire_duration(static_cast<uint16_t>(request["tx_expire_duration"].asUInt()));
    tx->set_fire_timestamp(request["send_timestamp"].asUInt64());
    tx->set_random_nonce(request["tx_random_nonce"].asUInt());
    tx->set_premium_price(request["premium_price"].asUInt());
    tx->set_last_nonce(request["last_tx_nonce"].asUInt64());
    std::string last_trans_hash = request["last_tx_hash"].asString();
    tx->set_last_hash(hex_to_uint64(last_trans_hash));
    tx->set_challenge_proof(request["challenge_proof"].asString());
    tx->set_memo(request["note"].asString());

    const auto & from = request["sender_action"]["tx_sender_account_addr"].asString();
    const auto & to = request["receiver_action"]["tx_receiver_account_addr"].asString();
    auto & source_action = tx->get_source_action();
    source_action.set_action_hash(request["sender_action"]["action_hash"].asUInt());
    source_action.set_action_type(static_cast<enum_xaction_type>(request["sender_action"]["action_type"].asUInt()));
    source_action.set_action_size(static_cast<uint16_t>(request["sender_action"]["action_size"].asUInt()));
    source_action.set_account_addr(from);
    source_action.set_action_name(request["sender_action"]["action_name"].asString());
    auto source_param_vec = hex_to_uint(request["sender_action"]["action_param"].asString());
    string source_param((char *)source_param_vec.data(), source_param_vec.size());
    source_action.set_action_param(std::move(source_param));
    auto source_ext_vec = hex_to_uint(request["sender_action"]["action_ext"].asString());
    string source_ext((char *)source_ext_vec.data(), source_ext_vec.size());
    source_action.set_action_ext(std::move(source_ext));

    source_action.set_action_authorization(request["sender_action"]["action_authorization"].asString());

    auto & target_action = tx->get_target_action();
    target_action.set_action_hash(request["receiver_action"]["action_hash"].asUInt());
    target_action.set_action_type(static_cast<enum_xaction_type>(request["receiver_action"]["action_type"].asUInt()));
    target_action.set_action_size(static_cast<uint16_t>(request["receiver_action"]["action_size"].asUInt()));
    target_action.set_account_addr(to);
    target_action.set_action_name(request["receiver_action"]["action_name"].asString());
    auto target_param_vec = hex_to_uint(request["receiver_action"]["action_param"].asString());
    string target_param((char *)target_param_vec.data(), target_param_vec.size());
    target_action.set_action_param(std::move(target_param));
    auto target_ext_vec = hex_to_uint(request["receiver_action"]["action_ext"].asString());
    string target_ext((char *)target_ext_vec.data(), target_ext_vec.size());
    target_action.set_action_ext(std::move(target_ext));

    target_action.set_action_authorization(request["receiver_action"]["action_authorization"].asString());

    auto ext_vec = hex_to_uint(request["ext"].asString());
    string ext((char *)ext_vec.data(), ext_vec.size());
    tx->set_ext(ext);

    tx->set_digest(std::move(hex_to_uint256(request["tx_hash"].asString())));

    if (json_proc.m_enable_sign == 0) {
        tx->set_len();

        const auto &tx_hash = uint_to_str(json_proc.m_tx_ptr->digest().data(), json_proc.m_tx_ptr->digest().size());
        xinfo_rpc("send tx hash:%s", tx_hash.c_str());
        XMETRICS_PACKET_INFO("rpc_tx_ip",
                             "tx_hash", tx_hash,
                             "client_ip", ip);
        json_proc.m_account_set.emplace(from);
        json_proc.m_response_json["tx_hash"] = tx_hash;
        json_proc.m_response_json["tx_size"] = tx->get_tx_len();
        return;
    }
    if (!request.isMember("authorization")) {
        tx->set_digest();
        std::unique_lock<std::mutex> lock(xedge_local_method<T>::m_mutex);
        auto iter = xedge_local_method<T>::m_account_key_map.find(source_action.get_account_addr());
        if (iter == xedge_local_method<T>::m_account_key_map.end()) {
            throw xrpc_error{enum_xrpc_error_code::rpc_param_param_lack, "no private_key find for " + source_action.get_account_addr()};
        }
        utl::xecprikey_t pri_key = iter->second;
        lock.unlock();
        utl::xecdsasig_t signature = pri_key.sign(tx->digest());

        string signature_str((char *)signature.get_compact_signature(), signature.get_compact_signature_size());
        tx->set_authorization(std::move(signature_str));
    } else {
        auto signature = hex_to_uint(request["authorization"].asString());
        string signature_str((char *)signature.data(), signature.size());  // hex_to_uint client send xstream_t data 0xaaaa => string
        tx->set_authorization(std::move(signature_str));
        if (!tx->digest_check()) {
            throw xrpc_error{enum_xrpc_error_code::rpc_param_param_error, "transaction hash error"};
        }
        if (!(target_action.get_account_addr() == sys_contract_rec_standby_pool_addr && target_action.get_action_name() == "nodeJoinNetwork")) {
            if (!tx->sign_check()) {
                throw xrpc_error{enum_xrpc_error_code::rpc_param_param_error, "transaction sign error"};
            }
        }
    }
    tx->set_len();

    const auto & tx_hash = uint_to_str(json_proc.m_tx_ptr->digest().data(), json_proc.m_tx_ptr->digest().size());
    xinfo_rpc("send tx hash:%s", tx_hash.c_str());
    XMETRICS_PACKET_INFO("rpc_tx_ip",
                         "tx_hash", tx_hash,
                         "client_ip", ip);
    json_proc.m_account_set.emplace(from);
    json_proc.m_response_json["tx_hash"] = tx_hash;
    json_proc.m_response_json["tx_size"] = tx->get_tx_len();
}

template <class T>
shared_ptr<xrpc_msg_request_t> xedge_method_base<T>::generate_request(const xvnode_address_t & source_address,
                                                                      const uint64_t uuid,
                                                                      const string account,
                                                                      xjson_proc_t & json_proc) {
    shared_ptr<xrpc_msg_request_t> edge_msg_ptr;
    const string client_id = json_proc.m_request_json[RPC_SEQUENCE_ID].asString();
    if (json_proc.m_tx_type == enum_xrpc_tx_type::enum_xrpc_query_type) {
        edge_msg_ptr = std::make_shared<xrpc_msg_request_t>(type(), json_proc.m_tx_type, source_address, uuid, client_id, account, json_proc.get_request());
    } else if (json_proc.m_tx_type == enum_xrpc_tx_type::enum_xrpc_tx_type) {
        base::xstream_t stream(xcontext_t::instance());
        // json_proc.m_tx_ptr->add_modified_count();
        json_proc.m_tx_ptr->serialize_to(stream);
        edge_msg_ptr =
            std::make_shared<xrpc_msg_request_t>(type(), json_proc.m_tx_type, source_address, uuid, client_id, account, string((const char *)stream.data(), stream.size()));
    } else {
        assert(false);
    }
    return edge_msg_ptr;
}

template <class T>
void xedge_method_base<T>::forward_method(shared_ptr<conn_type> & response, xjson_proc_t & json_proc) {
    do {
        // get shard address
        unordered_set<xvnode_address_t> shard_addr_set;
        std::vector<shared_ptr<xrpc_msg_request_t>> edge_msg_list;
        uint64_t uuid = m_edge_handler_ptr->add_seq_id();
        auto vd = m_edge_handler_ptr->get_rpc_edge_vhost()->get_vnetwork_driver();
        const xvnode_address_t & source_address = vd->address();

        for (const auto & account : json_proc.m_account_set) {
            auto cluster_addr = m_edge_handler_ptr->get_rpc_edge_vhost()->get_router()->sharding_address_from_account(
                common::xaccount_address_t{account}, vd->network_id(), common::xnode_type_t::consensus_validator);
            assert(common::has<common::xnode_type_t::consensus_validator>(cluster_addr.type()) || common::has<common::xnode_type_t::committee>(cluster_addr.type()) ||
                   common::has<common::xnode_type_t::zec>(cluster_addr.type()));
            vnetwork::xvnode_address_t shard_addr{std::move(cluster_addr)};

            if (shard_addr_set.find(shard_addr) == shard_addr_set.end()) {
                shared_ptr<xrpc_msg_request_t> edge_msg_ptr = generate_request(source_address, uuid, account, json_proc);
                edge_msg_list.emplace_back(edge_msg_ptr);
                shard_addr_set.emplace(shard_addr);
            }
        }
        if (edge_msg_list.empty()) {
            throw xrpc_error{enum_xrpc_error_code::rpc_param_param_lack, "msg list is empty"};
        }
        if (nullptr != json_proc.m_tx_ptr) {
            m_edge_handler_ptr->edge_send_msg(edge_msg_list, json_proc.m_tx_ptr->get_digest_hex_str(), json_proc.m_tx_ptr->get_source_addr());
        } else {
            m_edge_handler_ptr->edge_send_msg(edge_msg_list, "", "");
        }
#if (defined ENABLE_RPC_SESSION) && (ENABLE_RPC_SESSION != 0)
        m_edge_handler_ptr->insert_session(edge_msg_list, shard_addr_set, response);
#else
        if (json_proc.m_tx_type == enum_xrpc_tx_type::enum_xrpc_tx_type) {
            json_proc.m_response_json[RPC_ERRNO] = RPC_OK_CODE;
            json_proc.m_response_json[RPC_ERRMSG] = RPC_OK_MSG;
            XMETRICS_COUNTER_INCREMENT("rpc_edge_tx_response", 1);
            write_response(response, json_proc.get_response());
        } else {
            XMETRICS_COUNTER_INCREMENT("rpc_edge_query_response", 1);
            // auditor return query result directly, so clear shard addr set
            shard_addr_set.clear();
            m_edge_handler_ptr->insert_session(edge_msg_list, shard_addr_set, response);
        }
#endif
    } while (0);
}

NS_END2
