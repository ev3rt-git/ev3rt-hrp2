$ ==================== 
$ Handle CRE_DTQ
$ Input:
$     tnum_mod_cfg_entries
$ Output:
$     Data queue IDs in 'module_cfg.h/.c'
$     _module_cdtq_tab in 'module_cfg.c'
$     tnum_mod_cfg_entries (updated)
$     MODCFGTAB (updated)
$ ====================

$
$ Check static API
$
$FOREACH id DTQ.ID_LIST$

$   // Check protection domains
$IF !EQ(DTQ.DOMAIN[id], "") && !EQ(DTQ.DOMAIN[id], "TDOM_APP")$
    $ERROR DTQ.TEXT_LINE[id]$E_NOSPT: 
        $FORMAT(_("%1% `%2%\' in %3% must belong to TDOM_NONE or TDOM_APP in dynamic loading mode"), "data queue", id, DTQ.APINAME[id])$
    $END$   
$END$

$	// dtqatrが（［TA_TPRI］）でない場合（E_RSATR）
$IF (DTQ.DTQATR[id] & ~TA_TPRI) != 0$
	$ERROR DTQ.TEXT_LINE[id]$E_RSATR:
        $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "dtqatr", DTQ.DTQATR[id], id, "CRE_DTQ")$
    $END$
$END$

$	// dtqcntが負の場合（E_PAR）
$IF DTQ.DTQCNT[id] < 0$
	$ERROR DTQ.TEXT_LINE[id]$E_PAR:
        $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "dtqcnt", DTQ.DTQCNT[id], id, "CRE_DTQ")$
    $END$
$END$

$	// dtqmbがNULLでない場合（E_NOSPT）
$IF !EQ(DTQ.DTQMB[id], "NULL")$
	$ERROR DTQ.TEXT_LINE[id]$E_NOSPT:
        $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "dtqmb", DTQ.DTQMB[id], id, "CRE_DTQ")$
    $END$
$END$

$END$

$
$ Generate IDs
$ 
$FOREACH id DTQ.ID_LIST$
$FILE "module_cfg.h"$
    extern ID _module_id_$id$;$NL$
    #define $id$ ((const ID)(_module_id_$id$))$NL$
$FILE "module_cfg.c"$
    ID _module_id_$id$ __attribute__((section (".module.text")));$NL$
$END$

$
$ Generate '_module_cdtq_tab' and update '_module_cfg_tab'
$
$FILE "module_cfg.c"$
$index = 0$
static const T_CDTQ _module_cdtq_tab[$LENGTH(DTQ.ID_LIST)$] = {$NL$
$FOREACH id DTQ.ID_LIST$
    $TAB${ $DTQ.DTQATR[id]$, $DTQ.DTQCNT[id]$, $DTQ.DTQMB[id]$ },$NL$
    $MODCFGTAB.SFNCD[tnum_mod_cfg_entries] = "TSFN_CRE_DTQ"$
    $MODCFGTAB.ARGUMENT[tnum_mod_cfg_entries] = FORMAT("&_module_cdtq_tab[%1%]",index)$
    $MODCFGTAB.RETVALPTR[tnum_mod_cfg_entries] = FORMAT("&_module_id_%1%",id)$
    $tnum_mod_cfg_entries = tnum_mod_cfg_entries + 1$
    $index = index + 1$
$END$
};$NL$

