#include "kernel/types.h"
#include "kernel/stat.h" 
#include "user/user.h"

typedef struct task_t {
    int x;
    int y;
    char op;
    int priority;  // Used for slot allocation
    int client_id; // Added to uniquely identify clients
} task_t;

typedef struct result_t {
    int value;
    int error;
    int ready;     // Flag to indicate result is ready
} result_t;

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

void server(int read_fd) {
    task_t task;
    result_t *results;
    
    // Allocate shared memory for results
    results = (result_t*)shmget(0, 4096);
    if(results == (result_t*)-1) {
        printf("Server: Shared memory allocation failed\n");
        exit(1);
    }
    
    // Initialize all slots as not ready
    for(int i = 0; i < 10; i++) {
        results[i].ready = 0;
    }
    
    printf("Server started.\n");
    
    while(read(read_fd, &task, sizeof(task_t)) > 0) {
        int result = 0;
        int error = calc(task.x, task.y, task.op, &result);
        
        // Use client_id to determine slot (limited to 10 clients)
        int slot = task.client_id % 10;
        
        // Store result in shared memory
        results[slot].value = result;
        results[slot].error = error;
        
        // Mark the result as ready (this is the synchronization signal)
        results[slot].ready = 1;
    }
    
    close(read_fd);
    exit(0);
}

void client(int write_fd, int client_id, int x, int y, char op) {
    task_t task;
    result_t *results;
    
    // Allocate shared memory
    results = (result_t*)shmget(0, 4096);
    if(results == (result_t*)-1) {
        printf("Client %d: Shared memory allocation failed\n", client_id);
        exit(1);
    }
   
    task.x = x;
    task.y = y;
    task.op = op;
    task.priority = client_id; // Using client_id as priority for simplicity
    task.client_id = client_id;
    
    // Clear the ready flag before sending request
    int slot = client_id % 10;
    results[slot].ready = 0;
    
    // Send task to server
    if(write(write_fd, &task, sizeof(task_t)) != sizeof(task_t)) {
        printf("Client %d: Failed to send task\n", client_id);
        close(write_fd);
        exit(1);
    }
    
    // Wait for result to be ready (simple polling)
    while(results[slot].ready == 0) {
        sleep(1);
    }
    
    // Retrieve result from shared memory
    int result = results[slot].value;
    int error = results[slot].error;
    
    // Print result
    if(error == 0) {
        printf("Client %d: %d ", client_id, x);
        write(1, &op, 1); // Write the character directly
        printf(" %d = %d\n", y, result);
    } else {
        printf("Client %d: Error in calculation\n", client_id);
    }
    
    close(write_fd);
    exit(0);
}

int main(int argc, char *argv[]) {
    int p1[2]; 
    printf("Starting shared memory IPC test...\n");
    
    if(pipe(p1) < 0) {
        printf("Pipe creation failed\n");
        exit(1);
    }
    
    // Create server process
    if(fork() == 0) {
        close(p1[1]);
        server(p1[0]);
    }
    
    sleep(1); // Let server start
    
    // Create client processes with different operations
    for(int i = 0; i < 5; i++) {
        if(fork() == 0) {
            close(p1[0]);
            
            // Different operations for different clients
            switch(i) {
                case 0: client(p1[1], i, 10, 5, '+'); break;
                case 1: client(p1[1], i, 20, 4, '*'); break;
                case 2: client(p1[1], i, 15, 3, '/'); break;
                case 3: client(p1[1], i, 10, 0, '/'); break;
                case 4: client(p1[1], i, 25, 5, '-'); break;
            }
        }
    }
    
    // Parent closes pipe ends
    close(p1[0]);
    close(p1[1]);
    
    // Wait for all children to finish
    for(int i = 0; i < 6; i++) { // 1 server + 5 clients
        wait(0);
    }
    
    printf("Shared memory IPC test completed.\n");
    exit(0);
}