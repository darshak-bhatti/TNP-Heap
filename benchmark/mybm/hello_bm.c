#include <stdio.h>
#include <stdlib.h>
#include <sys/syscall.h>
#include <time.h>
#include <npheap.h>
#include <tnpheap.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/time.h>
#include <string.h>
#include <sys/wait.h>
#include <malloc.h>


int main(int argc, char *argv[])
{
    int npheap_dev, tnpheap_dev;
    //tnpheap_init();

    npheap_dev = open("/dev/npheap",O_RDWR);
    tnpheap_dev = open("/dev/tnpheap",O_RDWR);

    START_TX(npheap_dev, tnpheap_dev);
    char * mapped_data = (char *)tnpheap_alloc(npheap_dev,tnpheap_dev,5,1024);
    COMMIT(npheap_dev, tnpheap_dev);

    return 0;
}