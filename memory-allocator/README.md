# Memory allocator

The goal of this task was to suggest an implementation of a buffer allocator that can allocate contiguous arrays of memory. The allocator was described in the task description as a ring buffer / circular buffer working in a FIFO fashion.

Since this task was given to me while applying for an Embedded Software Developer position, I would usually implement this using statically allocated arrays for the buffer allocator, but the API requirements given in the task didn't allow for such an implementation, so I ended up using `malloc()`.

I did change some parts of the API, in particular with regards to error handling. Something else that I decided against in the API was to pass the storage data buffer to the allocator:

```
allocator_t* allocator_init(size_t buffer_size,
                            uint8_t min_block_size,
                            uint8_t max_block_size);
```

instead of

```
allocator_t* allocator_init(uint8_t* p_buffer,
                            size_t buffer_size,
                            uint8_t min_block_size,
                            uint8_t max_block_size);
```

It didn't make sense to me to force the users of this "library" to allocate their own buffer when the allocator is perfectly capable of initializing its own buffers by itself. This is more robust than the suggested API in the task description, because it prevents the case where the user passes a buffer size that doesn't match the size of the buffer `p_buffer` points to.

Something else that could be done that I didn't do is adding `ASSERT()`s in the implementation of the public API to prevent the functions from being used with `NULL` pointers, or length zero, or stuff like that.
