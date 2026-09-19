#include "frame_parser.h"

static void parser_reset(frame_parser_t *parser)
{
    parser->state = PARSER_WAIT_SOF_0;
    parser->payload_index = 0U;
    parser->received_crc = 0U;
    parser->frame.payload_length = 0U;
    parser->frame.command = 0U;
}

void frame_parser_init(frame_parser_t *parser)
{
    if (parser != NULL) {
        parser_reset(parser);
    }
}

uint16_t protocol_crc16(const uint8_t *data, size_t length)
{
    uint16_t crc = 0xFFFFU;
    size_t i;

    if (data == NULL) {
        return crc;
    }

    for (i = 0U; i < length; i++) {
        uint8_t bit;
        crc ^= data[i];
        for (bit = 0U; bit < 8U; bit++) {
            crc = ((crc & 1U) != 0U) ? (uint16_t)((crc >> 1U) ^ 0xA001U)
                                     : (uint16_t)(crc >> 1U);
        }
    }
    return crc;
}

static uint16_t frame_crc(const protocol_frame_t *frame)
{
    uint8_t crc_input[1U + FRAME_MAX_PAYLOAD];

    crc_input[0] = frame->command;
    for (uint16_t i = 0U; i < frame->payload_length; i++) {
        crc_input[1U + i] = frame->payload[i];
    }
    return protocol_crc16(crc_input, 1U + frame->payload_length);
}

frame_event_t frame_parser_consume(frame_parser_t *parser, uint8_t byte)
{
    uint16_t expected_crc;

    if (parser == NULL) {
        return FRAME_EVENT_NONE;
    }

    switch (parser->state) {
    case PARSER_WAIT_SOF_0:
        if (byte == FRAME_SOF_0) {
            parser->state = PARSER_WAIT_SOF_1;
        }
        break;
    case PARSER_WAIT_SOF_1:
        parser->state = (byte == FRAME_SOF_1) ? PARSER_READ_LENGTH_LOW
                                              : PARSER_WAIT_SOF_0;
        break;
    case PARSER_READ_LENGTH_LOW:
        parser->frame.payload_length = byte;
        parser->state = PARSER_READ_LENGTH_HIGH;
        break;
    case PARSER_READ_LENGTH_HIGH:
        parser->frame.payload_length |= (uint16_t)((uint16_t)byte << 8U);
        if (parser->frame.payload_length > FRAME_MAX_PAYLOAD) {
            parser_reset(parser);
            return FRAME_EVENT_BAD_LENGTH;
        }
        parser->state = PARSER_READ_COMMAND;
        break;
    case PARSER_READ_COMMAND:
        parser->frame.command = byte;
        parser->payload_index = 0U;
        parser->state = (parser->frame.payload_length == 0U)
                            ? PARSER_READ_CRC_LOW
                            : PARSER_READ_PAYLOAD;
        break;
    case PARSER_READ_PAYLOAD:
        parser->frame.payload[parser->payload_index++] = byte;
        if (parser->payload_index == parser->frame.payload_length) {
            parser->state = PARSER_READ_CRC_LOW;
        }
        break;
    case PARSER_READ_CRC_LOW:
        parser->received_crc = byte;
        parser->state = PARSER_READ_CRC_HIGH;
        break;
    case PARSER_READ_CRC_HIGH:
        parser->received_crc |= (uint16_t)((uint16_t)byte << 8U);
        expected_crc = frame_crc(&parser->frame);
        if (expected_crc == parser->received_crc) {
            parser->state = PARSER_WAIT_SOF_0;
            return FRAME_EVENT_READY;
        }
        parser_reset(parser);
        return FRAME_EVENT_BAD_CRC;
    default:
        parser_reset(parser);
        break;
    }

    return FRAME_EVENT_NONE;
}

