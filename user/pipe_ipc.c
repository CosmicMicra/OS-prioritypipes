// proj2/user/ipc_test.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NUM_CLIENTS 3
#define BUFFER_SIZE sizeof(task_t)

typedef struct task_t {
    int priority;  // Added for Part 2
    int x;
    int y;
    char op;
    int result;
    int error;
} task_t;

// Calculate function similar to HW1
int calc(int x, int y, char op, int *result) {
    switch(op) {
        case '+':
            *result = x + y;
            return 0;
        case '-':
            *result = x - y;
            return 0;
        case '*':
            *result = x * y;
            return 0;
        case '/':
            if (y == 0) return -1;
            *result = x / y;
            return 0;
        default:
            return -1;
    }
}

void server(int read_fd, int write_fd) {
    task_t task;
    while (1) {
        // Read task from pipe
        if (read(read_fd, &task, BUFFER_SIZE) != BUFFER_SIZE) {
            break;
        }

        // Process task
        task.error = calc(task.x, task.y, task.op, &task.result);

        // Write result back
        write(write_fd, &task, BUFFER_SIZE);
    }
}

void client(int write_fd, int read_fd, task_t task) {
    // Send task
    write(write_fd, &task, BUFFER_SIZE);

    // Read result
    read(read_fd, &task, BUFFER_SIZE);

    if (task.error == 0) {
        printf("Result for %d %c %d = %d (priority: %d)\n", 
               task.x, task.op, task.y, task.result, task.priority);
    } else {
        printf("Error in calculation (priority: %d)\n", task.priority);
    }
}

int main() {
    int p1[2], p2[2];  // Two pipes: p1 for requests, p2 for responses

    // Create pipes
    if (pipe(p1) < 0 || pipe(p2) < 0) {
        printf("Pipe creation failed\n");
        exit(1);
    }

    // Create clients
    for (int i = 0; i < NUM_CLIENTS; i++) {
        if (fork() == 0) {  // Child process (client)
            close(p1[0]);  // Close unused pipe ends
            close(p2[1]);

            // Create sample task
            task_t task = {
                .priority = i + 1,  // Different priorities
                .x = i + 5,
                .y = i + 1,
                .op = "+-*/"[i % 4],
                .result = 0,
                .error = 0
            };

            client(p1[1], p2[0], task);
            exit(0);
        }
    }

    // Parent process (server)
    close(p1[1]);  // Close unused pipe ends
    close(p2[0]);

    server(p1[0], p2[1]);

    // Wait for all children
    for (int i = 0; i < NUM_CLIENTS; i++) {
        wait(0);
    }

    exit(0);
}