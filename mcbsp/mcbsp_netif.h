/*
 * Driver for mcbsp-netif
 *
 * Copyright (C) 2012 Galilsoft.
 *
 * Authors:
 *	Kuzminov Dmitry	<dima@galilsoft.com>
 *
 */
#ifndef __MCBSP_NETIF_H__ 
#define __MCBSP_NETIF_H__ 

#include <linux/list.h>
#include <linux/skbuff.h>
#include <linux/device.h>
#include <linux/workqueue.h>

//#define DEBUG
//#define SUPPORT_SPCR1_RSYNC_ERROR

#define PERROR(fmt, args...) \
    printk(KERN_ERR "MCBSP NETIF [%s:%d]: " fmt, __FUNCTION__, __LINE__,  ##args)

#ifdef DEBUG
#define PDEBUG(fmt, args...) \
    printk(KERN_DEBUG "MCBSP NETIF [%s:%d]: " fmt, __FUNCTION__, __LINE__,  ##args)
#else
#define PDEBUG(fmt, args...)
#endif

#define MAX_BUFFER_ELEMENTS 375 /* each element 4 octets, total 1500 octets */
#define ETHERNET_HEADER_SIZE 14

struct mcbsp_driver;

struct mcbsp_msg {
	struct list_head    list;
	struct sk_buff      *skb;
    unsigned int        actual_data_length;
};

struct mcbsp_dev {
    struct mcbsp_driver *driver;
	struct net_device   *net;

    /* tx section */
    u32                 tx_aligned_buffer[MAX_BUFFER_ELEMENTS];

    /* rx section */
    enum {
        MCBSP_PACKAGE_WAIT_PREAMBLE_0 = 0,
        MCBSP_PACKAGE_WAIT_PREAMBLE_1,
        MCBSP_PACKAGE_WAIT_LENGTH_0,
        MCBSP_PACKAGE_WAIT_LENGTH_1,
        MCBSP_PACKAGE_READING_DATA
    } MCBSP_STATE_MACHINE;
    int                 read_parse_status;
    int                 read_parse_length;
    int                 read_parse_count;
    char                *rx_data;
    struct sk_buff      *skb_reading;
	struct list_head    msg_list;
    spinlock_t          rx_msg_list_lock;
    struct work_struct  rx_work;
    
    /* need to be released */
    struct workqueue_struct     *rx_work_queue_ptr;
};

struct mcbsp_driver {
	struct module       *owner;
    char                *name;
    struct mcbsp_dev    *dev;

    /* ops */
    int     (*write)        (const char *buf, int count, struct mcbsp_dev *dev);
    void    (*parse_read)   (u32 *data, struct mcbsp_dev *dev);
    void    (*read_callback)(struct work_struct *work);
};

void*   mcbsp_netif_alloc_dev(void);
int     mcbsp_netif_setup_netdev(struct mcbsp_dev *ue, struct device_type *dev_type);
void    mcbsp_netif_remove_netdev(struct mcbsp_dev *ue);
struct  mcbsp_driver* mcbsp_drv_get_driver_from_mcbsp_dev(struct mcbsp_dev *dev);

void    mcbsp_netif_rx_handler(struct work_struct *work);

#endif /* __MCBSP_NETIF_H__ */ 
