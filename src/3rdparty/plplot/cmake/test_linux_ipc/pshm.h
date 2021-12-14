#ifndef __PSHM_H__
#define __PSHM_H__

// Name for shared memory that is opened by shm_open.  Should start
// with "/" according to shm_open man page.
#define SHM_PATH "/test_linux_ipc_shared_memory"
#define BUF_SIZE 1024
typedef struct {       // Buffer in shared memory
  sem_t wsem;         // Writer semaphore
  sem_t rsem;         // Reader semaphore
  int cnt;            // Number of bytes used in 'buf'
  char buf[BUF_SIZE]; // Data being transferred
} shmbuf;
#endif
