#ifndef DMA_PING_PONG_H
#define DMA_PING_PONG_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

typedef enum {
    DMA_BUFFER_FREE = 0,
    DMA_BUFFER_FILLING,
    DMA_BUFFER_READY,
    DMA_BUFFER_PROCESSING
} dma_buffer_state_t;

typedef struct {
    uint8_t *data;
    size_t capacity;
    size_t valid_length;
    dma_buffer_state_t state;
} dma_buffer_t;

typedef struct {
    dma_buffer_t buffers[2];
    uint8_t active_dma_index;
    uint32_t overrun_count;
} dma_ping_pong_t;

bool dma_ping_pong_init(dma_ping_pong_t *ctx,
                        uint8_t *buffer_a,
                        uint8_t *buffer_b,
                        size_t capacity);
bool dma_ping_pong_complete(dma_ping_pong_t *ctx, size_t received_length);
dma_buffer_t *dma_ping_pong_acquire_ready(dma_ping_pong_t *ctx);
bool dma_ping_pong_release(dma_buffer_t *buffer);

#endif

