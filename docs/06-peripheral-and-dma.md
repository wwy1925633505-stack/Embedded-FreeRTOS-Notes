# 06. MCU 外设与 DMA

## 1. DMA 解决什么问题

DMA 在外设和内存之间搬运数据，减少 CPU 逐字节读写开销。CPU 仍需负责配置、缓冲区管理、完成通知和错误恢复。

适合 DMA 的场景：

- UART 连续接收或批量发送。
- ADC 多通道循环采样。
- SPI 大块收发。
- I²S 连续音频流。

数据很短、频率很低或启动 DMA 的成本高于搬运本身时，中断或轮询可能更简单。

## 2. Ping-Pong Buffer

双缓冲将“DMA 写入”和“CPU 处理”解耦：

```text
Time N:     DMA → Buffer A    CPU → Buffer B
Time N + 1: DMA → Buffer B    CPU → Buffer A
```

必须定义缓冲区状态：`FREE`、`DMA_OWNED`、`CPU_READY`、`CPU_OWNED`。当 CPU 处理速度长期低于 DMA 生产速度时，再多缓冲区也只能延缓溢出，不能消除吞吐不平衡。

## 3. UART DMA + Idle

通用接收链路：

1. DMA 循环或普通模式接收字节。
2. 空闲中断或 DMA 半满/全满中断确定新增数据范围。
3. ISR 只更新写指针并通知任务。
4. 任务从环形缓冲区取数据并执行帧解析。
5. 溢出时记录计数，丢弃到已知帧头并重新同步。

## 4. Cache 与内存屏障

带数据 Cache 的 MCU/SoC 上，DMA 与 CPU 可能看到不同的数据。应根据平台要求：

- DMA 发送前清理 Cache。
- DMA 接收后失效对应 Cache 行。
- 保证缓冲区对齐和大小满足 Cache 行要求。
- 在所有权切换处使用平台要求的内存屏障。

`volatile` 不能替代 Cache 维护、原子操作或内存屏障。

## 5. 错误处理

- 检查 DMA 传输错误、FIFO 错误和外设错误标志。
- 处理启动失败、超时、重复完成中断和意外停止。
- 保留 `overrun_count`、`dma_error_count`、`max_processing_time`。
- 恢复前先停止外设请求，再复位 DMA 状态并重新建立缓冲区所有权。

