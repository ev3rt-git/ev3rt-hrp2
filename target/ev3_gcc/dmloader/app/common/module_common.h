typedef struct {
    FN         sfncd;
    const void *argument;
    void       *retvalptr;
} MOD_CFG_ENTRY;

#define TSFN_CRE_TSK (1)
#define TSFN_CRE_SEM (2)
#define TSFN_CRE_FLG (3)
#define TSFN_CRE_DTQ (4)
#define TSFN_CRE_PDQ (5)
#define TSFN_CRE_MTX (6)
