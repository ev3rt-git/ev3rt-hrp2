OUTPUT_FORMAT("elf32-littlearm", "elf32-bigarm","elf32-littlearm") 
OUTPUT_ARCH(arm)
ENTRY(_module_cfg_tab)

PHDRS
{
    text PT_LOAD;
    data PT_LOAD;
}

/**
 * Only .text, .rodata, .got, .data, .bss and DEBUG sections are supported.
 */

SECTIONS
{
    /**
     * Text Segment including the following sections.
     * 1) .text
     * 2) .rodata
     * 3) .rel
     * 4) .rel.plt
     * 5) .got
     * 6) .got.plt
     * 7) .dynsym
     * 8) .dynstr
     * 9) .hash
     */
    .text :
    {
        . = ALIGN(4);
        *(.text)
        *(.text.*)
        *(.module.text)
    } : text
    .rodata : 
    { 
        . = ALIGN(4);
        *(.rodata)
        *(.rodata.*)
        *(.data.rel.ro)
        . = ALIGN(4);
        __init_array_start = .;
        *(SORT(.init_array.*))
        *(.init_array)
        __init_array_end = .;
        __fini_array_start = .;
        *(SORT(.fini_array.*))
        *(.fini_array)
        __fini_array_end = .;
    } : text
    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } : text
    .ARM.exidx :
    {
        . = ALIGN(4);
        __exidx_start = .;
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
        __exidx_end = .;
    } : text
    .rel.plt :
    {
     __dummy_rel_plt = .;
/*        . = ALIGN(4);
        *(.rel.plt) */
    } : text
    .rel : 
    {
        . = ALIGN(4);
        *(.rel.*) 
    } : text
    .dynsym : 
    { 
        . = ALIGN(4);
        *(.dynsym) 
    } : text
    .dynstr : 
    { 
        . = ALIGN(4);
        *(.dynstr)
    } : text
    .hash : 
    { 
        . = ALIGN(4);
        *(.hash) 
    } : text
    .got :
    { 
        . = ALIGN(4);
        *(.got)
        *(.got.plt)
    } : text
    /*.got.plt :
    { 
        . = ALIGN(4);
    } : text*/

    /**
     * Data Segment including the following sections.
     * 1) .data
     * 2) .bss
     */
    .data :
    {
        . = ALIGN(4);
        *(.data)
        *(.data.*)
    } : data
    .bss :
    {
        . = ALIGN(4);   
        *(.bss)
        *(.bss.*)
        *(COMMON)
    } : data

	/* DWARF debug sections.
	Symbols in the DWARF debugging sections are relative to 
	the beginning of the section so we begin them at 0.  */

	/* DWARF 1 */
	.debug          0 : { *(.debug) }
	.line           0 : { *(.line) }

	/* GNU DWARF 1 extensions */
	.debug_srcinfo  0 : { *(.debug_srcinfo) }
	.debug_sfnames  0 : { *(.debug_sfnames) }

	/* DWARF 1.1 and DWARF 2 */
	.debug_aranges  0 : { *(.debug_aranges) }
	.debug_pubnames 0 : { *(.debug_pubnames) }

	/* DWARF 2 */
	.debug_info     0 : { *(.debug_info) }
	.debug_abbrev   0 : { *(.debug_abbrev) }
	.debug_line     0 : { *(.debug_line) }
	.debug_frame    0 : { *(.debug_frame) }
	.debug_str      0 : { *(.debug_str) }
	.debug_loc      0 : { *(.debug_loc) }
	.debug_macinfo  0 : { *(.debug_macinfo) }
	.debug_ranges   0 : { *(.debug_ranges) }

	/* SGI/MIPS DWARF 2 extensions */
	.debug_weaknames 0 : { *(.debug_weaknames) }
	.debug_funcnames 0 : { *(.debug_funcnames) }
	.debug_typenames 0 : { *(.debug_typenames) }
	.debug_varnames  0 : { *(.debug_varnames) }

    /**
     * Discard unsupported input sections
     */
    /DISCARD/ : { *(*) }

}
