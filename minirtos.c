/**
 * \file           minirtos.c
 * \brief          Scheduler source file
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
/*****************************************************************************/
/* Include Files                                                             */
/*****************************************************************************/
#include "minirtos.h"
/*****************************************************************************/
/* Private Variables                                                         */
/*****************************************************************************/
uint8_t glbInitialized; /* To verify whether scheduler is started by User or not */
uint8_t glbNumberOfTasks; /* Counter for the scheduler to get user defined tasks*/

Task_Descriptor_t *gptrTaskSchedule; /*Pointer to the next task that is scheduled to run*/
Task_Descriptor_t *gptrTaskFirst = NULL; /* Pointer to the first task to the scheduler*/

/*****************************************************************************
 * @brief Create/init a queue instance.
 *
 * @details Creates a new queue for application/user.
 *
 * @param ptrq   Descriptor of the queue.
 *
 * @param buffer User allocated queue space for operation
 *
 * @param elementSize Size of each element in the queue.
 *
 * @param maxElementrs Maximum elements allowed in particular queue
 *
 * @return True or False
 *
 * @see @Queue_Descriptor_t
 *****************************************************************************/
bool minirtos_Queue_Create(Queue_Descriptor_t *ptrq, void *buffer, uint16_t elementSize, uint16_t maxElements)
{
    if (!ptrq || !buffer || elementSize == 0 || maxElements == 0)
    	{
    		return false;
    	}

    ptrq->buffer = buffer;
    ptrq->elementSize = elementSize;
    if (maxElements > MAX_NO_OF_QUEUE_ELEMENTS)
    {
    	ptrq->maxElements = MAX_NO_OF_QUEUE_ELEMENTS;
    }
    else
    {
    	ptrq->maxElements = maxElements;
    }
    ptrq->head = 0;
    ptrq->tail = 0;
    ptrq->count = 0;

    return true;
}
/*****************************************************************************
 * @brief Enqueue data into queue.
 *
 * @details Copies an item (of size specified at queue creation) into the queue.
 *   This function can be safely called from both main code and interrupt context, as it enters a critical
 *   section (disables interrupts) to protect the internal data structure. Operation is quick and minimal for ISR safety.
 *
 * @param ptrq   Descriptor of the queue.
 *
 * @param ptrmsg User message in the given queue.
 *
 * @return True or False
 *
 * @see @Queue_Descriptor_t
 *****************************************************************************/
bool minirtos_Queue_Send(Queue_Descriptor_t *ptrq, const void *ptrmsg)
{
	MINIRTOS_QUEUE_ENTER_CRITICAL();
    if (ptrq->count == ptrq->maxElements)
    	{
    	    MINIRTOS_QUEUE_EXIT_CRITICAL();
    		return false; // Queue is full
    	}

    uint8_t *ptrbuf = (uint8_t *)ptrq->buffer;
    uint16_t idx = (ptrq->tail * ptrq->elementSize);    // Calculate where to write

    memcpy(&ptrbuf[idx], ptrmsg, ptrq->elementSize);       // Copy in the data

    ptrq->tail = (ptrq->tail + 1) % ptrq->maxElements;     // Increment/cycle tail
    ptrq->count++;                                // Increment item count
    MINIRTOS_QUEUE_EXIT_CRITICAL();
    return true;
}

/*****************************************************************************
 * @brief Dequeue data from queue.
 *
 * @details  Copies an item out of the queue into user memory. Can be safely called from both
 *   main code and interrupt context, as it disables interrupts during the operation to prevent data races.
 *
 * @param ptrq   Descriptor of the queue.
 *
 * @param ptrmsg User message removed from queue.
 *
 * @return True or False
 *
 * @see @Queue_Descriptor_t
 *****************************************************************************/
bool minirtos_Queue_Receive(Queue_Descriptor_t *ptrq, void *ptrmsg)
{
	MINIRTOS_QUEUE_ENTER_CRITICAL();
    if (ptrq->count == 0)
    	{
    		MINIRTOS_QUEUE_EXIT_CRITICAL();
    		return false;           // Queue is empty
    	}

    uint8_t *ptrbuf = (uint8_t *)ptrq->buffer;
    uint16_t idx = (ptrq->head * ptrq->elementSize);    // Where to read

    memcpy(ptrmsg, &ptrbuf[idx], ptrq->elementSize);       // Copy out the data

    ptrq->head = (ptrq->head + 1) % ptrq->maxElements;     // Move head forward
    ptrq->count--;                                // Decrement item count
    MINIRTOS_QUEUE_EXIT_CRITICAL();
    return true;
}

/*****************************************************************************
 * @brief Get number of items currently in queue.
 *
 * @details No of elements available in the queue.
 *
 * @param ptrq   Descriptor of the queue.
 *
 * @retval no of elements present at the movement.
 *
 * @see @Queue_Descriptor_t
 *****************************************************************************/
uint16_t minirtos_Queue_Count(Queue_Descriptor_t *ptrq)
{
    return ptrq->count;
}

/*****************************************************************************
 * @brief Flush (clear) all contents from the queue.
 *
 * @details Resets head, tail, and count so queue is empty.
 *   Protects structure with critical section for safe use from main/IRQ context.
 *
 * @param ptrq   Descriptor of the queue.
 *
 * @return
 *
 * @see @Queue_Descriptor_t
 *****************************************************************************/
void minirtos_Queue_Flush(Queue_Descriptor_t *ptrq)
{
	MINIRTOS_QUEUE_ENTER_CRITICAL();
	ptrq->head = 0;
	ptrq->tail = 0;
	ptrq->count = 0;
	MINIRTOS_QUEUE_EXIT_CRITICAL();
}
/*****************************************************************************
 * @brief Entry function for the user to initiate scheduler.
 *
 * @details This function as to be called before doing anything with the tasker. It
 * initiates the tasker subsystems. If this funtion is not called before doing
 * anything with the tasker all functions will return false.
 *
 * @param   None
 *
 * @return  None
 *
 * @retval  None
 *
 * @note Should be called once from `main()` or before scheduler to start application.
 *****************************************************************************/
void minirtos_Init(void)
{
	glbInitialized = true;
	glbSysTicks = 0;
	glbNumberOfTasks = 0;
	gptrTaskSchedule = NULL;
}
/*****************************************************************************
 * @brief Add the task in the scheduler.
 *
 * @details Add a task into the circular linked list for the scheduler.
 *
 * @param ptrTaskDescriptor   Descriptor of the task.
 *
 * @param ptrUserTask         Function pointer on the task body
 *
 * @param taskInterval Scheduled interval in milliseconds at which you want your
 *                     routine to be executed.
 *
 * @param taskStatus Status of the task to be added to the scheduler, status can
 *                   be:
 *                   PAUSE, for a task that doesn't have to start immediately;
 *                   SCHEDULED, for a normal task that has to start after its scheduling;
 *                   ONE_SHOT, for a task that has to run only once;
 *                   RUN_NOW, for a task that has to be executed once it
 *                   has been added to the scheduler;
 *                   ONE_SHOT_NOW, for a task that as to be executed now and it will only be
 *                   executed one time.
 *
 * @return True or False
 *
 * @see @Task_Status_e, @Task_Descriptor_t
 *****************************************************************************/
bool minirtos_AddTask(Task_Descriptor_t *ptrTaskDescriptor, void (*ptrUserTask)(void),
                    uint32_t taskInterval, Task_Status_e taskStatus)
{
	Task_Descriptor_t *ptrNewTask = (Task_Descriptor_t *)malloc(sizeof(Task_Descriptor_t));

    if ((glbInitialized == false) || (glbNumberOfTasks == MAX_TASKS_NUMBER) || (ptrUserTask == NULL))
    {
    	free(ptrNewTask);
        return false;
    }
    if ((taskInterval < 0) || (taskInterval > MAX_TASK_INTERVAL))
    {
        taskInterval = DEFAULT_TASK_INTERVAL;
    }

    /*check if taskStatus is valid, if not schedule*/
    switch(taskStatus) {
        case TASK_PAUSE:

            break;
        case TASK_SCHEDULED:

            break;
        case TASK_ONE_SHOT:

            break;
        case TASK_RUN_NOW:

            break;
        case TASK_ONE_SHOT_NOW:

            break;
        case TASK_NOT_FOUND:
        case TASK_RUNNING:
        default:
        	taskStatus = TASK_SCHEDULED;
            break;
    }
    if (ptrTaskDescriptor != NULL)
        {
            if (gptrTaskFirst != NULL)
            {
                /* Initialize the work pointer with the scheduler pointer */
                ptrNewTask = gptrTaskSchedule;
                /*As we need to implement circular Task list for our scheduler pass the first task pointer
                 * to newly added task to complete the circle*/
                while (ptrNewTask->gptrTaskNext != gptrTaskFirst)
                {
                    /* Set the work pointer on the next task*/
                	ptrNewTask = ptrNewTask->gptrTaskNext;
                }
                /* Replace the last task->next task pointer with new task at the end of the circular linked list*/
                ptrNewTask->gptrTaskNext = ptrTaskDescriptor;
                /* The new task is linked back at the first task*/
                ptrTaskDescriptor->gptrTaskNext = gptrTaskFirst;
            }
            else
            {
                /* There is no task in the scheduler, this task become the First task in the circular linked list*/
            	/* The gptrTaskFirst pointer is initialized with the New Task*/
            	gptrTaskFirst = ptrTaskDescriptor;
            	/* Initialize the scheduled task pointer at the first task*/
            	gptrTaskSchedule = gptrTaskFirst;
            	/* The next task is itself because there is just one task in the circular linked list*/
                ptrTaskDescriptor->gptrTaskNext = ptrTaskDescriptor;
            }


            /*Tasks with ONE SHOT or RUN NOW are planned immediately*/
            if(taskStatus == TASK_RUN_NOW || taskStatus == TASK_ONE_SHOT_NOW)
            {
            	ptrTaskDescriptor->plannedTask = glbSysTicks;
            }
            else
            {
            	ptrTaskDescriptor->taskStatus = taskStatus;
            	ptrTaskDescriptor->plannedTask = glbSysTicks + taskInterval;
            }

            // Set the period of the task
            ptrTaskDescriptor->taskInterval = taskInterval;
            // Set the task pointer on the task body
            ptrTaskDescriptor->taskPointer = ptrUserTask;

            glbNumberOfTasks++;


            return true;
        }
    free(ptrNewTask);
    return false;
}
/*****************************************************************************
 * @brief Remove the task from the scheduler.
 *
 * @details This function is used to remove the task from the scheduler.
 *
 * @param Task_Descriptor_t Descriptor of the task to be removed.
 *
 * @return Return true if all went well, false otherwise.
 *
 * @return True or False
 *
 * @see @Task_Descriptor_t
 *****************************************************************************/
bool minirtos_RemoveTask(Task_Descriptor_t *ptrUserTaskDescriptor)
{
	Task_Descriptor_t *ptrNewTask = (Task_Descriptor_t *)malloc(sizeof(Task_Descriptor_t));

    if ((glbInitialized == false) || (glbNumberOfTasks == MAX_TASKS_NUMBER) || (ptrUserTaskDescriptor == NULL))
    {
    	free(ptrNewTask);
        return false;
    }
    /* Initialize the work pointer with the scheduler pointer */
    ptrNewTask = gptrTaskFirst;

    while (ptrNewTask->gptrTaskNext != ptrUserTaskDescriptor)
    {
        /* Set the work pointer on the next task */
    	ptrNewTask = ptrNewTask->gptrTaskNext;
    }

    if (ptrNewTask->gptrTaskNext == gptrTaskFirst)
    {
    	gptrTaskFirst = ptrNewTask->gptrTaskNext->gptrTaskNext;
    }
    else
    {
    	ptrNewTask->gptrTaskNext = ptrUserTaskDescriptor->gptrTaskNext;
    }

    glbNumberOfTasks--;

    free(ptrNewTask);
    return true;
}
/*****************************************************************************
 * @brief Pause the task in the scheduler.
 *
 * @details This function is used to pause the task on the scheduler.
 *
 * @param Task_Descriptor_t Descriptor of the task to be removed.
 *
 * @return Return true if all went well, false otherwise.
 *
 * @return True or False
 *
 * @see @Task_Descriptor_t
 *****************************************************************************/
bool minirtos_PauseTask(Task_Descriptor_t *ptrTaskDescriptor)
{
    if ((glbInitialized == false) || (ptrTaskDescriptor == NULL))
    {
        return false;
    }

    ptrTaskDescriptor->taskStatus = TASK_PAUSE;

    return true;
}
/*****************************************************************************
 * @brief Resume the task in the scheduler.
 *
 * @details This function is used to restart a task that has been paused.
 *
 * @param Task_Descriptor_t Descriptor of the task to be removed.
 *
 * @return Return true if all went well, false otherwise.
 *
 * @return True or False
 *
 * @see @Task_Descriptor_t
 *****************************************************************************/
bool minirtos_ResumeTask(Task_Descriptor_t *ptrTaskDescriptor)
{
    if ((glbInitialized == false) || (ptrTaskDescriptor == NULL))
    {
        return false;
    }

    ptrTaskDescriptor->taskStatus = TASK_SCHEDULED;

    ptrTaskDescriptor->plannedTask = glbSysTicks + ptrTaskDescriptor->taskInterval;

    return true;
}
/*****************************************************************************
 * @brief Modify the task in the scheduler.
 *
 * @details This function is used to modify a task parameters.
 *
 * @param ptrTaskDescriptor   Descriptor of the task.
 *
 * @param ptrUserTask         Function pointer on the task body
 *
 * @param taskInterval Scheduled interval in milliseconds at which you want your
 *                     routine to be executed.
 *
 * @param taskStatus Status of the task to be added to the scheduler, status can
 *                   be:
 *                   PAUSE, for a task that doesn't have to start immediately;
 *                   SCHEDULED, for a normal task that has to start after its scheduling;
 *                   ONE_SHOT, for a task that has to run only once;
 *                   RUN_NOW, for a task that has to be executed once it
 *                   has been added to the scheduler;
 *                   ONE_SHOT_NOW, for a task that as to be executed now and it will only be
 *                   executed one time.
 *
 * @return True or False
 *
 * @see @Task_Status_e, @Task_Descriptor_t
 *****************************************************************************/
bool minirtos_ModifyTask(Task_Descriptor_t *ptrTaskDescriptor, uint32_t taskInterval, Task_Status_e taskStatus)
{
    if ((glbInitialized == false) || (ptrTaskDescriptor == NULL))
    {
        return false;
    }

    if (taskStatus > TASK_ONE_SHOT_NOW)
    {
        return false;
    }

    ptrTaskDescriptor->taskInterval = taskInterval;
    ptrTaskDescriptor->taskStatus = taskStatus;

    if (taskStatus == TASK_SCHEDULED || taskStatus == TASK_ONE_SHOT)
    {
    	ptrTaskDescriptor->plannedTask = glbSysTicks + taskInterval;
    }
    else
    {
    	ptrTaskDescriptor->plannedTask = 0;
    }

    return true;
}
/*****************************************************************************
 * @brief Get status of the tasks.
 *
 * @details Function to check if a task is running or paused etc.
 *
 * @param Task_Descriptor_t Descriptor of the task to be removed.
 *
 * @return taskStatus Status of the task to be added to the scheduler, status can
 *                   be:
 *                   PAUSE, for a task that doesn't have to start immediately;
 *                   SCHEDULED, for a normal task that has to start after its scheduling;
 *                   ONE_SHOT, for a task that has to run only once;
 *                   RUN_NOW, for a task that has to be executed once it
 *                   has been added to the scheduler;
 *                   ONE_SHOT_NOW, for a task that as to be executed now and it will only be
 *                   executed one time.
 *
 * @see @Task_Descriptor_t
 *****************************************************************************/
Task_Status_e minirtos_GetTaskStatus(Task_Descriptor_t *ptrTaskDescriptor)
{
    if ((glbInitialized == false) || (ptrTaskDescriptor == NULL))
    {
        return TASK_NOT_FOUND;
    }

    return ptrTaskDescriptor->taskStatus;
}
/*****************************************************************************
 * @brief Scheduler.
 *
 * @details Scheduling. This runs the kernel itself. control logic.
 *
 * @param   None
 *
 * @return  None
 *
 * @retval  None
 *
 * @note Should be called after system and peripheral initialization.
 *
 * @warning This function runs in an infinite loop. Make sure all critical
 *          initialization is done before calling it.
 *****************************************************************************/
void minirtos_Scheduler(void)
{
	while (1)
	   {
	       if (gptrTaskSchedule != NULL && glbNumberOfTasks != 0)
	       {
	           /*the task is running*/
	           if (gptrTaskSchedule->taskStatus > TASK_PAUSE)
	           {
	               /*this trick overrun the overflow of System ticks*/
	        	   intmax_t elapsedTime = (gptrTaskSchedule->plannedTask - glbSysTicks);
	               if (elapsedTime <= ZERO)
	               {
	                   if (gptrTaskSchedule->taskStatus & TASK_ONE_SHOT)
	                   {
	                	   gptrTaskSchedule->taskPointer(); //call the task
	                	   gptrTaskSchedule->taskStatus = TASK_PAUSE; //pause the task
	                   }
	                   else
	                   {
	                       /* let's schedule next start */
	                	   gptrTaskSchedule->plannedTask = glbSysTicks + gptrTaskSchedule->taskInterval;
	                	   /* call the task */
	                	   gptrTaskSchedule->taskPointer();
	                   }
	               }
	           }
	           /* If no task are added, the pointer is null */
	           if (gptrTaskSchedule != NULL)
	           {
	               /* Set the scheduler pointer on the next task */
	        	   gptrTaskSchedule = gptrTaskSchedule->gptrTaskNext;
	           }
	       }
	   }
}
/************************************END*****************************************/








