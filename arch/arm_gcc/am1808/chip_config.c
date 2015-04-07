/*
 *  チップ依存モジュール（AM1808用）
 */

#include "kernel_impl.h"
#include <sil.h>
#include "am1808.h"
#include "target_serial.h"

extern void* vector_table();

#if !defined(TOPPERS_SUPPORT_PROTECT)
static uint32_t section_table[4096] __attribute__((aligned(0x4000/*ARM_SECTION_TABLE_ALIGN*/),nocommon));

static void section_table_init() {
    for(uint32_t i = 0; i < 4096; ++i)    /* I/O Devices (Cache off, buffer off) */
        section_table[i] = (i << 20) | 0x12;
    for(uint32_t i = 3072; i < (3072+128); ++i) /* RAM (Cache on, buffer on) */
        section_table[i] = (i << 20) | 0x1E;
    for(uint32_t i = 1536; i < (1536+16); ++i) /* ROM (Cache on, buffer on) */
        section_table[i] = (i << 20) | 0x1E;
//    for(uint32_t i = 2048; i < (2048+1); ++i) /* localRAM (Cache on, buffer on) */ // Must set off for PRU
//        section_table[i] = (i << 20) | 0x1E;
}
#endif

/*
 *  チップ依存の初期化
 */
void
chip_initialize(void)
{
	/*
	 *  ARM依存の初期化
	 */
	core_initialize();

    /*
     *  ベクタテーブルをHigh exception vectors(0xFFFF0000)にコピー
     */
    char *vt  = (char*)0xFFFF0000;
    char *vts = (char*)vector_table;
    for(int i = 0; i < 512; ++i)
        vt[i] = vts[i];

	/*
     *  Select high exception vectors (0xFFFF0000)
	 *  by setting V bit in CP15 c1 (control register).
	 */
	asm("mrc    p15, 0, r0, c1, c0, 0");
	asm("orr    r0, r0, #0x00002000");
	asm("mcr    p15, 0, r0, c1, c0, 0");

    /*
     *  AINTCの初期化
     */
    AINTC.GER  = 0x1;
    AINTC.HIER = 0x2;
    AINTC.EISR = 21;
    AINTC.CR   = 0x14;
    AINTC.VBR  = (uint32_t)ISR_VECTORS;
    AINTC.VSR  = 0;
    x_set_ipm(0); /* Set IPM to 0 */

    /**
     * 低レベル出力シリアルI/Oの初期化
     */
    sio_initialize_low();

	/*
	 * キャッシュを無効に
	 */
	uint32_t bits = 0;
	CP15_CONTROL_READ(bits);
	bits &= ~(CP15_CONTROL_I_BIT | CP15_CONTROL_C_BIT);
	CP15_CONTROL_WRITE(bits);
	//cache_disable();

#if !defined(TOPPERS_SUPPORT_PROTECT)
    section_table_init();
#endif

    /**
     * MMUを有効に
     */
	CP15_TTB0_WRITE((uintptr_t)section_table);
#if defined(TOPPERS_SUPPORT_PROTECT)
	CP15_DOMAINS_WRITE(0x01U/*D0_CLIENT*/);
#else
	CP15_DOMAINS_WRITE(0x03U/*D0_MANAGER*/);
#endif
	//CP15_ASID_SET(1);
	CP15_CONTROL_READ(bits);
	bits |= CP15_CONTROL_M_BIT;
	CP15_CONTROL_WRITE(bits);
    //*(uint32_t*)0x1E2608C = 0x0;
    //mmu_init();
    //MCR p15, 0, <Rd>, c2, c0, 0; write TTBR

	/*
	 * キャッシュを有効に
	 */
	CP15_CONTROL_READ(bits);
	bits |= (CP15_CONTROL_I_BIT | CP15_CONTROL_C_BIT);
	CP15_CONTROL_WRITE(bits);
}

/*
 *  割込み要求ラインの属性の設定
 *
 *  ASPカーネルでの利用を想定して，パラメータエラーはアサーションでチェッ
 *  クしている．FI4カーネルに利用する場合には，エラーを返すようにすべき
 *  であろう．
 */
void
x_config_int(INTNO intno, ATR intatr, PRI intpri)
{
	assert(VALID_INTNO(intno));
	assert(TMIN_INTPRI <= intpri && intpri <= TMAX_INTPRI);

	/*
	 *  割込み要求のマスク
	 *
	 *  割込みを受け付けたまま，レベルトリガ／エッジトリガの設定や，割
	 *  込み優先度の設定を行うのは危険なため，割込み属性にかかわらず，
	 *  一旦マスクする．
	 */    
	x_disable_int(intno);

	/*
	 *  割込み優先度をセット
	 */
    AINTC.CMR[intno] = PRI_TO_CHN(intpri);
    
	/*
	 *  割込みを許可
	 */
	if ((intatr & TA_ENAINT) != 0U){
		(void)x_enable_int(intno);
	} 
}
