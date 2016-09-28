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
#include "zctype.h"

#include "ams_mg.h"
#include "dti.h"
#include "xr_names.h"

#include "reco.h"
#include "langid.h"

#if USE_POSTPROC
#include "pdf_file.h"
#endif

#ifndef LSTRIP

#if !DTI_LOAD_FROM_ROM
#include <stdio.h>
#endif


/* ************************************************************************* */
/* *   DTI load                                                            * */
/* ************************************************************************* */

_INT dti_load(p_CHAR dtiname, _INT what_to_load, _VOID _PTR _PTR dp, _INT lang )
{
    _INT                  i;
    p_dti_descr_type      dti_descr;
    p_dte_sym_header_type sym_descr;
#if !DTI_LOAD_FROM_ROM
    dti_header_type       dti_header;
    FILE *                dti_file = _NULL;
#endif  //     #if !DTI_LOAD_FROM_ROM
    
#if DTI_COMPRESSED
    _INT                  j, k;
    p_dte_index_type      dte_index;
#else
    let_table_type _PTR   sym_table;
#endif
    
    *dp = _NULL;
    
    /* ----------- Dig in file header -------------------------------------- */
    
    dti_descr = (p_dti_descr_type)HWRMemoryAlloc(sizeof(dti_descr_type));
    
    if (dti_descr == _NULL)
		goto err;
    HWRMemSet(dti_descr, 0, sizeof(*dti_descr));
    
#if !(DTI_LOAD_FROM_ROM)
    if ((dti_file = fopen(dtiname, "rb")) == _NULL)
		goto err;
    
    if (fread(&dti_header, 1, sizeof(dti_header), dti_file) != sizeof(dti_header)) 
		goto err;
    
    HWRSwapLong(&dti_header.dte_offset);
    HWRSwapLong(&dti_header.dte_len);
    HWRSwapLong(&dti_header.dte_chsum);
    
    HWRSwapLong(&dti_header.xrt_offset);
    HWRSwapLong(&dti_header.xrt_len);
    HWRSwapLong(&dti_header.xrt_chsum);
    HWRSwapLong(&dti_header.pdf_offset);
    HWRSwapLong(&dti_header.pdf_len);
    HWRSwapLong(&dti_header.pdf_chsum);
    
    HWRSwapLong(&dti_header.pict_offset);
    HWRSwapLong(&dti_header.pict_len);
    HWRSwapLong(&dti_header.pict_chsum);
    
    if (HWRStrnCmp(dti_header.object_type, DTI_DTI_OBJTYPE, DTI_ID_LEN) != 0) 
		goto err;
    if (HWRStrnCmp(dti_header.version, DTI_DTI_VER, DTI_ID_LEN/2) != 0)
		goto err;
    if ((what_to_load & DTI_DTE_REQUEST) && dti_header.dte_offset == 0l)
		goto err;
    if ((what_to_load & DTI_XRT_REQUEST) && dti_header.xrt_offset == 0l)
		goto err;
    if ((what_to_load & DTI_PDF_REQUEST) && dti_header.pdf_offset == 0l)
		goto err;
    if ((what_to_load & DTI_PICT_REQUEST) && dti_header.pict_offset == 0l)
		goto err;
    
    /* ----------- Begin load operations ----------------------------------- */
    
    HWRMemCpy(dti_descr->object_type, dti_header.object_type, DTI_ID_LEN);
    HWRMemCpy(dti_descr->type, dti_header.type, DTI_ID_LEN);
    HWRMemCpy(dti_descr->version, dti_header.version, DTI_ID_LEN);
    HWRStrnCpy(dti_descr->dti_fname, dtiname, DTI_FNAME_LEN-1);
#else
    UNUSED(dtiname);
#endif // DTI_LOAD_FROM_ROM
    
    /* ----------- DTE load operations ------------------------------------- */
    
    if (what_to_load & DTI_DTE_REQUEST)
    {
#if !DTI_LOAD_FROM_ROM
        _ULONG dte_i, dte_chsum;
        
        dti_descr->h_dte = HWRMemoryAllocHandle(dti_header.dte_len);
        if (dti_descr->h_dte == _NULL)
			goto err;
        dti_descr->p_dte = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dti_descr->h_dte);
        if (dti_descr->p_dte == _NULL)
			goto err;
#else  // DTI_LOAD_FROM_ROM
        HWRMemCpy((p_VOID)(dti_descr), dti_get_header(lang), sizeof(*dti_descr));
        dti_descr->p_dte = (p_UCHAR)dti_get_body(lang);
#endif // DTI_LOAD_FROM_ROM
        
        dti_descr->h_vex = HWRMemoryAllocHandle(DTI_SIZEOFVEXT + DTI_SIZEOFCAPT);
        if (dti_descr->h_vex == _NULL) 
			goto err;
        dti_descr->p_vex = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dti_descr->h_vex);
        if (dti_descr->p_vex == _NULL)
			goto err;
        HWRMemSet(dti_descr->p_vex, 0, DTI_SIZEOFVEXT + DTI_SIZEOFCAPT);
        
#if !DTI_LOAD_FROM_ROM
        if (fseek(dti_file, dti_header.dte_offset, SEEK_SET) != 0) 
			goto err;
        if (fread(dti_descr->p_dte, 1, (size_t)dti_header.dte_len, dti_file) != dti_header.dte_len)
			goto err;
        
        for (dte_i = 0l, dte_chsum = 0l; dte_i < dti_header.dte_len; dte_i ++)
            dte_chsum += ((p_UCHAR)(dti_descr->p_dte))[dte_i];
        
        if (dte_chsum != dti_header.dte_chsum)
			goto err;
#endif  //     #if !DTI_LOAD_FROM_ROM
        
        
#if DTI_COMPRESSED
        dte_index = (p_dte_index_type)dti_descr->p_dte;
        
#if !DTI_LOAD_FROM_ROM
        for (i = 0; i < 256; i ++)
            HWRSwapInt(&dte_index->sym_index[i]);
#endif //   #if !DTI_LOAD_FROM_ROM
        
        for (i = DTI_FIRSTSYM; i < DTI_FIRSTSYM + DTI_NUMSYMBOLS; i ++)
        {
            p_dte_var_header_type pvh;
            
            if (dte_index->sym_index[i] == 0) continue;
            sym_descr = (p_dte_sym_header_type)((p_CHAR)dti_descr->p_dte + (dte_index->sym_index[i] << 2));
            //      HWRMemCpy(dti_descr->p_vex+(i-DTI_FIRSTSYM)*DTI_MAXVARSPERLET, sym_descr->var_vexs, DTI_MAXVARSPERLET);
            pvh    = (p_dte_var_header_type)((p_UCHAR)sym_descr + sizeof(*sym_descr));
            for (j = 0; j < sym_descr->num_vars; j ++)
            {
                *((p_UCHAR)dti_descr->p_vex+(i-DTI_FIRSTSYM)*DTI_MAXVARSPERLET + j) = (_UCHAR)(pvh->nx_and_vex >> DTI_VEX_OFFS);
                k   = sizeof(*pvh) + sizeof(xrp_type)*((pvh->nx_and_vex & DTI_NXR_MASK)-1);
                pvh = (p_dte_var_header_type)((p_UCHAR)pvh + k);
            }
        }
#else
        sym_table = (let_table_type _PTR)dti_descr->p_dte;
        
#if !DTI_LOAD_FROM_ROM
        for (i = 0; i < 256; i ++) HWRSwapLong(&(*sym_table)[i]);
#endif //   #if !DTI_LOAD_FROM_ROM
        
        for (i = DTI_FIRSTSYM; i < DTI_FIRSTSYM + DTI_NUMSYMBOLS; i ++)
        {
            if ((*sym_table)[i] == 0) continue;
            sym_descr = (p_dte_sym_header_type)((p_CHAR)dti_descr->p_dte + (*sym_table)[i]);
            HWRMemCpy(dti_descr->p_vex+(i-DTI_FIRSTSYM)*DTI_MAXVARSPERLET, sym_descr->var_vexs, DTI_MAXVARSPERLET);
        }
#endif
        
#if !DTI_LOAD_FROM_ROM
        dti_descr->p_dte = _NULL;
        dti_descr->p_vex = _NULL;
#endif  //  #if !DTI_LOAD_FROM_ROM
    }
    
    /* ----------- XRT load operations ------------------------------------- */
    
#if !DTI_LOAD_FROM_ROM
    if (what_to_load & DTI_XRT_REQUEST)
    {
        _ULONG xrt_i, xrt_chsum;
        
        dti_descr->h_xrt = HWRMemoryAllocHandle(dti_header.xrt_len);
        if (dti_descr->h_xrt == _NULL)
            goto err;
        dti_descr->p_xrt = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dti_descr->h_xrt);
        if (dti_descr->p_xrt == _NULL)
            goto err;
        
        if (fseek(dti_file, dti_header.xrt_offset, SEEK_SET) != 0)
            goto err;
        if (fread(dti_descr->p_xrt, 1, (size_t)dti_header.xrt_len, dti_file) != dti_header.xrt_len)
            goto err;
        
        for (xrt_i = 0l, xrt_chsum = 0l; xrt_i < dti_header.xrt_len; xrt_i ++)
            xrt_chsum += (_UCHAR)*(dti_descr->p_xrt + (_UINT)xrt_i);
        
        if (xrt_chsum != dti_header.xrt_chsum)
            goto err;
        
        dti_descr->p_xrt = _NULL;
    }
#endif  //     #if !DTI_LOAD_FROM_ROM
    
    /* ----------- PDF load operations ------------------------------------- */
    
#if !DTI_LOAD_FROM_ROM
    if (what_to_load & DTI_PDF_REQUEST)
    {
        dti_descr->h_pdf     = 0l;
        dti_descr->p_pdf     = 0l;
        dti_descr->pdf_chsum = 0l;
#if USE_POSTPROC
        if (!PDFLoadFile((p_CHAR)dti_file, dti_header.pdf_offset, &(dti_descr->h_pdf)))
            goto err;
#endif
    }
#endif  //     #if !DTI_LOAD_FROM_ROM
    
    /* ----------- Pictures load operations -------------------------------- */
    
#if !DTI_LOAD_FROM_ROM
    if (what_to_load & DTI_PICT_REQUEST)
    {
        dti_descr->h_pict    = 0l;      /* Nothing here now */
        dti_descr->p_pict    = 0l;
        dti_descr->pict_chsum= 0l;
    }
#endif  //     #if !DTI_LOAD_FROM_ROM
    
    /* ----------- Closing down -------------------------------------------- */
    
#if !DTI_LOAD_FROM_ROM
    fclose(dti_file);
#endif  //     #if !DTI_LOAD_FROM_ROM
    
    *dp = (p_VOID _PTR)dti_descr;
    
    /* ----- Time to exit ----------------------------------------------------- */
    return 0;
err:
#if !DTI_LOAD_FROM_ROM
    if (dti_file)
		fclose(dti_file);
#endif  //     #if !DTI_LOAD_FROM_ROM
    
    if (dti_descr != _NULL)
    {
#if !DTI_LOAD_FROM_ROM
        if (dti_descr->h_dte)
            HWRMemoryFreeHandle((_HANDLE)dti_descr->h_dte);
        if (dti_descr->h_xrt)
            HWRMemoryFreeHandle((_HANDLE)dti_descr->h_xrt);
        if (dti_descr->h_vex)
            HWRMemoryFreeHandle((_HANDLE)dti_descr->h_vex);
        if (dti_descr->h_pdf)
            HWRMemoryFreeHandle((_HANDLE)dti_descr->h_pdf);
        if (dti_descr->h_pict)
            HWRMemoryFreeHandle((_HANDLE)dti_descr->h_pict);
#endif  //     #if !DTI_LOAD_FROM_ROM
        
        HWRMemoryFree(dti_descr);
    }
    return 1;
}


/* ************************************************************************* */
/*      Unload dat ingredients                                               */
/* ************************************************************************* */
_INT dti_unload(p_VOID _PTR dp)
{
    p_dti_descr_type pdti;
    
    if (dp == _NULL) 
		goto err;
    
    pdti = (p_dti_descr_type)(*dp);
    
    if (pdti != _NULL)
    {
#if !DTI_LOAD_FROM_ROM
        dti_unlock((p_VOID)pdti);
        if (pdti->h_dte)
            HWRMemoryFreeHandle((_HANDLE)pdti->h_dte);
        if (pdti->h_xrt)
            HWRMemoryFreeHandle((_HANDLE)pdti->h_xrt);
        if (pdti->h_pict)
            HWRMemoryFreeHandle((_HANDLE)pdti->h_pict);
        if (pdti->h_ram_dte)
            HWRMemoryFreeHandle((_HANDLE)pdti->h_ram_dte);
#endif /* #if !DTI_LOAD_FROM_ROM */
        
#if USE_POSTPROC
        if(pdti->h_pdf)
            PDFUnloadFile(&(pdti->h_pdf), &(pdti->p_pdf));
#endif
        
#if !USE_EXTERN_BUFFER_FOR_LEARNING_INFO
        if (pdti->h_vex)
            HWRMemoryFreeHandle((_HANDLE)pdti->h_vex);
#endif /* #if !USE_EXTERN_BUFFER_FOR_LEARNING_INFO */
        
        HWRMemoryFree(*dp);
        *dp = _NULL;
    }
err:
    return 0;
}

/* ************************************************************************* */
/* *   Writes DTI to a disk file                                           * */
/* ************************************************************************* */
#if !DTI_LOAD_FROM_ROM && DTI_COMPRESSED
_INT dti_save(p_CHAR fname, _INT what_to_save, p_VOID dp)
{
    _ULONG i;
    p_UCHAR ptr;
    p_dti_descr_type dti = (p_dti_descr_type)dp;
    dti_header_type dti_h;
    FILE *file = _NULL;
    
    if (dp == _NULL) 
		goto err;
    
    if ((file = fopen(fname, "wb")) == _NULL)
		goto err;
    
    // ----------------- Fill dti file header --------------------------------------
    
    HWRMemSet( (p_VOID)&dti_h, 0, sizeof(dti_h));
    
    HWRMemCpy( (p_VOID)dti_h.object_type, (p_VOID)DTI_DTI_OBJTYPE, DTI_ID_LEN);
    HWRMemCpy( (p_VOID)dti_h.type, (p_VOID)DTI_DTI_TYPE, DTI_ID_LEN);
    HWRMemCpy( (p_VOID)dti_h.version, (p_VOID)DTI_DTI_VER, DTI_ID_LEN);
    
    if (what_to_save | DTI_DTE_REQUEST)
    {
        dti_h.dte_offset = sizeof(dti_h);
        dti_h.dte_len = ((p_dte_index_type)dti->p_dte)->len;
        for (i = 0, ptr = dti->p_dte; i < dti_h.dte_len; i ++, ptr ++)
            dti_h.dte_chsum += *ptr;
    }
    
    //xrt_offset;
    // xrt_len;
    // xrt_chsum;
    
    // pdf_offset;
    // pdf_len;
    // pdf_chsum;
    
    // pict_offset;
    // pict_len;
    // pict_chsum;
    
    // ----------------- Save dti components to file -------------------------------
    
    if (fwrite(&dti_h, sizeof(dti_h), 1, file) != 1)
        goto err;
    
    if (what_to_save | DTI_DTE_REQUEST)
    {
        if (fwrite(dti->p_dte, dti_h.dte_len, 1, file) != 1)
            goto err;
    }
    
    // ----------------- Close file ------------------------------------------------
    
    if (file)
		fclose(file);
    return 0;
err:
    if (file)
		fclose(file);
    return 1;
}

#endif /* !DTI_LOAD_FROM_ROM && defined DTI_COMPRESSED */

#if !DTI_LOAD_FROM_ROM
/* ************************************************************************* */
/* *   Lock DTI                                                            * */
/* ************************************************************************* */
_INT dti_lock(p_VOID dti_ptr)
{
    p_dti_descr_type dp;
    
    if (dti_ptr == _NULL)
        goto err;
    
    dp = (p_dti_descr_type)(dti_ptr);
    if (dp == _NULL)
        goto err;
    
#if !USE_LOCKED_RESOURCES
    if (dp->p_dte == _NULL && dp->h_dte != _NULL)
        dp->p_dte = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dp->h_dte);
    if (dp->p_xrt == _NULL && dp->h_xrt != _NULL)
        dp->p_xrt = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dp->h_xrt);
    if (dp->p_pict== _NULL && dp->h_pict!= _NULL)
        dp->p_pict= (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dp->h_pict);
#endif /* #if !USE_LOCKED_RESOURCES */
    
    if (dp->p_ram_dte == _NULL && dp->h_ram_dte != _NULL)
        dp->p_ram_dte = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dp->h_ram_dte);
    
    if (dp->p_pdf == _NULL && dp->h_pdf != _NULL)
        dp->p_pdf = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dp->h_pdf);
    
    if (dp->p_vex == _NULL && dp->h_vex != _NULL)
        dp->p_vex = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dp->h_vex);
    
    return 0;
err:
    return 1;
}
#endif //     #if !DTI_LOAD_FROM_ROM

#if !DTI_LOAD_FROM_ROM
/* ************************************************************************* */
/* *   UnLock DTI                                                          * */
/* ************************************************************************* */
_INT dti_unlock(p_VOID dti_ptr)
{
    p_dti_descr_type dp = (p_dti_descr_type)(dti_ptr);
    
    if (dp == _NULL) goto err;
    
#if !USE_LOCKED_RESOURCES
    if (dp->p_dte != _NULL && dp->h_dte != _NULL)
    {
        dp->p_dte = _NULL;
    }
    if (dp->p_xrt != _NULL && dp->h_xrt != _NULL)
    {
        dp->p_xrt = _NULL;
    }
    if (dp->p_pict!= _NULL && dp->h_pict!= _NULL)
    {
        dp->p_pict= _NULL;
    }
#endif /* #if !USE_LOCKED_RESOURCES */
    
    if (dp->p_ram_dte != _NULL && dp->h_ram_dte != _NULL)
    {
        dp->p_ram_dte = _NULL;
    }
    
    if (dp->p_pdf != _NULL && dp->h_pdf != _NULL)
    {
        dp->p_pdf = _NULL;
    }
    
    if (dp->p_vex != _NULL && dp->h_vex != _NULL)
    {
        dp->p_vex = _NULL;
    }
    
    return 0;
err:
    return 1;
}
#endif //     #if !DTI_LOAD_FROM_ROM

#if DTI_LRN_SUPPORTFUNC
/* ************************************************************************* */
/*        Check if variant enabled for current xrcm                          */
/* ************************************************************************* */

_INT  CheckVarActive(_UCHAR chIn, _UCHAR nv, _UCHAR ww, p_VOID dtp)
{
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    
#if DTI_COMPRESSED
    p_dte_var_header_type pvh;
    
    if ( GetVarHeader( (_UCHAR)OSToRec(chIn, dp->language ), nv, &pvh, dp ) )
        goto err;
    if ((pvh->veis & (_UCHAR)(ww << DTI_OFS_WW)) == 0)
        goto err;
#else
    _INT                  numv;
    p_dte_sym_header_type let_descr;
    
    if ((numv = GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), nv, &let_descr, dp)) < 0)
        goto err;
    if ((let_descr->var_veis[numv] & (_UCHAR)(ww << DTI_OFS_WW)) == 0)
        goto err;
#endif
    
    return 1;
err:
    return 0;
}

/* ************************************************************************* */
/*        Get number of vars of requested letter                             */
/* ************************************************************************* */
_INT  GetNumVarsOfChar(_UCHAR chIn, p_VOID dtp)
{
    _INT                  nrom, nram;
    p_dte_sym_header_type let_descr;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    
    if (GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), 0, &let_descr, dp) < 0)
        goto err;
    nrom = let_descr->num_vars;
    if (GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), (_UCHAR)nrom, &let_descr, dp) < 0)
        nram = 0;
    else
        nram = let_descr->num_vars;
    
    return nrom+nram;
err:
    return 0;
}

/* ************************************************************************* */
/*        Get variant of a character                                         */
/* ************************************************************************* */
_INT GetVarOfChar(_UCHAR chIn, _UCHAR nv, p_xrp_type xvb, p_VOID dtp)
{
#if DTI_COMPRESSED
    _INT                  varlen;
    p_dte_var_header_type pvh;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    
    if (GetVarHeader((_UCHAR)OSToRec( chIn, dp->language ), nv, &pvh, dp))
        goto err;
    
    varlen = pvh->nx_and_vex & DTI_NXR_MASK;
    
    HWRMemCpy(xvb, pvh->xrs, varlen * sizeof(xrp_type));
    HWRMemSet((xvb+varlen), 0, sizeof(xrp_type));
#else
    
    _INT                  i, varlen, numv;
    p_dte_sym_header_type let_descr;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    p_xrp_type            varptr;
    
    if ((numv = GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), nv, &let_descr, dp)) < 0)
        goto err;
    
    varptr = (p_xrp_type)((p_UCHAR)let_descr + sizeof(dte_sym_header_type));
    
    for (i = 0; i < numv && i < DTI_MAXVARSPERLET; i ++)
        varptr += let_descr->var_lens[i];
    
    varlen = let_descr->var_lens[numv];
    
    HWRMemCpy(xvb, varptr, varlen * sizeof(xrp_type));
    HWRMemSet((xvb+varlen), 0, sizeof(xrp_type));
#endif
    
    return varlen;
err:
    return 0;
}

/* ************************************************************************* */
/*        Get variant length for character                                   */
/* ************************************************************************* */
_INT  GetVarLenOfChar(_UCHAR chIn, _UCHAR nv, p_VOID dtp)
{
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    _INT                  varlen;
    
#if DTI_COMPRESSED
    p_dte_var_header_type pvh;
    
    if (GetVarHeader((_UCHAR)OSToRec( chIn, dp->language ), nv, &pvh, dp))
        goto err;
    varlen = pvh->nx_and_vex & DTI_NXR_MASK;
    if (varlen > DTI_XR_SIZE)
        goto err;
#else
    _INT                  numv;
    p_dte_sym_header_type let_descr;
    
    if ((numv = GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), nv, &let_descr, dp)) < 0)
        goto err;
    varlen = let_descr->var_lens[numv];
    if (varlen > DTI_XR_SIZE)
        goto err;
#endif
    
    return varlen;
err:
    return 0;
}

/* ************************************************************************* */
/*        Return extra value (2-nd byte) for letter variant                  */
/* ************************************************************************* */
_INT  GetVarExtra(_UCHAR chIn, _UCHAR nv, p_VOID dtp)
{
    p_dti_descr_type dp = (p_dti_descr_type)dtp;
    _INT             varveis;
#if DTI_COMPRESSED
    p_dte_var_header_type pvh;
    
    if (GetVarHeader((_UCHAR)OSToRec( chIn, dp->language ), nv, &pvh, dp))
        goto err;
    varveis = pvh->veis;
#else
    _INT                  numv;
    p_dte_sym_header_type let_descr;
    
    if ((numv = GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), nv, &let_descr, dp)) < 0)
        goto err;
    varveis   = let_descr->var_veis[numv];
#endif
    
    return varveis;
err:
    return -1;
}

/* ************************************************************************* */
/*        Return VEX value for letter variant                                */
/* ************************************************************************* */
_INT  GetVarVex(_UCHAR chIn, _UCHAR nv, p_VOID dtp)
{
    _USHORT               varvex;
    p_dte_sym_header_type let_descr;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    p_dte_vex_type        vexbuf;
    
    if (GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), nv, &let_descr, dp) < 0)
        goto err;
    
    vexbuf = (p_dte_vex_type)dp->p_vex;
    if (vexbuf == _NULL)
        goto err;
    
    varvex = (*vexbuf)[OSToRec(chIn, dp->language )-DTI_FIRSTSYM][nv];
    varvex &= 0x07;
    
    return (_SHORT)varvex;
err:
    return -1;
}
/* ************************************************************************* */
/*        Set VEX value for letter variant                                   */
/* ************************************************************************* */
_INT  SetVarVex(_UCHAR chIn, _UCHAR nv, _UCHAR vex, p_VOID dtp)
{
    p_dte_sym_header_type let_descr;
    _UCHAR                ch;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    p_dte_vex_type        vexbuf;
    
    if (GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), nv, &let_descr, dp) < 0)
        goto err;
    
    vexbuf = (p_dte_vex_type)dp->p_vex;
    if (vexbuf == _NULL)
        goto err;
    
    ch = (_UCHAR)OSToRec(chIn, dp->language );
    
    (*vexbuf)[ch-DTI_FIRSTSYM][nv] &= 0xf8;
    (*vexbuf)[ch-DTI_FIRSTSYM][nv] |= (_UCHAR)(vex & 0x07);
    
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/*        Return Flag of capitalization change for the variant               */
/* ************************************************************************* */
_INT  GetVarCap(_UCHAR chIn, _UCHAR nv, p_VOID dtp)
{
    _INT                  varcap;
    p_dte_sym_header_type let_descr;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    p_dte_vex_type        vexbuf;
    p_UCHAR               capbuf;
    
    if (GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), nv, &let_descr, dp) < 0)
        goto err;
    
    vexbuf = (p_dte_vex_type)dp->p_vex;
    if (vexbuf == _NULL)
        goto err;
    
    capbuf = (p_UCHAR)vexbuf + DTI_SIZEOFVEXT;
    varcap = (capbuf[((OSToRec(chIn, dp->language )-DTI_FIRSTSYM)*DTI_MAXVARSPERLET+nv)/8] & (0x01 << (nv%8))) != 0;
    
    return varcap;
err:
    return -1;
}
/* ************************************************************************* */
/*        Set Capitalization flag for a variant                              */
/* ************************************************************************* */
_INT  SetVarCap(_UCHAR chIn, _UCHAR nv, _UCHAR cap, p_VOID dtp)
{
    _UCHAR                ch;
    p_dte_sym_header_type let_descr;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    p_dte_vex_type        vexbuf;
    p_UCHAR               capbuf;
    
    if (GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), nv, &let_descr, dp) < 0)
        goto err;
    
    vexbuf = (p_dte_vex_type)dp->p_vex;
    if (vexbuf == _NULL)
        goto err;
    
    ch = (_UCHAR)OSToRec(chIn, dp->language );
    
    capbuf = (p_UCHAR)vexbuf + DTI_SIZEOFVEXT;
    capbuf[((ch-DTI_FIRSTSYM)*DTI_MAXVARSPERLET+nv)/8] &= (_UCHAR)(~(0x01 << (nv%8)));
    if (cap) capbuf[((ch-DTI_FIRSTSYM)*DTI_MAXVARSPERLET+nv)/8] |= (_UCHAR)(0x01 << (nv%8));
    
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/*        Set default values of cap flags                                    */
/* ************************************************************************* */
_INT  SetDefCaps(p_VOID dtp)
{
    p_dti_descr_type     dp = (p_dti_descr_type)dtp;
    p_dte_vex_type       vexbuf;
    p_UCHAR              capbuf;
    
    if (dp == _NULL)
        goto err;
    vexbuf      = (p_dte_vex_type)dp->p_vex;
    if (vexbuf == _NULL)
        goto err;
    
    capbuf = (p_UCHAR)vexbuf + DTI_SIZEOFVEXT;
    
    HWRMemSet(capbuf, 0, DTI_SIZEOFCAPT);
    
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/*        Reset VEX values for all letter variants                           */
/* ************************************************************************* */

_INT  SetDefVexes(p_VOID dtp)
{
    _INT                  i;
    _INT                  loc;
    p_dte_sym_header_type let_descr;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    p_dte_vex_type        vexbuf;
    
    
#if DTI_COMPRESSED
    _INT                  j, k, d;
    p_dte_var_header_type pvh;
    
    if (dp == _NULL || dp->p_vex == _NULL)
        goto err;
    
    vexbuf      = (p_dte_vex_type)dp->p_vex;
    
    for (i = DTI_FIRSTSYM; i < DTI_FIRSTSYM + DTI_NUMSYMBOLS; i ++)
    {
        for (d = 0, loc = 0; d < 2; d ++) // Take RamDTE into the loop
        {
            if (GetSymDescriptor((_UCHAR)i, loc, &let_descr, dp) < 0)
                continue;
            
            pvh    = (p_dte_var_header_type)((p_UCHAR)let_descr + sizeof(*let_descr));
            (*vexbuf)[i-DTI_FIRSTSYM][loc] = (_UCHAR)(pvh->nx_and_vex >> DTI_VEX_OFFS);
            for (j = 1; j < let_descr->num_vars && j < DTI_MAXVARSPERLET; j ++)
            {
                k   = sizeof(*pvh) + sizeof(xrp_type)*((pvh->nx_and_vex & DTI_NXR_MASK)-1);
                pvh = (p_dte_var_header_type)((p_UCHAR)pvh + k);
                (*vexbuf)[i-DTI_FIRSTSYM][j+loc] = (_UCHAR)(pvh->nx_and_vex >> DTI_VEX_OFFS);
            }
            
            loc = let_descr->num_vars;
        }
    }
    //    for (j = 0; j < let_descr->nv; j ++)
    //     {
    //      k = GetVarVex((_UCHAR)(i), (_UCHAR)(j), dtp);
    //      if (k < 0) k = 0;
    //      (*vexbuf)[i-DTI_FIRSTSYM][j] = (_UCHAR)k;
    //     }
    
#else  //   #if DTI_COMPRESSED
    
    if (dp == _NULL || dp->p_vex == _NULL) goto err;
    
    vexbuf      = (p_dte_vex_type)dp->p_vex;
    
    for (i = DTI_FIRSTSYM; i < DTI_FIRSTSYM + DTI_NUMSYMBOLS; i ++)
    {
        if (GetSymDescriptor((_UCHAR)i, 0, &let_descr, dp) < 0) loc = 0;
        else
        {
            HWRMemCpy(&(*vexbuf)[i-DTI_FIRSTSYM][0], let_descr->var_vexs, sizeof(let_descr->var_vexs));
            loc = let_descr->num_vars;
        }
        
        if (GetSymDescriptor((_UCHAR)i, (_UCHAR)loc, &let_descr, dp) >= 0)
            HWRMemCpy(&(*vexbuf)[i-DTI_FIRSTSYM][loc], let_descr->var_vexs, sizeof(let_descr->var_vexs)-loc*(sizeof(let_descr->var_vexs[0])));
    }
    
#endif //   #if DTI_COMPRESSED
    
    
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/*        Set fly learn counter value                                        */
/* ************************************************************************* */
_INT  SetVarCounter(_UCHAR chIn, _UCHAR nv, _UCHAR cnt, p_VOID dtp)
{
    p_dte_sym_header_type let_descr;
    _UCHAR                ch;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    p_dte_vex_type        vexbuf;
    
    if (GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), nv, &let_descr, dp) < 0)
        goto err;
    
    if ((vexbuf = (p_dte_vex_type)dp->p_vex) == _NULL)
        goto err;
    
    ch = (_UCHAR)OSToRec(chIn, dp->language );
    
    (*vexbuf)[ch-DTI_FIRSTSYM][nv] &= 0x7;
    (*vexbuf)[ch-DTI_FIRSTSYM][nv] |= (_UCHAR)((cnt & 0x1F) << 3);
    
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/*        Return Group number for a prototype                                */
/* ************************************************************************* */
_INT  GetVarGroup(_UCHAR chIn, _UCHAR nv, p_VOID dtp)
{
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
#define GROUP_MASK  0x0007
#define GROUP_SHIFT      1
    
#if DTI_COMPRESSED
    p_dte_var_header_type pvh;
    
    if (GetVarHeader((_UCHAR)OSToRec( chIn, dp->language ), nv, &pvh, dp))
        goto err;
    return ((pvh->veis >> GROUP_SHIFT) & GROUP_MASK);
#else
    _INT                  numv;
    _SHORT                vargroup;
    p_dte_sym_header_type let_descr;
    
    if ((numv = GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), nv, &let_descr, dp)) < 0)
        goto err;
    
    vargroup = let_descr->var_veis[numv];
    vargroup = (_SHORT)((vargroup >> GROUP_SHIFT) & GROUP_MASK);
    
    return vargroup;
#endif
err:
    return -1;
}

#endif // DTI_LRN_SUPPORTFUNC

/* ************************************************************************* */
/*        Return Flag allowing this proto be used as one with opp cap        */
/* ************************************************************************* */
_INT  GetVarRewcapAllow(_UCHAR chIn, _UCHAR nv, p_VOID dtp)
{
    p_dti_descr_type dp = (p_dti_descr_type)dtp;
    
#if DTI_COMPRESSED
    p_dte_var_header_type pvh;
    
    if (GetVarHeader((_UCHAR)OSToRec( chIn, dp->language ), nv, &pvh, dp))
        goto err;
    return (pvh->veis & (_UCHAR)(DTI_CAP_BIT)) ? 0 : 1;
#else
    
    _INT numv;
    p_dte_sym_header_type let_descr;
    
    if ((numv = GetSymDescriptor((_UCHAR)OSToRec(chIn, dp->language ), nv, &let_descr, dp)) < 0)
        goto err;
    return (let_descr->var_veis[numv] & (_UCHAR)(DTI_CAP_BIT)) ? 0 : 1;
#endif
    
err:
    return -1;
}

/* ************************************************************************* */
/*        Return CHL position value for variant                              */
/* ************************************************************************* */
_INT  GetVarPosSize(_UCHAR chIn, _UCHAR nv, p_VOID dtp)
{
#if DTI_COMPRESSED
    _INT                  j, k;
    _INT                  varpos, numv;
    p_dte_sym_header_type let_descr;
    p_dte_var_header_type pvh;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    
    if ((numv = GetSymDescriptor((_UCHAR)OSToRec( chIn, dp->language ), nv, &let_descr, dp)) < 0) goto err;
    
    pvh    = (p_dte_var_header_type)((p_UCHAR)let_descr + sizeof(*let_descr));
    for (j = 0; j < numv; j ++)
    {
        k   = sizeof(*pvh) + sizeof(xrp_type)*((pvh->nx_and_vex & DTI_NXR_MASK)-1);
        pvh = (p_dte_var_header_type)((p_UCHAR)pvh + k);
    }
    
    varpos  = pvh->pos;
    varpos |= pvh->size << 8;
    if (varpos) varpos |= let_descr->loc_vs_border << 16; // Temp preserve -- if not defined data in DTE, let it be 0 (for later checks)
    
#else // Compressed
    
    _INT                 num_vars;
    _INT                 varpos;
    _UCHAR               ch;
    let_table_type  _PTR letxr_tabl;
    p_dte_sym_header_type let_descr;
    p_dti_descr_type     dp = (p_dti_descr_type)dtp;
    
    if (dp == _NULL) goto err;
    
    ch          = (_UCHAR)OSToRec(chIn, dp->language );
    let_descr   = _NULL;
    
    if (dp->p_ram_dte != _NULL) // Is RAM dte present?
    {
        letxr_tabl = (p_let_table_type)dp->p_ram_dte;
        if ((*letxr_tabl)[ch] != 0l)
            let_descr = (p_dte_sym_header_type)(dp->p_ram_dte + (*letxr_tabl)[ch]);
    }
    
    if (let_descr == _NULL && dp->p_dte != _NULL) // Is ROM dte present?
    {
        letxr_tabl = (p_let_table_type)dp->p_dte;
        if ((*letxr_tabl)[ch] != 0l)
            let_descr = (p_dte_sym_header_type)(dp->p_dte + (*letxr_tabl)[ch]);
    }
    
    if (let_descr == _NULL) goto err;
    num_vars  = let_descr->num_vars;
    
    if (nv >= num_vars) goto err; /* No such numbered variant in this letter */
    
    varpos    = let_descr->var_pos[nv];
    varpos   |= let_descr->var_size[nv] << 8;
    if (varpos) varpos |= let_descr->loc_vs_border << 16; // Temp preserve -- if not defined data in DTE, let it be 0 (for later checks)
    
#endif // Compressed
    
    return (_INT)(varpos);
err:
    return -1;
}

/* ************************************************************************* */
/*        Return AutoCorr value for letter variant                           */
/* ************************************************************************* */
#if 0
#if DTI_COMPRESSED

_INT  GetAutoCorr(_UCHAR chIn, _UCHAR nv, p_VOID dtp)
{
    _INT                  numv, varlen, i, autoCorr;
    p_dte_var_header_type varptr;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    p_xrp_type            xrp;
    
    if (GetVarHeader((_UCHAR)OSToRec(chIn), nv, &varptr, dp))
        goto err;
    
    varlen = varptr->nx_and_vex & DTI_NXR_MASK;
    for(i = 0, autoCorr = 0; i < varlen; i++)
    {
        if ((varptr->xrs[i].penl & DTI_PENL_MASK) != 0)
            autoCorr++;
    }
    
    return autoCorr*XRMC_DEF_CORR_VALUE;
err:
    return -1;
}

#else // #if DTI_COMPRESSED

_INT  GetAutoCorr(_UCHAR chIn, _UCHAR nv, p_VOID dtp)
{
    _INT                  numv, varlen, i, autoCorr;
    p_dte_sym_header_type let_descr;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    p_xrp_type            varptr;
    
    if ((numv = GetSymDescriptor((_UCHAR)OSToRec(chIn), nv, &let_descr, dp)) < 0)
        goto err;
    
    varptr = (p_xrp_type)((p_UCHAR)let_descr + sizeof(dte_sym_header_type));
    
    for (i = 0; i < numv && i < DTI_MAXVARSPERLET; i ++)
        varptr += let_descr->var_lens[i];
    
    varlen = let_descr->var_lens[numv];
    for(i = 0, autoCorr = 0; i < varlen; i++)
    {
        if (varptr[i].penl != 0) autoCorr++;
    }
    
    return autoCorr*XRMC_DEF_CORR_VALUE;
err:
    return -1;
}

#endif // #if DTI_COMPRESSED
#endif // 0

#if DTI_COMPRESSED
/* ************************************************************************* */
/*        Get pointer to variant header                                      */
/* ************************************************************************* */

_INT GetVarHeader(_UCHAR sym, _UCHAR nv, p_dte_var_header_type _PTR ppvh, p_VOID dtp)
{
    _INT                  j, k;
    _INT                  numv;
    p_dte_sym_header_type let_descr;
    p_dte_var_header_type pvh;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    
    
    if ((numv = GetSymDescriptor(sym, nv, &let_descr, dp)) < 0)
        goto err;
    
    pvh    = (p_dte_var_header_type)((p_UCHAR)let_descr + sizeof(*let_descr));
    for (j = 0; j < numv; j ++)
    {
        k   = sizeof(*pvh) + sizeof(xrp_type)*((pvh->nx_and_vex & DTI_NXR_MASK)-1);
        pvh = (p_dte_var_header_type)((p_UCHAR)pvh + k);
    }
    
    *ppvh = pvh;
    
    return 0;
err:
    return 1;
}
#endif  // DTI_COMPRESSED

#if DTI_COMPRESSED && DTI_LRN_SUPPORTFUNC

/* ************************************************************************* */
/*        Get Correct index table for a given sym                            */
/* ************************************************************************* */

_INT GetSymIndexTable(_UCHAR sym, _UCHAR numv, p_dte_index_type _PTR pi, p_VOID dtp)
{
    _INT                  n, nv;
    _INT                  lrom = 0;
    p_dte_sym_header_type sfc;
    p_dte_index_type      plt = _NULL;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
    
    if (dp == _NULL)
        goto err;
    
    for (n = 0, nv = 0; n < 2; n ++) // Cycle by DTI change
    {
        sfc = _NULL;
        
        if (n == 0 && dp->p_dte != _NULL)     // Is ROM dte present?
        {
            plt = (p_dte_index_type)dp->p_dte;
            if (plt->sym_index[sym] != 0l)
                sfc = (p_dte_sym_header_type)(dp->p_dte + (plt->sym_index[sym] << 2));
            if (sfc == _NULL)
                continue; // Pointer was 0!, symbol not defined
        }
        
        if (n == 1 && dp->p_ram_dte != _NULL) // Is RAM dte present?
        {
            plt = (p_dte_index_type)dp->p_ram_dte;
            if (plt->sym_index[sym] != 0l)
                sfc = (p_dte_sym_header_type)(dp->p_ram_dte + (plt->sym_index[sym] << 2));
            lrom = nv;
        }
        
        if (sfc == _NULL)
            continue;
        if (sfc->num_vars == 0)
            continue;
        
        nv += sfc->num_vars;
        if (nv > DTI_MAXVARSPERLET)
            goto err;
        
        if (numv < nv)
            break;
    }
    
    if (nv   <= 0)
        goto err;
    if (numv >= nv)
        goto err;
    
    *pi = plt;
    
    return numv-lrom;
err:
    return -1;
}

#endif  // DTI_COMPRESSED && DTI_LRN_SUPPORTFUNC

/* ************************************************************************* */
/*        Get Sym Descriptor Header Pointer and new Variant Number           */
/* ************************************************************************* */

_INT GetSymDescriptor(_UCHAR sym, _UCHAR numv, p_dte_sym_header_type _PTR psfc, p_VOID dtp)
{
    _INT                  n, nv;
    _INT                  lrom = 0;
    p_dte_sym_header_type sfc = _NULL;
    p_dti_descr_type      dp = (p_dti_descr_type)dtp;
#if DTI_COMPRESSED
    p_dte_index_type      plt;
#else
    p_let_table_type      plt;
#endif
    
    if (dp == _NULL)
        goto err;
    
    for (n = 0, nv = 0; n < 2; n ++) // Cycle by DTI change
    {
        sfc = _NULL;
        
        if (n == 0 && dp->p_dte != _NULL)     // Is ROM dte present?
        {
#if DTI_COMPRESSED
            plt = (p_dte_index_type)dp->p_dte;
            if (plt->sym_index[sym] != 0l)
                sfc = (p_dte_sym_header_type)(dp->p_dte + (plt->sym_index[sym] << 2));
#else
            plt = (p_let_table_type)dp->p_dte;
            if ((*plt)[sym] != 0l)
                sfc = (p_dte_sym_header_type)(dp->p_dte + (*plt)[sym]);
#endif
            if (sfc == _NULL)
                continue; // Pointer was 0!, symbol not defined
        }
        
        if (n == 1 && dp->p_ram_dte != _NULL) // Is RAM dte present?
        {
#if DTI_COMPRESSED
            plt = (p_dte_index_type)dp->p_ram_dte;
            if (plt->sym_index[sym] != 0l)
                sfc = (p_dte_sym_header_type)(dp->p_ram_dte + (plt->sym_index[sym] << 2));
#else
            plt = (p_let_table_type)dp->p_ram_dte;
            if ((*plt)[sym] != 0l)
                sfc = (p_dte_sym_header_type)(dp->p_ram_dte + (*plt)[sym]);
#endif
            lrom = nv;
        }
        
        if (sfc == _NULL)
            continue;
        if (sfc->num_vars == 0)
            continue;
        
        nv += sfc->num_vars;
        if (nv > DTI_MAXVARSPERLET)
        {
            goto err;
        }
        if (numv < nv)
            break;
    }
    
    if (nv   <= 0)
        goto err;
    if (numv >= nv)
        goto err;
    
    *psfc = sfc;
    return numv-lrom;
err:
    return -1;
}

#endif // LSTRIP
