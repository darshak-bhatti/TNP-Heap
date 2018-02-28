#include <npheap/tnpheap_ioctl.h>
#include <npheap/npheap.h>
#include <npheap.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <signal.h>
#include <malloc.h>
#include <string.h>

int global_check=0;

unsigned long my_transaction_number;

struct transaction_map{
    unsigned long offset; // Object_id of npheap
    unsigned long vn; // Version number from tnpheap
    char *buffer;
    unsigned long size;
    struct transaction_map *next;
} *transaction_map_head = NULL;


int insert_tm(unsigned long offset, unsigned long vn){

    //printf("Entering insert_tm with offset %d and vn %d\n", offset,vn);

    if(transaction_map_head == NULL){
        transaction_map_head = (struct transaction_map *)malloc(sizeof(struct transaction_map));
        transaction_map_head->offset = offset;
        transaction_map_head->vn = vn;
        transaction_map_head->buffer = NULL;
        transaction_map_head->size = 0;
        transaction_map_head->next = NULL;

        //printf("Ending insert_tm with offset %d and vn %d\n", offset,vn);

        return 0;
    }

    struct transaction_map *temp, *temp2;

    temp = (struct transaction_map *)malloc(sizeof(struct transaction_map));
    temp->offset = offset;
    temp->vn = vn;
    temp->buffer = NULL;
    temp->size = 0;
    temp->next = NULL;

    temp2 = transaction_map_head;

    while(temp2->next != NULL){
        if(temp2->offset == offset){
            temp2->vn = vn;
            //printf("Entry already there, Ending insert_tm with offset %d and vn %d\n", offset,vn);
            return 0;

        }
        temp2 = temp2->next;
    }

    temp2->next = temp;

    //printf("Ending insert_tm with offset %d and vn %d\n", offset,vn);
    return 0;
}

char * update_tm_buffer(unsigned long offset, unsigned long size){

    //printf("Entering update_tm_buffer with offset %d and size %d\n", offset,size);

    if(transaction_map_head == NULL){
        //printf("\n::update_tm_buffer :  Empty transaction map\n");
        //printf("Ending update_tm_buffer with offset %d and size %d\n", offset,size);
        return NULL;
    }
    struct transaction_map *temp;

    temp = transaction_map_head;

    while(temp != NULL && temp->offset != offset){
        temp = temp->next;
    }

    if(temp == NULL){
        //printf("\n::update_tm_buffer :  Could not find offset\n");
        //printf("Ending update_tm_buffer with offset %d and size %d\n", offset,size);
        return NULL;
    } else {
        temp->size = size;
        //temp->buffer = (char *)calloc(1,size);
        temp->buffer = (char *)malloc(size);
        //printf("Ending update_tm_buffer with offset %d and size %d\n", offset,size);
        //return memcpy(temp->buffer,mapped_data,size);
        return temp->buffer;
    }
}

void delete_tm()
{
    if(transaction_map_head == NULL){
        return;
    }

    struct transaction_map * iterator = transaction_map_head;
    struct transaction_map * temp;
    while(iterator != NULL){
        temp = iterator;
        iterator = iterator->next;
        temp->next = NULL;
        free(temp);
    }

    transaction_map_head = NULL;
    return;
}


__u64 tnpheap_get_version(int npheap_dev, int tnpheap_dev, __u64 offset)
{
    //printf("Entering get_version with offset %d\n", offset);
    struct tnpheap_cmd cmd;
    cmd.offset = offset;
    
    unsigned long kernel_return_vn = ioctl(tnpheap_dev, TNPHEAP_IOCTL_GET_VERSION, &cmd);

    if(kernel_return_vn == -1){
        //printf("\n::tnpheap_get_version : Kernel Get Version retruned -1\n");
    } else {
        insert_tm(offset, kernel_return_vn);
    }

    //printf("Ending get_version with offset %d\n", offset);
    return kernel_return_vn;
}



int tnpheap_handler(int sig, siginfo_t *si)
{
    //printf("\n\n::tnpheap_handler sig : %d and error_no %d pid %d\n\n", sig, si->si_errno,getpid());

    //signal(sig, SIG_DFL);
    //kill(getpid(), 9);

    int m_ret = mprotect(si->si_addr, getpagesize(), PROT_READ | PROT_WRITE);
    if(m_ret == 0){
        printf("\n mprotect Success");
    } else {
        printf("\n Duh Mprotect error");
    }

    return 0;
}


void *tnpheap_alloc(int npheap_dev, int tnpheap_dev, __u64 offset, __u64 size)
{
    //printf("Entering tnpheap_alloc with offset %d and size %d\n", offset, size);
    unsigned long get_vn_return = tnpheap_get_version(npheap_dev, tnpheap_dev, offset);
    //char *mapped_data;
    char *return_buffer;
    //printf("\n::tnpheap_alloc : User-lib Get Version retruned %ld \n", get_vn_return);

    if(get_vn_return == -1){
        //printf("\n::tnpheap_alloc : User-lib Get Version retruned -1 \n");
    } else {
        //int npheap_lock_ret = npheap_lock(npheap_dev, offset);
        //mapped_data = (char *)npheap_alloc(npheap_dev,offset,size);
        //if(mapped_data == NULL){
          //  return NULL;
        //}
        return_buffer = update_tm_buffer(offset,size);
        //int npheap_unlock_ret = npheap_unlock(npheap_dev, offset);
    }

    if(return_buffer == NULL){
        return NULL;
        //printf("\n::tnpheap_alloc : update_tm_buffer retruned NULL \n");
    }

    //printf("Ending tnpheap_alloc with offset %d and size %d\n", offset, size);
    return return_buffer;     
}

//returns transaction_id to the user
__u64 tnpheap_start_tx(int npheap_dev, int tnpheap_dev)
{
    //printf("Entering tnpheap_start_tx\n");

    struct tnpheap_cmd cmd;
    cmd.version = 5;
    if(global_check == 0){
        //printf("\n global_check zero");
        global_check = rand() % 1000;
    }

    //printf("User library : %d\n", global_check);

    //printf("ending tnpheap_start_tx and calling kernel function \n");

    return ioctl(tnpheap_dev, TNPHEAP_IOCTL_START_TX, &cmd);
}

int tnpheap_commit(int npheap_dev, int tnpheap_dev)
{
    //printf("Entering tnpheap_commit_tx\n");
    struct transaction_map *temp, *temp2;
    temp = transaction_map_head;
    while(temp!= NULL){
        int npheap_lock_ret = npheap_lock(npheap_dev, temp->offset);
        char * mapped_data = (char *)npheap_alloc(npheap_dev,temp->offset, temp->size);

        if(mapped_data == NULL){
            return 1;
        }

        struct tnpheap_cmd cmd;
        cmd.offset = temp->offset;
        cmd.version = temp->vn;

        int kernel_return_commit = ioctl(tnpheap_dev, TNPHEAP_IOCTL_COMMIT, &cmd);

        if(kernel_return_commit == 1){
            npheap_unlock(npheap_dev, temp->offset);
            //printf("Ending tnpheap_commiy_tx with transaction abort\n");
            //Delete the temp buffers
            delete_tm();
            return 1;
        }

        memcpy(mapped_data, temp->buffer, temp->size);

        int npheap_unlock_ret = npheap_unlock(npheap_dev, temp->offset);

        temp2 = temp;
        temp = temp->next;
        free(temp2); // Deleting the entry
    }
    transaction_map_head = NULL;
    //printf("User library Commit\n");
    //printf("Ending tnpheap_commiy_tx with transaction success!\n");
    return 0;
}

