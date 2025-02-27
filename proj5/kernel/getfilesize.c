#include "kernel/types.h"
#include "kernel/riscv.h"
#include "kernel/defs.h"
#include "kernel/param.h"
#include "kernel/stat.h"
#include "kernel/spinlock.h"
#include "kernel/proc.h"
#include "kernel/fs.h"
#include "kernel/sleeplock.h"
#include "kernel/file.h"
#include "kernel/fcntl.h"

uint64 sys_getfilesize(void) {
    char path[MAXPATH];
    int size; // Variable to hold the size of the file
    int size_ptr; // Variable to hold the user space pointer

    // Get the file path from user space
    if (argstr(1, path, MAXPATH) < 0) {
        return -1; // Invalid arguments
    }

    // Get the user space pointer to store the size
    argint(0, &size_ptr); // Store the pointer to where the size will be written

    struct inode *ip;
    if ((ip = namei(path)) == 0) {
        return -1; // File not found
    }

    ilock(ip); // Lock the inode

    // Check if the inode type is a regular file
    if (ip->type != T_FILE) {
        iunlockput(ip); // Unlock and put back the inode
        return -1; // Not a regular file
    }

    // Get the size from the inode
    size = ip->size; 

    // Copy the size to the user space pointer passed from the user
    if (copyout(myproc()->pagetable, (uint64)size_ptr, (char*)&size, sizeof(int)) < 0) {
        iunlockput(ip); // Unlock and put back the inode
        return -1; // Failed to copy size to user space
    }

    iunlockput(ip); // Unlock and put back the inode
    return 0; // Success
}
