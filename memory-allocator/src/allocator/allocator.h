#ifndef ALLOCATOR_H_
#define ALLOCATOR_H_

#include "stddef.h"
#include "stdint.h"

typedef struct {
    size_t head;
    size_t tail;
    size_t max_capacity;
} allocator_buffer_cb_t;

typedef struct {
    allocator_buffer_cb_t data_cb;
    allocator_buffer_cb_t size_cb;
    uint8_t* p_buffer;
    uint8_t* p_block_sizes;
    uint8_t min_block_size;
    uint8_t max_block_size;
} allocator_t;

typedef enum {
    ALLOCATOR_SUCCESS,
    ALLOCATOR_ERROR_OUT_OF_MEMORY,
    ALLOCATOR_ERROR_NOT_FOUND,
    ALLOCATOR_ERROR_UNSUPPORTED_SIZE,
} allocator_error_t;

/**
 * @brief       Initializes an allocator instance.
 * 
 * @param[in] buffer_size       size of the allocator's buffer
 * @param[in] min_block_size    minimum size of a block in the allocator's buffer
 * @param[in] max_block_size    maximum size of a block in the allocator's buffer
 * 
 * @return allocator_t*         pointer to allocator instance
 */
allocator_t* allocator_init(size_t buffer_size,
                            uint8_t min_block_size,
                            uint8_t max_block_size);

/**
 * @brief       Uninitializes an allocator instance.
 * 
 * @param[in] p_allocator       pointer to allocator instance
 */
void allocator_uninit(allocator_t* p_allocator);

/**
 * @brief       Allocates a block of a given size.
 * 
 * @param[in]  p_allocator      pointer to allocator 
 * @param[in]  block_size       size of the block to allocate
 * @param[out] pp_block         pointer to pointer to allocated block
 * 
 * @return allocator_error_t    - ALLOCATOR_SUCCESS if the block was allocated
 *                              - ALLOCATOR_ERROR_OUT_OF_MEMORY if the allocator buffer is full
 *                              - ALLOCATOR_ERROR_UNSUPPORTED_SIZE if the requested block size is not supported
 */
allocator_error_t allocator_alloc(allocator_t* p_allocator,
                                  size_t block_size,
                                  uint8_t** pp_block);

/**
 * @brief       Peeks at the oldest block allocated.
 * 
 * @param[in]  p_allocator      pointer to allocator
 * @param[out] pp_block         pointer to pointer to data block
 * @param[out] p_block_size     pointer to block size
 * 
 * @return allocator_error_t    - ALLOCATOR_SUCCESS if there was a block to peek at
 *                              - ALLOCATOR_ERROR_NOT_FOUND otherwise
 */
allocator_error_t allocator_peek(allocator_t* p_allocator,
                                 uint8_t** pp_block,
                                 size_t* p_block_size);

/**
 * @brief       Frees the oldest block allocated.
 * 
 * @param[in] p_allocator       pointer to allocator
 * 
 * @return allocator_error_t    - ALLOCATOR_SUCCESS if a block was freed
 *                              - ALLOCATOR_ERROR_NOT_FOUND if there was nothing to free
 */
allocator_error_t allocator_free(allocator_t* p_allocator);

#endif  // ALLOCATOR_H_