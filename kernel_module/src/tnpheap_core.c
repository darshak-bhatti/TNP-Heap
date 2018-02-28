//////////////////////////////////////////////////////////////////////
//                             North Carolina State University
//
//
//
//                             Copyright 2016
//
////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or modify it
// under the terms and conditions of the GNU General Public License,
// version 2, as published by the Free Software Foundation.
//
// This program is distributed in the hope it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
// more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
//
////////////////////////////////////////////////////////////////////////
//
//   Author:  Hung-Wei Tseng
//
//   Description:
//     Skeleton of NPHeap Pseudo Device
//
////////////////////////////////////////////////////////////////////////

#include "tnpheap_ioctl.h"

#include <asm/uaccess.h>
#include <linux/slab.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/poll.h>
#include <linux/mutex.h>
#include <linux/time.h>

struct miscdevice tnpheap_dev;

static unsigned long transaction_counter = 0;

struct object_vn_map
{      
    unsigned long object_id;        // Offset passed by the process
    unsigned long version_number;   //Version number of the object
    struct mutex object_mutex;      //Mutex for this mapping
    struct object_vn_map *next;     //Pointer to the next node
};

static struct object_vn_map *object_vn_map_head = NULL;

void initialize_object_vn(unsigned long object_id)
{   
    printk(KERN_INFO "Entering initialize_object_vn\n");

    struct object_vn_map *temp = (struct object_vn_map *) kmalloc(sizeof(struct object_vn_map),GFP_KERNEL);
    temp->object_id = object_id;
    temp->version_number = 1;
    temp->next = NULL;
    mutex_init(&(temp->object_mutex));

    if(object_vn_map_head == NULL) {
        object_vn_map_head = temp;
        printk(KERN_INFO "Exiting initialize_object_vn\n");
        return;
    }

    struct object_vn_map *iterator = object_vn_map_head;

    while(iterator->next != NULL){
        iterator = iterator->next;
    }

    iterator->next = temp;

    printk(KERN_INFO "Exiting initialize_object_vn\n");

    return;
}

unsigned long search_object_vn(unsigned long object_id)
{
    printk(KERN_INFO "Entering search_object_vn with offset %ld\n",object_id);
    if(object_vn_map_head == NULL){
        return 0;
    }

    struct object_vn_map *iterator = object_vn_map_head;
    
    while(iterator != NULL){
        if(iterator->object_id == object_id){
            printk(KERN_INFO "Exiting search_object_vn with offset %ld\n",object_id);
            return iterator->version_number;
        }

        iterator = iterator->next;
    }    

    printk(KERN_INFO "Exiting search_object_vn with offset %ld\n",object_id);
    return 0;
}

void update_object_vn(unsigned long object_id)
{
    printk(KERN_INFO "Entering update_object_vn with offset %ld\n",object_id);
    //No object
    if(object_vn_map_head == NULL){
        return;
    }

    struct object_vn_map *iterator = object_vn_map_head;

    while(iterator != NULL)
    {
        if(iterator->object_id == object_id)
        {
            iterator->version_number = iterator->version_number++;
            printk(KERN_INFO "Exiting update_object_vn with offset %ld\n",object_id);
            return;
        }

        iterator = iterator->next;
    }

    //No matches found
    printk(KERN_INFO "Exiting update_object_vn with offset %ld\n",object_id);
    return;
}

void acquire_lock(unsigned long object_id)
{
    printk(KERN_INFO "Entering acquire_lock with offset %ld\n",object_id);
     //No object
     if(object_vn_map_head == NULL){
        return;
    }

    struct object_vn_map *iterator = object_vn_map_head;
    
        while(iterator != NULL)
        {
            if(iterator->object_id == object_id)
            {
                mutex_lock(&(iterator->object_mutex));
                printk(KERN_INFO "Exiting acquire_lock with offset %ld with success\n",object_id);
                return;
            }
    
            iterator = iterator->next;
        }
    
        //No matches found
        printk(KERN_INFO "Exiting acquire_lock with offset %ld with error\n",object_id);
        return;

}

void release_lock(unsigned long object_id)
{
    printk(KERN_INFO "Entering release_lock with offset %ld\n",object_id);
     //No object
     if(object_vn_map_head == NULL){
        return;
    }

    struct object_vn_map *iterator = object_vn_map_head;
    
        while(iterator != NULL)
        {
            if(iterator->object_id == object_id)
            {
                mutex_unlock(&(iterator->object_mutex));
                printk(KERN_INFO "Exiting release_lock with offset %ld with success\n",object_id);
                return;
            }
    
            iterator = iterator->next;
        }
    
        //No matches found
        printk(KERN_INFO "Exiting release_lock with offset %ld with error\n",object_id);
        return;

}

__u64 tnpheap_get_version(struct tnpheap_cmd __user *user_cmd)
{
    printk(KERN_INFO "Entering get_version\n");
    struct tnpheap_cmd cmd;
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
        return -1 ;
    }  
    
    unsigned long version_number = search_object_vn(cmd.offset);
    if(version_number == 0){
        initialize_object_vn(cmd.offset);
        printk(KERN_INFO "Exiting get_version\n");
        return 1;
    }
    printk(KERN_INFO "Exiting get_version\n");
    return version_number;
}

__u64 tnpheap_start_tx(struct tnpheap_cmd __user *user_cmd)
{
    printk(KERN_INFO "Entering start_tx\n");
    struct tnpheap_cmd cmd;
    __u64 ret=0;

    printk(KERN_INFO "Hello I am kernel");

    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
        return -1 ;
    }    
    printk(KERN_INFO "Exiting start_tx\n");
    return (++transaction_counter);
}

__u64 tnpheap_commit(struct tnpheap_cmd __user *user_cmd)
{
    printk(KERN_INFO "Entering commit_tx\n");
    struct tnpheap_cmd cmd;
    __u64 ret=0;
    if (copy_from_user(&cmd, user_cmd, sizeof(cmd)))
    {
        return -1 ;
    }

    acquire_lock(cmd.offset);
    if(cmd.version == search_object_vn(cmd.offset)){
        ret = 0;
        update_object_vn(cmd.offset);
    }
    else{
        ret = 1;
    }

    release_lock(cmd.offset);
    printk(KERN_INFO "Exiting commit_tx\n");
    return ret;
}



__u64 tnpheap_ioctl(struct file *filp, unsigned int cmd,
                                unsigned long arg)
{
    switch (cmd) {
    case TNPHEAP_IOCTL_START_TX:
        printk(KERN_INFO "AAYA");
        return tnpheap_start_tx((void __user *) arg);
    case TNPHEAP_IOCTL_GET_VERSION:
        return tnpheap_get_version((void __user *) arg);
    case TNPHEAP_IOCTL_COMMIT:
        return tnpheap_commit((void __user *) arg);
    default:
        return -ENOTTY;
    }
}

static const struct file_operations tnpheap_fops = {
    .owner                = THIS_MODULE,
    .unlocked_ioctl       = (long) tnpheap_ioctl,
};

struct miscdevice tnpheap_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "tnpheap",
    .fops = &tnpheap_fops,
};

static int __init tnpheap_module_init(void)
{
    int ret = 0;
    if ((ret = misc_register(&tnpheap_dev)))
        printk(KERN_ERR "Unable to register \"tnpheap\" misc device\n");
    else
        printk(KERN_ERR "\"tnpheap\" misc device installed\n");
    return ret;
}

static void __exit tnpheap_module_exit(void)
{
    misc_deregister(&tnpheap_dev);
    return;
}

MODULE_AUTHOR("Hung-Wei Tseng <htseng3@ncsu.edu>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");
module_init(tnpheap_module_init);
module_exit(tnpheap_module_exit);
