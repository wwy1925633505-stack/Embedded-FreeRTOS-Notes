# Embedded FreeRTOS Notes

[![Language](https://img.shields.io/badge/Language-C-00599C.svg)](https://en.cppreference.com/w/c/language)
[![RTOS](https://img.shields.io/badge/RTOS-FreeRTOS-2C5F2D.svg)](https://www.freertos.org/)
[![CI](https://github.com/wwy1925633505-stack/Embedded-FreeRTOS-Notes/actions/workflows/ci.yml/badge.svg)](https://github.com/wwy1925633505-stack/Embedded-FreeRTOS-Notes/actions)
[![License](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)

一个面向嵌入式开发、知识复盘的 FreeRTOS 学习仓库。

## Repository Map

| 模块 | 内容 | 能力证明 |
| --- | --- | --- |
| FreeRTOS 基础 | 任务、状态、调度、上下文切换 | 理解实时内核运行机制 |
| IPC 与同步 | 队列、信号量、互斥量、事件组、任务通知 | 能选择合适的任务通信方式 |
| 中断设计 | ISR 约束、延迟中断处理、优先级 | 能设计安全的中断—任务链路 |
| 外设与 DMA | UART DMA、Ping-Pong Buffer、环形缓冲区 | 能处理异步数据流和吞吐问题 |
| 通信协议 | UART/CAN/SPI/I²C、帧解析、CRC | 能设计分层、可恢复的通信模块 |
| C 与内存 | 生命周期、越界、对齐、volatile、静态分配 | 能编写适合 MCU 的防御式 C 代码 |
| 调试与 Git | 故障定位、日志、分支与提交规范 | 能形成可复现的工程流程 |

## Learning Path

1. [FreeRTOS 任务模型](docs/01-task-model.md)
2. [调度器与上下文切换](docs/02-scheduler-context-switch.md)
3. [任务通信与同步](docs/03-ipc-and-synchronization.md)
4. [中断与任务协作](docs/04-isr-to-task.md)
5. [内存管理与栈分析](docs/05-memory-and-stack.md)
6. [MCU 外设与 DMA](docs/06-peripheral-and-dma.md)
7. [UART、CAN、SPI、I²C](docs/07-communication-protocols.md)
8. [嵌入式 C 与内存安全](docs/08-embedded-c.md)
9. [调试方法与 Git 工作流](docs/09-debugging-and-git.md)

## Generic Examples

| 示例 | 说明 | 是否可在主机测试 |
| --- | --- | --- |
| `ring_buffer` | 无动态内存的字节环形缓冲区 | ✅ |
| `frame_parser` | 带长度与 CRC16 的流式帧解析器 | ✅ |
| `dma_ping_pong` | DMA 双缓冲状态机 | ✅ |
| `task_notification` | ISR 通过任务通知唤醒处理任务 | 需 FreeRTOS |
| `uart_rx_pipeline` | UART DMA/空闲中断到解析任务 | 需 FreeRTOS/适配层 |

## Build and Test

主机端测试只覆盖与硬件无关的纯 C 模块：

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build --output-on-failure
```

FreeRTOS 示例用于展示设计模式，需放入具体 MCU 工程并实现 HAL 适配接口。

## Design Rules

- ISR 只做采样、清标志和通知，耗时逻辑放到任务中。
- 数据流与控制流分开：队列传数据，通知/信号量传事件。
- 优先使用静态内存或在初始化阶段完成分配。
- 共享资源必须明确所有权、临界区范围和超时策略。
- 错误必须可观测：返回值、计数器、日志和复现步骤缺一不可。

## Disclaimer

本仓库仅包含通用学习笔记和重新编写的示例代码，不包含任何公司源码、产品参数或保密实现。

