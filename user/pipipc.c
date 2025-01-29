#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

typedef struct task_t {
    int x;
    int y;
    char op; 
    int result;
    int error;
} task_t;

int calc(int x, int y, char op, int *result) {
    switch (op) {
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
        if (y == 0) {
            return -1; 
        }
        *result = x / y;
        return 0; 
    default:
        return -1; 
    }
}

void server(int read_fd, int write_fd) {
    task_t task;
    while (read(read_fd, &task, sizeof(task_t)) > 0) {
        task.error = calc(task.x, task.y, task.op, &task.result);
        write(write_fd, &task, sizeof(task_t));
    }
    exit(0);
}

void client(int write_fd, int read_fd, task_t task) {
    write(write_fd, &task, sizeof(task_t));
    read(read_fd, &task, sizeof(task_t));
    if (task.error == 0) {
        printf("Result: %d %c %d = %d\n", task.x, task.op, task.y, task.result);
    } else if (task.error == -1) {
        printf("Error: Division by zero.\n");
    } else {
        printf("Error: Unsupported operation '%c'.\n", task.op);
    }
    exit(0);
}

int main() {
    int p1[2], p2[2];
    pipe(p1); 
    pipe(p2); 

    int num_clients = 3; 
    for (int i = 0; i < num_clients; i++) {
        if (fork() == 0) { 
            task_t task;
            task.x = i + 1;
            task.y = i + 2;
            task.op = (i % 2 == 0) ? '+' : '*'; 
            client(p1[1], p2[0], task);
        }
    }

    
    close(p1[1]); 
    close(p2[0]); 

    
    server(p1[0], p2[1]);

    
    for (int i = 0; i < num_clients; i++) {
        wait(0);
    }

    exit(0);
}
