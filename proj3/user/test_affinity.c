// user/test_affinity.c
#include "user/user.h"

int main() {
    int cpuid = 0;
    set_cpu_affinity(1 << cpuid);  // Pin process to CPU 0
    for(int i = 0; i < 5; i++) {
        printf("Running on CPU %d\n", cpuid);
        sleep(20);
    }

    printf("\n=== Test Complete ===\n");
    exit(0);
}