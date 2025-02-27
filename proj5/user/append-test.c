#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fcntl.h"

int main() {
    // Open the file for writing, creating it if it doesn't exist, and appending
    int fd = open("testfile.txt", O_WRONLY | O_CREATE | O_APPEND);
    if (fd < 0) {
        printf("Failed to open file\n");
        exit(1);
    }

    // Write to the file
    if (write(fd, "Hello\n", 6) != 6) {
        printf("Failed to write to file\n");
        exit(1);
    }
    close(fd);

    // Open the file again for appending
    fd = open("testfile.txt", O_WRONLY | O_APPEND);
    if (fd < 0) {
        printf("Failed to open file\n");
        exit(1);
    }

    // Write again to the file
    if (write(fd, "World\n", 6) != 6) {
        printf("Failed to write to file\n");
        exit(1);
    }
    close(fd);
    
    exit(0);
}
