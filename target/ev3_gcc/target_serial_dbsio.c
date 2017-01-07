#include "kernel/kernel_impl.h"
#include "target_serial_dbsio.h"
#include "serial_mod.h" // For sio_rdy_rcv()

//#define DBSIO_SEND_BUFFER_SIZE BT_SND_BUF_SIZE
//#define DBSIO_RECV_BUFFER_SIZE (2048)
struct DBSIOCB {
    intptr_t sio_recv_data;
    intptr_t sio_exinf;
    uint8_t  send_buffer_index;
    uint8_t  sio_snd_cbr_ena;
    uint8_t  sio_rcv_cbr_ena;
    DBSIOBF  send_buffer[2];
//    DBSIOBF  recv_buffer; // unused
};

struct DBSIOCB dbsio_spp_master_test;

static void dbsio_reset(struct DBSIOCB *dbsio) {
    dbsio->send_buffer_index    = 0;
    dbsio->sio_recv_data        = -1;
    dbsio->sio_snd_cbr_ena      = false;
    dbsio->sio_rcv_cbr_ena      = false;
    dbsio->send_buffer[0].bytes = 0;
    dbsio->send_buffer[1].bytes = 0;
//    dbsio->recv_buffer.bytes    = 0;
}

DBSIOBF* dbsio_next_send_buffer(struct DBSIOCB *dbsio) {
    ER ercd;

    // TODO: use mutex/lock instead of lock cpu?
    ercd = loc_cpu();
    assert(ercd == E_OK);

    // Switch send buffer
    uint8_t old_snd_buf_idx = dbsio->send_buffer_index;
    dbsio->send_buffer_index = 1 - dbsio->send_buffer_index;
    dbsio->send_buffer[dbsio->send_buffer_index].bytes = 0;

    ercd = unl_cpu();
    assert(ercd == E_OK);

    return &(dbsio->send_buffer[old_snd_buf_idx]);
}

void dbsio_recv_fill(struct DBSIOCB *dbsio, uint8_t *buf, uint32_t bytes) {
	// TODO: check dis_dsp & sig_sem
	SVC_CALL(dis_dsp)();
	for(uint32_t i = 0; i < bytes; ++i) {
		dbsio->sio_recv_data = *buf;
        buf++;
		sio_rdy_rcv(dbsio->sio_exinf);
	}
	SVC_CALL(ena_dsp)();
}

/**
 * Interface for SIOPCB
 */

intptr_t dbsio_opn_por(struct DBSIOCB* dbsio, intptr_t exinf) {
    dbsio->sio_exinf = exinf;
    dbsio_reset(dbsio); // TODO: might it be called twice?
    return true;
}

intptr_t dbsio_cls_por(struct DBSIOCB* dbsio) {
    dbsio_reset(dbsio); // TODO: should we call dbsio_init() HERE?
    return true;
}

intptr_t dbsio_snd_chr(struct DBSIOCB* dbsio, char c) {
    bool_t retval = false;
	SIL_PRE_LOC;
	SIL_LOC_INT();
    DBSIOBF *send_buffer = &(dbsio->send_buffer[dbsio->send_buffer_index]);
	if(send_buffer->bytes < DBSIO_BUFFER_SIZE) {
		send_buffer->buffer[send_buffer->bytes++] = c;
		retval = true;
	} else assert(false); // Buffer is full!
	SIL_UNL_INT();
	return retval;
}

intptr_t dbsio_rcv_chr(struct DBSIOCB* dbsio) {
    return dbsio->sio_recv_data;
}

intptr_t dbsio_ena_cbr(struct DBSIOCB* dbsio, intptr_t cbrtn) {
    switch (cbrtn) {
    case SIO_RDY_SND:
        dbsio->sio_snd_cbr_ena = true;
        break;
    case SIO_RDY_RCV:
        dbsio->sio_rcv_cbr_ena = true;
        break;
    }
    return true;
}

intptr_t dbsio_dis_cbr(struct DBSIOCB* dbsio, intptr_t cbrtn) {
    switch (cbrtn) {
	case SIO_RDY_SND:
		dbsio->sio_snd_cbr_ena = false;
		break;
	case SIO_RDY_RCV:
		dbsio->sio_rcv_cbr_ena = false;
		break;
	}
    return true;
}

void dbsio_cyc(intptr_t _dbsio) {
    struct DBSIOCB* dbsio = (struct DBSIOCB*)_dbsio;
    DBSIOBF *send_buffer  = &(dbsio->send_buffer[dbsio->send_buffer_index]);
	// Fill the send buffer
	while(dbsio->sio_snd_cbr_ena && send_buffer->bytes < DBSIO_BUFFER_SIZE)
		sio_irdy_snd(dbsio->sio_exinf);
}
