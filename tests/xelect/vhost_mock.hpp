#include <map>
#include "xvnetwork/xvhost_face.h"
#include "xbase/xobject.h"
#include "xbasic/xrunnable.h"
#include "xcommon/xmessage_category.h"
#include "xcommon/xsharding_info.h"
#include "xdata/xelect_transaction.hpp"
#include "xnetwork/xmessage_transmission_property.h"
#include "xnetwork/xnode.h"
#include "xvnetwork/xaddress.h"
#include "xvnetwork/xmessage_ready_callback.h"
#include "xvnetwork/xmessage.h"
#include "xvnetwork/xvhost_face_fwd.h"
#include "xvnetwork/xvhost_role.h"
#include "xvnetwork/xvnetwork_construction_data.h"
#include "xvnetwork/xvnetwork_driver_face.h"
#include "xvnetwork/xvnode_fwd.h"
#include "xvnetwork/xvnetwork_driver_face.h"
#include "xvnetwork/tests/xdummy_vhost.h"

using namespace top::vnetwork; //NOLINT
namespace top {
namespace xchain {
class vhost_mock : public vnetwork::tests::xdummy_vhost_t {
 private:
    std::mutex m_mutex;
    std::map<xvnode_address_t, xmessage_ready_callback_t> m_cb_map;

 public:
    virtual
    void
    register_message_ready_notify(xvnode_address_t const & vaddr,
                                  xmessage_ready_callback_t cb) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cb_map.insert({vaddr, cb});
    }

    /**
     * \brief Un-register the message notify for a vitual address.
     * \param vaddr The virtual address that don't need to monitor the message sent to it.
     */
    virtual
    void
    unregister_message_ready_notify(xvnode_address_t const & vaddr) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cb_map.erase(vaddr);
    }

    virtual
    void
    handshake(xvnode_address_t const & src, std::vector<xvnode_address_t> const & seeds) {
        xinfo("hand shake");
    }

    /**
     * \brief Send message.
     * \param message The message to be sent.
     * \param src The send address of the message.
     * \param dst The receive address of the message.
     * \param transmission_property Transmission property.
     */
    virtual
    void
    send(xmessage_t const & message,
         xvnode_address_t const & src,
         xvnode_address_t const & dst,
         network::xtransmission_property_t const & transmission_property = {}) {
        std::lock_guard<std::mutex> lock(m_mutex);
        for (auto item = m_cb_map.begin(); item != m_cb_map.end(); item++) {
            if (dst.node_id() == item->first.node_id()) {
                item->second(src, message, 0);
            }
        }
    }

    /**
     * \brief Send message to all nodes with src & dst specified.
              The receiver itself makes the decision if the dst doesn't match
              its address.
     * \param message The message to be broadcasted.
     * \param src The send address of the message.
     * \param dst The receive address of the message.  Usually the boradcast
     *            message should be sent to the address representing the whole
     *            network.  But the reality is the new joining node which doesn't
     *            have the ability to get the virtual network topology info.  It
     *            needs to broadcast the join message to the REC cluster.  Only
     *            REC nodes will handle this message.
     */
    virtual
    void
    broadcast_to_all(xmessage_t const & message,
                     xvnode_address_t const & src,
                     xvnode_address_t const & dst) {
        for (auto item = m_cb_map.begin(); item != m_cb_map.end(); item++) {
             item->second(src, message, 0);
        }
    }

    /**
     * \brief Broadcast the message to the network that holds the src.
     * \param message The message to be broadcast.
     * \param src The src address of the broadcast message.
     */
    virtual
    void
    broadcast(xmessage_t const & message,
              xvnode_address_t const & src) {
        for (auto item = m_cb_map.begin(); item != m_cb_map.end(); item++) {
             item->second(src, message, 0);
        }
    }

    virtual
    void
    forward_broadcast_message(xmessage_t const & message,
                              xvnode_address_t const & src,
                              xvnode_address_t const & dst){
        for (auto item = m_cb_map.begin(); item != m_cb_map.end(); item++) {
             item->second(src, message, 0);
        }
    }


    virtual
    std::vector<xvnode_address_t>
    member_of_group(xcluster_address_t const & cluster_address,
                      vnetwork::xversion_t const & version) const {
        std::vector<xvnode_address_t> address;
        for (auto item = m_cb_map.begin(); item != m_cb_map.end(); item++) {
            address.push_back(item->first);
        }
        return address;
    }
};
}
}
