/**
 * @file ruuvi_library_ringbuffer.h
 * @author Otso Jousimaa
 * @date 2019-07-22
 * @brief Ringbuffer with emphasis on interrupt tolerace and speed
 * @copyright Copyright 2019 Ruuvi Innovations.
 *   This project is released under the BSD-3-Clause License.
 *
 * This ringbuffer implementation is optimized for bluetooth GATT connection usage.
 * It has special emphasis on being usable when data is read in interrupt contect - no locking
 * The tradeoff is that the data storage must have size of power of two and 
 * the elemements are treated in chunks with size of power of two.
 */
 /*@{*/

#ifndef  RUUVI_LIBRARY_RINGBUFFER_H
#define  RUUVI_LIBRARY_RINGBUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief atomic flag check and set/clear function
 *
 * Uses whatever mechanism underlying platform provides to check and set
 * or clear flag. Used to implement mutex, check-and-set flag to reserve
 *
 * Generally used like this:
 * \code{.c}
 * if(!buffer->lock(&(buffer->readlock), true))  { return RUUVI_LIBRARY_ERROR_CONCURRENCY; }
 * do_some_critical_stuff();
 * if(!buffer->lock(&(buffer->readlock), false)) { return RUUVI_LIBRARY_ERROR_FATAL; }
 * \endcode
 *
 * It's important to return if lock can't be had rather than busylooping:
 * if the interrupt level which fails to get lock is higher than the call which has the lock
 * program will deadlock in the busyloop. Likewise failure to release the lock will
 * cause deadlock in the next execution, fail immediately.
 *
 * @param[in] flag uint32_t address of bitfield to check
 * @param[in] set true to set flag, false to clear flag.
 * @return    true if operation was successful
 */
typedef bool(*ruuvi_library_atomic_flag)(volatile void* const flag, const bool set);

/* @brief Struct definition for ringbuffer. 
 * 
 * Initialization example:
 * \code{.c}
 * static uint8_t buffer_data[1024}; //!< Must be allocated on stack
 * static ruuvi_interface_atomic_t buffer_wlock = RUUVI_INTERFACE_ATOMIC_INIT;
 * static ruuvi_interface_atomic_t buffer_rlock = RUUVI_INTERFACE_ATOMIC_INIT;
 * static ruuvi_library_ringbuffer_t ringbuf = {.head = 0,
 *                                              .tail = 0,
 *                                              .block_size = 32,
 *                                              .storage_size = sizeof(buffer_data),
 *                                              .index_mask = (sizeof(buffer_data) / 32) - 1,
 *                                              .storage = buffer_data;
 *                                              .lock = ruuvi_interface_atomic_flag 
 *                                              .writelock = &buffer_wlock;
 *                                              .readlock  = &buffer_rlock};
 * \endcode
 */
typedef struct {
  size_t head;               //!< Ringbuffer head index
  size_t tail;               //!< Ringbuffer tail index
  const size_t block_size;   //!< Block size of elements, must be power of two
  const size_t storage_size; //!< Size of storage element, must be power of two. 
  const size_t index_mask;   //!< Bitmask of indexes. Must be (storege_size / block_size) -1
  void* const storage;       //!< Pointer to storage.
  /** @brief Function pointer to lock ringbuffer. Can be a dummy implementation which only 
   *         returns true if buffer is used in single-thread environment without interrupts. 
   */
  const ruuvi_library_atomic_flag lock;
  volatile void* const writelock;     //!< Memory address for flag locking write function
  volatile void* const readlock;      //!< Memory address for flag locking read function.
}ruuvi_library_ringbuffer_t;

/**
 * @brief Queue data into ringbuffer
 *
 * This function operates on the head of the buffer, and rejects operation if 
 * there is no more room in the buffer. 
 * 
 * @param[in,out] buffer Pointer to ringbuffer to store data into
 * @param[in]     data Data to store
 * @param[in]     data_length length of data, at most @ref block_size. 
 * @return        RUUVI_LIBRARY_SUCCESS if data was queued
 * @return        RUUVI_LIBRARY_ERROR_NO_MEM if the ringbuffer was full
 * @warning       This function has no input checking. 
 */
ruuvi_library_status_t ruuvi_library_ringbuffer_queue(ruuvi_library_ringbuffer_t* const buffer, 
                                                      const void* const data,
                                                      const size_t data_length);

/**
 * @brief Dequeue data from ringbuffer
 *
 * This function operates on the tail of the buffer, and rejects operation if 
 * there is no more elements in the buffer. Returns pointer to stored data,
 * does not copy it. The stored data can be overwritten by after function returns,
 * so take a deep copy of data if required. You can also implement a peek function to 
 * copy the data before dequeuing
 * 
 * @param[in,out] buffer Pointer to ringbuffer to store data into
 * @param[out]    data Pointer to data, will be assigned at the start of the stored object
 * @return        RUUVI_LIBRARY_SUCCESS if data was queued
 * @return        RUUVI_LIBRARY_ERROR_NO_MEM if the ringbuffer was full
 * @warning       This function has no input checking
 * @warning       Data returned by this function can be overwritten, take a deep copy if required. 
 */
ruuvi_library_status_t ruuvi_library_ringbuffer_dequeue(ruuvi_library_ringbuffer_t* const buffer, 
                                                        const void *data);

/*
 * return true if rinbuffer is full
 */
bool ruuvi_library_ringbuffer_full(const ruuvi_library_ringbuffer_t* const buffer);

/*
 * return true if rinbuffer is empty
 */
bool ruuvi_library_ringbuffer_empty(const ruuvi_library_ringbuffer_t* const buffer);

#endif

 /*@}*/