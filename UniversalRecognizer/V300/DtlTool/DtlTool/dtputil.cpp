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
#include "hwr_swap.h"
#include "zctype.h"

#include "ams_mg.h"
#include "xr_names.h"
#include "langid.h"
#include "langutil.h"
#include "xrword.h"

#include "dtp.h"

#define DUMP_DTP 1


#define HWRMemoryUnlockHandle( x )  ((void *)(x))
#define LockRamParaData             HWRMemoryLockHandle
#define UnlockRamParaData           HWRMemoryUnlockHandle

#define LockRAMPDF                  HWRMemoryLockHandle
#define UnlockRAMPDF( x, y )

#if !LOAD_RESOURCES_FROM_ROM

#if DUMP_DTP
static _INT DumpDtpToC( const char * name, _INT len, p_dtp_descr_type dtp_descr );
#endif // DUMP_DTP

RECO_DATA_EXTERNAL _UCHAR os_rec_ctbl_let[LANGUAGE_SIZE][CONV_TBL_SIZE][2];

static _INT DtpOSToRec( _INT sym, _INT langID )
{
    sym &= 0x0FF;
    if (sym >= ST_CONV_RANGE)
    {
        _INT  i;
        _INT s = sym;
        
        sym = 0;
        
        for (i = 0; i < CONV_TBL_SIZE; i ++)
        {
            if ( s == (_INT)os_rec_ctbl_let[langID][i][0])
            {
                sym = (_INT)os_rec_ctbl_let[langID][i][1];
                break;
            }
        }
    }
    
    return sym;
}


/* ************************************************************************* */
/* *   DTP load                                                            * */
/* ************************************************************************* */

_INT dtp_load(p_CHAR dtpname, _INT what_to_load, _VOID _PTR _PTR dp)
{
    _INT                 i;
    FILE *               dtp_file = _NULL;
    dtp_header_type      dtp_header;
    p_dtp_descr_type     dtp_descr;
    let_table_type _PTR  sym_table;
    p_dtp_sym_header_type sym_descr;
#if HWR_SYSTEM == MACINTOSH
    _HMEM                dtp_descrH = _NULL;
#endif
    
    *dp = _NULL;
    
    /* ----------- Dig in file header -------------------------------------- */
    
#if HWR_SYSTEM == MACINTOSH
    if((dtp_descrH = HWRMemoryAllocHandle(sizeof(dtp_descr_type))) == _NULL)
    {
        dtp_descr = _NULL;
        goto err;
    }
    if((dtp_descr = (p_dtp_descr_type)HWRMemoryLockHandle(dtp_descrH)) == _NULL)
    {
        HWRMemoryFreeHandle(dtp_descrH);
        goto err;
    }
#else
    dtp_descr = (p_dtp_descr_type)HWRMemoryAlloc(sizeof(dtp_descr_type));
#endif
    if (dtp_descr == _NULL) goto err;
    HWRMemSet(dtp_descr, 0, sizeof(*dtp_descr));
    
    if ((dtp_file = fopen(dtpname, "rb")) == _NULL)
        goto err;
    
    if (fread(&dtp_header, 1, sizeof(dtp_header), dtp_file) != sizeof(dtp_header))
        goto err;
    
    HWRSwapLong(&dtp_header.dtp_offset);
    HWRSwapLong(&dtp_header.dtp_len);
    HWRSwapLong(&dtp_header.dtp_chsum);
    
    HWRSwapLong(&dtp_header.xrt_offset);
    HWRSwapLong(&dtp_header.xrt_len);
    HWRSwapLong(&dtp_header.xrt_chsum);
    HWRSwapLong(&dtp_header.pdf_offset);
    HWRSwapLong(&dtp_header.pdf_len);
    HWRSwapLong(&dtp_header.pdf_chsum);
    
    HWRSwapLong(&dtp_header.pict_offset);
    HWRSwapLong(&dtp_header.pict_len);
    HWRSwapLong(&dtp_header.pict_chsum);
    
    if (HWRStrnCmp(dtp_header.object_type, DTP_DTP_OBJTYPE, DTP_ID_LEN) != 0)
        goto err;
    if (HWRStrnCmp(dtp_header.version, DTP_DTP_VER, DTP_ID_LEN/2) != 0)
        goto err;
    if ((what_to_load & DTI_DTE_REQUEST) && dtp_header.dtp_offset == 0l)
        goto err;
    if ((what_to_load & DTI_XRT_REQUEST) && dtp_header.xrt_offset == 0l)
        goto err;
    if ((what_to_load & DTI_PDF_REQUEST) && dtp_header.pdf_offset == 0l)
        goto err;
    if ((what_to_load & DTI_PICT_REQUEST) && dtp_header.pict_offset == 0l)
        goto err;
    
    /* ----------- Begin load operations ----------------------------------- */
    
    HWRMemCpy(dtp_descr->object_type, dtp_header.object_type, DTP_ID_LEN);
    HWRMemCpy(dtp_descr->type, dtp_header.type, DTP_ID_LEN);
    HWRMemCpy(dtp_descr->version, dtp_header.version, DTP_ID_LEN);
    HWRStrnCpy(dtp_descr->dtp_fname, dtpname, DTP_FNAME_LEN-1);
    
    /* ----------- DTE load operations ------------------------------------- */
    
    if (what_to_load & DTI_DTE_REQUEST)
    {
        _ULONG          dtp_i, dtp_chsum;
        
        dtp_descr->h_dtp = HWRMemoryAllocHandle(dtp_header.dtp_len);
        if (dtp_descr->h_dtp == _NULL)
            goto err;
        dtp_descr->p_dtp = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dtp_descr->h_dtp);
        if (dtp_descr->p_dtp == _NULL)
            goto err;
        
        dtp_descr->h_vex = HWRMemoryAllocHandle(DTP_SIZEOFVEXT + DTP_SIZEOFCAPT);
        if (dtp_descr->h_vex == _NULL)
            goto err;
        dtp_descr->p_vex = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dtp_descr->h_vex);
        if (dtp_descr->p_vex == _NULL)
            goto err;
        
        HWRMemSet(dtp_descr->p_vex, 0, DTP_SIZEOFVEXT + DTP_SIZEOFCAPT);
        
        if (fseek(dtp_file, dtp_header.dtp_offset, SEEK_SET) != 0) goto err;
        if (fread(dtp_descr->p_dtp, 1, (size_t)dtp_header.dtp_len, dtp_file) != dtp_header.dtp_len)
            goto err;
        
        for (dtp_i = 0l, dtp_chsum = 0l; dtp_i < dtp_header.dtp_len; dtp_i ++)
            dtp_chsum += ((p_UCHAR)(dtp_descr->p_dtp))[dtp_i];
        
        if (dtp_chsum != dtp_header.dtp_chsum)
            goto err;
        
        sym_table = (let_table_type _PTR)dtp_descr->p_dtp;
        for (i = 0; i < 256; i ++) HWRSwapLong(&(*sym_table)[i]);
        
        for (i = DTP_FIRSTSYM; i < DTP_FIRSTSYM + DTP_NUMSYMBOLS; i ++)
        {
            if ((*sym_table)[i] == 0) continue;
            sym_descr = (p_dtp_sym_header_type)((p_CHAR)dtp_descr->p_dtp + (*sym_table)[i]);
            HWRMemCpy(dtp_descr->p_vex+(i-DTP_FIRSTSYM)*DTP_MAXVARSPERLET, sym_descr->var_vexs, DTP_MAXVARSPERLET);
        }
        
#if DUMP_DTP
        DumpDtpToC( "dti_img_dump.cpp", dtp_header.dtp_len, dtp_descr );
#endif /* DUMP_DTP */
        
        HWRMemoryUnlockHandle((_HANDLE)dtp_descr->h_dtp); dtp_descr->p_dtp = _NULL;
        HWRMemoryUnlockHandle((_HANDLE)dtp_descr->h_vex); dtp_descr->p_vex = _NULL;
    }
    
    /* ----------- XRT load operations ------------------------------------- */
    
    if (what_to_load & DTI_XRT_REQUEST)
    {
        _ULONG xrt_i, xrt_chsum;
        
        dtp_descr->h_xrt = HWRMemoryAllocHandle(dtp_header.xrt_len);
        if (dtp_descr->h_xrt == _NULL)
            goto err;
        dtp_descr->p_xrt = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dtp_descr->h_xrt);
        if (dtp_descr->p_xrt == _NULL)
            goto err;
        
        if (fseek(dtp_file, dtp_header.xrt_offset, SEEK_SET) != 0)
            goto err;
        if (fread(dtp_descr->p_xrt, 1, (size_t)dtp_header.xrt_len, dtp_file) != dtp_header.xrt_len)
            goto err;
        
        for (xrt_i = 0l, xrt_chsum = 0l; xrt_i < dtp_header.xrt_len; xrt_i ++)
            xrt_chsum += (_UCHAR)*(dtp_descr->p_xrt + (_UINT)xrt_i);
        
        if (xrt_chsum != dtp_header.xrt_chsum)
            goto err;
        
        HWRMemoryUnlockHandle((_HANDLE)dtp_descr->h_xrt);
        dtp_descr->p_xrt = _NULL;
    }
    
    /* ----------- PDF load operations ------------------------------------- */
    
    if (what_to_load & DTI_PDF_REQUEST)
    {
        dtp_descr->h_pdf     = 0l;
        dtp_descr->p_pdf     = 0l;
        dtp_descr->pdf_chsum = 0l;
#if USE_POSTPROC
        if (!PDFLoadFile((p_CHAR)dtp_file, dtp_header.pdf_offset, &(dtp_descr->h_pdf)))
            goto err;
#endif
    }
    
    /* ----------- Pictures load operations -------------------------------- */
    
    if (what_to_load & DTI_PICT_REQUEST)
    {
        dtp_descr->h_pict    = 0l;      /* Nothing here now */
        dtp_descr->p_pict    = 0l;
        dtp_descr->pict_chsum= 0l;
        
        //    HWRMemoryUnlockHandle(dtp_descr->h_pict); dtp_descr->p_pict = _NULL;
    }
    
    /* ----------- Closing down -------------------------------------------- */
    
    fclose(dtp_file);
#if HWR_SYSTEM == MACINTOSH
    HWRMemoryUnlockHandle(dtp_descrH);
    *dp = (p_VOID)dtp_descrH;
#else
    *dp = (p_VOID _PTR)dtp_descr;
#endif
    
    /* ----- Time to exit ----------------------------------------------------- */
    return 0;
err:
    if (dtp_file)
        fclose(dtp_file);
    if (dtp_descr != _NULL)
    {
        if (dtp_descr->p_dtp)
            HWRMemoryUnlockHandle((_HANDLE)dtp_descr->h_dtp);
        if (dtp_descr->h_dtp)
            HWRMemoryFreeHandle((_HANDLE)dtp_descr->h_dtp);
        if (dtp_descr->p_xrt)
            HWRMemoryUnlockHandle((_HANDLE)dtp_descr->h_xrt);
        if (dtp_descr->h_xrt)
            HWRMemoryFreeHandle((_HANDLE)dtp_descr->h_xrt);
        if (dtp_descr->p_vex)
            HWRMemoryUnlockHandle((_HANDLE)dtp_descr->h_vex);
        if (dtp_descr->h_vex)
            HWRMemoryFreeHandle((_HANDLE)dtp_descr->h_vex);
        if (dtp_descr->p_pdf)
            HWRMemoryUnlockHandle((_HANDLE)dtp_descr->h_pdf);
        if (dtp_descr->h_pdf)
            HWRMemoryFreeHandle((_HANDLE)dtp_descr->h_pdf);
        if (dtp_descr->p_pict)
            HWRMemoryUnlockHandle((_HANDLE)dtp_descr->h_pict);
        if (dtp_descr->h_pict)
            HWRMemoryFreeHandle((_HANDLE)dtp_descr->h_pict);
#if HWR_SYSTEM == MACINTOSH
        HWRMemoryUnlockHandle(dtp_descrH);
        HWRMemoryFreeHandle(dtp_descrH);
#else
        HWRMemoryFree(dtp_descr);
#endif
    }
    return 1;
}


#endif /* #if !LOAD_RESOURCES_FROM_ROM */
/* ************************************************************************* */
/*      Unload dat ingredients                                               */
/* ************************************************************************* */
_INT dtp_unload(p_VOID _PTR dp)
{
    p_dtp_descr_type pdtp;
    
    if (dp == _NULL) goto err;
    
#if HWR_SYSTEM == MACINTOSH
    pdtp = (p_dtp_descr_type)
    HWRMemoryLockHandle((_HMEM)(*dp));
    if (pdtp == _NULL)
        HWRMemoryFreeHandle((_HMEM)(*dp));
#else
    pdtp = (p_dtp_descr_type)(*dp);
#endif
    
    if (pdtp != _NULL)
    {
        dtp_unlock((p_VOID)pdtp);
        
#if !LOAD_RESOURCES_FROM_ROM
        if (pdtp->h_dtp)
            HWRMemoryFreeHandle((_HANDLE)pdtp->h_dtp);
        if (pdtp->h_xrt)
            HWRMemoryFreeHandle((_HANDLE)pdtp->h_xrt);
        if (pdtp->h_pict)
            HWRMemoryFreeHandle((_HANDLE)pdtp->h_pict);
        if (pdtp->h_ram_dtp)
            HWRMemoryFreeHandle((_HANDLE)pdtp->h_ram_dtp);
#endif /* #if !LOAD_RESOURCES_FROM_ROM */
        
#if USE_POSTPROC
        if(pdtp->h_pdf)
            PDFUnloadFile(&(pdtp->h_pdf), &(pdtp->p_pdf));
#endif
        
#if !USE_EXTERN_BUFFER_FOR_LEARNING_INFO
        if (pdtp->h_vex)
            HWRMemoryFreeHandle((_HANDLE)pdtp->h_vex);
#endif /* #if !USE_EXTERN_BUFFER_FOR_LEARNING_INFO */
#if HWR_SYSTEM == MACINTOSH
        HWRMemoryUnlockHandle((_HMEM)(*dp));
        HWRMemoryFreeHandle((_HMEM)(*dp));
#else
        HWRMemoryFree(*dp);
#endif
        
        *dp = _NULL;
    }
err:
    return 0;
}

/* ************************************************************************* */
/* *   Writes DTP to a disk file                                           * */
/* ************************************************************************* */

#if !LOAD_RESOURCES_FROM_ROM

_INT dtp_save(p_CHAR fname, _INT what_to_save, p_VOID dp, _INT langID )
{
    _ULONG i;
    p_UCHAR ptr;
    p_dtp_descr_type dtp = (p_dtp_descr_type)dp;
    dtp_header_type dtp_h;
    FILE *file = _NULL;
    
    if (dp == _NULL)
        goto err;
    
    if ((file = fopen(fname, "wb")) == _NULL)
        goto err;
    
    // ----------------- Fill dti file header --------------------------------------
    
    HWRMemSet((p_VOID)&dtp_h, 0, sizeof(dtp_h));
    
    HWRMemCpy( (p_CHAR)dtp_h.object_type, (p_CHAR)DTP_DTP_OBJTYPE, DTP_ID_LEN);
    HWRMemCpy( (p_CHAR)dtp_h.type, (p_CHAR)"EngM", DTP_ID_LEN);
    HWRMemCpy( (p_CHAR)dtp_h.version, (p_CHAR)DTP_DTP_VER, DTP_ID_LEN);
    
    if (what_to_save | DTI_DTE_REQUEST)
    {
        dtp_h.dtp_offset = sizeof(dtp_h);
        dtp_h.dtp_len    = dtp->xrt_chsum;
        for (i = 0, ptr = dtp->p_dtp; i < dtp_h.dtp_len; i ++, ptr ++)
            dtp_h.dtp_chsum += *ptr;
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
    
    // ----------------- Save dtp components to file -------------------------------
    
    if (fwrite(&dtp_h, sizeof(dtp_h), 1, file) != 1)
        goto err;
    
    if (what_to_save | DTI_DTE_REQUEST)
    {
        if (fwrite(dtp->p_dtp, dtp_h.dtp_len, 1, file) != 1)
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

#endif /* #if !LOAD_RESOURCES_FROM_ROM */
/* ************************************************************************* */
/* *   Lock DTP                                                            * */
/* ************************************************************************* */
_INT dtp_lock(p_VOID dtp_ptr)
{
    p_dtp_descr_type dp;
    
    if (dtp_ptr == _NULL) goto err;
    
    dp = (p_dtp_descr_type)(dtp_ptr);
    
    if (dp == _NULL) goto err;
    
#if !USE_LOCKED_RESOURCES
    if (dp->p_dtp == _NULL && dp->h_dtp != _NULL)
        dp->p_dtp = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dp->h_dtp);
    if (dp->p_xrt == _NULL && dp->h_xrt != _NULL)
        dp->p_xrt = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dp->h_xrt);
    if (dp->p_pict== _NULL && dp->h_pict!= _NULL)
        dp->p_pict= (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dp->h_pict);
    
#endif /* #if !USE_LOCKED_RESOURCES */
    if (dp->p_ram_dtp == _NULL && dp->h_ram_dtp != _NULL)
        dp->p_ram_dtp = (p_UCHAR)LockRamParaData((_HANDLE)dp->h_ram_dtp);
#if HWR_SYSTEM != MACINTOSH
    if (dp->p_pdf == _NULL && dp->h_pdf != _NULL)
        dp->p_pdf = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dp->h_pdf);
#else
    if (dp->p_ram_pdf != _NULL && dp->p_pdf == _NULL)
    {
        if((dp->p_pdf = (p_UCHAR)LockRAMPDF(dp->p_ram_pdf)) == _NULL)
            dp->p_ram_pdf = _NULL;
    }
    if (dp->p_pdf == _NULL && dp->h_pdf != _NULL)
        dp->p_pdf = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dp->h_pdf);
#endif
    if (dp->p_vex == _NULL && dp->h_vex != _NULL)
        dp->p_vex = (p_UCHAR)HWRMemoryLockHandle((_HANDLE)dp->h_vex);
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/* *   UnLock DTP                                                          * */
/* ************************************************************************* */
_INT dtp_unlock(p_VOID dtp_ptr)
{
    p_dtp_descr_type dp = (p_dtp_descr_type)(dtp_ptr);
    
    if (dp == _NULL) goto err;
    
#if !USE_LOCKED_RESOURCES
    if (dp->p_dtp != _NULL && dp->h_dtp != _NULL)
    {
        HWRMemoryUnlockHandle((_HANDLE)dp->h_dtp);
        dp->p_dtp = _NULL;
    }
    if (dp->p_xrt != _NULL && dp->h_xrt != _NULL)
    {
        HWRMemoryUnlockHandle((_HANDLE)dp->h_xrt);
        dp->p_xrt = _NULL;
    }
    if (dp->p_pict!= _NULL && dp->h_pict!= _NULL)
    {
        HWRMemoryUnlockHandle((_HANDLE)dp->h_pict);
        dp->p_pict= _NULL;
    }
    
#endif /* #if !USE_LOCKED_RESOURCES */
    if (dp->p_ram_dtp != _NULL && dp->h_ram_dtp != _NULL)
    {
        UnlockRamParaData((_HANDLE)dp->h_ram_dtp);
        dp->p_ram_dtp = _NULL;
    }
#if HWR_SYSTEM != MACINTOSH
    if (dp->p_pdf != _NULL && dp->h_pdf != _NULL)
    {
        HWRMemoryUnlockHandle((_HANDLE)dp->h_pdf);
        dp->p_pdf = _NULL;
    }
#else
    if (dp->p_ram_pdf == _NULL && dp->p_pdf != _NULL && dp->h_pdf != _NULL)
    {
        HWRMemoryUnlockHandle((_HANDLE)dp->h_pdf);
        dp->p_pdf = _NULL;
    }
    if (dp->p_ram_pdf != _NULL && dp->p_pdf != _NULL)
    {
        UnlockRAMPDF( dp->p_ram_pdf, dp->p_pdf );
        dp->p_pdf = _NULL;
    }
#endif
    if (dp->p_vex != _NULL && dp->h_vex != _NULL)
    {
        HWRMemoryUnlockHandle((_HANDLE)dp->h_vex);
        dp->p_vex = _NULL;
    }
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/*        Check if variant enabled for current xrcm                          */
/* ************************************************************************* */
_INT  CheckVarActive(_UCHAR chIn, _UCHAR nv, _UCHAR ww, p_VOID dtp, _INT langID)
{
    _INT                  numv;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    
    
    if ((numv = GetSymDescriptor((_UCHAR)DtpOSToRec( chIn, langID ), nv, &let_descr, dp)) < 0)
        goto err;
    if ((let_descr->var_veis[numv] & (_UCHAR)(ww << DTP_OFS_WW)) == 0)
        goto err;
    
    return 1;
err:
    return 0;
}

/* ************************************************************************* */
/*        Get number of vars of requested letter                             */
/* ************************************************************************* */
_INT  GetNumVarsOfChar(_UCHAR chIn, p_VOID dtp, _INT langID)
{
    _INT                  nrom, nram;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    
    if (GetSymDescriptor((_UCHAR)DtpOSToRec( chIn, langID ), 0, &let_descr, dp) < 0)
        goto err;
    nrom = let_descr->num_vars;
    if (GetSymDescriptor((_UCHAR)DtpOSToRec( chIn, langID ), (_UCHAR)nrom, &let_descr, dp) < 0)
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
_INT GetVarOfChar(_UCHAR chIn, _UCHAR nv, p_xrpp_type xvb, p_VOID dtp, _INT langID )
{
    _INT                  i, varlen, numv;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    p_xrpp_type            varptr;
    
    if ((numv = GetSymDescriptor((_UCHAR)DtpOSToRec( chIn, langID ), nv, &let_descr, dp)) < 0)
        goto err;
    
    varptr = (p_xrpp_type)((p_UCHAR)let_descr + sizeof(dtp_sym_header_type));
    
    for (i = 0; i < numv && i < DTP_MAXVARSPERLET; i ++)
        varptr += let_descr->var_lens[i];
    
    varlen = let_descr->var_lens[numv];
    
    HWRMemCpy(xvb, varptr, varlen * sizeof(xrpp_type));
    HWRMemSet((xvb+varlen), 0, sizeof(xrpp_type));
    
    return varlen;
err:
    return 0;
}

/* ************************************************************************* */
/*        Get variant length for character                                   */
/* ************************************************************************* */
_INT  GetVarLenOfChar(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID )
{
    _INT                  varlen, numv;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    
    if ((numv = GetSymDescriptor( (_UCHAR)DtpOSToRec( chIn, langID ), nv, &let_descr, dp)) < 0)
        goto err;
    
    varlen = let_descr->var_lens[numv];
    if (varlen > DTP_XR_SIZE)
        goto err;
    
    return varlen;
err:
    return 0;
}

/* ************************************************************************* */
/*        Return extra value (2-nd byte) for letter variant                  */
/* ************************************************************************* */
_INT  GetVarExtra(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID )
{
    _INT                  numv;
    _USHORT               varveis;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    
    if ((numv = GetSymDescriptor( (_UCHAR)DtpOSToRec( chIn, langID ), nv, &let_descr, dp)) < 0)
        goto err;
    
    varveis = let_descr->var_veis[numv];
    
    return (_INT)(varveis);
err:
    return -1;
}

/* ************************************************************************* */
/*        Return VEX value for letter variant                                */
/* ************************************************************************* */
_INT  GetVarVex(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID )
{
    _USHORT               varvex;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    p_dtp_vex_type        vexbuf;
    
    if (GetSymDescriptor((_UCHAR)DtpOSToRec( chIn, langID ), nv, &let_descr, dp) < 0)
        goto err;
    
    vexbuf = (p_dtp_vex_type)dp->p_vex;
    if (vexbuf == _NULL)
        goto err;
    
    varvex = (*vexbuf)[DtpOSToRec(chIn, langID)-DTP_FIRSTSYM][nv];
    
    varvex &= 0x07;
    
    return (_SHORT)varvex;
err:
    return -1;
}
/* ************************************************************************* */
/*        Set VEX value for letter variant                                   */
/* ************************************************************************* */
_INT  SetVarVex(_UCHAR chIn, _UCHAR nv, _UCHAR vex, p_VOID dtp, _INT langID)
{
    p_dtp_sym_header_type let_descr;
    _UCHAR                ch;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    p_dtp_vex_type        vexbuf;
    
    if (GetSymDescriptor((_UCHAR)DtpOSToRec(chIn, langID ), nv, &let_descr, dp) < 0) goto err;
    
    vexbuf = (p_dtp_vex_type)dp->p_vex;
    if (vexbuf == _NULL)
        goto err;
    
    ch = (_UCHAR)DtpOSToRec(chIn, langID );
    
    (*vexbuf)[ch-DTP_FIRSTSYM][nv] &= 0xf8;
    (*vexbuf)[ch-DTP_FIRSTSYM][nv] |= (_UCHAR)(vex & 0x07);
    
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/*        Return Flag of capitalization change for the variant               */
/* ************************************************************************* */
_INT  GetVarCap(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID)
{
    _INT                  varcap;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    p_dtp_vex_type        vexbuf;
    p_UCHAR               capbuf;
    
    if (GetSymDescriptor((_UCHAR)DtpOSToRec(chIn, langID ), nv, &let_descr, dp) < 0)
        goto err;
    
    vexbuf = (p_dtp_vex_type)dp->p_vex;
    if (vexbuf == _NULL)
        goto err;
    
    capbuf = (p_UCHAR)vexbuf + DTP_SIZEOFVEXT;
    varcap = (capbuf[((DtpOSToRec(chIn, langID )-DTP_FIRSTSYM)*DTP_MAXVARSPERLET+nv)/8] & (0x01 << (nv%8))) != 0;
    
    return varcap;
err:
    return -1;
}
/* ************************************************************************* */
/*        Set Capitalization flag for a variant                              */
/* ************************************************************************* */
_INT  SetVarCap(_UCHAR chIn, _UCHAR nv, _UCHAR cap, p_VOID dtp, _INT langID)
{
    _UCHAR                ch;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    p_dtp_vex_type        vexbuf;
    p_UCHAR               capbuf;
    
    if (GetSymDescriptor((_UCHAR)DtpOSToRec(chIn, langID ), nv, &let_descr, dp) < 0) goto err;
    
    vexbuf = (p_dtp_vex_type)dp->p_vex;
    if (vexbuf == _NULL)
        goto err;
    
    ch = (_UCHAR)DtpOSToRec(chIn, langID );
    
    capbuf = (p_UCHAR)vexbuf + DTP_SIZEOFVEXT;
    capbuf[((ch-DTP_FIRSTSYM)*DTP_MAXVARSPERLET+nv)/8] &= (_UCHAR)(~(0x01 << (nv%8)));
    if (cap)
        capbuf[((ch-DTP_FIRSTSYM)*DTP_MAXVARSPERLET+nv)/8] |= (_UCHAR)(0x01 << (nv%8));
    
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/*        Set default values of cap flags                                    */
/* ************************************************************************* */

_INT  SetDefCaps(p_VOID dtp)
{
    p_dtp_descr_type     dp = (p_dtp_descr_type)dtp;
    p_dtp_vex_type       vexbuf;
    p_UCHAR              capbuf;
    
    if (dp == _NULL)
        goto err;
    vexbuf= (p_dtp_vex_type)dp->p_vex;
    if (vexbuf == _NULL)
        goto err;
    
    capbuf= (p_UCHAR)vexbuf + DTP_SIZEOFVEXT;
    
    HWRMemSet(capbuf, 0, DTP_SIZEOFCAPT);
    
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/*        Reset VEX values for all letter variants                           */
/* ************************************************************************* */

_INT SetDefDtpVexes(p_VOID dtp)
{
    _INT                  i;
    _INT                  loc;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    p_dtp_vex_type        vexbuf;
    
    if (dp == _NULL || dp->p_vex == _NULL) goto err;
    
    vexbuf = (p_dtp_vex_type)dp->p_vex;
    
    for (i = DTP_FIRSTSYM; i < DTP_FIRSTSYM + DTP_NUMSYMBOLS; i ++)
    {
        if (GetSymDescriptor((_UCHAR)i, 0, &let_descr, dp) < 0) loc = 0;
        else
        {
            HWRMemCpy(&(*vexbuf)[i-DTP_FIRSTSYM][0], let_descr->var_vexs, sizeof(let_descr->var_vexs));
            loc = let_descr->num_vars;
        }
        
        if (GetSymDescriptor((_UCHAR)i, (_UCHAR)loc, &let_descr, dp) >= 0)
        {
            HWRMemCpy(&(*vexbuf)[i-DTP_FIRSTSYM][loc], let_descr->var_vexs,
                      sizeof(let_descr->var_vexs)-loc*(sizeof(let_descr->var_vexs[0])));
        }
    }
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/*        Return Flag allowing this proto be used as one with opp cap        */
/* ************************************************************************* */
_INT  GetVarRewcapAllow(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID )
{
    _INT                  numv;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    
    if ((numv = GetSymDescriptor((_UCHAR)DtpOSToRec(chIn, langID ), nv, &let_descr, dp)) < 0)
        goto err;
    
    return (let_descr->var_veis[numv] & (_UCHAR)(DTP_CAP_BIT)) ? 0 : 1;
err:
    return -1;
}

/* ************************************************************************* */
/*        Set fly learn counter value                                        */
/* ************************************************************************* */
_INT  SetVarCounter(_UCHAR chIn, _UCHAR nv, _UCHAR cnt, p_VOID dtp, _INT langID )
{
    p_dtp_sym_header_type let_descr;
    _UCHAR                ch;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    p_dtp_vex_type        vexbuf;
    
    if (GetSymDescriptor((_UCHAR)DtpOSToRec(chIn, langID ), nv, &let_descr, dp) < 0)
        goto err;
    
    if ((vexbuf = (p_dtp_vex_type)dp->p_vex) == _NULL)
        goto err;
    
    ch = (_UCHAR)DtpOSToRec(chIn, langID );
    
    (*vexbuf)[ch-DTP_FIRSTSYM][nv] &= 0x7;
    (*vexbuf)[ch-DTP_FIRSTSYM][nv] |= (_UCHAR)((cnt & 0x1F) << 3);
    
    return 0;
err:
    return 1;
}

/* ************************************************************************* */
/*        Return Group number for a prototype                                */
/* ************************************************************************* */
_INT  GetVarGroup(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID )
{
    _INT                  numv;
    _SHORT                vargroup;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    
#define GROUP_MASK  0x0007
#define GROUP_SHIFT      1
    
    if ((numv = GetSymDescriptor((_UCHAR)DtpOSToRec(chIn, langID ), nv, &let_descr, dp)) < 0)
        goto err;
    
    vargroup = let_descr->var_veis[numv];
    vargroup = (_SHORT)((vargroup >> GROUP_SHIFT) & GROUP_MASK);
    
    return vargroup;
err:
    return -1;
}

/* ************************************************************************* */
/*        Return Group number for a corresponding cap pair                   */
/* ************************************************************************* */

_INT GetPairCapGroup(_UCHAR let, _UCHAR groupNum, _UCHAR EnableVariantSet, _INT langID )
{
    _INT            i;
    _UCHAR          sym;
    cap_table_type *Small2Cap;
    _BOOL           Cap = 0;
    _INT            vargroup;
    RECO_DATA_EXTERNAL _UCHAR DefaultSmall2Cap[CAP_TABLE_NUM_LET][CAP_TABLE_NUM_VAR];
    RECO_DATA_EXTERNAL _UCHAR PalmerSmall2Cap[CAP_TABLE_NUM_LET][CAP_TABLE_NUM_VAR];
    RECO_DATA_EXTERNAL _UCHAR BlockSmall2Cap[CAP_TABLE_NUM_LET][CAP_TABLE_NUM_VAR];
    
    
    switch(EnableVariantSet)
    {
        case WW_GENERAL:
            Small2Cap = &DefaultSmall2Cap;
            break;
        case WW_PALMER:
            Small2Cap = &PalmerSmall2Cap;
            break;
        case WW_BLOCK:
            Small2Cap = &BlockSmall2Cap;
            break;
        default:
            goto err;
    }
    
    if (groupNum >= CAP_TABLE_NUM_VAR) goto err;
    sym = (_UCHAR)ToLower(let);
    if (sym < 'a')
        goto err;
    if (sym != let)
        Cap = 1;
    
    if ( IsUmlautSupport( langID ) )
    {
        switch(sym)
        {
            case OS_a_umlaut :
                sym = ('z'-'a') + 1;
                break;
            
            case OS_o_umlaut :
                sym = ('z'-'a') + 2;
                break;


            case OS_u_umlaut:
                sym = ('z'-'a') + 4;
                break;
                
            case OS_a_angstrem :
                if ( langID == LANGUAGE_FINNISH || langID == LANGUAGE_NORWEGIAN ||
                    langID == LANGUAGE_SWEDISH || langID == LANGUAGE_DANISH )
                {
                    sym = ('z'-'a') + 3;
                    break;
                }
                
            default:
                if (sym > 'z')
                    goto err;
                sym -= 'a';
                break;
        }
    }
    else
    {
        sym -= 'a';
    }
    
    if (sym >= CAP_TABLE_NUM_LET)
        goto err;
    
    if (!Cap)
    {
        vargroup = (*Small2Cap)[sym][groupNum];
        if (vargroup < CAP_TABLE_NUM_VAR)
            goto done;
        else
            goto err;
    }
    
    for(i = 0; i < CAP_TABLE_NUM_VAR && (*Small2Cap)[sym][i] != groupNum; i++)
        ;
    if ( i == CAP_TABLE_NUM_VAR)
        goto err;
    else
        vargroup = i;
    
done:
    return vargroup;
err:
    return -1;
}

#if 0

/* ************************************************************************* */
/*        Temp function to tell DICT the length of each letter               */
/* ************************************************************************* */

_VOID LetXrLength(p_UCHAR min, p_UCHAR max, _SHORT let,  p_VOID dtp, _INT langID)
{
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    
    if (GetSymDescriptor((_UCHAR)DtpOSToRec(let, langID ), 0, &let_descr, dp) < 0)
        goto err;
    
    *min = let_descr->ave_len;
    *max = let_descr->ave_len;
    
    return;
err:
    *min = 1;
    *max = 3;
    return;
}
#endif /* if 0 */

/* ************************************************************************* */
/*        Return CHL position value for variant                              */
/* ************************************************************************* */

_INT  GetVarPosSize(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID )
{
    _INT                 num_vars;
    _INT                 varpos;
    _UCHAR               ch;
    let_table_type  _PTR letxr_tabl;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type     dp = (p_dtp_descr_type)dtp;
    
    if (dp == _NULL) goto err;
    
    ch          = (_UCHAR)DtpOSToRec(chIn, langID );
    let_descr   = _NULL;
    
    if (dp->p_ram_dtp != _NULL) // Is RAM dtp present?
    {
        letxr_tabl = (p_let_table_type)dp->p_ram_dtp;
        if ((*letxr_tabl)[ch] != 0l)
            let_descr = (p_dtp_sym_header_type)(dp->p_ram_dtp + (*letxr_tabl)[ch]);
    }
    
    if (let_descr == _NULL && dp->p_dtp != _NULL) // Is ROM dtp present?
    {
        letxr_tabl = (p_let_table_type)dp->p_dtp;
        if ((*letxr_tabl)[ch] != 0l)
            let_descr = (p_dtp_sym_header_type)(dp->p_dtp + (*letxr_tabl)[ch]);
    }
    
    if (let_descr == _NULL) goto err;
    num_vars  = let_descr->num_vars;
    
    if (nv >= num_vars) goto err; /* No such numbered variant in this letter */
    
    varpos    = let_descr->var_pos[nv];
    varpos   |= let_descr->var_size[nv] << 8;
    if (varpos) varpos |= let_descr->loc_vs_border << 16; // Temp preserve -- if not defined data in DTE, let it be 0 (for later checks)
    
    return (_INT)(varpos);
err:
    return -1;
}

/* ************************************************************************** */
/* *  Convert symbol from OS coding to REC internal                         * */
/* ************************************************************************** */

//  If either rcv==_NULL or xrdv==_NULL, then "interTable" value
// is used to set current charset.
//  Otherwise "interTable" is ignored, and the new settings are
// defined by current settings and xrdata ("xrdv"):

_INT SetInternationalCharSet( _INT interTable, p_VOID rcv, p_VOID xrdv )
{
    _INT              oldInter = TBL_AMERICAN;
#if HWR_SYSTEM == MACINTOSH
    p_rc_type         rc     = (p_rc_type)rcv;
#endif
    xrdata_type _PTR  xrdata = (xrdata_type _PTR)xrdv;
    p_xrd_el_type     xrd    = (p_xrd_el_type)xrdata->xrd;
    
    //Find out what the current settings are:
#if HWR_SYSTEM == MACINTOSH
    if  ( rc->alpha_charset == alpha_charset_swe )
        oldInter = TBL_INTERNATIONAL;
    else  if  ( rc->alpha_charset == alpha_charset_swe_nodiacr )
        oldInter = TBL_INTER_NODIACR;
#endif /* HWR_SYSTEM == MACINTOSH */
    
    //  For international settings define whether
    // to use non-diacritical charset:
    if  ( xrdv != _NULL )
    {
        if  ( oldInter == TBL_INTERNATIONAL )
        {
            _INT  i;
            for ( i=0; i < xrdata->len && xrd[i].xr.type != 0;  i ++ )
            {
                if  ( xrd[i].xr.type == X_ST    ||
                     xrd[i].xr.type == X_XT_ST ||
                     xrd[i].xr.type == X_UMLAUT  )
                {
                    if  ( xrd[i].xr.height >= 7 )
                        goto  EXIT_ACTIONS; //there is a high point - do standard processing.
                }
            }
            interTable = TBL_INTER_NODIACR;
        }
        else
            goto  EXIT_ACTIONS;
    }
    
    //
#if HWR_SYSTEM == MACINTOSH
    if  ( interTable == TBL_INTERNATIONAL )
        rc->alpha_charset = alpha_charset_swe;
    else  if  ( interTable == TBL_INTER_NODIACR )
        rc->alpha_charset = alpha_charset_swe_nodiacr;
    else
        rc->alpha_charset = alpha_charset_eng;
#endif /* HWR_SYSTEM == MACINTOSH */
    
    
EXIT_ACTIONS:;
    return  oldInter;
}


/* ************************************************************************** */
/* *  Convert symbol from REC internal coding to OS                         * */
/* ************************************************************************** */
/*
static _INT RecToOS( _INT sym, _INT langID )
{
    
    if (sym >= ST_CONV_RANGE)
    {
        _INT  i;
        _INT s = sym;
        
        sym = 0;
        
        for (i = 0; i < CONV_TBL_SIZE; i ++)
        {
            if (s == (_INT)os_rec_ctbl_let[langID][i][1])
            {
                sym = (_INT)os_rec_ctbl_let[langID][i][0];
                break;
            }
        }
    }
    return sym;
}
*/

/* ************************************************************************* */
/*        Return Shift value for letter variant,n-proto, n-xr                */
/* ************************************************************************* */

_INT  GetShiftCorr(_UCHAR chIn, _UCHAR nv,_UCHAR nXr, _UCHAR nIn, p_VOID dtp, _INT langID )
{
    _INT                  numv, varlen, i, half, index, byte;
    p_dtp_sym_header_type let_descr;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    p_xrpp_type            varptr;
    
    if ((numv = GetSymDescriptor( (_UCHAR)DtpOSToRec( chIn, langID ), nv, &let_descr, dp)) < 0) goto err;
    
    varptr = (p_xrpp_type)((p_UCHAR)let_descr + sizeof(dtp_sym_header_type));
    
    for (i = 0; i < numv && i < DTP_MAXVARSPERLET; i ++)
        varptr += let_descr->var_lens[i];
    
    varlen = let_descr->var_lens[numv];
    
    if (nXr >= varlen) goto err;
    
    index = nIn/2;
    half =  nIn % 2;
    if (half == 0)
    {
        /*  Set the upper half.  */
        byte = varptr[nXr].xsc[index] ;
        byte &= 0xF0;
        byte >>= 4;
    }
    else
    {
        /*  Set the lower half.  */
        byte = varptr[nXr].xsc[index];
        byte &= 0x0F;
    }
    
    return byte;
err:
    return -1;
}

/* ************************************************************************* */
/*        Return AutoCorr value for letter variant                           */
/* ************************************************************************* */

#if HWR_SYSTEM != MACINTOSH

_INT GetAutoCorr(_UCHAR chIn, _UCHAR nv, p_VOID dtp, _INT langID )
{
    _INT                    numv, varlen, i, autoCorr = 0;
    p_dtp_sym_header_type   let_descr;
    p_dtp_descr_type        dp = (p_dtp_descr_type)dtp;
    p_xrpp_type             varptr;
    
    if ((numv = GetSymDescriptor((_UCHAR)DtpOSToRec( chIn, langID ), nv, &let_descr, dp)) < 0)
        goto err;
    
    varptr = (p_xrpp_type)((p_UCHAR)let_descr + sizeof(dtp_sym_header_type));
    
    for (i = 0; i < numv && i < DTP_MAXVARSPERLET; i ++)
        varptr += let_descr->var_lens[i];
    
    varlen = let_descr->var_lens[numv];
    for(i = 0, autoCorr = 0; i < varlen; i++)
    {
        if (varptr[i].penl != 0)
            autoCorr++;
    }
    
    return autoCorr * XRMC_DEF_CORR_VALUE;
err:
    return -1;
}

#endif

/* ************************************************************************* */
/*        Get Sym Descriptor Header Pointer and new Variant Number           */
/* ************************************************************************* */

_INT GetSymDescriptor(_UCHAR sym, _UCHAR numv, p_dtp_sym_header_type _PTR psfc, p_VOID dtp)
{
    _INT                  n, nv;
    _INT                  lrom = 0;
    p_dtp_sym_header_type sfc = _NULL;
    p_let_table_type      plt;
    p_dtp_descr_type      dp = (p_dtp_descr_type)dtp;
    
    if (dp == _NULL)
        goto err;
    
    for (n = 0, nv = 0; n < 2; n ++) // Cycle by DTP change
    {
        sfc = _NULL;
        
        if (n == 0 && dp->p_dtp != _NULL)     // Is ROM dtp present?
        {
            plt = (p_let_table_type)dp->p_dtp;
            if ((*plt)[sym] != 0l) sfc = (p_dtp_sym_header_type)(dp->p_dtp + (*plt)[sym]);
            if (sfc == _NULL)
                continue; // Pointer was 0!, symbol not defined
        }
        
        if (n == 1 && dp->p_ram_dtp != _NULL) // Is RAM dtp present?
        {
            plt = (p_let_table_type)dp->p_ram_dtp;
            if ((*plt)[sym] != 0l)
                sfc = (p_dtp_sym_header_type)(dp->p_ram_dtp + (*plt)[sym]);
            lrom = nv;
        }
        
        if (sfc == _NULL)
            continue;
        if (sfc->num_vars == 0)
            continue;
        
        nv += sfc->num_vars;
        if (nv > DTP_MAXVARSPERLET)
            goto err;
        
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

#if DUMP_DTP

/* ************************************************************************* */
/*        Dump DTP to C file                                                 */
/* ************************************************************************* */

_INT DumpDtpToC( const char * name, _INT dtp_len, p_dtp_descr_type dtp_descr )
{
    _ULONG    i, len;
    p_ULONG ptr;
    FILE *  file;
    
    if ((file = fopen( name, "wt")) != _NULL)
    {
        fprintf(file, "// **************************************************************************\n");
        fprintf(file, "// *    Generated DTI file                                                  *\n");
        fprintf(file, "// **************************************************************************\n");
        
        fprintf(file, "\n#include \"ams_mg.h\"  \n");
        fprintf(file, "#include \"dti.h\"  \n\n");
        
        fprintf(file, "// ****   DTI body   ********************************************************\n");
        
        len = dtp_len/4; // len was in bytes
        fprintf(file, "RECO_DATA _ULONG img_dti_body[%d] =  \n", len + 1);
        fprintf(file, " {  \n");
        for (i = 0, ptr = (p_ULONG)dtp_descr->p_dtp; i < len; i ++, ptr ++)
        {
            fprintf(file, "0x%08X", *ptr);
            if (i < len-1) fprintf(file, ", ");
            if (i%8 == 7)  fprintf(file, "\n");
        }
        fprintf(file, " }; \n\n");
        
        fprintf(file, "// ****   DTI header   ******************************************************\n\n");
        
        fprintf(file, "RECO_DATA dti_descr_type img_dti_header =  \n");
        fprintf(file, " { \n");
        fprintf(file, "    {\"%s\"},    \n", dtp_descr->dtp_fname);
        fprintf(file, "    {'%c','%c','%c',0x%02X},    \n", dtp_descr->object_type[0], dtp_descr->object_type[1], dtp_descr->object_type[2], dtp_descr->object_type[3]);
        fprintf(file, "    {'%c','%c','%c',0x%02X},    \n", dtp_descr->type[0], dtp_descr->type[1], dtp_descr->type[2], dtp_descr->type[3]);
        fprintf(file, "    {'%c','%c','%c',0x%02X},    \n", dtp_descr->version[0], dtp_descr->version[1], dtp_descr->version[2], dtp_descr->version[3]);
        
        fprintf(file, "    0,                          // h_dte  \n");
        //fprintf(file, "  (p_UCHAR)&img_dti_body[0],    // p_dte  \n");
        fprintf(file, "    0,                          // p_dte  \n");
        
        fprintf(file, "    0,                          // h_ram_dte \n");
        fprintf(file, "    0,                          // p_ram_dte \n");
        
        fprintf(file, "    %d,                         // cheksum   \n", dtp_descr->dtp_chsum);
        
        fprintf(file, "    0,                          // h_vex     \n");
        fprintf(file, "    0,                          // p_vex     \n");
        
        fprintf(file, "    0,                          // h_xrt     \n");
        fprintf(file, "    0,                          // p_xrt     \n");
        fprintf(file, "    0,                          // cheksum   \n");
        
        fprintf(file, "    0,                          // h_pdf     \n");
        fprintf(file, "    0,                          // p_pdf     \n");
        fprintf(file, "    0,                          // p_ram_pdf \n");
        fprintf(file, "    0,                          // cheksum   \n");
        
        fprintf(file, "    0,                          // h_pict    \n");
        fprintf(file, "    0,                          // p_pict    \n");
        fprintf(file, "    0,                          // p_ram_pict\n");
        fprintf(file, "    0                           // cheksum   \n");
        
        fprintf(file, " }; \n\n");
        
        
        fprintf(file, "// **************************************************************************\n");
        fprintf(file, "// *    END OF GENERATED FILE                                               *\n");
        fprintf(file, "// **************************************************************************\n");
        
        fclose(file);
        //    err_msg("DTI output to dti_img.cpp.");
    }
    
    return 0;
}

#endif
