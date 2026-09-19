/* Generic FreeRTOS pattern. Connect peripheral_irq_clear() to the target HAL. */
#include "FreeRTOS.h"
#include "task.h"

static TaskHandle_t processing_task_handle;

static void peripheral_irq_clear(void)
{
    /* Clear the real peripheral interrupt flag here. */
}

void Peripheral_IRQHandler(void)
{
    BaseType_t higher_priority_task_woken = pdFALSE;

    peripheral_irq_clear();
    vTaskNotifyGiveFromISR(processing_task_handle,
                           &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

static void ProcessingTask(void *argument)
{
    (void)argument;

    for (;;) {
        /* pdTRUE clears the accumulated notification count on exit. */
        (void)ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        /* Run non-ISR processing here. */
    }
}

void task_notification_example_init(void)
{
    (void)xTaskCreate(ProcessingTask,
                      "process",
                      256U,
                      NULL,
                      tskIDLE_PRIORITY + 2U,
                      &processing_task_handle);
}

