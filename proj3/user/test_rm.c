// proj3/user/periodic_test.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"


void periodic_task(int period, int task_id) {
    if (setperiod(period) < 0) {
        printf("Task %d: Failed to set period %d\n", task_id, period);
        exit(0);
    }

    while (1) {
        int tick = uptime();  // Get current system time in ticks
        printf("[Tick %d] Task %d (Period %d): ", tick, task_id, period);

        // Print some work output
        for (int i = 0; i < 10; i++) {
            printf("1");
        }
        printf("\n");

        // Wait until the next scheduled period
        wait_until_next_period();
    }
}

int main() {
    int num_tasks = 3;
    int periods[] = {1,5,10};  // Different periods
    int task_ids[] = {1,2,3};   // Assign unique task IDs

    if(num_tasks < 5) {
        for (int i = 0; i < num_tasks; i++) {
            if (fork() == 0) {  // Child process
                periodic_task(periods[i], task_ids[i]);
                exit(0);
            }
        }
    }else{
        printf("Too many tasks running. Error code: -1\n");
        return -1;
    }

    // Wait for all tasks (though they run indefinitely)
    for (int i = 0; i < num_tasks; i++) {
        wait(0);
    }

    exit(0);
}