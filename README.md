# MiniRTOS – Time-Based Round Robin RTOS for Bare-Metal Systems

MiniRTOS is a lightweight, cooperative, time-based round-robin RTOS written in C for bare-metal microcontroller systems.  
It is designed to be simple, readable, and educational, making it ideal for learning RTOS fundamentals and building small embedded applications.

This project was developed from scratch and focuses on clarity over complexity.

---

## Features

- Time-based round-robin task scheduling
- Cooperative multitasking (no preemption)
- Periodic, one-shot, and immediate task execution
- Runtime task control (add, remove, pause, resume, modify)
- Circular linked-list based scheduler
- Lightweight interrupt-safe queue implementation
- No dynamic memory allocation
- Low RAM and CPU usage
- CMSIS-compatible critical section handling
- Suitable for small microcontrollers

---

## Architecture Overview

MiniRTOS uses a cooperative scheduling model where tasks are executed sequentially in a round-robin manner.

- A global system tick counter is used for time management
- Each task contains:
  - Function pointer
  - Execution interval (milliseconds)
  - Next scheduled execution time
  - Current task state
- Tasks are stored in a circular linked list
- The scheduler scans tasks and executes eligible ones

No context switching or stack management is involved.

---

## File Structure
```c
MiniRTOS/
│
├── minirtos.c        - Core scheduler and queue implementation
├── minirtos.h        - Public APIs and data structures
├── StdUtil.h         - Utility macros and helper definitions
├── Version.h         - Version and build information
└── README.md
```
---

## Getting Started

### 1. Include MiniRTOS

#include "minirtos.h"

---

### 2. Initialize the RTOS

Call once before creating tasks:

minirtos_Init();

---

### 3. Create a Task

void Task_LED(void)
{
    // Toggle LED
}

Task_Descriptor_t ledTask;

minirtos_AddTask(
    &ledTask,
    Task_LED,
    500,
    TASK_SCHEDULED
);

---

### 4. Run the Scheduler

Call the scheduler continuously inside the main loop:

while (1)
{
    minirtos_Scheduler();
}

---

## Supported Task States

- TASK_SCHEDULED – Periodic task execution
- TASK_ONE_SHOT – Executes once after interval
- TASK_RUN_NOW – Executes immediately
- TASK_ONE_SHOT_NOW – Executes once immediately
- TASK_PAUSE – Task execution paused
- TASK_RUNNING – Task currently executing

---

## Queue Support

MiniRTOS includes a simple interrupt-safe queue mechanism.

Queue APIs:
- minirtos_Queue_Send()
- minirtos_Queue_Receive()
- minirtos_Queue_Count()
- minirtos_Queue_Flush()

Queue operations are protected using critical sections and are safe for ISR usage.

---

## Limitations

- No task preemption
- No priority-based scheduling
- No software timers
- No mutexes or semaphores
- Requires an external system tick source

---

## License

This project is released under the MIT License.

---

## Author

Sourabh Potdar  
Embedded Systems & Firmware Developer
