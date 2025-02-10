// proj3/user/periodic_test.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

// Function to simulate a periodic task
void periodic_task(int period, int iterations) {
    if (setperiod(period) < 0) {
        printf("Error: setperiod failed for period %d\n", period);
        exit(1);
    }

    for (int i = 0; i < iterations; i++) {
        // Print task information to verify correct timing
        printf("Task with period %d: iteration %d\n", period, i);

        // Simulate some work
        for (volatile int j = 0; j < 1000000; j++);
        
        // Wait for the next period (this should put the task in a sleeping state)
        if (wait_until_next_period() < 0) {
            printf("Error: wait_until_next_period failed for period %d\n", period);
            exit(1);
        }
    }
    exit(0);
}

int main(int argc, char *argv[]) {
    // Test CPU affinity
    printf("Setting CPU affinity to CPU 0\n");
    if (set_cpu_affinity(1) < 0) { // 1 = CPU 0
        printf("Error: set_cpu_affinity failed\n");
        exit(1);
    }

    // Create multiple periodic tasks with different periods
    if (fork() == 0) {
        periodic_task(10, 5);  // Task 1: period 10 ticks, 5 iterations
    }

    if (fork() == 0) {
        periodic_task(20, 3);  // Task 2: period 20 ticks, 3 iterations
    }

    // Wait for children to complete
    for (int i = 0; i < 2; i++) {
        wait(0);
    }

    printf("All tasks completed\n");
    exit(0);
}
