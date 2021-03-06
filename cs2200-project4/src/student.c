/*
 * student.c
 *
 * Multi-threaded OS Simulation for CS 2200, Project 4
 * Summer 2021
 *
 * This file contains the CPU scheduler for the simulation.
 *
 * Name:
 * GTID:
 */

#include "student.h"

#include <assert.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

/** Function prototypes **/
extern void idle(unsigned int cpu_id);
extern void preempt(unsigned int cpu_id);
extern void yield(unsigned int cpu_id);
extern void terminate(unsigned int cpu_id);
extern void wake_up(pcb_t *process);

/*
 * current[] is an array of pointers to the currently running processes.
 * There is one array element corresponding to each CPU in the simulation.
 *
 * current[] should be updated by schedule() each time a process is scheduled
 * on a CPU.  Since the current[] array is accessed by multiple threads, you
 * will need to use a mutex to protect it.  current_mutex has been provided
 * for your use.
 * 
 * rq is a pointer to a struct you should use for your ready queue
 * implementation. The head of the queue corresponds to the process
 * that is about to be scheduled onto the CPU, and the tail is for
 * convenience in the enqueue function. See student.h for the
 * relevant function and struct declarations.
 *
 * Similar to current[], rq is accessed by multiple threads,
 * so you will need to use a mutex to protect it. ready_mutex has been
 * provided for that purpose.
 *
 * The condition variable queue_not_empty has been provided for you
 * to use in conditional waits and signals.
 *
 * Please look up documentation on how to properly use pthread_mutex_t
 * and pthread_cond_t.
 *
 * A scheduler_algorithm variable and sched_algorithm_t enum have also been
 * supplied to you to keep track of your scheduler's current scheduling
 * algorithm. You should update this variable according to the program's
 * command-line arguments. Read student.h for the definitions of this type.
 */

static pcb_t **current;
static pthread_mutex_t current_mutex;

// Values that should be protected by the `queue_mutex` mutex.
// Whenever you try to access the following two variables, make sure you are
// doing so in a safe manner.
static queue_t *rq = NULL;
static int empty = 1;

static pthread_mutex_t queue_mutex;
static pthread_cond_t queue_not_empty;
static int timeslice = -1;
static sched_algorithm_t scheduler_algorithm = FCFS;
unsigned int cpu_count;

/** Scheduler type */

/*
 * rq_add() is a helper function to add a process to the ready queue.
 *
 * This function should add the process to the ready queue at the
 * appropriate location. Make sure to signal the proper conditions and
 * set `empty` to 0 whenever this function runs.
 *
 * NOTE: For Priority scheduling, you will need to have additional logic
 * in either this function or the rq_remove function to make the ready queue
 * a priority queue. Also, remember that a lower value in the priority field
 * means the process has a higher priority.
 */
static void rq_add(pcb_t *pcb)
{
    pthread_mutex_lock(&queue_mutex);
    pcb->next = NULL;
    pcb->state = PROCESS_READY;
    if (rq->head == NULL)
    {
        rq->head = pcb;
        rq->tail = pcb;
        empty = 0;
        pthread_cond_signal(&queue_not_empty);
    }
    else
    {
        rq->tail->next = pcb;
        rq->tail = rq->tail->next;
    }
    pthread_mutex_unlock(&queue_mutex);
}

/*
 * dequeue() is a helper function to remove a process to the ready queue.
 *
 * This function should remove the process at the head of the ready queue
 * and return a pointer to that process. If the queue is empty, simply return
 * NULL. If the queue becomes empty after removing an element, make sure to
 * set `empty` to 1.
 *
 * NOTE: For Priority scheduling, you will need to have additional logic
 * in either this function or the rq_add function to make the ready queue
 * a priority queue. Also, remember that a lower value in the priority field
 * means the process has a higher priority.
 */
static pcb_t *rq_remove()
{
    pthread_mutex_lock(&queue_mutex);
    pcb_t *return_val = rq->head;
    if (scheduler_algorithm == PS)
    {
        pcb_t *temp = rq->head;
        int lowest_priority = 1000;
        if (!temp)
        {
            empty = 1;
            pthread_mutex_unlock(&queue_mutex);
            return NULL;
        }
        while (temp)
        {

            if (temp->priority < lowest_priority)
            {
                lowest_priority = temp->priority;
            }
            temp = temp->next;
        }
        if (rq->head->priority == lowest_priority)
        {
            rq->head = rq->head->next;
            if (!rq->head)
            {
                rq->tail = NULL;
            }
        }

        else
        {
            while (return_val->next)
            {
                if (return_val->next->priority == lowest_priority)
                {
                    temp = return_val->next;
                    return_val->next = return_val->next->next;
                    if (return_val->next == NULL)
                    {
                        rq->tail = return_val;
                    }
                    pthread_mutex_unlock(&queue_mutex);
                    return temp;
                }
                else
                {
                    return_val = return_val->next;
                }
            }
        }
        pthread_mutex_unlock(&queue_mutex);
        return return_val;
    }
    else
    {

        if (return_val == NULL)
        {
            empty = 1;
        }
        else if (return_val->next == NULL)
        {
            rq->head = NULL;
            rq->tail = NULL;
            empty = 1;
        }
        else
        {
            rq->head = rq->head->next;
        }
        pthread_mutex_unlock(&queue_mutex);
    }
    return return_val;
}

/*
 * schedule() is your CPU scheduler.  It should perform the following tasks:
 *
 *   1. Select and remove a runnable process from your ready queue which
 *    you will have to implement with a linked list or something of the sort.
 *
 *   2. Set the process state to RUNNING
 *
 *   3. Set the currently running process using the current array
 *
 *   4. Call context_switch(), to tell the simulator which process to execute
 *      next on the CPU.  If no process is runnable, call context_switch()
 *      with a pointer to NULL to select the idle process.
 *
 *    The current array (see above) is how you access the currently running
 * process indexed by the cpu id. See above for full description.
 *    context_switch() is prototyped in os-sim.h. Look there for more
 * information about it and its parameters.
 */
static void schedule(unsigned int cpu_id)
{
    pcb_t *proc = rq_remove();
    if (proc)
    {
        proc->state = PROCESS_RUNNING;
        pthread_mutex_lock(&current_mutex);
        current[cpu_id] = proc;
        context_switch(cpu_id, proc, timeslice);
        pthread_mutex_unlock(&current_mutex);
    }
    else
    {
        pthread_mutex_lock(&current_mutex);
        context_switch(cpu_id, proc, timeslice);
        pthread_mutex_unlock(&current_mutex);
    }
}

/*
 * idle() is your idle process.  It is called by the simulator when the idle
 * process is scheduled.
 *
 * This function should block until a process is added to your ready queue.
 * It should then call schedule() to select the process to run on the CPU.
 */
extern void idle(unsigned int cpu_id)
{
    pthread_mutex_lock(&queue_mutex);
    while (rq->head == NULL)
    {
        pthread_cond_wait(&queue_not_empty, &queue_mutex);
    }
    pthread_mutex_unlock(&queue_mutex);
    schedule(cpu_id);
}

/*
 * preempt() is the handler called by the simulator when a process is
 * preempted due to its timeslice expiring.
 *
 * This function should place the currently running process back in the
 * ready queue, and call schedule() to select a new runnable process.
 *
 * Remember to set the status of the process to the proper value.
 */
extern void preempt(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    current[cpu_id]->state = PROCESS_READY;
    rq_add(current[cpu_id]);
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}

/*
 * yield() is the handler called by the simulator when a process yields the
 * CPU to perform an I/O request.
 *
 * It should mark the process as WAITING, then call schedule() to select
 * a new process for the CPU.
 */
extern void yield(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    current[cpu_id]->state = PROCESS_WAITING;
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}

/*
 * terminate() is the handler called by the simulator when a process completes.
 * It should mark the process as terminated, then call schedule() to select
 * a new process for the CPU.
 */
extern void terminate(unsigned int cpu_id)
{
    pthread_mutex_lock(&current_mutex);
    current[cpu_id]->state = PROCESS_TERMINATED;
    pthread_mutex_unlock(&current_mutex);
    schedule(cpu_id);
}

/*
 * wake_up() is the handler called by the simulator when a process's I/O
 * request completes.  It should perform the following tasks:
 *
 *   1. Mark the process as READY, and insert it into the ready queue.
 *
 */
extern void wake_up(pcb_t *process)
{
    process->state = PROCESS_READY;
    rq_add(process);
}

/*
 * main() simply parses command line arguments, then calls start_simulator().
 * You will need to modify it to support the -r and -s command-line parameters.
 */
int main(int argc, char *argv[])
{
    scheduler_algorithm = FCFS;
    if (strncmp(*argv + 11, "-p", 2) == 0)
    {
        scheduler_algorithm = PS;
    }
    else if (strncmp(*argv + 11, "-r", 2) == 0)
    {
        timeslice = ((int)*argv[3] - 48);
    }
    unsigned int cpu_count;

    unsigned int showUsageAndExit = 0;

    /*
     * FIX ME - Add support for -r and -p parameters.
     *
     * Update scheduler_algorithm (see student.h) from the program arguments. If
     * no argument has been supplied, you should default to FCFS.  Use the
     * scheduler_algorithm variable in your scheduler to keep track of the
     * scheduling algorithm you're using.
     *
     * Your main function should check for invalid command line inputs. Your
     * program should print the usage lines and exit if it detected an invalid
     * set of arguments.
     */
    // Update this flag according to the arguments
    showUsageAndExit =
        1; // Set this flag as you parse through the arguments

    /* Print the usage string and exit if we don't have a valid set of
     * arguments. */

    if (!showUsageAndExit)
    {
        fprintf(stderr,
                "CS 2200 Project 4 -- Multithreaded OS Simulator\n"
                "Usage: ./os-sim <# CPUs> [ -r <time slice> | -s ]\n"
                "    Default : FCFS Scheduler\n"
                "         -r : Round-Robin Scheduler\n"
                "         -p : Priority Scheduler\n\n");
        return -1;
    }
    /* Parse the CPU count from command line arguments */
    cpu_count = strtoul(argv[1], NULL, 0);

    /* Allocate the eq ready queue */
    rq = (queue_t *)malloc(sizeof(queue_t));
    assert(rq != NULL);

    pthread_mutex_init(&queue_mutex, NULL);
    pthread_cond_init(&queue_not_empty, NULL);

    /* Allocate the current[] array and its mutex */
    current = malloc(sizeof(pcb_t *) * cpu_count);
    assert(current != NULL);

    pthread_mutex_init(&current_mutex, NULL);

    /* Start the simulator in the library */
    start_simulator(cpu_count);
    return 0;
}

#pragma GCC diagnostic pop
