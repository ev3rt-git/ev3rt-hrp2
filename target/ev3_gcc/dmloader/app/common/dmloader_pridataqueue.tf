$ ==================== 
$ Handle CRE_PDQ
$ Input:
$     tnum_mod_cfg_entries
$ Output:
$     Priority data queue IDs in 'module_cfg.h/.c'
$     _module_cpdq_tab in 'module_cfg.c'
$     tnum_mod_cfg_entries (updated)
$     MODCFGTAB (updated)
$ ====================

$
$ Check static API
$
$FOREACH id PDQ.ID_LIST$

$   // Check protection domains
$IF !EQ(PDQ.DOMAIN[id], "") && !EQ(PDQ.DOMAIN[id], "TDOM_APP")$
    $ERROR PDQ.TEXT_LINE[id]$E_NOSPT: 
        $FORMAT(_("%1% `%2%\' in %3% must belong to TDOM_NONE or TDOM_APP in dynamic loading mode"), "priority data queue", id, PDQ.APINAME[id])$
    $END$   
$END$

$	// pdqatrが（［TA_TPRI］）でない場合（E_RSATR）
$IF (PDQ.PDQATR[id] & ~TA_TPRI) != 0$
	$ERROR PDQ.TEXT_LINE[id]$E_RSATR:
        $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "pdqatr", PDQ.PDQATR[id], id, "CRE_PDQ")$
    $END$
$END$

$	// pdqcntが負の場合（E_PAR）
$IF PDQ.PDQCNT[id] < 0$
	$ERROR PDQ.TEXT_LINE[id]$E_PAR:
        $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "pdqcnt", PDQ.PDQCNT[id], id, "CRE_PDQ")$
    $END$
$END$

$	// (TMIN_DPRI <= maxdpri && maxdpri <= TMAX_DPRI)でない場合（E_PAR）
$IF !(TMIN_DPRI <= PDQ.MAXDPRI[id] && PDQ.MAXDPRI[id] <= TMAX_DPRI)$
	$ERROR PDQ.TEXT_LINE[id]$E_PAR:
        $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "maxdpri", PDQ.MAXDPRI[id], id, "CRE_PDQ")$
    $END$
$END$

$	// pdqmbがNULLでない場合（E_NOSPT）
$IF !EQ(PDQ.PDQMB[id], "NULL")$
	$ERROR PDQ.TEXT_LINE[id]$E_NOSPT:
        $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "pdqmb", PDQ.PDQMB[id], id, "CRE_PDQ")$
    $END$
$END$

$END$

$
$ Generate IDs
$ 
$FOREACH id PDQ.ID_LIST$
$FILE "module_cfg.h"$
    extern ID _module_id_$id$;$NL$
    #define $id$ ((const ID)(_module_id_$id$))$NL$
$FILE "module_cfg.c"$
    ID _module_id_$id$ __attribute__((section (".module.text")));$NL$
$END$

$
$ Generate '_module_cpdq_tab' and update '_module_cfg_tab'
$
$FILE "module_cfg.c"$
$index = 0$
static const T_CPDQ _module_cpdq_tab[$LENGTH(PDQ.ID_LIST)$] = {$NL$
$FOREACH id PDQ.ID_LIST$
    $TAB${ $PDQ.PDQATR[id]$, $PDQ.PDQCNT[id]$, $PDQ.MAXDPRI[id]$, $PDQ.PDQMB[id]$ },$NL$
    $MODCFGTAB.SFNCD[tnum_mod_cfg_entries] = "TSFN_CRE_PDQ"$
    $MODCFGTAB.ARGUMENT[tnum_mod_cfg_entries] = FORMAT("&_module_cpdq_tab[%1%]",index)$
    $MODCFGTAB.RETVALPTR[tnum_mod_cfg_entries] = FORMAT("&_module_id_%1%",id)$
    $tnum_mod_cfg_entries = tnum_mod_cfg_entries + 1$
    $index = index + 1$
$END$
};$NL$

