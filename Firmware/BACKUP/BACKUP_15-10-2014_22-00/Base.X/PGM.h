
typedef struct _PGM_CFG
{
    unsigned char valChk1;
    unsigned char valChk2;

    unsigned char menu;
    unsigned char sensId;
    unsigned char baseId;
}ts_PGM_CFG;

ts_PGM_CFG pgm_cfg;

void PGMInit(unsigned char);
void PGMSelLong(void);
void PGMWriteNvm(void);
#define PGMGetSensId() pgm_cfg.senorId;
#define PGMGetBaseId() pgm_cfg.baseId;
