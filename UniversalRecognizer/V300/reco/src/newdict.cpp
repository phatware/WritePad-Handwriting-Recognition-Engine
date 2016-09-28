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

#include <stdio.h>

#include "hwr_sys.h"
#include "ams_mg.h"                           /* Most global definitions     */
#include "xrwdict.h"
#include "pydict.h"

// --------------- Defines -----------------------------------------------------

#define PYDICT_ST_GETRANK(state) (((state) >> 24) & 0xFF)
#define PYDICT_ST_GETNUM(state) (((state) >> 8) & 0xFFFF)
#define PYDICT_FORMSTATE(rank,num_in_layer) ( ((rank) << 24) + ((num_in_layer)<<8) )

// --------------- Redirect PZDICT calls to PYDICT  ----------------------------------------

#if !defined PYDICT_SUPPORT_PROGRAM && !defined RECO_TOOLKIT
#define PYDictGetNextSyms     PZDictGetNextSyms
#define PYDictAddWord         PZDictAddWord
#define PYDictCreateDict      PZDictCreateDict
#define PYDictFreeDict        PZDictFreeDict
#define PYDictLoadDict        PZDictLoadDict
#define PYDictSaveDict        PZDictSaveDict
#define PYDictCheckWord       PZDictCheckWord
#define PYDictGetDictStatus   PZDictGetDictStatus
#define PYDictGetDictType     PZDictGetDictType

#endif //PYDICT_SUPPORT_PROGRAM

// ----------------------- Redirect HWR ----------------------------------------

#define GetShort(p)  ( (short)( *(unsigned char *)(p) ) + \
                       ( ((short)( *((char *)(p) +1) )) <<8 )  )

#define GetUShort(p)  ( (unsigned short)( *(unsigned char *)(p) ) + \
                        ( ((unsigned short)( *((unsigned char *)(p) +1) )) <<8 )  )

#define GetUInt(p)   ( (unsigned int)( *(unsigned char *)(p) ) + \
                        ( ((unsigned int)( *((unsigned char *)(p) +1) )) <<8 ) + \
                        ( ((unsigned int)( *((unsigned char *)(p) +2) )) <<16 ) + \
                        ( ((unsigned int)( *((unsigned char *)(p) +3) )) <<24 ) \
                      )
/* *************************************************************************** */
/* *       Find dvset size                                                   * */
/* *************************************************************************** */

_INT find_dvset_size( _UCHAR *p, _INT dvset_len )
{
    _INT i;
    _INT size = 0;
    
    for ( i = 0; i < dvset_len; i++ )
    {
        if ( (*p) & SHORT_VADR_FLAG )
        {
            size += 2;
            p += 2;
        }
        else
        {
            size += 3;
            p += 3;
        }
    }
    
    return (size);
}

/* *************************************************************************** */
/* *       find dvset                                                        * */
/* *************************************************************************** */

_UCHAR *find_dvset( _CHAR *pDvsetTabl, _INT dvset_num, _INT *pdvset_len )
{
    _ULONG tshift;
    _ULONG tbshift, dv_shift;
    _INT i;
    _INT num_set;
    _CHAR *ptb, *pth;
    _UCHAR *p;
    
    if ( dvset_num < MIN_LONG_DVSET_NUM )
        tshift = sizeof(_ULONG);
    else
        tshift = GetUInt(pDvsetTabl);
    
    ptb = pDvsetTabl + tshift;
    pth = ptb;
    tbshift = 0;
    dv_shift = GetUInt(pth+2);
    while ( tbshift < dv_shift && GetShort(pth) < dvset_num )
    {
        pth += 7;
        tbshift += 7;
    }
    if ( tbshift >= dv_shift || GetShort(pth) > dvset_num )
        pth -= 7;
    
    *pdvset_len = (_UCHAR) pth[6];
    tbshift = GetUInt(pth+2);
    num_set = GetShort(pth);
    p = (_UCHAR *) (ptb + tbshift);
    for ( i = 0; i < dvset_num - num_set; i++ )
        p += find_dvset_size( p, *pdvset_len );
    return (p);
}

/* *************************************************************************** */
/* *       find dvset len                                                    * */
/* *************************************************************************** */

_INT find_dvset_len( _CHAR *pDvsetTabl, _INT dvset_num )
{
    _ULONG  tshift;
    _ULONG  tbshift, dv_shift;
    _CHAR * pth;
    _INT    dvset_len;
    
    if ( dvset_num < MIN_LONG_DVSET_NUM )
        tshift = sizeof(_ULONG);
    else
        tshift = GetUInt(pDvsetTabl);
    
    pth = pDvsetTabl + tshift;
    tbshift = 0;
    dv_shift = GetUInt(pth+2);
    while ( tbshift < dv_shift && GetShort(pth) < dvset_num )
    {
        pth += 7;
        tbshift += 7;
    }
    if ( tbshift >= dv_shift || GetShort(pth) > dvset_num )
        pth -= 7;
    
    dvset_len = (_UCHAR) pth[6];
    return (dvset_len);
}

/* *************************************************************************** */
/* *       Find chset                                                        * */
/* *************************************************************************** */

_CHAR *find_chset( _CHAR *pChsetTabl, _INT chset_num, _INT *pchsetlen )
{
    _ULONG  tshift;
    _ULONG  tbshift, syms_shift;
    _INT    num_set;
    _CHAR * ptb;
    _CHAR * p;
    _CHAR * pth;
    
    if ( chset_num < MIN_LONG_CHSET_NUM )
        tshift = sizeof(_ULONG);
    else
        tshift = GetUInt(pChsetTabl);
    
    ptb = pChsetTabl + tshift;
    pth = ptb;
    tbshift = 0;
    syms_shift = GetUInt(pth+2);
    while ( tbshift < syms_shift && GetShort(pth) < chset_num )
    {
        pth += 7;
        tbshift += 7;
    }
    if ( tbshift >= syms_shift || GetShort(pth) > chset_num )
        pth -= 7;
    
    *pchsetlen = (_UCHAR) pth[6];
    tbshift = GetUInt(pth+2);
    num_set = GetShort(pth);
    p = ptb + tbshift + (chset_num - num_set) * (*pchsetlen);
    return (p);
}

/* *************************************************************************** */
/* *      Find chset len                                                     * */
/* *************************************************************************** */
_INT find_chset_len( _CHAR *pChsetTabl, _INT chset_num )
{
    _ULONG tshift;
    _ULONG tbshift, syms_shift;
    _INT chset_len;
    _CHAR *pth;
    
    if ( chset_num < MIN_LONG_CHSET_NUM )
        tshift = sizeof(_ULONG);
    else
        tshift = GetUInt(pChsetTabl);
    
    pth = pChsetTabl + tshift;
    tbshift = 0;
    syms_shift = GetUInt(pth+2);
    while ( tbshift < syms_shift && GetShort(pth) < chset_num )
    {
        pth += 7;
        tbshift += 7;
    }
    if ( tbshift >= syms_shift || GetShort(pth) > chset_num )
        pth -= 7;
    
    chset_len = (_UCHAR) pth[6];
    return (chset_len);
}

/* *************************************************************************** */
/* *       Copy dvsset                                                       * */
/* *************************************************************************** */
_INT copy_dvset( _INT *dvset, _UCHAR *p, _INT dvset_len )
{
    _INT i;
    _INT size = 0;
    
    for ( i = 0; i < dvset_len; i++ )
    {
        if ( (*p) & SHORT_VADR_FLAG )
        {
            dvset[i] = (((*p) & 0x7F) << 8);
            p++;
            dvset[i] += (*p);
            p++;
            size += 2;
        }
        else
        {
            dvset[i] = ((*p) << 16);
            p++;
            dvset[i] += ((*p) << 8);
            p++;
            dvset[i] += (*p);
            p++;
            size += 3;
        }
    }
    return (size);
}

/* *************************************************************************** */
/* *       Decode vert                                                       * */
/* *************************************************************************** */
_CHAR *decode_vert( _VOID *pVoc, _CHAR *vert, _INT *dvset, _INT *pdvset_len, _CHAR *chset, _INT *pchset_len )
{
    _UCHAR * p = (_UCHAR *)vert;
    _UCHAR  byte1;
    _CHAR * cchset;
    _UCHAR *tdvset;
    _INT    dvset_num, chset_num, chset_len;
    _CHAR * pDvsetTabl = (_CHAR *) PYDictGetDvsetTabl(pVoc);
    _CHAR * pChsetTabl = (_CHAR *) PYDictGetChsetTabl(pVoc);
    _BOOL   NotLast;
    
    byte1 = (_UCHAR) (*p);
    if ( byte1 & ONE_BYTE_FLAG ) //1000 0000
    {
        chset[0] = (_CHAR) (byte1 & 0x7F); //0111 1111
        *pchset_len = (chset[0] < 5) ? 0 : 1;
        *pdvset_len = 0;
        p++;
    }
    else if ( IsTreeMerged(pVoc) )
    {
        if ( !(byte1 & END_WRD_FLAG) && (byte1 & CODED_DVSET_FLAG) ) //0010 0000
        {
            if ( byte1 & SHORT_DVSET_NUM_FLAG ) //0001 0000
            {
                dvset_num = byte1 & DVSET_NUM_MASK;
                p++;
            } //0000 1111
            else
            {
                dvset_num = (byte1 & DVSET_NUM_MASK) << 8;
                p++;
                dvset_num += (_UCHAR) (*p);
                p++;
            }
            tdvset = find_dvset( pDvsetTabl, dvset_num, pdvset_len );
            copy_dvset( dvset, tdvset, *pdvset_len );
        }
        else
        {
            *pdvset_len = byte1 & DVSET_LEN_MASK;
            p++; //0000 1111
            p += copy_dvset( dvset, (_UCHAR *) p, *pdvset_len );
        }
        
        if ( (*p) & CODED_CHSET_FLAG ) //1000 0000
        {
            if ( (*p) & SHORT_CHSET_NUM_FLAG ) //0100 0000
            {
                chset_num = (*p) & CHSET_NUM_MASK; //0011 1111
                p++;
            }
            else
            {
                chset_num = ((*p) & CHSET_NUM_MASK) << 8;
                p++;
                chset_num += (_UCHAR) (*p);
                p++;
            }
            cchset = find_chset( pChsetTabl, chset_num, pchset_len );
            memcpy( chset, cchset, (*pchset_len) );
        }
        else
        {
            NotLast = true;
            chset[0] = (*p); //chset len >1 !!!
            p++;
            chset_len = 1;
            while ( NotLast )
            {
                chset[chset_len] = (*p) & (_CHAR) 0x7F; //0111 1111
                NotLast = (((*p) & LAST_SYM_FLAG) == 0); //1000 0000
                p++;
                chset_len++;
            }
            *pchset_len = chset_len;
        }
        
    }
    else //if PlainTree
    {
        _INT i;
        
        if ( (*p) & SHORT_ECHSET_LEN_FLAG )
        {
            chset_len = (*p) & SHORT_ECHSET_LEN_MASK;
            p++;
        }
        else
        {
            chset_len = ((*p) & SHORT_ECHSET_LEN_MASK) << 8;
            p++;
            chset_len += (*p);
            p++;
        }
        
        for ( i = 0; i < chset_len; i++ )
        {
            chset[i] = *p;
            p++;
        }
        *pchset_len = chset_len;
        *pdvset_len = 0;
    }

    return (_CHAR *)(p);
}

/* *************************************************************************** */
/* *       Pass vert                                                         * */
/* *************************************************************************** */

_CHAR *pass_vert( _VOID *pVoc, _CHAR *vert )
{
    _CHAR *p = vert;
    _CHAR byte1 = *p;
    _INT dvset_len, chset_len;
    _BOOL NotLast;
    
    if ( byte1 & ONE_BYTE_FLAG ) //1000 0000
        p++;
    else if ( IsTreeMerged(pVoc) )
    {
        if ( !(byte1 & END_WRD_FLAG) && (byte1 & CODED_DVSET_FLAG) ) //0010 0000
        {
            if ( byte1 & SHORT_DVSET_NUM_FLAG ) //0001 0000
                p++;
            else
                p += 2;
        }
        else
        {
            dvset_len = byte1 & DVSET_LEN_MASK;
            p++;
            //           p+=sizeof(short)*dvset_len;
            p += find_dvset_size( (_UCHAR *) p, dvset_len );
        }
        
        if ( (*p) & CODED_CHSET_FLAG ) //1000 0000
        {
            if ( (*p) & SHORT_CHSET_NUM_FLAG ) //0100 0000
                p++;
            else
                p += 2;
        }
        else
        {
            NotLast = true; //chsetlen>1 !!!
            while ( NotLast )
            {
                NotLast = (((*p) & LAST_SYM_FLAG) == 0); //1000 0000   //true for first byte!!!
                p++;
            }
        }
    }
    else //Plain Tree
    {
        if ( (*p) & SHORT_ECHSET_LEN_FLAG )
        {
            chset_len = (*p) & SHORT_ECHSET_LEN_MASK;
            p++;
        }
        else
        {
            chset_len = ((*p) & SHORT_ECHSET_LEN_MASK) << 8;
            p++;
            chset_len += (*p);
            p++;
        }
        
        p += chset_len;
    }
    
    return (p);
}

/* *************************************************************************** */
/* *       Find vert status                                                  * */
/* *************************************************************************** */
_UCHAR find_vert_status_and_attr( _CHAR *vert, _UCHAR *pattr )
{
    _UCHAR status;
    _UCHAR byte1 = *vert, sym;
    
    if ( byte1 & ONE_BYTE_FLAG ) //1000 0000
    {
        sym = (_UCHAR) (byte1 & 0x7F);
        if ( sym < 5 )
        {
            status = (sym > 0) ? XRWD_BLOCKEND : DICT_INIT;
            *pattr = (_UCHAR) (sym - 1);
        }
        else
        {
            status = XRWD_MIDWORD;
            *pattr = 0;
        }
    }
    else
    {
        if ( byte1 & END_WRD_FLAG ) //0100 0000
        {
            status = XRWD_WORDEND;
            *pattr = (_UCHAR) ((byte1 & ATTR_MASK) >> 4);
        } //0011 0000
        else
        {
            status = XRWD_MIDWORD;
            *pattr = 0;
        }
    }
    
    return (status);
}

/* *************************************************************************** */
/* *       Pass vert                                                         * */
/* *************************************************************************** */
_CHAR *pass_vert_and_find_setslen( _VOID *pVoc, _CHAR *vert, _INT *pdvset_len, _INT *pchset_len )
{
    _CHAR *p = vert;
    //     _UCHAR hbyte1,qbyte2;
    _UCHAR byte1;
    _INT dvset_num, chset_num;
    _INT chset_len;
    //     _INT dvset_len;
    _CHAR *pDvsetTabl = (_CHAR *) PYDictGetDvsetTabl(pVoc);
    _CHAR *pChsetTabl = (_CHAR *) PYDictGetChsetTabl(pVoc);
    _CHAR sym;
    _BOOL NotLast;
    
    byte1 = (_UCHAR) (*p);
    if ( byte1 & ONE_BYTE_FLAG ) //1000 0000
    {
        sym = (_CHAR) (byte1 & 0x7F);
        *pchset_len = (sym < 5) ? 0 : 1;
        *pdvset_len = 0;
        p++;
    }
    else if ( IsTreeMerged(pVoc) )
    {
        if ( !(byte1 & END_WRD_FLAG) && (byte1 & CODED_DVSET_FLAG) ) //0010 0000
        {
            if ( byte1 & SHORT_DVSET_NUM_FLAG ) //0001 0000
            {
                dvset_num = byte1 & DVSET_NUM_MASK;
                p++;
            } //0000 1111
            else
            {
                dvset_num = (byte1 & DVSET_NUM_MASK) << 8;
                p++;
                dvset_num += (_UCHAR) (*p);
                p++;
            }
            *pdvset_len = find_dvset_len( pDvsetTabl, dvset_num );
        }
        else
        {
            *pdvset_len = byte1 & DVSET_LEN_MASK;
            p++;
            p += find_dvset_size( (_UCHAR *) p, *pdvset_len );
        }
        
        if ( (*p) & CODED_CHSET_FLAG ) //1000 0000
        {
            if ( (*p) & SHORT_CHSET_NUM_FLAG ) //0100 0000
            {
                chset_num = (*p) & CHSET_NUM_MASK;
                p++;
            } //0011 1111
            else
            {
                chset_num = ((*p) & CHSET_NUM_MASK) << 8;
                p++;
                chset_num += (_UCHAR) (*p);
                p++;
            }
            *pchset_len = find_chset_len( pChsetTabl, chset_num );
            
        }
        else
        {
            NotLast = true;
            p++;
            chset_len = 1; //chset len >1 !!!
            while ( NotLast )
            {
                NotLast = (((*p) & LAST_SYM_FLAG) == 0); //1000 0000   //true for first byte!!!
                p++;
                chset_len++;
            }
            *pchset_len = chset_len;
        }
        
    }
    else //Plain Tree
    {
        
        if ( (*p) & SHORT_ECHSET_LEN_FLAG )
        {
            chset_len = (*p) & SHORT_ECHSET_LEN_MASK;
            p++;
        }
        else
        {
            chset_len = ((*p) & SHORT_ECHSET_LEN_MASK) << 8;
            p++;
            chset_len += (*p);
            p++;
        }
        
        p += chset_len;
        *pchset_len = chset_len;
        *pdvset_len = 0;
    }
    
    return (p);
}

/* *************************************************************************** */
/* *       Find Vert                                                         * */
/* *************************************************************************** */
_CHAR *find_vert( _VOID *pVoc, _INT rank, _INT num_in_layer )
{
    _CHAR *pGraph = (_CHAR *) PYDictGetGraph(pVoc);
    _INT *gheader = (_INT *) pGraph;
    _INT lshift = gheader[(rank << 1)];
    //     unsigned short *lheader=(unsigned short *)(pGraph+lshift);
    unsigned char *lheader = (unsigned char *) (pGraph + lshift);
    _CHAR *pv;
    _INT i, r, j;
    _LONG lbshift;
    
    if ( rank == 20 )
    {
        i = num_in_layer >> LHDR_STEP_LOG;
        r = (IsTreeMerged(pVoc)) ? num_in_layer & LHDR_STEP_MASK : num_in_layer;
    }
    else
    {
        i = num_in_layer >> LHDR_STEP_LOG;
        r = (IsTreeMerged(pVoc)) ? num_in_layer & LHDR_STEP_MASK : num_in_layer;
    }
    //     lbshift = (IsTreeMergedi(pVoc)) ? lheader[(i<<1)] : 0;
    lbshift = (IsTreeMerged(pVoc)) ? GetUInt(lheader+(i<<3)) : 0;
    pv = (_CHAR *) lheader + lbshift;
    for ( j = 0; j < r; j++ )
        pv = pass_vert( pVoc, pv );
    
    return (pv);
}

/* *************************************************************************** */
/* *       Find vert rank                                                    * */
/* *************************************************************************** */
_INT find_vert_rank( _VOID *pVoc, _INT nvert, _INT *pnum_in_layer )
{
    _CHAR *pGraph = (_CHAR *) PYDictGetGraph(pVoc);
    _INT two_rank = 0, *pgh = (_INT *) pGraph;
    _INT two_nranks = pgh[0] / sizeof(_INT);
    
    while ( two_rank < two_nranks - 2 && pgh[two_rank + 1] < nvert )
        two_rank += 2;
    if ( pgh[two_rank + 1] > nvert )
        two_rank -= 2;
    *pnum_in_layer = nvert - pgh[two_rank + 1];
    
    return ((two_rank >> 1));
}

/* *************************************************************************** */
/* *       Find next child                                                   * */
/* *************************************************************************** */
_INT find_next_d_child( _VOID *pVoc, _INT nv, _INT *pnum_in_layer, _UCHAR *pstatus, _UCHAR *pattr )
{
    _INT rank = find_vert_rank( pVoc, nv, pnum_in_layer );
    _CHAR *pv = find_vert( pVoc, rank, *pnum_in_layer );
    *pstatus = find_vert_status_and_attr( pv, pattr );
    
    return (rank);
}

/* *************************************************************************** */
/* *       Find child                                                        * */
/* *************************************************************************** */
_CHAR *find_d_child( _VOID *pVoc, _INT nv, _INT *prank, _INT *pnum_in_layer )
{
    *prank = find_vert_rank( pVoc, nv, pnum_in_layer );
    _CHAR *pv = find_vert( pVoc, *prank, *pnum_in_layer );
    return (pv);
}

/* *************************************************************************** */
/* *       Find first                                                        * */
/* *************************************************************************** */
_INT find_first_nd_child_num( _VOID *pVoc, _INT rank, _INT num_in_layer )
{
    _CHAR *pGraph = (_CHAR *) PYDictGetGraph(pVoc);
    _INT *gheader = (_INT *) pGraph;
    _INT lshift = gheader[(rank << 1)];
    //     unsigned short *lheader=(unsigned short *)(pGraph+lshift);
    unsigned char *lheader = (unsigned char *) (pGraph + lshift);
    _CHAR *pv;
    _INT i, r, j, dvset_len, chset_len;
    _LONG lbshift, num_prev_nd_childs;
    
    i = (num_in_layer >> LHDR_STEP_LOG) << 1; //i=2*i
    r = (IsTreeMerged(pVoc)) ? num_in_layer & LHDR_STEP_MASK : num_in_layer;
    //     lbshift = (IsTreeMerged(pVoc)) ? lheader[i] : 0;
    lbshift = (IsTreeMerged(pVoc)) ? GetUInt(lheader+(i<<2)) : 0;
    pv = (_CHAR *) lheader + lbshift;
    //     num_prev_nd_childs = (IsTreeMerged(pVoc)) ? lheader[i+1] : 0;
    num_prev_nd_childs = (IsTreeMerged(pVoc)) ? GetUInt(lheader+((i+1)<<2)) : 0;
    for ( j = 0; j < r; j++ )
    {
        pv = pass_vert_and_find_setslen( pVoc, pv, &dvset_len, &chset_len );
        num_prev_nd_childs += (chset_len - dvset_len);
    }
    
    return (_INT)(num_prev_nd_childs);
}

/* *************************************************************************** */
/* *       Find next child                                                   * */
/* *************************************************************************** */
_CHAR *find_next_nd_child( _VOID *pVoc, _INT rank, _INT num_in_layer, _CHAR *prev_ndch_vert, _INT *pndch_num_in_layer, _UCHAR *pstatus,
                          _UCHAR *pattr )
{
    _CHAR *ndch_vert;
    if ( prev_ndch_vert == _NULL )
    {
        *pndch_num_in_layer = find_first_nd_child_num( pVoc, rank, num_in_layer );
        ndch_vert = find_vert( pVoc, rank + 1, *pndch_num_in_layer );
    }
    else
    {
        (*pndch_num_in_layer)++;
        ndch_vert = pass_vert( pVoc, prev_ndch_vert );
    }
    if ( pstatus != _NULL )
        *pstatus = find_vert_status_and_attr( ndch_vert, pattr );
    return (ndch_vert);
}

/* *************************************************************************** */
/* *       Get next syms                                                     * */
/* *************************************************************************** */

_INT PYDictGetNextSyms( p_VOID cur_fw, p_VOID fwb, p_VOID pVoc )
{
    p_fw_buf_type psl = (p_fw_buf_type) fwb;
    p_fw_buf_type cfw = (p_fw_buf_type) cur_fw;
    _ULONG state, nextstate;
    _UCHAR status, attr;
    _INT rank, num_in_layer;
    _CHAR *vert;
    _CHAR chset[MAX_CHSET_LEN];
    _INT dvset[MAX_DVSET_LEN];
    _INT chsetlen, dvsetlen, num_nd_childs, num_d_childs, ind, id, i;
    _CHAR sym, *pnd_sym, *pd_sym;
    _INT dch_rank, dch_num_in_layer, ndch_num_in_layer;
    _CHAR *ndch_vert = _NULL;
    
    if ( cfw == 0 )
        state = 0;
    else
        state = cfw->state;
    rank = PYDICT_ST_GETRANK(state);
    num_in_layer = PYDICT_ST_GETNUM(state);
    
    vert = find_vert( pVoc, rank, num_in_layer );
    decode_vert( pVoc, vert, dvset, &dvsetlen, chset, &chsetlen );
    
    num_nd_childs = chsetlen - dvsetlen;
    num_d_childs = dvsetlen;
    pnd_sym = chset;
    pd_sym = chset + num_nd_childs;
    ind = 0;
    id = 0;
    i = 0;
    
    while ( ind + id < chsetlen )
    {
        if ( id >= num_d_childs || (ind < num_nd_childs && pnd_sym[ind] < pd_sym[id]) )
        {
            sym = pnd_sym[ind];
            //           TimeB= (_ULONG)GetTickCount();
            ndch_vert = find_next_nd_child( pVoc, rank, num_in_layer, ndch_vert, &ndch_num_in_layer, &status, &attr );
            //           TimeND+=( (_ULONG)GetTickCount()-TimeB);
            nextstate = PYDICT_FORMSTATE(rank+1,ndch_num_in_layer);
            ind++;
        }
        else //if (ind>=num_nd_childs || id<num_d_childs && pd_sym[id]<pnd_sym[ind])
        {
            sym = pd_sym[id];
            //           TimeB= (_ULONG)GetTickCount();
            dch_rank = find_next_d_child( pVoc, dvset[id], &dch_num_in_layer, &status, &attr );
            //           TimeD+=( (_ULONG)GetTickCount()-TimeB);
            nextstate = PYDICT_FORMSTATE(dch_rank,dch_num_in_layer);
            id++;
        }
        
        psl[i].sym = sym;
        psl[i].l_status = status;
        psl[i].attribute = attr;
        psl[i].chain_num = 0;
        psl[i].penalty = 0;
        psl[i].cdb_l_status = 0; /* Delayed status for a codebook entry */
        psl[i].codeshift = 0; /* Shift in the codebook      */
        psl[i].state = nextstate;
        
        i++;
    }
    
    return (chsetlen);
}

/* *************************************************************************** */
/* *       Decode vert                                                       * */
/* *************************************************************************** */

_CHAR *decode_vert( _VOID *pVoc, _CHAR *vert, _CHAR *chset, _INT *pchset_len, _UCHAR *pstatus )
{
    _CHAR *p = vert;
    _UCHAR byte1;
    _INT chset_len;
    
    if ( IsTreeMerged(pVoc) )
        goto err;
    
    byte1 = (_UCHAR) (*p);
    if ( byte1 & ONE_BYTE_FLAG ) //1000 0000
    {
        chset[0] = (_CHAR) (byte1 & 0x7F); //0111 1111
        if ( chset[0] < 5 )
        {
            *pchset_len = 0;
            *pstatus = (chset[0] > 0) ? XRWD_BLOCKEND : DICT_INIT;
        }
        else
        {
            *pchset_len = 1;
            *pstatus = XRWD_MIDWORD;
        }
        p++;
    }
    else
    {
        _UCHAR ShortChsetLenFlag, ShortChsetLenMask;
        _INT i;
        
        ShortChsetLenFlag = SHORT_ECHSET_LEN_FLAG;
        ShortChsetLenMask = SHORT_ECHSET_LEN_MASK;
        
        if ( (*p) & END_WRD_FLAG ) //0100 0000
            *pstatus = XRWD_WORDEND;
        else
            *pstatus = XRWD_MIDWORD;
        
        if ( (*p) & ShortChsetLenFlag )
        {
            chset_len = (*p) & ShortChsetLenMask;
            p++;
        }
        else
        {
            chset_len = ((*p) & ShortChsetLenMask) << 8;
            p++;
            chset_len += (*p);
            p++;
        }
        
        for ( i = 0; i < chset_len; i++ )
        {
            chset[i] = *p;
            p++;
        }
        *pchset_len = chset_len;
    }
    return (p);
err:
    return (_NULL);
}

/* *************************************************************************** */
/* *       Place word                                                        * */
/* *************************************************************************** */
_INT place_word_and_calc_add_mem( _VOID *pVoc, _INT rank, _INT num_in_layer, _CHAR *vert, _CHAR *chset, _UCHAR *word, _INT *padd_mem, _BOOL *pnew_word )
{
    _CHAR csym = (_CHAR) (*word);
    _CHAR *ndch_vert;
    _UCHAR status;
    _INT chsetlen, nchild, i, ndch_num_in_layer = 0, wlen;
    
    if ( decode_vert( pVoc, vert, chset, &chsetlen, &status ) == _NULL )
        goto err;
    
    if ( csym == 0 && status == XRWD_MIDWORD ) //new word-end
    {
        *pnew_word = true;
        if ( (*vert) & ONE_BYTE_FLAG )
            *padd_mem += 1;
    }
    else if ( csym != 0 )
    {
        nchild = -1;
        for ( i = 0; i < chsetlen; i++ )
        {
            if ( chset[i] == csym )
            {
                nchild = i;
                break;
            }
        }
        if ( nchild < 0 ) //new word continuation
        {
            *pnew_word = true; //mem for new child sym
            if ( (*vert) & ONE_BYTE_FLAG )
            {
                if ( status == DICT_INIT )
                {
                    if ( *word >= 128 )
                        *padd_mem += 1;
                }
                else if ( status == XRWD_BLOCKEND )
                    *padd_mem += 1;
                else
                    *padd_mem += 2;
            }
            else
            {
                if ( chsetlen == SHORT_ECHSET_LEN_MASK )
                    *padd_mem += 2;
                else
                    *padd_mem += 1;
            }
            //mem for additional nodes
            //          *padd_mem+=HWRStrLen((_CHAR *)word);
            wlen = HWRStrLen( (_CHAR *) word );
            for ( i = 0; i < wlen; i++ )
            {
                if ( word[i + 1] < 128 )
                    *padd_mem += 1; //01-19-00
                else
                    *padd_mem += 2;
            }
        }
        else //this prefix exists
        {
            ndch_vert = _NULL;
            for ( i = 0; i <= nchild; i++ )
                ndch_vert = find_next_nd_child( pVoc, rank, num_in_layer, ndch_vert, &ndch_num_in_layer, _NULL, _NULL );
            if ( place_word_and_calc_add_mem( pVoc, rank + 1, ndch_num_in_layer, ndch_vert, chset, word + 1, padd_mem, pnew_word ) == PYDICT_ERR )
                goto err;
        }
    }
    
    return (PYDICT_NOERR);
err:
    return (PYDICT_ERR);
}

/* *************************************************************************** */
/* *       Shift Block                                                       * */
/* *************************************************************************** */
_INT shift_block( _VOID *pVoc, _CHAR *block_to_shift, _INT add_len )
{
    _CHAR *shifted_block = block_to_shift + add_len;
    _CHAR *pend = (_CHAR *) PYDictGetGraph(pVoc) + PYDictGetGraphSize(pVoc);
    _INT block_len = (_INT) (pend - block_to_shift);
    _CHAR *tmp_block = (_CHAR *) HWRMemoryAlloc( block_len );
    
    if ( block_len > 0 && tmp_block == _NULL )
        goto err;
    if ( tmp_block != _NULL )
    {
        HWRMemCpy( tmp_block, block_to_shift, block_len );
        HWRMemCpy( shifted_block, tmp_block, block_len );
        HWRMemoryFree( tmp_block );
    }
    PYDictPutGraphSize(pVoc,PYDictGetGraphSize(pVoc)+add_len);
    
    return (PYDICT_NOERR);
err:
    return (PYDICT_ERR);
}

_VOID update_level_tabl( _VOID *pVoc, _INT rank, _INT shift, _INT nnew_vert )
{
    _INT *pgh = (_INT *) PYDictGetGraph(pVoc);//(pVoc->pGraph);
    _INT i;
    
    for ( i = rank + 1; i < PYDICT_MAX_WORDLEN; i++ )
    {
        pgh[2 * i] += shift;
        pgh[2 * i + 1] += nnew_vert;
    }
    return;
}

/* *************************************************************************** */
/* *       Insert new child vert                                             * */
/* *************************************************************************** */
_INT insert_new_child_vert( _VOID *pVoc, _INT rank, _INT num_in_layer, _UCHAR *word, _INT attr, _INT nch )
{
    _INT ndch_num_in_layer = find_first_nd_child_num( pVoc, rank, num_in_layer ) + nch;
    _CHAR *ndch_vert = find_vert( pVoc, rank + 1, ndch_num_in_layer );
    _INT addlen = (*word < 128) ? 1 : 2;
    
    if ( shift_block( pVoc, ndch_vert, addlen ) == PYDICT_ERR )
        goto err;
    update_level_tabl( pVoc, rank + 1, addlen, 1 );
    
    if ( *word != 0 )
    {
        if ( *word < 128 )
            *ndch_vert = (_CHAR) (ONE_BYTE_FLAG | (*word));
        else
        {
            *ndch_vert = (_CHAR) (SHORT_ECHSET_LEN_FLAG | 0x1);
            *(ndch_vert + 1) = *word;
        }
        if ( insert_new_child_vert( pVoc, rank + 1, ndch_num_in_layer, word + 1, attr, 0 ) == PYDICT_ERR )
            goto err;
    }
    else
    {
        *ndch_vert = (_CHAR) (ONE_BYTE_FLAG | (_UCHAR) (attr + 1));
    }
    return (PYDICT_NOERR);
err:
    return (PYDICT_ERR);
}

/* *************************************************************************** */
/* *       Insert word                                                       * */
/* *************************************************************************** */
_INT insert_word_in_voc( _VOID *pVoc, _INT rank, _INT num_in_layer, _CHAR *vert, _CHAR *chset, _UCHAR *word, _UCHAR attr )
{
    _CHAR csym = (_CHAR) (*word), *ndch_vert, osym, *psyms;
    _INT chsetlen, nchild, i, ndch_num_in_layer = 0;
    _UCHAR status, oattr;
    
    if ( decode_vert( pVoc, vert, chset, &chsetlen, &status ) == _NULL )
        goto err;
    
    if ( csym == 0 ) // && status==XRWD_MIDWORD)            //new end of word!!
    {
        if ( (*vert) & ONE_BYTE_FLAG )
        {
            if ( shift_block( pVoc, vert + 1, 1 ) == PYDICT_ERR )
                goto err;
            update_level_tabl( pVoc, rank, 1, 0 );
            
            osym = (_CHAR) ((*vert) & 0x7F); //sym in one-byte node is < 128
            *vert = (_CHAR) (END_WRD_FLAG | (attr << 4) | SHORT_ECHSET_LEN_FLAG | 0x1);
            *(vert + 1) = osym;
        }
        else
            *vert |= (_CHAR) (END_WRD_FLAG | (attr << 4));
    }
    else //csym!=0
    {
        nchild = -1;
        for ( i = 0; i < chsetlen; i++ )
        {
            if ( chset[i] == csym )
            {
                nchild = i;
                break;
            }
        }
        if ( nchild < 0 ) //new child-sym!!
        {
            if ( (*vert) & ONE_BYTE_FLAG )
            {
                if ( status == DICT_INIT )
                {
                    if ( *word < 128 )
                        *vert = (_CHAR) (ONE_BYTE_FLAG | (*word));
                    else
                    {
                        if ( shift_block( pVoc, vert + 1, 1 ) == PYDICT_ERR )
                            goto err; //shift_len=1;
                        update_level_tabl( pVoc, rank, 1, 0 );
                        *vert = (_CHAR) (SHORT_ECHSET_LEN_FLAG | 0x1);
                        *(vert + 1) = *word;
                    }
                    nchild = 0;
                }
                else if ( status == XRWD_BLOCKEND )
                {
                    if ( shift_block( pVoc, vert + 1, 1 ) == PYDICT_ERR )
                        goto err; //shift_len=1;
                    update_level_tabl( pVoc, rank, 1, 0 );
                    
                    oattr = (_UCHAR) (((*vert) & 0x7F) - 1);
                    *vert = (_CHAR) (END_WRD_FLAG | (oattr << 4) | SHORT_ECHSET_LEN_FLAG | 0x1); //chset_len=1;
                    *(vert + 1) = csym;
                    nchild = 0;
                }
                else //if (status==XRWD_MIDWORD)
                {
                    if ( shift_block( pVoc, vert + 1, 2 ) == PYDICT_ERR )
                        goto err; //shift_len=2;
                    update_level_tabl( pVoc, rank, 2, 0 );
                    
                    osym = (_CHAR) ((*vert) & 0x7F); //sym in one-byte node is < 128
                    *vert = SHORT_ECHSET_LEN_FLAG | 0x2; //chset_len=2;
                    if ( (_UCHAR) csym < (_UCHAR) osym )
                    {
                        *(vert + 1) = csym;
                        *(vert + 2) = osym;
                        nchild = 0;
                    }
                    else
                    {
                        *(vert + 1) = osym;
                        *(vert + 2) = csym;
                        nchild = 1;
                    }
                }
            }
            else //if !ONE_BYTE_FLAG
            {
                if ( chsetlen < SHORT_ECHSET_LEN_MASK )
                {
                    *vert = (_UCHAR) ((*vert) & 0xF8) | (_UCHAR) (chsetlen + 1);
                    psyms = vert + 1;
                }
                else if ( chsetlen == SHORT_ECHSET_LEN_MASK )
                {
                    if ( shift_block( pVoc, vert + 1, 1 ) == PYDICT_ERR )
                        goto err; //shift_len=1;
                    update_level_tabl( pVoc, rank, 1, 0 );
                    
                    *vert = (_UCHAR) ((*vert) & 0xF0) | (_UCHAR) ((chsetlen + 1) >> 8);
                    *(vert + 1) = (_UCHAR) ((chsetlen + 1) & 0xFF);
                    psyms = vert + 2;
                }
                else
                {
                    *vert = (_UCHAR) ((*vert) & 0xF0) | (_UCHAR) ((chsetlen + 1) >> 8);
                    *(vert + 1) = (_UCHAR) ((chsetlen + 1) & 0xFF);
                    psyms = vert + 2;
                }
                
                i = 0;
                while ( i < chsetlen && (_UCHAR) chset[i] < (_UCHAR) csym )
                    i++;
                nchild = i;
                if ( shift_block( pVoc, psyms + nchild, 1 ) == PYDICT_ERR )
                    goto err;
                update_level_tabl( pVoc, rank, 1, 0 );
                
                *(psyms + nchild) = csym;
            }
            
            if ( insert_new_child_vert( pVoc, rank, num_in_layer, word + 1, attr, nchild ) == PYDICT_ERR )
                goto err;
        }
        else //this prefix exists!!
        {
            ndch_vert = _NULL;
            for ( i = 0; i <= nchild; i++ )
                ndch_vert = find_next_nd_child( pVoc, rank, num_in_layer, ndch_vert, &ndch_num_in_layer, _NULL, _NULL );
            if ( insert_word_in_voc( pVoc, rank + 1, ndch_num_in_layer, ndch_vert, chset, word + 1, attr ) == PYDICT_ERR )
                goto err;
        }
        
    } // csym!=0
    
    return (PYDICT_NOERR);
err:
    return (PYDICT_ERR);
}

/* *************************************************************************** */
/* *        Add dict                                                         * */
/* *************************************************************************** */

_INT PYDictAddWord( p_UCHAR word, _UCHAR attr, p_VOID _PTR pd )
{
    _VOID *pVoc = (*pd);
    _VOID *npVoc;
    _INT add_mem = 0, voc_size;
    _CHAR *vert, chset[256];
    _BOOL new_word = false;
    
    int len;
    
    len = HWRStrLen( (_STR) word );
    if ( len == 0 || len >= PYDICT_MAX_WORDLEN )
        goto err;
    if ( attr > 3 )
        attr = 3;
    
    if ( IsTreeMerged(pVoc) )
        goto err;
    
    vert = find_vert( pVoc, 0, 0 );
    if ( place_word_and_calc_add_mem( pVoc, 0, 0, vert, chset, word, &add_mem, &new_word ) == PYDICT_ERR )
        goto err;
    
    if ( add_mem > 0 )
    {
        voc_size = PYDictGetVocHeaderSize(pVoc) + PYDictGetGraphSize(pVoc);
        if ( (npVoc = (_VOID *) HWRMemoryAlloc( voc_size + add_mem )) == _NULL )
            goto err;
        HWRMemCpy( npVoc, pVoc, voc_size );
        HWRMemoryFree( pVoc );
        *pd = npVoc;
    }
    
    if ( new_word )
    {
        vert = find_vert( *pd, 0, 0 );
        if ( insert_word_in_voc( *pd, 0, 0, vert, chset, word, attr ) == PYDICT_ERR )
            goto err;
        PutVocIsChanged(*pd);
    }
    
    return new_word ? PYDICT_NOERR : PYDICT_OLDWORD;
err:
    return PYDICT_ERR;
}

/* *************************************************************************** */
/* *      Createdict                                                         * */
/* *************************************************************************** */
_INT PYDictCreateDict( p_VOID _PTR pd )
{
    _VOID *pVoc;
    _UCHAR TreeMerge = 0;
    _INT ghsize = 2 * sizeof(_INT) * PYDICT_MAX_WORDLEN;
    _INT gsize = ghsize + 1;
    _INT vhsize = PYDictHeaderSize(TreeMerge);
    _INT vsize = vhsize + gsize;
    _INT *pgh, i;
    _CHAR *pvert;
    
    if ( (pVoc = (_VOID *) HWRMemoryAlloc( vsize )) == _NULL )
        goto err;
    
    PutPYDictID(pVoc);
    PutTreeMerge(pVoc,TreeMerge);
    PYDictPutGraphSize(pVoc,gsize);
    
    pgh = (_INT *) PYDictGetGraph(pVoc);
    pgh[0] = ghsize;
    pgh[1] = 0;
    for ( i = 1; i < PYDICT_MAX_WORDLEN; i++ )
    {
        pgh[2 * i] = ghsize + 1;
        pgh[2 * i + 1] = 1;
    }
    pvert = (_CHAR *) PYDictGetGraph(pVoc) + ghsize;
    *pvert = (_CHAR) ONE_BYTE_FLAG;
    
    *pd = pVoc;
    
    return PYDICT_NOERR;
err:
    if ( pVoc != _NULL )
        HWRMemoryFree( pVoc );
    return PYDICT_ERR;
}

/* *************************************************************************** */
/* *       Free dict                                                         * */
/* *************************************************************************** */

_INT PYDictFreeDict( p_VOID _PTR pd )
{
    if ( pd == _NULL )
        goto err;
    
    if ( *pd != _NULL )
        HWRMemoryFree( *pd );
    *pd = _NULL;
    
    return PYDICT_NOERR;
err:
    return PYDICT_ERR;
}

// OLD Dictionary Header
#define PYDICT_ID_STRING		"PLK dict v.1.10."
#define PLAIN_TREE_ID			"NB: PLAIN TREE  "
#define MERGED_TREE_ID			"NB: MERGED TREE "


/* *************************************************************************** */
/* *       Load dict                                                         * */
/* *************************************************************************** */

_INT PYDictLoadDictFromFile(_UCHAR *name, p_VOID _PTR pd)
{
    _VOID * pVoc;
    FILE *  voc_file=_NULL;
    _UCHAR  IsMerged;
    _INT    vhsize,gsize,dvsize,chsize,vsize;

    SRecDictFileHeader header = {0};
    
    if ( pd == _NULL )
        goto err;
    if ( (voc_file = fopen((_STR)name,"rb")) == _NULL )
        goto err;
    
    if ( fread( &header,sizeof(header),1,voc_file ) !=1 )
		goto err;
    
    if ( header.usDataOffset != sizeof(header) )
    {
        _CHAR   ids[64];

        fseek( voc_file, 0, SEEK_SET );
        if ( fread( ids, PYDICT_ID_LEN, 1, voc_file ) != 1 )
            goto err;
        ids[PYDICT_ID_LEN]=0;
        if ( HWRStrCmp( ids, PYDICT_ID_STRING) != 0 )
            goto err;
        
        if (fread(ids,PYDICT_TREE_ID_LEN,1,voc_file)!=1)
            goto err;
        ids[PYDICT_TREE_ID_LEN]=0;
        if (HWRStrCmp(ids,PLAIN_TREE_ID)==0)
            IsMerged=0;
        else if (HWRStrCmp(ids,MERGED_TREE_ID)==0)
            IsMerged=1;
        else
            goto err;
    }
    else
    {
        IsMerged = header.bType;
    }
    
    if (fread(&gsize,sizeof(_INT),1,voc_file)!=1)
        goto err;
    if (IsMerged)
    {
        if (fread(&chsize,sizeof(_INT),1,voc_file)!=1)
            goto err;
        if (fread(&dvsize,sizeof(_INT),1,voc_file)!=1)
            goto err;
    }
    else
    {
        chsize=0;
        dvsize=0;
    }
    vhsize = PYDictHeaderSize(IsMerged);
    vsize = vhsize+gsize+chsize+dvsize;
    
    if ((*pd=HWRMemoryAlloc(vsize))==_NULL)
        goto err;
    pVoc=*pd;
    
    PutPYDictID(pVoc);
    PutTreeMerge(pVoc,IsMerged);
    PYDictPutGraphSize(pVoc,gsize);
    if (IsMerged)
    {
        PYDictPutChsetTablSize(pVoc,chsize);
        PYDictPutDvsetTablSize(pVoc,dvsize);
    }
    
    if (fread(PYDictGetGraph(pVoc),gsize+chsize+dvsize,1,voc_file)!=1)
        goto err;
    if (voc_file!=_NULL)
        fclose(voc_file);
    return(PYDICT_NOERR);
err:
    if (voc_file!=_NULL)
        fclose(voc_file);
    if (pd!=_NULL)
        PYDictFreeDict(pd);
    return(PYDICT_ERR);
}

_INT PYDictLoadDict( p_UCHAR store, p_VOID _PTR pd )
{
    _VOID * pVoc;
    p_UCHAR p;
    _UCHAR  IsMerged;
    _INT    gsize, chsize, dvsize, vhsize, vsize;
    
    if ( pd == _NULL || store == _NULL )
        goto err;
    
    p = store;
    
    SRecDictFileHeader header;
    HWRMemCpy( &header, p, sizeof(header) );
    
    if ( header.usDataOffset != sizeof(header) )
    {
        // try old dictionary header
        _CHAR ids[64];
        HWRMemCpy( ids, p, PYDICT_ID_LEN );
        ids[PYDICT_ID_LEN] = 0;
        p += PYDICT_ID_LEN;
        
        if ( HWRStrCmp( ids, PYDICT_ID_STRING ) != 0 )
            goto err;
        
        HWRMemCpy( ids, p, PYDICT_TREE_ID_LEN);
        ids[PYDICT_TREE_ID_LEN] = 0;
        p += PYDICT_TREE_ID_LEN;
        if ( HWRStrCmp( ids, PLAIN_TREE_ID ) == 0 )
            IsMerged = 0;
        else if ( HWRStrCmp( ids, MERGED_TREE_ID ) == 0 )
            IsMerged = 1;
        else
        {
            goto err;
        }
    }
    else
    {
        IsMerged = header.bType;
        p += sizeof(header);
    }
    
    HWRMemCpy( &gsize, p, sizeof(_INT) );
    p += sizeof(_INT);
    
    if ( IsMerged )
    {
        HWRMemCpy( &chsize, p, sizeof(_INT) );
        p += sizeof(_INT);
        HWRMemCpy( &dvsize, p, sizeof(_INT) );
        p += sizeof(_INT);
    }
    else
    {
        chsize = 0;
        dvsize = 0;
    }
    
    vhsize = PYDictHeaderSize( IsMerged );
    vsize = vhsize + gsize + chsize + dvsize;
    
    if ( (*pd = HWRMemoryAlloc( vsize )) == _NULL )
        goto err;
    pVoc = *pd;
    
    PutPYDictID( pVoc );
    PutTreeMerge( pVoc, IsMerged );
    PYDictPutGraphSize( pVoc, gsize );
    
    if ( IsMerged )
    {
        PYDictPutChsetTablSize( pVoc, chsize );
        PYDictPutDvsetTablSize( pVoc, dvsize );
    }
    
    
    HWRMemCpy( PYDictGetGraph( pVoc ), p, gsize + chsize + dvsize );
    return (PYDICT_NOERR);
err:
    
    if ( pd != _NULL )
        PYDictFreeDict( pd );
    return (PYDICT_ERR);
}

/* *************************************************************************** */
/* *       Save dict                                                         * */
/* *************************************************************************** */

_INT PYDictSaveDictToFile( _UCHAR *name, p_VOID pVoc )
{
    FILE *  voc_file=_NULL;
    SRecDictFileHeader fileHead =
    {
        "RCDT", 0, 0, 0
    };
    
    if (pVoc==_NULL)
        goto err;
    if ((voc_file=fopen((_STR)name,"wb"))==_NULL)
        goto err;
    
    fileHead.bType = IsTreeMerged(pVoc) ? 1 : 0;
    fileHead.usDataOffset = sizeof(fileHead);
    fileHead.dwDataSize = PYDictGetGraphSize(pVoc);
    if ( fwrite(&fileHead, sizeof(fileHead), 1, voc_file ) != 1 )
        goto err;
    
    if (fwrite(&PYDictGetGraphSize(pVoc),sizeof(_INT),1,voc_file)!=1)
        goto err;
    if (IsTreeMerged(pVoc))
    {
        if (fwrite(&PYDictGetChsetTablSize(pVoc),sizeof(_INT),1,voc_file)!=1)
            goto err;
        if (fwrite(&PYDictGetDvsetTablSize(pVoc),sizeof(_INT),1,voc_file)!=1)
            goto err;
    }
    if (fwrite(PYDictGetGraph(pVoc),PYDictGetGraphSize(pVoc),1,voc_file)!=1)
        goto err;
    if (IsTreeMerged(pVoc))
    {
        if (fwrite(PYDictGetChsetTabl(pVoc),PYDictGetChsetTablSize(pVoc),1,voc_file)!=1)
            goto err;
        if (fwrite(PYDictGetDvsetTabl(pVoc),PYDictGetDvsetTablSize(pVoc),1,voc_file)!=1)
            goto err;
    }
    
    if (voc_file!=_NULL)
        fclose(voc_file);
    return(PYDICT_NOERR);
err:
    if (voc_file!=_NULL)
        fclose(voc_file);
    return(PYDICT_ERR);
}

_INT PYDictSaveDict( p_UCHAR store, p_VOID pVoc )
{
    p_UCHAR p;
    SRecDictFileHeader fileHead =
    {
        "RCDT", 0, 0, 0
    };
    
    if ( pVoc == _NULL )
        goto err;
    
    p = store;
    
    fileHead.bType = IsTreeMerged(pVoc) ? 1 : 0;
    fileHead.usDataOffset = sizeof(fileHead);
    fileHead.dwDataSize = PYDictGetGraphSize(pVoc);
    HWRMemCpy( p, &fileHead, sizeof(fileHead) );
    p += sizeof(fileHead);
    
    HWRMemCpy( p, &PYDictGetGraphSize(pVoc), sizeof(_INT) );
    p += sizeof(_INT);
    
    if ( IsTreeMerged(pVoc) )
    {
        HWRMemCpy( p, &PYDictGetChsetTablSize(pVoc), sizeof(_INT) );
        p += sizeof(_INT);
        HWRMemCpy( p, &PYDictGetDvsetTablSize(pVoc), sizeof(_INT) );
        p += sizeof(_INT);
    }
    
    HWRMemCpy( p, PYDictGetGraph(pVoc), PYDictGetGraphSize(pVoc) );
    p += PYDictGetGraphSize(pVoc);
    if ( IsTreeMerged(pVoc) )
    {
        HWRMemCpy( p, PYDictGetChsetTabl(pVoc), PYDictGetChsetTablSize(pVoc) );
        p += PYDictGetChsetTablSize(pVoc);
        HWRMemCpy( p, PYDictGetDvsetTabl(pVoc), PYDictGetDvsetTablSize(pVoc) );
        //      p+=PYDictGetDvsetTablSize(pVoc);
    }
    return (PYDICT_NOERR);
err:
    return (PYDICT_ERR);
}

/* *************************************************************************** */
/* *       Check next letter                                                 * */
/* *************************************************************************** */

#define PYDICT_WORD_PRESENT 1
#define PYDICT_NO_WORD      0

_INT PYDictCheckNextLet( _VOID *pVoc, _INT rank, _INT num_in_layer, _CHAR *vert, _UCHAR *word, _UCHAR *status, _UCHAR *attr )
{
    _CHAR   sym = *word;
    _UCHAR  cur_status;
    _INT    chsetlen, dvsetlen;
    _INT    dvset[128];
    _CHAR   chset[128];
    _INT    i, nchild, num_nd_childs, next_num_in_layer = 0, next_rank;
    _CHAR * next_vert;
    _INT    retval;
    
    cur_status = find_vert_status_and_attr( vert, attr );
    
    if ( sym == 0 )
    {
        if ( cur_status == XRWD_BLOCKEND || cur_status == XRWD_WORDEND )
            goto WORD_PRESENT;
        else
            goto NO_WORD;
    }
    
    decode_vert( pVoc, vert, dvset, &dvsetlen, chset, &chsetlen );
    
    nchild = -1;
    for ( i = 0; i < chsetlen; i++ )
    {
        if ( chset[i] == sym )
            nchild = i;
    }
    if ( nchild < 0 )
        goto NO_WORD;
    
    num_nd_childs = chsetlen - dvsetlen;
    if ( nchild < num_nd_childs )
    {
        next_vert = _NULL;
        for ( i = 0; i <= nchild; i++ )
            next_vert = find_next_nd_child( pVoc, rank, num_in_layer, next_vert, &next_num_in_layer, _NULL, _NULL );
        retval = PYDictCheckNextLet( pVoc, rank + 1, next_num_in_layer, next_vert, word + 1, status, attr );
    }
    else
    {
        next_vert = find_d_child( pVoc, dvset[nchild - num_nd_childs], &next_rank, &next_num_in_layer );
        retval = PYDictCheckNextLet( pVoc, next_rank, next_num_in_layer, next_vert, word + 1, status, attr );
    }
    return (retval);
    
WORD_PRESENT:
    *status = cur_status;
    return (PYDICT_WORD_PRESENT);
    
NO_WORD:
    *status = XRWD_NOP;
    *attr = 0;
    return (PYDICT_NO_WORD);
}

/* *************************************************************************** */
/* *       Check word                                                        * */
/* *************************************************************************** */

_INT PYDictCheckWord( _UCHAR *word, _UCHAR *status, _UCHAR *attr, p_VOID pVoc )
{
    _CHAR *vert = find_vert( pVoc, 0, 0 );
    
    _INT e = PYDictCheckNextLet( pVoc, 0, 0, vert, word, status, attr );
    
    if ( e == PYDICT_WORD_PRESENT )
        return (PYDICT_NOERR);
    else
        return (PYDICT_ERR);
}

/* *************************************************************************** */
/* *       Get size of dictionary file                                       * */
/* *************************************************************************** */

_INT PYDictGetDictStatus( p_INT plen, p_VOID pVoc )
{
    
    *plen = (IsTreeMerged(pVoc)) ? PYDICT_ID_LEN + PYDICT_TREE_ID_LEN + 3 * sizeof(_INT) + PYDictGetGraphSize(pVoc)
    + PYDictGetChsetTablSize(pVoc) + PYDictGetDvsetTablSize(pVoc) : PYDICT_ID_LEN + PYDICT_TREE_ID_LEN + sizeof(_INT)
    +PYDictGetGraphSize(pVoc);
    
    return (IsVocChanged(pVoc));
}

/* *************************************************************************** */
/* *       Get size of mem for dictionary                                    * */
/* *************************************************************************** */

_INT PYDictGetDictMemSize( p_VOID pVoc )
{
    _INT size;
    
    size = (IsTreeMerged(pVoc)) ? PYDictHeaderSize(1) + PYDictGetGraphSize(pVoc) + PYDictGetChsetTablSize(pVoc)
    +PYDictGetDvsetTablSize(pVoc) : PYDictHeaderSize(0) + PYDictGetGraphSize(pVoc);
    
    return (size);
}

/* *************************************************************************** */
/* *       Get dictionary type -- compressed mode                            * */
/* *************************************************************************** */

_INT PYDictGetDictType( p_VOID pVoc )
{
    return (IsTreeMerged(pVoc));
}

