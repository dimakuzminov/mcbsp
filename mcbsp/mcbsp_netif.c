/*
 * Driver for mcbsp-serial
 *
 * Copyright (C) 2012 Galilsoft.
 *
 * Authors:
 *	Kuzminov Dmitry	<dima@galilsoft.com>
 *
 */

#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/init.h>
#include <linux/skbuff.h>
#include <linux/spinlock.h>
#include <linux/mii.h>
#include <linux/ethtool.h>
#include <linux/wait.h>
#include <linux/sched.h>
#include <linux/random.h>

#include "mcbsp_netif.h"

static int watchdog = 5000;
module_param(watchdog, int, 0400);
MODULE_PARM_DESC(watchdog, "transmit timeout in milliseconds");

const u8 template_ethernet_header[] = {0x00, 0xbb, 0xaa, 0xcc, 0xdd, 0x44, 0x00, 0xa0, 0xb0, 0xc0, 0xd0, 0x60, 0x08, 0x00};

void *mcbsp_netif_alloc_dev(void)
{
	struct mcbsp_dev *dev;
	struct net_device *ndev;
	ndev = alloc_etherdev(sizeof(struct mcbsp_dev));
	if(!ndev) {
        PERROR("CANNOT ALLOCATE ETHERNET DEVICE\n");
		return NULL;
    }
	dev = netdev_priv(ndev);
	dev->net = ndev;
    PDEBUG("ETHERNET DEVICE ALLOCATED\n");
    return dev;
}

void mcbsp_netif_remove_netdev(struct mcbsp_dev *dev)
{
	unregister_netdev(dev->net);
	free_netdev(dev->net);
    PDEBUG("\n");
}

void mcbsp_netif_dump_hex(const char *label, char * buf, int len)
{
#ifdef DEBUG
    int i;
    PDEBUG("DUMP REGISTER START:\n");
    printk("%s hex, %d bytes:\n", label, len);
    for (i = 0; i < len; i++) {
        printk("0x%02x  ", buf[i]);
        if (!((i+1) % 20))
            printk("\n");
    }
    printk("\n");
    PDEBUG("DUMP REGISTER FINISHED:\n");
#endif
}

void mcbsp_netif_rx_handler(struct work_struct *work)
{
    struct mcbsp_dev *dev = container_of(work, struct mcbsp_dev, rx_work);
    struct net_device *net = dev->net;
    struct mcbsp_msg *msg;
    struct sk_buff *skb;
    unsigned long flags;

    PDEBUG("start\n");
    while(true) {
        spin_lock_irqsave(&dev->rx_msg_list_lock, flags);
        if (list_empty(&dev->msg_list)) {
            spin_unlock_irqrestore(&dev->rx_msg_list_lock, flags);
            return;
        }
        msg = list_entry(dev->msg_list.next, struct mcbsp_msg, list);
        list_del(&msg->list);
        spin_unlock_irqrestore(&dev->rx_msg_list_lock, flags);

        skb = skb_clone(msg->skb, GFP_ATOMIC | GFP_DMA);
        skb_trim(skb, msg->actual_data_length);
        mcbsp_netif_dump_hex("RX skb", skb->data, skb->len);
        kfree_skb(msg->skb);

        skb->dev = net;
        skb->protocol = eth_type_trans(skb, net);
        skb->ip_summed = CHECKSUM_UNNECESSARY; 

        /* cannot use netif_rx, because soft interrupt */
        netif_rx_ni(skb);

        net->stats.rx_bytes += skb->len; 
        net->stats.rx_packets++;
    }
    PDEBUG("finished\n");
}

static int mcbsp_netif_start_xmit(struct sk_buff *skb, struct net_device *net)
{
	char *data;
    int len;
    struct mcbsp_dev *dev = netdev_priv(net);
    struct mcbsp_driver *driver = mcbsp_drv_get_driver_from_mcbsp_dev(dev); 
    data = skb->data;
    len = skb->len;
    net->trans_start = jiffies;
    mcbsp_netif_dump_hex("TX skb", data, len);
    driver->write(data, len, dev);
	kfree_skb(skb);
	net->stats.tx_bytes += len; 
	net->stats.tx_packets++;
    PDEBUG(" WRITTEN %d BYTES\n", len);
	return NETDEV_TX_OK;
}

static void mcbsp_netif_timeout(struct net_device *net)
{
	net->trans_start = jiffies;
    PDEBUG("\n");
}

static int mcbsp_netif_open(struct net_device *dev)
{
	netif_start_queue(dev);
    PDEBUG("\n");
	return 0;
}

static int mcbsp_netif_stop(struct net_device *ndev)
{
    netif_stop_queue(ndev);
    PDEBUG("\n");
	return 0;
}

static const struct net_device_ops ue_netdev_ops = {
	.ndo_open		        = mcbsp_netif_open,
	.ndo_stop		        = mcbsp_netif_stop,
	.ndo_start_xmit		    = mcbsp_netif_start_xmit,
	.ndo_tx_timeout		    = mcbsp_netif_timeout,
	.ndo_change_mtu		    = eth_change_mtu,
	.ndo_validate_addr	    = eth_validate_addr,
	.ndo_set_mac_address	= eth_mac_addr,
};

void obtain_eth_addr(char * mac)
{
    int i;
    unsigned char uuid[16];
    generate_random_uuid(uuid);
    mac[0]=0x00;
    for (i = 1; i < 6; i++) {
        mac[i] = uuid[i-1];
    }
}

int mcbsp_netif_setup_netdev(struct mcbsp_dev *dev, struct device_type* dev_type)
{
	int ret;
	struct net_device *ndev = dev->net;
	obtain_eth_addr(ndev->dev_addr);
#if 0
	ndev->flags = IFF_NOARP;
#else
    ndev->flags = IFF_BROADCAST;  
#endif    
    strcpy(ndev->name,"mcbsp%d");
	SET_NETDEV_DEVTYPE(ndev, dev_type);
    ndev->netdev_ops        = &ue_netdev_ops;
	ndev->watchdog_timeo	= msecs_to_jiffies(watchdog);
	ndev->tx_queue_len      = 100;
	ret = register_netdev(ndev);
	if (ret) {
        PERROR("CANNOT REGISTER NETDEV\n");
		return ret;
    }
	netif_carrier_on(ndev);
	netif_dormant_off(ndev);
    PDEBUG("NETDEV IS READY\n");
    return ret;
}
