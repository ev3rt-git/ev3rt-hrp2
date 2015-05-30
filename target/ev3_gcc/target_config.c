#include "kernel_impl.h"
#include <sil.h>
#include "target_syssvc.h"
#include "am1808.h"
#include "tlsf.h"

#if defined(BUILD_EV3_PLATFORM)
static bool_t tlsf_initialized = false;

void
initialize_kmm(void)
{
	if (init_memory_pool(kmmsz, kmm) >= 0) {
		tlsf_initialized = true;
	}
}

void *
kernel_malloc(SIZE size)
{
	if (tlsf_initialized) {
		return(malloc_ex(size, kmm));
	}
	else {
		return(NULL);
	}
}

void
kernel_free(void *ptr)
{
	if (tlsf_initialized) {
		free_ex(ptr, kmm);
	}
}
#endif

bool_t VALID_INTNO_CREISR(INTNO intno) {
    if (intno == INTNO_UART_PORT1)
        return true;
    else
	    return false;
}

/**
 * Initialize the target.
 */
void
target_initialize(void)
{
	/*
	 * チップ依存の初期化
	 */
	chip_initialize();
}

/*
 *  ターゲット依存の終了処理
 */
void
target_exit(void)
{
	// TODO: check if this is necessary
    // Flush low level output by write 2K bytes.
    for(int i = 0; i < 2048; ++i)
        target_fput_log('\0');

    // Power off
    GPIO67.OUT_DATA &= ~GPIO_ED_PIN11;
	
    while(1);
}
