#include "ruuvi_library.h"
#include "ruuvi_library_ringbuffer.h"
#include <string.h>


ruuvi_library_status_t ruuvi_library_ringbuffer_queue(ruuvi_library_ringbuffer_t* const buffer, 
                                                      const void* const data,
                                                      const size_t data_length)
{
  if(NULL == buffer || NULL == data)         { return RUUVI_LIBRARY_ERROR_NULL; }
  if(ruuvi_library_ringbuffer_full(buffer))  { return RUUVI_LIBRARY_ERROR_NO_MEM; }
  if(buffer->block_size < data_length)       { return RUUVI_LIBRARY_ERROR_DATA_LENGTH; }
  if(!buffer->lock(buffer->writelock, true)) { return RUUVI_LIBRARY_ERROR_CONCURRENCY; }

  memcpy((buffer->storage) + (buffer->head * buffer->block_size), 
         data, data_length);
  buffer->head = ((buffer->head + 1) & buffer->index_mask);
  if(!buffer->lock(buffer->writelock, false)) { return RUUVI_LIBRARY_ERROR_FATAL; }
  return RUUVI_LIBRARY_SUCCESS;
}

ruuvi_library_status_t ruuvi_library_ringbuffer_dequeue(ruuvi_library_ringbuffer_t* const buffer, 
                                                        const void *data)
{
  if(NULL == buffer || NULL == data)         { return RUUVI_LIBRARY_ERROR_NULL; }
  if(ruuvi_library_ringbuffer_empty(buffer)) { return RUUVI_LIBRARY_ERROR_NO_DATA; }
  if(!buffer->lock(buffer->readlock, true))  {return RUUVI_LIBRARY_ERROR_CONCURRENCY; }

  void** p_data = (void**)data;
  *p_data = buffer->storage + (buffer->tail * buffer->block_size);
  buffer->tail = ((buffer->tail + 1) & buffer->index_mask);

  if(!buffer->lock(buffer->readlock, false)) {return RUUVI_LIBRARY_ERROR_FATAL; }
  return RUUVI_LIBRARY_SUCCESS;
}

ruuvi_library_status_t ruuvi_library_ringbuffer_peek(ruuvi_library_ringbuffer_t* const buffer, 
                                                        const void *data, const size_t index)
{
  if(NULL == buffer || NULL == data)         { return RUUVI_LIBRARY_ERROR_NULL; }
  if((buffer->head - (buffer->tail + index)) > buffer->index_mask) { return RUUVI_LIBRARY_ERROR_NO_DATA; }
  if(!buffer->lock(buffer->readlock, true))  {return RUUVI_LIBRARY_ERROR_CONCURRENCY; }

  void** p_data = (void**)data;
  *p_data = buffer->storage + (((buffer->tail + index) & buffer->index_mask) * buffer->block_size);

  if(!buffer->lock(buffer->readlock, false)) {return RUUVI_LIBRARY_ERROR_FATAL; }
  return RUUVI_LIBRARY_SUCCESS;
}

bool ruuvi_library_ringbuffer_full(const ruuvi_library_ringbuffer_t* const buffer)
{
  return ((buffer->head - buffer->tail) & buffer->index_mask) == buffer->index_mask;
}

bool ruuvi_library_ringbuffer_empty(const ruuvi_library_ringbuffer_t* const buffer)
{
  return buffer->head == buffer->tail;
}