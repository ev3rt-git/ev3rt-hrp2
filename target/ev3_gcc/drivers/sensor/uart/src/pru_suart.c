/*
 * pru_suart.c
 *
 *  Created on: Oct 17, 2013
 *      Author: liyixiao
 */

#include "ev3.h"
#include "driver_common.h"
#include "linux/circ_buf.h"
#include "asm/termbits.h"
#include "pru_suart_fw.h"
#include "kernel/kernel_impl.h"
#include "kernel_cfg.h"
#include "sil.h"
#include "kernel/semaphore.h"
#include <errno.h>
#include <string.h>

//#define __SUART_DEBUG 1
//#define __SSC_DEBUG 1

typedef uint8_t u8;
typedef uint32_t u32;
typedef u32 __u32;
typedef u32 dma_addr_t;
typedef unsigned int upf_t;

#define __devexit
#define ENOTSUPP    524 /* Operation is not supported */
#define TIOCSER_TEMT    0x01    /* Transmitter physically empty */
#define UART_CONFIG_TYPE    (1 << 0)
#define OMAPL_PRU_SUART 91 /* omapl pru uart emulation */
#define uart_circ_empty(circ)       ((circ)->head == (circ)->tail)
#define uart_circ_clear(circ)       ((circ)->head = (circ)->tail = 0)

struct uart_icount {
    __u32   cts;
    __u32   dsr;
    __u32   rng;
    __u32   dcd;
    __u32   rx;
    __u32   tx;
    __u32   frame;
    __u32   overrun;
    __u32   parity;
    __u32   brk;
    __u32   buf_overrun;
};

struct uart_port {
//    spinlock_t      lock;           /* port lock */
    unsigned long       iobase;         /* in/out[bwl] */
    unsigned char    *membase;       /* read/write[bwl] */
    unsigned int        (*serial_in)(struct uart_port *, int);
    void            (*serial_out)(struct uart_port *, int, int);
    unsigned int        irq;            /* irq number */
    unsigned long       irqflags;       /* irq flags  */
    unsigned int        uartclk;        /* base uart clock */
    unsigned int        fifosize;       /* tx fifo size */
    unsigned char       x_char;         /* xon/xoff char */
    unsigned char       regshift;       /* reg offset shift */
    unsigned char       iotype;         /* io access style */
    unsigned char       unused1;

#define UPIO_PORT       (0)
#define UPIO_HUB6       (1)
#define UPIO_MEM        (2)
#define UPIO_MEM32      (3)
#define UPIO_AU         (4)         /* Au1x00 type IO */
#define UPIO_TSI        (5)         /* Tsi108/109 type IO */
#define UPIO_DWAPB      (6)         /* DesignWare APB UART */
#define UPIO_RM9000     (7)         /* RM9000 type IO */

    unsigned int        read_status_mask;   /* driver specific */
    unsigned int        ignore_status_mask; /* driver specific */
    struct uart_state   *state;         /* pointer to parent state */
    struct uart_icount  icount;         /* statistics */

    struct console      *cons;          /* struct console, if any */
#if defined(CONFIG_SERIAL_CORE_CONSOLE) || defined(SUPPORT_SYSRQ)
    unsigned long       sysrq;          /* sysrq timeout */
#endif

    upf_t           flags;

#define UPF_FOURPORT        ((__force upf_t) (1 << 1))
#define UPF_SAK         ((__force upf_t) (1 << 2))
#define UPF_SPD_MASK        ((__force upf_t) (0x1030))
#define UPF_SPD_HI      ((__force upf_t) (0x0010))
#define UPF_SPD_VHI     ((__force upf_t) (0x0020))
#define UPF_SPD_CUST        ((__force upf_t) (0x0030))
#define UPF_SPD_SHI     ((__force upf_t) (0x1000))
#define UPF_SPD_WARP        ((__force upf_t) (0x1010))
#define UPF_SKIP_TEST       ((__force upf_t) (1 << 6))
#define UPF_AUTO_IRQ        ((__force upf_t) (1 << 7))
#define UPF_HARDPPS_CD      ((__force upf_t) (1 << 11))
#define UPF_LOW_LATENCY     ((__force upf_t) (1 << 13))
#define UPF_BUGGY_UART      ((__force upf_t) (1 << 14))
#define UPF_NO_TXEN_TEST    ((__force upf_t) (1 << 15))
#define UPF_MAGIC_MULTIPLIER    ((__force upf_t) (1 << 16))
#define UPF_CONS_FLOW       ((__force upf_t) (1 << 23))
#define UPF_SHARE_IRQ       ((__force upf_t) (1 << 24))
/* The exact UART type is known and should not be probed.  */
#define UPF_FIXED_TYPE      ((__force upf_t) (1 << 27))
#define UPF_BOOT_AUTOCONF   ((__force upf_t) (1 << 28))
#define UPF_FIXED_PORT      ((__force upf_t) (1 << 29))
#define UPF_DEAD        ((__force upf_t) (1 << 30))
#define UPF_IOREMAP     ((__force upf_t) (1 << 31))

#define UPF_CHANGE_MASK     ((__force upf_t) (0x17fff))
#define UPF_USR_MASK        ((__force upf_t) (UPF_SPD_MASK|UPF_LOW_LATENCY))

    unsigned int        mctrl;          /* current modem ctrl settings */
//    unsigned int        timeout;        /* character-based timeout */
    unsigned int        type;           /* port type */
    const struct uart_ops   *ops;
    unsigned int        custom_divisor;
    unsigned int        line;           /* port index */
//    resource_size_t     mapbase;        /* for ioremap */
//    struct device       *dev;           /* parent device */
    unsigned char       hub6;           /* this should be in the 8250 driver */
    unsigned char       suspended;
    unsigned char       unused[2];
    void            *private_data;      /* generic platform data pointer */
};

#define INDEX_SEM(semid)	((uint_t)((semid) - TMIN_SEMID))
#define get_semcb(semid)	(&(semcb_table[INDEX_SEM(semid)]))

static ER
ipol_sem(ID semid)
{
    SEMCB   *p_semcb;
    ER      ercd;

//    LOG_POL_SEM_ENTER(semid);
//    CHECK_TSKCTX_UNL();
//    CHECK_SEMID(semid);
    p_semcb = get_semcb(semid);

    t_lock_cpu();
/*    if (p_semcb->p_seminib->sematr == TA_NOEXS) {
        ercd = E_NOEXS;
    }
    else */if (p_semcb->semcnt >= 1) {
        p_semcb->semcnt -= 1;
        ercd = E_OK;
    }
    else {
        ercd = E_TMOUT;
    }
    t_unlock_cpu();

//  error_exit:
//    LOG_POL_SEM_LEAVE(ercd);
    return(ercd);
}

static ER
lsig_sem(ID semid)
{
    SEMCB   *p_semcb;
    TCB     *p_tcb;
    ER      ercd;

//    LOG_ISIG_SEM_ENTER(semid);
//    CHECK_INTCTX_UNL();
//    CHECK_SEMID(semid);
    p_semcb = get_semcb(semid);

    i_lock_cpu();
/*    if (p_semcb->p_seminib->sematr == TA_NOEXS) {
        ercd = E_NOEXS;
    }
    else*/ if (!queue_empty(&(p_semcb->wait_queue))) {
        p_tcb = (TCB *) queue_delete_next(&(p_semcb->wait_queue));
        if (wait_complete(p_tcb)) {
            reqflg = true;
        }
        ercd = E_OK;
    }
    else if (p_semcb->semcnt < p_semcb->p_seminib->maxsem) {
        p_semcb->semcnt += 1;
        ercd = E_OK;
    }
    else {
        ercd = E_QOVR;
    }
    i_unlock_cpu();

//  error_exit:
//    LOG_ISIG_SEM_LEAVE(ercd);
    return(ercd);
}

struct semaphore {
    int id;
};

static int down_trylock(struct semaphore *sem) {
    ER ercd = ipol_sem(sem->id);

    if(ercd == E_OK)
        return 0;

    assert(ercd == E_TMOUT);
    return 1;
}

static void up(struct semaphore *sem) {
    ER ercd = (sns_loc() || sns_ctx()) ? lsig_sem(sem->id) : sig_sem(sem->id);
    assert(ercd == E_OK);
}

#define spin_lock_irqsave(...) SIL_PRE_LOC;SIL_LOC_INT()
#define spin_unlock_irqrestore(...) SIL_UNL_INT()
#define request_irq(irq, ...) (ena_int(irq) == E_OK ? 0 : 1)
#define free_irq(irq, ...) dis_int(irq)

#define dev_err(dev, fmt, ...) syslog(LOG_INFO, fmt, ##__VA_ARGS__)

/*
 * Reuse of 'lego_ti_omapl_pru_suart.c' from LEGO MINDSTORMS EV3 source code
 */
#include "../d_uart/Linuxmod_AM1808/lego_ti_omapl_pru_suart.c"

/*
 * res_mem[0]->start: OMAPL138_PRU_MEM_BASE
 * res_mem[1]->start: DAVINCI_DA8XX_MCASP0_REG_BASE
 * res_mem[2]->start: DA8XX_PSC0_BASE
 * res_mem[3]->start: DA8XX_PSC1_BASE
 * res_mem[4]->start: DA8XX_SHARED_RAM_BASE
 * IRQ[0]: OMAPL138_INT_PRU_SUART_1 IRQ_DA8XX_EVTOUT0
 * IRQ[1]: OMAPL138_INT_PRU_SUART_2 IRQ_DA8XX_EVTOUT1
 */
#define OMAPL138_PRU_MEM_BASE         ((void*)0x01C30000)
#define DAVINCI_DA8XX_MCASP0_REG_BASE ((void*)0x01D00000)
//#define DA8XX_PSC0_BASE               ((void*)0x01c10000)
//#define DA8XX_PSC1_BASE               ((void*)0x01e27000)
//#define IO_PHYS                       ((void*)0x01c00000)
#define DA8XX_SYSCFG0_BASE            (IO_PHYS + 0x14000)
#define DA8XX_SHARED_RAM_BASE         ((void*)0x80000000)

static const int suart_irq[NR_SUART] = {SUART1_INT, SUART2_INT};

static const int suart_sem[NR_SUART] = {SUART1_SEM, SUART2_SEM};

//TODO: not checked yet
#define CLK_FREQ_PRU (CORE_CLK_MHZ * 1000000 / 2)
#define CLK_FREQ_MCASP (OSCIN_MHZ * 1000000)

void pru_suart_isr(intptr_t portline) {
    struct uart_port *port = &soft_uart->port[portline];
    irqreturn_t ret = omapl_pru_suart_interrupt(port->irq, port);
    assert(ret == IRQ_HANDLED);
}

static struct omapl_pru_suart suart;

/**
 *
 * Prototype: omapl_pru_suart_probe(), lego_pru_suart_init()
 */
int lego_pru_suart_init()
{
  //struct omapl_pru_suart *soft_uart;
  int err, i;
  unsigned char *fw_data = PRU_SUART_Emulation;

  __suart_debug("Enter omapl_pru_suart_probe\n");

  soft_uart = &suart;
  memset(soft_uart, 0, sizeof(struct omapl_pru_suart));
  assert(soft_uart != NULL);

  soft_uart->pru_arm_iomap.pru_io_addr    = OMAPL138_PRU_MEM_BASE;
  soft_uart->pru_arm_iomap.mcasp_io_addr  = DAVINCI_DA8XX_MCASP0_REG_BASE;
  soft_uart->pru_arm_iomap.psc0_io_addr   = (void*)DA8XX_PSC0_BASE;
  soft_uart->pru_arm_iomap.psc1_io_addr   = (void*)DA8XX_PSC1_BASE;
  soft_uart->pru_arm_iomap.syscfg_io_addr = (void*)DA8XX_SYSCFG0_BASE;

  soft_uart->clk_freq_pru = CLK_FREQ_PRU;

  soft_uart->clk_freq_mcasp = CLK_FREQ_MCASP;
//  clk_enable(soft_uart->clk_mcasp);
//  clk_enable(soft_uart->clk_pru);

  dma_phys_addr = (uint32_t)DA8XX_SHARED_RAM_BASE;
  dma_vaddr_buff = DA8XX_SHARED_RAM_BASE;

  soft_uart->pru_arm_iomap.pFifoBufferPhysBase = (void *)dma_phys_addr;
  soft_uart->pru_arm_iomap.pFifoBufferVirtBase = (void *)dma_vaddr_buff;
  soft_uart->pru_arm_iomap.pru_clk_freq = (soft_uart->clk_freq_pru / 1000000);

  err = pru_softuart_init(SUART_DEFAULT_BAUD, SUART_DEFAULT_BAUD,
                SUART_DEFAULT_OVRSMPL, fw_data,
                sizeof(PRU_SUART_Emulation), &soft_uart->pru_arm_iomap);

  assert(err == 0);

  for (i = 0; i < NR_SUART; i++) {
              /* SSC soft_uart->port[i].ops = &pru_suart_ops; */
      soft_uart->port[i].iotype = UPIO_MEM;   /* user conf parallel io */
//      soft_uart->port[i].flags = UPF_BOOT_AUTOCONF | UPF_IOREMAP;
//      soft_uart->port[i].mapbase = DAVINCI_DA8XX_MCASP0_REG_BASE;
      soft_uart->port[i].membase =
          (unsigned char *)&soft_uart->pru_arm_iomap;
//      soft_uart->port[i].type = OMAPL_PRU_SUART;
      soft_uart->port[i].irq = suart_irq[i];
//      soft_uart->port[i].dev = &pdev->dev;
      soft_uart->port[i].irqflags = IRQF_SHARED;
      soft_uart->port[i].uartclk = soft_uart->clk_freq_mcasp; /* 24MHz */
      soft_uart->port[i].fifosize = SUART_FIFO_LEN;
      soft_uart->tx_loadsz = SUART_FIFO_LEN;
      soft_uart->port[i].custom_divisor = 1;
      soft_uart->port[i].line = i;    /* need the id/line from pdev */
      soft_uart->suart_hdl[i].uartNum = i + 1;
//      spin_lock_init(&soft_uart->port[i].lock);
      soft_uart->port[i].serial_in = NULL;
    soft_uart->break_rcvt[i] = 0;
    soft_uart->baud[i] = 0;
    soft_uart->read_buf[i].buf = (char*)&soft_uart->read_data[i][0];
    soft_uart->write_buf[i].buf = (char*)&soft_uart->write_data[i][0];

      soft_uart->suart_dma_addr[i].dma_vaddr_buff_tx =
                      dma_vaddr_buff + (2 * SUART_CNTX_SZ * i);

      soft_uart->suart_dma_addr[i].dma_vaddr_buff_rx =
                      dma_vaddr_buff + ((2 * SUART_CNTX_SZ * i) + SUART_CNTX_SZ);

      soft_uart->suart_dma_addr[i].dma_phys_addr_tx =
                      dma_phys_addr + (2 * SUART_CNTX_SZ * i);

      soft_uart->suart_dma_addr[i].dma_phys_addr_rx =
                      dma_phys_addr + ((2 * SUART_CNTX_SZ * i) + SUART_CNTX_SZ);

      soft_uart->port[i].serial_out = NULL;
      /* SSC uart_add_one_port(&pru_suart_reg, &soft_uart->port[i]); */

      soft_uart->port_sem[i].id = suart_sem[i];
  }
//  platform_set_drvdata(pdev, &soft_uart->port[0]);

  return 0;
}
