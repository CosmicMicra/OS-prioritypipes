#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
    printf("# Before shmget is called\n");
    if (show_vm_areas() < 0) {
        printf("Error: Failed to execute show_vm_areas\n");
    }

    // Call shmget and store the address
    void *shm = (void *)shmget(0, 4096);
    if (shm == (void *)-1) {
        printf("Error: shmget failed\n");
        exit(1);
    }

    printf("# After shmget is called\n");
    if (show_vm_areas() < 0) {
        printf("Error: Failed to execute show_vm_areas\n");
    }
    exit(0);
}
