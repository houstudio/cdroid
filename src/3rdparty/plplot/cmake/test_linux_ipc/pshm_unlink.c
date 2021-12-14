// Copyright (C) Alan W. Irwin, 2016
// Copyright (C) Michael Kerrisk, 2016
//
// This program is free software. You may use, modify, and redistribute it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 or (at your option) any
// later version. This program is distributed without any warranty.  See
// the file COPYING.gpl-v3 for details.

// pshm_unlink.c

// Usage: pshm_unlink
// Unlink shared memory object created by pshm_write, if there has
// been an unclean exit from the latter.

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
    // Clean up
    if(shm_unlink(SHM_PATH) == -1)
      {
	fprintf(stderr, "shm_unlink\n");
	exit(EXIT_FAILURE);
      }

    exit(EXIT_SUCCESS);
}
