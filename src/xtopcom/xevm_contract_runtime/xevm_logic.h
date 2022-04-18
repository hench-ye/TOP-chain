#pragma once
#include "xbasic/xmemory.hpp"
#include "xevm_contract_runtime/xevm_storage_face.h"
#include "xevm_contract_runtime/xevm_type.h"
#include "xevm_runner/evm_logic_face.h"

#include <cstdint>
#include <map>
#include <memory>
#include <string>

NS_BEG3(top, contract_runtime, evm)

class xtop_evm_logic : public top::evm::xevm_logic_face_t {
public:
    xtop_evm_logic(std::shared_ptr<xevm_storage_face_t> storage_ptr, observer_ptr<evm_runtime::xevm_context_t> const & context);
    xtop_evm_logic(xtop_evm_logic const &) = delete;
    xtop_evm_logic & operator=(xtop_evm_logic const &) = delete;
    xtop_evm_logic(xtop_evm_logic &&) = default;
    xtop_evm_logic & operator=(xtop_evm_logic &&) = default;
    ~xtop_evm_logic() override = default;

private:
    std::shared_ptr<xevm_storage_face_t> m_storage_ptr;
    observer_ptr<evm_runtime::xevm_context_t> m_context;
    std::map<uint64_t, xbytes_t> m_registers;
    xbytes_t m_return_data_value;
    std::pair<uint32_t, uint64_t> m_return_error_value;

public:
    // for runtime
    xbytes_t get_return_value() override;
    std::pair<uint32_t, uint64_t> get_return_error() override;

public:
    // interface to evm_import_instance:

    // register:
    void read_register(uint64_t register_id, uint64_t ptr) override;
    uint64_t register_len(uint64_t register_id) override;

    // context:
    void sender_address(uint64_t register_id) override;
    void input(uint64_t register_id) override;

    // math:
    void random_seed(uint64_t register_id) override;
    void sha256(uint64_t value_len, uint64_t value_ptr, uint64_t register_id) override;
    void keccak256(uint64_t value_len, uint64_t value_ptr, uint64_t register_id) override;
    void ripemd160(uint64_t value_len, uint64_t value_ptr, uint64_t register_id) override;

    // others:
    void value_return(uint64_t value_len, uint64_t value_ptr) override;
    void error_return(uint32_t ec,uint64_t used_gas) override;
    void log_utf8(uint64_t len, uint64_t ptr) override;

    // storage:
    uint64_t storage_write(uint64_t key_len, uint64_t key_ptr, uint64_t value_len, uint64_t value_ptr, uint64_t register_id) override;
    uint64_t storage_read(uint64_t key_len, uint64_t key_ptr, uint64_t register_id) override;
    uint64_t storage_remove(uint64_t key_len, uint64_t key_ptr, uint64_t register_id) override;

private:
    // inner api
    std::string get_utf8_string(uint64_t len, uint64_t ptr);
    void internal_write_register(uint64_t register_id, std::vector<uint8_t> const & context_input);
    std::vector<uint8_t> get_vec_from_memory_or_register(uint64_t offset, uint64_t len);
    void memory_set_slice(uint64_t offset, std::vector<uint8_t> buf);
    std::vector<uint8_t> memory_get_vec(uint64_t offset, uint64_t len);
    std::vector<uint8_t> internal_read_register(uint64_t register_id);
};
using xevm_logic_t = xtop_evm_logic;

NS_END3