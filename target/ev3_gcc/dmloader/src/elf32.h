/*
 * elf32.h
 *
 *  Created on: Nov 8, 2013
 *      Author: liyixiao
 */


#pragma once

/**
 * \brief The context used to load an ELF32 file.
 */
typedef struct elf32_load_context {
	void    *text_buf;                  //!< [IN]  Pointer of text buffer to load text segment
	uint32_t text_bufsz;                //!< [IN]  Size of text buffer
	void    *data_buf;                  //!< [IN]  Pointer of data buffer to load data segment
	uint32_t data_bufsz;                //!< [IN]  Size of data buffer
	uint32_t text_seg_vaddr;            //!< [OUT] Virtual address of text segment
	uint32_t text_seg_sz;               //!< [OUT] Size of text segment
	uint32_t data_seg_vaddr;            //!< [OUT] Virtual address of data segment
	uint32_t data_seg_sz;               //!< [OUT] Size of data segment
	void    *sym__module_cfg_tab;       //!< [OUT] Pointer of the symbol '_module_cfg_tab'
	void    *sym__module_cfg_entry_num; //!< [OUT] Pointer of the symbol '_module_cfg_entry_num'
	void    *sym__module_pil_version;   //!< [OUT] Pointer of the symbol '_module_pil_version'
} elf32_ldctx_t;

/**
 * \brief               Load an ELF32 file and update the context.
 * \param elf32_data    Pointer of the buffer holding the ELF32 file to be loaded
 * \param elf32_data_sz Size of the ELF32 data buffer
 * \param ctx           Context for loading. All inputs ([IN]) must be initialized before calling this function.
 * \param E_OK          Success. All outputs ([OUT]) in the context have been updated.
 * \param E_NOMEM       The text buffer or data buffer is too small.
 * \param E_PAR         The ELF32 file is corrupted.
 */
ER elf32_load(const void *elf32_data, uint32_t elf32_data_sz, elf32_ldctx_t *ctx);

