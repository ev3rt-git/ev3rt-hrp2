$ ==================== 
$ Handle CRE_SEM
$ Input:
$     tnum_mod_cfg_entries
$ Output:
$     Semaphore IDs in 'module_cfg.h/.c'
$     _module_csem_tab in 'module_cfg.c'
$     tnum_mod_cfg_entries (updated)
$     MODCFGTAB (updated)
$ ====================

$
$ Check task domains
$
$FOREACH id SEM.ID_LIST$
$IF !EQ(SEM.DOMAIN[id], "") && !EQ(SEM.DOMAIN[id], "TDOM_APP")$
    $ERROR SEM.TEXT_LINE[id]$E_NOSPT: 
        $FORMAT(_("%1% `%2%\' in %3% must belong to TDOM_NONE or TDOM_APP in dynamic loading mode"), "semaphore", id, SEM.APINAME[id])$
    $END$   
$END$
$END$

$
$ Generate IDs
$ 
$FOREACH id SEM.ID_LIST$
$FILE "module_cfg.h"$
    extern ID _module_id_$id$;$NL$
    #define $id$ ((const ID)(_module_id_$id$))$NL$
$FILE "module_cfg.c"$
    ID _module_id_$id$ __attribute__((section (".module.text")));$NL$
$END$

$
$ Generate '_module_csem_tab' and update '_module_cfg_tab'
$
$FILE "module_cfg.c"$
$index = 0$
static const T_CSEM _module_csem_tab[$LENGTH(SEM.ID_LIST)$] = {$NL$
$FOREACH id SEM.ID_LIST$
    $TAB${ $SEM.SEMATR[id]$, $SEM.ISEMCNT[id]$, $SEM.MAXSEM[id]$ },$NL$
    $MODCFGTAB.SFNCD[tnum_mod_cfg_entries] = "TSFN_CRE_SEM"$
    $MODCFGTAB.ARGUMENT[tnum_mod_cfg_entries] = FORMAT("&_module_csem_tab[%1%]",index)$
    $MODCFGTAB.RETVALPTR[tnum_mod_cfg_entries] = FORMAT("&_module_id_%1%",id)$
    $tnum_mod_cfg_entries = tnum_mod_cfg_entries + 1$
    $index = index + 1$
$END$
};$NL$

