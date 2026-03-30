#ifndef CSR_H
#define CSR_H

#define CSR_CRMD        0x0
#define CSR_PRMD        0x1
#define CSR_EUEN        0x2
#define CSR_MISC        0x3
#define CSR_ECFG        0x4
#define CSR_ESTAT       0x5
#define CSR_ERA         0x6
#define CSR_BADV        0x7
#define CSR_BADI        0x8
#define CSR_EENTRY      0xC
#define CSR_TLBIDX      0x10
#define CSR_TLBEHI      0x11
#define CSR_TLBELO0     0x12
#define CSR_TLBELO1     0x13
#define CSR_ASID        0x18
#define CSR_PGDL        0x19
#define CSR_PGDH        0x1A
#define CSR_PGD         0x1B
#define CSR_PWCL        0x1C
#define CSR_PWCH        0x1D
#define CSR_STLBPS      0x1E
#define CSR_RVACFG      0x1F
#define CSR_CPUID       0x20
#define CSR_PRCFG1      0x21
#define CSR_PRCFG2      0x22
#define CSR_PRCFG3      0x23
#define CSR_SAVE0       0x30
#define CSR_SAVE1       0x31
#define CSR_SAVE2       0x32
#define CSR_SAVE3       0x33
#define CSR_SAVE4       0x34
#define CSR_SAVE5       0x35
#define CSR_SAVE6       0x36
#define CSR_SAVE7       0x37
#define CSR_SAVE8       0x38
#define CSR_SAVE9       0x39
#define CSR_SAVE10      0x3A
#define CSR_SAVE11      0x3B
#define CSR_SAVE12      0x3C
#define CSR_SAVE13      0x3D
#define CSR_SAVE14      0x3E
#define CSR_SAVE15      0x3F
#define CSR_TID         0x40
#define CSR_TCFG        0x41
#define CSR_TVAL        0x42
#define CSR_CNTC        0x43
#define CSR_TICLR       0x44
#define CSR_LLBCTL      0x60
#define CSR_IMPCTL1     0x80
#define CSR_IMPCTL2     0x81
#define CSR_TLBRENTRY   0x88
#define CSR_TLBRBADV    0x89
#define CSR_TLBRERA     0x8A
#define CSR_TLBRSAVE    0x8B
#define CSR_TLBRELO0    0x8C
#define CSR_TLBRELO1    0x8D
#define CSR_TLBREHI     0x8E
#define CSR_TLBRPRMD    0x8F
#define CSR_MERRCTL     0x90
#define CSR_MERRINFO1   0x91
#define CSR_MERRINFO2   0x92
#define CSR_MERRENTRY   0x93
#define CSR_MERRERA     0x94
#define CSR_MERRSAVE    0x95
#define CSR_CTAG        0x98
#define CSR_DMW0        0x180
#define CSR_DMW1        0x181
#define CSR_DMW2        0x182
#define CSR_DMW3        0x183
#define CSR_DBG         0x500
#define CSR_DERA        0x501
#define CSR_DSAVE       0x502

#define CSR_FIELD(val, high, low) (((val) >> (low)) & ((1ULL << ((high) - (low) + 1)) - 1))
#define CSR_SET_FIELD(val, high, low, new_val) \
    (((val) & ~(((1ULL << ((high) - (low) + 1)) - 1) << (low))) | ((new_val) << (low)))

#define CRMD_PLV_SHIFT      0
#define CRMD_PLV_WIDTH      2
#define CRMD_PLV_MASK       ((1ULL << CRMD_PLV_WIDTH) - 1)
#define CRMD_PLV(val)       CSR_FIELD(val, 1, 0)
#define CRMD_SET_PLV(val, v) CSR_SET_FIELD(val, 1, 0, v)

#define CRMD_IE_SHIFT       2
#define CRMD_IE             (1ULL << CRMD_IE_SHIFT)
#define CRMD_IE_SET(val)    ((val) | CRMD_IE)
#define CRMD_IE_CLEAR(val)  ((val) & ~CRMD_IE)

#define CRMD_DA_SHIFT       3
#define CRMD_DA             (1ULL << CRMD_DA_SHIFT)

#define CRMD_PG_SHIFT       4
#define CRMD_PG             (1ULL << CRMD_PG_SHIFT)

#define CRMD_DATF_SHIFT     5
#define CRMD_DATF_WIDTH     2
#define CRMD_DATF_MASK      ((1ULL << CRMD_DATF_WIDTH) - 1)
#define CRMD_DATF(val)      CSR_FIELD(val, 6, 5)

#define CRMD_DATM_SHIFT     7
#define CRMD_DATM_WIDTH     2
#define CRMD_DATM_MASK      ((1ULL << CRMD_DATM_WIDTH) - 1)
#define CRMD_DATM(val)      CSR_FIELD(val, 8, 7)

#define CRMD_WE_SHIFT       9
#define CRMD_WE             (1ULL << CRMD_WE_SHIFT)

#define PRMD_PPLV_SHIFT     0
#define PRMD_PPLV_WIDTH     2
#define PRMD_PPLV_MASK      ((1ULL << PRMD_PPLV_WIDTH) - 1)
#define PRMD_PPLV(val)      CSR_FIELD(val, 1, 0)
#define PRMD_SET_PPLV(val, v) CSR_SET_FIELD(val, 1, 0, v)

#define PRMD_PIE_SHIFT      2
#define PRMD_PIE            (1ULL << PRMD_PIE_SHIFT)

#define PRMD_PWE_SHIFT      3
#define PRMD_PWE            (1ULL << PRMD_PWE_SHIFT)

#define EUEN_FPE_SHIFT      0
#define EUEN_FPE            (1ULL << EUEN_FPE_SHIFT)

#define EUEN_SXE_SHIFT      1
#define EUEN_SXE            (1ULL << EUEN_SXE_SHIFT)

#define EUEN_ASXE_SHIFT     2
#define EUEN_ASXE           (1ULL << EUEN_ASXE_SHIFT)

#define EUEN_BTE_SHIFT      3
#define EUEN_BTE            (1ULL << EUEN_BTE_SHIFT)

#define ECFG_LIE_SHIFT      0
#define ECFG_LIE_WIDTH      13
#define ECFG_LIE_MASK       ((1ULL << ECFG_LIE_WIDTH) - 1)

#define ECFG_VS_SHIFT       16
#define ECFG_VS_WIDTH       3
#define ECFG_VS_MASK        ((1ULL << ECFG_VS_WIDTH) - 1)
#define ECFG_VS(val)        CSR_FIELD(val, 18, 16)

#define ESTAT_IS_SHIFT      0
#define ESTAT_IS_WIDTH      13
#define ESTAT_IS_MASK       ((1ULL << ESTAT_IS_WIDTH) - 1)
#define ESTAT_IS(val)       CSR_FIELD(val, 12, 0)

#define ESTAT_ECODE_SHIFT   16
#define ESTAT_ECODE_WIDTH   6
#define ESTAT_ECODE_MASK    ((1ULL << ESTAT_ECODE_WIDTH) - 1)
#define ESTAT_ECODE(val)    CSR_FIELD(val, 21, 16)

#define ESTAT_ESUBCODE_SHIFT 22
#define ESTAT_ESUBCODE_WIDTH 9
#define ESTAT_ESUBCODE_MASK ((1ULL << ESTAT_ESUBCODE_WIDTH) - 1)
#define ESTAT_ESUBCODE(val) CSR_FIELD(val, 30, 22)

#define ESTAT_EXCODE(val)   ((ESTAT_ECODE(val) << 6) | ESTAT_ESUBCODE(val))

#define EXCODE_INT          0
#define EXCODE_PIL          1
#define EXCODE_PIS          2
#define EXCODE_PIF          3
#define EXCODE_PME         4
#define EXCODE_PNR         5
#define EXCODE_PNX         6
#define EXCODE_PPI         7
#define EXCODE_ADE         8
#define EXCODE_ALE         9
#define EXCODE_BCE         10
#define EXCODE_SYS         11
#define EXCODE_BRK         12
#define EXCODE_INE         13
#define EXCODE_IPE         14
#define EXCODE_FPD         15
#define EXCODE_SXD         16
#define EXCODE_ASXD        17
#define EXCODE_FPE         18
#define EXCODE_WPE         19
#define EXCODE_BTD         20
#define EXCODE_BTE         21
#define EXCODE_GSPR        22
#define EXCODE_HVC         23
#define EXCODE_GCM         24
#define EXCODE_TLBRI       64
#define EXCODE_TLBRM       65
#define EXCODE_TLBRI_NX    66
#define EXCODE_TLBRI_NR    67
#define EXCODE_TLBPI       68
#define EXCODE_TLBPM       69
#define EXCODE_TLBPI_NX    70
#define EXCODE_TLBPI_NR    71

#define TLBIDX_INDEX_SHIFT  0
#define TLBIDX_INDEX_WIDTH  12
#define TLBIDX_INDEX_MASK   ((1ULL << TLBIDX_INDEX_WIDTH) - 1)
#define TLBIDX_INDEX(val)   CSR_FIELD(val, 11, 0)

#define TLBIDX_PS_SHIFT     24
#define TLBIDX_PS_WIDTH     6
#define TLBIDX_PS_MASK      ((1ULL << TLBIDX_PS_WIDTH) - 1)
#define TLBIDX_PS(val)      CSR_FIELD(val, 29, 24)
#define TLBIDX_SET_PS(val, v) CSR_SET_FIELD(val, 29, 24, v)

#define TLBIDX_NE_SHIFT     31
#define TLBIDX_NE           (1ULL << TLBIDX_NE_SHIFT)

#define TLBEHI_VPPN_SHIFT   13
#define TLBEHI_VPPN_WIDTH   35
#define TLBEHI_VPPN_MASK    ((1ULL << TLBEHI_VPPN_WIDTH) - 1)
#define TLBEHI_VPPN(val)    CSR_FIELD(val, 47, 13)
#define TLBEHI_SET_VPPN(val, v) CSR_SET_FIELD(val, 47, 13, v)

#define TLBELO_V_SHIFT      0
#define TLBELO_V            (1ULL << TLBELO_V_SHIFT)

#define TLBELO_D_SHIFT      1
#define TLBELO_D            (1ULL << TLBELO_D_SHIFT)

#define TLBELO_PLV_SHIFT    2
#define TLBELO_PLV_WIDTH    2
#define TLBELO_PLV_MASK     ((1ULL << TLBELO_PLV_WIDTH) - 1)
#define TLBELO_PLV(val)     CSR_FIELD(val, 3, 2)

#define TLBELO_MAT_SHIFT    4
#define TLBELO_MAT_WIDTH    2
#define TLBELO_MAT_MASK     ((1ULL << TLBELO_MAT_WIDTH) - 1)
#define TLBELO_MAT(val)     CSR_FIELD(val, 5, 4)

#define TLBELO_G_SHIFT      6
#define TLBELO_G            (1ULL << TLBELO_G_SHIFT)

#define TLBELO_PPN_SHIFT    12
#define TLBELO_PPN_WIDTH    36
#define TLBELO_PPN_MASK     ((1ULL << TLBELO_PPN_WIDTH) - 1)
#define TLBELO_PPN(val)     CSR_FIELD(val, 47, 12)
#define TLBELO_SET_PPN(val, v) CSR_SET_FIELD(val, 47, 12, v)

#define TLBELO_NR_SHIFT     61
#define TLBELO_NR           (1ULL << TLBELO_NR_SHIFT)

#define TLBELO_NX_SHIFT     62
#define TLBELO_NX           (1ULL << TLBELO_NX_SHIFT)

#define TLBELO_RPLV_SHIFT   63
#define TLBELO_RPLV         (1ULL << TLBELO_RPLV_SHIFT)

#define ASID_ASID_SHIFT     0
#define ASID_ASID_WIDTH     10
#define ASID_ASID_MASK      ((1ULL << ASID_ASID_WIDTH) - 1)
#define ASID_ASID(val)      CSR_FIELD(val, 9, 0)
#define ASID_SET_ASID(val, v) CSR_SET_FIELD(val, 9, 0, v)

#define ASID_ASIDBITS_SHIFT 16
#define ASID_ASIDBITS_WIDTH 8
#define ASID_ASIDBITS(val)  CSR_FIELD(val, 23, 16)

#define PGDL_BASE_SHIFT     12
#define PGDL_BASE_MASK      (~((1ULL << PGDL_BASE_SHIFT) - 1))
#define PGDL_BASE(val)      ((val) & PGDL_BASE_MASK)

#define TCFG_EN_SHIFT       0
#define TCFG_EN             (1ULL << TCFG_EN_SHIFT)

#define TCFG_PERIODIC_SHIFT 1
#define TCFG_PERIODIC       (1ULL << TCFG_PERIODIC_SHIFT)

#define TCFG_INITVAL_SHIFT  2
#define TCFG_INITVAL_WIDTH  46
#define TCFG_INITVAL_MASK   ((1ULL << TCFG_INITVAL_WIDTH) - 1)
#define TCFG_INITVAL(val)   CSR_FIELD(val, 47, 2)

#define TICLR_CLR_SHIFT     0
#define TICLR_CLR           (1ULL << TICLR_CLR_SHIFT)

#define LLBCTL_ROLLB_SHIFT  0
#define LLBCTL_ROLLB        (1ULL << LLBCTL_ROLLB_SHIFT)

#define LLBCTL_WCLLB_SHIFT  1
#define LLBCTL_WCLLB        (1ULL << LLBCTL_WCLLB_SHIFT)

#define LLBCTL_KLO_SHIFT    2
#define LLBCTL_KLO          (1ULL << LLBCTL_KLO_SHIFT)

#define TLBRERA_IS_TLBR_SHIFT 0
#define TLBRERA_IS_TLBR     (1ULL << TLBRERA_IS_TLBR_SHIFT)

#define TLBRPRMD_PPLV_SHIFT  0
#define TLBRPRMD_PPLV_WIDTH  2
#define TLBRPRMD_PPLV_MASK   ((1ULL << TLBRPRMD_PPLV_WIDTH) - 1)
#define TLBRPRMD_PPLV(val)   CSR_FIELD(val, 1, 0)

#define TLBRPRMD_PIE_SHIFT   2
#define TLBRPRMD_PIE         (1ULL << TLBRPRMD_PIE_SHIFT)

#define TLBRPRMD_PWE_SHIFT   4
#define TLBRPRMD_PWE         (1ULL << TLBRPRMD_PWE_SHIFT)

#define DMW_PLV0_SHIFT       0
#define DMW_PLV0             (1ULL << DMW_PLV0_SHIFT)

#define DMW_PLV1_SHIFT       1
#define DMW_PLV1             (1ULL << DMW_PLV1_SHIFT)

#define DMW_PLV2_SHIFT       2
#define DMW_PLV2             (1ULL << DMW_PLV2_SHIFT)

#define DMW_PLV3_SHIFT       3
#define DMW_PLV3             (1ULL << DMW_PLV3_SHIFT)

#define DMW_MAT_SHIFT        4
#define DMW_MAT_WIDTH        2
#define DMW_MAT_MASK         ((1ULL << DMW_MAT_WIDTH) - 1)
#define DMW_MAT(val)         CSR_FIELD(val, 5, 4)

#define DMW_VSEG_SHIFT       60
#define DMW_VSEG_WIDTH       4
#define DMW_VSEG_MASK        ((1ULL << DMW_VSEG_WIDTH) - 1)
#define DMW_VSEG(val)        CSR_FIELD(val, 63, 60)
#define DMW_SET_VSEG(val, v) CSR_SET_FIELD(val, 63, 60, v)

#define MAT_UC               0
#define MAT_WC               1
#define MAT_WB               2
#define MAT_WEAKNC           3

#define PLV_KERN             0
#define PLV_USER             3

#endif
