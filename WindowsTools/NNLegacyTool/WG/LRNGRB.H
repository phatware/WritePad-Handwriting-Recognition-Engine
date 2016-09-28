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

#include "hwr_sys.h"

#define CHARS_MAY_BE_ADDED  5                    /* Sp. for new xr def ch      */
#define DAT_RESERVE        500                   /* Sp. for new xr def ch      */
#define SPACE_VOC_RESERVE  300                   /* Sp. for voc add words      */

#define MXCORR_ISIZE       64                    /* Size of XrCorr matr. for input */
#define MXCORR_VSIZE       80                    /* Size of XrCorr matr. for voc.  */
#define MXCORR_VHSIZE (MXCORR_VSIZE/2)           /* Half of full size XrCorr matr for voc */
#define MHCORR_ISIZE       32                    /* Size of Hcorr matr.  for input */
#define MHCORR_VSIZE       32                    /* Size of Hcorr matr.  for voc.  */
#define MHCORR_VHSIZE (MHCORR_VSIZE/2)           /* Half of full size Hcorr matr for voc */

#define MAXVARSPERLET      16                    /* How many vars each letter has */
#define MAXPENALTY          9                    /* Max possible XR penalty */

#define DTEVEXBUFSIZE     106                    /* How many letters buffered  */
#define DTEFIRSTSYM        32                    /* First definable DTE sym    */

/* ----------- Batch Learning -------------------------------------------- */

#define XRWL_MAXSYMS     12
#define XRWL_LRNSUFFLEV  50

typedef struct
{
	xrdata_type   _PTR xrd;
	p_UCHAR        word;
	void * trj; //PS_point_type _PTR trj;
} xrwlearn_type;

typedef struct
{
	_SHORT        class_level;
	_SHORT        min_class_size;
	_SHORT        learn_suff;
	_SHORT        vocvar_reward;
} xrwlp_type;

typedef struct
{
	_WORD         num  : 4;       /* Number of class        */
	_WORD         del  : 4;       /* Reason of deletion     */
	_WORD         best : 1;       /* Best variant of class  */
	_WORD         lib  : 1;       /* Copied from default lib*/
} xlclass_type;

typedef struct
{
	_USHORT       next;           /* Short pointer to next  */
	_USHORT       num;            /* Num of w., where it was found or var_xr num*/
	xlclass_type  xlclass;        /* Class num and flags    */
	_UCHAR        xrd_beg;        /* Beg num of xrdata      */
	_UCHAR        xrd_len;        /* Len of corresp str     */
	_SHORT        maxw;           /* Max corr on ally voc v */
	_SHORT        nvars;          /* Num vars got or belong */
	_UCHAR        var_track;      /* Bit map of ally voc v  */
	_UCHAR        syms[XRWL_MAXSYMS];/* Alien syms          */
} lv_descr_type;

typedef struct
{
	_SHORT        pc_ally;        /* Perc. of ally inp v. got  */
	_SHORT        pc_unique;      /* % of uniquely got ally v. */
	_SHORT        pc_alien;       /* Sum % of got aliens       */
} libv_info_type;

typedef _SHORT  let_table_type[256];
typedef _ULONG  triad_type[32][32];
typedef _UCHAR  mxcorr_type[MXCORR_ISIZE][MXCORR_VHSIZE];
typedef _UCHAR  mhcorr_type[MHCORR_ISIZE][MHCORR_VHSIZE];
//typedef _SHORT  dtevex_type[DTEVEXBUFSIZE][MAXVARSPERLET];
/* TEMP */ typedef _UCHAR  dtevex_type[DTEVEXBUFSIZE][8];/* For ARM mem save */

typedef struct
{
	_HMEM         hdat;
	_HMEM         hmem;
	_CHAR         datname[80];
	p_UCHAR       mem;
	_USHORT       otriads;
	_USHORT       omcorr;
	_USHORT       oletxrv;
	p_VOID        triads;
	p_VOID        mcorr;
	p_VOID        letxrv;
	_SHORT        letxrv_len;
	_SHORT        letxrv_mem;
	_SHORT        fixed_block_size;
	_HMEM            hvexbuf;
	dtevex_type _PTR vexbuf;
} datptr_type;

typedef struct
{
//                long          type_id;
//                long          ver_id;
	long          length;
	long          checksum;
	long          s_triad[32][32];
	long          c_triad[32][32];
	_UCHAR        xrcorr[MXCORR_ISIZE][MXCORR_VHSIZE];
	_UCHAR        hcorr[MHCORR_ISIZE][MHCORR_VHSIZE];
	_SHORT        ichxr[256];
} dat_file_type;

typedef struct                              /* Descriptor of xr repr. of letter */
{
	_UCHAR         min_var_len; /* Shortest var (excluding tails, etc*/
	_UCHAR         num_of_vars; /* Number of xr variants for the let*/
	_UCHAR         padding1;
	_UCHAR         padding2;
	_UCHAR         var_lens[MAXVARSPERLET]; /* Lengths of vars                  */
	_SHORT         var_veis[MAXVARSPERLET]; /* Extra info for variants          */
} let_descr_type;


