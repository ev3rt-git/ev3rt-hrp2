/*
 *  EV3用ドライバ定義
 */

#ifndef TOPPERS_EV3_H
#define TOPPERS_EV3_H

#include <sil.h>
#include "target_serial.h"

/**
 * CPU frequency of EV3's processor (AM1808) in MHz
 */
#define CORE_CLK_MHZ (300)

/**
 * Frequency of oscillator input (OSCIN) in MHz
 * OSCIN_MHZ = CORE_CLK_MHZ * (PREDIV + 1)[if enabled]
 *     * (POSTDIV + 1)[if enabled] * (PLLDIV6 + 1)[if enabled]
 *     / (PLLM + 1)
 */
#define OSCIN_MHZ (24)

#define ARM_PAGE_TABLE_RATIO (20)

/**
 * Period in microseconds for the high resolution cyclic
 */
//#define PERIOD_UART1_SIO  (200U)
//#define PERIOD_UART_PORT1 (200U)
//#define PERIOD_UART_PORT2 (200U)
#define PERIOD_UART_SENSOR_CYC (10000U)

/**
 * Interrupt number
 */
#define INTNO_UART_PORT1 (UART1_INT)
#define INTNO_UART_PORT2 (UART0_INT)
#define INTNO_UART_PORT3 (SUART2_INT)
#define INTNO_UART_PORT4 (SUART1_INT)
#define INTNO_I2C_TIMER  (T64P1_TINT12)

/**
 * Interrupt priority
 */
//#define INTPRI_UART_SIO   (TMIN_INTPRI + 1)
#define INTPRI_I2C_TIMER   (TMIN_INTPRI)
#define INTPRI_UART_PORT1 (TMIN_INTPRI)
#define INTPRI_UART_PORT2 (TMIN_INTPRI)
#define INTPRI_UART_PORT3 (TMIN_INTPRI)
#define INTPRI_UART_PORT4 (TMIN_INTPRI)
#define INTPRI_USBMSC     (TMIN_INTPRI + 1)
#define INTPRI_BLUETOOTH  (TMIN_INTPRI + 1)
#define INTPRI_LCD_SPI    (TMIN_INTPRI + 1)

/**
 * Task priority
 */
#define TPRI_INIT_TASK       (TMIN_TPRI)
#define TPRI_USBMSC          (TMIN_TPRI + 1)
#define TPRI_BLUETOOTH_QOS   (TMIN_TPRI + 1)
#define TPRI_BLUETOOTH_HIGH  (TMIN_TPRI + 2)
#define TPRI_APP_TERM_TASK   (TMIN_TPRI + 3)
#define TPRI_EV3_LCD_TASK    (TMIN_TPRI + 3)
#define TPRI_EV3_MONITOR     (TMIN_TPRI + 4)
#define TPRI_PLATFORM_BUSY   (TMIN_TPRI + 5)
#define TPRI_APP_INIT_TASK   (TMIN_TPRI + 6)
#define TPRI_EV3_CYC         (TMIN_TPRI + 7)
#define TMIN_APP_TPRI        (TMIN_TPRI + 8)
#define TPRI_BLUETOOTH_LOW   (TMAX_TPRI)/*(TMIN_TPRI + 1)*/

/*
 *  タスクのスタックサイズ
 */
#ifndef STACK_SIZE
#define STACK_SIZE  4096
#endif

/**
 * Memory
 */
#define KERNEL_HEAP_SIZE (1024 * 1024) //!< Heap size for dynamic memory allocation in TDOM_KERNEL
#define APP_HEAP_SIZE    (1024 * 1024) //!< Heap size for dynamic memory allocation in TDOM_APP

/**
 * Default SIO Port for syslog etc.
 */
#define SIO_PORT_DEFAULT SIO_PORT_LCD

/**
 * Bluetooth configuration
 */
//#define BLUETOOTH_LOCAL_NAME   ("Mindstorms EV3") //!< Name for service discovery
//#define BLUETOOTH_PIN_CODE     ("0000")           //!< Pin code for authentication, NULL to use secure simple pairing
#define BT_SND_BUF_SIZE        (2048)             //!< Size of send buffer
#define BT_HIGH_PRI_TIME_SLICE (1)                //!< Time slice for BT_TSK in high priority mode (mS)
#define BT_LOW_PRI_TIME_SLICE  (19)               //!< Time slice for BT_TSK in low priority mode (mS)
#define BT_USE_EDMA_MODE       (false)            //!< true: EDMA mode, false: interrupt mode

/**
 * Loadable application module configuration (Dynamic loading)
 */
#define TMAX_APP_TSK_NUM     (32)          //!< Maximum number of tasks in a loadable application module
#define TMAX_APP_SEM_NUM     (16)          //!< Maximum number of semaphores in a loadable application module
#define TMAX_APP_FLG_NUM     (16)          //!< Maximum number of event flags in a loadable application module
#define TMAX_APP_DTQ_NUM     (16)          //!< Maximum number of data queues in a loadable application module
#define TMAX_APP_PDQ_NUM     (16)          //!< Maximum number of priority data queues in a loadable application module
#define TMAX_APP_MTX_NUM     (16)          //!< Maximum number of mutexes in a loadable application module
#define TMAX_APP_TEXT_SIZE   (1024 * 1024) //!< Maximum size of the text section in a loadable application module
#define TMAX_APP_DATA_SIZE   (1024 * 1024) //!< Maximum size of the data section in a loadable application module
#define TMAX_APP_BINARY_SIZE (1024 * 1024) //!< Maximum size of a loadable application module's binary file

/**
 * LCD configuration
 */
#define LCD_FRAME_RATE (25)

/**
 * Miscellaneous configuration
 */
#define FORCE_SHUTDOWN_TIMEOUT (500)  //!< Timeout in milliseconds of force shutdown feature by pressing BACK+LEFT+RIGHT buttons
#define TMAX_EV3_CYC_NUM       (16)   //!< Maximum number of EV3_CRE_CYC in a user application

/**
 * UART sensor driver
 */
#ifndef TOPPERS_MACRO_ONLY
extern void uart_sensor_isr(intptr_t irq);
#endif

/**
 * GPIO driver
 */
#ifndef TOPPERS_MACRO_ONLY
extern void gpio_initialize(intptr_t unused);
#endif

/**
 * Utility function for outputting SVC error
 */
#ifndef TOPPERS_MACRO_ONLY
extern void svc_perror(const char *file, int_t line, const char *expr, ER ercd);
#define SVC_PERROR(expr) svc_perror(__FILE__, __LINE__, #expr, (expr))
# if 0
#define SVC_PERROR(expr) do { \
	ER ercd = (expr); \
	if (ercd < 0) { \
		t_perror(LOG_ERROR, __FILE__, __LINE__, #expr, ercd); \
	} \
}while(0)
#endif
#endif

/**
 * PRU Soft UART Driver
 */
#define SUART1_INT 3
#define SUART2_INT 4
#ifndef TOPPERS_MACRO_ONLY
extern void pru_suart_isr(intptr_t portline);
#endif

#define TCNT_SYSLOG_BUFFER (1024)

#endif /* TOPPERS_EV3_H */
