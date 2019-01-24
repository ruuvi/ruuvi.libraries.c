/**
 * @file rust_allocator.h
 * @author Otso Jousimaa
 * @date 2019-01-24
 * @brief Functions for Rust library memory operations.
 * @copyright Copyright 2019 Ruuvi Innovations.
 *   This project is released under the BSD-3-Clause License.
 *
 * These functions let you interface Rust libraries on your c code. 
 * Typically you'd compile the library with
 * `/usr/local/gcc-arm-none-eabi-4_9-2015q3/bin/arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mabi=aapcs -Wall -Werror -O3 -g3 -mfloat-abi=hard -mfpu=fpv4-sp-d16 -ffunction-sections -fdata-sections -fno-strict-aliasing -fno-builtin --short-enums -c rust_allocator.c -o rust_allocator.o`
 * and then statically link your program against produced allocator.  
 */
#ifndef RUST_ALLOCATOR_H
#define RUST_ALLOCATOR_H
#include <stdlib.h>

/** @defgroup rust_allocator Rust Allocator
 *  These functions let you interface Rust libraries on your c code. 
 *  @{
 */

/**
 * @brief Allocate memory for Rust function. 
 *
 * A malloc function. Allocates a block of memory. Must be freed manually by unexec_free().
 * @param[in] usize Size of memory block to be allocated. 
 * @return Returns a pointer to allocated memory block, or @c NULL if memory could not be allocated.
 */
void* unexec_malloc(size_t usize);

/**
 * @brief Free memory for Rust function. 
 *
 * A free function. Frees a block of memory previously allocated by unexec_malloc()
 * @param[in] ptr Address of memory to free.
 */
void unexec_free(void* ptr);

/**
 * @brief Reallocation function for Rust function
 *
 * The reallocation is done by either:
 * -# Expanding or contracting the existing area pointed to by @c ptr, if possible. The contents of the area remain unchanged up to the lesser of the new and old sizes. If the area is expanded, the contents of the new part of the array are undefined.
 * -# Allocating a new memory block of size @c new_size bytes, copying memory area with size equal the lesser of the new and the old sizes, and freeing the old block.
 *
 * If there is not enough memory, the old memory block is not freed and null pointer is returned.
 *
 * If ptr is NULL, the behavior is the same as calling @c malloc(new_size).
 *
 * If new_size is zero, the behavior is implementation defined (null pointer may be returned (in which case the old memory block may or may not be freed), or some non-null pointer may be returned that may not be used to access storage).
 * @param[in] old_ptr Address of memory to reallocate.
 * @param[in] new_size Size of new memory block
 */
void* unexec_realloc(void* old_ptr, size_t new_size);

/** @} */ // End of group Rust Allocator
#endif
