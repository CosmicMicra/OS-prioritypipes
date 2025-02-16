#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main(void) {
    printf("Invoking show_vm_areas system call...\n");
    if (show_vm_areas() < 0) {
        printf("Error: Failed to execute show_vm_areas\n");
    }
    exit(0);
}
