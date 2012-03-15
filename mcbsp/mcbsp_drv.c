/*
 * Driver for mcbsp-netif
 *
 * Copyright (C) 2012 Galilsoft.
 *
 * Authors:
 *	Kuzminov Dmitry	<dima@galilsoft.com>
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/moduleparam.h>
#include <linux/fs.h>
#include <linux/gfp.h>
#include <linux/cdev.h>
#include <linux/sched.h>
#include <linux/proc_fs.h>
#include <linux/ioctl.h>
#include <linux/mm.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/sched.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <mach/memory.h>
#include <mach/hardware.h>
#include <plat/mcbsp.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/io.h>
#include <linux/sysctl.h>
#include <linux/dma-mapping.h>
#include <linux/delay.h>
#include <linux/workqueue.h>

#include "mcbsp_core.h"
#include "mcbsp_netif.h"

#ifdef MCBSP_DMA
#include <linux/netdevice.h>
#include <plat/dma.h>
#include <plat/mcbsp.h>
#endif

static int mcbsp_dma_setup(void);
static void mcbsp_read_dma(struct mcbsp_dev *dev, int len);

struct mcbsp_driver mcbsp_drv;
const u32 mcbsp_preamble[] = {0xfffefdfc, 0xfbfaf9f8};
#ifdef MCBSP_DMA
struct completion rx_dma_completion;
struct completion tx_dma_completion;
#endif

#define MCBSP_DRV_DRIVER_NAME "mcbsp-netif"
#define MCBSP_DRV_MAJOR 205

static u32 OMAP35XX_CM_FCLKEN1_CORE;
static u32 OMAP35XX_CM_ICLKEN1_CORE;
static u32 OMAP35XX_CONTROL_DEVCONF0;
static u32 CONTROL_PADCONF_X;
static u32 OMAP_MCBSP_1_CORE;

#define MCBSP_ACCESS_RETRY 100 /* 0.5 sec */
#define MCBSP_ACCESS_DELAY 5   /* 5 mls */
#define FALSE 0
#define TRUE 1

#define SRG_FPER  40
#define SRG_FWID  2
#define SRG_CLKDV 100
#define FRAME1_WORDS 1
#define FRAME2_WORDS 1

#define OMAP35XX_CM_FCLKEN1_CORE_BASE      0x48004a00
#define OMAP35XX_CM_ICLKEN1_CORE_BASE      0x48004a10
#define CONTROL_PADCONF_X_BASE             0x4800218c
#define OMAP35XX_CONTROL_DEVCONF0_BASE     0x48002274

#define OMAP_MCBSP_1_BASE                  0x48074000
#define OMAP_MCBSP_1_END                   0x48074fff
#define OMAP_MCBSP_1_SIZE                  SZ_4K

#if 1
    #define MCBSP_DRV_RX_IRQ INT_24XX_MCBSP1_IRQ_RX
#else
    #define MCBSP_DRV_RX_IRQ INT_34XX_MCBSP1_IRQ
#endif

#define OMAP_MCBSP1_REGISTER_READ(reg) \
    (*((u32*)(OMAP_MCBSP_1_CORE+reg)))

#define OMAP_MCBSP1_REGISTER_WRITE(reg, data) \
    ((*((u32*)(OMAP_MCBSP_1_CORE+reg))) = (u32)(data))

#define OMAP_MCBSP1_REGISTER_READ_PRINT(reg, reg_address) \
    printk("########## DUMP: %s = 0x%x\n", reg, OMAP_MCBSP1_REGISTER_READ(reg_address))

#define GPIO_REGISTER_READ(reg) \
    (*((u32*)(reg)))

#define GPIO_REGISTER_WRITE(reg, val) \
    (*((u32*)(reg))=(u32)(val))

void mcbsp_drv_dump_registers(void)
{
    char *sMCBSPLP_DRR_REG   = "MCBSPLP_DRR_REG";
    char *sMCBSPLP_DXR_REG   = "MCBSPLP_DXR_REG";
    char *sMCBSPLP_SPCR2_REG = "MCBSPLP_SPCR2_REG";
    char *sMCBSPLP_SPCR1_REG = "MCBSPLP_SPCR1_REG";
    char *sMCBSPLP_RCR2_REG  = "MCBSPLP_RCR2_REG";
    char *sMCBSPLP_RCR1_REG  = "MCBSPLP_RCR1_REG";
    char *sMCBSPLP_XCR2_REG  = "MCBSPLP_XCR2_REG";
    char *sMCBSPLP_XCR1_REG  = "MCBSPLP_XCR1_REG";
    char *sMCBSPLP_SRGR2_REG = "MCBSPLP_SRGR2_REG";
    char *sMCBSPLP_SRGR1_REG = "MCBSPLP_SRGR1_REG";
    char *sMCBSPLP_MCR2_REG  = "MCBSPLP_MCR2_REG";
    char *sMCBSPLP_MCR1_REG  = "MCBSPLP_MCR1_REG";
    char *sMCBSPLP_RCERA_REG = "MCBSPLP_RCERA_REG";
    char *sMCBSPLP_RCERB_REG = "MCBSPLP_RCERB_REG";
    char *sMCBSPLP_XCERA_REG = "MCBSPLP_XCERA_REG";
    char *sMCBSPLP_XCERB_REG = "MCBSPLP_XCERB_REG";
    char *sMCBSPLP_PCR_REG   = "MCBSPLP_PCR_REG";
    char *sMCBSPLP_RCERC_REG = "MCBSPLP_RCERC_REG";
    char *sMCBSPLP_RCERD_REG = "MCBSPLP_RCERD_REG";
    char *sMCBSPLP_XCERC_REG = "MCBSPLP_XCERC_REG";
    char *sMCBSPLP_XCERD_REG = "MCBSPLP_XCERD_REG";
    char *sMCBSPLP_RCERE_REG = "MCBSPLP_RCERE_REG";
    char *sMCBSPLP_RCERF_REG = "MCBSPLP_RCERF_REG";
    char *sMCBSPLP_XCERE_REG = "MCBSPLP_XCERE_REG";
    char *sMCBSPLP_XCERF_REG = "MCBSPLP_XCERF_REG";
    char *sMCBSPLP_RCERG_REG = "MCBSPLP_RCERG_REG";
    char *sMCBSPLP_RCERH_REG = "MCBSPLP_RCERH_REG";
    char *sMCBSPLP_XCERG_REG = "MCBSPLP_XCERG_REG";
    char *sMCBSPLP_XCERH_REG = "MCBSPLP_XCERH_REG";
    char *sMCBSPLP_REV_REG   = "MCBSPLP_REV_REG";

    char *sMCBSPLP_RINTCLR_REG   = "MCBSPLP_RINTCLR_REG";
    char *sMCBSPLP_XINTCLR_REG   = "MCBSPLP_XINTCLR_REG";
    char *sMCBSPLP_ROVFLCLR_REG  = "MCBSPLP_ROVFLCLR_REG";
    char *sMCBSPLP_SYSCONFIG_REG = "MCBSPLP_SYSCONFIG_REG";
    char *sMCBSPLP_THRSH2_REG    = "MCBSPLP_THRSH2_REG";
    char *sMCBSPLP_THRSH1_REG    = "MCBSPLP_THRSH1_REG";
    char *sMCBSPLP_IRQSTATUS_REG = "MCBSPLP_IRQSTATUS_REG";
    char *sMCBSPLP_IRQENABLE_REG = "MCBSPLP_IRQENABLE_REG";
    char *sMCBSPLP_WAKEUPEN_REG  = "MCBSPLP_WAKEUPEN_REG";
    char *sMCBSPLP_XCCR_REG      = "MCBSPLP_XCCR_REG";
    char *sMCBSPLP_RCCR_REG      = "MCBSPLP_RCCR_REG";
    char *sMCBSPLP_XBUFFSTAT_REG = "MCBSPLP_XBUFFSTAT_REG";
    char *sMCBSPLP_RBUFFSTAT_REG = "MCBSPLP_RBUFFSTAT_REG";
    char *sMCBSPLP_SSELCR_REG    = "MCBSPLP_SSELCR_REG";
    char *sMCBSPLP_STATUS_REG    = "MCBSPLP_STATUS_REG";


    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_DRR_REG,   MCBSPLP_DRR_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_DXR_REG,   MCBSPLP_DXR_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_SPCR2_REG, MCBSPLP_SPCR2_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_SPCR1_REG, MCBSPLP_SPCR1_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RCR2_REG,  MCBSPLP_RCR2_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RCR1_REG,  MCBSPLP_RCR1_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XCR2_REG,  MCBSPLP_XCR2_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XCR1_REG,  MCBSPLP_XCR1_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_SRGR2_REG, MCBSPLP_SRGR2_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_SRGR1_REG, MCBSPLP_SRGR1_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_MCR2_REG,  MCBSPLP_MCR2_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_MCR1_REG,  MCBSPLP_MCR1_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RCERA_REG, MCBSPLP_RCERA_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RCERB_REG, MCBSPLP_RCERB_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XCERA_REG, MCBSPLP_XCERA_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XCERB_REG, MCBSPLP_XCERB_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_PCR_REG,   MCBSPLP_PCR_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RCERC_REG, MCBSPLP_RCERC_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RCERD_REG, MCBSPLP_RCERD_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XCERC_REG, MCBSPLP_XCERC_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XCERD_REG, MCBSPLP_XCERD_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RCERE_REG, MCBSPLP_RCERE_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RCERF_REG, MCBSPLP_RCERF_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XCERE_REG, MCBSPLP_XCERE_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XCERF_REG, MCBSPLP_XCERF_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RCERG_REG, MCBSPLP_RCERG_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RCERH_REG, MCBSPLP_RCERH_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XCERG_REG, MCBSPLP_XCERG_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XCERH_REG, MCBSPLP_XCERH_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_REV_REG,   MCBSPLP_REV_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RINTCLR_REG  ,MCBSPLP_RINTCLR_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XINTCLR_REG  ,MCBSPLP_XINTCLR_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_ROVFLCLR_REG ,MCBSPLP_ROVFLCLR_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_SYSCONFIG_REG,MCBSPLP_SYSCONFIG_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_THRSH2_REG   ,MCBSPLP_THRSH2_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_THRSH1_REG   ,MCBSPLP_THRSH1_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_IRQSTATUS_REG,MCBSPLP_IRQSTATUS_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_IRQENABLE_REG,MCBSPLP_IRQENABLE_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_WAKEUPEN_REG ,MCBSPLP_WAKEUPEN_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XCCR_REG     ,MCBSPLP_XCCR_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RCCR_REG     ,MCBSPLP_RCCR_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_XBUFFSTAT_REG,MCBSPLP_XBUFFSTAT_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_RBUFFSTAT_REG,MCBSPLP_RBUFFSTAT_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_SSELCR_REG   ,MCBSPLP_SSELCR_REG);
    OMAP_MCBSP1_REGISTER_READ_PRINT(sMCBSPLP_STATUS_REG   ,MCBSPLP_STATUS_REG);
}

void mcbsp_drv_release_reset(void)
{
    int val;
    /* Release Rx reset*/
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR1_REG);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR1_REG, val|SPCR1_RRST);
    /* Release Tx reset*/
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR2_REG);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR2_REG, val|SPCR2_XRST);    
    /* Release Fsg reset*/
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR2_REG);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR2_REG, val|SPCR2_FRST);    
    /* Release srg reset*/
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR2_REG);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR2_REG, val|SPCR2_GRST);    
}

void mcbsp_drv_analog_loopback(int enable)
{   
    int val;
    if(enable)
    {
        val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR1_REG);
        OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR1_REG, val|SPCR1_ALB);
    }
    else
    {
        val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR1_REG);
        OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR1_REG, val&(~SPCR1_ALB));
    }
}

void mcbsp_drv_digitial_loopback(int enable)
{   
    int val;
    if(enable)
    {
        val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_XCCR_REG);
        OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_XCCR_REG, val|XCCR_DLB);
    }
    else
    {
        val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_XCCR_REG);
        OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_XCCR_REG, val&(~XCCR_DLB));/*shaul*/
    }
}

void mcbsp_drv_reset(void)
{
    int val;
    /* reset Rx */
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR1_REG);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR1_REG, val&(~SPCR1_RRST));

    /* reset Tx */
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR2_REG);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR2_REG, val&(~SPCR2_XRST));

    /* reset Fsg */
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR2_REG);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR2_REG, val&(~SPCR2_FRST)); 

    /* reset Srg */
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR2_REG);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR2_REG, val&(~SPCR2_GRST)); 
}

int mcbsp_drv_enable_clk(void)
{
    int val;

    /* enable MCBSP1_FCLK and MCBSP1_ICLK */
    val = GPIO_REGISTER_READ(OMAP35XX_CM_FCLKEN1_CORE);
    val = val|OMAP35XX_EN_MCBSP1;
    GPIO_REGISTER_WRITE(OMAP35XX_CM_FCLKEN1_CORE, val);
    PDEBUG(": val_OMAP35XX_CM_FCLKEN1_CORE = %d , 0x%x ; OMAP35XX_CM_FCLKEN1_CORE = 0x%x \n",
             val, val, OMAP35XX_CM_FCLKEN1_CORE);            

    val = GPIO_REGISTER_READ(OMAP35XX_CM_ICLKEN1_CORE);
    val = val|OMAP35XX_EN_MCBSP1;
    GPIO_REGISTER_WRITE(OMAP35XX_CM_ICLKEN1_CORE, val);
    PDEBUG(": val_OMAP35XX_CM_ICLKEN1_CORE = %d , 0x%x ; OMAP35XX_CM_ICLKEN1_CORE = 0x%x \n",
             val, val, OMAP35XX_CM_ICLKEN1_CORE);            

    /* set the internal Fclk as the CLKS source */     
    val = GPIO_REGISTER_READ(OMAP35XX_CONTROL_DEVCONF0);
    val = val&(~CONTROL_DEVCONF0_MCBSP1_CLKS);
    val &= (~CONTROL_DEVCONF0_MCBSP1_CLKR); 
    val &= (~CONTROL_DEVCONF0_MCBSP1_FSR);
    PDEBUG(": val_OMAP35XX_CONTROL_DEVCONF0 = %d , 0x%x ; OMAP35XX_CONTROL_DEVCONF0 = 0x%x \n",
             val, val, OMAP35XX_CONTROL_DEVCONF0);            
    GPIO_REGISTER_WRITE(OMAP35XX_CONTROL_DEVCONF0, val);

    /* clkr/fsr */ 
    val = GPIO_REGISTER_READ((CONTROL_PADCONF_X+PADCONF_CLKR));
    val &= PADCONF_MODE_0;
    val |= INPUTENABLE_FSR;
    val |= INPUTENABLE_CLKR;
    val &= (~PULLTYPESELECT_FSR);
    val &= (~PULLUDENABLE_FSR);
    PDEBUG(": val = %d , 0x%x ; (CONTROL_PADCONF_X+PADCONF_CLKR) = 0x%x \n",
             val, val,(CONTROL_PADCONF_X+PADCONF_CLKR));            
    GPIO_REGISTER_WRITE((CONTROL_PADCONF_X+PADCONF_CLKR), val);

    val = GPIO_REGISTER_READ((CONTROL_PADCONF_X+PADCONF_DX));
    val &= PADCONF_MODE_0;
    val |= INPUTENABLE_DR;
    PDEBUG(": val = %d , 0x%x ; (CONTROL_PADCONF_X+PADCONF_DX) = 0x%x \n",
             val, val,(CONTROL_PADCONF_X+PADCONF_DX));            
    GPIO_REGISTER_WRITE((CONTROL_PADCONF_X+PADCONF_DX), val);

    val = GPIO_REGISTER_READ((CONTROL_PADCONF_X+PADCONF_CLKS));
    val &= PADCONF_MODE_0;
    val |= INPUTENABLE_FSX;
    PDEBUG(": val = %d , 0x%x ; (CONTROL_PADCONF_X+PADCONF_CLKS) = 0x%x \n",
             val, val,(CONTROL_PADCONF_X+PADCONF_CLKS));            
    GPIO_REGISTER_WRITE((CONTROL_PADCONF_X+PADCONF_CLKS), val);

    val = GPIO_REGISTER_READ((CONTROL_PADCONF_X+PADCONF_CLKX));
    val &= PADCONF_MODE_0;
    val |= INPUTENABLE_CLKX;
    PDEBUG(": val = %d , 0x%x ; (CONTROL_PADCONF_X+PADCONF_CLKX) = 0x%x \n",
             val, val,(CONTROL_PADCONF_X+PADCONF_CLKX));            
    GPIO_REGISTER_WRITE((CONTROL_PADCONF_X+PADCONF_CLKX), val);
    return MCBSP_DRV_OK;
}

int mcbsp_drv_configure(MCBSP_INIT_CONFIG_FULL* mcbspStruct)
{
    int val;

    /* enable McBsp port Fclk and Iclk, select FCLK as MCBSP1_CLKS source */
    mcbsp_drv_enable_clk();

    /* reset McBsp module */
    mcbsp_drv_reset();

    /* config Rx mode */
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_PCR_REG);
    val = val&(~PCR_FSRM)&(~PCR_CLKRM)&(~PCR_FSRP)&(~PCR_CLKRP);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_PCR_REG, val
            |((mcbspStruct->fsrm) << PCR_FSRM_BIT)
            |((mcbspStruct->clkrm) << PCR_CLKRM_BIT)
            |((mcbspStruct->fsrp) << PCR_FSRP_BIT)
            |((mcbspStruct->clkrp) << PCR_CLKRP_BIT));  

    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_RCCR_REG);
    val = val&(~RCCR_RFULL_CYCLE);
    val = val|(mcbspStruct->rFullCycle << RCCR_RFULL_CYCLE_BIT);
#ifdef MCBSP_DMA
    val = val&(~RCCR_RDMAEN);
    val = val|(mcbspStruct->rdmaen << RCCR_RDMAEN_BIT);
#endif
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_RCCR_REG, val); 

    /* config Tx mode */
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_PCR_REG);
    val = val&(~PCR_FSXM)&(~PCR_CLKXM)&(~PCR_FSXP)&(~PCR_CLKXP);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_PCR_REG, val
            |((mcbspStruct->fsxm)<<PCR_FSXM_BIT)
            |((mcbspStruct->clkxm)<<PCR_CLKXM_BIT)
            |((mcbspStruct->fsxp)<<PCR_FSXP_BIT)
            |((mcbspStruct->clkxp)<<PCR_CLKXP_BIT));  

    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_XCCR_REG);
    val = val&(~RCCR_RFULL_CYCLE)&(~XCCR_EXTCLKGATE);
#ifdef MCBSP_DMA
    val = val&(~XCCR_XDMAEN);
    val = val|(mcbspStruct->xdmaen << XCCR_XDMAEN_BIT);
#endif
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_XCCR_REG, val
            |(mcbspStruct->xFullCycle << RCCR_RFULL_CYCLE_BIT)
            |(mcbspStruct->extClkGate << XCCR_EXTCLKGATE_BIT));

    /* config Rx Frames */
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_RCR1_REG, 
            ((mcbspStruct->rfLen1-1) << RCR1_RFRLEN1_BIT)
            |(mcbspStruct->rwdLen1   << RCR1_RWDLEN1_BIT));

    /* OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_RCR1_REG, 0); */
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_RCR2_REG, 
            ((mcbspStruct->rPhase) << RCR2_RPHASE_BIT)
            |(mcbspStruct->rfLen2 << RCR2_RFRLEN2_BIT)
            |(mcbspStruct->rwdLen2 << RCR2_RWDLEN2_BIT)
            |(mcbspStruct->rReverse << RCR2_RREVERSE_BIT)
            |(mcbspStruct->rDatDly << RCR2_RDATDLY_BIT));   

    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_RCR2_REG);
    val = val&(~SPCR1_RJUST_MASK);
    val = val|((mcbspStruct->rJust) << SPCR1_RJUST_BIT);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_RCR2_REG, val);

    /* config Tx Frames */
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_XCR1_REG, 
            ((mcbspStruct->xfLen1-1) << XCR1_XFRLEN1_BIT)
            |(mcbspStruct->xwdLen1 << XCR1_XWDLEN1_BIT));

    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_XCR2_REG, 
            ((mcbspStruct->xPhase) << XCR2_XPHASE_BIT)
            |(mcbspStruct->xfLen2 << XCR2_XFRLEN2_BIT)
            |(mcbspStruct->xwdLen2 << XCR2_XWDLEN2_BIT)
            |(mcbspStruct->xReverse << XCR2_XREVERSE_BIT)
            |(mcbspStruct->xDatDly << XCR2_XDATDLY_BIT));    

    /* config SRG */
    val = ((mcbspStruct->fWid) << SRGR1_FWID_BIT)
        |((mcbspStruct->clkGdv) << SRGR1_CLKGDV_BIT);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SRGR1_REG, val);

    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SRGR2_REG);
    val = val&(~SRGR2_GSYNC)&(~SRGR2_CLKSP)&(~SRGR2_FPER_MASK)&(~SRGR2_FSGM);
    val = val|((mcbspStruct->gSync) << SRGR2_GSYNC_BIT)
        |((mcbspStruct->clkSp) << SRGR2_CLKSP_BIT)
        |((mcbspStruct->fsgm) << SRGR2_FSGM_BIT)
        |((mcbspStruct->fPer - 1) << SRGR2_FPER_BIT);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SRGR2_REG, val);

    /* set the SRG input clock , CLKSM & CLKME */
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_PCR_REG);
    val = val&(~PCR_SCLKME);
    val = val|((mcbspStruct->sclkMe) << PCR_SCLKME_BIT);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_PCR_REG, val);

    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SRGR2_REG);
    val = val&(~SRGR2_CLKSM);
    val = val|((mcbspStruct->clkSm) << SRGR2_CLKSM_BIT);
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SRGR2_REG, val); 

    return MCBSP_DRV_OK;
}

#ifdef MCBSP_RX_INTERRUPT
int mcbsp_drv_readl(int* readBuffer)
{
    int val;
#ifdef SUPPORT_SPCR1_RSYNC_ERROR
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR1_REG);
    if (val & SPCR1_RSYNCERR) {
        /* clean rsyncerr */
        OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR1_REG, val&(~SPCR1_RSYNCERR));
        PERROR("SPCR1_RSYNCERROR\n");
    }
#endif
    val = 0;
    while (!(OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR1_REG)&SPCR1_RRDY)) {
        if (val++ > MCBSP_ACCESS_RETRY) {
            /* reset and reenable Rx */
            val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR1_REG);
            OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR1_REG, val&(~SPCR1_RRST));
            OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR1_REG, val|SPCR1_RRST); 
            PDEBUG("MCBSP READ FAILED\n");
            return -2;
        }
    }

    *readBuffer = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_DRR_REG);
    return MCBSP_DRV_OK;
}
#endif

#ifdef MCBSP_TX_INTERRUPT
int mcbsp_drv_writel(int* writeBuffer)
{
    int val;
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_DXR_REG, *writeBuffer); 

    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR2_REG);
    if (val & SPCR2_XSYNCERR) {
        OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR2_REG, val&(~SPCR2_XSYNCERR));
        PERROR("SPCR2_XSYNCERR\n");
    }

    val = 0;
    while (!(OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR2_REG)&SPCR2_XRDY)) {
        if (val++ > MCBSP_ACCESS_RETRY) {
            /* reset and reenable Tx */
            val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_SPCR2_REG);
            OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR2_REG, val&(~SPCR2_XRST));
            OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_SPCR2_REG, val|SPCR2_XRST);
            PERROR("MCBSP WRITE FAILED\n");
            return MCBSP_DRV_ERROR;
        }
    }
    return MCBSP_DRV_OK;
}
#endif

#ifdef DEBUG_LOOP_TEST
static void mcbsp_drv_self_test(void)
{
    int i;
    char loopnumber = 20;
    PDEBUG(": ############# IN TEST: WRITING.... ###############\n");
    mcbsp_drv_dump_registers();
    for(i = 0 ;i < loopnumber; i++) {
        mcbsp_drv_writel(&i);
    }
}
#endif

#ifdef MCBSP_RX_INTERRUPT
static irqreturn_t mcbsp_drv_rx_irq_handler(int irq, void *dev_id)
{
    struct mcbsp_driver *drv = (struct mcbsp_driver*) dev_id;
    int val  = 0;
    int buff = 0;
    if (mcbsp_drv_readl(&buff) == MCBSP_DRV_ERROR) {
        PERROR("INTERRUPT ERROR\n");
    } else { 
        drv->parse_read(&buff, drv->dev);
    }
    val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_IRQSTATUS_REG);
    PDEBUG("buff=%x irq_status=%x\n", buff, val);
	OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_IRQSTATUS_REG, val);
	return IRQ_HANDLED;
}

static void mcbsp_drv_free_interrupt(void)
{
    free_irq(MCBSP_DRV_RX_IRQ, &mcbsp_drv);
}
#endif

void mcbsp_drv_tx_threshold(int threshold_value )
{
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_THRSH2_REG, threshold_value);
}

void mcbsp_drv_rx_threshold(int threshold_value )
{
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_THRSH1_REG, threshold_value);
}

static void mcbsp_drv_set_interrupt(void)
{
	MCBSP_INTERRUPTS_ENABLE mcbspIntrEnable;
	int val;

#ifdef SUPPORT_SPCR1_RSYNC_ERROR
	mcbsp_drv_tx_threshold(0x10);
	mcbsp_drv_rx_threshold(0x10);
#endif

	mcbspIntrEnable.x_empty_eof_en = MCBSP_INTR_OFF;  
	mcbspIntrEnable.x_ovfl_en      = MCBSP_INTR_OFF;
	mcbspIntrEnable.x_undfl_en     = MCBSP_INTR_OFF;
	mcbspIntrEnable.x_rdy_en       = MCBSP_INTR_OFF;
	mcbspIntrEnable.x_eof_en       = MCBSP_INTR_OFF;
	mcbspIntrEnable.x_fsx_en       = MCBSP_INTR_OFF;
	mcbspIntrEnable.x_sync_err_en  = MCBSP_INTR_OFF;			 
	mcbspIntrEnable.r_ovfl_en      = MCBSP_INTR_ON;
	mcbspIntrEnable.r_undf_en      = MCBSP_INTR_ON;
	mcbspIntrEnable.r_rdy_en       = MCBSP_INTR_ON;
	mcbspIntrEnable.r_eof_en       = MCBSP_INTR_ON;
	mcbspIntrEnable.r_fsr_en       = MCBSP_INTR_ON;
#ifdef SUPPORT_SPCR1_RSYNC_ERROR
	mcbspIntrEnable.r_sync_err_en  = MCBSP_INTR_ON;
#else
	mcbspIntrEnable.r_sync_err_en  = MCBSP_INTR_OFF;
#endif

	val = OMAP_MCBSP1_REGISTER_READ(MCBSPLP_IRQENABLE_REG);
    val |=((mcbspIntrEnable.x_empty_eof_en)  << IRQENABLE_XEMPTYEOFEN_BIT )
        | ((mcbspIntrEnable.x_ovfl_en)       << IRQENABLE_XOVFLSTATEN_BIT )
        | ((mcbspIntrEnable.x_undfl_en)      << IRQENABLE_XUNDFLSTATEN_BIT )	                             
        | ((mcbspIntrEnable.x_rdy_en)        << IRQENABLE_XRDYEN_BIT )
        | ((mcbspIntrEnable.x_eof_en)        << IRQENABLE_XEOFEN_BIT )
        | ((mcbspIntrEnable.x_fsx_en)        << IRQENABLE_XFSXEN_BIT )
        | ((mcbspIntrEnable.x_sync_err_en)   << IRQENABLE_XSYNCERREN_BIT ) 
        | ((mcbspIntrEnable.r_ovfl_en) 		 << IRQENABLE_ROVFLSTATEN_BIT )
        | ((mcbspIntrEnable.r_undf_en) 		 << IRQENABLE_RUNDFLSTATEN_BIT )
        | ((mcbspIntrEnable.r_rdy_en) 		 << IRQENABLE_RRDYEN_BIT )
        | ((mcbspIntrEnable.r_eof_en) 		 << IRQENABLE_REOFEN_BIT )
        | ((mcbspIntrEnable.r_fsr_en) 		 << IRQENABLE_RFSREN_BIT )
        | ((mcbspIntrEnable.r_sync_err_en) 	 << IRQENABLE_RSYNCERREN_BIT ) 
        ;  
        
#ifdef MCBSP_RX_INTERRUPT
    OMAP_MCBSP1_REGISTER_WRITE(MCBSPLP_IRQENABLE_REG, val); 
    val = request_irq(MCBSP_DRV_RX_IRQ, mcbsp_drv_rx_irq_handler, 0, "McBSP", &mcbsp_drv);
    if (val != 0) {
        /* TODO: Do we need to stop driver and exit???? */
        PERROR("CANNOT SETUP IRQ HANDLING\n");
    } else {
        PDEBUG(" IRQ handling established\n");
    }
#endif
}

static void mcbsp_drv_setup(void)
{
    MCBSP_INIT_CONFIG_FULL mcbspStruct;
    mcbspStruct.port = MCBSP1;

    mcbspStruct.fsrm        = SYNC_EXTERNAL; 
    mcbspStruct.clkrm       = CLK_EXTERNAL; 
    mcbspStruct.fsrp        = SYNC_ACTIVE_HIGH;
    mcbspStruct.clkrp       = CLKR_SAMPLE_FALL;
    mcbspStruct.rFullCycle  = FULL_CYCLE;

    mcbspStruct.fsxm        = SYNC_INTERNAL;
    mcbspStruct.clkxm       = CLK_INTERNAL;
    mcbspStruct.fsxp        = SYNC_ACTIVE_HIGH;
    mcbspStruct.clkxp       = CLKX_DRIVE_RISE;
    mcbspStruct.xFullCycle  = FULL_CYCLE;
    mcbspStruct.extClkGate  = FSX_GATED;

    mcbspStruct.rfLen1      = FRAME1_WORDS;
    mcbspStruct.rwdLen1     = WORD_32BITS;
    mcbspStruct.rPhase      = SINGLE_PHASE;
    mcbspStruct.rfLen2      = FRAME2_WORDS;
    mcbspStruct.rwdLen2     = WORD_32BITS;
    mcbspStruct.rReverse    = MSB_FIRST;
    mcbspStruct.rDatDly     = DELAY_1BIT;
    mcbspStruct.rJust       = RIGHT_JUST_ZERO;

    mcbspStruct.xfLen1      = FRAME1_WORDS;
    mcbspStruct.xwdLen1     = WORD_32BITS;
    mcbspStruct.xPhase      = SINGLE_PHASE;
    mcbspStruct.xfLen2      = FRAME2_WORDS;
    mcbspStruct.xwdLen2     = WORD_32BITS;
    mcbspStruct.xReverse    = MSB_FIRST;
    mcbspStruct.xDatDly     = DELAY_1BIT;

    mcbspStruct.fWid        = 1;
    mcbspStruct.clkGdv      = 0x01; /* 0..255 */

    mcbspStruct.gSync       = SRG_FREE;
    mcbspStruct.clkSp       = CLKS_RISE;
    mcbspStruct.fPer        = 300;
    mcbspStruct.fsgm        = 0;

    mcbspStruct.sclkMe      = 0; 
    mcbspStruct.clkSm       = 0;

#ifdef MCBSP_DMA
    mcbspStruct.rdmaen      = 1;
    mcbspStruct.xdmaen      = 1;
#endif
    mcbsp_drv_configure(&mcbspStruct);
    mcbsp_drv_set_interrupt();
    mcbsp_drv_digitial_loopback(FALSE);
#if (defined(MCBSP_DMA) && !defined(MCBSP_RX_INTERRUPT))
    mcbsp_dma_setup();
#endif
    mcbsp_drv_analog_loopback(FALSE); 
    mcbsp_drv_release_reset(); 
}

#ifdef MCBSP_DMA
void mcbsp_stop_rx(struct mcbsp_dev *dev)
{
	unsigned long flags;
	spin_lock_irqsave(&dev->rx_msg_list_lock, flags);
	dev->rx_stop = 1;
	spin_unlock_irqrestore(&dev->rx_msg_list_lock, flags);
	complete(&rx_dma_completion);
}

int mcbsp_is_rx_stop(struct mcbsp_dev *dev)
{
	int rx_stop = 0;
	unsigned long flags;
	spin_lock_irqsave(&dev->rx_msg_list_lock, flags);
	rx_stop = dev->rx_stop; //TODO - add initalizer
	spin_unlock_irqrestore(&dev->rx_msg_list_lock, flags);
	return rx_stop;
}

void mcbsp_rx_work_dma_handler(struct work_struct *work)
{
	int len;
	struct mcbsp_dev *dev = container_of(work, struct mcbsp_dev, rx_work_dma);

	while (!mcbsp_is_rx_stop(dev))
	{
		if (dev->read_parse_status == MCBSP_PACKAGE_READING_DATA && dev->read_parse_length != -1)
			len = dev->read_parse_length;
		else
			len = 1;

		init_completion(&rx_dma_completion);
		mcbsp_read_dma(dev, len);
		wait_for_completion(&rx_dma_completion);

		dev->driver->parse_read((u32*)dev->mcbsp_bufdst, dev);
	}

	PDEBUG_DMA("exit mcbsp_rx_work_dma_handler\n");
}

static void mcbsp_rx_dma_callback(int lch, u16 ch_status, void *data)
{
	complete(&rx_dma_completion);
}

static void mcbsp_release_dma(void)
{
	struct mcbsp_dev *mcbsp = mcbsp_drv.dev;

	omap_free_dma(mcbsp->dma_rx_lch);
	mcbsp->dma_rx_lch = -1;

	omap_free_dma(mcbsp->dma_tx_lch);
	mcbsp->dma_tx_lch = -1;
}

static void mcbsp_read_dma(struct mcbsp_dev *mcbsp, int len)
{
	omap_set_dma_transfer_params(mcbsp->dma_rx_lch,
					OMAP_DMA_DATA_TYPE_S32,
					len == 1 ? 1 : len >> 1 , 1,
					OMAP_DMA_SYNC_ELEMENT,
					mcbsp->dma_rx_sync, 0);

	omap_set_dma_src_params(mcbsp->dma_rx_lch,
				0,
				OMAP_DMA_AMODE_CONSTANT,
				OMAP_MCBSP_1_BASE + OMAP_MCBSP_REG_DRR,
				0, 0);


	omap_set_dma_dest_params(mcbsp->dma_rx_lch,
					0,
					OMAP_DMA_AMODE_POST_INC ,
					mcbsp->mcbsp_physdst,
					0, 0);

	omap_start_dma(mcbsp->dma_rx_lch);
}

static void mcbsp_tx_dma_callback(int lch, u16 ch_status, void *data)
{
	complete(&tx_dma_completion);
}

static int mcbsp_dma_setup(void)
{
	struct mcbsp_dev *mcbsp = mcbsp_drv.dev;
	int dma_rx_ch;
	int dma_tx_ch;

	mcbsp->dma_rx_sync = OMAP24XX_DMA_MCBSP1_RX;
	mcbsp->dma_tx_sync = OMAP24XX_DMA_MCBSP1_TX;

	/* Allocation for TX */
	mcbsp->mcbsp_bufsrc = dma_alloc_coherent(NULL, MCBSP_DMA_BUFF_SIZE, &(mcbsp->mcbsp_physsrc), 0);
	if (!mcbsp->mcbsp_bufsrc) {
		PERROR ("dma_alloc_coherent failed for physsrc\n");
		return -ENOMEM;
	}

	/* Allocation for RX */
	mcbsp->mcbsp_bufdst = dma_alloc_coherent(NULL, MCBSP_DMA_BUFF_SIZE, &(mcbsp->mcbsp_physdst), 0);
	if (! mcbsp->mcbsp_bufdst) {
		PERROR ("dma_alloc_coherent failed for physsrc\n");
		return -ENOMEM;
	}

	if (omap_request_dma(mcbsp->dma_rx_sync, "McBSP RX",
				mcbsp_rx_dma_callback,
				mcbsp,
				&dma_rx_ch)) {
		PERROR("Unable to request DMA channel for RX");
		return -EAGAIN;
	}

	mcbsp->dma_rx_lch = dma_rx_ch;

	if (omap_request_dma(mcbsp->dma_tx_sync, "McBSP TX",
				mcbsp_tx_dma_callback,
				mcbsp,
				&dma_tx_ch)) {

		PERROR("Unable to request DMA channel for TX\n");
		return -EAGAIN;
	}

	mcbsp->dma_tx_lch = dma_tx_ch;

	PDEBUG_DMA("mcbsp->dma_rx_lch=%d\n", mcbsp->dma_rx_lch);
	PDEBUG_DMA("mcbsp->dma_tx_lch=%d\n", mcbsp->dma_tx_lch);

	queue_work(mcbsp->rx_work_queue_dma_ptr, &mcbsp->rx_work_dma);

	return 0;
}

static int mcbsp_drv_write_data(const char *buf, int count, struct mcbsp_dev *dev)
{
	const char *src_pos = buf;

	memcpy((char*) (dev->mcbsp_bufsrc), (int*)&mcbsp_preamble[0] , 4);
	memcpy((char*) (dev->mcbsp_bufsrc+ 4), (int*)&mcbsp_preamble[1] , 4);
	memcpy((char*) (dev->mcbsp_bufsrc+ 8), &count , 4);
	memcpy((char*) (dev->mcbsp_bufsrc+ 12), &count , 4);
	memcpy((char*) (dev->mcbsp_bufsrc+ 16), src_pos, count);

	dev->bufsrc_len = count + 16;

	netif_stop_queue(dev->net);
	queue_work(dev->tx_work_queue_dma_ptr, &dev->tx_work_dma);
	return 0;
}

void mcbsp_tx_work_dma_handler(struct work_struct *work)
{
	struct mcbsp_dev *dev = container_of(work, struct mcbsp_dev, tx_work_dma);
	int length = 0;
	int src_port = 0;
	int dest_port = 0;

	length = dev->bufsrc_len;

	init_completion(&tx_dma_completion);
	omap_set_dma_transfer_params(dev->dma_tx_lch,
					 OMAP_DMA_DATA_TYPE_S32,
					 length >> 1, 1,
					 OMAP_DMA_SYNC_ELEMENT,
					 dev->dma_tx_sync, 0);

	omap_set_dma_dest_params(dev->dma_tx_lch,
				 src_port,
				 OMAP_DMA_AMODE_CONSTANT,
				 OMAP_MCBSP_1_BASE + OMAP_MCBSP_REG_DXR,
				 0, 0);

	omap_set_dma_src_params(dev->dma_tx_lch,
				dest_port,
				OMAP_DMA_AMODE_POST_INC,
				dev->mcbsp_physsrc,
				0, 0);

	omap_start_dma(dev->dma_tx_lch);
	wait_for_completion(&tx_dma_completion);
	netif_wake_queue(dev->net);
	PDEBUG_DMA("WRITTEN %d BYTES\n", length);
}

#else
static int mcbsp_drv_write_data(const char *buf, int count, struct mcbsp_dev *dev)
{
    int length = 0;
    int i, j;
    int blocks = 1;
    const char *src_pos = buf;
    unsigned int elements = 0;
    unsigned int buffer_size = 0;
    unsigned int aligned_buffer_size = sizeof(dev->tx_aligned_buffer);
    
    if (mcbsp_drv_writel((int*)&mcbsp_preamble[0]))
        return length;
    if (mcbsp_drv_writel((int*)&mcbsp_preamble[1]))
        return length;
    if (mcbsp_drv_writel(&count))
        return length;
    if (mcbsp_drv_writel(&count))
        return length;

    if (count > aligned_buffer_size)
        blocks = count/aligned_buffer_size;
    if ((blocks*aligned_buffer_size) < count)
        blocks++;

    buffer_size = (blocks == 1) ? count : aligned_buffer_size;
    for (i = 0; i < blocks; i++) {
        memset((char*) dev->tx_aligned_buffer, 0, aligned_buffer_size);
        memcpy((char*) dev->tx_aligned_buffer, src_pos, buffer_size);
        elements = (((buffer_size>>2)<<2) < buffer_size) ? ((buffer_size>>2)+1) : (buffer_size>>2);
        for (j = 0; j < elements; j++) {
            if (mcbsp_drv_writel(&dev->tx_aligned_buffer[j]) < 0)
                return length;
            length += 4;
        }
        src_pos += buffer_size;
        count -= buffer_size;
        buffer_size = (count > aligned_buffer_size) ? aligned_buffer_size : count;
    }
    PDEBUG("WRITTEN %d BYTES\n", count);
    return length;
}
#endif

static void mcbsp_drv_parse_read_data(u32 *data, struct mcbsp_dev *dev)
{
    switch(dev->read_parse_status) {
        case MCBSP_PACKAGE_WAIT_PREAMBLE_0:
            if (data[0] == mcbsp_preamble[0]){
                dev->read_parse_status = MCBSP_PACKAGE_WAIT_PREAMBLE_1;
                PDEBUG_DMA("MCBSP_PACKAGE_WAIT_PREAMBLE_0\n");
            }
            break;
        case MCBSP_PACKAGE_WAIT_PREAMBLE_1:
            if (data[0] == mcbsp_preamble[1]){
            	PDEBUG_DMA("MCBSP_PACKAGE_WAIT_PREAMBLE_1\n");
                dev->read_parse_status = MCBSP_PACKAGE_WAIT_LENGTH_0;
            }
            else{
            	PDEBUG_DMA("return -> MCBSP_PACKAGE_WAIT_PREAMBLE_0\n");
                dev->read_parse_status = MCBSP_PACKAGE_WAIT_PREAMBLE_0;
            }
            break;
        case MCBSP_PACKAGE_WAIT_LENGTH_0:
        	PDEBUG_DMA("MCBSP_PACKAGE_WAIT_LENGTH_0\n");
            dev->read_parse_length = data[0];
            dev->read_parse_status = MCBSP_PACKAGE_WAIT_LENGTH_1;
            break;
        case MCBSP_PACKAGE_WAIT_LENGTH_1:
        	PDEBUG_DMA("MCBSP_PACKAGE_WAIT_LENGTH_1\n");
            if (dev->read_parse_length != data[0]) {
                dev->read_parse_status = MCBSP_PACKAGE_WAIT_PREAMBLE_0;
            } else {
                int aligned_size = (((dev->read_parse_length>>2)<<2)+4);
                dev->read_parse_status = MCBSP_PACKAGE_READING_DATA;
                dev->read_parse_count = 0;
                dev->skb_reading = dev_alloc_skb(aligned_size+sizeof(struct mcbsp_msg));
                if (dev->skb_reading == NULL) {
                    dev->read_parse_status = MCBSP_PACKAGE_WAIT_PREAMBLE_0;
                } else {
                    dev->skb_reading->len = dev->read_parse_length;
                    PDEBUG_DMA("skb_reading->len=%d, aligned_size=%d\n",dev->read_parse_length, aligned_size);
                    dev->rx_data = skb_put(dev->skb_reading, aligned_size);
                }
            }
            break;
        case MCBSP_PACKAGE_READING_DATA:
#ifdef MCBSP_DMA
        	PDEBUG_DMA("MCBSP_PACKAGE_READING_DATA\n");
        	PDEBUG_DMA("read_parse_length=%d\n", dev->read_parse_length);
			memcpy(dev->rx_data, (char*) data, dev->read_parse_length);
#else
        	memcpy(dev->rx_data+dev->read_parse_count, (char*) data, 4);
			dev->read_parse_count = dev->read_parse_count+4;
			if (dev->read_parse_count >= dev->read_parse_length)
#endif
            {
                unsigned long flags;
                struct mcbsp_msg *msg;

                spin_lock_irqsave(&dev->rx_msg_list_lock, flags);
                msg = (struct mcbsp_msg*)skb_put(dev->skb_reading, sizeof(struct mcbsp_msg));
                msg->skb = dev->skb_reading;
                msg->actual_data_length = dev->read_parse_length;
                list_add_tail(&msg->list, &dev->msg_list);
                spin_unlock_irqrestore(&dev->rx_msg_list_lock, flags);
                
                queue_work(dev->rx_work_queue_ptr, &dev->rx_work);

                dev->read_parse_status = MCBSP_PACKAGE_WAIT_PREAMBLE_0;
                dev->skb_reading = NULL;
                PDEBUG("ENTIRE PACKAGE RECEIVED \n");
                dev->read_parse_length = -1; //TODO - add initializer
            }
            break;
    }
}

static struct device_type mcbsp_net_type = {
	.name	= "mcbspnet",
};

struct mcbsp_driver mcbsp_drv = {
    owner:          THIS_MODULE,
    name:           MCBSP_DRV_DRIVER_NAME,
    write:          mcbsp_drv_write_data,
    parse_read:     mcbsp_drv_parse_read_data,
    read_callback:  mcbsp_netif_rx_handler,
};

static void mcbsp_drv_iomap(void)
{
    OMAP35XX_CM_FCLKEN1_CORE   = (u32) ioremap(OMAP35XX_CM_FCLKEN1_CORE_BASE , 0x0004);
    OMAP35XX_CM_ICLKEN1_CORE   = (u32) ioremap(OMAP35XX_CM_ICLKEN1_CORE_BASE , 0x0004);
    CONTROL_PADCONF_X          = (u32) ioremap(CONTROL_PADCONF_X_BASE        , 0x0010);
    OMAP35XX_CONTROL_DEVCONF0  = (u32) ioremap(OMAP35XX_CONTROL_DEVCONF0_BASE, 0x0004);
    OMAP_MCBSP_1_CORE          = (u32) ioremap(OMAP_MCBSP_1_BASE, OMAP_MCBSP_1_SIZE);
}

static void mcbsp_drv_iounmap(void)
{
    iounmap((void*) OMAP35XX_CM_FCLKEN1_CORE);
    iounmap((void*) OMAP35XX_CM_ICLKEN1_CORE);
    iounmap((void*) OMAP35XX_CONTROL_DEVCONF0);
    iounmap((void*) CONTROL_PADCONF_X);
    iounmap((void*) OMAP_MCBSP_1_CORE);
}

static void mcbsp_drv_set_mcbsp_dev(struct mcbsp_driver *driver,struct mcbsp_dev *dev)
{
    driver->dev = dev;
    dev->driver = driver;
}

inline struct mcbsp_driver* mcbsp_drv_get_driver_from_mcbsp_dev(struct mcbsp_dev *dev)
{
    return dev->driver;
}

static int mcbsp_drv_register_driver(struct mcbsp_driver *drv)
{
    struct mcbsp_dev *dev;

    if ((dev = mcbsp_netif_alloc_dev()) == NULL) {
        PERROR(" CANNOT ALLOCATE NET DEVICE\n");
        return -1;
    }
    mcbsp_drv_set_mcbsp_dev(drv, dev);
	spin_lock_init(&dev->rx_msg_list_lock);
	INIT_LIST_HEAD(&dev->msg_list);
    dev->rx_work_queue_ptr = create_workqueue("rx_mcbsp_queue");
    INIT_WORK(&dev->rx_work, drv->read_callback);

#ifdef MCBSP_DMA
    dev->rx_work_queue_dma_ptr = create_workqueue("rx_mcbsp_queue_dma");
    dev->tx_work_queue_dma_ptr = create_workqueue("tx_mcbsp_queue_dma");
    INIT_WORK(&dev->rx_work_dma, mcbsp_rx_work_dma_handler);
    INIT_WORK(&dev->tx_work_dma, mcbsp_tx_work_dma_handler);
#endif

    PDEBUG("DRIVER IS REGISTERED\n");
    return 0;
}

static void mcbsp_drv_unregister_driver(struct mcbsp_driver *drv)
{
    struct mcbsp_dev *dev = drv->dev;

    destroy_workqueue(dev->rx_work_queue_ptr);
#ifdef MCBSP_DMA
    mcbsp_stop_rx(dev);
    destroy_workqueue(dev->rx_work_queue_dma_ptr);
    destroy_workqueue(dev->tx_work_queue_dma_ptr);
#endif
    PDEBUG("DRIVER IS UNREGISTERED\n");
}

static int __init mcbsp_drv_init(void)
{
    PDEBUG("\n");

    if (mcbsp_drv_register_driver(&mcbsp_drv)) {
        PERROR("CANNOT REGISTER DRIVER\n");
        return -1;
    }
    
    mcbsp_drv_iomap();
    mcbsp_drv_setup();
    mcbsp_drv_dump_registers();
#ifdef DEBUG_LOOP_TEST
    mcbsp_drv_self_test();
#endif    
    mcbsp_netif_setup_netdev(mcbsp_drv.dev, &mcbsp_net_type);    
    return 0;
}

static void __exit mcbsp_drv_exit(void)
{
#ifdef MCBSP_RX_INTERRUPT
    mcbsp_drv_free_interrupt();
#else
    mcbsp_release_dma();
#endif
    mcbsp_drv_iounmap();
    mcbsp_netif_remove_netdev(mcbsp_drv.dev);
    mcbsp_drv_unregister_driver(&mcbsp_drv);
    PDEBUG("\n");
}

module_init(mcbsp_drv_init);
module_exit(mcbsp_drv_exit);

MODULE_DESCRIPTION("OMAP MCBSP NETIF DRIVER");
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Galilsoft");
