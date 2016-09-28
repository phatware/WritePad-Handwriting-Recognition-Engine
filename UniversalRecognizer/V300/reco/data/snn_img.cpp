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

#include "ams_mg.h"
#include "snn.h"
#include "langid.h"
#include "mlp.h"

#if MLP_FAT_NET
#include "snnimgef.cpp"
#if defined(LANG_FRENCH) || defined(LANG_ITALIAN) || defined(LANG_PORTUGUESE)
#include "snnimgff.cpp"
#endif
#if defined(LANG_GERMAN)
#include "snnimggf.cpp"
#endif
#if defined(LANG_NORW) || defined(LANG_DAN) || defined(LANG_SPANISH) || defined(LANG_PORTUGUESE) || defined(LANG_PORTUGUESEB) || defined(LANG_DUTCH) || defined(LANG_SWED) || defined(LANG_FINNISH) || defined(LANG_INTERNATIONAL) || defined(LANG_INDONESIAN)
#include "snnimgif.cpp"
#endif
#else // MLP_FAT_NET
#include "snn_imge.cpp"
#if defined(LANG_FRENCH) || defined(LANG_ITALIAN) || defined(LANG_PORTUGUESE)
#include "snn_imgf.cpp"
#endif
#if defined(LANG_GERMAN)
#include "snn_imgg.cpp"
#endif
#if defined(LANG_NORW) || defined(LANG_DAN) || defined(LANG_SPANISH) || defined(LANG_PORTUGUESE) || defined(LANG_PORTUGUESEB) || defined(LANG_DUTCH) || defined(LANG_SWED) || defined(LANG_FINNISH) || defined(LANG_INTERNATIONAL) || defined(LANG_INDONESIAN)
#include "snn_imgi.cpp"
#endif
#endif

RECO_DATA p_mlp_net_type GetNetData(_INT lang)
{
    switch(lang)
    {
#if defined(LANG_FRENCH) || defined(LANG_ITALIAN) || defined(LANG_PORTUGUESE)
        case LANGUAGE_FRENCH:
        case LANGUAGE_ITALIAN:
        case LANGUAGE_PORTUGUESE:
            return (RECO_DATA p_mlp_net_type)&img_snet_body_frn;
#endif
#if defined(LANG_GERMAN)
        case LANGUAGE_GERMAN:
            return (RECO_DATA p_mlp_net_type)&img_snet_body_ger;
#endif
#if defined(LANG_NORW) || defined(LANG_DAN) || defined(LANG_SPANISH) || defined(LANG_PORTUGUESE) || defined(LANG_PORTUGUESEB) || defined(LANG_DUTCH) || defined(LANG_SWED) || defined(LANG_FINNISH) || defined(LANG_INTERNATIONAL) || defined(LANG_INDONESIAN)
        case LANGUAGE_NORWEGIAN :
        case LANGUAGE_DANISH :
        case LANGUAGE_SPANISH :
        case LANGUAGE_PORTUGUESEB :
        case LANGUAGE_DUTCH :
        case LANGUAGE_INDONESIAN :
        case LANGUAGE_SWEDISH :
        case LANGUAGE_FINNISH ://uses international nn, but ignores a_amstrome/a_umlaut score
            return (RECO_DATA p_mlp_net_type)&img_snet_body_int;
#endif
        default:
            return (RECO_DATA p_mlp_net_type)&img_snet_body_eng;
    }
}
