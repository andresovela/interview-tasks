#include "allocator.h"

#include "stdlib.h"

static size_t get_index_after_block(allocator_buffer_cb_t* p_cb, size_t index, uint8_t block_size) {
    // The new index would go beyond the buffer size after inserting the block
    // so the new index needs to wrap-around the buffer
    if (index + block_size > p_cb->max_capacity) {
        return index + block_size - p_cb->max_capacity;
    } else {
        return index + block_size;
    }
}

static size_t get_buffer_utilization(allocator_buffer_cb_t* p_cb) {
    // No wrap-around
    if (p_cb->head >= p_cb->tail) {
        return p_cb->head - p_cb->tail;
    }
    // The head has wrapped around the buffer
    else {
        return p_cb->max_capacity + p_cb->head - p_cb->tail;
    }
}

static size_t get_space_available(allocator_buffer_cb_t* p_cb) {
    return p_cb->max_capacity - get_buffer_utilization(p_cb);
}

static bool is_buffer_empty(allocator_buffer_cb_t* p_cb) {
    return (p_cb->head == p_cb->tail);
}

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
                            uint8_t max_block_size) {
    allocator_t* p_allocator = (allocator_t*)malloc(sizeof(allocator_t));

    // Allocate a buffer of the requested size + 1,
    // because we are using the circular buffer implementation that wastes a slot
    p_allocator->p_buffer = (uint8_t*)malloc(buffer_size + 1);

    p_allocator->min_block_size = min_block_size;
    p_allocator->max_block_size = max_block_size;

    p_allocator->data_cb.head = 0;
    p_allocator->data_cb.tail = 0;
    p_allocator->data_cb.max_capacity = buffer_size;

    p_allocator->size_cb.head = 0;
    p_allocator->size_cb.tail = 0;

    // We need to allocate a buffer in order to store the size of each block that gets allocated
    p_allocator->size_cb.max_capacity = buffer_size / min_block_size;
    p_allocator->p_block_sizes = (uint8_t*)malloc(p_allocator->size_cb.max_capacity);

    return p_allocator;
}

/**
 * @brief       Uninitializes an allocator instance.
 * 
 * @param[in] p_allocator       pointer to allocator instance
 */
void allocator_uninit(allocator_t* p_allocator) {
    free(p_allocator->p_block_sizes);
    free(p_allocator->p_buffer);
    free(p_allocator);
}

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
allocator_error_t allocator_alloc(allocator_t* p_allocator, size_t block_size, uint8_t** pp_block) {
    if ((block_size < p_allocator->min_block_size) ||
        (block_size > p_allocator->max_block_size)) {
        return ALLOCATOR_ERROR_UNSUPPORTED_SIZE;
    }

    if (block_size > get_space_available(&p_allocator->data_cb)) {
        return ALLOCATOR_ERROR_OUT_OF_MEMORY;
    }

    // All sanity checks passed, we can return a pointer to the current head
    // with the certainty that we have the space requested by the user
    *pp_block = &(p_allocator->p_buffer[p_allocator->data_cb.head]);

    // Advance the head by the block size we just "allocated"
    p_allocator->data_cb.head = get_index_after_block(&p_allocator->data_cb, p_allocator->data_cb.head, block_size);

    // Save the block size we just allocated and advance the head of the block size buffer
    p_allocator->p_block_sizes[p_allocator->size_cb.head] = block_size;
    p_allocator->size_cb.head = get_index_after_block(&p_allocator->size_cb, p_allocator->size_cb.head, 1);
}

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
allocator_error_t allocator_peek(allocator_t* p_allocator, uint8_t** pp_block, size_t* p_block_size) {
    if (is_buffer_empty(&p_allocator->data_cb) == true) {
        return ALLOCATOR_ERROR_NOT_FOUND;
    }

    *pp_block = &(p_allocator->p_buffer[p_allocator->data_cb.tail]);
    *p_block_size = p_allocator->p_block_sizes[p_allocator->size_cb.tail];
}

/**
 * @brief       Frees the oldest block allocated.
 * 
 * @param[in] p_allocator       pointer to allocator
 * 
 * @return allocator_error_t    - ALLOCATOR_SUCCESS if a block was freed
 *                              - ALLOCATOR_ERROR_NOT_FOUND if there was nothing to free
 */
allocator_error_t allocator_free(allocator_t* p_allocator) {
    if (is_buffer_empty(&p_allocator->data_cb) == true) {
        return ALLOCATOR_ERROR_NOT_FOUND;
    }

    // Save the block size we are about to free
    size_t freed_block_size = p_allocator->p_block_sizes[p_allocator->size_cb.tail];

    // Advance the tails of both buffers
    p_allocator->size_cb.tail = get_index_after_block(&p_allocator->size_cb, p_allocator->size_cb.tail, 1);
    p_allocator->data_cb.tail = get_index_after_block(&p_allocator->data_cb, p_allocator->data_cb.tail, freed_block_size);
}
