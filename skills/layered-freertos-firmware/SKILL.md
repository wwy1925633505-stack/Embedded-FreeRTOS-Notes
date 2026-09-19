---
name: layered-freertos-firmware
description: Design or refactor MCU FreeRTOS firmware with explicit driver, middleware, OS-adapter, and application layers. Use when creating peripherals, protocol modules, RTOS tasks, or reorganizing tightly coupled embedded C code; preserve an existing vendor toolchain and behavior unless the user asks for migration.
---

# Layered FreeRTOS Firmware

Generate embedded C code with visible module boundaries, one-way dependencies, explicit ownership, and small public interfaces. Retain the reference project's strengths—one peripheral per folder, `.c/.h` pairing, device context structures, centralized initialization, and grouped task configuration—while avoiding unsafe legacy patterns.

## Start From Repository Evidence

Before changing code:

1. Inspect the directory tree, build files, MCU family, vendor library or HAL, RTOS version, interrupt files, linker configuration, and existing naming style.
2. Trace the current call path from interrupt or task to driver and application behavior.
3. Identify shared state, ownership, execution context, timeouts, DMA buffers, and error paths.
4. Preserve the current toolchain, public behavior, pin assignment, clock configuration, and interrupt mapping unless the user explicitly requests a migration.

Do not reorganize an entire working firmware project merely to match a preferred folder tree. For existing projects, introduce boundaries incrementally and keep compatibility wrappers when required.

## Layer Model

Use the smallest set of layers that keeps dependencies clear:

```text
App/          Tasks, product state machines, use-case coordination
Services/     Device services and RTOS-facing orchestration when needed
Middleware/   Protocols, parsers, algorithms, filters, buffers, reusable state machines
OSAL/         Thin FreeRTOS adaptation used by reusable modules when isolation is useful
Drivers/      Peripheral/device drivers and ISR-facing hardware operations
BSP/          Board pins, clocks, DMA channels, IRQ mapping, board startup
Vendor/       CMSIS, MCU HAL/SPL, startup files; normally unchanged
Config/       Board and task configuration, feature flags, limits
```

In a legacy `SYSTEM / HARDWARE / USER` project, map rather than blindly move:

- `SYSTEM` usually maps to platform/BSP facilities.
- `HARDWARE/<device>` maps to Drivers.
- reusable parsing or algorithms should leave `USER` and enter Middleware.
- task functions and product behavior map to App.
- vendor library, CMSIS, startup, linker, and FreeRTOS kernel sources remain third-party/platform code.

## Dependency Rules

The dependency direction is one way:

```text
App → Services/Middleware → Drivers/BSP → Vendor
App → OSAL/FreeRTOS
ISR → Driver event capture → RTOS notification/queue → App task
```

Enforce these boundaries:

- Drivers must not include application headers or call product logic.
- Hardware-independent middleware must not include MCU register or vendor HAL headers.
- Drivers should not depend directly on FreeRTOS when a callback, event flag, or adapter can preserve reuse.
- Application tasks may coordinate modules but must not configure peripheral registers directly.
- Interrupt handlers capture status/data, clear flags, and signal deferred work; they do not run business logic.
- Do not create circular includes or use `extern` globals as a substitute for an interface.

## Module Shape

Keep one cohesive device or function per folder with a matching source/header pair:

```text
Drivers/Uart/uart_driver.h
Drivers/Uart/uart_driver.c
Middleware/Frame/frame_parser.h
Middleware/Frame/frame_parser.c
App/Comm/comm_task.h
App/Comm/comm_task.c
```

Headers contain only public types, constants, and functions. Implementation details, helper functions, mutable state, vendor handles, and private buffers stay in `.c` files and use `static` when module-local.

Prefer an explicit context/configuration structure over unrelated global variables:

```c
typedef struct {
    USART_TypeDef *instance;
    DMA_Channel_TypeDef *rx_dma;
    uint8_t *rx_buffer;
    uint16_t rx_capacity;
} uart_driver_config_t;

typedef struct {
    uart_driver_config_t config;
    volatile uint16_t rx_length;
    volatile uint32_t error_count;
} uart_driver_t;
```

Use `const` for configuration that does not change after initialization. State who owns every buffer and when ownership transfers.

## Naming and Formatting

When the repository has a consistent convention, follow it. For new modules use a stable `Module_Action` API similar to the reference project, but make capitalization consistent:

```c
UartDriver_Init()
UartDriver_StartRx()
UartDriver_GetStatus()
UartDriver_IrqHandler()
FrameParser_Init()
FrameParser_Input()
CommTask_Init()
```

Use:

- `snake_case_t` for types and `UPPER_SNAKE_CASE` for compile-time constants;
- fixed-width integer types from `<stdint.h>` at interfaces;
- `bool` or a documented status enum instead of magic integers;
- 4-space indentation and braces consistent with surrounding files;
- Chinese comments for design intent, hardware constraints, ownership, timing, and recovery—not comments that only restate a line of code.

Public functions must document parameters, return values, context restrictions, and blocking behavior. Avoid mixed naming such as `Adc_Init`, `led_config`, and `MYDMA_Config` inside one new module.

## Driver Layer

A driver owns register/HAL interaction and hardware state. Separate configuration from operation:

```c
driver_status_t UartDriver_Init(uart_driver_t *driver,
                                const uart_driver_config_t *config);
driver_status_t UartDriver_StartRx(uart_driver_t *driver);
driver_status_t UartDriver_Write(uart_driver_t *driver,
                                 const uint8_t *data,
                                 uint16_t length,
                                 uint32_t timeout_ms);
void UartDriver_IrqHandler(uart_driver_t *driver);
```

Driver requirements:

- validate pointers, lengths, channels, and current state;
- return explicit status instead of silently failing;
- keep pin/clock/DMA/IRQ choices in BSP or configuration;
- expose events or data, not vendor register details, to upper layers;
- make timeout and blocking behavior explicit;
- record useful error and overflow counters;
- provide recovery or reinitialization behavior when the hardware can fail;
- keep ISR-safe entry points separate from task-context APIs.

## Middleware Layer

Middleware owns reusable logic such as framing, CRC, ring buffers, filters, command decoding, and state machines. It accepts data through normal C interfaces and must be testable without the target MCU when practical.

Do not let a parser directly control GPIO, send UART data, or call an application task. Return an event/result or invoke a narrow callback supplied by the caller.

Prefer incremental state machines for streaming input. Handle partial frames, multiple frames, invalid length, CRC failure, timeout, overflow, and resynchronization explicitly.

## Application and Task Layer

Each task owns one clear responsibility. A task module should normally provide an initialization function and keep its task handle/function private:

```c
bool CommTask_Init(const comm_task_config_t *config);

static void CommTask_Entry(void *argument)
{
    for (;;) {
        /* 阻塞等待事件，处理完成后再次阻塞。 */
    }
}
```

Group task stack, priority, queue depth, and timing configuration in `Config/` or at the top of the task module. Name units: ticks, milliseconds, bytes, words, hertz, and samples must not be ambiguous.

Keep `main()` limited to platform initialization, module initialization, task creation, scheduler start, and fatal-startup handling. Move IRQ handlers to the platform interrupt file or driver adapter; move task bodies out of `main.c`.

## FreeRTOS Rules

Choose communication by semantics:

- queue: transfer typed data or ownership between contexts;
- task notification: lightweight event/count for one receiving task;
- binary semaphore: event synchronization when a named synchronization object improves clarity;
- counting semaphore: count interchangeable resources or repeated events;
- mutex: protect a task-shared resource and gain priority inheritance;
- event group: coordinate several boolean conditions;
- stream/message buffer: single-writer/single-reader byte stream or variable-length messages.

In an ISR:

- use only documented `...FromISR()` APIs;
- clear/capture the hardware event before exit;
- request `portYIELD_FROM_ISR()` only through the returned wake flag;
- never delay, block, allocate memory, take a mutex, print long logs, parse a protocol, or update a display.

Prefer blocking tasks over polling loops. Every external wait needs a deliberate timeout policy or a documented reason for `portMAX_DELAY`.

Create queues, semaphores, and task-visible resources before enabling their producer interrupts. Check every creation return value. Prefer static allocation or initialization-time allocation for fixed long-lived objects when the project requires deterministic memory use.

Derive priorities from deadlines, data-loss risk, and blocking relationships—not perceived feature importance. Measure task stack high-water marks under stress and keep a documented margin.

## Initialization Order

Use a visible, fail-fast startup sequence:

1. clock, vector table, and board safety state;
2. BSP pins and peripheral clocks;
3. driver contexts with interrupts/DMA requests still disabled;
4. middleware and application state;
5. queues, notifications, mutexes, and tasks;
6. enable peripheral interrupts and DMA producers;
7. start the scheduler.

If any required step fails, enter a defined safe state and expose a diagnostic code. Do not continue with a partially initialized dependency graph.

## Error and Concurrency Design

For every module, define:

- valid states and transitions;
- caller and execution context for each API;
- buffer and object ownership;
- ISR/task/shared data and atomicity needs;
- timeout, retry, overflow, and recovery policy;
- counters or logs needed to locate failures by layer.

`volatile` is only for values that can change outside normal program flow; it does not provide atomicity, mutual exclusion, cache coherency, or task synchronization.

Keep critical sections short. Never wrap task creation, peripheral calibration, logging, or other long work in a global critical section merely for convenience.

## Agent Output Contract

When designing or implementing a feature, return or create artifacts in this order:

1. current architecture and coupling problems discovered from the repository;
2. target dependency path and concise file tree;
3. public header interfaces and ownership rules;
4. implementation separated by layer;
5. initialization and integration changes;
6. task/ISR timing, memory, and concurrency considerations;
7. build or test evidence and unresolved hardware assumptions.

Do not invent MCU pins, DMA channels, interrupt priorities, clock frequencies, FreeRTOS configuration, or vendor APIs. Resolve them from the project; if unavailable, expose them as named configuration values and state the assumption.

## Completion Checklist

- Every new module has one purpose and a small public interface.
- Dependency arrows point downward; no driver calls application code.
- `main.c` contains orchestration rather than business logic.
- ISR work is bounded and deferred processing is explicit.
- Shared state and buffer ownership are documented.
- Queue depths, timeouts, task stacks, and priority choices have reasons.
- Driver and middleware failures reach the application through status/events.
- Hardware-independent middleware has host-testable boundaries where practical.
- Generated code follows the repository's toolchain and compiles without unrelated rewrites.
