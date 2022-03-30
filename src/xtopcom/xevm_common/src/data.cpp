// Copyright (c) 2018-2021 Telos Foundation & contributors
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "xevm_common/data.h"

namespace top {
namespace evm_common {
namespace rlp {

bytes subData(const bytes & indata, size_t index, size_t length) {
    size_t subLength = length;
    if (index + subLength > indata.size()) {
        subLength = indata.size() - index;
    }
    return data(indata.data() + index, subLength);
}

}  // namespace rlp
}  // namespace evm_common
}  // namespace top