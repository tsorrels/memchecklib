#include <openssl/md5.h>

struct memstat
{
    unsigned int textmemsize;
    unsigned int textoffset;
    char bintexthash [MD5_DIGEST_LENGTH];
    void * textaddr;    
};


int memcheckinit(struct memstat * statstructptr);

int checkmem(struct memstat statstruct);
