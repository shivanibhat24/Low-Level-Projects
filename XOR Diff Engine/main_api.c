#ifndef XOR_DIFF_ENGINE_H
#define XOR_DIFF_ENGINE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

/* =============================================================================
 * XOR DIFF ENGINE - Flash-Saving Swap Using XOR-Delta Patches
 * =============================================================================
 * 
 * This API provides efficient in-place data swapping using XOR operations
 * optimized for flash memory systems where write cycles are costly.
 * 
 * Features:
 * - In-place XOR swapping without temporary storage
 * - Delta patch generation and application
 * - Flash-optimized write patterns
 * - Rollback and versioning support
 * - Memory-efficient operations
 */

/* Return codes */
typedef enum {
    XOR_DIFF_SUCCESS = 0,
    XOR_DIFF_ERROR_NULL_POINTER = -1,
    XOR_DIFF_ERROR_INVALID_SIZE = -2,
    XOR_DIFF_ERROR_MEMORY_ALLOC = -3,
    XOR_DIFF_ERROR_FLASH_WRITE = -4,
    XOR_DIFF_ERROR_CHECKSUM_MISMATCH = -5,
    XOR_DIFF_ERROR_VERSION_MISMATCH = -6,
    XOR_DIFF_ERROR_PATCH_CORRUPT = -7
} xor_diff_result_t;

/* Patch metadata structure */
typedef struct {
    uint32_t version;           /* Patch version */
    uint32_t source_checksum;   /* CRC32 of source data */
    uint32_t target_checksum;   /* CRC32 of target data */
    size_t patch_size;          /* Size of patch data */
    size_t data_size;           /* Size of original data */
    uint8_t compression_level;  /* 0-9, 0=none */
    uint8_t flags;              /* Additional flags */
    uint16_t reserved;          /* Reserved for future use */
} xor_patch_header_t;

/* Patch structure */
typedef struct {
    xor_patch_header_t header;
    uint8_t* patch_data;        /* XOR delta data */
} xor_patch_t;

/* Configuration structure */
typedef struct {
    size_t block_size;          /* Processing block size (default: 4096) */
    bool enable_compression;    /* Enable RLE compression */
    bool enable_checksum;       /* Enable integrity checking */
    bool flash_optimized;       /* Use flash-friendly write patterns */
    uint8_t write_alignment;    /* Flash write alignment (1, 2, 4, 8) */
} xor_diff_config_t;

/* Context structure for stateful operations */
typedef struct xor_diff_context {
    xor_diff_config_t config;
    uint32_t current_version;
    size_t total_operations;
    uint64_t bytes_processed;
    void* internal_state;       /* Internal implementation state */
} xor_diff_context_t;

/* =============================================================================
 * CORE API FUNCTIONS
 * =============================================================================
 */

/* Initialize XOR diff engine context */
xor_diff_result_t xor_diff_init(xor_diff_context_t* ctx, 
                                const xor_diff_config_t* config);

/* Cleanup XOR diff engine context */
void xor_diff_cleanup(xor_diff_context_t* ctx);

/* Create default configuration */
xor_diff_config_t xor_diff_default_config(void);

/* =============================================================================
 * IN-PLACE XOR SWAP OPERATIONS
 * =============================================================================
 */

/* Perform in-place XOR swap between two memory regions */
xor_diff_result_t xor_swap_inplace(void* data_a, void* data_b, size_t size);

/* Perform in-place XOR swap with context (for tracking/logging) */
xor_diff_result_t xor_swap_inplace_ctx(xor_diff_context_t* ctx,
                                       void* data_a, void* data_b, size_t size);

/* Triple XOR swap (classic algorithm) */
xor_diff_result_t xor_swap_triple(void* data_a, void* data_b, size_t size);

/* Block-wise XOR swap for large data */
xor_diff_result_t xor_swap_blocks(xor_diff_context_t* ctx,
                                  void* data_a, void* data_b, size_t size);

/* =============================================================================
 * XOR DELTA PATCH OPERATIONS
 * =============================================================================
 */

/* Generate XOR delta patch between source and target */
xor_diff_result_t xor_diff_create_patch(xor_diff_context_t* ctx,
                                        const void* source_data,
                                        const void* target_data,
                                        size_t data_size,
                                        xor_patch_t* patch);

/* Apply XOR delta patch to source data */
xor_diff_result_t xor_diff_apply_patch(xor_diff_context_t* ctx,
                                       const xor_patch_t* patch,
                                       void* source_data);

/* Apply patch in-place with verification */
xor_diff_result_t xor_diff_apply_patch_safe(xor_diff_context_t* ctx,
                                            const xor_patch_t* patch,
                                            void* source_data,
                                            bool verify_checksum);

/* Generate reverse patch for rollback */
xor_diff_result_t xor_diff_create_reverse_patch(const xor_patch_t* forward_patch,
                                                xor_patch_t* reverse_patch);

/* =============================================================================
 * FLASH-OPTIMIZED OPERATIONS
 * =============================================================================
 */

/* Flash-optimized patch application with minimal writes */
xor_diff_result_t xor_diff_flash_apply(xor_diff_context_t* ctx,
                                       const xor_patch_t* patch,
                                       void* flash_data,
                                       size_t flash_sector_size);

/* Update data in flash using XOR delta with wear leveling */
xor_diff_result_t xor_diff_flash_update(xor_diff_context_t* ctx,
                                        void* flash_addr,
                                        const void* new_data,
                                        size_t data_size,
                                        size_t sector_size);

/* Batch multiple patches for efficient flash writes */
xor_diff_result_t xor_diff_flash_batch_apply(xor_diff_context_t* ctx,
                                             const xor_patch_t* patches,
                                             size_t patch_count,
                                             void* flash_data,
                                             size_t flash_size);

/* =============================================================================
 * PATCH MANAGEMENT
 * =============================================================================
 */

/* Allocate patch structure */
xor_patch_t* xor_patch_alloc(size_t patch_size);

/* Free patch structure */
void xor_patch_free(xor_patch_t* patch);

/* Validate patch integrity */
xor_diff_result_t xor_patch_validate(const xor_patch_t* patch);

/* Compress patch data using RLE */
xor_diff_result_t xor_patch_compress(xor_patch_t* patch);

/* Decompress patch data */
xor_diff_result_t xor_patch_decompress(xor_patch_t* patch);

/* Serialize patch to buffer */
xor_diff_result_t xor_patch_serialize(const xor_patch_t* patch,
                                      uint8_t* buffer,
                                      size_t buffer_size,
                                      size_t* bytes_written);

/* Deserialize patch from buffer */
xor_diff_result_t xor_patch_deserialize(const uint8_t* buffer,
                                        size_t buffer_size,
                                        xor_patch_t* patch);

/* =============================================================================
 * UTILITY FUNCTIONS
 * =============================================================================
 */

/* Calculate CRC32 checksum */
uint32_t xor_diff_crc32(const void* data, size_t size);

/* Compare two data blocks efficiently */
bool xor_diff_data_equal(const void* data_a, const void* data_b, size_t size);

/* Find first difference between two blocks */
size_t xor_diff_find_first_diff(const void* data_a, const void* data_b, size_t size);

/* Get human-readable error string */
const char* xor_diff_error_string(xor_diff_result_t result);

/* Get engine version and build info */
void xor_diff_version_info(uint32_t* major, uint32_t* minor, uint32_t* patch);

/* =============================================================================
 * ADVANCED FEATURES
 * =============================================================================
 */

/* Create incremental patch chain */
xor_diff_result_t xor_diff_create_chain(xor_diff_context_t* ctx,
                                        const void** data_versions,
                                        size_t version_count,
                                        size_t data_size,
                                        xor_patch_t** patch_chain);

/* Apply patch chain in sequence */
xor_diff_result_t xor_diff_apply_chain(xor_diff_context_t* ctx,
                                       const xor_patch_t* patch_chain,
                                       size_t chain_length,
                                       void* data);

/* Optimize patch for specific access patterns */
xor_diff_result_t xor_diff_optimize_patch(xor_patch_t* patch,
                                          const uint8_t* access_pattern);

/* Statistics and profiling */
typedef struct {
    size_t patches_created;
    size_t patches_applied;
    size_t bytes_processed;
    size_t flash_writes_saved;
    double compression_ratio;
    uint64_t processing_time_us;
} xor_diff_stats_t;

/* Get operation statistics */
xor_diff_result_t xor_diff_get_stats(const xor_diff_context_t* ctx,
                                     xor_diff_stats_t* stats);

/* Reset statistics */
void xor_diff_reset_stats(xor_diff_context_t* ctx);

/* =============================================================================
 * CALLBACK DEFINITIONS
 * =============================================================================
 */

/* Flash write callback for custom flash drivers */
typedef xor_diff_result_t (*xor_flash_write_fn)(void* flash_addr,
                                                const void* data,
                                                size_t size,
                                                void* user_data);

/* Flash erase callback */
typedef xor_diff_result_t (*xor_flash_erase_fn)(void* flash_addr,
                                                size_t size,
                                                void* user_data);

/* Progress callback for long operations */
typedef void (*xor_progress_fn)(size_t current, size_t total, void* user_data);

/* Set flash operation callbacks */
xor_diff_result_t xor_diff_set_flash_callbacks(xor_diff_context_t* ctx,
                                               xor_flash_write_fn write_fn,
                                               xor_flash_erase_fn erase_fn,
                                               void* user_data);

/* Set progress callback */
xor_diff_result_t xor_diff_set_progress_callback(xor_diff_context_t* ctx,
                                                 xor_progress_fn progress_fn,
                                                 void* user_data);

/* =============================================================================
 * INLINE HELPER FUNCTIONS (for performance-critical paths)
 * =============================================================================
 */

#ifdef XOR_DIFF_ENABLE_INLINE

/* Fast XOR operation for aligned data */
static inline void xor_diff_fast_xor(uint8_t* dest, const uint8_t* src, size_t size) {
    const uint64_t* src64 = (const uint64_t*)src;
    uint64_t* dest64 = (uint64_t*)dest;
    size_t blocks = size / 8;
    
    for (size_t i = 0; i < blocks; i++) {
        dest64[i] ^= src64[i];
    }
    
    /* Handle remaining bytes */
    size_t remaining = size % 8;
    if (remaining) {
        uint8_t* dest8 = (uint8_t*)(dest64 + blocks);
        const uint8_t* src8 = (const uint8_t*)(src64 + blocks);
        for (size_t i = 0; i < remaining; i++) {
            dest8[i] ^= src8[i];
        }
    }
}

/* Check if memory is aligned */
static inline bool xor_diff_is_aligned(const void* ptr, size_t alignment) {
    return ((uintptr_t)ptr % alignment) == 0;
}

#endif /* XOR_DIFF_ENABLE_INLINE */

/* =============================================================================
 * COMPILE-TIME CONFIGURATION
 * =============================================================================
 */

#ifndef XOR_DIFF_DEFAULT_BLOCK_SIZE
#define XOR_DIFF_DEFAULT_BLOCK_SIZE 4096
#endif

#ifndef XOR_DIFF_MAX_PATCH_SIZE
#define XOR_DIFF_MAX_PATCH_SIZE (1024 * 1024) /* 1MB */
#endif

#ifndef XOR_DIFF_VERSION_MAJOR
#define XOR_DIFF_VERSION_MAJOR 1
#endif

#ifndef XOR_DIFF_VERSION_MINOR
#define XOR_DIFF_VERSION_MINOR 0
#endif

#ifndef XOR_DIFF_VERSION_PATCH
#define XOR_DIFF_VERSION_PATCH 0
#endif

#ifdef __cplusplus
}

/* =============================================================================
 * C++ WRAPPER CLASS (when compiled as C++)
 * =============================================================================
 */

#include <memory>
#include <vector>
#include <stdexcept>

namespace xor_diff {

class XorDiffException : public std::runtime_error {
public:
    explicit XorDiffException(xor_diff_result_t result) 
        : std::runtime_error(xor_diff_error_string(result)), result_(result) {}
    
    xor_diff_result_t result() const { return result_; }
    
private:
    xor_diff_result_t result_;
};

class Patch {
public:
    Patch() : patch_(nullptr) {}
    
    explicit Patch(size_t patch_size) : patch_(xor_patch_alloc(patch_size)) {
        if (!patch_) throw std::bad_alloc();
    }
    
    ~Patch() { 
        if (patch_) xor_patch_free(patch_); 
    }
    
    // Move constructor
    Patch(Patch&& other) noexcept : patch_(other.patch_) {
        other.patch_ = nullptr;
    }
    
    // Move assignment
    Patch& operator=(Patch&& other) noexcept {
        if (this != &other) {
            if (patch_) xor_patch_free(patch_);
            patch_ = other.patch_;
            other.patch_ = nullptr;
        }
        return *this;
    }
    
    // Delete copy operations
    Patch(const Patch&) = delete;
    Patch& operator=(const Patch&) = delete;
    
    xor_patch_t* get() { return patch_; }
    const xor_patch_t* get() const { return patch_; }
    
    void validate() const {
        auto result = xor_patch_validate(patch_);
        if (result != XOR_DIFF_SUCCESS) {
            throw XorDiffException(result);
        }
    }
    
private:
    xor_patch_t* patch_;
};

class XorDiffEngine {
public:
    explicit XorDiffEngine(const xor_diff_config_t& config = xor_diff_default_config()) {
        auto result = xor_diff_init(&ctx_, &config);
        if (result != XOR_DIFF_SUCCESS) {
            throw XorDiffException(result);
        }
    }
    
    ~XorDiffEngine() {
        xor_diff_cleanup(&ctx_);
    }
    
    // Delete copy operations
    XorDiffEngine(const XorDiffEngine&) = delete;
    XorDiffEngine& operator=(const XorDiffEngine&) = delete;
    
    // Move operations
    XorDiffEngine(XorDiffEngine&& other) noexcept : ctx_(other.ctx_) {
        other.ctx_ = {};
    }
    
    void swapInPlace(void* data_a, void* data_b, size_t size) {
        auto result = xor_swap_inplace_ctx(&ctx_, data_a, data_b, size);
        if (result != XOR_DIFF_SUCCESS) {
            throw XorDiffException(result);
        }
    }
    
    Patch createPatch(const void* source, const void* target, size_t size) {
        Patch patch(size);
        auto result = xor_diff_create_patch(&ctx_, source, target, size, patch.get());
        if (result != XOR_DIFF_SUCCESS) {
            throw XorDiffException(result);
        }
        return patch;
    }
    
    void applyPatch(const Patch& patch, void* data) {
        auto result = xor_diff_apply_patch(&ctx_, patch.get(), data);
        if (result != XOR_DIFF_SUCCESS) {
            throw XorDiffException(result);
        }
    }
    
    xor_diff_stats_t getStats() const {
        xor_diff_stats_t stats;
        auto result = xor_diff_get_stats(&ctx_, &stats);
        if (result != XOR_DIFF_SUCCESS) {
            throw XorDiffException(result);
        }
        return stats;
    }
    
    void resetStats() {
        xor_diff_reset_stats(&ctx_);
    }
    
private:
    xor_diff_context_t ctx_;
};

} // namespace xor_diff

#endif /* __cplusplus */

#endif /* XOR_DIFF_ENGINE_H */
