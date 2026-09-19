#include "dma_ping_pong.h"

bool dma_ping_pong_init(dma_ping_pong_t *ctx,
                        uint8_t *buffer_a,
                        uint8_t *buffer_b,
                        size_t capacity)
{
    if ((ctx == NULL) || (buffer_a == NULL) || (buffer_b == NULL) ||
        (capacity == 0U)) {
        return false;
    }

    ctx->buffers[0] = (dma_buffer_t){buffer_a, capacity, 0U, DMA_BUFFER_FILLING};
    ctx->buffers[1] = (dma_buffer_t){buffer_b, capacity, 0U, DMA_BUFFER_FREE};
    ctx->active_dma_index = 0U;
    ctx->overrun_count = 0U;
    return true;
}

bool dma_ping_pong_complete(dma_ping_pong_t *ctx, size_t received_length)
{
    uint8_t completed_index;
    uint8_t next_index;

    if ((ctx == NULL) || (ctx->active_dma_index > 1U)) {
        return false;
    }

    completed_index = ctx->active_dma_index;
    next_index = (uint8_t)(completed_index ^ 1U);

    if ((ctx->buffers[completed_index].state != DMA_BUFFER_FILLING) ||
        (received_length > ctx->buffers[completed_index].capacity)) {
        return false;
    }

    ctx->buffers[completed_index].valid_length = received_length;
    ctx->buffers[completed_index].state = DMA_BUFFER_READY;

    if (ctx->buffers[next_index].state != DMA_BUFFER_FREE) {
        ctx->overrun_count++;
        return false;
    }

    ctx->buffers[next_index].valid_length = 0U;
    ctx->buffers[next_index].state = DMA_BUFFER_FILLING;
    ctx->active_dma_index = next_index;
    return true;
}

dma_buffer_t *dma_ping_pong_acquire_ready(dma_ping_pong_t *ctx)
{
    uint8_t i;

    if (ctx == NULL) {
        return NULL;
    }

    for (i = 0U; i < 2U; i++) {
        if (ctx->buffers[i].state == DMA_BUFFER_READY) {
            ctx->buffers[i].state = DMA_BUFFER_PROCESSING;
            return &ctx->buffers[i];
        }
    }
    return NULL;
}

bool dma_ping_pong_release(dma_buffer_t *buffer)
{
    if ((buffer == NULL) || (buffer->state != DMA_BUFFER_PROCESSING)) {
        return false;
    }

    buffer->valid_length = 0U;
    buffer->state = DMA_BUFFER_FREE;
    return true;
}

