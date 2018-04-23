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
