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
    switch(op) {
        case '+':
            *result = x + y;
            break;
        case '-':
            *result = x - y;
            break;
        case '*':
            *result = x * y;
            break;
        case '/':
            if(y == 0)
                return -1;
            *result = x / y;
            break;
        default:
            return -1;
    }
    return 0;
}

void server(int read_fd, int write_fd) {
    task_t task;
    
    printf("Server started.\n");
    
    while(read(read_fd, &task, sizeof(task_t)) > 0) {
        task.error = calc(task.x, task.y, task.op, &task.result);
        write(write_fd, &task, sizeof(task_t));
        sleep(1);
    }
    exit(0);
}

void client(int write_fd, int read_fd, int x, int y, char op) {
    task_t task;
    
   
    task.x = x;
    task.y = y;
    task.op = op;

    // Send task to server
    write(write_fd, &task, sizeof(task_t));
    
    // Get result back
    read(read_fd, &task, sizeof(task_t));
    
    // Print result
    if(task.error == 0) {
        printf("Result: %d ", task.x);
        write(1, &task.op, 1);
        printf(" %d = %d\n", task.y, task.result);
    } else {
        printf("division by zero- invalid\n");
    }
    
    exit(0);
}

int main(int argc, char *argv[]) {
    int p1[2], p2[2]; 

    printf("Starting test...\n");

    if(pipe(p1) < 0 || pipe(p2) < 0) {
        printf("Pipe creation failed\n");
        exit(1);
    }

    // Create server process
    if(fork() == 0) {
        close(p1[1]);
        close(p2[0]); 
        server(p1[0], p2[1]);
    }

    sleep(1); // Let server start

    // Create three client processes with different operations
    if(fork() == 0) {
        close(p1[0]);
        close(p2[1]);
        client(p1[1], p2[0], 10, 5, '+');
    }
    sleep(1);

    if(fork() == 0) {
        close(p1[0]);
        close(p2[1]);
        client(p1[1], p2[0], 20, 4, '*');
    }
    sleep(1);

    if(fork() == 0) {
        close(p1[0]); 
        close(p2[1]);
        client(p1[1], p2[0], 15, 3, '/');
    }
    sleep(1);
    
    // Test division by zero
    if(fork() == 0) {
        close(p1[0]); 
        close(p2[1]);
        client(p1[1], p2[0], 10, 0, '/');
    }
    sleep(1);


    // Parent closes all pipe ends
    close(p1[0]);
    close(p1[1]);
    close(p2[0]);
    close(p2[1]);

    // Wait for all children to finish
    for(int i = 0; i < 5; i++) {
        wait(0);
    }

    printf("Test completed.\n");
    exit(0);
}