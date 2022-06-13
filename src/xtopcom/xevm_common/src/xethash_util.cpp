#include "xevm_common/xeth/xethash_util.h"
#include "xevm_common/xeth/xeth_util.h"
#include "xutility/xhash.h"
#include "xevm_common/common_data.h"
#include <boost/numeric/conversion/cast.hpp>
NS_BEG3(top, evm_common, eth)

bool ethash_util::verify(xeth_block_header_t *header) {
    const ethash::epoch_context_ptr context = ethash::create_epoch_context(epoch(header->number()));
    const ethash::hash256 headerHash = toHash256(header->hashWithoutSeal());
    const ethash::hash256 mixHash = toHash256(header->mixDigest());
    // bigint difficulty = bigint(bigint(1) << 256) / header->difficulty();
    // const ethash::hash256 boundray = toHash256((u256)(difficulty));
    uint64_t nonce = std::stoull(header->nonce().hex(), nullptr, 16);
    const ethash::hash256 difficulty = toHash256(u256(header->difficulty()));
    auto ret = ethash::verify_against_difficulty(*context, headerHash, mixHash, nonce, difficulty);
    if (ret != ETHASH_SUCCESS) {
        return false;
    }
    return true;
}

int ethash_util::epoch(bigint height) {
    return static_cast<int>(height / block_number_of_per_epoch);
}

ethash::hash256 ethash_util::toHash256(h256 hash) {
    ethash::hash256 ehash = {};
    std::memcpy(ehash.bytes, hash.data(), 32);
    return ehash;
}

ethash::hash256 ethash_util::toHash256(u256 value) {
    ethash::hash256 ehash = {};
    std::array<byte,32> converted;
    toBigEndian<u256, std::array<byte,32>>(value, converted);
    std::memcpy(ehash.bytes, converted.data(), 32);

    /*
    auto hashValue = utl::xkeccak256_t::digest(v.data(), v.size());
    auto hash = FixedHash<32>(hashValue.data(), h256::ConstructFromPointer);
    std::memcpy(ehash.bytes, hash.data(), 32); */
    return ehash;
}

// uint64_t ethash_util::datasetSize(uint64_t height) {
//     int32_t epoch = (int32_t)(height / block_number_of_per_epoch);
//     if (epoch < max_epoch) {
//         return m_dataset_sizes[epoch];
//     }
//     return calcDatasetSize(epoch);
// }

// uint64_t ethash_util::calcDatasetSize(int32_t epoch ){
//     uint64_t size = dataset_init_bytes + dataset_growth_bytes*(uint64_t)epoch - mix_bytes;
//     uint64_t tmp = size / mix_bytes;
//     // while (!tmp) {
//     //     size -= 2 * mix_bytes;
//     // }
//     return size;
// }

// uint64_t ethash_util::cacheSize(uint64_t height){
//     int32_t epoch = (int32_t)(height / block_number_of_per_epoch);
//     if (epoch < max_epoch) {
//         return m_cache_sizes[epoch];
//     }
//     return calcCacheSize(epoch);
// }

// uint64_t ethash_util::calcCacheSize(int32_t epoch){
//     uint64_t size = dataset_init_bytes + dataset_growth_bytes*(uint64_t)epoch - hash_bytes;
//     uint64_t tmp = size / hash_bytes;
//     // while (!tmp) {
//     //     size -= 2 * mix_bytes;
//     // }
//     return size;
// }
NS_END3