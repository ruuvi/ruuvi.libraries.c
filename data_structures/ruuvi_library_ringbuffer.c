#include "ruuvi_library.h"
#include "ruuvi_library_ringbuffer.h"
#include <string.h>
/**
 * @brief Queue data into ringbuffer
 *
 * This function operates on the head of the buffer, and rejects operation if 
 * there is no more room in the buffer. 
 * 
 * @param[in] buffer Pointer to ringbuffer to store data into
 * @param[in] data Data to store
 * @param[in] data_length length of data, at most @ref block_size. 
 * @return    RUUVI_LIBRARY_SUCCESS if data was queued
 * @return    RUUVI_LIBRARY_ERROR_NO_MEM if the ringbuffer was full
 * @warning   This function has no input checking. 
 */
ruuvi_library_status_t ruuvi_library_ringbuffer_queue(ruuvi_library_ringbuffer_t* const buffer, 
                                                      const void* const data,
                                                      const size_t data_length)
{
  if(ruuvi_library_ringbuffer_full(buffer))  { return RUUVI_LIBRARY_ERROR_NO_MEM; }
  if(!buffer->lock(buffer->writelock, true)) { return RUUVI_LIBRARY_ERROR_CONCURRENCY; }

  memcpy((buffer->storage) + (buffer->head * buffer->block_size), 
         data, data_length);
  buffer->head = ((buffer->head + 1) & buffer->index_mask);
  if(!buffer->lock(buffer->writelock, false)) { return RUUVI_LIBRARY_ERROR_FATAL; }
  return RUUVI_LIBRARY_SUCCESS;
}


/**
 * @brief Dequeue data from ringbuffer
 *
 * This function operates on the tail of the buffer, and rejects operation if 
 * there is no more elements in the buffer. 
 * 
 * @param[in]  buffer Pointer to ringbuffer to store data into
 * @param[out] data Data to retrieve. Returns pointer to data in ringbuffer, a shallow copy.
 * @return     RUUVI_LIBRARY_SUCCESS if data was queued
 * @return     RUUVI_LIBRARY_ERROR_NO_MEM if the ringbuffer was full
 * @warning    This function has no input checking
 * @warning    Returns a shallow copy of data. 
 */
ruuvi_library_status_t ruuvi_library_ringbuffer_dequeue(ruuvi_library_ringbuffer_t* const buffer, 
                                                        const void *data)
{
  if(ruuvi_library_ringbuffer_empty(buffer)) { return RUUVI_LIBRARY_ERROR_NO_DATA; }
  if(!buffer->lock(buffer->readlock, true)) {return RUUVI_LIBRARY_ERROR_CONCURRENCY; }

  void** p_data = (void**)data;
  *p_data = buffer->storage + (buffer->tail * buffer->block_size);
  buffer->tail = ((buffer->tail + 1) & buffer->index_mask);

  if(!buffer->lock(buffer->readlock, false)) {return RUUVI_LIBRARY_ERROR_FATAL; }
  return RUUVI_LIBRARY_SUCCESS;
}

/*
 * @return true if ringbuffer is full
 */
bool ruuvi_library_ringbuffer_full(const ruuvi_library_ringbuffer_t* const buffer)
{
  return ((buffer->head - buffer->tail) & buffer->index_mask) == buffer->index_mask;
}

/*
 * @return true if ringbuffer is empty
 */
bool ruuvi_library_ringbuffer_empty(const ruuvi_library_ringbuffer_t* const buffer)
{
  return buffer->head == buffer->tail;
}