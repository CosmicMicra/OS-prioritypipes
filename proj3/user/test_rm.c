#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "proj3/user/user.h"

void periodic_task(int period) {
    setperiod(period);  
    
    for (int i = 0; i < 3; i++) {
        printf("Process %d executing task\n", getpid());
        printf("xv6timer_forward: current tick %ld, next tick set at %ld\n", getticks(), getticks() + period);
        wait_until_next_period();  
    }
    printf("Process %d finished\n", getpid());
}

int main() {
    printf("[DEBUG] Starting test with 4 periodic tasks\n");

    if (fork() == 0) {
        periodic_task(5); 
        exit(0);
    }
    
    if (fork() == 0) {
        periodic_task(10);
        exit(0);
    }
    
    if (fork() == 0) {
        periodic_task(15);
        exit(0);
    }
    
    if (fork() == 0) {
        periodic_task(20); 
        exit(0);
    }

    if (fork() == 0) {
        printf("[DEBUG] Attempting to add 5th periodic task\n");
        if (setperiod(25) == -1) {
            printf("[DEBUG] Failed to add 5th periodic task, task limit reached\n");
        }
        exit(0);
    }

    wait(0);
    wait(0);
    wait(0);
    wait(0);

    printf("Test complete\n");
    exit(0);
}