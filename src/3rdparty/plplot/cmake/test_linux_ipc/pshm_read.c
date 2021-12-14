// Copyright (C) Alan W. Irwin, 2016
// Copyright (C) Michael Kerrisk, 2016
//
// This program is free software. You may use, modify, and redistribute it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 or (at your option) any
// later version. This program is distributed without any warranty.  See
// the file COPYING.gpl-v3 for details.

// pshm_read.c

// Usage: pshm_read
// Read shared memory object created by pshm_write under unnamed
// semaphore control and dump the results to stdout.

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
#include <sys/stat.h>
#include "pshm.h"

int
main(int argc, char *argv[])
{
    int fd;
    shmbuf *shmp;
    int xfrs, bytes;

    if (argc != 1)
      {
	fprintf(stderr, "%s should have no arguments because it only writes output to stdout\n", argv[0]);
	exit(EXIT_FAILURE);
      }

    // Open existing shared object already created by pshm_write.
    fd = shm_open(SHM_PATH, O_RDWR, 0);
    if (fd == -1)
      {
	fprintf(stderr, "shm-open\n");
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

    for (xfrs = 0, bytes = 0; ; xfrs++, bytes += shmp->cnt)
      {
	// Wait for our turn
	if(sem_wait(&shmp->rsem) == -1)
	  {
	    fprintf(stderr, "sem_wait\n");
	    exit(EXIT_FAILURE);
	  }

	// pshm_write encountered an EOF.
	if(shmp->cnt == 0)
	  break;

	write(STDOUT_FILENO, shmp->buf, shmp->cnt);

	// Give pshm_write a turn
	if(sem_post(&shmp->wsem) == -1)
	  {
	    fprintf(stderr, "sem_post\n");
	    exit(EXIT_FAILURE);
	  }
    }
    // Let pshm_write know we are finished.
    if(sem_post(&shmp->wsem) == -1)
      {
	fprintf(stderr, "sem_post\n");
	exit(EXIT_FAILURE);
      }

    exit(EXIT_SUCCESS);
}
