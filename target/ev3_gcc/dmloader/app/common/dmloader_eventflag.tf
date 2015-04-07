$ ==================== 
$ Handle CRE_FLG
$ Input:
$     tnum_mod_cfg_entries
$ Output:
$     Event flag IDs in 'module_cfg.h/.c'
$     _module_cflg_tab in 'module_cfg.c'
$     tnum_mod_cfg_entries (updated)
$     MODCFGTAB (updated)
$ ====================

$
$ Check protection domains
$
$FOREACH id FLG.ID_LIST$
$IF !EQ(FLG.DOMAIN[id], "") && !EQ(FLG.DOMAIN[id], "TDOM_APP")$
    $ERROR FLG.TEXT_LINE[id]$E_NOSPT: 
        $FORMAT(_("%1% `%2%\' in %3% must belong to TDOM_NONE or TDOM_APP in dynamic loading mode"), "event flag", id, FLG.APINAME[id])$
    $END$   
$END$
$END$

$
$ Generate IDs
$ 
$FOREACH id FLG.ID_LIST$
$FILE "module_cfg.h"$
    extern ID _module_id_$id$;$NL$
    #define $id$ ((const ID)(_module_id_$id$))$NL$
$FILE "module_cfg.c"$
    ID _module_id_$id$ __attribute__((section (".module.text")));$NL$
$END$

$
$ Generate '_module_cflg_tab' and update '_module_cfg_tab'
$
$FILE "module_cfg.c"$
$index = 0$
static const T_CFLG _module_cflg_tab[$LENGTH(FLG.ID_LIST)$] = {$NL$
$FOREACH id FLG.ID_LIST$
    $TAB${ $FLG.FLGATR[id]$, $FLG.IFLGPTN[id]$ },$NL$
    $MODCFGTAB.SFNCD[tnum_mod_cfg_entries] = "TSFN_CRE_FLG"$
    $MODCFGTAB.ARGUMENT[tnum_mod_cfg_entries] = FORMAT("&_module_cflg_tab[%1%]",index)$
    $MODCFGTAB.RETVALPTR[tnum_mod_cfg_entries] = FORMAT("&_module_id_%1%",id)$
    $tnum_mod_cfg_entries = tnum_mod_cfg_entries + 1$
    $index = index + 1$
$END$
};$NL$

