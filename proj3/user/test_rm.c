// proj3/user/periodic_test.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Periodic task function
void periodic_task(int period, int task_id) {
    if (setperiod(period) < 0) {
        printf("Task %d: Failed to set period %d\n", task_id, period);
        exit(0);
    }

    while (1) {
        int tick = uptime();  // Get current system time in ticks
        
        // Use a single printf to prevent garbled output
        printf("[Tick %d] Task %d (Period %d): 1111111111\n", tick, task_id, period);
        
        // Wait for the next scheduled period
        wait_until_next_period();  // Ensure that the kernel handles task synchronization here
    }
}

int main() {
    int num_tasks = 3;
    int periods[] = {5, 10, 15};  // Different periods for each task
    int task_ids[] = {1, 2, 3};   // Assign unique task IDs

    // Fork child processes to run the periodic tasks
    for (int i = 0; i < num_tasks; i++) {
        if (fork() == 0) {  // Child process
            periodic_task(periods[i], task_ids[i]);
            exit(0);  // Ensure the child process exits after completing the task
        }
    }

    // Parent process waits for all tasks (though they run indefinitely)
    for (int i = 0; i < num_tasks; i++) {
        wait(0);  // Ensure that the parent process waits for all child tasks
    }

    exit(0);
}
