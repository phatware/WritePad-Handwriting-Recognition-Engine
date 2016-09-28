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

#pragma hdrstop

#include "hwr_sys.h"

#include "ams_mg.h"

#if !VER_PALK_DICT

#include "elk.h"
#include "xrwdict.h"

// --------------- Defines -----------------------------------------------------
#define ELK_ID_LEN         16
#define ELK_ID_STRING      "ELK dict v.2.10."
#define ELK_VER_ID         (('2' << 0) | ('.' << 8) | ('1' << 16) | ('0' << 24))

#define ELK_ID_STRING_PREV "ELK dict v.2.02."
#define ELK_VER_ID_PREV    (('2' << 0) | ('.' << 8) | ('0' << 16) | ('2' << 24))

#ifdef  PG_DEBUG_ON
#define ELK_DEBUG          1
#else
#define ELK_DEBUG          0
#endif

#ifndef ELK_FILEOP
#ifdef  PEGASUS
#define ELK_FILEOP        0
#define ELK_SUPPORTFUNCS  0
#else
#define ELK_FILEOP        1
#define ELK_SUPPORTFUNCS  1
#endif
#endif


#ifndef ELK_SUPPORTFUNCS
#define ELK_SUPPORTFUNCS  1
#endif
#ifndef ELK_OPTIMIZEDICT
#define ELK_OPTIMIZEDICT   0
#endif

// ------ Private  Defines -----------------------------------------------------

#define ELK_MEMBLOK_SIZE     4096
#define ELK_MAXSHIFT        130000    // Max shift for 2 byte ELK_SHFT_SIZE plus one flag bit
#define ELK_MAX_SYMINNODE  (256-32)   // Max num of symbol in one interim node
#define CTBL_INDEX_SIZE    (256/8)    // Size of codebook index

#define ELK_FLAG_SIZE         1       // Size of flag field
#define ELK_SNUM_SIZE         1       // Size of num sym field
#define ELK_SYMB_SIZE         1       // Size of sym field
#define ELK_STDNODE_SIZE     (ELK_FLAG_SIZE+ELK_SNUM_SIZE)

#define ELK_INDEX_SIZE        5
#define ELK_NNODES_PER_I_SH   6
#define ELK_NNODES_PER_INDEX (0x01 << ELK_NNODES_PER_I_SH)
#define ELK_MAX_WORDLEN2     (ELK_MAX_WORDLEN+2)

#define ELK_FL_STDND     0x80    // This is full size standard node
#define ELK_FL_STDND_N   0x81    // This is full size standard node created anew
#define ELK_FL_STDND_NW  0x82    // This is full size standard node created anew from WBLK
#define ELK_FL_STDND_N2  0x83    // Request to make std node with two symbols
#define ELK_FL_ENDWBLK_2 0x84    // Request to create two WBLK nodes

#define ELK_FL_NOP       0x00    // Not valid node type
#define ELK_FL_ENDWORD   0x80    // This was word end with branches further

#define ELK_FL_STDBLK    0x10    // This was middle ring
#define ELK_FL_STDSBLK   0x20    // This was middle ring with single letter
#define ELK_FL_ENDWBLK   0x30    // This is just attribute for subset word
#define ELK_FL_ENDBL_NA  0x40    // This was last-last letter of the branch
#define ELK_FL_ENDBL_A   0x50    // This was last-last letter of the branch has its own  attr
#define ELK_FL_ENDBL_MNA 0x60    // This was last-last multi-letter
#define ELK_FL_ENDBL_MA  0x70    // This was last-last multi-letter
#define ELK_FL_STSMASK   0xF0    // Mask for selecting flags
#define ELK_FL_STBMASK   0x70    // Mask for selecting block flags
#define ELK_SHIFT_MSB    0x08    // Most significant bit of shift
#define ELK_ATTR_MASK    0x07    // LS 4 bits are for word attribute

//------------------- ELK header type field inhabitants ------------------------

#define ELK_FL_CHANGED 0x10000000// Flag of dictionary modification
#define ELK_TYPE_MASK    0xFF    // One byte is reserved for the ELK file type

// --------------- Calc Macros -------------------------------------------------

#if ELK_DEBUG
#define ELK_ASSERT(cond, num) {if (cond) throw(num);}
#define ELK_ASSRT(num) {throw(num);}
#else
#define ELK_ASSERT(cond, num) {if (cond) goto err;}
#define ELK_ASSRT(num) {goto err;}
#endif

//#define ELK_NSHIFT(node) ((_INT)node[2] + ((_INT)node[3] << 8) + ((node[0] & ELK_SHIFT_MSB) ? 0x10000 : 0))
//#define ELK_PSHIFT(node, shift) {ELK_ASSERT((shift) > ELK_MAXSHIFT, 0x0002); node[2] = (_UCHAR)((shift) & 0xFF); node[3] = (_UCHAR)((shift) >> 8); node[0] &= ~(ELK_SHIFT_MSB); if ((_INT)(shift) & 0x10000) node[0] |= ELK_SHIFT_MSB;}

#define ELK_IS_ENDWORD(node)   ((node[0] & ELK_FL_ENDWORD))
#define ELK_IS_ENDBLOK(node)   ((node[0] & ELK_FL_STBMASK) > ELK_FL_STDSBLK)
#define ELK_IS_STDBLOK(node)   ((node[0] & ELK_FL_STBMASK) <= ELK_FL_STDSBLK)

#define ELK_IS_STDSBLK(node)   ((node[0] & ELK_FL_STBMASK) == ELK_FL_STDSBLK)
#define ELK_IS_ENDWBLK(node)   ((node[0] & ELK_FL_STBMASK) == ELK_FL_ENDWBLK)
#define ELK_IS_ENDBL_NA(node)  ((node[0] & ELK_FL_STBMASK) == ELK_FL_ENDBL_NA)
#define ELK_IS_ENDBL_A(node)   ((node[0] & ELK_FL_STBMASK) == ELK_FL_ENDBL_A)
#define ELK_IS_ENDBL_MNA(node) ((node[0] & ELK_FL_STBMASK) == ELK_FL_ENDBL_MNA)
#define ELK_IS_ENDBL_MA(node)  ((node[0] & ELK_FL_STBMASK) == ELK_FL_ENDBL_MA)
#define ELK_IS_ENDBL_MM(node)  (ELK_IS_ENDBL_MNA(node) | ELK_IS_ENDBL_MA(node))
#define ELK_IS_ENDBL_AA(node)  (ELK_IS_ENDBL_A(node) | ELK_IS_ENDBL_MA(node))
#define ELK_IS_ENDBL_SE(node)  (ELK_IS_ENDBL_A(node) | ELK_IS_ENDBL_NA(node))

#define ELK_SET_FLAGS(where, what) {where &= (_UCHAR)(~(ELK_FL_STSMASK)); where |= (_UCHAR)(what);}

#define ELK_ST_GETLAYER(state) (((state) >> 24) & 0xFF)
#define ELK_ST_GETLNUM(state) (((state) >> 17) & 0x7F)
#define ELK_ST_GETNNUM(state) ((state) & 0x1FFFF)
#define ELK_FORMSTATE(layer, lnum, nnum) (((layer) << 24) + ((lnum) << 17) + (nnum))
#define ELK_ST_ENDBL_M(state) (ELK_ST_GETLNUM(state) != 0)

#define ELK_SET_ISHIFT(p, v) {(p)[0] = (_UCHAR)(v); (p)[1] = (_UCHAR)((v) >> 8); (p)[2] = (_UCHAR)((v) >> 16);}
#define ELK_GET_ISHIFT(p)    ((p)[0] + ((p)[1] << 8) + ((p)[2] << 16))
#define ELK_SET_INNODES(p, v) {(p)[3] = (_UCHAR)(v); (p)[4] = (_UCHAR)((v) >> 8);}
#define ELK_GET_INNODES(p)    ((p)[3] + ((p)[4] << 8))

#define ELK_GETNEXTND(node) {                                                \
							 switch (node[0] & ELK_FL_STBMASK)               \
															{                                              \
							   case ELK_FL_STDBLK:    node += node[1] + ELK_STDNODE_SIZE;    break; \
							   case ELK_FL_STDSBLK:   node += ELK_FLAG_SIZE + ELK_SYMB_SIZE; break; \
							   case ELK_FL_ENDWBLK:   node += ELK_FLAG_SIZE;                 break; \
							   case ELK_FL_ENDBL_A:   node += ELK_FLAG_SIZE + ELK_SYMB_SIZE + ELK_FLAG_SIZE; break; \
							   case ELK_FL_ENDBL_NA:  node += ELK_FLAG_SIZE + ELK_SYMB_SIZE; break; \
							   case ELK_FL_ENDBL_MA:  node += node[1]+ELK_SNUM_SIZE+ELK_FLAG_SIZE + ELK_FLAG_SIZE; break; \
							   case ELK_FL_ENDBL_MNA: node += node[1]+ELK_SNUM_SIZE+ELK_FLAG_SIZE; break; \
							   default: ELK_ASSRT(0x0009);                   \
															}                                              \
														}

#define ELK_GET_NNSYM(node, n) {                                             \
							 switch (node[0] & ELK_FL_STBMASK)               \
															{                                              \
							   case ELK_FL_ENDWBLK:   n = 0;          break; \
							   case ELK_FL_STDSBLK:                          \
							   case ELK_FL_ENDBL_A:                          \
							   case ELK_FL_ENDBL_NA:  n = 1;          break; \
							   case ELK_FL_STDBLK:                           \
							   case ELK_FL_ENDBL_MA:                         \
							   case ELK_FL_ENDBL_MNA: n = node[1];    break; \
							   default: ELK_ASSRT(0x000A);                   \
															}                                              \
														}

#define ELK_GET_STBLKSYM(node, n) ((ELK_IS_STDSBLK(node)) ? node[1] : node[ELK_STDNODE_SIZE+n])

// --------------- Types -------------------------------------------------------

typedef struct
{
	_ULONG        ver;
	_ULONG        type;
	_LONG         elk_size;
	_LONG         ctbl_size;
	_LONG         alloc_size;
	_ULONG        layers[ELK_MAX_WORDLEN2];
	_USHORT       l_nodes[ELK_MAX_WORDLEN2];
	_UCHAR        elk;
} elk_header_type, _PTR p_elk_header_type;

// --------------- Prototypes --------------------------------------------------

_INT    ElkVerifyPrefix(p_UCHAR word, p_INT wp, p_INT sl, p_INT bp, p_UCHAR _PTR ppbase, p_UCHAR _PTR ppnext, p_elk_header_type elk);
_INT    ElkGetNodePtr(_ULONG state, p_elk_header_type elk, p_UCHAR _PTR block, p_INT nn);
_ULONG  ElkUpdateLayerIndex(_INT sl, p_elk_header_type elk, p_UCHAR cur_node);
_INT    ElkGetPrefixState(p_UCHAR word, p_ULONG state, p_UCHAR _PTR pnode, p_ULONG pns, p_elk_header_type elk);
_INT    ElkFollowBranch(_INT depth, p_UCHAR word, p_INT height, p_INT counter, _INT fbn, p_fw_buf_type fwb, p_VOID pd);
_INT    ElkOptimizeWord(p_UCHAR inpw, p_VOID pd);
_INT    ElkOptStdBlocks(p_elk_header_type elk);
_USHORT ElkDecodeSym(p_UCHAR psym, p_VOID pd);
_INT    ElkCheckNextLet(p_INT ans, _INT dep, p_UCHAR word, _INT cur, _INT free, _INT max, p_fw_buf_type buf, p_VOID pd);

// ----------------------- Redirect HWR ----------------------------------------
#if ELK_FILEOP
#include <stdlib.h>
#include <string.h>
#define HWRMemoryAlloc(x)   malloc(x)
#define HWRMemoryFree(x)    free(x)
#ifndef HWRMemSet
#define HWRMemSet(p, v, l)  memset(p, v, l)
#endif
#define HWRMemCpy(d, s, l)  memmove(d, s, l)
#define HWRStrLen(p)        strlen(p)
#define HWRStrCmp(p, v)     strcmp(p, v)
#define HWRStrnCmp(p, v, l) strncmp(p, v, l)
#define HWRStrCpy(d, s)     strcpy(d, s)
#endif /* ELK_FILEOP */

/* *************************************************************************** */
/* *       Get list of possible continuations                                * */
/* *************************************************************************** */
_INT ElkGetNextSyms(p_VOID cur_fw, p_VOID fwb, p_VOID pd)
{
	_INT    i, nn, cl;
	_INT    nsym, bp;
	_INT    s;
	_ULONG  state;
	_INT    codeshift;
	p_elk_header_type elk = (p_elk_header_type) pd;
	p_UCHAR cur_block, next_block;
	p_fw_buf_type psl = (p_fw_buf_type) fwb;
	p_fw_buf_type cfw = (p_fw_buf_type) cur_fw;

	if (cfw == 0)
	{
		state = 0;
		codeshift = 0;
	}
	else
	{
		state = cfw->state;
		codeshift = cfw->codeshift;
	}

	if (codeshift && *((p_UCHAR) elk + elk->elk_size + codeshift)) // On continuation of compressed sequence
	{
		psl[0] = *cfw;
		psl[0].sym = *((p_UCHAR) elk + elk->elk_size + codeshift);
		psl[0].codeshift++;
		if (*((p_UCHAR) elk + elk->elk_size + codeshift + 1) == 0)
		{
			psl[0].l_status = psl[0].cdb_l_status;
		}
		nsym = 1;
	}
	else
	{
		cl = ELK_ST_GETLAYER(state);
		nn = ELK_ST_GETLNUM(state);
		ElkGetNodePtr(state, elk, &cur_block, &bp);
		ELK_GET_NNSYM(cur_block, nsym);

		if (ELK_IS_ENDBLOK(cur_block)) // ending block, take special care ...
		{
			if (nsym == 0)
			{
				goto done;    // Empty dict -- no symbols there at all!
			}

			if (ELK_IS_ENDBL_MM(cur_block))
			{
				s = (nn == nsym - 1) ? XRWD_BLOCKEND : XRWD_MIDWORD;
			}
			else
			{
				s = XRWD_BLOCKEND;
			}

			psl[0].sym = (ELK_IS_ENDBL_MM(cur_block)) ? cur_block[nn + ELK_FLAG_SIZE + ELK_SNUM_SIZE] : cur_block[1];
			psl[0].attribute = (_UCHAR) ((ELK_IS_ENDBL_AA(cur_block)) ? ((ELK_IS_ENDBL_MM(cur_block)) ? cur_block[cur_block[1] + ELK_FLAG_SIZE + ELK_SNUM_SIZE] : cur_block[2]) : 0);
			psl[0].chain_num = 0;
			psl[0].penalty = 0;
			psl[0].state = ELK_FORMSTATE(cl, nn + 1, ELK_ST_GETNNUM(state));

			if ((psl[0].codeshift = ElkDecodeSym(&psl[0].sym, pd)) == 0) // Not a macro
			{
				psl[0].l_status = psl[0].cdb_l_status = (_UCHAR) s;
			}
			else
			{
				psl[0].l_status = XRWD_MIDWORD;
				psl[0].cdb_l_status = (_UCHAR) s;
			}

			nsym = 1;
		}
		else
		{
			ElkGetNodePtr(ELK_FORMSTATE(cl + 1, 0, bp), elk, &next_block, &i);
			for (i = 0; i < nsym; i++)
			{
				s = XRWD_MIDWORD;
				if (ELK_IS_ENDWORD(next_block))
				{
					s = XRWD_WORDEND;
				}
				if (ELK_IS_ENDWBLK(next_block))
				{
					s = XRWD_BLOCKEND;
				}

				psl[i].sym = (ELK_IS_STDSBLK(cur_block)) ? cur_block[ELK_FLAG_SIZE] : cur_block[ELK_STDNODE_SIZE + i];
				//        psl[i].l_status  = (_UCHAR)s;
				psl[i].attribute = (_UCHAR) (next_block[0] & ELK_ATTR_MASK);
				psl[i].chain_num = 0;
				psl[i].penalty = 0;
				psl[i].state = ELK_FORMSTATE(cl + 1, 0, bp + i);
				//        psl[i].codeshift = ElkDecodeSym(&psl[i].sym, pd);

				if ((psl[i].codeshift = ElkDecodeSym(&psl[i].sym, pd)) == 0) // Not a macro
				{
					psl[i].l_status = psl[i].cdb_l_status = (_UCHAR) s;
				}
				else
				{
					psl[i].l_status = XRWD_MIDWORD;
					psl[i].cdb_l_status = (_UCHAR) s;
				}

				ELK_GETNEXTND(next_block);
			}
		}
	} // End of Codeshift check

done:
	return nsym;
}

/* *************************************************************************** */
/* *       Check word if present and get attribute                           * */
/* *************************************************************************** */
_INT ElkCheckWord(p_UCHAR word, p_UCHAR status, p_UCHAR attr, p_VOID pd)
{
	_INT i, e = 0, ans = 0;
	p_fw_buf_type  pfwb;
#define DEF_BLOCK_SIZE 512

	for (i = 1, ans = -1; i < 4 && ans < 0; i++)
	{
		pfwb = (p_fw_buf_type) HWRMemoryAlloc(sizeof(fw_buf_type)*DEF_BLOCK_SIZE*i);
		if (pfwb == 0)
		{
			goto err;
		}
		HWRMemSet(pfwb, 0, sizeof(fw_buf_type));

		ans = 0;
		e = ElkCheckNextLet(&ans, 0, word, 0, 1, DEF_BLOCK_SIZE*i, pfwb, pd);

		if (e == 2) // Successfully parsed the word
		{
			*status = pfwb[ans].l_status;
			*attr = pfwb[ans].attribute;
		}

		HWRMemoryFree(pfwb);
	}

	if (e != 2)
	{
		goto err;
	}

	return ELK_NOERR;
err:
	*status = XRWD_NOP;
	*attr = 0;
	return ELK_ERR;
}

/* *************************************************************************** */
/* *       SubFunc of ElkCheckWord -- recursively check word                 * */
/* *************************************************************************** */
_INT ElkCheckNextLet(p_INT ans, _INT dep, p_UCHAR word, _INT cur, _INT free, _INT max, p_fw_buf_type buf, p_VOID pd)
{
	_INT i;
	_INT nsym, e = 0;

	if (dep >= ELK_MAX_WORDLEN || word[dep] == 0)
	{
		goto done;
	}
	if (max - free < 256)
	{
		goto err;
	}
	if (buf[cur].l_status == XRWD_BLOCKEND)
	{
		goto done;
	}

	if ((nsym = ElkGetNextSyms((p_VOID) &buf[cur], (p_VOID) (&buf[free]), pd)) == 0)
	{
		goto done;
	}

	for (i = 0; i < nsym; i++)
	{
		if (buf[free + i].sym == word[dep]) // Found corresponding letter
		{
			if (word[dep + 1] == 0)
			{
				if (buf[free + i].l_status > XRWD_MIDWORD)
				{
					*ans = free + i;    // Word found!!!
					e = 2;
					break;
				}
			}
			else
			{
				if ((e = ElkCheckNextLet(ans, dep + 1, word, free + i, free + nsym, max, buf, pd)) != 0)
				{
					break;
				}
			}
		}
	}

done:
	return e;
err:
	return 1;
}

/* *************************************************************************** */
/* *       Get pointer to node and its next number from state                * */
/* *************************************************************************** */
_INT  ElkGetNodePtr(_ULONG state, p_elk_header_type elk, p_UCHAR _PTR block, p_INT nn)
{
	_INT    i;
	_INT    sl, nnum, lnum, shift, index, rnum;
	p_UCHAR ptr, pind;

	sl = ELK_ST_GETLAYER(state);
	nnum = ELK_ST_GETNNUM(state);
	lnum = ELK_ST_GETLNUM(state);

	ELK_ASSERT(nnum > elk->l_nodes[sl], 0x0002);

	ptr = (p_UCHAR) elk + elk->layers[sl];
	index = nnum >> ELK_NNODES_PER_I_SH;
	pind = ptr + index*ELK_INDEX_SIZE;
	shift = ELK_GET_ISHIFT(pind);
	lnum += ELK_GET_INNODES(pind);
	rnum = nnum & (ELK_NNODES_PER_INDEX - 1);
	ptr += shift;

	for (i = 0; i < rnum; i++)
	{
		if (ELK_IS_STDBLOK(ptr))
		{
			lnum += (ELK_IS_STDSBLK(ptr)) ? 1 : ptr[1];
		}
		ELK_GETNEXTND(ptr);
	}

	*block = ptr;
	*nn = lnum;

	return ELK_NOERR;
}

/* *************************************************************************** */
/* *       Verify prefix and if exists get state of its end                  * */
/* *************************************************************************** */
_INT   ElkGetPrefixState(p_UCHAR word, p_ULONG state, p_UCHAR _PTR pnode, p_ULONG pns, p_elk_header_type elk)
{
	_INT    i, n, pos, nn;
	_INT    bp, lnum;
	_ULONG  cur_state;
	p_UCHAR cur_block;


	bp = 0;
	lnum = 0;
	cur_state = 0;
	ElkGetNodePtr(cur_state, elk, &cur_block, &lnum);

	for (i = pos = nn = 0; word[i] != 0 && i < ELK_MAX_WORDLEN; i++)
	{
		if (ELK_IS_STDBLOK(cur_block))
		{
			ELK_GET_NNSYM(cur_block, n);
			for (bp = 0; bp < n; bp++)
				if (word[i] == ELK_GET_STBLKSYM(cur_block, bp))
				{
					break;    // Found letter in this transit block
				}
			if (bp >= n)
			{
				goto err;    // No such letter!
			}
			if (word[i + 1] == 0)
			{
				break;
			}

			pos++;

			cur_state = ELK_FORMSTATE(pos, nn, lnum + bp);
			ElkGetNodePtr(cur_state, elk, &cur_block, &lnum);
		}
		else
		{
			if (ELK_IS_ENDBL_MM(cur_block))
			{
				if (nn >= cur_block[1])
				{
					goto err;
				}
				if (word[i] != cur_block[ELK_STDNODE_SIZE + nn])
				{
					goto err;
				}
				nn++;
			}
			else
			{
				if (ELK_IS_ENDWBLK(cur_block))
				{
					goto err;    // No cur letter in branch -- it has eneded!
				}
				else
					if (word[i] == cur_block[ELK_FLAG_SIZE] && word[i + 1] == 0)
					{
						break;
					}
					else
					{
						goto err;
					}
			}

			//      if (pos == wlen-1) break; // Found end block, and it was last letter
			//       else goto err;
		}
	}

	*state = cur_state;
	*pns = ELK_FORMSTATE(pos + 1, nn, lnum + bp);
	*pnode = cur_block;

	return ELK_NOERR;
err:
	return ELK_ERR;
}

/* *************************************************************************** */
/* *       Decode symbol to real symbol and code book shift                  * */
/* *************************************************************************** */
_USHORT ElkDecodeSym(p_UCHAR psym, p_VOID pd)
{
	_INT i;
	_INT shift = 0;
	_UCHAR sym = *psym;
	p_elk_header_type elk = (p_elk_header_type) pd;
	p_UCHAR ptr = (p_UCHAR) elk + elk->elk_size;

	if (elk->ctbl_size && (ptr[sym >> 3] & (0x01 << (sym & 0x07)))) // Needs decoding
	{
		for (i = CTBL_INDEX_SIZE; i < elk->ctbl_size; i++) if (ptr[i] == sym)
			{
				break;
			}
		*psym = ptr[i + 1];
		shift = i + 2;
	}

	return (_USHORT) shift;
}

#ifdef ELK_SUPPORTFUNCS
/* *************************************************************************** */
/* *       Add word to dict                                                  * */
/* *************************************************************************** */
_INT ElkAddWord(p_UCHAR word, _UCHAR attr, p_VOID _PTR pd)
{
	_INT    i, len, wlen, pos, bp, last;
	_INT    fsp1, fsp2, first, wpresent, lnum;
	_INT    cur_node_type, next_node_type;
	_INT    cur_attr, next_attr, nxpr_attr;
	_INT    added = 0;
	_ULONG  cur_state;
	p_elk_header_type elk = (p_elk_header_type) *pd;
	p_elk_header_type telk;
	p_UCHAR cur_block, next_block;
	p_UCHAR ptr;


	ELK_ASSERT((elk->type != ELK_STND_TYPE), 0x000B);

	// --------------- Do we need more memory ? ------------------------------------

	if (elk->alloc_size - (elk->elk_size + (_LONG)sizeof(elk_header_type)) < ELK_MAX_WORDLEN2 * 4)
	{
		if ((telk = (p_elk_header_type) HWRMemoryAlloc(elk->alloc_size + ELK_MEMBLOK_SIZE)) == _NULL)
		{
			goto err;
		}
		HWRMemCpy(telk, elk, elk->alloc_size);
		HWRMemoryFree(elk);
		elk = telk;
		elk->alloc_size += ELK_MEMBLOK_SIZE;
		*pd = elk;
	}

	// ----------------- Wire in new word ------------------------------------------

	first = 1;
	cur_state = 0;
	wpresent = 1;
	wlen = HWRStrLen((_STR) word);

	if (wlen > ELK_MAX_WORDLEN)
	{
		goto done;
	}

	//  if (HWRStrCmp((_STR)word, "already") == 0)
	//    cur_block = 0;

	//{
	//static cnt = 0;
	//if (cnt++ % 10 == 9)
	// cur_block = 0;
	//}

	for (pos = 0; pos < wlen; pos++)  // Step by step encroach word in dict
	{
		last = (word[pos + 1] == 0) ? 1 : 0;
		bp = 0;

		// ------------- Let's see if we could find cur letter in cur node ---------

		ElkGetNodePtr(cur_state, elk, &cur_block, &lnum);

		if (ELK_IS_STDBLOK(cur_block))
		{
			for (bp = 0; bp < cur_block[1] && word[pos] > cur_block[bp + 2]; bp++);
			if (bp < cur_block[1] && word[pos] == cur_block[bp + ELK_STDNODE_SIZE])  // Found letter in this transit block
			{
				cur_state = ELK_FORMSTATE(pos + 1, 0, lnum + bp);
				continue;
			}
		}
		else
		{
			if (last && ELK_IS_ENDBL_SE(cur_block) && cur_block[1] == word[pos])
			{
				wpresent = 2; // No second attempt to end it
				break;  // Matching ending block
			}
		}

		ElkGetNodePtr(ELK_FORMSTATE(pos + 1, 0, lnum + bp), elk, &next_block, &lnum);

		// ---------------- Calculate space needed on current and next layers ------

		switch (cur_block[0] & ELK_FL_STBMASK)
		{
			case ELK_FL_STDBLK: // If STD, insert in it one letter, and add WBLK on next layer
			{
				fsp1 = ELK_SYMB_SIZE;              // One symbol is to be inserted in cur block
				fsp2 = ELK_FLAG_SIZE;              // WBLK blok will be created next
				cur_node_type = ELK_FL_STDND;
				next_node_type = ELK_FL_ENDWBLK;
				next_attr = (last) ? (_UCHAR) ((attr & ELK_ATTR_MASK) | ELK_FL_ENDWORD) : 0;
				bp += ELK_STDNODE_SIZE;
				//        first = 0;
				break;
			}

			case ELK_FL_ENDWBLK:
			{
				if (last)    // If converting last word letter to ENDBL node
				{
					fsp1 = (attr) ? ELK_SYMB_SIZE + ELK_FLAG_SIZE : ELK_SYMB_SIZE;
					fsp2 = 0;
					cur_node_type = (attr) ? ELK_FL_ENDBL_A : ELK_FL_ENDBL_NA;
					next_node_type = ELK_FL_NOP;
					//          next_attr      = (_UCHAR)(((first) ? ELK_FL_ENDWORD : 0) | (cur_block[0] & ELK_ATTR_MASK));
					cur_attr = (_UCHAR) (cur_block[0] & (ELK_ATTR_MASK | ELK_FL_ENDWORD));
					bp += ELK_FLAG_SIZE;
				}
				else        // Continuing current word  with ENDWBLK node
				{
					fsp1 = ELK_SNUM_SIZE + ELK_SYMB_SIZE;
					fsp2 = ELK_FLAG_SIZE;
					cur_node_type = ELK_FL_STDND_NW;
					next_node_type = ELK_FL_ENDWBLK;
					//          next_attr      = (_UCHAR)(((first) ? ELK_FL_ENDWORD : 0) | (cur_block[0] & ELK_ATTR_MASK));
					next_attr = 0;
					cur_attr = (_UCHAR) (cur_block[0] & (ELK_ATTR_MASK | ELK_FL_ENDWORD));
					bp += ELK_FLAG_SIZE;
				}
				break;
			}

			case ELK_FL_ENDBL_NA:
			{
				if (word[pos] != cur_block[1]) // Need to create std node with two symbols, and next two WBLK nodes for them
				{
					fsp1 = (ELK_STDNODE_SIZE + ELK_SYMB_SIZE + ELK_SYMB_SIZE) - (ELK_FLAG_SIZE + ELK_SYMB_SIZE);
					fsp2 = ELK_FLAG_SIZE + ELK_FLAG_SIZE;
					cur_node_type = ELK_FL_STDND_N2;
					next_node_type = ELK_FL_ENDWBLK_2;
					//          next_attr      = 0;
					next_attr = (last) ? (_UCHAR) ((attr & ELK_ATTR_MASK) | ELK_FL_ENDWORD) : 0;
					nxpr_attr = ELK_FL_ENDWORD;
					bp += ELK_FLAG_SIZE + ELK_SYMB_SIZE;
				}
				else      // Continuing current word
				{
					fsp1 = (ELK_STDNODE_SIZE + ELK_SYMB_SIZE) - (ELK_FLAG_SIZE + ELK_SYMB_SIZE);
					fsp2 = ELK_FLAG_SIZE;              // WBLK blok will be created next
					//          fsp2 = (last && attr) ? ELK_FLAG_SIZE + ELK_SYMB_SIZE + ELK_FLAG_SIZE : ELK_FLAG_SIZE + ELK_SYMB_SIZE;
					cur_node_type = ELK_FL_STDND_N;
					next_node_type = ELK_FL_ENDWBLK;
					//          next_node_type = (last && attr) ? ELK_FL_ENDBL_A : ELK_FL_ENDBL_NA;
					next_attr = (last) ? (_UCHAR) ((attr & ELK_ATTR_MASK) | ELK_FL_ENDWORD) : 0;
					next_attr |= (first) ? ELK_FL_ENDWORD : 0;
					bp += ELK_FLAG_SIZE + ELK_SYMB_SIZE;
				}
				//        first = 0;
				break;
			}

			case ELK_FL_ENDBL_A:
			{
				if (word[pos] != cur_block[1])
				{
					fsp1 = (ELK_STDNODE_SIZE + ELK_SYMB_SIZE + ELK_SYMB_SIZE) - (ELK_FLAG_SIZE + ELK_SYMB_SIZE + ELK_FLAG_SIZE);
					fsp2 = ELK_FLAG_SIZE + ELK_FLAG_SIZE;
					cur_node_type = ELK_FL_STDND_N2;
					next_node_type = ELK_FL_ENDWBLK_2;
					next_attr = (last) ? (_UCHAR) ((attr & ELK_ATTR_MASK) | ELK_FL_ENDWORD) : 0;
					nxpr_attr = (cur_block[2] & ELK_ATTR_MASK) | ELK_FL_ENDWORD;
					bp += ELK_FLAG_SIZE + ELK_SYMB_SIZE + ELK_FLAG_SIZE;
				}
				else
				{
					fsp1 = (ELK_STDNODE_SIZE + ELK_SYMB_SIZE) - (ELK_FLAG_SIZE + ELK_SYMB_SIZE + ELK_FLAG_SIZE);
					fsp2 = ELK_FLAG_SIZE;              // WBLK blok will be created next
					//          fsp2 = (last && attr) ? ELK_FLAG_SIZE + ELK_SYMB_SIZE + ELK_FLAG_SIZE : ELK_FLAG_SIZE + ELK_SYMB_SIZE;
					cur_node_type = ELK_FL_STDND_N;
					next_node_type = ELK_FL_ENDWBLK;
					//          next_node_type = (last && attr) ? ELK_FL_ENDBL_A : ELK_FL_ENDBL_NA;
					next_attr = ((first) ? ELK_FL_ENDWORD : 0) | (cur_block[2] & ELK_ATTR_MASK);
					bp += ELK_FLAG_SIZE + ELK_SYMB_SIZE + ELK_FLAG_SIZE; // Shift to the end of node
				}
				//        first = 0;
				break;
			}
		}

		// ---------------- Make room in current layer ----------------------------

		ptr = (p_UCHAR) elk + elk->elk_size;
		len = (_ULONG) ptr - (_ULONG) next_block;
		HWRMemCpy(next_block + fsp2 + fsp1, next_block, len);
		elk->elk_size += fsp2 + fsp1;

		// ---------------- Make room in next layer --------------------------------

		len = (_INT) ((_ULONG) next_block - ((_ULONG) cur_block + bp));
		HWRMemCpy(cur_block + bp + fsp1, cur_block + bp, len);
		next_block += fsp1;  // Now it is place for new node

		// ---------------- Update page index -------------------------------------

		elk->layers[pos + 1] += fsp1;
		for (i = pos + 2; i < ELK_MAX_WORDLEN2; i++)
		{
			elk->layers[i] += fsp2 + fsp1;
		}

		// ---------------- Update parent node -------------------------------------

		switch (cur_node_type)
		{
			case ELK_FL_STDND:  // Current always now like that (when present)
			{
				cur_block[bp] = word[pos];        // And insert there symbol
				cur_block[1] ++;
				break;
			}

			case ELK_FL_STDND_NW: // Create new STD node from WBLK
			{
				cur_block[0] &= (_UCHAR) (~ELK_FL_STBMASK);
				cur_block[0] |= (_UCHAR) (ELK_FL_STDBLK);
				cur_block[1] = 1;
				cur_block[ELK_STDNODE_SIZE] = word[pos];
				break;
			}

			case ELK_FL_STDND_N: // Create new STD node from ending one
			{
				cur_block[0] &= (_UCHAR) (~ELK_FL_STBMASK);
				cur_block[0] |= (_UCHAR) (ELK_FL_STDBLK);
				cur_block[ELK_STDNODE_SIZE] = cur_block[1]; // There was symbol here in ending node
				cur_block[1] = 1;
				break;
			}

			case ELK_FL_STDND_N2: // Create new STD node with two symbols from ending one
			{
				cur_block[0] &= (_UCHAR) (~ELK_FL_STBMASK);
				cur_block[0] |= (_UCHAR) (ELK_FL_STDBLK);
				cur_block[ELK_STDNODE_SIZE] = (word[pos] < cur_block[1]) ? word[pos] : cur_block[1];
				cur_block[ELK_STDNODE_SIZE + 1] = (cur_block[1] < word[pos]) ? word[pos] : cur_block[1];
				cur_block[1] = 2;
				break;
			}

			case ELK_FL_ENDBL_NA:
			{
				cur_block[0] = (_UCHAR) (cur_attr | ELK_FL_ENDBL_NA);
				cur_block[1] = word[pos];
				break;
			}

			case ELK_FL_ENDBL_A:
			{
				cur_block[0] = (_UCHAR) (cur_attr | ELK_FL_ENDBL_A);
				cur_block[1] = word[pos];
				cur_block[2] = (_UCHAR) (attr & ELK_ATTR_MASK);
				break;
			}

			default:
				ELK_ASSRT(0x0003);
		}

		// ---------------- Update next node ---------------------------------------

		switch (next_node_type)
		{
			case ELK_FL_ENDWBLK:
			{
				next_block[0] = (_UCHAR) (next_attr | ELK_FL_ENDWBLK);
				break;
			}

			case ELK_FL_ENDWBLK_2: // Create two WBLK nodes
			{
				if (cur_block[ELK_STDNODE_SIZE] == word[pos])
				{
					next_block[0] = (_UCHAR) (next_attr | ELK_FL_ENDWBLK);
					next_block[1] = (_UCHAR) (nxpr_attr | ELK_FL_ENDWBLK);
				}
				else
				{
					next_block[1] = (_UCHAR) (next_attr | ELK_FL_ENDWBLK);
					next_block[0] = (_UCHAR) (nxpr_attr | ELK_FL_ENDWBLK);
					next_block++;  // Next block is second of created !
				}
				break;
			}

			case ELK_FL_ENDBL_NA:
			{
				next_block[0] = (_UCHAR) (next_attr | ELK_FL_ENDBL_NA);
				next_block[1] = word[pos];
				break;
			}

			case ELK_FL_ENDBL_A:
			{
				next_block[0] = (_UCHAR) (next_attr | ELK_FL_ENDBL_A);
				next_block[1] = word[pos];
				next_block[2] = (_UCHAR) (attr & ELK_ATTR_MASK);
				break;
			}

			case ELK_FL_NOP:
				break;

			default:
				ELK_ASSRT(0x0008);
		}

		// ---------------- Update cur node layer references -----------------------

		ElkUpdateLayerIndex(pos, elk, cur_block);

		// ---------------- Update nextnode layer references -----------------------

		cur_state = ElkUpdateLayerIndex(pos + 1, elk, next_block);

		// ---------------- That's it !---------------------------------------------

		first = 0;
		wpresent = 0;    // There was no exactly same word in dict
	} // ===================== End of add word cycle ============================

	if (wpresent == 1) // Word is present, but may be part of some other, let's see
	{
		// If was middle of some word, let's put there enw mark and attr
		ElkGetNodePtr(ELK_FORMSTATE(pos, 0, lnum + bp), elk, &next_block, &lnum);

		if ((!ELK_IS_ENDWORD(next_block)) && (!ELK_IS_ENDWBLK(next_block)))
		{
			next_block[0] |= (_UCHAR) (ELK_FL_ENDWORD | (attr & ELK_ATTR_MASK));
			added = 1;
		}
	}

	if (!wpresent)
	{
		added = 1;
	}

done:
	return added;
err:
	return 0;
}

/* *************************************************************************** */
/* *       Create new index for the layer                                    * */
/* *************************************************************************** */
_ULONG ElkUpdateLayerIndex(_INT sl, p_elk_header_type elk, p_UCHAR cur_node)
{
	_INT i;
	_INT ret = 0;
	_INT nnum, lnum, len;
	_INT max_nnodes, index;
	p_UCHAR cur_block, cur_layer, next_layer;
	p_UCHAR ptr;

	cur_layer = (p_UCHAR) elk + elk->layers[sl];
	cur_block = cur_layer + ELK_GET_ISHIFT(cur_layer);
	next_layer = (p_UCHAR) elk + elk->layers[sl + 1];

	if (cur_block >= next_layer)
	{
		goto done;
	}

	max_nnodes = elk->l_nodes[sl];

	for (nnum = lnum = index = 0; nnum <= max_nnodes; nnum++)
	{
		if (nnum == (index << ELK_NNODES_PER_I_SH)) // Time to fill in nerw index
		{
			if (nnum >= max_nnodes) // Index table is fulled, need to add new index
			{
				ptr = cur_layer + index*ELK_INDEX_SIZE;
				len = (_ULONG) (elk) +elk->elk_size - (_ULONG) ptr;
				HWRMemCpy(ptr + ELK_INDEX_SIZE, ptr, len);
				elk->elk_size += ELK_INDEX_SIZE;
				cur_block += ELK_INDEX_SIZE;
				cur_node += ELK_INDEX_SIZE;
				next_layer += ELK_INDEX_SIZE;
				for (i = sl + 1; i < ELK_MAX_WORDLEN2; i++)
				{
					elk->layers[i] += ELK_INDEX_SIZE;
				}
				for (i = 0, ptr = cur_layer; i < index; i++, ptr += ELK_INDEX_SIZE)
				{
					len = ELK_GET_ISHIFT(cur_layer + i*ELK_INDEX_SIZE);
					ELK_SET_ISHIFT(cur_layer + i*ELK_INDEX_SIZE, len + ELK_INDEX_SIZE);
				}
				max_nnodes += ELK_NNODES_PER_INDEX;
				elk->l_nodes[sl] += (_SHORT) ELK_NNODES_PER_INDEX;
			}

			ELK_SET_ISHIFT(cur_layer + index*ELK_INDEX_SIZE, (_ULONG) cur_block - (_ULONG) cur_layer);
			ELK_SET_INNODES(cur_layer + index*ELK_INDEX_SIZE, lnum);
			index++;
		}

		if (cur_block >= next_layer)
		{
			break;
		}

		if (ELK_IS_STDBLOK(cur_block))
		{
			lnum += (ELK_IS_STDSBLK(cur_block)) ? 1 : cur_block[1];
		}
		if (cur_block == cur_node)
		{
			ret = nnum;    // We found asked node position
		}

		ELK_GETNEXTND(cur_block);
	}

	if (nnum < max_nnodes - (ELK_NNODES_PER_INDEX + 1))  // Reduction of layer
	{
		index = (max_nnodes - 1) >> ELK_NNODES_PER_I_SH;
		ptr = cur_layer + (index + 1)*ELK_INDEX_SIZE;
		len = (_ULONG) (elk) +elk->elk_size - (_ULONG) ptr;
		HWRMemCpy(ptr - ELK_INDEX_SIZE, ptr, len);
		elk->elk_size -= ELK_INDEX_SIZE;
		ret -= ELK_INDEX_SIZE;
		for (i = sl + 1; i < ELK_MAX_WORDLEN2; i++)
		{
			elk->layers[i] -= ELK_INDEX_SIZE;
		}
		for (i = 0, ptr = cur_layer; i < index; i++, ptr += ELK_INDEX_SIZE)
		{
			len = ELK_GET_ISHIFT(cur_layer + i*ELK_INDEX_SIZE);
			ELK_SET_ISHIFT(cur_layer + i*ELK_INDEX_SIZE, len - ELK_INDEX_SIZE);
		}
		elk->l_nodes[sl] -= (_SHORT) ELK_NNODES_PER_INDEX;
	}

done:
	return ELK_FORMSTATE(sl, 0, ret);
}

#if ELK_DEBUG || ELK_OPTIMIZEDICT
/* *************************************************************************** */
/* *       Get layer statistics                                              * */
/* *************************************************************************** */
_INT ElkGetStat(_INT layer, p_INT stats, p_VOID pd)
{
	p_elk_header_type elk = (p_elk_header_type) pd;
	p_UCHAR cur_block, next_layer;

	cur_block = (p_UCHAR) elk + elk->layers[layer];
	cur_block += ELK_GET_ISHIFT(cur_block);
	next_layer = (p_UCHAR) elk + elk->layers[layer + 1];


	if (cur_block >= next_layer)
	{
		goto err;
	}

	while (cur_block < next_layer)
	{
		switch (cur_block[0] & ELK_FL_STBMASK)
		{
			case ELK_FL_STDBLK:
				stats[0] ++;
				if (cur_block[1] == 1)
				{
					stats[1] ++;
				}
				if (cur_block[1] == 2)
				{
					stats[2] ++;
				}
				break;
			case ELK_FL_STDSBLK:
				stats[0] ++;
				stats[1] ++;
			case ELK_FL_ENDWBLK:
				stats[3] ++;
				break;
			case ELK_FL_ENDBL_A:
				stats[4] ++;
				break;
			case ELK_FL_ENDBL_NA:
				stats[5] ++;
				break;
			case ELK_FL_ENDBL_MA:
				stats[6] ++;
				break;
			case ELK_FL_ENDBL_MNA:
				stats[7] ++;
				break;
		}

		ELK_GETNEXTND(cur_block);
	}

	return ELK_NOERR;
err:
	return ELK_ERR;
}


/* *************************************************************************** */
/* *       Optimize      dictionary                                          * */
/* *************************************************************************** */
_INT ElkOptimizeDict(p_VOID _PTR pd)
{
	_INT    fbn, counter, height, depth;
	_UCHAR  word[ELK_MAX_WORDLEN];
	fw_buf_type fwb[ELK_MAX_SYMINNODE];
	p_elk_header_type elk, telk;

	elk = (p_elk_header_type) (*pd);
	if ((telk = (p_elk_header_type) HWRMemoryAlloc(elk->alloc_size + ELK_MEMBLOK_SIZE)) == _NULL)
	{
		goto err;
	}
	HWRMemCpy(telk, elk, elk->alloc_size);
	HWRMemoryFree(elk);
	elk = telk;
	elk->alloc_size += ELK_MEMBLOK_SIZE;


	ElkOptStdBlocks(elk);

	counter = 0;

	do
	{
		depth = 0;
		height = 0;
		fbn = ElkGetNextSyms(0, &fwb[0], elk);
		if (ElkFollowBranch(depth, word, &height, &counter, fbn, &fwb[0], elk))
		{
			goto err;
		}
	}
	while (height < 0);   // Need to reread top level list sometimes

	elk->type = ELK_OPT1_TYPE;
	*pd = (p_VOID) elk;

	return counter;
err:
	return -1;
}

/* *************************************************************************** */
/* *       Attach statistical code table to an optimized dictionary          * */
/* *************************************************************************** */
_INT ElkAttachCodeTable(_INT ctbl_size, p_UCHAR code_table, p_VOID _PTR pd)
{
	_INT   i, f;
	_INT   size;
	_UCHAR sym;
	_UCHAR index[CTBL_INDEX_SIZE];
	p_elk_header_type elk, telk;

	HWRMemSet(index, 0, sizeof(index));
	for (i = 0, f = 1; i < ctbl_size; i++)
	{
		sym = code_table[i];
		if (f)
		{
			index[sym >> 3] |= (_UCHAR) (0x01 << (sym & 0x07));
		}
		if (sym == 0)
		{
			f = 1;
		}
		else
		{
			f = 0;
		}
	}

	elk = (p_elk_header_type) (*pd);
	size = elk->elk_size + ctbl_size + sizeof(index);
	if ((telk = (p_elk_header_type) HWRMemoryAlloc(size)) == _NULL)
	{
		goto err;
	}
	HWRMemCpy(telk, elk, elk->elk_size);
	HWRMemCpy((p_UCHAR) telk + elk->elk_size, index, sizeof(index));
	HWRMemCpy((p_UCHAR) telk + elk->elk_size + sizeof(index), code_table, ctbl_size);
	HWRMemoryFree(elk);
	elk = telk;
	elk->ctbl_size = ctbl_size + sizeof(index);
	elk->alloc_size = size;
	elk->type = ELK_OPT2_TYPE;

	*pd = (p_VOID) elk;

	return ELK_NOERR;
err:
	return ELK_ERR;
}
/* **************************************************************************** */
/* *        Recursive procedure for traversing branches                       * */
/* **************************************************************************** */
_INT ElkFollowBranch(_INT depth, p_UCHAR word, p_INT height, p_INT counter, _INT fbn, p_fw_buf_type fwb, p_VOID pd)
{
	_INT i, lfbn;
	fw_buf_type lfwb[ELK_MAX_SYMINNODE];

	ELK_ASSERT((depth + 1 >= ELK_MAX_WORDLEN), 0x0004);

	for (i = 0; i < fbn; i++)
	{
		word[depth] = fwb[i].sym;
		word[depth + 1] = 0;

		if (fwb[i].l_status < XRWD_BLOCKEND)
		{
			lfbn = ElkGetNextSyms(&fwb[i], &lfwb[0], pd);
			if (ElkFollowBranch(depth + 1, word, height, counter, lfbn, &lfwb[0], pd))
			{
				goto err;
			}

			if (*height == -1)
			{
				*height = 0;
				i--;
				continue;
			}
			if (*height > 0 && fbn == 1) if (!ELK_ST_ENDBL_M(fwb[i].state))
				{
					(*height)++;
				}
			if (*height > 1 && fbn > 1)
			{
				(*counter) += *height - 1;
				if (ElkOptimizeWord(word, pd))
				{
					goto err;
				}
				*height = -1;
				break; // Need to go up and reread buffer
			}
			if (*height > 0 && fbn > 1)
			{
				(*height) = 0;
			}
			if (fwb[i].l_status > XRWD_MIDWORD)
			{
				(*height) = 0;
			}
		}
		else
		{
			if (fbn == 1)
			{
				*height = 1;    // Begin ascend from blockend
			}
		}
	}

	return ELK_NOERR;
err:
	return ELK_ERR;
}

/* **************************************************************************** */
/* *        Recursive procedure for traversing branches                       * */
/* **************************************************************************** */
_INT ElkOptimizeWord(p_UCHAR inpw, p_VOID pd)
{
	_INT    i, j, pos, ns, len;
	_INT    fsp1, fsp2;
	_INT    next_node_type;
	_INT    lnum;
	_UCHAR  word[ELK_MAX_WORDLEN];
	_UCHAR  prev_attr;
	_ULONG  state, nstate;
	p_elk_header_type elk = (p_elk_header_type) pd;
	p_UCHAR cur_block, next_block;
	p_UCHAR ptr;


	HWRStrCpy((_STR) word, (_STR) inpw);

	for (i = HWRStrLen((_STR) inpw) - 1; i > 0; i--)
	{
		word[i] = 0;

		if (ElkGetPrefixState(word, &state, &cur_block, &nstate, elk))
		{
			goto err;
		}

		pos = ELK_ST_GETLAYER(state);


		if (!ELK_IS_STDSBLK(cur_block))
		{
			break;    // Works only on compressing std blocks
		}

		ElkGetNodePtr(nstate, elk, &next_block, &lnum);

		switch (next_block[0] & ELK_FL_STBMASK)
		{
			case ELK_FL_ENDBL_NA:
			{
				fsp1 = -((ELK_FLAG_SIZE + ELK_SYMB_SIZE) - (ELK_FLAG_SIZE + ELK_SNUM_SIZE + ELK_SYMB_SIZE + ELK_SYMB_SIZE));
				fsp2 = -(ELK_FLAG_SIZE + ELK_SYMB_SIZE);
				ns = ELK_SYMB_SIZE + ELK_SYMB_SIZE;
				next_node_type = ELK_FL_ENDBL_MNA;
				break;
			}
			case ELK_FL_ENDBL_A:
			{
				fsp1 = -((ELK_FLAG_SIZE + ELK_SYMB_SIZE) - (ELK_FLAG_SIZE + ELK_SNUM_SIZE + ELK_SYMB_SIZE + ELK_FLAG_SIZE + ELK_SYMB_SIZE));
				fsp2 = -(ELK_FLAG_SIZE + ELK_SYMB_SIZE + ELK_FLAG_SIZE);
				ns = ELK_SYMB_SIZE + ELK_SYMB_SIZE;
				next_node_type = ELK_FL_ENDBL_MA;
				prev_attr = next_block[2];
				break;
			}
			case ELK_FL_ENDBL_MNA:
			{
				fsp1 = -((ELK_FLAG_SIZE + ELK_SYMB_SIZE) - (ELK_FLAG_SIZE + ELK_SNUM_SIZE + next_block[1] + ELK_SYMB_SIZE));
				fsp2 = -(ELK_FLAG_SIZE + ELK_SNUM_SIZE + next_block[1]);
				ns = next_block[1] + 1;
				next_node_type = ELK_FL_ENDBL_MNA;
				break;
			}
			case ELK_FL_ENDBL_MA:
			{
				fsp1 = -((ELK_FLAG_SIZE + ELK_SYMB_SIZE) - (ELK_FLAG_SIZE + ELK_SNUM_SIZE + next_block[1] + ELK_SYMB_SIZE + ELK_FLAG_SIZE));
				fsp2 = -(ELK_FLAG_SIZE + ELK_SNUM_SIZE + next_block[1] + ELK_FLAG_SIZE);
				ns = next_block[1] + 1;
				next_node_type = ELK_FL_ENDBL_MA;
				prev_attr = next_block[next_block[1] + 2];
				break;
			}
			default:
				ELK_ASSERT(i, 0x0005);
		}

		// --- Create needed space for current MultiNode ---------------------------

		ptr = cur_block;
		ELK_GETNEXTND(ptr);
		len = (_ULONG) ((p_UCHAR) elk + elk->elk_size) - (_ULONG) ptr;
		HWRMemCpy(ptr + fsp1, ptr, len);
		elk->elk_size += fsp1;
		next_block += fsp1;

		// ---------------- Update page index -------------------------------------

		for (j = pos + 1; j < ELK_MAX_WORDLEN2; j++)
		{
			elk->layers[j] += fsp1;
		}

		cur_block[0] &= ~(ELK_FL_STBMASK);
		cur_block[0] |= (_UCHAR) next_node_type;
		cur_block[1] = (_UCHAR) ns;
		cur_block[2] = word[i - 1];
		for (j = 0; j < ns - 1; j++)
		{
			cur_block[3 + j] = next_block[(ns > 2) ? j + 2 : j + 1];
		}
		if (next_node_type == ELK_FL_ENDBL_MA)
		{
			cur_block[ns + 2] = prev_attr;
		}

		// ----------------- Remove next node --------------------------------------

		ptr = next_block;
		ELK_GETNEXTND(ptr);
		len = (_ULONG) ((p_UCHAR) elk + elk->elk_size) - (_ULONG) ptr;
		HWRMemCpy(ptr + fsp2, ptr, len);
		elk->elk_size += fsp2;

		// ---------------- Update page index -------------------------------------

		for (j = pos + 2; j < ELK_MAX_WORDLEN2; j++)
		{
			elk->layers[j] += fsp2;
		}

		// ---------------- Update cur node layer references -----------------------

		ElkUpdateLayerIndex(pos, elk, cur_block);

		// ---------------- Update nextnode layer references -----------------------

		ElkUpdateLayerIndex(pos + 1, elk, next_block);

		// ---------------- That's it !---------------------------------------------
	}

	return ELK_NOERR;
err:
	return ELK_ERR;
}

/* *************************************************************************** */
/* *       Optimiza sing-letter std blocks                                   * */
/* *************************************************************************** */
_INT ElkOptStdBlocks(p_elk_header_type elk)
{
	_INT i, sl, len;
	p_UCHAR cur_block, next_layer, ptr;

	for (sl = 1; sl < ELK_MAX_WORDLEN; sl++)
	{
		cur_block = (p_UCHAR) elk + elk->layers[sl];
		cur_block += ELK_GET_ISHIFT(cur_block);
		next_layer = (p_UCHAR) elk + elk->layers[sl + 1];

		if (cur_block >= next_layer)
		{
			goto err;
		}

		while (cur_block < next_layer)
		{
			if ((cur_block[0] & ELK_FL_STBMASK) == ELK_FL_STDBLK && cur_block[1] == 1)
			{
				//        index = (max_nnodes-1) >> ELK_NNODES_PER_I_SH;
				cur_block[0] &= (_UCHAR) (~(ELK_FL_STBMASK));
				cur_block[0] |= ELK_FL_STDSBLK;
				ptr = cur_block + ELK_STDNODE_SIZE;
				len = (_ULONG) (elk) +elk->elk_size - (_ULONG) ptr;
				HWRMemCpy(ptr - ELK_SNUM_SIZE, ptr, len);
				elk->elk_size -= ELK_SNUM_SIZE;
				next_layer -= ELK_SNUM_SIZE;
				for (i = sl + 1; i < ELK_MAX_WORDLEN2; i++)
				{
					elk->layers[i] -= ELK_SNUM_SIZE;
				}
				ElkUpdateLayerIndex(sl, elk, cur_block);
			}

			ELK_GETNEXTND(cur_block);
		}
	}
	return ELK_NOERR;
err:
	return ELK_ERR;
}

#endif //#if ELK_DEBUG

/* *************************************************************************** */
/* *       Creates empty dictionary                                          * */
/* *************************************************************************** */
_INT ElkCreateDict(p_VOID _PTR pd)
{
	_INT i, sh;
	p_elk_header_type elk;

	if ((elk = (p_elk_header_type) HWRMemoryAlloc(ELK_MEMBLOK_SIZE)) == _NULL)
	{
		goto err;
	}

	HWRMemSet(elk, 0, ELK_MEMBLOK_SIZE);

	elk->ver = ELK_VER_ID;
	elk->type = ELK_STND_TYPE;
	elk->elk_size =
	    elk->alloc_size = (_ULONG) &elk->elk - (_ULONG) elk + ELK_FLAG_SIZE + ELK_INDEX_SIZE*ELK_MAX_WORDLEN2;
	//  elk->alloc_size = ELK_MEMBLOK_SIZE;

	sh = (_ULONG) &elk->elk - (_ULONG) elk;
	elk->layers[0] = sh;
	sh += ELK_INDEX_SIZE + ELK_FLAG_SIZE;
	for (i = 1; i < ELK_MAX_WORDLEN2; i++, sh += ELK_INDEX_SIZE)
	{
		elk->layers[i] = sh;
	}
	for (i = 0; i < ELK_MAX_WORDLEN2; i++)
	{
		elk->l_nodes[i] = ELK_NNODES_PER_INDEX;
	}
	for (i = 0; i < ELK_MAX_WORDLEN2; i++)
	{
		ELK_SET_ISHIFT((p_UCHAR) elk + elk->layers[i], ELK_INDEX_SIZE);
	}

	*((p_UCHAR) elk + elk->layers[0] + ELK_INDEX_SIZE) = ELK_FL_ENDWBLK;

	*pd = (p_VOID) elk;

	return ELK_NOERR;
err:
	return ELK_ERR;
}

/* *************************************************************************** */
/* *       Free dict                                                         * */
/* *************************************************************************** */
_INT ElkFreeDict(p_VOID _PTR pd)
{

	if (*pd == _NULL)
	{
		goto err;
	}

	HWRMemoryFree(*pd);
	*pd = _NULL;

	return ELK_NOERR;
err:
	return ELK_ERR;
}

#if ELK_FILEOP
/* *************************************************************************** */
/* *       Load dict                                                         * */
/* *************************************************************************** */
_INT ElkLoadDict(p_UCHAR name, p_VOID _PTR pd)
{
	_INT   i;
	_INT   e = ELK_NOERR;
	_INT   size, shift = 0;
	_UCHAR ids[64] = { ELK_ID_STRING };
	elk_header_type elk;
	p_elk_header_type pelk;
	FILE * file;

	if ((file = fopen((_STR) name, "rb")) == _NULL)
	{
		goto err;
	}

	fread(ids, ELK_ID_LEN, 1, file);
	ids[ELK_ID_LEN] = 0;

	if (HWRStrCmp((_STR) ids, ELK_ID_STRING) != 0)
	{
		if (HWRStrCmp((_STR) ids, ELK_ID_STRING_PREV) != 0)
		{
			fclose(file);
			ELK_ASSRT(0x0006);
		}
		e = ELK_PREV_VER_ERR; // Mark that
		shift = sizeof(elk.ctbl_size);
	}

	fread(&elk, sizeof(elk), 1, file);

	size = elk.elk_size + ((e == ELK_PREV_VER_ERR) ? 0 : elk.ctbl_size);
	if ((pelk = (p_elk_header_type) HWRMemoryAlloc(size + 16)) == _NULL)
	{
		fclose(file);
		ELK_ASSRT(0x0006);
	}

	*pelk = elk;
	if (e == ELK_PREV_VER_ERR) // Convert header
	{
		HWRMemCpy(&pelk->alloc_size, &pelk->ctbl_size, sizeof(*pelk));
		pelk->ctbl_size = 0;
		pelk->elk_size += shift;
		for (i = 0; i < ELK_MAX_WORDLEN2; i++)
		{
			pelk->layers[i] += shift;
		}
	}

	pelk->alloc_size = pelk->elk_size + pelk->ctbl_size;

	fread((p_UCHAR) (pelk + 1) + shift, pelk->alloc_size - (sizeof(elk) - shift), 1, file);

	fclose(file);

	*pd = (p_VOID) pelk;

	return e;
err:
	return ELK_ERR;
}

/* *************************************************************************** */
/* *       Save dict                                                         * */
/* *************************************************************************** */
_INT ElkSaveDict(p_UCHAR name, p_VOID pd)
{
	_UCHAR ids [] = { ELK_ID_STRING };
	p_elk_header_type elk = (p_elk_header_type) pd;
	FILE * file;

	if ((file = fopen((_STR) name, "wb")) == _NULL)
	{
		goto err;
	}

	fwrite(ids, HWRStrLen((_STR) ids), 1, file);
	fwrite(elk, elk->elk_size + elk->ctbl_size, 1, file);

	fclose(file);

	return ELK_NOERR;
err:
	return ELK_ERR;
}

#else //#if ELK_FILEOP

/* *************************************************************************** */
/* *       Load dict                                                         * */
/* *************************************************************************** */
_INT ElkLoadDict(p_UCHAR store, p_VOID _PTR pd)
{
	_INT i;
	_INT e = ELK_NOERR;
	_INT size, shift = 0;
	_CHAR  ids[] = {ELK_ID_STRING};
	_CHAR  idsp[] = {ELK_ID_STRING_PREV};
	p_elk_header_type elk, pelk;


	if (store == _NULL)
	{
		goto err;
	}

	if (HWRStrnCmp((_STR)store, ids, sizeof(ids)-1) != 0)
	{
		if (HWRStrnCmp((_STR)store, idsp, sizeof(idsp)-1) != 0)
		{
			ELK_ASSRT(0x0006);
		}
		e = ELK_PREV_VER_ERR; // Mark that
		elk = (p_elk_header_type)(store + sizeof(idsp)-1);
		shift = sizeof(elk->ctbl_size);
	}
	else
	{
		elk = (p_elk_header_type)(store + sizeof(ids)-1);
	}

	size = elk->elk_size + ((e == ELK_PREV_VER_ERR) ? 0:elk->ctbl_size);
	if ((pelk = (p_elk_header_type)HWRMemoryAlloc(size+16)) == _NULL)
	{
		ELK_ASSRT(0x0006);
	}

	*pelk = *elk;
	if (e == ELK_PREV_VER_ERR) // Convert header
	{
		HWRMemCpy(&pelk->alloc_size, &pelk->ctbl_size, sizeof(*pelk));
		pelk->ctbl_size = 0;
		pelk->elk_size += shift;
		for (i = 0; i < ELK_MAX_WORDLEN2; i ++)
		{
			pelk->layers[i] += shift;
		}
	}

	HWRMemCpy(&pelk->elk, (p_UCHAR)(&elk->elk)-shift, elk->elk_size+pelk->ctbl_size-(((_ULONG)(&(elk->elk))-(_ULONG)(elk)) - shift));

	pelk->alloc_size = pelk->elk_size+pelk->ctbl_size;

	*pd = (p_VOID)pelk;

	return e;
err:
	*pd = _NULL;
	return ELK_ERR;
}

/* *************************************************************************** */
/* *       Save dict                                                         * */
/* *************************************************************************** */
_INT ElkSaveDict(p_UCHAR store, p_VOID pd)
{
	_UCHAR ids[] = {ELK_ID_STRING};
	p_elk_header_type elk = (p_elk_header_type)pd;

	if (store == _NULL)
	{
		goto err;
	}

	HWRStrCpy((_STR)store, (_STR)ids);
	HWRMemCpy(store+sizeof(ids)-1, elk, elk->elk_size+elk->ctbl_size);

	return ELK_NOERR;
err:
	return ELK_ERR;
}

#endif //#if ELK_FILEOP

/* *************************************************************************** */
/* *       Get dictionary len and changed attributes                         * */
/* *************************************************************************** */
_INT ElkGetDictStatus(p_INT plen, p_VOID pd)
{
	int ret = 0;
	_UCHAR ids [] = { ELK_ID_STRING };
	p_elk_header_type elk = (p_elk_header_type) pd;

	if (elk->elk_size + elk->ctbl_size != elk->alloc_size)
	{
		ret = 1;
	}

	*plen = (_INT) (elk->elk_size + elk->ctbl_size + sizeof(ids));

	return ret;
}

/* *************************************************************************** */
/* *       Get dictionary type -- compressed mode                            * */
/* *************************************************************************** */
_INT ElkGetDictType(p_VOID pd)
{
	p_elk_header_type elk = (p_elk_header_type) pd;

	return (_INT) elk->type;
}

#endif // ELK_SUPPORTFUNCS
/* *************************************************************************** */
/* *       END OF ALL                                                        * */
/* *************************************************************************** */

// Some kind of description:
//
//  Dictionary consists of layers of nodes, and before optimization
//  each layer holds nodes corresponding to the same order-number of
//  letter of words (3 layer, say, holds letters 'tlk' from words test, hello, marker).
//  There are 7 node types -- STD, STDS, WBLK, A, NA, MA, MNA.
//  STD node holds letters that have continuations in next layer (or WBLKs).
//  STDS node is subset of STD, which is shorter, and designate one-latter
//  STD node. WBLK is terminating one-byte node, not holding any letter.
//  It marks end of word on previous layer's STD node. A and NA are terminating
//  nodes with one last letter of a word, A with attribute, NA without.
//  MA and MNA are analogous to A and NA, but may hold many leters.
//  There are two index tables in dictionary -- one is layer index,
//  other holds max number of nodes for each layer (for layer indexes).
//  Also, there are indexes in the beginning of each layer. They are
//  not really required, but added for speeding access to dictionary.
//  Each record holds two numbers -- shift from beginnig of layer and
//  number of nodes that layer to that point generates on next layer.
//  Each index is created for a given number of nodes on the layer.
//  There is no pointers or direct references from some node to its
//  continuation nodes, -- they are calculated by node's number in
//  its layer. Number of continuation node is calculated as number
//  letters in all node up to current in a given layer, which require
//  continuation. Thus, if there are 1234 such letters in nodes of
//  current layer up to a given node, continuation nodes for this
//  particular node will start from node number 1234 on next layer.
//  Continuations for letters of the node will go in the same order
//  as the letters in the node. Layer indexes help to find required
//  node quicker. So, if index size is 64, then shift to node 1234
//  may be found as shift in index number (1234/64) with following
//  run, counting (1234%64) additional nodes.
//  Initial creation routine creates dictionary with STD, A, NA, and
//  WBLK nodes. Optional optimization adds STDS, NA, and MNA nodes,
//  freeing some space in by changing frequent special cases with
//  those more compact nodes.
//
//  Additions: Stat coding tables support.
//  New function and changes to ver 2.1 allow for appending a coding table to
//  the dictionary. This table stores expansions of certain symbols to strings
//  of regular symbols. When parsing program founds such symbol (represented by
//  1 in the header index of the coding table (256x1)) it should substitute it
//  with sequence of symbols from the string. Code symbols and assigned to them
//  strings are stored in the coding table after the index, separated by 0.

#endif // VER_PALK_DICT




