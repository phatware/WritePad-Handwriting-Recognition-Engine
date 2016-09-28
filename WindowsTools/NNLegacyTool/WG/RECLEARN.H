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

#include <bastypes.h>

#define XRWL_MAXSYMS     12
#define XRWL_LRNSUFFLEV  50


typedef struct
{
	_SHORT          x;
	_SHORT          y;
} PS_point_type;

typedef struct
{
	_UCHAR          xr;            /* XrElem definition     */
	_UCHAR          a;
	_UCHAR          p;
	_UCHAR          h;
} xrvoc_type;

typedef struct
{
	xrvoc_type      xr;
	_SHORT          begpoint;
	_SHORT          endpoint;
	_SHORT          box_left;
	_SHORT          box_up;
	_SHORT          box_right;
	_SHORT          box_down;
} xrdata_type;

typedef struct
{
	xrdata_type   _PTR xrd;
	p_UCHAR        word;
	PS_point_type _PTR trj;
} xrwlearn_type;

typedef struct
{
	_SHORT        class_level;
	_SHORT        min_class_size;
	_SHORT        learn_suff;
	_SHORT        vocvar_reward;
} xrwlp_type;

#define MAXVARSPERLET      16                    /* How many vars each letter has */
#define DTEVEXBUFSIZE     106                    /* How many letters buffered  */
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

typedef _SHORT  let_table_type[256];

typedef struct
{
	_WORD         num : 4;       /* Number of class        */
	_WORD         del : 4;       /* Reason of deletion     */
	_WORD         best : 1;       /* Best variant of class  */
	_WORD         lib : 1;       /* Copied from default lib*/
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


typedef struct                              /* Descriptor of xr repr. of letter */
{
	_UCHAR         min_var_len; /* Shortest var (excluding tails, etc*/
	_UCHAR         num_of_vars; /* Number of xr variants for the let*/
	_UCHAR         padding1;
	_UCHAR         padding2;
	_UCHAR         var_lens[MAXVARSPERLET]; /* Lengths of vars                  */
	_SHORT         var_veis[MAXVARSPERLET]; /* Extra info for variants          */
} let_descr_type;
