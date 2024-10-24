/*
 * chai.h 
 * CAN Hardware Abstraction Interface 
 * Version 2.5.0
 *
 * Copyright (C) 1998-2011 Fedor Nedeoglo <fedor@marathon.ru>
 * Copyright (C) 2002-2011 Marathon Ltd. Moscow Russia <http://www.marathon.ru>
 *
 * Date: 12 Jul 2011
 *
 */

#ifndef __CHAI_H_
#define __CHAI_H_

#ifdef __cplusplus
extern "C" {
#endif

#define CHAI_STATUS
#define CHAI_EXTRA

#ifdef WIN32
#include <BaseTsd.h>

#ifndef _u8
#define _u8  UINT8
#endif

#ifndef _s8
#define _s8  INT8
#endif

#ifndef _u16
#define _u16 UINT16
#endif

#ifndef _s16
#define _s16 INT16
#endif

#ifndef _u32
#define _u32 ULONG32
#endif

#ifndef _s32
#define _s32 INT32
#endif

#else //#ifdef WIN32 (LINUX)

#ifndef _u8
#define _u8  unsigned char
#endif

#ifndef _s8
#define _s8  char
#endif

#ifndef _u16
#define _u16 unsigned short int
#endif

#ifndef _s16
#define _s16 short int
#endif

#ifndef _u32
#define _u32 unsigned long
#endif

#ifndef _s32
#define _s32 int
#endif

#endif //#ifdef WIN32

    typedef struct {
        _u32 id;
        _u8 data[8];
        _u8 len;
        _u16 flags;   /* bit 0 - RTR, 2 - EFF */
        _u32 ts;
    } canmsg_t;

    typedef struct {
        _u8 chan;
        _u8 wflags;
        _u8 rflags;
    } canwait_t;

    typedef struct {
        _u8 brdnum;
        _u32 hwver;
        _s16 chip[4];
        char name[64];
        char manufact[64];
    } canboard_t;

    typedef struct {
        _u16 ewl;
        _u16 boff;
        _u16 hwovr;
        _u16 swovr;
        _u16 wtout;
    } canerrs_t;

    typedef struct {
        _s16 type;
        _s16 brdnum;
        _s32 irq;
        _u32 baddr;
        _u8 state;
        _u32 hovr_cnt;
        _u32 sovr_cnt;
        char _pad[32];
    } chipstat_t;

    typedef struct {
        _s16 type;
        _s16 brdnum;
        _s32 irq;
        _u32 baddr;
        _u8 state;
        _u32 hovr_cnt;
        _u32 sovr_cnt;

        _u8 mode;
        _u8 stat;
        _u8 inten;
        _u8 clkdiv;
        _u8 ecc;
        _u8 ewl;
        _u8 rxec;
        _u8 txec;
        _u8 rxmc;
        _u32 acode;
        _u32 amask;
        _u8 btr0;
        _u8 btr1;
        _u8 outctl;
        char _pad[8];
    } sja1000stat_t;

    typedef struct {
        _s16 type;
        _s16 brdnum;
        _s32 irq;
        _u32 baddr;
        _u8 state;
        _u32 hovr_cnt;
        _u32 sovr_cnt;

        _u8 canctl0;
        _u8 canctl1;
        _u8 canbtr0;
        _u8 canbtr1;
        _u8 canrflg;
        _u8 canrier;
        _u8 cantflg;
        _u8 cantier;
        _u8 cantarq;
        _u8 cantaak;
        _u8 canidac;
        _u8 canrxerr;
        _u8 cantxerr;
        char _pad[19];
    } tqmscanstat_t;

#define CI_CHSTAT_MAXLEN 16
#define CI_CHSTAT_STRNUM 32

    typedef struct {
        char name[CI_CHSTAT_STRNUM][CI_CHSTAT_MAXLEN];
        char val[CI_CHSTAT_STRNUM][CI_CHSTAT_MAXLEN];
    } chstat_desc_t;

#define CI_BRD_NUMS      8
#define CI_CHAN_NUMS     8

#define CIQUE_RC         0
#define CIQUE_TR         1

#define CIQUE_DEFSIZE_RC 4096

/* default write timeout in milliseconds */
#define CI_WRITE_TIMEOUT_DEF 20
/* maximium write timeout in milliseconds */
#define CI_WRITE_TIMEOUT_MAX 500

/* states of CAN controller */
#define CAN_INIT      0
#define CAN_RUNNING   1

/*
 *  predefined baud rates (recommended by CiA)
 *  Phillips SJA1000 (16 MHz quartz)
 */

#define BCI_1M_bt0    0x00
#define BCI_1M_bt1    0x14

#define BCI_800K_bt0  0x00
#define BCI_800K_bt1  0x16

#define BCI_500K_bt0  0x00
#define BCI_500K_bt1  0x1c

#define BCI_250K_bt0  0x01
#define BCI_250K_bt1  0x1c

#define BCI_125K_bt0  0x03
#define BCI_125K_bt1  0x1c

#define BCI_100K_bt0  0x04
#define BCI_100K_bt1  0x1c

#define BCI_50K_bt0   0x09
#define BCI_50K_bt1   0x1c

#define BCI_20K_bt0   0x18
#define BCI_20K_bt1   0x1c

#define BCI_10K_bt0   0x31
#define BCI_10K_bt1   0x1c

#define BCI_1M    BCI_1M_bt0, BCI_1M_bt1
#define BCI_800K  BCI_800K_bt0, BCI_800K_bt1
#define BCI_500K  BCI_500K_bt0, BCI_500K_bt1
#define BCI_250K  BCI_250K_bt0, BCI_250K_bt1
#define BCI_125K  BCI_125K_bt0, BCI_125K_bt1
#define BCI_100K  BCI_100K_bt0, BCI_100K_bt1
#define BCI_50K   BCI_50K_bt0, BCI_50K_bt1
#define BCI_20K   BCI_20K_bt0, BCI_20K_bt1
#define BCI_10K   BCI_10K_bt0, BCI_10K_bt1

/*
 *  Error codes
 */
#define ECIOK      0            /* success */
#define ECIGEN     1            /* generic (not specified) error */
#define ECIBUSY    2            /* device or resourse busy */
#define ECIMFAULT  3            /* memory fault */
#define ECISTATE   4            /* function can't be called for chip in current state */
#define ECIINCALL  5            /* invalid call, function can't be called for this object */
#define ECIINVAL   6            /* invalid parameter */
#define ECIACCES   7            /* can not access resource */
#define ECINOSYS   8            /* function or feature not implemented */
#define ECIIO      9            /* input/output error */
#define ECINODEV   10           /* no such device or object */
#define ECIINTR    11           /* call was interrupted by event */
#define ECINORES   12           /* no resources */
#define ECITOUT    13           /* time out occured */
/*
 *  Possible events
 */
#define CIEV_RC      1
#define CIEV_TR      2
#define CIEV_CANERR  6

#define CIEV_EWL     3
#define CIEV_BOFF    4
#define CIEV_HOVR    5
#define CIEV_WTOUT   7
#define CIEV_SOVR    8

/* 
 *  Possible flags for CiOpen
 */
#define CIO_BLOCK   0x1         // ignored (block mode was removed in CHAI 2.x
#define CIO_CAN11   0x2
#define CIO_CAN29   0x4

/* 
 *  Possible flags for CiWaitEvent
 */
#define CI_WAIT_RC   0x1
#define CI_WAIT_TR   0x2
#define CI_WAIT_ER   0x4


/* 
 *  Possible commands for CiSetLom
 */
#define CI_LOM_OFF   0x0
#define CI_LOM_ON    0x1

/*
 * Possible bit numbers in canmsg_t.flags field
 */
#define MSG_RTR  0
#define MSG_FF   2              /* if set - extended frame format is used */

#define FRAME_RTR 0x1
#define FRAME_EFF 0x4

/*
 *  Symbolic names for CAN-controllers 
 */
#define CHIP_UNKNOWN      0
#define SJA1000           1
#define EMU               2
#define MSCAN             3

/*
 *  Symbolyc names for manufacturers 
 */
#define MANUF_UNKNOWN      0
#define MARATHON           1
#define SA                 2
#define FREESCALE          3

/*
 *  Symbolyc names for CAN adapter types 
 */
#define BRD_UNKNOWN       0
#define CAN_BUS_ISA       1
#define CAN_BUS_MICROPC   2
#define CAN_BUS_PCI       3
#define CAN_EMU           4
#define CAN2_PCI_M        5
#define MPC5200TQM        6
#define CAN_BUS_USB       7
#define CAN_BUS_PCI_E     8
#define CAN_BUS_USB_NP    9
#define CAN_BUS_USB_NPS   10

#define CHAI_VER(maj,min,sub) (maj<<16)|(min<<8)|(sub)
#define UNICAN_VER(maj,min,sub) (maj<<16)|(min<<8)|(sub)
#define VERSUB(ver) (_s32) ((ver) & 0xff)
#define VERMIN(ver) (_s32) (((ver)>>8) & 0xff)
#define VERMAJ(ver) (_s32) (((ver)>>16) & 0xff)

#if defined WIN32 && defined CHAI_EXPORTS
#define DECLHEAD __declspec( dllexport)
#else
#ifdef WIN32
#define DECLHEAD __declspec( dllimport)
#else
#define DECLHEAD extern
#endif
#endif

    DECLHEAD _s16 CiInit(void);
    DECLHEAD _s16 CiOpen(_u8 chan, _u8 flags);
    DECLHEAD _s16 CiClose(_u8 chan);
    DECLHEAD _s16 CiStart(_u8 chan);
    DECLHEAD _s16 CiStop(_u8 chan);
    DECLHEAD _s16 CiSetFilter(_u8 chan, _u32 acode, _u32 amask);
    DECLHEAD _s16 CiSetBaud(_u8 chan, _u8 bt0, _u8 bt1);
    DECLHEAD _s16 CiWrite(_u8 chan, canmsg_t * mbuf, _s16 cnt);
    DECLHEAD _s16 CiRead(_u8 chan, canmsg_t * mbuf, _s16 cnt);
    DECLHEAD _s16 CiSetCB(_u8 chan, _u8 ev, void (*ci_handler) (_s16));

#ifdef CHAI_STATUS
    DECLHEAD _u32 CiGetLibVer(void);
    DECLHEAD _u32 CiGetDrvVer(void);
    DECLHEAD _s16 CiChipStat(_u8 chan, chipstat_t * stat);
    DECLHEAD _s16 CiChipStatToStr(chipstat_t * status, chstat_desc_t * desc);
    DECLHEAD _s16 CiBoardInfo(canboard_t * binfo);
    DECLHEAD _s16 CiBoardGetSerial(_u8 brdnum, char *sbuf, _u16 bufsize);
#endif

#ifdef CHAI_EXTRA
    DECLHEAD _s16 CiSetCBex(_u8 chan, _u8 ev, void (*ci_cb_ex) (_u8, _s16, void *), void *udata);
	DECLHEAD _s16 CiCB_lock(void);
	DECLHEAD _s16 CiCB_unlock(void);
    DECLHEAD _s16 CiWaitEvent(canwait_t * cw, int cwcount, int tout);
    DECLHEAD _s16 CiErrsGetClear(_u8 chan, canerrs_t * errs);
    DECLHEAD _s16 CiQueResize(_u8 chan, _u16 size);
    DECLHEAD _s16 CiRcQueEmpty(_u8 chan);
    DECLHEAD _s32 CiRcGetCnt(_u8 chan);
    DECLHEAD _s16 CiHwReset(_u8 chan);
    DECLHEAD _s16 CiSetLom(_u8 chan, _u8 mode);
    DECLHEAD _s16 CiSetWriteTout(_u8 chan, _u16 msec);
    DECLHEAD _s16 CiGetWriteTout(_u8 chan, _u16 * msec);
    DECLHEAD _s16 CiHwRead(_u8 chan, _s16 offset, _u8 * val);
    DECLHEAD _s16 CiHwWrite(_u8 chan, _s16 offset, _u8 * val);
    DECLHEAD void CiStrError(_s16 cierrno, char *buf, _s16 n);
    DECLHEAD void CiPerror(_s16 cierrno, const char *s);
    // backward compatibility prototypes, in new code use CiSetLom
    DECLHEAD _s16 CiSJA1000SetLom(_u8 chan);
    DECLHEAD _s16 CiSJA1000ClearLom(_u8 chan);
#endif

/*================================================================
   Struct canmsg_t operations
  ================================================================*/
    DECLHEAD void msg_zero(canmsg_t * msg);
    DECLHEAD _s16 msg_isrtr(canmsg_t * msg);
    DECLHEAD void msg_setrtr(canmsg_t * msg);
    DECLHEAD _s16 msg_iseff(canmsg_t * msg);
    DECLHEAD void msg_seteff(canmsg_t * msg);

#ifdef __cplusplus
}
#endif
#endif                          /* __CHAI_H_ */
