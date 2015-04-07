#pragma once

#define EMBEDDED

//#define HAVE_INIT_SCRIPT
//#define HAVE_BZERO
#define HAVE_TICK

//#define HAVE_EHCILL

//#define ENABLE_LOG_DEBUG
//#define ENABLE_LOG_INFO
#define ENABLE_LOG_ERROR

#define HCI_ACL_PAYLOAD_SIZE 1021

//#define DUMP

/**
 * Do not use malloc()
 */
#if 0
#define HAVE_MALLOC
#define malloc kernel_malloc
#define free kernel_free
#else
#define MAX_SPP_CONNECTIONS 1
#define MAX_NO_BNEP_SERVICES (0)
#define MAX_NO_BNEP_CHANNELS (0)
#define MAX_NO_HCI_CONNECTIONS MAX_SPP_CONNECTIONS
#define MAX_NO_L2CAP_SERVICES  2
#define MAX_NO_L2CAP_CHANNELS  (1+MAX_SPP_CONNECTIONS)
#define MAX_NO_RFCOMM_MULTIPLEXERS MAX_SPP_CONNECTIONS
#define MAX_NO_RFCOMM_SERVICES 1
#define MAX_NO_RFCOMM_CHANNELS MAX_SPP_CONNECTIONS
#define MAX_NO_DB_MEM_DEVICE_LINK_KEYS  2
#define MAX_NO_DB_MEM_DEVICE_NAMES 0
#define MAX_NO_DB_MEM_SERVICES 1
#endif

/**
 * Use syslog() instead of printf()
 */
#include <stdio.h>
#undef printf
#include "syssvc/syslog.h"
#define printf(fmt, ...) syslog(LOG_NOTICE, fmt, ##__VA_ARGS__)
