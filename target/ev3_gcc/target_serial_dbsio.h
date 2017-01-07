/**
 * Double buffered serial I/O
 * Mainly used for Bluetooth SPP
 */

#pragma once

#ifndef TOPPERS_MACRO_ONLY

struct DBSIOCB; // Double buffered serial I/O control block

#define DBSIO_BUFFER_SIZE BT_SND_BUF_SIZE
typedef struct dbsio_buffer {
    uint8_t  buffer[DBSIO_BUFFER_SIZE];
    uint32_t bytes;
} DBSIOBF;

//bool_t   dbsio_send_char(struct DBSIOCB *dbsio);            // Put a character into send buffer. (NOTE: MUST be called when CPU is locked!)
void     dbsio_recv_fill(struct DBSIOCB *dbsio, uint8_t *buf, uint32_t bytes); // Fill receive buffer with bytes.
DBSIOBF* dbsio_next_send_buffer(struct DBSIOCB *dbsio);     // Acquire next send buffer. Last buffer acquired will be released.

/**
 * Interface for SIO driver
 */
intptr_t dbsio_opn_por(struct DBSIOCB* dbsio, intptr_t exinf);
intptr_t dbsio_cls_por(struct DBSIOCB* dbsio);
intptr_t dbsio_snd_chr(struct DBSIOCB* dbsio, char c);
intptr_t dbsio_rcv_chr(struct DBSIOCB* dbsio);
intptr_t dbsio_ena_cbr(struct DBSIOCB* dbsio, intptr_t cbrtn);
intptr_t dbsio_dis_cbr(struct DBSIOCB* dbsio, intptr_t cbrtn);
extern void dbsio_cyc(intptr_t exinf); // Cyclic handler to fill send buffer

/**
 * DBSIO control blocks for EV3RT
 */
extern struct DBSIOCB dbsio_spp_master_test;

#endif
