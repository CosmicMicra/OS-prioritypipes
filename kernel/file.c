#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "fs.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "file.h"
#include "stat.h"
#include "proc.h"
#include "pipe_rt.h"
#include "fcntl.h"

struct devsw devsw[NDEV];
struct {
  struct spinlock lock;
  struct file file[NFILE];
} ftable;

void
fileinit(void)
{
  initlock(&ftable.lock, "ftable");
}

int
fdalloc(struct file *f)
{
  int fd;
  struct proc *p = myproc();

  for(fd = 0; fd < NOFILE; fd++){
    if(p->ofile[fd] == 0){
      p->ofile[fd] = f;
      return fd;
    }
  }
  return -1;
}

// Allocate a file structure.
struct file*
filealloc(void)
{
  struct file *f;

  acquire(&ftable.lock);
  for(f = ftable.file; f < ftable.file + NFILE; f++){
    if(f->ref == 0){
      f->ref = 1;
      release(&ftable.lock);
      return f;
    }
  }
  release(&ftable.lock);
  return 0;
}

// Increment ref count for file f.
struct file*
filedup(struct file *f)
{
  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("filedup");
  f->ref++;
  release(&ftable.lock);
  return f;
}

// Close file f.  (Decrement ref count, close when reaches 0.)
void
fileclose(struct file *f)
{
  struct file ff;

  acquire(&ftable.lock);
  if(f->ref < 1)
    panic("fileclose");
  if(--f->ref > 0){
    release(&ftable.lock);
    return;
  }
  ff = *f;
  f->ref = 0;
  f->type = FD_NONE;
  release(&ftable.lock);

  if(ff.type == FD_PIPE){
    pipeclose(ff.pipe, ff.writable);
  } else if(ff.type == FD_PIPE_RT){                   
    pipe_rt_close((struct pipe_rt*)ff.pipe, ff.writable);  
  } else if(ff.type == FD_INODE || ff.type == FD_DEVICE){
    begin_op();
    iput(ff.ip);
    end_op();
  }
}

// Get metadata about file f.
// addr is a user virtual address, pointing to a struct stat.
int
filestat(struct file *f, uint64 addr)
{
  struct proc *p = myproc();
  struct stat st;
  
  if(f->type == FD_INODE || f->type == FD_DEVICE){
    ilock(f->ip);
    stati(f->ip, &st);
    iunlock(f->ip);
    if(copyout(p->pagetable, addr, (char *)&st, sizeof(st)) < 0)
      return -1;
    return 0;
  }
  return -1;
}

// Read from file f.
// addr is a user virtual address.
int
fileread(struct file *f, uint64 addr, int n)
{
  int r = 0;

  if(f->readable == 0)
    return -1;

  switch(f->type) {
    case FD_PIPE:
      r = piperead(f->pipe, addr, n);
      break;
    case FD_PIPE_RT:
      r = pipe_rt_read((struct pipe_rt*)f->pipe, addr, n);  
      break;
    case FD_INODE:
      ilock(f->ip);
      if((r = readi(f->ip, 1, addr, f->off, n)) > 0)
        f->off += r;
      iunlock(f->ip);
      break;
    case FD_DEVICE:
      if(f->major < 0 || f->major >= NDEV || !devsw[f->major].read)
        return -1;
      r = devsw[f->major].read(1, addr, n);
      break;
    default:
      panic("fileread");
  }

  return r;
}

// Write to file f.
// addr is a user virtual address.
int filewrite(struct file *f, uint64 addr, int n) {
    int r, ret = 0;

    if (f->writable == 0) {
        return -1;
    }

    switch (f->type) {
        case FD_PIPE:
            ret = pipewrite(f->pipe, addr, n);
            break;

        case FD_PIPE_RT:
            ret = pipe_rt_write((struct pipe_rt *)f->pipe, addr, n);
            break;

        case FD_INODE:
            begin_op();
            ilock(f->ip);

            // Ensure offset is set correctly when O_APPEND is used
            if (f->writable & O_APPEND) {
                f->off = f->ip->size;
            }

            int max = ((MAXOPBLOCKS - 1 - 1 - 2) / 2) * BSIZE;
            int i = 0;

            while (i < n) {
                int n1 = n - i;
                if (n1 > max)
                    n1 = max;

                if ((r = writei(f->ip, 1, addr + i, f->off, n1)) > 0)
                    f->off += r; // Increment offset

                if (r < 0) {
                    break;
                }

                i += r;
            }

            iunlock(f->ip);
            end_op();
            ret = (i == n) ? n : -1;
            break;

        case FD_DEVICE:
            if (f->major < 0 || f->major >= NDEV || !devsw[f->major].write) {
                return -1;
            }
            ret = devsw[f->major].write(1, addr, n);
            break;

        default:
            return -1;
    }

    return ret;
}
