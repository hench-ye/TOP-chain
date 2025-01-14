// Copyright (c) 2017-2019 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#pragma once

#include <functional>
#include "xtransport/transport_fwd.h"
#include "xkad/proto/kadmlia.pb.h"
#include "xbase/xpacket.h"

namespace top {
namespace transport {
typedef std::function<void(transport::protobuf::RoutingMessage & message, base::xpacket_t & packet)> HandlerProc;
}
namespace wrouter {

void WrouterRegisterMessageHandler(int msg_type, transport::HandlerProc handler_proc);
void WrouterUnregisterMessageHandler(int msg_type);
void WrouterRegisterMessageRequestType(int msg_type, int request_type);
void WrouterUnregisterMessageRequestType(int msg_type);
int WrouterGetRequestType(int msg_type);

}  // namespace wrouter

}  // namespace top
