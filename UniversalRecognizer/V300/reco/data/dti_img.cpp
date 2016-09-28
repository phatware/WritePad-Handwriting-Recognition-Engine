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

#if defined(DTI_LOAD_FROM_EXTERNAL_ROM)
#include "ams_mg.h"
#endif // DTI_LOAD_FROM_EXTERNAL_ROM
#include "dti.h"
#include "langid.h"

#ifndef LSTRIP
#if defined(DTI_LOAD_FROM_EXTERNAL_ROM)
RECO_DATA p_ULONG img_dti_body = _NULL;
RECO_DATA dti_descr_type dti_descr;
#else
#if DTI_COMPRESSED
#include "dti_imgc.cpp"
#if defined(LANG_FRENCH) || defined(LANG_ITALIAN) || defined(LANG_PORTUGUESE)
#include "dtiimgcf.cpp"
#endif
#if defined LANG_GERMAN
#include "dtiimgcg.cpp"
#endif
#if defined(LANG_NORW) || defined(LANG_DAN) || defined(LANG_SPANISH) || defined(LANG_PORTUGUESE) || defined(LANG_PORTUGUESEB) || defined(LANG_DUTCH) || defined(LANG_SWED) || defined(LANG_FINNISH) || defined(LANG_INTERNATIONAL) || defined(LANG_INDONESIAN)
#include "dtiimgci.cpp"
#endif
#else //DTI_COMPRESSED
#include "dti_imgf.cpp"
#if defined(LANG_FRENCH) || defined(LANG_ITALIAN) || defined(LANG_PORTUGUESE)
#include "dtiimgff.cpp"
#endif
#if defined LANG_GERMAN
#include "dtiimgfg.cpp"
#endif
#if defined(LANG_DAN) || defined(LANG_NORW)
#include "dti_imgdan.cpp"
#endif
#endif //DTI_COMPRESSED
#endif //DTI_LOAD_FROM_EXTERNAL_ROM
#endif //LSTRIP

RECO_DATA p_dti_descr_type dti_get_header(_INT lang)
{
#if defined(DTI_LOAD_FROM_EXTERNAL_ROM)
    if (dti_descr.language != lang)
        return NULL;
    return &dti_descr;
#else
    switch(lang)
    {
#if defined(LANG_FRENCH) || defined(LANG_ITALIAN) || defined(LANG_PORTUGUESE)
        case LANGUAGE_FRENCH:
        case LANGUAGE_ITALIAN:
        case LANGUAGE_PORTUGUESE:
            img_dti_header_frn.language = lang;
            return (RECO_DATA p_dti_descr_type)&img_dti_header_frn;
#endif
#if defined LANG_GERMAN
        case LANGUAGE_GERMAN:
            img_dti_header_ger.language = lang;
            return (RECO_DATA p_dti_descr_type)&img_dti_header_ger;
#endif
#if defined(LANG_DAN) || defined(LANG_NORW)
        case LANGUAGE_DANISH:
        case LANGUAGE_NORWEGIAN:
            img_dti_header_dan.language = lang;
            return (RECO_DATA p_dti_descr_type)&img_dti_header_dan;
#endif
        default:
            img_dti_header_eng.language = lang;
            return (RECO_DATA p_dti_descr_type)&img_dti_header_eng;
    }
#endif
}

RECO_DATA p_ULONG dti_get_body(_INT lang)
{
#if defined(DTI_LOAD_FROM_EXTERNAL_ROM)
    if (dti_descr.language != lang || img_dti_body == NULL)
        return _NULL;
    return img_dti_body;
#else
    switch(lang)
    {
#if defined(LANG_FRENCH) || defined(LANG_ITALIAN) || defined(LANG_PORTUGUESE)
        case LANGUAGE_FRENCH:
        case LANGUAGE_ITALIAN:
        case LANGUAGE_PORTUGUESE:
            return (RECO_DATA p_ULONG)&img_dti_body_frn[0];
#endif
#if defined LANG_GERMAN
        case LANGUAGE_GERMAN:
            return (RECO_DATA p_ULONG)&img_dti_body_ger[0];
#endif
#if defined(LANG_DAN) || defined(LANG_NORW)
        case LANGUAGE_DANISH:
        case LANGUAGE_NORWEGIAN:
            return (RECO_DATA p_ULONG)&img_dti_body_dan[0];
#endif
        default:
            return (RECO_DATA p_ULONG)&img_dti_body_eng[0];
    }
#endif
}

RECO_DATA _INT dti_set_external_data(_INT lang, p_CHAR data)
{
#if defined(DTI_LOAD_FROM_EXTERNAL_ROM)
    _INT result = 0;
    HWRMemSet(&dti_descr, 0, sizeof(dti_descr));
    
    p_dti_header_type img_dti_header = (p_dti_header_type)data;
    HWRMemCpy(dti_descr.object_type, img_dti_header->object_type, DTI_ID_LEN);
    HWRMemCpy(dti_descr.type, img_dti_header->type, DTI_ID_LEN);
    HWRMemCpy(dti_descr.version, img_dti_header->version, DTI_ID_LEN);
    dti_descr.language = lang;
    img_dti_body = (p_ULONG)(data + img_dti_header->dte_offset);
    
    _ULONG dte_i, dte_chsum;
    for (dte_i = 0l, dte_chsum = 0l; dte_i < img_dti_header->dte_len; dte_i ++)
        dte_chsum += ((p_UCHAR)(img_dti_body))[dte_i];
    if (dte_chsum == img_dti_header->dte_chsum)
        result = 1;
    return result;
#else
    return 0;
#endif
}
