$
$ Generate 'ev3api_cfg.h' & 'ev3api_cfg.c'
$

$FILE "ev3api_cfg.c"$
#include <kernel.h>$NL$
$INCLUDES$
#include "platform_interface_layer.h"$NL$
$NL$

$FILE "ev3api_cfg.h"$
#pragma once$NL$
$NL$

$ ==================== 
$ Handle EV3_CRE_CYC
$ ====================

$
$ Generate IDs
$ 

$FOREACH id EV3CYC.ID_LIST$
$FILE "ev3api_cfg.h"$
    extern ID _ev3api_id_$id$;$NL$
    #define $id$ ((const ID)(_ev3api_id_$id$))$NL$
$FILE "ev3api_cfg.c"$
    ID _ev3api_id_$id$;$NL$
$END$

$
$ Generate initialize function
$ 

$FILE "ev3api_cfg.c"$

void _initialize_ev3api_cyc() {$NL$
$IF LENGTH(EV3CYC.ID_LIST)$
    $TAB$ER_ID ercd;$NL$
    $TAB$T_CCYC pk_ccyc;$NL$
$END$

$FOREACH id EV3CYC.ID_LIST$
    $NL$
    $TAB$pk_ccyc.cycatr = $EV3CYC.CYCATR[id]$;$NL$
    $TAB$pk_ccyc.exinf = $EV3CYC.EXINF[id]$;$NL$
    $TAB$pk_ccyc.cychdr = $EV3CYC.CYCHDR[id]$;$NL$
    $TAB$pk_ccyc.cyctim = $EV3CYC.CYCTIM[id]$;$NL$
    $TAB$pk_ccyc.cycphs = $EV3CYC.CYCPHS[id]$;$NL$
    $TAB$ercd = _ev3_acre_cyc(&pk_ccyc);$NL$
    $TAB$assert(ercd > 0);$NL$
    $TAB$_ev3api_id_$id$ = ercd;$NL$
$END$

}$NL$

