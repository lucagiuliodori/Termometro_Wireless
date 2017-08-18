
typedef struct _PGM_CFG
{
    unsigned char valChk1;
    unsigned char valChk2;

    unsigned char menu;
    unsigned char sensId;
    unsigned char baseId;
}ts_PGM_CFG;

typedef union _PGM_FLG
{
    unsigned char val;
    struct
    {
        unsigned int pgmonoff:1;
        unsigned int evPgmOn:1;
        unsigned int evPgmOff:1;
    };
}tu_PGM_FLG;

tu_PGM_FLG pgm_flg;
ts_PGM_CFG pgm_cfg;
const ts_PGM_CFG nvm_pgm_cfg={0x5C,0xC5,0,0,0};

void PGMInit(unsigned char);
void PGMTick(void);
void PGMSelDown(void);
void PGMSelLong(void);
void PGMWriteNvm(void);
void PGMPgmOff(void);
#define PGMGetSensId() pgm_cfg.senorId;
#define PGMGetBaseId() pgm_cfg.baseId;
