#include <assert.h>
#include <stdio.h>

#include "dma_ping_pong.h"
#include "frame_parser.h"
#include "ring_buffer.h"

static void test_ring_buffer_wraparound(void)
{
    uint8_t storage[4];
    uint8_t value;
    ring_buffer_t rb;

    assert(ring_buffer_init(&rb, storage, sizeof(storage)));
    assert(ring_buffer_push(&rb, 1U));
    assert(ring_buffer_push(&rb, 2U));
    assert(ring_buffer_push(&rb, 3U));
    assert(ring_buffer_pop(&rb, &value) && (value == 1U));
    assert(ring_buffer_push(&rb, 4U));
    assert(ring_buffer_push(&rb, 5U));
    assert(!ring_buffer_push(&rb, 6U));

    for (uint8_t expected = 2U; expected <= 5U; expected++) {
        assert(ring_buffer_pop(&rb, &value));
        assert(value == expected);
    }
    assert(ring_buffer_size(&rb) == 0U);
}

static size_t build_frame(uint8_t *out,
                          uint8_t command,
                          const uint8_t *payload,
                          uint16_t payload_length)
{
    uint8_t crc_input[1U + FRAME_MAX_PAYLOAD];
    uint16_t crc;

    out[0] = FRAME_SOF_0;
    out[1] = FRAME_SOF_1;
    out[2] = (uint8_t)(payload_length & 0xFFU);
    out[3] = (uint8_t)(payload_length >> 8U);
    out[4] = command;
    crc_input[0] = command;

    for (uint16_t i = 0U; i < payload_length; i++) {
        out[5U + i] = payload[i];
        crc_input[1U + i] = payload[i];
    }

    crc = protocol_crc16(crc_input, 1U + payload_length);
    out[5U + payload_length] = (uint8_t)(crc & 0xFFU);
    out[6U + payload_length] = (uint8_t)(crc >> 8U);
    return 7U + payload_length;
}

static void test_frame_parser(void)
{
    const uint8_t payload[] = {0x10U, 0x20U, 0x30U};
    uint8_t bytes[16];
    const size_t length = build_frame(bytes, 0x42U, payload, sizeof(payload));
    frame_parser_t parser;
    frame_event_t event = FRAME_EVENT_NONE;

    frame_parser_init(&parser);
    for (size_t i = 0U; i < length; i++) {
        event = frame_parser_consume(&parser, bytes[i]);
    }

    assert(event == FRAME_EVENT_READY);
    assert(parser.frame.command == 0x42U);
    assert(parser.frame.payload_length == sizeof(payload));
    assert(parser.frame.payload[2] == 0x30U);

    bytes[length - 1U] ^= 0x01U;
    frame_parser_init(&parser);
    for (size_t i = 0U; i < length; i++) {
        event = frame_parser_consume(&parser, bytes[i]);
    }
    assert(event == FRAME_EVENT_BAD_CRC);
}

static void test_dma_ping_pong(void)
{
    uint8_t a[8];
    uint8_t b[8];
    dma_ping_pong_t ctx;
    dma_buffer_t *ready;

    assert(dma_ping_pong_init(&ctx, a, b, sizeof(a)));
    assert(dma_ping_pong_complete(&ctx, 6U));
    assert(ctx.active_dma_index == 1U);

    ready = dma_ping_pong_acquire_ready(&ctx);
    assert(ready != NULL);
    assert(ready->data == a);
    assert(ready->valid_length == 6U);
    assert(dma_ping_pong_release(ready));

    assert(dma_ping_pong_complete(&ctx, 4U));
    assert(ctx.active_dma_index == 0U);
}

int main(void)
{
    test_ring_buffer_wraparound();
    test_frame_parser();
    test_dma_ping_pong();
    puts("All host-side tests passed.");
    return 0;
}

