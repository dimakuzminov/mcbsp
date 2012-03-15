/*
 * Driver for mcbsp-serial
 *
 * Copyright (C) 2012 Galilsoft.
 *
 * Authors:
 *	Kuzminov Dmitry	<dima@galilsoft.com>
 *
 *
 */
#ifndef _MCBSP_SERIAL_H_
#define _MCBSP_SERIAL_H_

/* MCBSP Module Macro define */
#define MCBSP1                     0
#define MCBSP2                     1
#define MCBSP3                     2
#define MCBSP4                     3
#define MCBSP5                     4

#define MCBSP_DRV_OK      (0)
#define MCBSP_DRV_ERROR   (-1)

#define OMAP35XX_CM_FCLKEN_PER        0x48005000
#define OMAP35XX_CM_ICLKEN_PER        0x48005010

#define OMAP35XX_EN_MCBSP1            (0x1 << 9)
#define OMAP35XX_EN_MCBSP2            (0x1 << 0)
#define OMAP35XX_EN_MCBSP3            (0x1 << 1)
#define OMAP35XX_EN_MCBSP4            (0x1 << 2)
#define OMAP35XX_EN_MCBSP5            (0x1 << 10)

/* MCBSP register define */
#define MCBSPLP_DRR_REG                0x0
#define MCBSPLP_DXR_REG                0x8
#define MCBSPLP_SPCR2_REG              0x10
#define MCBSPLP_SPCR1_REG              0x14
#define MCBSPLP_RCR2_REG               0x18
#define MCBSPLP_RCR1_REG               0x1C
#define MCBSPLP_XCR2_REG               0x20
#define MCBSPLP_XCR1_REG               0x24
#define MCBSPLP_SRGR2_REG              0x28
#define MCBSPLP_SRGR1_REG              0x2C
#define MCBSPLP_MCR2_REG               0x30
#define MCBSPLP_MCR1_REG               0x34
#define MCBSPLP_RCERA_REG              0x38
#define MCBSPLP_RCERB_REG              0x3C
#define MCBSPLP_XCERA_REG              0x40
#define MCBSPLP_XCERB_REG              0x44
#define MCBSPLP_PCR_REG                0x48
#define MCBSPLP_RCERC_REG              0x4C
#define MCBSPLP_RCERD_REG              0x50
#define MCBSPLP_XCERC_REG              0x54
#define MCBSPLP_XCERD_REG              0x58
#define MCBSPLP_RCERE_REG              0x5C
#define MCBSPLP_RCERF_REG              0x60
#define MCBSPLP_XCERE_REG              0x64
#define MCBSPLP_XCERF_REG              0x68
#define MCBSPLP_RCERG_REG              0x6C
#define MCBSPLP_RCERH_REG              0x70
#define MCBSPLP_XCERG_REG              0x74
#define MCBSPLP_XCERH_REG              0x78
#define MCBSPLP_REV_REG                0x7C
#define MCBSPLP_RINTCLR_REG            0x80
#define MCBSPLP_XINTCLR_REG            0x84
#define MCBSPLP_ROVFLCLR_REG           0x88
#define MCBSPLP_SYSCONFIG_REG          0x8C
#define MCBSPLP_THRSH2_REG             0x90
#define MCBSPLP_THRSH1_REG             0x94
#define MCBSPLP_IRQSTATUS_REG          0xA0
#define MCBSPLP_IRQENABLE_REG          0xA4
#define MCBSPLP_WAKEUPEN_REG           0xA8
#define MCBSPLP_XCCR_REG               0xAC
#define MCBSPLP_RCCR_REG               0xB0
#define MCBSPLP_XBUFFSTAT_REG          0xB4
#define MCBSPLP_RBUFFSTAT_REG          0xB8
#define MCBSPLP_SSELCR_REG             0xBC
#define MCBSPLP_STATUS_REG             0xc0

/* CONTROL DEVCONF0 bit define */
#define CONTROL_DEVCONF0_MCBSP2_CLKS      0x40
#define CONTROL_DEVCONF0_MCBSP1_FSR       0x10
#define CONTROL_DEVCONF0_MCBSP1_CLKR      0x8
#define CONTROL_DEVCONF0_MCBSP1_CLKS      0x4
#define CONTROL_DEVCONF0_SENSDMAREQ1      0x2
#define CONTROL_DEVCONF0_SENSDMAREQ0      0x1

/* CONTROL_PADCONF */
#define BYTE_1 (0*8)
#define BYTE_2 (1*8)
#define BYTE_3 (2*8)
#define BYTE_4 (3*8)

#define INPUTENABLE_FSR    (1 << (8+BYTE_3))
#define INPUTENABLE_CLKR   (1 << (8+BYTE_1))
#define PULLTYPESELECT_FSR (1 << (4+BYTE_3))
#define PULLUDENABLE_FSR   (1 << (3+BYTE_3))
#define INPUTENABLE_DR     (1 << (8+BYTE_3))
#define INPUTENABLE_DX     (1 << (8+BYTE_1))
#define INPUTENABLE_FSX    (1 << (8+BYTE_3))
#define INPUTENABLE_CLKX   (1 << (8+BYTE_1))

#define PADCONF_CLKR    0x00
#define PADCONF_DX      0x04
#define PADCONF_CLKS    0x08
#define PADCONF_CLKX    0x0c
#define PADCONF_MODE_0  0xfff8fff8

/* CONTROL DEVCONF1 bit define */
#define CONTROL_DEVCONF1_MCBSP5_CLKS      0x10
#define CONTROL_DEVCONF1_MCBSP4_CLKS      0x4
#define CONTROL_DEVCONF1_MCBSP3_CLKS      0x1

/* MCBSPLP_SPCR2_REG bit define */
#define SPCR2_FREE                     0x0200
#define SPCR2_SOFT                     0x0100
#define SPCR2_FRST                     0x0080
#define SPCR2_GRST                     0x0040
#define SPCR2_XINTM_1                  0x0020
#define SPCR2_XINTM_0                  0x0010
#define SPCR2_XSYNCERR                 0x0008
#define SPCR2_XEMPTY                   0x0004
#define SPCR2_XRDY                     0x0002
#define SPCR2_XRST                     0x0001

/* MCBSPLP_SPCR1_REG bit define */
#define SPCR1_ALB                      0x8000
#define SPCR1_RJUST_1                  0x4000
#define SPCR1_RJUST_0                  0x2000
#define SPCR1_DXENA                    0x0080
#define SPCR1_RINTM_1                  0x0020
#define SPCR1_RINTM_0                  0x0010
#define SPCR1_RSYNCERR                 0x0008
#define SPCR1_RFULL                    0x0004
#define SPCR1_RRDY                     0x0002
#define SPCR1_RRST                     0x0001

#define SPCR1_ALB_BIT                  15
#define SPCR1_RJUST_BIT                13
#define SPCR1_DXENA_BIT                7
#define SPCR1_RINTM_BIT                4
#define SPCR1_RSYNCERR_BIT             3
#define SPCR1_RFULL_BIT                2
#define SPCR1_RRDY_BIT                 1
#define SPCR1_RRST_BIT                 0
#define SPCR1_RJUST_MASK               0x6000

/* MCBSPLP_RCR2_REG bit define */
#define RCR2_RPHASE_BIT                15
#define RCR2_RFRLEN2_BIT               8
#define RCR2_RWDLEN2_BIT               5
#define RCR2_RREVERSE_BIT              3
#define RCR2_RDATDLY_BIT               0

/* MCBSPLP_RCR1_REG bit define */
#define RCR1_RFRLEN1_BIT               8
#define RCR1_RWDLEN1_BIT               5

/* MCBSPLP_XCR2_REG bit define */
#define XCR2_XPHASE                   0x8000
#define XCR2_XFRLEN2_6                0x4000
#define XCR2_XFRLEN2_5                0x2000
#define XCR2_XFRLEN2_4                0x1000
#define XCR2_XFRLEN2_3                0x0800
#define XCR2_XFRLEN2_2                0x0400
#define XCR2_XFRLEN2_1                0x0200
#define XCR2_XFRLEN2_0                0x0100
#define XCR2_XWDLEN2_2                0x0080
#define XCR2_XWDLEN2_1                0x0040
#define XCR2_XWDLEN2_0                0x0020
#define XCR2_XREVERSE_1               0x0010
#define XCR2_XREVERSE_0               0x0008
#define XCR2_XDATDLY_1                0x0002
#define XCR2_XDATDLY_0                0x0001

#define XCR2_XPHASE_BIT               15
#define XCR2_XFRLEN2_BIT              8
#define XCR2_XWDLEN2_BIT              5
#define XCR2_XREVERSE_BIT             3
#define XCR2_XDATDLY_BIT              0

/* MCBSPLP_XCR1_REG bit define */
#define XCR1_XFRLEN1_6                0x4000
#define XCR1_XFRLEN1_5                0x2000
#define XCR1_XFRLEN1_4                0x1000
#define XCR1_XFRLEN1_3                0x0800
#define XCR1_XFRLEN1_2                0x0400
#define XCR1_XFRLEN1_1                0x0200
#define XCR1_XFRLEN1_0                0x0100
#define XCR1_XWDLEN1_2                0x0080
#define XCR1_XWDLEN1_1                0x0040
#define XCR1_XWDLEN1_0                0x0020

#define XCR1_XFRLEN1_BIT              8
#define XCR1_XWDLEN1_BIT              5

/* MCBSPLP_SRGR2_REG bit define */
#define SRGR2_GSYNC                   (0x8000)
#define SRGR2_CLKSP                   (0x4000)
#define SRGR2_CLKSM                   (0x2000)
#define SRGR2_FSGM                    (0x1000)
#define SRGR2_FPER_b                  (0x0800)
#define SRGR2_FPER_a                  (0x0400)
#define SRGR2_FPER_9                  (0x0200)
#define SRGR2_FPER_8                  (0x0100)
#define SRGR2_FPER_7                  (0x0080)
#define SRGR2_FPER_6                  (0x0040)
#define SRGR2_FPER_5                  (0x0020)
#define SRGR2_FPER_4                  (0x0010)
#define SRGR2_FPER_3                  (0x0008)
#define SRGR2_FPER_2                  (0x0004)
#define SRGR2_FPER_1                  (0x0002)
#define SRGR2_FPER_0                  (0x0001)
#define SRGR2_FPER_MASK               (0xFFF)
#define SRGR2_FPER_BIT                (0)

#define SRGR2_GSYNC_BIT               (15)
#define SRGR2_CLKSP_BIT               (14)
#define SRGR2_CLKSM_BIT               (13)
#define SRGR2_FSGM_BIT                (12)

/* MCBSPLP_SRGR1_REG bit define */
#define SRGR1_FWID_7                  (0x8000)
#define SRGR1_FWID_6                  (0x4000)
#define SRGR1_FWID_5                  (0x2000)
#define SRGR1_FWID_4                  (0x1000)
#define SRGR1_FWID_3                  (0x0800)
#define SRGR1_FWID_2                  (0x0400)
#define SRGR1_FWID_1                  (0x0200)
#define SRGR1_FWID_0                  (0x0100)
#define SRGR1_CLKGDV_7                (0x0080)
#define SRGR1_CLKGDV_6                (0x0040)
#define SRGR1_CLKGDV_5                (0x0020)
#define SRGR1_CLKGDV_4                (0x0010)
#define SRGR1_CLKGDV_3                (0x0008)
#define SRGR1_CLKGDV_2                (0x0004)
#define SRGR1_CLKGDV_1                (0x0002)
#define SRGR1_CLKGDV_0                (0x0001)
#define SRGR1_FWID_MASK               (0xFF00)
#define SRGR1_CLKGDV_MASK             (0x00FF)
#define SRGR1_FWID_BIT                (8)
#define SRGR1_CLKGDV_BIT              (0)

/* MCBSPLP_MCR2_REG bit define */
#define MCR2_XMCME                    (0x0200)
#define MCR2_XPBBLK_1                 (0x0100)
#define MCR2_XPBBLK_0                 (0x0080)
#define MCR2_XPABLK_1                 (0x0040)
#define MCR2_XPABLK_0                 (0x0020)
#define MCR2_XMCM_1                   (0x0002)
#define MCR2_XMCM_0                   (0x0001)

/* MCBSPLP_MCR1_REG bit define */
#define MCR1_RMCME                    (0x0200)
#define MCR1_RPBBLK_1                 (0x0100)
#define MCR1_RPBBLK_0                 (0x0080)
#define MCR1_RPABLK_1                 (0x0040)
#define MCR1_RPABLK_0                 (0x0020)
#define MCR1_RMCM                     (0x0001)

/* MCBSPLP_RCERA_REG bit define */
#define RCERA_F                       (0x8000)
#define RCERA_E                       (0x4000)
#define RCERA_D                       (0x2000)
#define RCERA_C                       (0x1000)
#define RCERA_B                       (0x0800)
#define RCERA_A                       (0x0400)
#define RCERA_9                       (0x0200)
#define RCERA_8                       (0x0100)
#define RCERA_7                       (0x0080)
#define RCERA_6                       (0x0040)
#define RCERA_5                       (0x0020)
#define RCERA_4                       (0x0010)
#define RCERA_3                       (0x0008)
#define RCERA_2                       (0x0004)
#define RCERA_1                       (0x0002)
#define RCERA_0                       (0x0001)

/* MCBSPLP_RCERB_REG bit define */
#define RCERB_F                       (0x8000)
#define RCERB_E                       (0x4000)
#define RCERB_D                       (0x2000)
#define RCERB_C                       (0x1000)
#define RCERB_B                       (0x0800)
#define RCERB_A                       (0x0400)
#define RCERB_9                       (0x0200)
#define RCERB_8                       (0x0100)
#define RCERB_7                       (0x0080)
#define RCERB_6                       (0x0040)
#define RCERB_5                       (0x0020)
#define RCERB_4                       (0x0010)
#define RCERB_3                       (0x0008)
#define RCERB_2                       (0x0004)
#define RCERB_1                       (0x0002)
#define RCERB_0                       (0x0001)

/* MCBSPLP_XCERA_REG bit define */
#define XCERA_F                       (0x8000)
#define XCERA_E                       (0x4000)
#define XCERA_D                       (0x2000)
#define XCERA_C                       (0x1000)
#define XCERA_B                       (0x0800)
#define XCERA_A                       (0x0400)
#define XCERA_9                       (0x0200)
#define XCERA_8                       (0x0100)
#define XCERA_7                       (0x0080)
#define XCERA_6                       (0x0040)
#define XCERA_5                       (0x0020)
#define XCERA_4                       (0x0010)
#define XCERA_3                       (0x0008)
#define XCERA_2                       (0x0004)
#define XCERA_1                       (0x0002)
#define XCERA_0                       (0x0001)

/* MCBSPLP_XCERB_REG bit define */
#define XCERB_F                       (0x8000)
#define XCERB_E                       (0x4000)
#define XCERB_D                       (0x2000)
#define XCERB_C                       (0x1000)
#define XCERB_B                       (0x0800)
#define XCERB_A                       (0x0400)
#define XCERB_9                       (0x0200)
#define XCERB_8                       (0x0100)
#define XCERB_7                       (0x0080)
#define XCERB_6                       (0x0040)
#define XCERB_5                       (0x0020)
#define XCERB_4                       (0x0010)
#define XCERB_3                       (0x0008)
#define XCERB_2                       (0x0004)
#define XCERB_1                       (0x0002)
#define XCERB_0                       (0x0001)

/* MCBSPLP_PCR_REG bit define */
#define PCR_IDLE_EN                   (0x4000)
#define PCR_XIOEN                     (0x2000)
#define PCR_RIOEN                     (0x1000)
#define PCR_FSXM                      (0x0800)
#define PCR_FSRM                      (0x0400)
#define PCR_CLKXM                     (0x0200)
#define PCR_CLKRM                     (0x0100)
#define PCR_SCLKME                    (0x0080)
#define PCR_CLKS_STAT                 (0x0040)
#define PCR_DX_STAT                   (0x0020)
#define PCR_DR_STAT                   (0x0010)
#define PCR_FSXP                      (0x0008)
#define PCR_FSRP                      (0x0004)
#define PCR_CLKXP                     (0x0002)
#define PCR_CLKRP                     (0x0001)

#define PCR_IDLE_EN_BIT               (14)
#define PCR_XIOEN_BIT                 (13)
#define PCR_RIOEN_BIT                 (12)
#define PCR_FSXM_BIT                  (11)
#define PCR_FSRM_BIT                  (10)
#define PCR_CLKXM_BIT                 (9)
#define PCR_CLKRM_BIT                 (8)
#define PCR_SCLKME_BIT                (7)
#define PCR_CLKS_STAT_BIT             (6)
#define PCR_DX_STAT_BIT               (5)
#define PCR_DR_STAT_BIT               (4)
#define PCR_FSXP_BIT                  (3)
#define PCR_FSRP_BIT                  (2)
#define PCR_CLKXP_BIT                 (1)
#define PCR_CLKRP_BIT                 (0)

/* MCBSPLP_SYSCONFIG_REG bit define */
#define SYSCONFIG_CLOCKACTIVITY_1   (0x0200)
#define SYSCONFIG_CLOCKACTIVITY_0   (0x0100)
#define SYSCONFIG_SIDLEMODE_1       (0x0010)
#define SYSCONFIG_SIDLEMODE_0       (0x0008)
#define SYSCONFIG_ENAWAKEUP         (0x0004)
#define SYSCONFIG_SOFTRESET         (0x0002)

/* MCBSPLP_IRQSTATUS_REG bit define */
#define IRQSTATUS_XEMPTYEOF         (0x4000)
#define IRQSTATUS_XOVFLSTAT         (0x1000)
#define IRQSTATUS_XUNDFLSTAT        (0x0800)
#define IRQSTATUS_XRDY              (0x0400)
#define IRQSTATUS_XEOF              (0x0200)
#define IRQSTATUS_XFSX              (0x0100)
#define IRQSTATUS_XSYNCERR          (0x0080)
#define IRQSTATUS_ROVFLSTAT         (0x0020)
#define IRQSTATUS_RUNDFLSTAT        (0x0010)
#define IRQSTATUS_RRDY              (0x0008)
#define IRQSTATUS_REOF              (0x0004)
#define IRQSTATUS_RFSR              (0x0002)
#define IRQSTATUS_RSYNCERR          (0x0001)

/* MCBSPLP_IRQENABLE_REG bit define */
#define IRQENABLE_XEMPTYEOFEN       (0x4000)
#define IRQENABLE_XOVFLSTATEN       (0x1000)
#define IRQENABLE_XUNDFLSTATEN      (0x0800)
#define IRQENABLE_XRDYEN            (0x0400)
#define IRQENABLE_XEOFEN            (0x0200)
#define IRQENABLE_XFSXEN            (0x0100)
#define IRQENABLE_XSYNCERREN        (0x0080)
#define IRQENABLE_ROVFLSTATEN       (0x0020)
#define IRQENABLE_RUNDFLSTATEN      (0x0010)
#define IRQENABLE_RRDYEN            (0x0008)
#define IRQENABLE_REOFEN            (0x0004)
#define IRQENABLE_RFSREN            (0x0002)
#define IRQENABLE_RSYNCERREN        (0x0001)

#define IRQENABLE_XEMPTYEOFEN_BIT       (14)
#define IRQENABLE_XOVFLSTATEN_BIT       (12)
#define IRQENABLE_XUNDFLSTATEN_BIT      (11)
#define IRQENABLE_XRDYEN_BIT            (10)
#define IRQENABLE_XEOFEN_BIT            (9)
#define IRQENABLE_XFSXEN_BIT            (8)
#define IRQENABLE_XSYNCERREN_BIT        (7)
#define IRQENABLE_ROVFLSTATEN_BIT       (5)
#define IRQENABLE_RUNDFLSTATEN_BIT      (4)
#define IRQENABLE_RRDYEN_BIT            (3)
#define IRQENABLE_REOFEN_BIT            (2)
#define IRQENABLE_RFSREN_BIT            (1)
#define IRQENABLE_RSYNCERREN_BIT        (0)


/* MCBSPLP_WAKEUPEN_REG bit define */
#define WAKEUPEN_XEMPTYEOFEN        (0x4000)
#define WAKEUPEN_XRDYEN             (0x0400)
#define WAKEUPEN_XEOFEN             (0x0200)
#define WAKEUPEN_XFSXEN             (0x0100)
#define WAKEUPEN_XSYNCERREN         (0x0080)
#define WAKEUPEN_RRDYEN             (0x0008)
#define WAKEUPEN_REOFEN             (0x0004)
#define WAKEUPEN_RFSREN             (0x0002)
#define WAKEUPEN_RSYNCERREN         (0x0001)

/* MCBSPLP_XCCR_REG bit define */
#define XCCR_EXTCLKGATE             (0x8000)
#define XCCR_PPCONECT               (0x4000)
#define XCCR_DXENDLY_1              (0x2000)
#define XCCR_DXENDLY_0              (0x1000)
#define XCCR_XFULL_CYCLE            (0x0800)
#define XCCR_DLB                    (0x0020)
#define XCCR_XDMAEN                 (0x0008)
#define XCCR_XDISABLE               (0x0001)

#define XCCR_EXTCLKGATE_BIT         (15)
#define XCCR_PPCONECT_BIT           (14)
#define XCCR_DXENDLY_BIT            (12)
#define XCCR_XFULL_CYCLE_BIT        (11)
#define XCCR_DLB_BIT                (5)
#define XCCR_XDMAEN_BIT             (3)
#define XCCR_XDISABLE_BIT           (0)

/* MCBSPLP_RCCR_REG bit define */
#define RCCR_RFULL_CYCLE            (0x0800)
#define RCCR_RDMAEN                 (0x0008)
#define RCCR_RDISABLE               (0x0001)

#define RCCR_RFULL_CYCLE_BIT        (11)
#define RCCR_RDMAEN_BIT             (3)
#define RCCR_RDISABLE_BIT           (0)

/* MCBSPLP_XBUFFSTAT_REG
   McBSP2 has 11 bits [10:0]
   Others only have 8 bits [7:0] - TBC */
#define XBUFFSTAT_A                (0x0200)
#define XBUFFSTAT_8                (0x0100)
#define XBUFFSTAT_7                (0x0080)
#define XBUFFSTAT_6                (0x0040)
#define XBUFFSTAT_5                (0x0020)
#define XBUFFSTAT_4                (0x0010)
#define XBUFFSTAT_3                (0x0008)
#define XBUFFSTAT_2                (0x0004)
#define XBUFFSTAT_1                (0x0002)
#define XBUFFSTAT_0                (0x0001)

/* MCBSPLP_RBUFFSTAT_REG
   McBSP2 has 11 bits [10:0]
   Others only have 8 bits [7:0] - TBC */
#define RBUFFSTAT_A                (0x0200)
#define RBUFFSTAT_8                (0x0100)
#define RBUFFSTAT_7                (0x0080)
#define RBUFFSTAT_6                (0x0040)
#define RBUFFSTAT_5                (0x0020)
#define RBUFFSTAT_4                (0x0010)
#define RBUFFSTAT_3                (0x0008)
#define RBUFFSTAT_2                (0x0004)
#define RBUFFSTAT_1                (0x0002)
#define RBUFFSTAT_0                (0x0001)

/* MCBSPLP_STATUS_REG */
#define STATUS_CLKMUXSTATUS        (0x0001)

/* mcbspSelectClksSource parameters */
#define  MCBSP_CLKS_PIN   1     // Set external mcbsp_clks pin as CLKS source
#define  INTERNAL_FCLK    0     // Set intern FCLK as CLKS source


/* mcbspRxFrameSetup & mcbspTxFrameSetup parameters */
#define SINGLE_PHASE                0
#define DUAL_PHASE                  0
#define RIGHT_JUST_ZERO             0
#define RIGHT_JUST_SIGN             1
#define LEFT_JUST_ZERO              2
#define MSB_FIRST                   0
#define LSB_FIRST                   1
#define DELAY_0BIT                  0
#define DELAY_1BIT                  1
#define DELAY_2BIT                  2
#define WORD_8BITS                  0
#define WORD_12BITS                 1 
#define WORD_16BITS                 2 
#define WORD_20BITS                 3 
#define WORD_24BITS                 4 
#define WORD_32BITS                 5 


/* mcbspSelectSrgClockSource parameters */
#define CLKS                        0
#define CLKI                        1
#define CLKR                        2
#define CLKX                        3
#define CLKS_RISE                   0
#define CLKS_FALL                   1
#define SRG_FREE                    0
#define SRG_SYNC                    1


/* mcbspSetTxMode & mcbspSetRxMode parameters */
#define CLK_EXTERNAL                0
#define CLK_INTERNAL                1
#define SYNC_EXTERNAL               0
#define SYNC_INTERNAL               1
#define SYNC_ACTIVE_HIGH            0
#define SYNC_ACTIVE_LOW             1
#define CLKX_DRIVE_RISE             0
#define CLKX_DRIVE_FALL             1
#define CLKR_SAMPLE_RISE            1
#define CLKR_SAMPLE_FALL            0
#define FULL_CYCLE                  1
#define HALF_CYCLE                  0
#define DXENA_OFF                   0
#define DXENA_DLY1                  1
#define DXENA_DLY2                  2
#define DXENA_DLY3                  3
#define DXENA_DLY4                  4
#define XCLK_GATED                  1
#define XCLK_FREE                   0
#define FSX_GATED                   0
#define FSX_FREE                    1

#define NR_MCBSPS                   5

/* Uart Control baud rate select */
#define UCR_BAUD_SEL_9600					0x00  /**< UART 9600 baudrate         */
#define UCR_BAUD_SEL_19200					0x01  /**< UART 19200 baudrate        */
#define UCR_BAUD_SEL_38400					0x02  /**< UART 38400 baudrate        */
#define UCR_BAUD_SEL_57600					0x03  /**< UART 57600 baudrate        */
#define UCR_BAUD_SEL_115200					0x04  /**< UART 115200 baudrate       */
#define UCR_BAUD_SEL_230400					0x05  /**< UART 230400 baudrate       */
#define UCR_BAUD_SEL_460800					0x06  /**< UART 460800 baudrate       */

#define USR_PARITY_EVEN  				    0x08  /**< UART even parity           */
#define USR_PARITY_ODD  				    0x00  /**< UART odd parity            */
#define USR_PARITY_ENABLE				    0x10  /**< UART parity enable         */
#define USR_PARITY_DISABLE					0x00  /**< UART parity disable        */
#define USR_ONE_STOP_BIT					0x00  /**< UART 1 stop bit            */
#define USR_TWO_STOP_BITS					0x20  /**< UART 2 stop bits           */ 

#define DEFAULT_BAUDRATE              UCR_BAUD_SEL_115200  /**< Default baudrate */
#define DEFAULT_PARITY_TYPE			  USR_PARITY_EVEN      /**< Default parity	type */ 
#define DEFAULT_PARITY_USE			  USR_PARITY_DISABLE	/**< Default parity	use  */	
#define DEFAULT_STOPBITS_NUM		  USR_ONE_STOP_BIT     /**< Default number of stopbits*/ 

#define MCBSP1_INTERRUPT		16

typedef struct mcbspConfigStruct
{
    int port;
    /*rx mode config*/
    int fsrm;
    int clkrm;
    int fsrp;
    int clkrp;
    int rFullCycle;
    /*tx mode config*/
    int fsxm;
    int clkxm;
    int fsxp;
    int clkxp;
    int xFullCycle;
    int extClkGate;
    /*rx frame config*/
    int rfLen1;
    int rwdLen1;
    int rPhase;
    int rfLen2;
    int rwdLen2;
    int rReverse;
    int rDatDly;
    int rJust;
    /*tx frame config*/
    int xfLen1;
    int xwdLen1;
    int xPhase;
    int xfLen2;
    int xwdLen2;
    int xReverse;
    int xDatDly;
    /*SRG config*/  
    int fWid;
    int clkGdv;
    int gSync;
    int clkSp;
    int fPer;
    int sclkMe;
    int clkSm;
}MCBSP_INIT_CONFIG;


typedef struct mcbspConfigStructFull
{
    int port;
    /*rx mode config*/
    int fsrm;
    int clkrm;
    int fsrp;
    int clkrp;
    int rFullCycle;
    /*tx mode config*/
    int fsxm;
    int clkxm;
    int fsxp;
    int clkxp;
    int xFullCycle;
    int extClkGate;
    /*rx frame config*/
    int rfLen1;
    int rwdLen1;
    int rPhase;
    int rfLen2;
    int rwdLen2;
    int rReverse;
    int rDatDly;
    int rJust;
    /*tx frame config*/
    int xfLen1;
    int xwdLen1;
    int xPhase;
    int xfLen2;
    int xwdLen2;
    int xReverse;
    int xDatDly;
    /*SRG config*/  
    int fWid;
    int clkGdv;
    int gSync;
    int clkSp;
    int fsgm;
    int fPer;
    int sclkMe;
    int clkSm;
    int rdmaen;
    int xdmaen;

}MCBSP_INIT_CONFIG_FULL;


typedef struct mcbspInterruptsEnableStruct
{
    int port;
    /*tx mode config*/
    int  x_empty_eof_en;  
    int  x_ovfl_en;
    int  x_undfl_en;
    int  x_rdy_en;
    int  x_eof_en;
    int  x_fsx_en;
    int  x_sync_err_en;

    /*rx mode config*/
    int  r_ovfl_en;
    int  r_undf_en;
    int  r_rdy_en;
    int  r_eof_en;
    int  r_fsr_en;
    int  r_sync_err_en;

}MCBSP_INTERRUPTS_ENABLE;

#define MCBSP_INTR_ON  0x1;	
#define MCBSP_INTR_OFF 0x0;	
#endif /* _MCBSP_SERIAL_H_ */
