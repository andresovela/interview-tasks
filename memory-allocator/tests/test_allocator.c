
#include "allocator.h"
#include "unity.h"

void setUp(void) {
    // Nothing to set up
}

void tearDown(void) {
    // Nothing to clean up
}

void test_allocator_initialization_not_null(void) {
    allocator_t* p_allocator = allocator_init(100, 5, 10);
    TEST_ASSERT(p_allocator != NULL);
    TEST_ASSERT(p_allocator->p_buffer != NULL);
    TEST_ASSERT(p_allocator->p_block_sizes != NULL);
}

void test_allocator_alloc_success(void) {
    allocator_t* p_allocator = allocator_init(100, 5, 10);
    uint8_t* p_block = NULL;
    allocator_error_t result = allocator_alloc(p_allocator, 6, &p_block);

    TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
    TEST_ASSERT(p_block != NULL);
}

void test_allocator_alloc_error_below_min_block_size(void) {
    allocator_t* p_allocator = allocator_init(100, 5, 10);
    uint8_t* p_block = NULL;
    allocator_error_t result = allocator_alloc(p_allocator, 2, &p_block);

    TEST_ASSERT_EQUAL(ALLOCATOR_ERROR_UNSUPPORTED_SIZE, result);
    TEST_ASSERT(p_block == NULL);
}

void test_allocator_alloc_error_above_max_block_size(void) {
    allocator_t* p_allocator = allocator_init(100, 5, 10);
    uint8_t* p_block = NULL;
    allocator_error_t result = allocator_alloc(p_allocator, 20, &p_block);

    TEST_ASSERT_EQUAL(ALLOCATOR_ERROR_UNSUPPORTED_SIZE, result);
    TEST_ASSERT(p_block == NULL);
}

void test_allocator_free_error_on_empty_buffer(void) {
    allocator_t* p_allocator = allocator_init(100, 5, 10);
    allocator_error_t result = allocator_free(p_allocator);
    TEST_ASSERT_EQUAL(ALLOCATOR_ERROR_NOT_FOUND, result);
}

void test_allocator_alloc_full_buffer_one_by_one(void) {
    allocator_t* p_allocator = allocator_init(10, 1, 1);
    uint8_t* p_block;
    allocator_error_t result;

    // Fill and empty the entire buffer 100 times
    for (int cycles = 0; cycles < 100; cycles++) {
        // Allocate 10 blocks to fill the entire buffer
        for (int i = 0; i < 10; i++) {
            p_block = NULL;
            result = allocator_alloc(p_allocator, 1, &p_block);
            TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
            TEST_ASSERT(p_block != NULL);
        }

        // Further allocations should fail
        p_block = NULL;
        result = allocator_alloc(p_allocator, 1, &p_block);
        TEST_ASSERT_EQUAL(ALLOCATOR_ERROR_OUT_OF_MEMORY, result);
        TEST_ASSERT(p_block == NULL);

        // Free those 10 blocks
        for (int i = 0; i < 10; i++) {
            result = allocator_free(p_allocator);
            TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
        }

        // Further calls to free should fail, nothing to free
        result = allocator_free(p_allocator);
        TEST_ASSERT_EQUAL(ALLOCATOR_ERROR_NOT_FOUND, result);
    }
}

void test_allocator_many_allocs(void) {
    allocator_t* p_allocator = allocator_init(100, 5, 10);
    uint8_t* p_block = NULL;
    allocator_error_t result;

    // We created a buffer with size 100, we should be able
    // to allocate 20 blocks of size 5
    for (int i = 0; i < 20; i++) {
        result = allocator_alloc(p_allocator, 5, &p_block);
        TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
        TEST_ASSERT(p_block != NULL);
    }

    // Any further allocations should fail because
    // there shouldn't be any more space in the buffer
    result = allocator_alloc(p_allocator, 5, &p_block);
    TEST_ASSERT_EQUAL(ALLOCATOR_ERROR_OUT_OF_MEMORY, result);
}

void test_allocator_many_allocs_and_frees(void) {
    allocator_t* p_allocator = allocator_init(100, 5, 10);
    uint8_t* p_block = NULL;
    allocator_error_t result;

    // Allocate 10 blocks
    for (int i = 0; i < 10; i++) {
        result = allocator_alloc(p_allocator, 5, &p_block);
        TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
        TEST_ASSERT(p_block != NULL);
    }

    // Free those 10 blocks
    for (int i = 0; i < 10; i++) {
        result = allocator_free(p_allocator);
        TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
    }

    // Another free should not be possible
    result = allocator_free(p_allocator);
    TEST_ASSERT_EQUAL(ALLOCATOR_ERROR_NOT_FOUND, result);

    // The entire buffer should be free now, so we should be able to
    // allocate 20 blocks of 5 now
    for (int i = 0; i < 20; i++) {
        result = allocator_alloc(p_allocator, 5, &p_block);
        TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
        TEST_ASSERT(p_block != NULL);
    }

    // Further allocations should fail because the buffer is full
    result = allocator_alloc(p_allocator, 5, &p_block);
    TEST_ASSERT_EQUAL(ALLOCATOR_ERROR_OUT_OF_MEMORY, result);
}

void test_allocator_allocs_and_frees_different_sizes(void) {
    allocator_t* p_allocator = allocator_init(100, 5, 10);
    uint8_t* p_block = NULL;
    allocator_error_t result;

    // Do several rounds of allocs and frees
    for (int times = 0; times < 10; times++) {
        // Allocate 10 blocks of different sizes
        for (int i = 0; i < 10; i++) {
            size_t block_size = (i / 2) + 5;
            result = allocator_alloc(p_allocator, block_size, &p_block);
            TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
            TEST_ASSERT(p_block != NULL);
        }

        // Free those 10 blocks
        for (int i = 0; i < 10; i++) {
            result = allocator_free(p_allocator);
            TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
        }
    }
}

void test_allocator_peek_error_on_empty_buffer(void) {
    allocator_t* p_allocator = allocator_init(100, 5, 10);
    uint8_t* p_block = NULL;
    size_t block_size = 0;
    allocator_error_t result;

    result = allocator_peek(p_allocator, &p_block, &block_size);
    TEST_ASSERT_EQUAL(ALLOCATOR_ERROR_NOT_FOUND, result);
    TEST_ASSERT(p_block == NULL);
    TEST_ASSERT_EQUAL(block_size, 0);
}

void test_allocator_peek_last_alloc(void) {
    allocator_t* p_allocator = allocator_init(100, 5, 10);
    uint8_t* p_allocated_block = NULL;
    uint8_t* p_peeked_block = NULL;
    allocator_error_t result;

    result = allocator_alloc(p_allocator, 7, &p_allocated_block);
    TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
    TEST_ASSERT(p_allocated_block != NULL);

    size_t block_size = 0;
    result = allocator_peek(p_allocator, &p_peeked_block, &block_size);
    TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
    TEST_ASSERT(p_peeked_block != NULL);
    TEST_ASSERT_EQUAL(block_size, 7);
}

void test_allocator_check_peeked_data(void) {
    allocator_t* p_allocator = allocator_init(100, 5, 10);
    uint8_t* p_allocated_block = NULL;
    uint8_t* p_peeked_block = NULL;
    allocator_error_t result;
    size_t block_size = 0;

    // Allocate a block of data
    result = allocator_alloc(p_allocator, 8, &p_allocated_block);
    TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
    TEST_ASSERT(p_allocated_block != NULL);

    // Write some data to the block
    for (int i = 0; i < 8; i++) {
        p_allocated_block[i] = i;
    }

    // Allocate a block of data
    result = allocator_alloc(p_allocator, 6, &p_allocated_block);
    TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
    TEST_ASSERT(p_allocated_block != NULL);

    // Write some data to the block
    for (int i = 0; i < 6; i++) {
        p_allocated_block[i] = i * 4;
    }

    // Peek the oldest block in the buffer
    p_peeked_block = NULL;
    result = allocator_peek(p_allocator, &p_peeked_block, &block_size);
    TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
    TEST_ASSERT(p_peeked_block != NULL);

    // The block should have size 8 and it should have the same data we wrote before
    TEST_ASSERT_EQUAL(8, block_size);
    for (int i = 0; i < 8; i++) {
        TEST_ASSERT_EQUAL(i, p_peeked_block[i]);
    }

    // Free the oldest block
    result = allocator_free(p_allocator);
    TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);

    // Peek at the next block
    p_peeked_block = NULL;
    result = allocator_peek(p_allocator, &p_peeked_block, &block_size);
    TEST_ASSERT_EQUAL(ALLOCATOR_SUCCESS, result);
    TEST_ASSERT(p_peeked_block != NULL);

    // The block should have size 6 and it should have the same data we wrote before
    TEST_ASSERT_EQUAL(6, block_size);
    for (int i = 0; i < 6; i++) {
        TEST_ASSERT_EQUAL(i * 4, p_peeked_block[i]);
    }
}
