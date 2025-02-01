#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

typedef struct task_t {
    int priority;
    int x;
    int y;
    char op;      // Supports "+", "-", "*", "/"
    int *result;  // Stores the computation result
    int *error;   // Returns error if the input is invalid
} task_t;

int calc(int x, int y, char op, int *result) {
    switch(op) {
        case '+': *result = x + y; return 0;
        case '-': *result = x - y; return 0;
        case '*': *result = x * y; return 0;
        case '/': 
            if(y == 0) return -1;
            *result = x / y; 
            return 0;
        default: return -1;
    }
}

void server(int read_fd, int write_fd) {
    task_t task;
    int result, error;
    
    while(read(read_fd, &task, sizeof(task_t)) > 0) {
        error = calc(task.x, task.y, task.op, &result);
        write(write_fd, &result, sizeof(result));
        write(write_fd, &error, sizeof(error));
    }
    exit(0);
}

void client(int write_fd, int read_fd, task_t task) {
    int result, error;
    
    write(write_fd, &task, sizeof(task_t));
    read(read_fd, &result, sizeof(result));
    read(read_fd, &error, sizeof(error));
    
    printf("Priority %d task: %d %c %d = %d (error: %d)\n", 
           task.priority, task.x, task.op, task.y, result, error);
    
    exit(0);
}

int main() {
    int p1[2], p2[2];  // p1: client->server (priority pipe), p2: server->client (regular pipe)
    task_t tasks[] = {
        {1, 10, 5, '+', 0, 0},   // Low priority
        {3, 20, 4, '*', 0, 0},   // High priority
        {2, 15, 3, '/', 0, 0}    // Medium priority
    };
    int n_tasks = sizeof(tasks)/sizeof(task_t);
    
    printf("Starting priority pipe test...\n");
    
    // Create pipes: p1 is priority pipe (pipe_rt), p2 is regular pipe
    if(pipe_rt(p1) < 0 || pipe(p2) < 0) {
        printf("Pipe creation failed\n");
        exit(1);
    }
    
    // Create server first
    if(fork() == 0) {
        close(p1[1]);
        close(p2[0]);
        server(p1[0], p2[1]);
    }
    
    sleep(10);  // Let server start
    
    // Create clients
    for(int i = 0; i < n_tasks; i++) {
        if(fork() == 0) {
            close(p1[0]);
            close(p2[1]);
            client(p1[1], p2[0], tasks[i]);
        }
        sleep(1);  // Space out client creation
    }
    
    // Parent closes all pipe ends
    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);
    
    // Wait for all children
    for(int i = 0; i < n_tasks + 1; i++) {
        wait(0);
    }
    
    printf("Test completed.\n");
    exit(0);
}