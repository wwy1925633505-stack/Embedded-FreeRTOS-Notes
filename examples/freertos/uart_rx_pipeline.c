/*
 * Generic UART RX pipeline:
 * ISR/DMA callback -> queue fixed-size chunks -> parser task.
 * The HAL functions and DMA ownership rules must be adapted to the target.
 */
#include <stdint.h>
#include <string.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#include "frame_parser.h"

#define UART_CHUNK_SIZE 64U

typedef struct {
    uint8_t data[UART_CHUNK_SIZE];
    uint16_t length;
} uart_chunk_t;

static QueueHandle_t uart_rx_queue;
static volatile uint32_t uart_queue_drop_count;

void uart_rx_isr_submit(const uint8_t *dma_data, uint16_t length)
{
    BaseType_t higher_priority_task_woken = pdFALSE;
    uart_chunk_t chunk;

    if ((dma_data == NULL) || (length == 0U) || (length > UART_CHUNK_SIZE)) {
        return;
    }

    chunk.length = length;
    (void)memcpy(chunk.data, dma_data, length);

    if (xQueueSendFromISR(uart_rx_queue,
                          &chunk,
                          &higher_priority_task_woken) != pdPASS) {
        uart_queue_drop_count++;
    }
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void UartParserTask(void *argument)
{
    frame_parser_t parser;
    uart_chunk_t chunk;
    (void)argument;
    frame_parser_init(&parser);

    for (;;) {
        if (xQueueReceive(uart_rx_queue, &chunk, portMAX_DELAY) == pdPASS) {
            for (uint16_t i = 0U; i < chunk.length; i++) {
                frame_event_t event = frame_parser_consume(&parser, chunk.data[i]);
                if (event == FRAME_EVENT_READY) {
                    /* Dispatch parser.frame to the command handler. */
                }
            }
        }
    }
}

void uart_rx_pipeline_init(void)
{
    uart_rx_queue = xQueueCreate(8U, sizeof(uart_chunk_t));
    configASSERT(uart_rx_queue != NULL);

    (void)xTaskCreate(UartParserTask,
                      "uart_rx",
                      384U,
                      NULL,
                      tskIDLE_PRIORITY + 2U,
                      NULL);
}

