#pragma once
#include "assert.h"
#include "xevm_contract_runtime/xevm_storage_face.h"

#include <string>

NS_BEG3(top, contract_runtime, evm)

enum class storage_key_type {
    // Config = 0x0,
    Nonce = 0x1,
    Balance = 0x2,
    Code = 0x3,
    Storage = 0x4,
    Generation = 0x5,

    ALL = 0x6  // show all
};

struct storage_key {
    storage_key_type key_type;
    std::string address;  // todo use T6?...
    std::string extra_key;
};

class xtop_evm_storage_base : public xevm_storage_face_t {
public:
    xtop_evm_storage_base() = default;
    xtop_evm_storage_base(xtop_evm_storage_base const &) = delete;
    xtop_evm_storage_base & operator=(xtop_evm_storage_base const &) = delete;
    xtop_evm_storage_base(xtop_evm_storage_base &&) = default;
    xtop_evm_storage_base & operator=(xtop_evm_storage_base &&) = default;
    ~xtop_evm_storage_base() override = default;

protected:
    // todo might add some utl function here to decode key type.

    /// [1 byte]   -   [1 byte]   - [20 bytes]  - [?? bytes (36 max)]
    //  [version]  - [key perfix] -  [address]  - [extra_key for storage]
    storage_key decode_key_type(xbytes_t const & key) {
        storage_key res;
        assert(key.size() >= 22);
        res.key_type = storage_key_type(key[1]);
        res.address = "T60004";
        res.address.resize(46);  // 'T60004' + [hex;40]
        static constexpr char hex[] = "0123456789abcdef";
        for (std::size_t index = 0; index < 20; ++index) {
            res.address[6 + 2 * index] = hex[key[index + 2] / 16];
            res.address[6 + 2 * index + 1] = hex[key[index + 2] % 16];
        }
        if (key.size() > 22) {
            res.extra_key.resize(2 * (key.size() - 22));  // max 72
            for (std::size_t index = 20; index < key.size() - 2; ++index) {
                res.extra_key[2 * (index - 20)] = hex[key[index + 2] / 16];
                res.extra_key[2 * (index - 20) + 1] = hex[key[index + 2] % 16];
            }
        }
        return res;
    }
};

using xevm_storage_base_t = xtop_evm_storage_base;
NS_END3