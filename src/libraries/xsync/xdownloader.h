// Copyright (c) 2017-2018 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <vector>
#include "xsync/xchain_downloader.h"
#include "xsync/xrole_chains_mgr.h"
#include "xsync/xsync_store.h"
#include "xsync/xsync_sender.h"
#include "xmbus/xbase_sync_event_monitor.hpp"
#include "xsync/xsync_ratelimit.h"
#include "xsync/xsync_time_rejecter.h"

NS_BEG2(top, sync)

class xdownloader_t;

class xdownloader_face_t {
public:
    virtual void push_event(const mbus::xevent_ptr_t &e) = 0;
};

class xaccount_timer_t : public top::base::xxtimer_t {
public:
    xaccount_timer_t(std::string vnode_id, base::xcontext_t &_context, int32_t timer_thread_id);
    void set_chain(xchain_downloader_face_ptr_t &chain_downloader);
    void del_chain(const std::string &address);

protected:
    ~xaccount_timer_t() override;

protected:
    bool on_timer_fire(const int32_t thread_id,const int64_t timer_id,const int64_t current_time_ms,const int32_t start_timeout_ms,int32_t & in_out_cur_interval_ms) override;

private:
    std::string m_vnode_id;
    std::vector<xchain_downloader_face_ptr_t> m_chains;
    xsync_time_rejecter_t m_time_rejecter{600};
    //const uint32_t m_max_concurrent_chains{30};
    uint32_t m_current_index_Of_chain{0};
};

class xevent_monitor_t : public mbus::xbase_sync_event_monitor_t {
public:
    xevent_monitor_t(uint32_t idx, observer_ptr<mbus::xmessage_bus_face_t> const &mb, 
        observer_ptr<base::xiothread_t> const & iothread,
        xaccount_timer_t *timer,
        xdownloader_t* downloader);
    bool filter_event(const mbus::xevent_ptr_t& e) override;
    void process_event(const mbus::xevent_ptr_t& e) override;

private:
    uint32_t m_idx{0};
    xaccount_timer_t *m_timer;
    xdownloader_t* m_downloader;
};

class xdownloader_t : public xdownloader_face_t {
public:
    friend class xevent_monitor_t;

    xdownloader_t(std::string vnode_id, xsync_store_face_t *sync_store,
                const observer_ptr<mbus::xmessage_bus_face_t> &mbus,
                const observer_ptr<base::xvcertauth_t> &certauth,
                xrole_chains_mgr_t *role_chains_mgr, xsync_sender_t *sync_sender,
                const std::vector<observer_ptr<base::xiothread_t>> &thread_pool, xsync_ratelimit_face_t *ratelimit);

    virtual ~xdownloader_t();

    void push_event(const mbus::xevent_ptr_t &e) override;

private:
    std::string get_address_by_event(const mbus::xevent_ptr_t &e);
    uint32_t get_idx_by_address(const std::string &address);

    void process_event(uint32_t idx, const mbus::xevent_ptr_t &e, xaccount_timer_t *timer);

    xchain_downloader_face_ptr_t on_add_role(uint32_t idx, const mbus::xevent_ptr_t &e, xaccount_timer_t *timer);
    xchain_downloader_face_ptr_t on_remove_role(uint32_t idx, const mbus::xevent_ptr_t &e, xaccount_timer_t *timer);
    xchain_downloader_face_ptr_t on_response_event(uint32_t idx, const mbus::xevent_ptr_t &e);
    xchain_downloader_face_ptr_t on_behind_event(uint32_t idx, const mbus::xevent_ptr_t &e);
    xchain_downloader_face_ptr_t on_chain_snapshot_response_event(uint32_t idx, const mbus::xevent_ptr_t &e);
private:
    xchain_downloader_face_ptr_t find_chain_downloader(uint32_t idx, const std::string &address);
    xchain_downloader_face_ptr_t create_chain_downloader(uint32_t idx, const std::string &address);
    void remove_chain_downloader(uint32_t idx, const std::string &address);

protected:
    std::string m_vnode_id;
    xsync_store_face_t *m_sync_store{};
    observer_ptr<mbus::xmessage_bus_face_t> m_mbus;
    observer_ptr<base::xvcertauth_t> m_certauth;
    xrole_chains_mgr_t *m_role_chains_mgr;
    xsync_sender_t *m_sync_sender{};
    xsync_ratelimit_face_t *m_ratelimit;

    std::vector<xaccount_timer_t*> m_timer_list;
    std::vector<std::shared_ptr<mbus::xmessage_bus_t>> m_mbus_list;
    std::vector<std::shared_ptr<xevent_monitor_t>> m_monitor_list;
    uint32_t m_thread_count{0};
    std::vector<std::unordered_map<std::string, xchain_downloader_face_ptr_t>> m_vector_chains;
};

NS_END2
