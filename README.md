# memchecklib

A lightweight library to check whether the running process's .text segment (program instructions) has been altered.

## Usage
```bash
 $ make lib
 $ gcc -o yourprog memchecklib.a -lcrypto

```


## Overview

The API consists of just two simple calls: 

```c
/* Call to initialize the library and hydrate the memstat struct with
 * critical binary metadata.  Returns 0 on success. */  
int memcheckinit(struct memstat * statstructptr);

/* Call to compare the currently loaded and running .text segment of the
 * program with the .text segment compiled into the target binary from which
 * the running program was loaded and launched.  Returns 1 if the .text 
 * segment contents match. */
int checkmem(struct memstat statstruct);

```


## Example
```c
#include <stdio.h>
#include <stdlib.h>
#include "memcheck.h"

void memexit();

int main()
{
    struct memstat statstruct;
    int memclean;
    int meminit;

    meminit = memcheckinit(&statstruct);
    if (meminit != 0)
    {
	printf("memcheckinit failed: exiting.\n");
	exit(1);
    }
    
    printf("Executing phase 1.\n");
    memclean = checkmem(statstruct);
    if (memclean != 0)
	memexit();

    printf("Executing phase 2.\n");
    memclean = checkmem(statstruct);
    if (memclean != 0)
	memexit();
    
    printf("Executing phase 3.\n");
    
    return 0;
}

void memexit()
{
    printf("Program instructions corrupted; exiting.\n");
    exit(0);	
}
```

####TODO
1. remove dependency on readelf
