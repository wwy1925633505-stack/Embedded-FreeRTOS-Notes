#ifndef FRAME_PARSER_H
#define FRAME_PARSER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define FRAME_SOF_0        0xAAU
#define FRAME_SOF_1        0x55U
#define FRAME_MAX_PAYLOAD  128U

typedef struct {
    uint8_t command;
    uint8_t payload[FRAME_MAX_PAYLOAD];
    uint16_t payload_length;
} protocol_frame_t;

typedef enum {
    FRAME_EVENT_NONE = 0,
    FRAME_EVENT_READY,
    FRAME_EVENT_BAD_LENGTH,
    FRAME_EVENT_BAD_CRC
} frame_event_t;

typedef enum {
    PARSER_WAIT_SOF_0 = 0,
    PARSER_WAIT_SOF_1,
    PARSER_READ_LENGTH_LOW,
    PARSER_READ_LENGTH_HIGH,
    PARSER_READ_COMMAND,
    PARSER_READ_PAYLOAD,
    PARSER_READ_CRC_LOW,
    PARSER_READ_CRC_HIGH
} parser_state_t;

typedef struct {
    parser_state_t state;
    protocol_frame_t frame;
    uint16_t payload_index;
    uint16_t received_crc;
} frame_parser_t;

void frame_parser_init(frame_parser_t *parser);
frame_event_t frame_parser_consume(frame_parser_t *parser, uint8_t byte);
uint16_t protocol_crc16(const uint8_t *data, size_t length);

#endif

