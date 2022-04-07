use crate::io::StorageIntermediate;
use engine_types::types::Address;
use engine_types::H256;
pub struct RegisterIndex(u64);

impl StorageIntermediate for RegisterIndex {
    fn len(&self) -> usize {
        unsafe {
            let result = exports::evm_register_len(self.0);
            // By convention, an unused register might return a length of U64::MAX
            if result < u64::MAX {
                result as usize
            } else {
                0
            }
        }
    }

    fn is_empty(&self) -> bool {
        self.len() == 0
    }

    fn copy_to_slice(&self, buffer: &mut [u8]) {
        unsafe { exports::evm_read_register(self.0, buffer.as_ptr() as u64) }
    }
}

#[derive(Copy, Clone, Default)]
pub struct Runtime;

impl Runtime {
    const READ_STORAGE_REGISTER_ID: RegisterIndex = RegisterIndex(0);
    const INPUT_REGISTER_ID: RegisterIndex = RegisterIndex(1);
    const WRITE_REGISTER_ID: RegisterIndex = RegisterIndex(2);
    const EVICT_REGISTER_ID: RegisterIndex = RegisterIndex(3);
    const ENV_REGISTER_ID: RegisterIndex = RegisterIndex(4);
}

impl crate::io::IO for Runtime {
    type StorageValue = RegisterIndex;

    fn read_input(&self) -> Self::StorageValue {
        unsafe {
            exports::evm_input(Runtime::INPUT_REGISTER_ID.0);
        }
        Runtime::INPUT_REGISTER_ID
    }

    fn return_output(&mut self, value: &[u8]) {
        unsafe {
            exports::evm_value_return(value.len() as u64, value.as_ptr() as u64);
        }
    }

    fn read_storage(&self, key: &[u8]) -> Option<Self::StorageValue> {
        unsafe {
            if exports::evm_storage_read(
                key.len() as u64,
                key.as_ptr() as u64,
                Runtime::READ_STORAGE_REGISTER_ID.0,
            ) == 1
            {
                Some(Runtime::READ_STORAGE_REGISTER_ID)
            } else {
                None
            }
        }
    }

    #[allow(unused_variables)]
    fn storage_has_key(&self, key: &[u8]) -> bool {
        unimplemented!()
        // unsafe { exports::evm_storage_has_key(key.len() as _, key.as_ptr() as _) == 1 }
    }

    fn write_storage(&mut self, key: &[u8], value: &[u8]) -> Option<Self::StorageValue> {
        unsafe {
            if exports::evm_storage_write(
                key.len() as u64,
                key.as_ptr() as u64,
                value.len() as u64,
                value.as_ptr() as u64,
                Runtime::WRITE_REGISTER_ID.0,
            ) == 1
            {
                Some(Runtime::WRITE_REGISTER_ID)
            } else {
                None
            }
        }
    }

    fn remove_storage(&mut self, key: &[u8]) -> Option<Self::StorageValue> {
        unsafe {
            if exports::evm_storage_remove(
                key.len() as _,
                key.as_ptr() as _,
                Runtime::EVICT_REGISTER_ID.0,
            ) == 1
            {
                Some(Runtime::EVICT_REGISTER_ID)
            } else {
                None
            }
        }
    }
}

impl crate::env::Env for Runtime {
    fn sender_address(&self) -> engine_types::types::Address {
        unsafe {
            exports::evm_sender_address(Self::ENV_REGISTER_ID.0);
        }
        let bytes = Self::ENV_REGISTER_ID.to_vec();
        match Address::try_from(bytes.as_slice()) {
            Ok(address) => address,
            Err(_) => unreachable!(),
        }
    }

    fn block_height(&self) -> u64 {
        unreachable!()
        // unsafe { exports::evm_block_index() }
    }

    fn block_timestamp(&self) -> crate::env::Timestamp {
        unreachable!()
        // let ns = unsafe { exports::evm_block_timestamp() };
        // crate::env::Timestamp::new(ns)
    }

    fn random_seed(&self) -> engine_types::H256 {
        unsafe {
            exports::evm_random_seed(0);
            let bytes = H256::zero();
            exports::evm_read_register(0, bytes.0.as_ptr() as *const u64 as u64);
            bytes
        }
    }
}

pub(crate) mod exports {
    #[allow(unused)]
    extern "C" {
        // Register
        pub(crate) fn evm_read_register(register_id: u64, ptr: u64);
        pub(crate) fn evm_register_len(register_id: u64) -> u64;

        pub(crate) fn evm_sender_address(register_id: u64);
        pub(crate) fn evm_input(register_id: u64);

        // Math
        pub(crate) fn evm_random_seed(register_id: u64);
        pub(crate) fn evm_sha256(value_len: u64, value_ptr: u64, register_id: u64);
        pub(crate) fn evm_keccak256(value_len: u64, value_ptr: u64, register_id: u64);
        pub(crate) fn evm_ripemd160(value_len: u64, value_ptr: u64, register_id: u64);
        pub(crate) fn evm_ecrecover(
            hash_len: u64,
            hash_ptr: u64,
            sig_len: u64,
            sig_ptr: u64,
            v: u64,
            malleability_flag: u64,
            register_id: u64,
        ) -> u64;

        // Others
        pub(crate) fn evm_value_return(value_len: u64, value_ptr: u64);
        pub(crate) fn evm_log_utf8(len: u64, ptr: u64);

        // Storage
        pub(crate) fn evm_storage_write(
            key_len: u64,
            key_ptr: u64,
            value_len: u64,
            value_ptr: u64,
            register_id: u64,
        ) -> u64;
        pub(crate) fn evm_storage_read(key_len: u64, key_ptr: u64, register_id: u64) -> u64;
        pub(crate) fn evm_storage_remove(key_len: u64, key_ptr: u64, register_id: u64) -> u64;
    }
}
