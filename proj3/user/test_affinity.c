// user/test_affinity.c
#include "user/user.h"

int main() {
    int cpuid = 0;
    set_cpu_affinity(1 << cpuid);  // Pin process to CPU 0
    while (1) {
        printf("Running on CPU %d\n", cpuid);
        sleep(20);
    }
    exit(0);
}