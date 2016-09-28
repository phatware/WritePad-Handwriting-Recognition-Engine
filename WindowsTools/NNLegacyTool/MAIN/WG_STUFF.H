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

#ifndef _WG_STUFF_INCLUDE_
#define _WG_STUFF_INCLUDE_

#ifdef _REQ_WIN
#include <windows.h>
#endif /* _REQ_WIN */


//CHE
#define STRIP_PENLAB      0
enum
{
	DBG_NOP,                    // nothing to do
	DBG_OPENTEXTWND,            // now not working
	DBG_CLOSETEXTWND,           // now not working
	DBG_PRINTW,                 // printf in text window
	DBG_DRAWINIT,               // init graphic debugging
	DBG_LINE,                   // draw line
	DBG_CURVE,                  // draw curve
	DBG_WAIT,                   // drawing breakpoint
	DBG_SCANW,                  // scanf from text window
	DBG_XRDRAW,                 // print xr symbol in current text position
	DBG_HILODATA,               // provide general recognition data
	DBG_TRACEDATA,              // provide trajectory data for debugging
	DBG_ANSWERSDATA,            // provide answer data
	DBG_RECWORDSDATA,           // provide rec_words data
	DBG_LINEDATA,               // provide baseline data
	DBG_WORDBEG,                // indicate begin of the group of the strokes
	DBG_WORDEND,                // indicate end of the group of the strokes
	DBG_SETTALKMODE,            // set special quiet output text mode
	DBG_ADDTRACE,               //
	DBG_COLORTRACE,             //
	DBG_ADDRASTER,              //
	DBG_ADDDRAWBOX,             //
	DBG_OFFLEARN,               // learning breakpoint
	DBG_GRAPHDATA,              // provide sygraph data
	DBG_PASSBEGIN,              // indicate begin of the baseline pass
	DBG_PASSEND,                // indicate end of the baseline pass
	DBG_READXRDATA,             // request to analyse xrdata
	DBG_GETXRDATA,              // provide xrdata
	DBG_SORTGRAPH,              //
	DBG_LETTER,                 //
	DBG_VARIANT,                //
	DBG_RULE,                   //
	DBG_OPERATION,              //
	DBG_FLTRACEDATA,            //
	DBG_CHUNKDATA,              //
	DBG_GETCMPWORD,             // request to get cmp for post statistics
	DBG_CORRECTLETTERS,         //
	DBG_NODESFILE,              //
	DBG_LETTERPOS,              //
	DBG_STROKES,               //
	DBG_GETFILENAME
};

enum
{
	HWDBG_ZERO,                 //
	HWDBG_DEBUGLEVEL,           // set debug level (mpr)
	HWDBG_WND,                  // set debugger application window
	HWDBG_FREEZE,               // freeze recognizer
	HWDBG_UNFREEZE,             // unfreeze recognizer
	HWDBG_LOOKLEVEL,            // set upper algs debug level
	HWDBG_MATPRINT,             // request to analyze matrix
	HWDBG_NOP,                  // nothing to do
	HWDBG_XRDATA,               // !!! not supported yet
	HWDBG_LOADVOC,              //
	HWDBG_FREEVOC,              //
	HWDBG_GETRC,                //
	HWDBG_USENEWRC,             //
	HWDBG_RESTORERC,            //
	HWDBG_GETWORDCUT,           //
	HWDBG_SETWORDCUT,           //
	HWDBG_CHE,                  // nothing /* ask IVB about this */
	HWDBG_GETSURELEVEL,         //
	HWDBG_SETSURELEVEL,         //
	HWDBG_BFUNC,                // request to analyze letter sequence
	HWDBG_OFFLEARN,             // request to start learning
	HWDBG_WORDLAYOUT,           // request to print xrdata for letter sequence
	HWDBG_WORDTRAJ,             //
	HWBATCH_WND,                // set batch processing window
	HWDBG_GETMATRIX,
	HWDBG_NEWFILE               // indicates new tap file start processing
};

// values for mapping for 'look' variable
enum
{
	VL_NOPRINT,
	VL_WORDPRINT,
	VL_VOCWPRINT,
	VL_MATRPRINT,
	VL_LCUTPRINT,
	VL_CSTMPRINT,
	VL_XRLPRINT,
	VL_XRCPRINT
};

#define DI_CLEARCURVES  100
#define DI_CLEARLINES   101

#define LAB_MAX_TRACE  32000 /* max points in trace */
#define LAB_RW_SIZE     40  /* w_lim */
#define LAB_XR_SIZE     12  /* XR_SIZE */

#define LAB_NUM_RW      10  /* NUM_RW */
#define LAB_XRINP_SIZE  120 /* XRINP_SIZE */

#define LAB_BLINE       10  /* CB_NUM_VERTEX */

#define PARA 16         /* # of bytes in paragraph */

#ifndef _PENWIN
/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
/* ************************************************************************** */
typedef struct
{
	_USHORT                 all_data_sent;    /* Data collection complete ?   */

	_USHORT                 num_points_got;   /* Number of points in trace */
	_USHORT                 done_to_point;    /* Trace recognized up to this point  */
	_USHORT                 strokes_got;      /* Up to this stroke converted to ps_trace */
	_USHORT                 num_points_inrec; /* Num points in recognition now */
	_USHORT                 num_words_done;   /* Num words place in buffer    */

	p_CHAR                  cmp_word;         /* Pointer to cmp word          */

	p_VOID                  pTrace;           /* PS_point_type _PTR  */
	_SHORT                  bb_top;           /* Boxed mode upper border pos */
	_SHORT                  bb_bottom;        /* Boxed mode lower border pos */

	p_VOID                  prc;              /* rc_type _PTR */
	p_VOID                  pxrdata;          /* xrdata_type _PTR */
	p_VOID                  prwords;          /* rec_w_type _PTR */
	p_VOID                  pmbuf;            /* DES_DATA_TYPE _PTR */
	p_VOID                  pavars;           /* ALS_NODE_TYPE _PTR */
	p_VOID                  prwg;             /* RWG_type _PTR */
	p_VOID                  pctx;             /* chunk data _PTR */

//AVP ???
	p_VOID                  p_word_strokes;   /* Pointer to current word definition structure */
	p_VOID                  p_strokes_info;   /* Pointer to strokes info arrray */
	_INT                    num_word;         /* Number of current word in ws arrary */
//???
	_CHAR                  aCS[PARA*4];
	_CHAR                  aintCS[PARA*4]; //Alpha international
	_CHAR                  andCS[PARA*4];  //Alpha without diacritics
	_CHAR                  nCS[PARA*4];
	_CHAR                  mCS[PARA*4];
	_CHAR                  lCS[PARA*4];
	_CHAR                  eCS[PARA*4];
	_CHAR                  oCS[PARA*4];

//  _UCHAR                  dtefname[PARA*8];
	_CHAR                  dtifname[PARA*8];
	_CHAR                  pdffname[PARA*8];
	_CHAR                  tr_fname[PARA*8];

	_CHAR                  ldbfname[PARA*8]; /* VIC May 21 1994 */
	_CHAR                  voc0name[PARA*8];
	_CHAR                  voc1name[PARA*8];
	_CHAR                  voc2name[PARA*8];
	_CHAR                  voc3name[PARA*8];

// KRAV
	_UCHAR             fl_cut_fly;
	_UCHAR             fl_cut;
	_USHORT            fl_numborder;
	_USHORT            fl_bedit_border;
	_USHORT            fl_separate_let;
	_USHORT            fl_small_caps;
	_CHAR              cmp_word_place[LAB_RW_SIZE];
	_UCHAR             multiply_des;
} rec_info_type, _PTR p_rec_info_type;

/* xr-node structure to describe analyzeable part of the input traj */
typedef struct _LAB_XRNODE_TYPE
{
	_UCHAR  xnxr;           /* xr-node id */
	_UCHAR  xna;            /* xr-node attributes */
	_UCHAR  xnp;            /* xr-node penalty */
	_UCHAR  xnh;            /* xr-node height and horizontal pos. */

	_UCHAR  xns;            // Shift
	_UCHAR  xno;            // Orientation
	_UCHAR  xnd;            // depth //size
} LAB_XRNODE_TYPE;
typedef LAB_XRNODE_TYPE _PTR p_LAB_XRNODE;

/* xr-node structure for use in desibrary */
typedef struct _DES_XRNODE_TYPE
{
	_UCHAR  dxrid;         /* xr-node id */
	_UCHAR  dxattrib;      /* xr-node attributes. */
	_UCHAR  dxpenalty;     /* xr-node penalty */
	_UCHAR  dxheight;      /* xr-node height and horizontal pos. */

} DES_XRNODE_TYPE;
typedef DES_XRNODE_TYPE _PTR p_DES_XRNODE;

typedef struct _ALS_NODE_TYPE
{
	_UCHAR          aword[LAB_RW_SIZE/2];                 /* word */
	_UCHAR          alias[LAB_RW_SIZE/2];                 /* aliases to xr sequences */
	DES_XRNODE_TYPE axrdte[LAB_RW_SIZE/2][LAB_XRINP_SIZE];/* dte xr sequences */
	DES_XRNODE_TYPE axrinp[LAB_XRINP_SIZE];               /* input xr sequence */
} ALS_NODE_TYPE;
typedef ALS_NODE_TYPE _PTR p_ALS_NODE;

typedef _UCHAR DES_VARS_TYPE[LAB_NUM_RW][LAB_RW_SIZE];

typedef struct _DES_DATA_TYPE
{
	_SHORT          num;
	_SHORT          ofs_of_xrattr;          /* Offset of xr strings   */
	_SHORT          pad1;
	_SHORT          ofs_of_vars;            /* Offset of voc variants */
	_SHORT          pad2;

	_SHORT          page;                   /* Page, word in          */
	_SHORT          line;                   /* Line, word in          */
	_SHORT          word;                   /* Number of word in line */
	_SHORT          st_offset;              /* St offset in TIF line  */
	_SHORT          en_offset;              /* End offset in TIF line */

	_SHORT          pen_width;              /* Pen width of cur word  */
	_SHORT          slope;                  /* Slope of cur word      */
	_SHORT          up_border_loc;          /* Line up border location*/
	_SHORT          dn_border_loc;          /* Line down border loc.  */
	_SHORT          up_border_sl;           /* Slope of up border     */
	_SHORT          dn_border_sl;           /* Slope of down border   */
	_SHORT          hstr_str;               /* universal HSTRr        */
	_UCHAR          curv_bord[LAB_BLINE*2]; /* Array describing NB    */
	_SHORT          ams1;
	_SHORT          ams2;
	_SHORT          custom_flags;           /* Custom flags           */
	_SHORT          s_stroke_num;           /* For tabl - num of f str*/
	_SHORT          num_strokes;            /* For tabl - num str in w*/

	_USHORT         time_low;               /* Time of low level procs*/
	_USHORT         time_upper;             /* Time of up level procs */

	_USHORT         xr_sum;                 /* Xr sum check for word  */

	_SHORT          percent;                /* Weight of the answer   */
	_SHORT          attrib;                 /* Some additional status */
	_CHAR           answer[LAB_RW_SIZE];    /* Buffer for answer      */
	_UCHAR          w_vars_weight[LAB_NUM_RW];/* Weights of variants  */
	_SHORT          num_of_vars;            /* Num of voc variants    */
	/*------------------------ variable length part ------------------- */
	DES_XRNODE_TYPE xrinp[LAB_XRINP_SIZE];      /* XrInp buffer           */
	DES_VARS_TYPE   w_vars;                 /* Voc word versions      */
} DES_DATA_TYPE;
typedef DES_DATA_TYPE _PTR p_DES_DATA;

typedef struct _TEST_DATAHANDLE_TYPE
{
	p_DES_DATA      p_des_data;
	p_ALS_NODE      p_als_data;
	p_VOID          p_rc;
} TEST_DATAHANDLE_TYPE;
typedef TEST_DATAHANDLE_TYPE _PTR p__TEST_DATAHANDLE;

/* destination structure to store xr traj's description */
typedef struct _LAB_XRDATA_TYPE
{
	LAB_XRNODE_TYPE xdxr;        /* xr-node */
	_SHORT          xdhotpt;     /* Hot spot pt for the xr-node on the input traj */
	_SHORT          xdbegpt;     /* beginning pt for the xr-node on the input traj */
	_SHORT          xdendpt;     /* ending pt for the xr-node on the input traj */
	_SHORT          xdleft;      /* bounding box for the xr-node on the input traj */
	_SHORT          xdtop;
	_SHORT          xdright;
	_SHORT          xdbottom;
} LAB_XRDATA_TYPE;
typedef LAB_XRDATA_TYPE _PTR p_LAB_XRDATA;

/* destination structure to store recognition result */
typedef struct _LAB_RECWORD_TYPE
{
	_CHAR   rwword[LAB_RW_SIZE];  /* recognition result */
	_UCHAR  rwlinp[LAB_RW_SIZE];  /* size of the xr seq used for the each letter of the word */
	_UCHAR  rwnvar[LAB_RW_SIZE];  /* variant number for each letter of the word + caps flag */
	_UCHAR  rwnone[LAB_RW_SIZE];  /* reserved */
	_SHORT  rwweight;             /* weight */
	_SHORT  rwsource;             /* source identificator */
	_SHORT  rwdattrib;            /* attributes bits for dictionary word */
	_SHORT  rwpadding;
} LAB_RECWORD_TYPE;
typedef LAB_RECWORD_TYPE _PTR p_LAB_RECWORD;

typedef struct _LAB_PT_TYPE
{
	_INT  ptx;
	_INT  pty;
} LAB_PT_TYPE;
typedef LAB_PT_TYPE _PTR p_LAB_PT;

/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
/* Type, defining graph element */
typedef struct _LAB_RWGNODE_TYPE
{
	_UCHAR  rnasym;      /* Symbol from the answer */
	_UCHAR  rnrsym;      /* Symbol really used for recognition (Cap) */
	_UCHAR  rntype;      /* Type of symbol: Normal/Split/Join/Alt */
	_UCHAR  rnvar;       /* Number of prototype used */

	_UCHAR  rnxbeg;      /* Num of begining xr-el of XrData, corresponding */
	_UCHAR  rnxlen;      /* Length of XrData string, corresponding */
	_UCHAR  rnweight;    /* Percent of quality of the symbol */
	_CHAR   rnsrc;       /* ID of source of appearance of the symbol */

	_UCHAR  rnattr;      /* Some attribute (voc attribute) */
	_UCHAR  rnuser;      /* Here can be temp user storage byte */
} LAB_RWGNODE_TYPE;
typedef LAB_RWGNODE_TYPE _PTR p_LAB_RWGNODE;

/* Type, defining PPD element */
typedef struct _LAB_PPNODE_TYPE
{
	LAB_XRNODE_TYPE   pnxr;     /* Prototype xr */
	_UCHAR            pnalias;  /* Number of XrData xr, corresponding to this */
	_UCHAR            pntype;   /* Type of correspondence - VocSkip, InpSkip, Corr */
} LAB_PPNODE_TYPE;
typedef LAB_PPNODE_TYPE _PTR p_LAB_PPNODE;

/* Type, def PPD letter */
typedef LAB_PPNODE_TYPE LAB_RWG_PPD_ARRAY[LAB_XR_SIZE+1];
/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
enum
{
	OBJ_ROOT,
	OBJ_XRDATA,
	OBJ_RECWORDS,
	OBJ_TRAJECTORY,
	OBJ_RWGRAPH,
	OBJ_RWGDATA,
	OBJ_RWGPPD,
	OBJ_BASELINE,
	OBJ_RCONTEXT,
	OBJ_RCCORRECTORDATA
};

enum
{
	CMD_NOP,
	CMD_INF, // get information
	CMD_GET,
	CMD_SET,
	CMD_VOC
};

#define ERROR_BAD_OBJECT  -1
#define ERROR_BAD_PARAM   -2
#define ERROR_BAD_CMD -3

#endif /* _PENWIN */

/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
_LONG DebugRequest (_INT nRequest, _LONG lParam);
#ifdef _REQ_WIN

#define WM_HWDBG        WM_USER+100
#define WM_REFRESH_DLG  (WM_USER+101)

#define PARAPENREC_NAME   "PSREC32.DLL"
#define SETDEBUGMODE_NAME "dbgSetDebugMode"

#define   REC_INIT        "InitRecognition"
#define   REC_CLOSE       "CloseRecognition"
#define   REC_RECOGNIZE   "DoRecognition"
#define   REC_DATA_ACCESS "RCBAccess"

typedef int (WINAPI *dbgSetDebugMode_TYPE)(int nMode, LPARAM lParam);
typedef LPARAM (WINAPI *INTERFACEPROC)(int nMode, LPARAM lParam);

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif  /* __cplusplus */

p_VOID DLLEXPORT LoadDB(int nDBType, p_CHAR pDBfilename);
p_VOID DLLEXPORT FreeDB(p_VOID pDB, int nDBType);
p_VOID DLLEXPORT EnableDB(p_VOID pRCB, p_VOID pDB, int nDBType);
p_VOID DLLEXPORT GetDB(p_VOID pRCB, int nDBType);
_INT DLLEXPORT InitRecognition (_UCHAR _PTR psinifilename, rec_info_type _PTR pri);
_INT DLLEXPORT CloseRecognition (rec_info_type _PTR pri);
_INT DLLEXPORT DoRecognition (rec_info_type _PTR pri);

_INT DLLEXPORT RCBAccess(p_VOID pObject, /* root object Handle */
                         _INT nObjectId,      /* object id */
                         _INT nCommand,       /* command id */
                         _UINT uiParam,       /* multifunction uint parameter */
                         _ULONG ulParam);     /* multifunction ulong parameter */

_INT DLLEXPORT dbgSetDebugMode (_INT nMode, _ULONG lParam);

_INT  GetDTIVersionForPenlab( p_rec_info_type pri );

#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif  /* __cplusplus */

#define READ_MATRIX_ERROR -100
#define MAX_DRAW_LINES    500
#define MAX_DRAW_CURVES   500
#define MAX_DRAW_POINTS   32000
#define MAX_RASTER_HEIGHT 32
#define MAX_RASTER_SIZE   3000
#define MAX_BOX_COUNT     50
#define MAX_SCANW_BUF     10 /* bigor 19-Apr-93 strange costant */

/* =============================================================== */
/* =============================================================== */
/* =============================================================== */
typedef struct _COLORPOINT_TYPE
{
	COLORREF       color ;
	_SHORT         x, y ;
} COLORPOINT_TYPE, _PTR p_COLORPOINT ;

typedef struct _COLORRANGE_TYPE
{
	COLORREF       color ;
	_SHORT         beg, end ;
} COLORRANGE_TYPE, _PTR p_COLORRANGE ;

typedef struct _CURVE_TYPE
{
	COLORREF       Ccolor;
	p_SHORT        Cx;
	p_SHORT        Cy;
	_SHORT         Cbeg;
	_SHORT         Cend;
	_SHORT         Cslow;
} CURVE_TYPE;
typedef CURVE_TYPE _PTR p_CURVE;

typedef struct _DRAWINIT_TYPE
{
	_SHORT         DImode;
	RECT           DIview;
	RECT           DIbox;
} DRAWINIT_TYPE;
typedef DRAWINIT_TYPE _PTR p_DRAWINIT;

typedef struct _XRDRAW_TYPE
{
	_UCHAR         XDxr;
	_UCHAR         XDa;
	_UCHAR         XDp;
	_UCHAR         XDh;
	_SHORT         XDinc;
} XRDRAW_TYPE;
typedef XRDRAW_TYPE _PTR p_XRDRAW;

typedef struct _LINE_TYPE
{
	COLORREF       Lcolor;
	POINT          Lbeg;
	POINT          Lend;
	_SHORT          Lstyle;
	_SHORT          Lwidth;
} LINE_TYPE;
typedef LINE_TYPE _PTR p_LINE;

typedef struct _PRINTFW_TYPE
{
	COLORREF       PWcolor;
	_SHORT         PWfont;
	p_CHAR _PTR   PWpArgs;
} PRINTFW_TYPE;
typedef PRINTFW_TYPE _PTR p_PRINTFW;

typedef struct _XYPRINTW_TYPE
{
	COLORREF       XYPColor;
	_SHORT         XYPx;
	_SHORT         XYPy;
	p_CHAR        XYPpArgs;
} XYPRINTW_TYPE;
typedef XYPRINTW_TYPE _PTR p_XYPRINTW;

typedef struct _TRACEDATA_TYPE
{
	_SHORT         TDnp;
	LPPOINT        TDpt;
} TRACEDATA_TYPE;
typedef TRACEDATA_TYPE _PTR p_TRACEDATA;

typedef struct _BOX_TYPE
{
	_SHORT    left;
	_SHORT    right;
	_SHORT    top;
	_SHORT    bottom;
	COLORREF  color;
	COLORREF  color1;
	_INT      style;
} BOX_TYPE;
typedef BOX_TYPE _PTR p_BOX_TYPE;

#endif /* _REQ_WIN */

typedef struct _SCANW_TYPE
{
	p_CHAR         SCpBuf;
	_SHORT         SCnScanw;
} SCANW_TYPE;
typedef SCANW_TYPE _PTR p_SCANW;

typedef struct _RASTERDATA_TYPE
{
	p_ULONG        lRaster;
	p_ULONG        hRaster;
	_SHORT         nWidth;
	_SHORT         nHeight;
} RASTERDATA_TYPE;
typedef RASTERDATA_TYPE _PTR p_RASTERDATA;

typedef struct _XR_TYPE
{
	_UCHAR         xr;            /* XrElem definition     */
	_UCHAR         a;
	_UCHAR         p;
	_UCHAR         h;

	_UCHAR         shift;
	_UCHAR         orient;
	_UCHAR         depth;


	_SHORT         begpoint;
	_SHORT         endpoint;
} XR_TYPE, _PTR p_XR;

typedef struct _XRDATA_TYPE
{
	XR_TYPE         xr;
	_SHORT          box_left;
	_SHORT          box_up;
	_SHORT          box_right;
	_SHORT          box_down;
} XRDATA_TYPE, _PTR p_XRDATA;

typedef struct _RWORD_TYPE
{
	_CHAR          word[LAB_RW_SIZE];
	_UCHAR         alias[LAB_RW_SIZE];
	_SHORT         stat;
	_SHORT         weight;
	_SHORT         percent;
	_SHORT         flags;
} RWORD_TYPE, _PTR p_RWORD;

/* Post processing data  */
typedef struct _PPD_TYPE
{
	_UCHAR         letter;                    /* Letter, info for */
	_UCHAR         rletter;                   /* Letter, info for */
	_UCHAR         percent;                   /* Weight of word, letter in */
	_UCHAR         num_var;
	_UCHAR         xr[LAB_XRINP_SIZE+1];      /* Used voc xr version */
	_UCHAR         attr[LAB_XRINP_SIZE+1];    /* Attributes for the xr */
	_UCHAR         height[LAB_XRINP_SIZE+1];  /* Height for the xr */
	_UCHAR         alias[LAB_XRINP_SIZE+1];   /* Aliases for the xr */
} PPD_TYPE;
typedef PPD_TYPE _PTR p_PPD;

typedef struct _MATRIX_INPUT
{
	LAB_XRDATA_TYPE xr;
	_UCHAR         sym;
	_SHORT         nvariant;
	_SHORT         nxr;
} MATRIX_INPUT, _PTR p_MATRIX_INPUT;

typedef struct _SETSURELEVEL_TYPE
{
	p_rec_info_type pri;
	_SHORT          surelevel;
} SETSURELEVEL_TYPE, _PTR p_SETSURELEVEL;

typedef struct _READMATRIX
{
	p_rec_info_type        pri;
	p_PPD         ppd;
	p_LAB_XRDATA  pxr_input;

	int           version;

	int           Pi;

	int           nxr;
	int           nVar;

	int           Cxr;
	int           Cattrib;
	int           Cpenalty;
	int           Cheight;
	int           Cshift;

	int           Corient;
	int           Cdepth;
} READMATRIX_TYPE, _PTR p_READMATRIX;

/* information about part of letter */
typedef struct _TRAJINTERVAL_TYPE
{
	_SHORT ibeg;                /* the beginning of the letter */
	_SHORT iend;                /* the end of the letter */
} TRAJINTERVAL_TYPE;
typedef  TRAJINTERVAL_TYPE _PTR p_TRAJINTERVAL;

/* letters output structure  */
typedef struct _LETTERPARTS_TYPE
{
	_UCHAR         parts[LAB_RW_SIZE]; /* number of parts in letter */
	TRAJINTERVAL_TYPE interval; /* beg/ end of parts */
} LETTERPARTS_TYPE;
typedef LETTERPARTS_TYPE _PTR p_LETTERPARTS;

typedef struct GRAPH_TYPE
{
	_UCHAR               type;     /* Graph type (word/letter/number                */
	_USHORT              size;     /* Number of elements in RWS array               */
#ifdef _PENWIN
	p_RWS_type          prwsb;    /* Pointer of array                              */
	p_RWG_PPD_el_type   pppdb;    /* Pointer of PPD                                */
#else /* _PENWIN */
	p_LAB_RWGNODE       prwsb;
	p_LAB_PPNODE        pppdb;
#endif /* _PENWIN */
	p_UCHAR             pselb;    /* Pointer to selected item - for PenLab          */
	p_ULONG             pnumb;    /* Pointer to selected item - for PenLab only     */
#ifdef _PENWIN
	p_RWS_type          prwse;    /* Pointer of array                              */
	p_RWG_PPD_el_type   pppde;    /* Pointer of PPD                                */
#else /* _PENWIN */
	p_LAB_RWGNODE       prwse;
	p_LAB_PPNODE        pppde;
#endif /* _PENWIN */
	p_UCHAR             psele;  /* Pointer to selected item - for PenLab after PP */
	p_ULONG             pnume;  /* Pointer to selected item - for PenLab after PP */

#ifndef _PENWIN
	_BOOL               bafterpp;
#endif /* _PENWIN */
} GRAPH_TYPE, _PTR p_GRAPH_TYPE;

typedef struct GRAPHDATA_TYPE
{
	_SHORT         rwg_size;
	_SHORT         rws_size;
	p_VOID         p_rwg;
	_SHORT         after_pp;
	_BOOL          bReversePass;
} GRAPHDATA_TYPE, _PTR p_GRAPHDATA;


typedef struct _GETXRDATA_TYPE
{
	_SHORT         nxr;
	p_XR           lpxr;
} GETXRDATA_TYPE, _PTR p_GETXRDATA;

typedef struct _HILODATA_TYPE
{
	_SHORT         HLnwords;
	p_RWORD        HLwords;
	_SHORT         HLnppd;
	p_PPD          HLppd;
	_SHORT         HLnparts;
	p_UCHAR        HLparts;
	_SHORT         HLnintervals;
	p_TRAJINTERVAL HLintervals;
} HILODATA_TYPE, _PTR p_HILODATA;

typedef struct _ANSWER_TYPE
{
	_INT          anweight;
	_INT          anstat;
	_INT          anpercent;
	_INT          anflags;
	_INT          ansources;
	_INT          anchain_num;
	_INT          ansrc_id;
	_CHAR         anword[LAB_RW_SIZE];
} ANSWER_TYPE, _PTR p_ANSWER;

typedef struct _ANSWERDATA_TYPE
{
	_SHORT        nanswers;
	p_VOID        answers; /* was p_answ_type, now p_ANSWER */
} ANSWERDATA_TYPE, _PTR p_ANSWERDATA;

typedef struct _RECWORDSDATA_TYPE
{
	_SHORT        nwords;
	p_VOID        words; /* was p_rec_w_type, now p_RWORD */
} RECWORDSDATA_TYPE, _PTR p_RECWORDSDATA;

typedef struct _MPRINT_TYPE
{
	_USHORT        MPflags;
	_SHORT         MPnxr;
	p_LAB_XRDATA   MPxr;
	_CHAR          MPtext[LAB_RW_SIZE];
	_SHORT         MPrange[2];
//??SD
	_INT           CapsMode;
	_INT           BadAmnisty;
// end ??SD
#ifndef _PENWIN
	_VOID _PTR     pInfo;
	_UINT          PrintMatrix;
#endif /* _PENWIN */
} MPRINT_TYPE;
typedef MPRINT_TYPE _PTR p_MPRINT;

#ifndef _PENWIN
typedef struct _LAYOUTWORD_TYPE
{
	rec_info_type _PTR  pRCInfo;
	_CHAR               Word[LAB_RW_SIZE];
	_UINT               SpMode;
	p_LAB_XRDATA        pxr;
} LAYOUTWORD_TYPE;
typedef LAYOUTWORD_TYPE _PTR p_LAYOUTWORD;
#else /* _PENWIN */
typedef struct _LAYOUTWORD_TYPE
{
	RWG_type _PTR     prwg;
	xrdata_type _PTR  pxrdata;
	rc_type _PTR      prc;
} LAYOUTWORD_TYPE;
typedef LAYOUTWORD_TYPE _PTR p_LAYOUTWORD;
#endif /* _PENWIN */

#define LAB_MAX_PARTS_IN_LETTER 8
/* must be the same value as #define MAX_PARTS_IN_LETTER in div_let.h */

typedef struct _PART_TYPE
{
	short ibeg;
	short iend;
} PART_TYPE, _PTR p_PART_TYPE;

typedef struct _WORDTRAJ
{
	char        parts_number[LAB_RW_SIZE];
	PART_TYPE   parts[LAB_MAX_PARTS_IN_LETTER*LAB_RW_SIZE];
} WORDTRAJ, _PTR p_WORDTRAJ;
typedef struct _WORDTRAJECTORY_TYPE
{
	p_LAB_XRDATA    pxr;
	p_LAB_RECWORD   pword;
	p_WORDTRAJ      ptraj;
	p_LAB_RWGNODE   pgraph;
} WORDTRAJECTORY_TYPE;
typedef WORDTRAJECTORY_TYPE _PTR p_WORDTRAJECTORY;

#define MP_DIRECTION       0x8000
#define MP_VIEWCORRELATION 0x0100
#define MP_VIEWEXTRAINFO   0x0200

typedef struct _XR4UPPER_TYPE
{
#ifndef _PENWIN
	_VOID _PTR pInfo;
#else /* _PENWIN */
	LPRC lpRC;
#endif /* _PENWIN */
	p_XR p_Xr;
} XR4UPPER_TYPE;
typedef XR4UPPER_TYPE _PTR p_XR4U;
typedef struct _VOCLOAD_TYPE
{
	p_VOID  vocl_ptr;
	p_CHAR  vocl_filename;
} VOCLOAD_TYPE;
typedef VOCLOAD_TYPE _PTR p_VOCLOAD;

typedef struct _LRNSTAT_TYPE
{
	_VOID _PTR zdp;
	_VOID _PTR zlv;
	_VOID _PTR zld;
	_VOID _PTR zlp;
} LRNSTAT_TYPE;
typedef LRNSTAT_TYPE _PTR p_LRNSTAT;

#define WG_OVR_FLAG         0x10    /* XR attributes */
#define WG_XSTRICT_FLAG     0x20
#define WG_HSTRICT_FLAG     0x40
#define WG_TAIL_FLAG        0x80
#define WG_END_LETTER_FLAG  0x01
#define WG_X_RIGHT_KREST    0x02
#define WG_END_WORD_FLAG    0x04
#define WG_ATTRIBUTES       8 /* GIT: was 4 */

#define WG_OVR_FLAG_COLOR         RGB(0, 0, 255)    /* XR attributes' color */
#define WG_XSTRICT_FLAG_COLOR     RGB(0, 255, 0)
#define WG_HSTRICT_FLAG_COLOR     RGB(255, 0, 0)
#define WG_TAIL_FLAG_COLOR        RGB(255, 255, 255)

#define OPERATION_LESS    6
#define OPERATION_GREATER 7
#define OPERATION_EQUALS  8

typedef struct _OPERATION_STRUCT_TYPE
{
	_LONG  number1;
	_LONG  number2;
	_SHORT operation;
} OPERATION_STRUCT_TYPE, _PTR p_OPERATION_STRUCT;

/* ----------- Symbol Graph definitions ---------------------------------- */

#define LAB_RWS_MAX_ELS        256    /* Arbitrary number of elements in graph */

#define LAB_RWGT_WORD          1      /* Types of RecWordsGraph */
#define LAB_RWGT_SEPLET        2
#define LAB_RWGT_ANY           3

#define LAB_RWST_SYM           1      /* Types of RecWordsSymbol (Symbol)*/
#define LAB_RWST_SPLIT         2      /* Begin division of alternatives  */
#define LAB_RWST_JOIN          3      /* End   division of alternatives  */
#define LAB_RWST_NEXT          4      /* Begin next alternative          */

#define LAB_RWSS_NULL          0      /* Mark of null symbol (terminator of smth else ) */
#define LAB_RWSS_PREFIX        1      /* Mark that there is prefix in the beg of branch */
#define LAB_RWSS_SUFFIX        1      /* Mark that there is suffix in the end of branch */
#define LAB_RWSS_UNIDENTIFIED '_'     /* Mark that there were no variants in this graph column */

#define LAB_XRWG_ALST_EMP      0      /* Types of aliases: EmptyVoc*/
#define LAB_XRWG_ALST_DS       1      /*DoubleSkip                 */
#define LAB_XRWG_ALST_VS       2      /*VocSkip,                   */
#define LAB_XRWG_ALST_CR       3      /*Corr,                      */

/********************* Chunk DBG data for PenLab *************************/

#define CH_MAX_ANSWER_SIZE    28  /* Multiple of 4 nearest to, and greater than 25 */
#define CH_MAX_WAPXPNT_NUM   400
#define CH_MAX_CHUNKS_NUM    200

/* Mirror structures from Chunk recognizer inner guts */

#define UP_CHUNK     1
#define DOWN_CHUNK   2
#define AIR_CHUNK    3

typedef struct _LAB_WAPX_TYPE
{
	_SHORT x;
	_SHORT y;
} LAB_WAPX_TYPE, _PTR p_LAB_WAPX;

typedef struct _LAB_CHUNK_TYPE
{
	_WORD wType;
	_SHORT iFirst;
	_SHORT iLast;
} LAB_CHUNK_TYPE, _PTR p_LAB_CHUNK;

typedef struct _LAB_LC_TYPE
{
	_ULONG ulSmLet;   //Smal letters's flags.
	_ULONG ulCpLet;   //Capital letters's flags.
} LAB_LC_TYPE, _PTR p_LAB_LC;
typedef struct _LAB_LOWOBJ_TYPE
{
	_WORD wAttr;
	_SHORT iBegTr;
	_SHORT iEndTr;
	_SHORT nIdent;
	LAB_LC_TYPE lc;
} LAB_LOWOBJ_TYPE, _PTR p_LAB_LOWOBJ;

typedef struct _LAB_CHUNK_DATA_TYPE
{
	_BOOL       fIsNumber;
	_BOOL       fLetters;
	_SHORT      nCharsInAnsw;
	_SHORT      nWApxPoints;
	_SHORT      nChNumb;
	_SHORT      nLONumb;
	_CHAR       szAnswer[CH_MAX_ANSWER_SIZE];
	LAB_WAPX_TYPE   pWApx[CH_MAX_WAPXPNT_NUM];
	LAB_CHUNK_TYPE  pCh[CH_MAX_CHUNKS_NUM];
	LAB_LOWOBJ_TYPE pLO[CH_MAX_ANSWER_SIZE];
} LAB_CHUNK_DATA_TYPE, _PTR p_LAB_CHUNK_DATA;

typedef struct _LAB_LETTERPOS_TYPE
{
	_CHAR       oldAnswer[CH_MAX_ANSWER_SIZE];
	_CHAR       insLetters[CH_MAX_ANSWER_SIZE];
} LAB_LETTERPOS_TYPE, _PTR p_LAB_LETTERPOS;
typedef struct _CHUNK_DES_HEAD
{
	_SHORT nLines;
} CHUNK_DES_HEAD_TYPE, _PTR p_CHUNK_DES_HEAD_TYPE;

typedef struct _CHUNK_DES_LINE
{
	_SHORT   fIsNumb; //The preventive number's classifying flag.
	_SHORT   nSymb;   //Total symbols number in the line.
	_SHORT   nLO;
	_CHAR    szAnswer[CH_MAX_ANSWER_SIZE];
	_USHORT  rgAttr[CH_MAX_ANSWER_SIZE];
	_SHORT   rgIdent[CH_MAX_ANSWER_SIZE];
} CHUNK_DES_LINE_TYPE, _PTR p_CHUNK_DES_LINE_TYPE;

typedef struct
{
	_INT DxRatioForGU;
	_INT DyRatioForGU;
	_INT DxRatioForO;
	_INT DyRatio1ForO;
	_INT DyRatio2ForO;
	_INT Ratio1PointsInCrossToBeO;
	_INT Ratio2PointsInCrossToBeO;
	_INT DltGammToBeO;
} LOW_DBG_INFO_TYPE;

/* =============================================================== */
/* =============================================================== */
/* =============================================================== */

#endif /* _WG_STUFF_INCLUDE_ */
