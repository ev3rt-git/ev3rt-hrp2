/* This file is generated from core_rename.def by genrename. */

/* This file is included only when core_rename.h has been included. */
#ifdef TOPPERS_CORE_RENAME_H
#undef TOPPERS_CORE_RENAME_H

/*
 *  kernel_mem.c
 */
#undef section_table
#undef page_table

/*
 *  arm.c
 */
#undef dcache_enable
#undef dcache_disable
#undef icache_enable
#undef icache_disable
#undef cache_enable
#undef cache_disable
#undef mmu_init

/*
 *  core_support.S
 */
#undef vector_table
#undef vector_ref_tbl
#undef dispatch
#undef start_dispatch
#undef exit_and_dispatch
#undef call_exit_kernel
#undef start_stask_r
#undef start_utask_r
#undef ret_int
#undef ret_exc
#undef ret_int_1
#undef undef_handler
#undef svc_handler
#undef prefetch_handler
#undef data_abort_handler
#undef fiq_handler
#undef current_sr
#undef set_sr
#undef call_ret_tex
#undef prepare_texrtn_utask
#undef ret_tex

/*
 *  core_config.c
 */
#undef excpt_nest_count
#undef core_initialize
#undef core_terminate
#undef x_install_exc
#undef default_exc_handler
#undef check_stack

/*
 *  gic.c
 */
#undef gic_cpuif_init
#undef gic_cpuif_stop
#undef gic_dist_disable_int
#undef gic_dist_enable_int
#undef gic_dist_clear_pending
#undef gic_dist_set_pending
#undef gic_dist_probe_int
#undef gic_dist_config
#undef gic_dist_set_priority
#undef gic_dist_set_target
#undef gic_dist_init
#undef gic_dist_stop

/*
 *  gic_support.S
 */
#undef irq_handler
#undef target_exc_handler
#undef exch_tbl
#undef enable_all_ipm

#ifdef TOPPERS_LABEL_ASM

/*
 *  kernel_mem.c
 */
#undef _section_table
#undef _page_table

/*
 *  arm.c
 */
#undef _dcache_enable
#undef _dcache_disable
#undef _icache_enable
#undef _icache_disable
#undef _cache_enable
#undef _cache_disable
#undef _mmu_init

/*
 *  core_support.S
 */
#undef _vector_table
#undef _vector_ref_tbl
#undef _dispatch
#undef _start_dispatch
#undef _exit_and_dispatch
#undef _call_exit_kernel
#undef _start_stask_r
#undef _start_utask_r
#undef _ret_int
#undef _ret_exc
#undef _ret_int_1
#undef _undef_handler
#undef _svc_handler
#undef _prefetch_handler
#undef _data_abort_handler
#undef _fiq_handler
#undef _current_sr
#undef _set_sr
#undef _call_ret_tex
#undef _prepare_texrtn_utask
#undef _ret_tex

/*
 *  core_config.c
 */
#undef _excpt_nest_count
#undef _core_initialize
#undef _core_terminate
#undef _x_install_exc
#undef _default_exc_handler
#undef _check_stack

/*
 *  gic.c
 */
#undef _gic_cpuif_init
#undef _gic_cpuif_stop
#undef _gic_dist_disable_int
#undef _gic_dist_enable_int
#undef _gic_dist_clear_pending
#undef _gic_dist_set_pending
#undef _gic_dist_probe_int
#undef _gic_dist_config
#undef _gic_dist_set_priority
#undef _gic_dist_set_target
#undef _gic_dist_init
#undef _gic_dist_stop

/*
 *  gic_support.S
 */
#undef _irq_handler
#undef _target_exc_handler
#undef _exch_tbl
#undef _enable_all_ipm

#endif /* TOPPERS_LABEL_ASM */


#endif /* TOPPERS_CORE_RENAME_H */
