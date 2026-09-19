# 04. 中断与任务协作

## 1. 延迟中断处理

中断负责快速响应，任务负责复杂处理：

```text
Peripheral IRQ → read/clear status → capture minimum data
               → notify task → parse/process/recover in task context
```

## 2. ISR 中应该做什么

- 读取并清除必要的中断标志。
- 保存必须立即保存的数据或 DMA 写入位置。
- 更新简单的诊断计数器。
- 使用 `...FromISR()` API 唤醒任务。
- 根据返回值请求在退出中断时切换任务。

不应在 ISR 中阻塞、获取互斥量、执行大段协议解析、打印大量日志或调用不可重入接口。

## 3. 任务通知模板

参见 [`examples/freertos/task_notification.c`](../examples/freertos/task_notification.c)。核心点是：

- ISR 使用 `vTaskNotifyGiveFromISR()`。
- 任务使用 `ulTaskNotifyTake()` 阻塞等待。
- `portYIELD_FROM_ISR()` 只在唤醒了更高优先级任务时请求切换。

## 4. 中断优先级约束

能调用 FreeRTOS FromISR API 的中断，其 NVIC 优先级必须满足当前端口对 `configMAX_SYSCALL_INTERRUPT_PRIORITY` 的要求。数值更小通常代表硬件优先级更高，因此不能只凭“数字大小”判断是否合法。

## 5. 可观测性

建议为每条中断链路保留以下计数器：

- IRQ 总次数。
- 通知/入队成功和失败次数。
- DMA 错误、中止与溢出次数。
- 任务实际处理次数。
- 最大队列深度或最大处理延迟。

“总线有帧但应用少帧”时，这些数据能帮助判断丢失发生在硬件、ISR、队列还是组包层。

