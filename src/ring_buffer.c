#include "ring_buffer.h"

static bool ring_buffer_valid(const ring_buffer_t *rb)
{
    return (rb != NULL) && (rb->storage != NULL) && (rb->capacity > 0U);
}

bool ring_buffer_init(ring_buffer_t *rb, uint8_t *storage, size_t capacity)
{
    if ((rb == NULL) || (storage == NULL) || (capacity == 0U)) {
        return false;
    }

    rb->storage = storage;
    rb->capacity = capacity;
    rb->head = 0U;
    rb->tail = 0U;
    rb->count = 0U;
    return true;
}

bool ring_buffer_push(ring_buffer_t *rb, uint8_t value)
{
    if (!ring_buffer_valid(rb) || (rb->count == rb->capacity)) {
        return false;
    }

    rb->storage[rb->head] = value;
    rb->head = (rb->head + 1U) % rb->capacity;
    rb->count++;
    return true;
}

bool ring_buffer_pop(ring_buffer_t *rb, uint8_t *value)
{
    if (!ring_buffer_valid(rb) || (value == NULL) || (rb->count == 0U)) {
        return false;
    }

    *value = rb->storage[rb->tail];
    rb->tail = (rb->tail + 1U) % rb->capacity;
    rb->count--;
    return true;
}

size_t ring_buffer_write(ring_buffer_t *rb, const uint8_t *data, size_t length)
{
    size_t written = 0U;

    if (data == NULL) {
        return 0U;
    }

    while ((written < length) && ring_buffer_push(rb, data[written])) {
        written++;
    }
    return written;
}

size_t ring_buffer_read(ring_buffer_t *rb, uint8_t *data, size_t length)
{
    size_t read_count = 0U;

    if (data == NULL) {
        return 0U;
    }

    while ((read_count < length) && ring_buffer_pop(rb, &data[read_count])) {
        read_count++;
    }
    return read_count;
}

size_t ring_buffer_size(const ring_buffer_t *rb)
{
    return ring_buffer_valid(rb) ? rb->count : 0U;
}

size_t ring_buffer_free(const ring_buffer_t *rb)
{
    return ring_buffer_valid(rb) ? (rb->capacity - rb->count) : 0U;
}

