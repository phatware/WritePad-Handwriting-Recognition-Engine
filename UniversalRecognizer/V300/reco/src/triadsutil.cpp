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
#include "hwr_swap.h"

#include "ams_mg.h"
#include "triads.h"
#include "langid.h"

#if !LOAD_RESOURCES_FROM_ROM
#include <stdio.h>

RECO_DATA _UCHAR triads_mapping_ge[256] = {                        \
    /*   0 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   1 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   2 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   3 */                   0,  0,  1,  0,  0,  0,  0,  0,  0, 40, \
    /*   4 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   5 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   6 */                   0,  0,  0,  0,  0,  2,  3,  4,  5,  6, \
    /*   7 */                   7, 8,  9,  10, 11, 12, 13, 14, 15, 16, \
    /*   8 */                  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, \
    /*   9 */                  27,  0,  0,  0,  0,  0,  0,  2,  3,  4, \
    /*  10 */                   5,  6,  7,  8,  9, 10, 11, 12, 13, 14, \
    /*  11 */                  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, \
    /*  12 */                  25, 26, 27,  0,  0,  0,  0,  0,  0,  0, \
    /*  13 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  14 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  15 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  16 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  17 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  18 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  19 */                   0,  0,  0,  0,  0,  0, 29,  0,  0,  0, \
    /*  20 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  21 */                   0,  0,  0,  0, 30,  0,  0,  0,  0,  0, \
    /*  22 */                  31,  0,  0, 32,  0,  0,  0,  0, 29,  0, \
    /*  23 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  24 */                   0,  0,  0,  0,  0,  0, 30,  0,  0,  0, \
    /*  25 */                   0,  0, 31,  0,  0,  0                  \
};

//Ant: ?WHAT? #error Check coding -- it is MAC now!
RECO_DATA _UCHAR triads_mapping_da_no_se[256] = {                  \
    /*   0 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   1 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   2 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   3 */                   0,  0,  1,  0,  0,  0,  0,  0,  0, 40, \
    /*   4 */                   0,  0,  0,  0,  0,  0, 99, 99,  0,  0, \
    /*   5 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   6 */                   0,  0,  0,  0, 99,  2,  3,  4,  5,  6, \
    /*   7 */                   7,  8,  9, 10, 11, 12, 13, 14, 15, 16, \
    /*   8 */                  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, \
    /*   9 */                  27,  0, 99,  0,  0,  0,  0,  2,  3,  4, \
    /*  10 */                   5,  6,  7,  8,  9, 10, 11, 12, 13, 14, \
    /*  11 */                  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, \
    /*  12 */                  25, 26, 27,  0,  0,  0,  0,  0, 29, 28, \
    /*  13 */                  30, 31, 15, 33, 34,  2, 29, 35, 29,  2, \
    /*  14 */                  28, 30, 31, 36, 37,  6, 10, 10, 38, 32, \
    /*  15 */                  15, 16, 16, 33, 33, 16, 22, 34, 39, 34, \
    /*  16 */                   0,  0,  0,  0,  0,  0,  0, 20,  0,  0, \
    /*  17 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  18 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  19 */                   0,  0,  0,  0,  0,  0,  0,  0,  2,  0, \
    /*  20 */                   0,  0,  0, 29,  2, 16,  0,  0,  0,  0, \
    /*  21 */                   0,  0,  0,  0,  16,  0, 26, 26,  0,  0, \
    /*  22 */                   0,  0,  0,  0,  0,  0,  0,  0,  0, 35, \
    /*  23 */                   2,  2,  6, 36,  9, 38, 32,  9, 16, 33, \
    /*  24 */                   0, 16, 22, 39, 34,  0,  16,  0, 16,  0, \
    /*  25 */                   0,  0,  0,  0,  0,  0                  \
};

RECO_DATA _UCHAR triads_mapping_it_fr_po[256] = {                  \
    /*   0 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   1 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   2 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   3 */                   0,  0,  1,  0,  0,  0,  0,  0,  0, 40, \
    /*   4 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   5 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   6 */                   0,  0,  0,  0,  0,  2,  3,  4,  5,  6, \
    /*   7 */                   7,  8,  9, 10, 11, 12, 13, 14, 15, 16, \
    /*   8 */                  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, \
    /*   9 */                  27,  0,  0,  0,  0,  0,  0,  2,  3,  4, \
    /*  10 */                   5,  6,  7,  8,  9, 10, 11, 12, 13, 14, \
    /*  11 */                  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, \
    /*  12 */                  25, 26, 27,  0,  0,  0,  0,  0,  0,  0, \
    /*  13 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  14 */                  41,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  15 */                   0,  0,  0,  0,  0,  0, 41,  0,  0,  0, \
    /*  16 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  17 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  18 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  19 */                   0,  0, 29,  0, 35,  0,  0,  0,  0, 30, \
    /*  20 */                  36, 31, 37, 28,  0,  0, 38, 32,  0,  0, \
    /*  21 */                   0,  0, 33,  0,  0,  0,  0, 34,  0, 39, \
    /*  22 */                   0,  0,  0,  0,  0,  0, 35,  0,  0,  0, \
    /*  23 */                   0, 30, 36, 31, 37, 28,  0,  0, 38, 32, \
    /*  24 */                   0,  0,  0, 29, 33,  0,  0,  0,  0, 34, \
    /*  25 */                   0, 39,  0,  0,  0,  0                  \
};

RECO_DATA _UCHAR triads_mapping_default[256] = {                   \
    /*   0 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   1 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   2 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   3 */                   0,  0,  1,  0,  0,  0,  0,  0,  0, 40, \
    /*   4 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   5 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*   6 */                   0,  0,  0,  0,  0,  2,  3,  4,  5,  6, \
    /*   7 */                   7, 8,  9,  10, 11, 12, 13, 14, 15, 16, \
    /*   8 */                  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, \
    /*   9 */                  27,  0,  0,  0,  0,  0,  0,  2,  3,  4, \
    /*  10 */                   5,  6,  7,  8,  9, 10, 11, 12, 13, 14, \
    /*  11 */                  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, \
    /*  12 */                  25, 26, 27,  0,  0,  0,  0,  0,  0,  0, \
    /*  13 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  14 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  15 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  16 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  17 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  18 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  19 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  20 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  21 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  22 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  23 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  24 */                   0,  0,  0,  0,  0,  0,  0,  0,  0,  0, \
    /*  25 */                   0,  0,  0,  0,  0,  0                  \
};

/* ************************************************************************* */
/* *   Triads load                                                         * */
/* ************************************************************************* */

_INT triads_load(p_CHAR triadsname, _INT what_to_load, p_VOID _PTR tp)
{
    FILE *               tr_file = _NULL;
    tr_header_type       tr_header;
    p_tr_descr_type      tr_descr = _NULL;
    *tp = _NULL;
    
    /* ----------- Dig in file header -------------------------------------- */
    
    tr_descr = (p_tr_descr_type)HWRMemoryAlloc(sizeof(tr_descr_type));
    
    if (tr_descr == _NULL)
        goto err;
    HWRMemSet(tr_descr, 0, sizeof(*tr_descr));
    
    if ((tr_file = fopen(triadsname, "rb")) == _NULL)
        goto err;
    if (fread(&tr_header, 1, sizeof(tr_header), tr_file) != sizeof(tr_header))
        goto err;
    
    HWRSwapLong(&tr_header.tr_offset);
    HWRSwapLong(&tr_header.tr_len);
    HWRSwapLong(&tr_header.tr_chsum);
    
    if (HWRStrnCmp(tr_header.object_type, TR_OBJTYPE, TR_ID_LEN) != 0)
        goto err;
    if ((what_to_load & TR_REQUEST) && tr_header.tr_offset == 0l)
        goto err;
    
    /* ----------- Begin load operations ----------------------------------- */
    
    HWRMemCpy(tr_descr->object_type, tr_header.object_type, TR_ID_LEN);
    HWRMemCpy(tr_descr->type, tr_header.type, TR_ID_LEN);
    HWRMemCpy(tr_descr->version, tr_header.version, TR_ID_LEN);
    HWRStrnCpy(tr_descr->tr_fname, triadsname, TR_FNAME_LEN-1);
    
    /* ----------- Beg Triads load operations ------------------------------------- */
    
    if (what_to_load & TR_REQUEST)
    {
        _ULONG  tr_i, tr_chsum;
        
        tr_descr->h_tr = HWRMemoryAllocHandle(tr_header.tr_len);
        if (tr_descr->h_tr == _NULL)
            goto err;
        tr_descr->p_tr = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)tr_descr->h_tr);
        if (tr_descr->p_tr == _NULL)
            goto err;
        if (fseek(tr_file, tr_header.tr_offset, SEEK_SET) != 0)
            goto err;
        if (fread(tr_descr->p_tr, 1, (_UINT)tr_header.tr_len, tr_file) != tr_header.tr_len)
            goto err;
        
        for (tr_i = 0l, tr_chsum = 0l; tr_i < tr_header.tr_len; tr_i ++)
        {
            tr_chsum += (_UCHAR)(*(tr_descr->p_tr + (_UINT)tr_i));
        }
        
        if (tr_chsum != tr_header.tr_chsum)
            goto err;
        
        tr_descr->p_tr = _NULL;
    }
    
    /* ----------- Closing down -------------------------------------------- */
    fclose(tr_file); tr_file = _NULL;
    *tp = (p_VOID _PTR)tr_descr;
    
    return 0;
    
err:
    if (tr_file)
        fclose(tr_file);
    if (tr_descr)
    {
        if (tr_descr->h_tr)
            HWRMemoryFreeHandle((_HANDLE)tr_descr->h_tr);
        HWRMemoryFree(tr_descr);
    }
    return 1;
}

/* ************************************************************************* */
/*      Unload dat ingredients                                               */
/* ************************************************************************* */

_INT triads_unload(p_VOID _PTR tp)
{
    p_tr_descr_type ptr = (p_tr_descr_type)(*tp);
    
    if (ptr != _NULL)
    {
        triads_unlock((p_VOID)ptr);
        
#if !LOAD_RESOURCES_FROM_ROM
        if (ptr->h_tr)
            HWRMemoryFreeHandle((_HANDLE)ptr->h_tr);
#endif /* #if !LOAD_RESOURCES_FROM_ROM */
        
        HWRMemoryFree(*tp);
        *tp = _NULL;
    }
    
    return 0;
}

/* ************************************************************************* */
/* *   Writes  Triads to a disk file                                       * */
/* ************************************************************************* */

_INT triads_save(p_CHAR fname, _INT what_to_save, p_VOID tp)
{
    
    UNUSED(fname);
    UNUSED(what_to_save);
    UNUSED(tp);
    
    /* Now it is empty ?! */
    /* Until somebody will need it ... */
    
    return 0;
}

/* ************************************************************************* */
/* *   Lock Triads                                                         * */
/* ************************************************************************* */

_INT triads_lock(p_VOID tr_ptr)
{
    p_tr_descr_type tp = (p_tr_descr_type)(tr_ptr);
    
    if (tp == _NULL)
        goto err;
    
    if (tp->p_tr == _NULL && tp->h_tr != _NULL)
    {
        tp->p_tr = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)tp->h_tr);
        if (tp->p_tr == _NULL)
            goto err;
    }
    
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/* *   UnLock DTI                                                          * */
/* ************************************************************************* */

_INT triads_unlock(p_VOID tr_ptr)
{
    p_tr_descr_type tp = (p_tr_descr_type)(tr_ptr);
    
    if (tp == _NULL)
        goto err;
    
    if (tp->p_tr != _NULL && tp->h_tr != _NULL)
    {
        tp->p_tr = _NULL;
    }
    
    return 0;
err:
    return 1;
}

#endif /* #if !LOAD_RESOURCES_FROM_ROM */

_UCHAR triads_get_mapping(int lang, int index)
{
    switch(lang)
    {
        case LANGUAGE_GERMAN:
            return triads_mapping_ge[index];
        case LANGUAGE_DANISH:
        case LANGUAGE_NORWEGIAN:
        case LANGUAGE_SWEDISH:
        case LANGUAGE_FINNISH:
            return triads_mapping_da_no_se[index];
        case LANGUAGE_FRENCH:
        case LANGUAGE_ITALIAN:
        case LANGUAGE_PORTUGUESE:
            return triads_mapping_it_fr_po[index];
        default:
            return triads_mapping_default[index];
    }
}
