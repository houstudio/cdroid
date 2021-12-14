// Copyright (C) Alan W. Irwin, 2016
// Copyright (C) Michael Kerrisk, 2016
//
// This program is free software. You may use, modify, and redistribute it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 or (at your option) any
// later version. This program is distributed without any warranty.  See
// the file COPYING.gpl-v3 for details.

// pshm_write.c

// Usage: pshm_write
// Copy stdin into a POSIX shared memory object under unnamed semaphore control.
// To be used in conjunction with pshm_read (which reads from that shared
// memory object under unnamed semaphore control and outputs the results to stdout).

#include <sys/types.h>  // Type definitions used by many programs
#include <stdio.h>      // Standard I/O functions
#include <stdlib.h>     // Prototypes of commonly used library functions,
                        // plus EXIT_SUCCESS and EXIT_FAILURE constants
#include <unistd.h>     // Prototypes for many system calls
#include <errno.h>      // Declares errno and defines error constants
#include <string.h>     // Commonly used string-handling functions
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include "pshm.h"

int
main(int argc, char *argv[])
{
    int fd;
    shmbuf *shmp;
    int xfrs, bytes;

    if (argc != 1)
      {
	fprintf(stderr, "%s should have no arguments because it only accepts input from stdin\n", argv[0]);
	exit(EXIT_FAILURE);
      }

    // Create new instance of shared memory with read-write permissions of just the current user.
    fd = shm_open(SHM_PATH, O_CREAT|O_EXCL|O_RDWR, S_IRUSR|S_IWUSR);
    if (fd == -1)
      {
	fprintf(stderr, "shm-open\n");
	exit(EXIT_FAILURE);
      }

    // Resize object to hold struct.
    if (ftruncate(fd, sizeof(shmbuf)) == -1)
      {
	fprintf(stderr, "ftruncate\n");
	exit(EXIT_FAILURE);
      }
    shmp = mmap(NULL, sizeof(shmbuf), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shmp == MAP_FAILED)
      {
	fprintf(stderr, "mmap\n");
	exit(EXIT_FAILURE);
      }

    // 'fd' is no longer needed
    if (close(fd) == -1)
      {
	fprintf(stderr, "close\n");
	exit(EXIT_FAILURE);
      }

    if (sem_init(&shmp->rsem, 1, 0) == -1)
      {
	fprintf(stderr, "sem_init\n");
	exit(EXIT_FAILURE);
      }

    // We (pshm_write) gets first turn
    if (sem_init(&shmp->wsem, 1, 1) == -1)
      {
	fprintf(stderr, "sem_init\n");
	exit(EXIT_FAILURE);
      }

    for (xfrs = 0, bytes = 0; ; xfrs++, bytes += shmp->cnt) {
      // Wait for our turn
      if(sem_wait(&shmp->wsem) == -1)
	{
	  fprintf(stderr, "sem_wait\n");
	  exit(EXIT_FAILURE);
	}

      shmp->cnt = read(STDIN_FILENO, shmp->buf, BUF_SIZE);
      if(shmp->cnt == -1)
	{
	  fprintf(stderr, "read\n");
	  exit(EXIT_FAILURE);
	}

      // Give pshm_read a turn
      if(sem_post(&shmp->rsem) == -1)
	{
	  fprintf(stderr, "read\n");
	  exit(EXIT_FAILURE);
	}

      // EOF on stdin?
      if (shmp->cnt == 0)
	break;

    }
    // Wait for pshm_read to finish
    if(sem_wait(&shmp->wsem) == -1)
      {
	fprintf(stderr, "sem_wait\n");
	exit(EXIT_FAILURE);
      }

    // Clean up
    if(shm_unlink(SHM_PATH) == -1)
      {
	fprintf(stderr, "shm_unlink\n");
	exit(EXIT_FAILURE);
      }

    exit(EXIT_SUCCESS);
}
