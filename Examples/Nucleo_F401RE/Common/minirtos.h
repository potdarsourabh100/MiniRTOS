/**
 * \file           minirtos.h
 * \brief          Scheduler header file
 */

/*
 * Copyright (c) 2024 Sourabh Potdar
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sub-license, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * Author:          Sourabh Potdar
 * Version:         V1.0.0
 */

#ifndef MINIRTOS_H_
#define MINIRTOS_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*****************************************************************************/
/* Include Files                                                             */
/*****************************************************************************/
#include "StdUtil.h"
#include "Version.h"
#include "cmsis_gcc.h"
/*****************************************************************************/
/* Private Defines                                                           */
/*****************************************************************************/
/**
 * @brief Maximum no of Tasks.
 *
 * @details Maximum no of tasks allowed is 255.
 */
#define MAX_TASKS_NUMBER            255

/**
 * @brief Maximum task time periods.
 *
 * @details Represents 1 Hour (1 hour * 60 Minutes * 60 * Seconds * 1000 milliseconds = 3600000 MilliSeconds).
 */
#define MAX_TASK_INTERVAL           3600000UL

/**
 * @brief Default task interval
 *
 * @details Default task intervals in case user puts an odd value
 *
 * @note currently set to 100mSec can be changed as per needs
 */
#define DEFAULT_TASK_INTERVAL       100

/**
 * @brief Default No of Queue elements
 *
 * @details Default no of queue elements in case user puts an odd value
 *
 * @note currently set to 20 can be changed as per needs
 */
#define MAX_NO_OF_QUEUE_ELEMENTS    20

/**
 * @brief Enter MiniRTOS queue critical section (disable interrupts, save state).
 *
 * @details
 *   Disables all interrupts to protect queue data structure from concurrent access
 *   in both main code and interrupt context. Remembers previous PRIMASK state for safe restore.
 *
 * @warning
 *   Only use around minimal, fast code. Do not place long-latency code or blocking operations inside.
 */
#define MINIRTOS_QUEUE_ENTER_CRITICAL()   uint32_t primask = __get_PRIMASK(); __set_PRIMASK(1)

/**
 * @brief Exit MiniRTOS queue critical section (restore prior interrupt state).
 *
 * @details
 *   If interrupts were previously enabled, this re-enables them.
 *   Should always be called after MINIRTOS_QUEUE_ENTER_CRIT().
 */
#define MINIRTOS_QUEUE_EXIT_CRITICAL()    __set_PRIMASK(primask)
/*****************************************************************************/
/* Private typedefs                                                          */
/*****************************************************************************/
/**
 * @brief Pointer for Task functions.
 *
 * @details Function pointer on the task body.
 */
typedef void (*gptr_Task_Function)(void);
/*****************************************************************************/
/* Private Enums                                                             */
/*****************************************************************************/
/**
 * @brief Enum for Task status.
 *
 * @details Used to switch between different task states.
 */
typedef enum
{
    // For a task that doesn't have to start immediately.
    TASK_PAUSE              = 0x00,
    // For a normal task that has to start after its scheduling.
    TASK_SCHEDULED          = 0x01,
    // For a task that has to run only once.
    TASK_ONE_SHOT           = 0x02,
    // For a task that has to be executed once it has been added.
    TASK_RUN_NOW            = 0x03,
    // For the task to be executed one time as soon as it is added.
    TASK_ONE_SHOT_NOW       = 0x05,
	// for the task which is currently running
	TASK_RUNNING            = 0x06,
    // Error, task not found.
    TASK_NOT_FOUND          = 0xFF
} Task_Status_e;
/*****************************************************************************/
/* Private Structures                                                        */
/*****************************************************************************/
/**
 * @brief Queue object for MiniRTOS.
 *
 * @details Keeps buffer information and internal state (head/tail/count).
 */
typedef struct {
	/* Pointer to user-allocated queue memory. */
    void *buffer;
    /* Size (in bytes) of each element. */
    uint16_t elementSize;
    /* Maximum elements queue can hold. */
    uint16_t maxElements;
    /* Read index. */
    uint16_t head;
    /* Write index. */
    uint16_t tail;
    /* Number of elements currently in queue. */
    uint16_t count;
} Queue_Descriptor_t;

/**
 * @brief Structure for task description.
 *
 * @details Task related flags, functions & details.
 */
typedef struct _Task_Descriptor_t
{
    /*Used to store the pointers to user's tasks*/
	gptr_Task_Function taskPointer;
    /*Used to store the interval between each task's run*/
    uint32_t taskInterval;
    /*Used to store the next time a task will have to be executed*/
    uint32_t plannedTask;
    /*Used to store the status of the tasks*/
    Task_Status_e taskStatus;
    /*Pointer to the next task in the list.*/
    struct _Task_Descriptor_t *gptrTaskNext;
} Task_Descriptor_t;
/*****************************************************************************/
/* Private Variables                                                         */
/*****************************************************************************/

extern volatile uintmax_t glbSysTicks; /** System tick value that increments periodically **/

/*****************************************************************************/
/* User Functions                                                            */
/*****************************************************************************/
/**
 * @brief Create/init a queue instance.
 *
 * @details Creates a new queue for application/user.
 */
bool minirtos_Queue_Create(Queue_Descriptor_t *ptrq, void *buffer, uint16_t elementSize, uint16_t maxElements);

/**
 * @brief Enqueue data into queue.
 *
 * @details Copies an item (of size specified at queue creation) into the queue.
 *   This function can be safely called from both main code and interrupt context, as it enters a critical
 *   section (disables interrupts) to protect the internal data structure. Operation is quick and minimal for ISR safety.
 *
 */
bool minirtos_Queue_Send(Queue_Descriptor_t *ptrq, const void *ptrmsg);

/**
 * @brief Dequeue data from queue.
 *
 * @details  Copies an item out of the queue into user memory. Can be safely called from both
 *   main code and interrupt context, as it disables interrupts during the operation to prevent data races.
 *
 */
bool minirtos_Queue_Receive(Queue_Descriptor_t *ptrq, void *ptrmsg);

/**
 * @brief Get number of items currently in queue.
 *
 * @details No of elements available in the queue.
 */
uint16_t minirtos_Queue_Count(Queue_Descriptor_t *ptrq);

/**
 * @brief Flush (clear) all contents from the queue.
 *
 * @details Resets head, tail, and count so queue is empty.
 *   Protects structure with critical section for safe use from main/IRQ context.
 */
void minirtos_Queue_Flush(Queue_Descriptor_t *ptrq);

/**
 * @brief Entry function for the user to initiate scheduler.
 *
 * @details This function as to be called before doing anything with the tasker. It
 * initiates the tasker subsystems. If this funtion is not called before doing
 * anything with the tasker all functions will return false.
 *
 * @note Should be called once from `main()` or before scheduler to start application.
 */
void minirtos_Init(void);
/**
 * @brief Add the task in the scheduler.
 *
 * @details Add a task into the circular linked list for the scheduler.
 */

bool minirtos_AddTask(Task_Descriptor_t *ptrTaskDescriptor, void (*ptrUserTask)(void),
                    uint32_t taskInterval, Task_Status_e taskStatus);
/**
 * @brief Remove the task from the scheduler.
 *
 * @details This function is used to remove the task from the scheduler.
 */

bool minirtos_RemoveTask(Task_Descriptor_t *ptrUserTaskDescriptor);

/**
 * @brief Pause the task in the scheduler.
 *
 * @details This function is used to pause the task on the scheduler.
 */

bool minirtos_PauseTask(Task_Descriptor_t *ptrTaskDescriptor);

/**
 * @brief Resume the task in the scheduler.
 *
 * @details This function is used to restart a task that has been paused.
 */

bool minirtos_ResumeTask(Task_Descriptor_t *ptrTaskDescriptor);

/**
 * @brief Modify the task in the scheduler.
 *
 * @details This function is used to modify a task parameters.
 */

bool minirtos_ModifyTask(Task_Descriptor_t *ptrTaskDescriptor, uint32_t taskInterval, Task_Status_e taskStatus);

/**
 * @brief Get status of the tasks.
 *
 * @details Function to check if a task is running or paused etc.
 */

Task_Status_e minirtos_GetTaskStatus(Task_Descriptor_t *ptrTaskDescriptor);

/**
 * @brief Scheduler.
 *
 * @details Scheduling. This runs the kernel itself.
 */

void minirtos_Scheduler(void);
#ifdef __cplusplus
}
#endif

#endif /* MINIRTOS_H_ */
