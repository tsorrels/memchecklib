#include <fcntl.h>
#include <openssl/md5.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "memcheck.h"


const char * readelfstr = "readelf -S /proc/";
const char * exestr = "/exe";
const char * grepstr = " | grep -A 1 \".text\"";

#define TERM_WIDTH 80
#define HEX_BIT_LENGTH 16
#define WHITESPACE " \t\n"
#define ELF_ADDR_TEXT_SEGMENT_INDEX 3
#define ELF_SIZE_TEXT_SEGMENT_INDEX 0
#define ELF_OFFSET_TEXT_SEGMENT_INDEX 4


void hashmem(char * md5hashbuf, void * startaddr, uint memsize);
int comparemd5hash (unsigned char * hashbufleft,  unsigned char * hashbufright);
void printmd5hash (unsigned char * hashbuf);
void hashbinary(char * md5hashbuf, uint offset, uint memsize, pid_t pid);
void * gettextaddr(char * line);
void gettextsegment(int segmentindex, char * line, char * segmentbuf);
int iswhitespace(char * string);
unsigned int gettextsize(char * line);
unsigned int gettextoffset(char * line);


int memcheckinit(struct memstat * statstructptr)
{
    pid_t pid;
    char pidstr[20];
    char readelfexestr[100];
    char line1buf[TERM_WIDTH + 1];
    char line2buf[TERM_WIDTH + 1];
    void * starttextaddr;
    uint memsize;
    uint offset;
    FILE * fp;
    char texthash [MD5_DIGEST_LENGTH];    
        
    pid = getpid();
    
    snprintf(pidstr, 20, "%u", pid);

    readelfexestr[0] = 0;

    strcat(readelfexestr, readelfstr);
    strcat(readelfexestr, pidstr);
    strcat(readelfexestr, exestr);
    strcat(readelfexestr, grepstr);    
    
    fp = popen(readelfexestr, "r");

    fgets(line1buf, TERM_WIDTH + 1, fp);
    fgets(line2buf, TERM_WIDTH + 1, fp);
    
    pclose(fp);
    
    memsize = 0;
    starttextaddr = gettextaddr(line1buf);

    offset = gettextoffset(line1buf);
    
    memsize = gettextsize(line2buf);

    hashbinary(texthash, offset, memsize, pid);
    
    statstructptr->textmemsize = memsize;
    statstructptr->textoffset = offset;   
    statstructptr->textaddr = starttextaddr;
    memcpy(statstructptr->bintexthash, texthash, MD5_DIGEST_LENGTH);
    
    return 0;
}


void hashbinary(char * md5hashbuf, uint offset, uint memsize, pid_t pid)
{
    int fd;
    char pathstr[100];
    char pidstr[20];
    MD5_CTX c;
    unsigned int curoffset;
    char byte;
    unsigned char hashbuf[MD5_DIGEST_LENGTH];

    pathstr[0] = 0;
    
    snprintf(pidstr, 20, "%u", pid);
    
    strcat(pathstr, "/proc/");    
    strcat(pathstr, pidstr);
    strcat(pathstr, exestr);
    
    fd = open(pathstr, O_RDONLY);

    lseek(fd, offset, SEEK_SET);

    curoffset = offset;
    
    MD5_Init(&c);

    while (curoffset < offset + memsize)
    {
	read(fd, &byte, sizeof(char));
        MD5_Update(&c, &byte, sizeof(char));
	curoffset ++;
    }

    close(fd);
    
    MD5_Final(hashbuf, &c);
    memcpy(md5hashbuf, hashbuf, MD5_DIGEST_LENGTH);    
}


int checkmem(struct memstat statstruct)
{
    int result;
    int ismatch;
    unsigned char memhashbuf[MD5_DIGEST_LENGTH];

    result = -1;
    
    hashmem(memhashbuf, statstruct.textaddr, statstruct.textmemsize);

    ismatch = comparemd5hash (memhashbuf, statstruct.bintexthash );

    if (ismatch == 1)
    {
	result = 0;
    }
    
    return result;
}


void hashmem(char * md5hashbuf, void * startaddr, uint memsize)
{
    void * endaddr; 
    char * memptr; 

    MD5_CTX c;
    unsigned char hashbuf[MD5_DIGEST_LENGTH];

    memptr = (char *) startaddr;
    endaddr = (char *) startaddr + memsize; 
    
    MD5_Init(&c);
   
    while ((void *) memptr <  endaddr)
    {
        MD5_Update(&c, memptr, sizeof(char));
	memptr ++;
    }
	
    MD5_Final(hashbuf, &c);

    memcpy(md5hashbuf, hashbuf, MD5_DIGEST_LENGTH);
}


void * gettextaddr(char * line)
{
    void * textaddrptr;
    unsigned long memaddr;
    char textaddrbuffer[HEX_BIT_LENGTH + 1];

    gettextsegment(ELF_ADDR_TEXT_SEGMENT_INDEX, line, textaddrbuffer);
    
    memaddr = strtoul(textaddrbuffer, NULL, 16);
	
    textaddrptr = (void *) memaddr;

    return textaddrptr;
}


void gettextsegment(int segmentindex, char * originalline, char * segmentbuf)
{
    char * token;
    int i;
    char line [TERM_WIDTH + 1];

    strcpy(line, originalline);
    
    token = strtok(line, WHITESPACE);
    while (iswhitespace(token))
    {
	token = strtok(NULL, WHITESPACE);
    }
    
    for (i = 1 ; i <= segmentindex ; i ++)
    {
	token = strtok(NULL, WHITESPACE);
	while (iswhitespace(token))
	{
	    token = strtok(NULL, WHITESPACE);
	}
    }

    strcpy(segmentbuf, token);
}


int iswhitespace(char * string)
{
    int iswhitespace;
    int i;

    i = strlen(string);

    for (i = 0 ; i < strlen(string) ; i ++)
    {
	if (string[i] != ' ' && string[i] != '\t' && string[i] != '\n')
	{
	    iswhitespace = 0;
	}
    }    

    return iswhitespace;
}


unsigned int gettextsize(char * line)
{
    unsigned long textsize;
    char textsizebuffer[HEX_BIT_LENGTH + 1];

    gettextsegment(ELF_SIZE_TEXT_SEGMENT_INDEX, line, textsizebuffer);
    
    textsize = strtoul(textsizebuffer, NULL, 16);
	
    return textsize;
}


unsigned int gettextoffset(char * line)
{
    unsigned long textoffset;
    char textsizebuffer[HEX_BIT_LENGTH + 1];
    
    gettextsegment(ELF_OFFSET_TEXT_SEGMENT_INDEX, line, textsizebuffer);
    
    textoffset = strtoul(textsizebuffer, NULL, 16);
	
    return textoffset;
}




void printmd5hash (unsigned char * hashbuf)
{
    int i;

    printf("Hash = ");
    
    for(i=0 ; i < MD5_DIGEST_LENGTH ; i++)
    {
        printf("%02x", hashbuf[i]);
    }
    printf("\n");    
}



int comparemd5hash (unsigned char * hashbufleft,  unsigned char * hashbufright)
{
    int ismatch;      
    char hashstringleft[MD5_DIGEST_LENGTH + 1];
    char hashstringright[MD5_DIGEST_LENGTH + 1];


    ismatch = 0;
    memcpy(hashstringleft, hashbufleft, MD5_DIGEST_LENGTH);
    memcpy(hashstringright, hashbufright, MD5_DIGEST_LENGTH);
    
    if (strcmp(hashstringleft, hashstringright) == 0)
    {
	ismatch = 1;	
    }

    return ismatch;
}
