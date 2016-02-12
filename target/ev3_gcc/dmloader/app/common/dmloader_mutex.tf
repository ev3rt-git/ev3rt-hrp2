$ ==================== 
$ Handle CRE_MTX
$ Input:
$     tnum_mod_cfg_entries
$ Output:
$     Mutex IDs in 'module_cfg.h/.c'
$     _module_cmtx_tab in 'module_cfg.c'
$     tnum_mod_cfg_entries (updated)
$     MODCFGTAB (updated)
$ ====================

$
$ Check static API
$
$FOREACH id MTX.ID_LIST$

$   // Check protection domains
$IF !EQ(MTX.DOMAIN[id], "") && !EQ(MTX.DOMAIN[id], "TDOM_APP")$
    $ERROR MTX.TEXT_LINE[id]$E_NOSPT: 
        $FORMAT(_("%1% `%2%\' in %3% must belong to TDOM_NONE or TDOM_APP in dynamic loading mode"), "mutex", id, MTX.APINAME[id])$
    $END$   
$END$

$	// mtxatrが（［TA_TPRI｜TA_CEILING］）でない場合（E_RSATR）
$IF !(MTX.MTXATR[id] == 0 || MTX.MTXATR[id] == TA_TPRI || MTX.MTXATR[id] == TA_CEILING)$
	$ERROR MTX.TEXT_LINE[id]$E_RSATR:
        $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "mtxatr", MTX.MTXATR[id], id, "CRE_MTX")$
    $END$
$END$

$	// ceilpriが未指定の場合は0と見なす
$IF !LENGTH(MTX.CEILPRI[id])$
	$MTX.CEILPRI[id] = 0$
$END$

$	// (TMIN_TPRI <= ceilpri && ceilpri <= TMAX_TPRI)でない場合（E_PAR）
$IF MTX.MTXATR[id] == TA_CEILING && (MTX.CEILPRI[id] < TMIN_TPRI || TMAX_TPRI < MTX.CEILPRI[id])$
	$ERROR MTX.TEXT_LINE[id]$E_PAR:
        $FORMAT(_("illegal %1% `%2%\' of `%3%\' in %4%"), "ceilpri", MTX.CEILPRI[id], id, "CRE_MTX")$
    $END$
$END$

$END$

$
$ Generate IDs
$ 
$FOREACH id MTX.ID_LIST$
$FILE "module_cfg.h"$
    extern ID _module_id_$id$;$NL$
    #define $id$ ((const ID)(_module_id_$id$))$NL$
$FILE "module_cfg.c"$
    ID _module_id_$id$ __attribute__((section (".module.text")));$NL$
$END$

$
$ Generate '_module_cmtx_tab' and update '_module_cfg_tab'
$
$FILE "module_cfg.c"$
$index = 0$
static const T_CMTX _module_cmtx_tab[$LENGTH(MTX.ID_LIST)$] = {$NL$
$FOREACH id MTX.ID_LIST$
    $TAB${ $MTX.MTXATR[id]$, $MTX.CEILPRI[id]$ },$NL$
    $MODCFGTAB.SFNCD[tnum_mod_cfg_entries] = "TSFN_CRE_MTX"$
    $MODCFGTAB.ARGUMENT[tnum_mod_cfg_entries] = FORMAT("&_module_cmtx_tab[%1%]",index)$
    $MODCFGTAB.RETVALPTR[tnum_mod_cfg_entries] = FORMAT("&_module_id_%1%",id)$
    $tnum_mod_cfg_entries = tnum_mod_cfg_entries + 1$
    $index = index + 1$
$END$
};$NL$

