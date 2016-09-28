/***************************************************************************************
 *
 *  WRITEPAD(r): Handwriting Recognition Engine (HWRE) and components.
 *  Copyright (c) 2001-2016 PhatWare (r) Corp. All rights reserved.
 *
 *  Licensing and other inquires: <developer@phatware.com>
 *  Developer: Stan Miasnikov, et al. (c) PhatWare Corp. <http://www.phatware.com>
 *
 *  WRITEPAD HWRE is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU "AS-IS"
 *  AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR OTHERWISE,
 *  INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY OR
 *  FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL PHATWARE CORP.
 *  BE LIABLE TO YOU OR ANYONE ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER,
 *  INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS OF USE, SAVINGS
 *  OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, WHETHER OR NOT PHATWARE CORP.
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH LOSS, HOWEVER CAUSED AND ON
 *  ANY THEORY OF LIABILITY, ARISING OUT OF OR IN CONNECTION WITH THE
 *  POSSESSION, USE OR PERFORMANCE OF THIS SOFTWARE.
 *  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with WritePad.  If not, see <http://www.gnu.org/licenses/>.
 *
 **************************************************************************************/

#ifndef DTP_H_INCLUDED
#define DTP_H_INCLUDED

/* -------------------------- Defines for DTP -------------------------------- */

#define XT_COUNT            64                   /* Number of XBEND elements   */
#define DTP_XTB_SIZE     (XT_COUNT/2)            /* Size of dtp buffer for XR  */
#define XZ_COUNT            16                   /* Number of sizes            */
#define DTP_XZB_SIZE     (XZ_COUNT/2)            /* Size of dtp buffer for XZ  */
#define XH_COUNT            16                   /* Number of Height values    */
#define DTP_XHB_SIZE     (XH_COUNT/2)            /* Size of dtp buffer for H values */
#define XS_COUNT            16                   /* Number of Shift values     */
#define DTP_XSB_SIZE     (XS_COUNT/2)            /* Size of dtp buffer for S values */
#define XO_COUNT            32                   /* Number of Orientation values     */
#define DTP_XOB_SIZE     (XO_COUNT/2)            /* Size of dtp buffer for O values */

#define XR_COUNT           XT_COUNT

#define DTP_MAXVARSPERLET   16                   /* How many vars each letter has */
#define DTP_FIRSTSYM        32                   /* First definable DTP sym    */
#define DTP_XR_SIZE         12                   /* Max length of DTP prototype*/

#define ST_CONV_RANGE      128

#define CONV_TBL_SIZE       72
#define DTP_NUMSYMBOLS    (127+CONV_TBL_SIZE-DTP_FIRSTSYM)  /* How many letters in DTI    */

#define DTP_DTP_OBJTYPE   "DTI4"                 /* Object type of current DTP */
#define DTP_DTP_VER       "2.00"                 /* Version of current DTP */
#define DTP_ID_LEN          4                    /* LEngth of ID string in DTP */
#define DTP_FNAME_LEN      128                   /* Pathname of dtp len        */

#define DTP_CAP_BIT        0x01                  /* Var capitalization flagiting */
#define DTP_PICT_OFS        1                    /* Offset from right of picture group */
//#define DTP_ARDLV_BIT      0x10                  /* Ardulovskij flazhok */
#define DTP_OFS_WW          4                    /* Offset from right of WritingWay field in var_veis */
#define DTP_RESTRICTED_SET 0x10                  /* Veis flag of restricted prototype set */

#define DTP_SIZEOFVEXT (DTP_NUMSYMBOLS*DTP_MAXVARSPERLET*sizeof(_UCHAR))  // Size of learn vex table
#define DTP_SIZEOFCAPT (DTP_NUMSYMBOLS*DTP_MAXVARSPERLET/8)               // Size of capitalization table

/* ---------------- Definitions for OS to REC conversions ------------------ */


/*
 #define  OS_A_umlaut    ((_UCHAR)128)
 #define  OS_a_umlaut    ((_UCHAR)138)
 #define  OS_O_umlaut    ((_UCHAR)133)
 #define  OS_o_umlaut    ((_UCHAR)154)
 #define  OS_U_umlaut    ((_UCHAR)134)
 #define  OS_u_umlaut    ((_UCHAR)159)
 #define  OS_ESZET       ((_UCHAR)167)
 
 #define  REC_A_umlaut   ((_UCHAR)128)
 #define  REC_a_umlaut   ((_UCHAR)129)
 #define  REC_O_umlaut   ((_UCHAR)130)
 #define  REC_o_umlaut   ((_UCHAR)131)
 #define  REC_U_umlaut   ((_UCHAR)132)
 #define  REC_u_umlaut   ((_UCHAR)133)
 #define  REC_ESZET      ((_UCHAR)134)
 */

/* ---------------- Definitions for OS to REC conversions ------------------ */

#define REC_BritishPound    ((_UCHAR)0x7f)
#define REC_A_umlaut        ((_UCHAR)0x80)
#define REC_a_umlaut        ((_UCHAR)0x81)
#define REC_O_umlaut        ((_UCHAR)0x82)
#define REC_o_umlaut        ((_UCHAR)0x83)
#define REC_U_umlaut        ((_UCHAR)0x84)
#define REC_u_umlaut        ((_UCHAR)0x85)
#define REC_ESZET           ((_UCHAR)0x86)

#define REC_A_grave         ((_UCHAR)0x87)
#define REC_a_grave         ((_UCHAR)0x88)
#define REC_A_circumflex    ((_UCHAR)0x89)
#define REC_a_circumflex    ((_UCHAR)0x8a)
#define REC_C_cedilla       ((_UCHAR)0x8b)
#define REC_c_cedilla       ((_UCHAR)0x8c)
#define REC_E_grave         ((_UCHAR)0x8d)
#define REC_e_grave         ((_UCHAR)0x8e)
#define REC_E_acute         ((_UCHAR)0x8f)
#define REC_e_acute         ((_UCHAR)0x90)
#define REC_E_circumflex    ((_UCHAR)0x91)
#define REC_e_circumflex    ((_UCHAR)0x92)
#define REC_I_circumflex    ((_UCHAR)0x93)
#define REC_i_circumflex    ((_UCHAR)0x94)
#define REC_I_umlaut        ((_UCHAR)0x95)
#define REC_i_umlaut        ((_UCHAR)0x96)
#define REC_O_circumflex    ((_UCHAR)0x97)
#define REC_o_circumflex    ((_UCHAR)0x98)
#define REC_U_grave         ((_UCHAR)0x99)
#define REC_u_grave         ((_UCHAR)0x9a)
#define REC_U_circumflex    ((_UCHAR)0x9b)
#define REC_u_circumflex    ((_UCHAR)0x9c)
#define REC_e_umlaut        ((_UCHAR)0x9d)
#define REC_N_numero        ((_UCHAR)0x9e)
#define REC_n_numero        ((_UCHAR)0x9f)

#define REC_A_angstrem      ((_UCHAR)0xa0)
#define REC_a_angstrem      ((_UCHAR)0xa1)
#define REC_Yenn_sign       ((_UCHAR)0xa2)

#define REC_DblBrace_left   ((_UCHAR)0xa3)
#define REC_DblBrace_right  ((_UCHAR)0xa4)
//#define REC_Paragraph_sign  ((_UCHAR)0xa5)
#define REC_Copyright_sign  ((_UCHAR)0xa5)

#define REC_Y_umlaut        ((_UCHAR)0xa6)
#define REC_y_umlaut        ((_UCHAR)0xa7)
#define REC_N_tilda         ((_UCHAR)0xa8)
#define REC_n_tilda         ((_UCHAR)0xa9)

//#define REC_Cent_sign       ((_UCHAR)0xaa)
#define REC_TradeName_sign  ((_UCHAR)0xaa)

#define REC_Question_inv    ((_UCHAR)0xab)
#define REC_Exclamation_inv ((_UCHAR)0xac)

#define REC_A_acute         ((_UCHAR)0xad)
#define REC_a_acute         ((_UCHAR)0xae)
#define REC_I_acute         ((_UCHAR)0xaf)
#define REC_i_acute         ((_UCHAR)0xb0)
#define REC_I_grave         ((_UCHAR)0xb1)
#define REC_i_grave         ((_UCHAR)0xb2)
#define REC_O_acute         ((_UCHAR)0xb3)
#define REC_o_acute         ((_UCHAR)0xb4)
#define REC_O_grave         ((_UCHAR)0xb5)
#define REC_o_grave         ((_UCHAR)0xb6)
#define REC_U_acute         ((_UCHAR)0xb7)
#define REC_u_acute         ((_UCHAR)0xb8)
#define REC_A_tilda         ((_UCHAR)0xb9)
#define REC_a_tilda         ((_UCHAR)0xba)
#define REC_O_tilda         ((_UCHAR)0xbb)
#define REC_o_tilda         ((_UCHAR)0xbc)
#define REC_E_umlaut        ((_UCHAR)0xbd)
#define REC_oe_letter       ((_UCHAR)0xbe)
#define REC_OE_letter       ((_UCHAR)0xbf)


/*  These definitions are for Windows OS: */

#define OS_BritishPound    ((unsigned char)0xa3)
#define OS_A_umlaut        ((unsigned char)0xc4)
#define OS_a_umlaut        ((unsigned char)0xe4)
#define OS_O_umlaut        ((unsigned char)0xd6)
#define OS_o_umlaut        ((unsigned char)0xf6)
#define OS_U_umlaut        ((unsigned char)0xdc)
#define OS_u_umlaut        ((unsigned char)0xfc)
#define OS_ESZET           ((unsigned char)0xdf)

#define OS_A_grave         ((unsigned char)0xc0)
#define OS_a_grave         ((unsigned char)0xe0)
#define OS_A_circumflex    ((unsigned char)0xc2)
#define OS_a_circumflex    ((unsigned char)0xe2)
#define OS_C_cedilla       ((unsigned char)0xc7)
#define OS_c_cedilla       ((unsigned char)0xe7)
#define OS_E_grave         ((unsigned char)0xc8)
#define OS_e_grave         ((unsigned char)0xe8)
#define OS_E_acute         ((unsigned char)0xc9)
#define OS_e_acute         ((unsigned char)0xe9)
#define OS_E_circumflex    ((unsigned char)0xca)
#define OS_e_circumflex    ((unsigned char)0xea)
#define OS_I_circumflex    ((unsigned char)0xce)
#define OS_i_circumflex    ((unsigned char)0xee)
#define OS_I_umlaut        ((unsigned char)0xcf)
#define OS_i_umlaut        ((unsigned char)0xef)
#define OS_O_circumflex    ((unsigned char)0xd4)
#define OS_o_circumflex    ((unsigned char)0xf4)
#define OS_U_grave         ((unsigned char)0xd9)
#define OS_u_grave         ((unsigned char)0xf9)
#define OS_U_circumflex    ((unsigned char)0xdb)
#define OS_u_circumflex    ((unsigned char)0xfb)
#define OS_e_umlaut        ((unsigned char)0xeb)
#define OS_N_numero        ((unsigned char)0xaa)
#define OS_n_numero        ((unsigned char)0xba)

#define OS_A_angstrem      ((unsigned char)0xc5)
#define OS_a_angstrem      ((unsigned char)0xe5)

#define OS_Yenn_sign       ((unsigned char)0xa5)

#define OS_DblBrace_left   ((unsigned char)0xab)
#define OS_DblBrace_right  ((unsigned char)0xbb)
//#define OS_Paragraph_sign  ((unsigned char)0xb6)

#define OS_Copyright_sign  ((unsigned char)0xa9)

#define OS_Y_umlaut        ((unsigned char)0x9f)
#define OS_y_umlaut        ((unsigned char)0xff)
#define OS_N_tilda         ((unsigned char)0xd1)
#define OS_n_tilda         ((unsigned char)0xf1)

//#define OS_Cent_sign       ((unsigned char)0xa2)

#define OS_TradeName_sign  ((unsigned char)0xae)

#define OS_Question_inv    ((unsigned char)0xbf)
#define OS_Exclamation_inv ((unsigned char)0xa1)

#define OS_A_acute         ((unsigned char)0xc1)
#define OS_a_acute         ((unsigned char)0xe1)
#define OS_I_acute         ((unsigned char)0xcd)
#define OS_i_acute         ((unsigned char)0xed)
#define OS_I_grave         ((unsigned char)0xcc)
#define OS_i_grave         ((unsigned char)0xec)
#define OS_O_acute         ((unsigned char)0xd3)
#define OS_o_acute         ((unsigned char)0xf3)
#define OS_O_grave         ((unsigned char)0xd2)
#define OS_o_grave         ((unsigned char)0xf2)
#define OS_U_acute         ((unsigned char)0xda)
#define OS_u_acute         ((unsigned char)0xfa)
#define OS_A_tilda         ((unsigned char)0xc3)
#define OS_a_tilda         ((unsigned char)0xe3)
#define OS_O_tilda         ((unsigned char)0xd5)
#define OS_o_tilda         ((unsigned char)0xf5)
#define OS_E_umlaut        ((unsigned char)0xcb)
#define OS_oe_letter       ((unsigned char)0x9c)
#define OS_OE_letter       ((unsigned char)0x8c)


/* ------------------- Defines for ROM tables -------------------------------- */
/*
#if defined (FOR_FRENCH)
#define CAP_TABLE_NUM_LET   1
#define CAP_TABLE_NUM_VAR   1
#elif defined (FOR_GERMAN)
#define CAP_TABLE_NUM_LET   30
#define CAP_TABLE_NUM_VAR   9
#elif defined (INTERNATIONAL)
#define CAP_TABLE_NUM_LET   1
#define CAP_TABLE_NUM_VAR   1
#elif defined (FOR_SWED)
#define CAP_TABLE_NUM_LET   29
#define CAP_TABLE_NUM_VAR   9
#else
#define CAP_TABLE_NUM_LET   26
#define CAP_TABLE_NUM_VAR   9
#endif
*/
/* -------------------------- Types definitions ------------------------------ */
// TODO: check

#define CAP_TABLE_NUM_LET   30
#define CAP_TABLE_NUM_VAR   9

typedef _UCHAR  cap_table_type[CAP_TABLE_NUM_LET][CAP_TABLE_NUM_VAR];

typedef _ULONG let_table_type[256];             /* Header of DTE (sym loc table) */
typedef let_table_type _PTR p_let_table_type;
typedef _UCHAR  dtp_xrt_type[XT_COUNT][DTP_XTB_SIZE];/* Reference XR Corr table */
typedef dtp_xrt_type _PTR p_dtp_xrt_type;

typedef struct
{                                 /* Header of DTP file         */
    _CHAR  object_type[DTP_ID_LEN];  /* Type file (now DTP1)       */
    _CHAR  type[DTP_ID_LEN];         /* Type of DTP (EngM, EngP..) */
    _CHAR  version[DTP_ID_LEN];      /* Version number             */
    
    _ULONG dtp_offset;               /* File offset of DTE part    */
    _ULONG dtp_len;                  /* Length of DTE part of data */
    _ULONG dtp_chsum;                /* Checksum of DTE part       */
    
    _ULONG xrt_offset;               /* File offset of XRT part    */
    _ULONG xrt_len;                  /* Length of XRT part of data */
    _ULONG xrt_chsum;                /* Checksum of XRT part       */
    
    _ULONG pdf_offset;               /* Start of PDF part in DTP   */
    _ULONG pdf_len;                  /* Length of PDF part         */
    _ULONG pdf_chsum;                /* ChekSum of PDF part        */
    
    _ULONG pict_offset;              /* Offset of pictures from the beg of DTP file */
    _ULONG pict_len;                 /* Length of pictures         */
    _ULONG pict_chsum;               /* CheckSum of pictures       */
} dtp_header_type, _PTR p_dtp_header_type;

typedef struct
{                                 /* DTP memory descriptor      */
    _CHAR  dtp_fname[DTP_FNAME_LEN]; /* File name of loaded DTP    */
    _CHAR  object_type[DTP_ID_LEN];  /* Type file (now DTP1)       */
    _CHAR  type[DTP_ID_LEN];         /* Type of DTP (EngM, EngP..) */
    _CHAR  version[DTP_ID_LEN];      /* Version number             */
    
    p_VOID h_dtp;                    /* Handle of dtp memory       */
    p_UCHAR p_dtp;                    /* Pointer to locked dtp mem  */
    p_VOID h_ram_dtp;                /* Handle of dtp memory       */
    p_UCHAR p_ram_dtp;                /* Pointer to RAM dtp addresses */
    _ULONG dtp_chsum;                /* CheckSum of dtp memory     */
    p_VOID h_vex;                    /* Handle of vex learning buffer */
    p_UCHAR p_vex;                    /* Pointer of vex learning buffer */
    
    p_VOID h_xrt;                    /* Handle of reference xrtabl */
    p_UCHAR p_xrt;                    /* Pointer to locked xrtabl   */
    _ULONG xrt_chsum;                /* CheckSum of xrtabl memory  */
    
    p_VOID h_pdf;                    /* Same for pdf */
    p_UCHAR p_pdf;
    p_UCHAR p_ram_pdf;
    _ULONG pdf_chsum;
    
    p_VOID h_pict;                   /* Same for pictures */
    p_UCHAR p_pict;
    p_UCHAR p_ram_pict;
    _ULONG pict_chsum;
    
   
} dtp_descr_type, _PTR p_dtp_descr_type;

typedef struct
{                                 /* Header of symbol in DTP    */
    _UCHAR num_vars;                 /* Number of variants of sym  */
    _UCHAR loc_vs_border;            /* Sym location vs border */
    _UCHAR let;                      /* Temp: symbol itself        */
    _UCHAR language;                 /* Temp: language of symbol   */
    _UCHAR var_lens[DTP_MAXVARSPERLET];  /* Array of var lens          */
    _UCHAR var_vexs[DTP_MAXVARSPERLET];  /* Array of VarExtraInfo's    */
    _UCHAR var_veis[DTP_MAXVARSPERLET];  /* Array of Additional VEXs   */
    _UCHAR var_pos[DTP_MAXVARSPERLET];  /* Array of Additional VEXs   */
    _UCHAR var_size[DTP_MAXVARSPERLET];  /* Array of Additional VEXs   */
} dtp_sym_header_type, _PTR p_dtp_sym_header_type;

typedef struct
{                                 /* Prototype XR element definition */
    _UCHAR type;                     /* Temp: main type definition   */
    _UCHAR height;                   /* Temp: main h of xr         */
    _UCHAR attr;                     /* Attributes (flag bits)     */
    _UCHAR penl;                     /* Penalty value for element  */
    _UCHAR xtc[DTP_XTB_SIZE];        /* Type(64) Line of XR corr vaues      */
    _UCHAR xhc[DTP_XHB_SIZE];        /* Height(16) Line of H corr values      */
    _UCHAR xsc[DTP_XSB_SIZE];        /* Shift(16) Line of S corr values      */
    _UCHAR xzc[DTP_XZB_SIZE];        /* Size(16) Line of Z corr vaues      */
    _UCHAR xoc[DTP_XOB_SIZE];        /* Orientation(16) of O corr values      */
} xrpp_type, _PTR p_xrpp_type;

#ifndef DTI_H_INCLUDED
typedef struct
{
    _USHORT len;      /* length of groups' descriptions */
    _UCHAR symbol;   /* ASCII code of symbol           */
    _UCHAR num_groups; /* number of groups             */
    /* groups are placed  after this header */
} pict_symb_header_type, _PTR p_pict_symb_header_type;

typedef struct
{
    _USHORT len;      /* length of pictures' descriptions */
    _UCHAR groupno;  /* number of group                  */
    _UCHAR num_pictures; /* number of groups             */
    /* pictures are placed  after this header */
} pict_group_header_type, _PTR p_pict_group_header_type;

typedef struct
{
    _USHORT len;      /* length of pictures' descriptions */
    /* traces are placed  after this header */
} pict_picture_header_type, _PTR p_pict_picture_header_type;

typedef struct
{
    _UCHAR x;
    _UCHAR y;
} pict_point_type, _PTR p_pict_point_type;
#endif // #ifndef DTP_H_INCLUDED

typedef _UCHAR dtp_vex_type[DTP_NUMSYMBOLS][DTP_MAXVARSPERLET];/* Type for learning vex buffer */
typedef dtp_vex_type _PTR p_dtp_vex_type;          /* Type for learning vex buffer */

/* -------------------------- Prototypes ------------------------------------- */

#ifdef DTE_CONVERTER

#define  dtp_load         dtp_load_2
#define  dtp_unload       dtp_unload_2
#define  dtp_save         dtp_save_2
#define  dtp_lock         dtp_lock_2
#define  dtp_unlock       dtp_unlock_2

#define  LetXrLength      LetXrLength_2

#define  GetNumVarsOfChar GetNumVarsOfChar_2
#define  GetVarOfChar     GetVarOfChar_2
#define  GetVarLenOfChar  GetVarLenOfChar_2
#define  GetVarVex        GetVarVex_2
#define  GetVarExtra      GetVarExtra_2
#define  SetVarVex        SetVarVex_2
#define  SetDefVexes      SetDefVexes_2
#define  SetVarCounter    SetVarCounter_2
#define  GetVarGroup      GetVarGroup_2
#define  SetDefCaps       SetDefCaps_2
#define  SetDefDtpVexes   SetDefDtpVexes_2


#define  SetDefCaps       SetDefCaps_2

#endif // DTE_CONVERTER

_INT     dtp_load(p_CHAR dtpname, _INT what_to_load, p_VOID _PTR dp, _INT langID);
_INT     dtp_unload(p_VOID _PTR dp, _INT langID);
_INT     dtp_save(p_CHAR fname, _INT what_to_save, p_VOID dp, _INT langID);
_INT     dtp_lock(p_VOID dtp_ptr);
_INT     dtp_unlock(p_VOID dtp_ptr);

_VOID    LetXrLength(p_UCHAR min, p_UCHAR max, _SHORT let, _VALUE hdtp, _INT langID);

_INT     CheckVarActive(_UCHAR chIn, _UCHAR nv, _UCHAR ww, p_VOID dtp, _INT langID);
_INT     GetNumVarsOfChar(_UCHAR chIn, p_VOID dtp);
_INT     GetVarOfChar(_UCHAR chIn, _UCHAR nv, p_xrpp_type xvb, p_VOID dtp, _INT langID);
_INT     GetVarLenOfChar(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID);
_INT     GetVarVex(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID);
_INT     GetVarExtra(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID);
_INT     SetVarVex(_UCHAR chIn, _UCHAR nv, _UCHAR vex, p_VOID dtp, _INT langID);
_INT     SetDefDtpVexes(p_VOID dtp);
_INT     SetVarCounter(_UCHAR chIn, _UCHAR nv, _UCHAR cnt, p_VOID dtp, _INT langID);
_INT     GetVarGroup(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID);

_INT     GetPairCapGroup(_UCHAR let, _UCHAR groupNum, _UCHAR EnableVariantSet);
_INT     SetDefCaps(p_VOID dtp);
_INT     SetVarCap(_UCHAR chIn, _UCHAR nv, _UCHAR cap, p_VOID dtp, _INT langID);
_INT     GetVarCap(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID);
_INT     GetVarRewcapAllow(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID);

_INT     GetAutoCorr(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID);
_INT     GetShiftCorr(_UCHAR chIn, _UCHAR nv,_UCHAR nXr, _UCHAR nIn, p_VOID dtp, _INT langID);
_INT     GetSymDescriptor(_UCHAR sym, _UCHAR numv, p_dtp_sym_header_type _PTR psfc, p_VOID dtp);

_INT     GetVarPosSize(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID);


/* ------------------------------------------------------------------------- */
/*  For international versions: changes the os-rec table */
/* and charset used:                                     */

#define  TBL_AMERICAN       0
#define  TBL_INTERNATIONAL  1
#define  TBL_INTER_NODIACR  2

_VOID  SetOsRecTableAndCharSet( _INT interTable );
_INT   SetInternationalCharSet( _INT interTable, p_VOID rcv, p_VOID xrd );

/* --------------------------------------------------------------------------- */

#endif

