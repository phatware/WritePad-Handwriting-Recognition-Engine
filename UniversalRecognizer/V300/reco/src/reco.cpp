/***************************************************************************************
 *
 *  WRITEPAD(r): Handwriting Recognition Engine (HWRE) and components.
 *  Copyright (c) 2001-2017 PhatWare (r) Corp. All rights reserved.
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
#include "zctype.h"

#include "ams_mg.h"
#include "lowlevel.h"
#include "xrword.h"
#include "ldbtypes.h"
#include "xrlv.h"
#include "pydict.h"
#include "ws.h"

#include "precutil.h"
#include "recoutil.h"
#include "recokern.h"
#include "reco.h"
#include "ligstate.h"
#include "dti_lrn.h"

#include "post_capital.h" //okjeong 10/11/11

#include "recotypes.h"
#include <string.h>


//#include "DioHwr.h"
// -------------------- Defines and globals --------------------------


#define RECO_VER_ID     "WritePad Engine 5.6"

#define RECO_LANG_STRING "ALL-"

#ifdef DTI_COMPRESSED_ON
#define RECO_DTI_STRING "C"
#else
#define RECO_DTI_STRING "U"
#endif

#ifdef SNN_FAT_NET_ON
#define RECO_MLP_STRING "F"
#else
#define RECO_MLP_STRING "S"
#endif

#define RECO_ID_STRING RECO_VER_ID RECO_LANG_STRING RECO_DTI_STRING RECO_MLP_STRING

extern int UNICODEtoStr(char * str, CUCHR * tstr, int cMax);

#define PR_ANSW_EXTALLOC         256
#define PR_TOOMANYWORDSWAITING   5
#define PR_TENTATIVELIST_SIZE    10

#define WST_DEF_H_LINE           80
#define WST_DEF_S_DELAY          3
#define WST_DEF_UNSURE_LEVEL     50

// ------------------------- Structures -------------------------------------

typedef struct
{
    _INT nword;
    _INT nparts;
    _INT len;
} tentative_list_type, *p_tentative_list_type;

typedef struct
{
    _INT ok;
    _INT flags;
    _INT sp_vs_q;

    rc_type rc;
    xrdata_type xrdata;
    RWG_type rwg;

    _INT g_stroke_num;

    word_strokes_type w_str[WS_MAX_WORDS];
    ws_results_type wsr;
    ws_word_info_type wswi;
    ink_info_type ink_info;
    ws_control_type wsc;
    RECO_baseline_type baseline;
    tentative_list_type tentative_list[PR_TENTATIVELIST_SIZE];
    _INT num_tentative_words;

    tentative_list_type unfinished_data; // Storage of infinished recognition attr

    _INT rr_alloc_size; // Allocated Results buffer
    _INT rr_filled_size; // Butes used there
    _INT rr_num_answers; // Num of all registered answers
    _INT rr_num_finished_answers; // Num of non-tentative answers
    _INT rr_nparts; // Number of parts in last recognized and registered answer
    p_UCHAR recres;

    vocptr_type main_dict;
    vocptr_type pref_dict;
    vocptr_type suff_dict;
    vocptr_type user_dict;

    info_func_type InfoCallBack;
    p_VOID ICB_param;

    p_dti_descr_type p_dtih;
    tr_descr_type p_trh;


#ifdef HW_RECINT_UNICODE
    UCHR uans_buf[w_lim];
#endif
} rec_inst_type, _PTR p_rec_inst_type;

// ------------------------- Prototypes -------------------------------------

static _INT HWR_RecWord(p_rec_inst_type pri);
static _INT HWR_RecInit(p_rec_inst_type _PTR pri, int language );
static _INT HWR_RecClose(p_rec_inst_type _PTR pri);
static _INT HWR_AddToAnsw(p_rec_inst_type pri, p_UCHAR answ, p_INT weights, _INT ns, p_INT stroke_ids);
static _INT HWR_RegNewAnsw(p_rec_inst_type pri, _INT er);
static _INT HWR_ResetTentativeStorage(_INT st_index, p_rec_inst_type pri);
static _INT HWR_ValidateNextTentativeWord(_INT nparts, p_rec_inst_type pri);
static _INT HWR_CleanUpContext(p_rec_inst_type pri);
static p_INT HWR_GetAnswerBlockPtr(_INT nw, p_rec_inst_type pri);

RECO_DATA_EXTERNAL _ULONG img_voc[];
RECO_DATA_EXTERNAL _ULONG img_vocpref[];
RECO_DATA_EXTERNAL _ULONG img_vocsuff[];
RECO_DATA_EXTERNAL _UCHAR sp_vs_q_ts[];
RECO_DATA_EXTERNAL _UCHAR sp_vs_q_bd[];
RECO_DATA_EXTERNAL _UCHAR num_charset[];
RECO_DATA_EXTERNAL _UCHAR math_charset[];

static char gError[HW_MAXWORDLEN] = "<--->";
const _ULONG _PTR GetLDBImgBody(_INT lang, _INT index);

/* *************************************************************** */
/* *  Get info string and capabilities of the recognizer         * */
/* *************************************************************** */
void RecoSetErrorText(CUCHR * pErrorText)
{
    if (pErrorText)
    {
        if (0 == *pErrorText)
            gError[0] = 0;
        else
        {
#ifdef HW_RECINT_UNICODE
            UNICODEtoStr( (char *)gError, pErrorText, HW_MAXWORDLEN-1);
#else
            HWRStrnCpy((_STR) gError, (_STR) pErrorText, HW_MAXWORDLEN - 1);
#endif // HW_RECINT_UNICODE
            gError[HW_MAXWORDLEN - 1] = 0;
        }
    }
    else
    {
        HWRStrCpy((_STR) gError, (_STR) "<--->");
    }
}

/* *************************************************************** */
/* *  Get info string and capabilities of the recognizer         * */
/* *************************************************************** */
int RecoGetRecID(p_RECO_ID_type p_inf)
{
    p_UCHAR idstr = (p_UCHAR) RECO_ID_STRING;

    if (p_inf == 0)
        goto err;

    p_inf->capabilities = HW_CPFL_CURS | HW_CPFL_SPVSQ;

#if defined LANG_INTERNATIONAL
    p_inf->capabilities |= HW_CPFL_INTER;
#endif

    HWRStrCpy( p_inf->id_string, (_CSTR) idstr);

    return HW_RECINT_ID_001;
    err: return 0;
}

int RecoGetRecLangID( RECOCTX context )
{
    p_rec_inst_type pri = (p_rec_inst_type) (context);
    int lang = pri->rc.lang;
    return lang;
}

/* *************************************************************** */
/* *  Create context for recognition                             * */
/* *************************************************************** */
RECOCTX RecoCreateContext( int language )
{
    p_rec_inst_type pri = _NULL;

    if (!HWR_RecInit(&pri, language ) && pri != _NULL)
    {
        HWR_CleanUpContext(pri);
        return (RECOCTX) pri;
    }
    return _NULL;
}

/* *************************************************************** */
/* *  Close recognition context and free all the memory          * */
/* *************************************************************** */
int RecoCloseContext(RECOCTX context)
{
    p_rec_inst_type pri = (p_rec_inst_type) (context);

    if (pri != _NULL)
    {
        pri->InfoCallBack = NULL;
        RecoCloseSession(context);
        HWR_CleanUpContext(pri);
        HWR_RecClose(&pri);
        return 0;
    }
    return 1;
}

/* *************************************************************** */
/* *  Open recognition session and init context to default state * */
/* *************************************************************** */
int RecoOpenSession( p_RECO_control_type ctrl, RECOCTX context )
{
    p_rec_inst_type pri = (p_rec_inst_type) (context);
    // pri->lang = language;
    p_rc_type prc = &pri->rc;
    _INT lang = prc->lang;

    // ----------------- Absorb cotrol values --------------------------

    pri->flags = ctrl->flags;
    pri->sp_vs_q = (ctrl->sp_vs_q & 0xFF);
    pri->InfoCallBack = ctrl->InfoCallBack;
    pri->ICB_param = ctrl->ICB_param;

    // ---------------- RC defaults -------------------------------------

    prc->enabled_cs = CS_ALPHA | CS_NUMBER | CS_LPUNCT | CS_EPUNCT | CS_OTHER | CS_MATH;
    prc->enabled_ww = WW_PALMER | WW_BLOCK | WW_GENERAL;
    switch (lang)
    {
        case LANGUAGE_ENGLISH :
        case LANGUAGE_ENGLISHUK :
            prc->enabled_languages = EL_ENGLISH;
            break;
            
        case LANGUAGE_INDONESIAN :
            prc->enabled_languages = EL_ENGLISH;
            break;

        case LANGUAGE_FRENCH :
            prc->enabled_languages = EL_FRENCH;
            break;
            
        case LANGUAGE_ITALIAN :
            prc->enabled_languages = EL_ENGLISH|EL_GERMAN|EL_FRENCH;
            break;
            
        case LANGUAGE_GERMAN :
            prc->enabled_languages = EL_GERMAN;
            break;
            
        case LANGUAGE_SWEDISH:
        case LANGUAGE_NORWEGIAN:
        case LANGUAGE_DANISH:
        case LANGUAGE_FINNISH:
            prc->enabled_languages = EL_ENGLISH | EL_SWEDISH;
            break;
            
        default:
            prc->enabled_languages = EL_ENGLISH | EL_GERMAN | EL_FRENCH | EL_SWEDISH;
            break;
            
    }
    
    prc->algorithm = XRWALG_XR_SPL;
    prc->corr_mode = 0;
    prc->xrw_mode = XRWM_VOC | XRWM_LD | XRWM_CS | XRWM_TRIAD | XRWM_MWORD;
    prc->answer_level = 15;
    prc->sure_level = 85;
    prc->answer_allow = 30;
    prc->bad_amnesty = 12;
    prc->caps_mode = 255;
    prc->f_xd_data = XRLV_DATA_USE | XRLV_DATA_SAVE; // Enables XRLV continue mode

    prc->rec_mode = RECM_TEXT;
    prc->low_mode = LMOD_CHECK_PUNCTN | LMOD_BORDER_GENERAL | LMOD_FREE_TEXT;

    prc->ws_handle = (0 != (ctrl->flags & HW_RECFL_STATICSEG)) ? 6 : 0;
    // prc->ws_handle      = ((ctrl->sp_vs_q >> 8) & 0xFF); // 6; // 0; // <- auto WS segmentation with learning, was 6;

    //  prc->xrw_min_wlen    = 0;
    //  prc->xrw_max_wlen    = 0;
    //  prc->xrw_chr_size    = 0;
    //  prc->use_len_limits  = 0;
    //  prc->lrn_learn_suff  = 2;
    //  prc->lrn_vocvar_rew  = 5;
    //  prc->fly_learn       = 0;
    prc->use_vars_inf = 1;
    //  prc->lmod_border_used= 0;
    //  prc->fl_fil          = 0;
    //  prc->fl_post         = 0;
    //  prc->ws_handle       = 6;
    //  prc->FakeRecognition = 0;
    //  prc->fl_chunk        = 0;
    //  prc->fl_chunk_let    = 0;

    // ------------------ Modifiers & pointers ----------------------------

#ifdef INTERNAL_DICT
    pri->main_dict.hvoc_dir = (p_VOID)((img_voc[0] == 0) ? 0:(&img_voc[0]));
    pri->user_dict.hvoc_dir = ctrl->h_user_dict;
    pri->pref_dict.hvoc_dir = (p_VOID)((img_vocpref[0] == 0) ? 0:(&img_vocpref[0]));
    pri->suff_dict.hvoc_dir = (p_VOID)((img_vocsuff[0] == 0) ? 0:(&img_vocsuff[0]));
    // pri->pref_dict            = &img_vocpref_header; // Switch to internal dict
#else
    pri->main_dict.hvoc_dir = ctrl->h_main_dict;
    pri->user_dict.hvoc_dir = ctrl->h_user_dict;
    pri->pref_dict.hvoc_dir = (p_VOID) ((img_vocpref[0] == 0) ? 0 : (&img_vocpref[0]));
    pri->suff_dict.hvoc_dir = (p_VOID) ((img_vocsuff[0] == 0) ? 0 : (&img_vocsuff[0]));
#endif

    if (pri->sp_vs_q < 1)
        pri->sp_vs_q = 1;
    if (pri->sp_vs_q > 14)
        pri->sp_vs_q = 14;
    prc->xrw_tag_size = sp_vs_q_ts[pri->sp_vs_q];
    prc->bad_distance = sp_vs_q_bd[pri->sp_vs_q];

    if (pri->flags & HW_RECFL_DICTONLY)
        prc->xrw_mode &= ~(XRWM_LD | XRWM_CS | XRWM_TRIAD);
    if (pri->flags & HW_RECFL_SEPLET)
        prc->corr_mode = XRCM_SEPLET;

    // Anton: removed XRWM_LD to get rid of 'a' and 'p' in numeric mode

    // Anton: for the split-screen mode use only limited charsets
    /*
     if ( pri->flags & HW_RECFL_INTERNET )
     {
     prc->xrw_mode = XRWM_CS;
     prc->enabled_cs = CS_ADDRESS|CS_NUMBER|CS_ALPHA;
     }
     else
     */
    
    prc->num_charset = (p_UCHAR) num_charset;
    HWRStrnCpy((_STR)prc->math_charset, (_STR)math_charset, MAX_CUSTOM_SYM - 1);
    HWRStrnCpy((_STR)prc->other_charset, (_STR)other_charset[lang], MAX_CUSTOM_SYM - 1);
    HWRStrnCpy((_STR)prc->lpunct_charset, (_STR)lpunct_charset[lang], MAX_CUSTOM_SYM - 1);
    HWRStrnCpy((_STR)prc->epunct_charset, (_STR)epunct_charset[lang], MAX_CUSTOM_SYM - 1);
    prc->math_charset[MAX_CUSTOM_SYM - 1] = '\0';
    prc->other_charset[MAX_CUSTOM_SYM - 1] = '\0';
    prc->epunct_charset[MAX_CUSTOM_SYM - 1] = '\0';
    prc->epunct_charset[MAX_CUSTOM_SYM - 1] = '\0';
    
    if ( 0 != (pri->flags & HW_RECFL_CUSTOM) )
    {
        prc->xrw_mode = XRWM_CS;
        prc->enabled_cs = 0; // CS_NUMBER | CS_MATH;
        prc->other_charset[0] = '\0';
        prc->lpunct_charset[0] = '\0';
        prc->epunct_charset[0] = '\0';
        if ( ctrl->customNumbers )
        {
            prc->enabled_cs |= CS_NUMBER;
            // prc->xrw_mode |= CS_NUMBER;
            prc->num_charset = (p_UCHAR) ctrl->customNumbers;
        }
        else
        {
            prc->num_charset = (p_UCHAR)"";
        }
        if ( ctrl->customPunct )
        {
            prc->enabled_cs |= CS_MATH;
            // prc->xrw_mode |= CS_MATH;
            HWRStrnCpy((_STR)prc->math_charset, (_STR)ctrl->customPunct, MAX_CUSTOM_SYM - 1);
            prc->math_charset[MAX_CUSTOM_SYM - 1] = '\0';
        }
        else
        {
            prc->math_charset[0] = '\0';
        }
    }
    else if ( 0 != (pri->flags & HW_RECFL_CUSTOM_WITH_ALPHA) )
    {
        // prc->enabled_cs = CS_ALPHA | CS_NUMBER | CS_LPUNCT | CS_EPUNCT | CS_OTHER | CS_MATH;
        // prc->xrw_mode = XRWM_CS | XRWM_MWORD | XRWM_VOC | XRWM_TRIAD | CS_NUMBER | CS_MATH;
        if ( ctrl->customPunct )
        {
            RemoveDisabledSymbol(prc->math_charset, (p_UCHAR)ctrl->customPunct);
            RemoveDisabledSymbol(prc->lpunct_charset, (p_UCHAR)ctrl->customPunct);
            RemoveDisabledSymbol(prc->epunct_charset, (p_UCHAR)ctrl->customPunct);
            RemoveDisabledSymbol(prc->other_charset, (p_UCHAR)ctrl->customPunct);
            
            if ( HWRStrLen((_STR)prc->math_charset) < 1 )
            {
                prc->enabled_cs &= ~CS_MATH;
            }
            if ( HWRStrLen((_STR)prc->lpunct_charset) < 1 )
            {
                prc->enabled_cs &= ~CS_LPUNCT;
            }
            
            if ( HWRStrLen((_STR)prc->epunct_charset) < 1 )
            {
                prc->enabled_cs &= ~CS_EPUNCT;
            }
            if ( HWRStrLen((_STR)prc->other_charset) < 1 )
            {
                prc->enabled_cs &= ~CS_OTHER;
            }
        }
        if ( ctrl->customNumbers )
        {
            if ( 0 == *ctrl->customNumbers )
            {
                prc->enabled_cs &= ~(CS_NUMBER);
                prc->xrw_mode &= ~XRWM_LD;
//                prc->xrw_mode &= ~(XRWM_CS);
            }
            prc->num_charset = (p_UCHAR) ctrl->customNumbers;
        }
    }		
    
    if (pri->flags & HW_RECFL_NUMONLY)
    {
        prc->xrw_mode = /*XRWM_LD|*/XRWM_CS;
        if (pri->flags & HW_RECFL_PURE)
            prc->enabled_cs = CS_NUMBER;
        else
            prc->enabled_cs = CS_NUMBER | CS_OTHER | CS_MATH;
    }
    else if ((pri->flags & HW_RECFL_PURE) && (pri->flags & HW_RECFL_CAPSONLY))
    {
        prc->xrw_mode = XRWM_CS;
        prc->enabled_cs = CS_ALPHA;
    }
    else if ( pri->flags & HW_RECFL_ALPHAONLY )
    {
        prc->xrw_mode = XRWM_CS | XRWM_MWORD | XRWM_VOC | XRWM_TRIAD;
        if (pri->flags & HW_RECFL_DICTONLY)
            prc->xrw_mode &= ~(XRWM_LD | XRWM_CS | XRWM_TRIAD);
        if ( (pri->flags & HW_RECFL_PURE) )
            prc->enabled_cs = CS_ALPHA;
        else
            prc->enabled_cs = CS_ALPHA | CS_NUMBER | CS_OTHER;	
    }

    if (pri->main_dict.hvoc_dir != 0 && (prc->xrw_mode & XRWM_MWORD)) // Dictionary segmentation makes sense only in presence of dictionary!
    {
        prc->lrn_class_level = 94; //WS_SURE_LEVEL;
        prc->lrn_min_class = 94; //WS_SURE_LEVEL;
    }
    else
    {
        prc->lrn_class_level = 0; //WS_SURE_LEVEL;
        prc->lrn_min_class = 0; //WS_SURE_LEVEL;
    }

#if defined (LANG_INTERNATIONAL)
    if (0 != (pri->flags & HW_RECFL_INTL_CS))
        prc->alpha_charset = (p_UCHAR) alpha_charset;
    else
        prc->alpha_charset = (p_UCHAR) alpha_charset_eng;
#else
    prc->alpha_charset = (p_UCHAR)alpha_charset[lang];
#endif

    pri->wsc.num_points = 0;
    pri->wsc.flags = 0;
    pri->wsc.sure_level = prc->lrn_class_level; // WST_DEF_UNSURE_LEVEL;
    pri->wsc.word_dist_in = prc->ws_handle; //+1;
    pri->wsc.line_dist_in = 0;
    pri->wsc.def_h_line = WST_DEF_H_LINE;
    pri->wsc.x_delay = WST_DEF_S_DELAY; // 0; // Delay in 'letters' or 0 -- only on line end

    HWR_CleanUpContext(pri);

    pri->ok = 1;
    return 0;
}

/* *************************************************************** */
/* *  Recognizes incoming strokes and sends results as ready     * */
/* *************************************************************** */

int RecoCloseSession(RECOCTX context)
{
    int err = 1;
    p_rec_inst_type pri = (p_rec_inst_type) (context);

    if (pri == _NULL || !pri->ok)
        return err;

    err = RecoRecognize(0, 0, context);

    FreeInkInfo(&pri->ink_info);

    pri->g_stroke_num = 0;
    pri->baseline.size = 0;
    pri->num_tentative_words = 0;
    pri->ok = 0;
    return err;
}

/* *************************************************************** */
/* *  Recognizes incoming strokes and sends results as ready     * */
/* *************************************************************** */
int RecoRecognize(int npoints, p_RECO_point_type strokes_win,
        RECOCTX context)
{
    p_PS_point_type strokes = (p_PS_point_type) strokes_win;
    _INT i, j, k;
    _INT num_strokes, prev_nstrokes, len, f, cur_nstrokes;
    _INT er = 0, skip, min_next_word, cur_tt_word;
    PS_point_type _PTR p_tr = _NULL;
    PS_point_type _PTR stroke;
    p_rec_inst_type pri = (p_rec_inst_type) (context);

    if (pri == _NULL)
        goto err;
    if (!pri->ok)
        goto err;

    if ((p_tr = (PS_point_type _PTR) HWRMemoryAlloc(sizeof(PS_point_type)
            * (pri->ink_info.num_points + npoints + 4))) == _NULL)
        goto err;

    prev_nstrokes = num_strokes = pri->ink_info.num_strokes;
    cur_nstrokes = 0;

    pri->wsc.x_delay = WST_DEF_S_DELAY; // 0; // Delay in 'letters' or 0 -- only on line end
    if (pri->InfoCallBack != _NULL)
    {
        if ((*pri->InfoCallBack)(pri->ICB_param) == 1)
            pri->wsc.x_delay = 0; // Do not attempt segmentation to words if there are more strokes coming (save time)
    }

    if (npoints > 0) // --------- Add current stroke(s) to ink storage -----------------------
    {
        // ------------------- Label new strokes ------------------------------
        HWRMemCpy(p_tr, strokes, (1 + npoints) * sizeof(PS_point_type));
        len = npoints;
        if (p_tr[len - 1].y >= 0)
            p_tr[len++].y = -1; // Terminate stroke if was not terminated

        for (i = 0, stroke = p_tr; i < len; i++, stroke++)
        {
            if (stroke->y < 0)
                stroke->x = (_SHORT) (pri->g_stroke_num + cur_nstrokes++);
        }

        // ------------------- Put them in store ------------------------------
        num_strokes = CreateInkInfo(p_tr, len, &pri->ink_info);
        if (num_strokes == 0)
            goto err;

        if (num_strokes < 0) // Stroke storage is overfilled!!!
        {
            if (pri->wsr.num_finished_words == 0)
                goto err; // Can't help if nothing developed yet ...

            // ------- Get all unused strokes --------------------------

            for (i = len = 0; i < prev_nstrokes; i++)
            {
                for (j = f = 0; !f && j < pri->wsr.num_words; j++)
                {
                    if (((*pri->wsr.pwsa)[j].flags & WS_FL_PROCESSED) == 0)
                        continue;
                    for (k = 0; !f && k < (*pri->wsr.pwsa)[j].num_strokes; k++)
                    {
                        if (pri->wsr.stroke_index[k
                                + (*pri->wsr.pwsa)[j].first_stroke_index] == i)
                            f = 1;
                    }
                }
                if (f)
                    continue;

                GetInkStrokeCopy(i, p_tr + len, &pri->ink_info);
                len += GetInkStrokeLen(i, &pri->ink_info);
            }

            // ------- Destroy old store ------------------------------

            FreeInkInfo(&pri->ink_info);
            prev_nstrokes = num_strokes = 0;

            // ------------------- Label new strokes ------------------------------

            HWRMemCpy(p_tr + len, strokes, npoints * sizeof(PS_point_type));
            if (p_tr[len + npoints - 1].y >= 0)
                p_tr[len + (npoints++)].y = -1; // Terminate stroke if was not terminated
            for (i = cur_nstrokes = 0, stroke = p_tr + len; i < npoints; i++, stroke++)
            {
                if (stroke->y < 0)
                    stroke->x = (_SHORT) (pri->g_stroke_num + cur_nstrokes++);
            }
            len += npoints;

            // ------- Recreate stroke storage -------------------------

            if (len)
            {
                if ((num_strokes = CreateInkInfo(p_tr, len, &pri->ink_info))
                        <= 0)
                    goto err;
            }
            if (num_strokes <= 0)
                goto err;

            // ------- Close previous session of segmentation ----------

            pri->wsc.flags |= WS_FL_LAST;
            pri->wsc.num_points = 0;
            if (WordStrokes(_NULL, &pri->wsc, &pri->wsr))
                goto err;
            pri->wsr.num_words = pri->wsr.num_finished_words
                    = pri->wsr.num_finished_strokes = 0;
        }

        pri->g_stroke_num += cur_nstrokes;
    }

    // ---------------- Process Trace ----------------------------------------------

    if ((pri->flags & HW_RECFL_NSEG) == 0) // If segmentation enabled
    {
        // --------- Feed new info to segmentation ------------------------------

        for (i = prev_nstrokes; i <= num_strokes; i++)
        {
            if (i < num_strokes)
            {
                stroke = (p_PS_point_type) GetInkStrokePtr(i, &pri->ink_info);
                len = GetInkStrokeLen(i, &pri->ink_info);

                if (stroke == _NULL || len == 0)
                    goto err;

                pri->wsc.flags = 0; // WS_FL_SPGESTURE;
                pri->wsc.num_points = len;
                if (WordStrokes(stroke, &pri->wsc, &pri->wsr))
                    break;
            }
            else
            {
                if (npoints == 0)
                {
                    pri->wsc.flags |= WS_FL_LAST;
                    pri->wsc.num_points = 0;
                    WordStrokes(_NULL, &pri->wsc, &pri->wsr);
                }
            }
        }

        // -------------- Let's see if any new words resulted --------------------

        if (((pri->flags & HW_RECFL_NCSEG) == 0) || (pri->wsc.flags& WS_FL_LAST))
        {
            skip = 0; // If there are strokes waiting, let's get some more before recognizing
            if (pri->InfoCallBack != _NULL)
            {
                if ((*pri->InfoCallBack)(pri->ICB_param) == 1)
                    skip = 1;
            }

            for (i = k = 0; i < pri->wsr.num_words && i < WS_MAX_WORDS - 1; i++)
                if (((*pri->wsr.pwsa)[i].flags & WS_FL_PROCESSED) == 0)
                    k++;

            if (k > PR_TOOMANYWORDSWAITING)
                skip = 0; // Too many words in line, let's recognize them
            if (pri->wsc.flags & WS_FL_LAST)
                skip = 0; // Need to recognize on last

            if (!skip) // Let's recognize all ready words and try tentative one
            {
                cur_tt_word = min_next_word = 0;
                while ((len = GetNextWordInkCopy(RM_COMBINE_CARRY,
                        min_next_word, &pri->wsr, p_tr, &pri->ink_info,
                        &pri->wswi)) > 0)
                {
                    if (pri->wswi.flags & WS_FL_TENTATIVE)
                    {
                        if (!(pri->flags & HW_RECFL_TTSEG))
                            break; // If not allowed to work with tentative words

                        if (cur_tt_word < pri->num_tentative_words && // If there was appropriate tentative word, skip to next
                                pri->tentative_list[cur_tt_word].nword
                                        == pri->wswi.nword
                                && pri->tentative_list[cur_tt_word].len == len)
                        {
                            cur_tt_word++;
                            min_next_word = pri->wswi.nword + 1; // Advance to the next tentative word
                            continue;
                        }

                        pri->rc.pFuncYield = pri->InfoCallBack; // Allow interrupting of recognition
                        pri->rc.FY_param = pri->ICB_param;
                        HWR_ResetTentativeStorage(cur_tt_word, pri); // Tentative did not prove itself from this point, remove
                    }
                    else
                    {
                        if (pri->num_tentative_words > 0 && // If there was appropriate tentative word
                                pri->tentative_list[0].nword == pri->wswi.nword && pri->tentative_list[0].len == len)
                        {
                            if (HWR_ValidateNextTentativeWord( pri->tentative_list[0].nparts, pri))
                                break;
                            HWRMemCpy(&pri->tentative_list[0], &pri->tentative_list[1],
                                    sizeof(tentative_list_type) * (PR_TENTATIVELIST_SIZE - 1));
                            pri->num_tentative_words--;
                            continue;
                        }
                        else
                        {
                            HWR_ResetTentativeStorage(0, pri); // Tentative did not prove itself from this point, remove
                        }

                        pri->rc.pFuncYield = _NULL; // No interruption on real word recognition
                    }

                    if (pri->rc.p_xd_data) // If there is unfinished word, restore XD
                    {
                        if (pri->unfinished_data.nword == pri->wswi.nword
                                && pri->unfinished_data.len == len)
                        {
                            // If word matches current segmentation, use prev XD.
                        }
                        else
                        {
                            // If word does not match, release unfinished context
                            XrlvDealloc( (p_xrlv_data_type _PTR) &pri->rc.p_xd_data);
                        }
                    }

                    pri->rc.trace = p_tr;
                    pri->rc.ii = (_SHORT) len;

                    er = HWR_RecWord(pri); // < ------ Call recognizer

                    if (pri->rc.p_xd_data) // Save XD for possible next do-recognition
                    {
                        pri->unfinished_data.nword = pri->wswi.nword;
                        pri->unfinished_data.len = len;
                    }

                    if (er == XRLV_YIELD_BREAK)
                        break; // Recognition was aborted

                    if (pri->wswi.flags & WS_FL_TENTATIVE)
                    {
                        i = pri->num_tentative_words;
                        if (i >= PR_TENTATIVELIST_SIZE)
                            break; // If too many tentative words, enough!
                        pri->tentative_list[i].nword = pri->wswi.nword;
                        pri->tentative_list[i].nparts = pri->rr_nparts;
                        pri->tentative_list[i].len = len;
                        pri->num_tentative_words++;
                        cur_tt_word++;

                        min_next_word = pri->wswi.nword + 1; // Advance to the next tentative word
                    }

                    if (pri->InfoCallBack != _NULL)
                    {
                        // If there are more strokes waiting, let's go get them
                        if ((*pri->InfoCallBack)(pri->ICB_param) == 1)
                            break;
                    }
                }
            }
        }
    }
    else // ------------------- If segmentation was off ------------------------
    {
        if (npoints == 0)
        {
            for (i = 0, len = 1; i < num_strokes; i++)
            {
                GetInkStrokeCopy(i, p_tr + len, &pri->ink_info);
                len += GetInkStrokeLen(i, &pri->ink_info);
            }

            pri->rc.trace = p_tr;
            pri->rc.ii = (_SHORT) len;
            pri->rc.trace->x = 0;
            pri->rc.trace->y = -1;

            er = HWR_RecWord(pri); // Call recognizer
        }
    }

    // ----------- Free memory and leave ---------------------------------

    if (p_tr)
        HWRMemoryFree(p_tr);
    return 0;

err:
    if (p_tr)
        HWRMemoryFree(p_tr);
    return 1;
}

/* ************************************************************************** */
/* *  Recognize one symbol                                                  * */
/* ************************************************************************** */

int RecoRecognizeSymbol(int npoints, p_RECO_point_type strokes_win, p_RECO_baseline_type baseline, RECOCTX context)
{
    p_PS_point_type strokes = (p_PS_point_type) strokes_win;
    _INT err = 0;
    _INT mwl;
    p_rec_inst_type pri = (p_rec_inst_type) (context);

    if (pri == _NULL)
        goto err;

    HWR_CleanUpContext(pri);

    pri->rc.trace = strokes;
    pri->rc.ii = (_SHORT) npoints;

    pri->baseline = *baseline;

    mwl = pri->rc.xrw_max_wlen;
    pri->rc.xrw_max_wlen = 1;
    // pri->ok = 1;

    // -------------- Recognize it! ------------------------------------------------

    err += HWR_RecWord(pri); // Call recognizer

    // --------------- Simple! Da? --------------------------------------------------

    pri->rc.xrw_max_wlen = (_SHORT) mwl;

    if (err || pri->recres == _NULL)
        goto err;

    return 0;
err:
    HWR_CleanUpContext(pri);
    return 1;
}

/* ************************************************************************** */
/* *  Returns current recognized words & info                               * */
/* ************************************************************************** */

RECO_ANSWER RecoGetAnswers(int what, int nw, int na, RECOCTX context)
{
    _INT i, j;
    RECO_ANSWER result;
    p_INT iptr;
    p_UCHAR ptr;
    p_rec_inst_type pri = (p_rec_inst_type) (context);

    result.value = 0;
    result.word = _NULL;
    if (pri == _NULL || nw >= pri->rr_num_answers)
        goto err;

    switch (what)
    {
        case HW_NUM_ANSWERS:
            result.value = pri->rr_num_answers;
            break;
            
        case HW_NUM_ALTS:
            if ((iptr = HWR_GetAnswerBlockPtr(nw, pri)) == _NULL)
                goto err;
            ptr = (p_UCHAR) (iptr + 1);
            for (i = j = 0; i < ((*iptr - 1) << 2); i++)
                if (ptr[i] == 0)
                    j++;
            result.value = j;
            break;
            
        case HW_ALT_WORD:
            if ((iptr = HWR_GetAnswerBlockPtr(nw, pri)) == _NULL)
                goto err;
            ptr = (p_UCHAR) (iptr + 1);
            for (i = j = 0; i < ((*iptr - 1) << 2); i++)
            {
                if (ptr[i] == 0 || i == 0)
                {
                    if (j == na)
                    {
                        _INT n;
                        if (i)
                            n = i + 1;
                        else
                            n = i; // Move from prev zero
#ifdef HW_RECINT_UNICODE
                        _INT k;
                        for (k = 0; ptr[n+k] != 0; k ++ )
                            pri->uans_buf[k] = (UCHR)ptr[n+k];
                        pri->uans_buf[k] = 0;
                        result.word = (UCHR *)(&pri->uans_buf[0]);
#else
                        result.word = (UCHR *)(&ptr[n]);
#endif
                        break;
                    }
                    else
                        j++;
                }
            }
            break;
            
        case HW_ALT_WEIGHT:
            if ((iptr = HWR_GetAnswerBlockPtr(nw, pri)) == _NULL)
                goto err;
            iptr += *iptr; // Advance to weights block
            result.value = (int)(*(iptr + 1 + na));
            break;
            
        case HW_ALT_NSTR:
            if ((iptr = HWR_GetAnswerBlockPtr(nw, pri)) == _NULL)
                goto err;
            iptr += *iptr; // Advance to weights block
            iptr += *iptr; // Advance to strokes block
            result.value = (int)((*iptr) - 1);
            break;
            
        case HW_ALT_STROKES:
            if ((iptr = HWR_GetAnswerBlockPtr(nw, pri)) == _NULL)
                goto err;
            iptr += *iptr; // Advance to weights block
            iptr += *iptr; // Advance to strokes block
            result.array = (iptr + 1);
            break;
            
        default:
            goto err;
    }
    
err:
    return result;
}

/* ************************************************************************** */
/* *  Interface to get and set vexes via letter pictures                    * */
/* ************************************************************************** */

int RecoGetSetPicturesWeights(int operation, void * buf, RECOCTX context)
{
#if DTI_LRN_SUPPORTFUNC
    _INT op;
    p_VOID dp = ((p_rec_inst_type) (context))->rc.dtiptr;
    
    switch (operation)
    {
        case LRN_SETDEFWEIGHTS_OP:
            op = DTILRN_SETDEFWEIGHTS;
            break;
        case LRN_GETCURWEIGHTS_OP:
            op = DTILRN_GETCURWEIGHTS;
            break;
        case LRN_SETCURWEIGHTS_OP:
            op = DTILRN_SETCURWEIGHTS;
            break;
        default:
            return -1;
    }
    
    return GetSetPicturesWeights((_INT) op, (p_VOID) buf, (p_VOID) dp);
#else
    return -1;
#endif
}

// ============================================================================

/* ************************************************************************** */
/* *  Get pointer to requested answer block                                 * */
/* ************************************************************************** */
static p_INT HWR_GetAnswerBlockPtr(_INT nw, p_rec_inst_type pri)
{
    _INT i;
    p_INT iptr;

    if (pri->recres == _NULL || nw > pri->rr_num_answers)
    {
        return _NULL;
    }
    else
    {
        for (i = 0, iptr = (p_INT) pri->recres; i < nw * 3; i++)
            iptr += *iptr;
        return iptr;
    }
}

/* ************************************************************************ */
/* *   Main recognition function for one word                             * */
/* ************************************************************************ */
static _INT HWR_RecWord(p_rec_inst_type pri)
{
    _INT size, pos, nl, er = 0;
    RCB_inpdata_type rcb;

    // DWORD time;
    // CHAR  str[64];
    HWRMemSet(&rcb, 0, sizeof(rcb));

    if (!pri->ok)
        goto err;

    // time = GetTickCount();

    // --------------------- Preprocess trace --------------------------------------

    PreprocessTrajectory(&pri->rc); // May ne moved lower, when trace will not be rewritten by replay of tentaive words

    // --------------------- Low-level block, optional -----------------------------

    if (pri->rc.p_xd_data == _NULL || !(pri->rc.f_xd_data & XRLV_DATA_USE))
    {
        // ------------- Set basic data for Stroka --------------------
        rcb.trace = pri->rc.trace;
        rcb.num_points = pri->rc.ii;

        if (pri->rr_num_answers == 0)
        {
            rcb.flags |= (_SHORT) (RCBF_NEWAREA);
            rcb.prv_size = rcb.prv_dn_pos = rcb.prv_size_sure
                    = rcb.prv_pos_sure = 0;
        }
        else
        {
            rcb.prv_size = pri->rc.stroka.size_out;
            rcb.prv_dn_pos = pri->rc.stroka.dn_pos_out;
            rcb.prv_size_sure = pri->rc.stroka.size_sure_out;
            rcb.prv_pos_sure = pri->rc.stroka.pos_sure_out;
        }

        rcb.flags |= (_SHORT) RCBF_PREVBORD;

        if (GetWSBorder(pri->wswi.nword, &pri->wsr, &size, &pos, &nl) == 0)
        {
            rcb.ws_size = (_SHORT) size;
            rcb.ws_dn_pos = (_SHORT) pos;
            rcb.flags |= (_SHORT) RCBF_WSBORD;
            if (nl)
                rcb.flags |= (_SHORT) (RCBF_NEWLINE);
        }

        if (pri->baseline.size)
        {
            rcb.bx_size = (_SHORT) (pri->baseline.size);
            rcb.bx_dn_pos = (_SHORT) (pri->baseline.base);
            rcb.flags |= (_SHORT) RCBF_BOXBORD;
        }

        SetRCB(&rcb, &(pri->rc.stroka));

        // --------------------- Preprocess trace --------------------------------------

        // PreprocessTrajectory(&pri->rc); Moved up, to escape rewriting trajectory without desloping

        // --------------------- Low level start ---------------------------------------

        er = (low_level(pri->rc.trace, &pri->xrdata, &pri->rc) != SUCCESS);

        // --------------------- Mword flags -------------------------------------------

        SetMultiWordMarksDash(&pri->xrdata);
        SetMultiWordMarksWS(pri->rc.lrn_min_class, &pri->xrdata, &pri->rc);
    }

    // --------------------- Protection scramble -------------------------------------

    // {	for (int i = 0; g_rec_protect_locked && i < pri->xrdata.len; i ++) (*pri->xrdata.xrd)[i].xr.type += 1;}

    // --------------------- Upper level start ---------------------------------------

    if (!er)
        er = xrlv(&pri->xrdata, &pri->rwg, &pri->rc);

    HWR_RegNewAnsw(pri, er);
    FreeRWGMem(&pri->rwg);
    return er;

err:
    HWR_RegNewAnsw(pri, 1);
    FreeRWGMem(&pri->rwg);
    return 1;
}

/* ************************************************************************ */
/* *   Init recognition instance                                          * */
/* ************************************************************************ */
static _INT HWR_RecInit(p_rec_inst_type _PTR ppri, int language )
{
    _INT lang = language;
    p_rec_inst_type pri;
    _INT i;
    _INT nLdb;
    Automaton am;
    p_Ldb pLDB;

    //  if (*ppri != _NULL) goto err;

    // Count LDB headers.
    for (nLdb = 0; GetLDBImgBody(lang, nLdb) != _NULL; nLdb++)
    {
    }

    if ((pri = (p_rec_inst_type) HWRMemoryAlloc(sizeof(rec_inst_type)
            + sizeof(Ldb) * nLdb)) == _NULL)
        goto err;

    HWRMemSet((p_VOID) (pri), 0, sizeof(rec_inst_type));
    
    // Cahin LDBs.
    if (nLdb > 0)
    {
        pLDB = (p_Ldb) (pri + 1);
        for (i = 0; (am = (Automaton) GetLDBImgBody(lang, i)) != _NULL; i++)
        {
            pLDB[i].am = am;
            if (i != 0)
                pLDB[i - 1].next = pLDB + i;
        }
        pLDB[i - 1].next = _NULL;
    }
    else
        pLDB = _NULL;

    if (AllocXrdata(&pri->xrdata, XRINP_SIZE))
        goto err;

    dti_load(0, DTI_DTE_REQUEST, (p_VOID _PTR) &(pri->p_dtih), language );

    HWRMemCpy((p_VOID) (&pri->p_trh), (p_VOID) (triads_get_header(lang)),
            sizeof(pri->p_trh));
    pri->p_trh.p_tr = (p_UCHAR)(triads_get_body(lang));

    pri->rc.dtiptr = (p_VOID) (pri->p_dtih);
    pri->rc.vocptr[0] = (p_VOID) (&pri->main_dict);
    pri->rc.vocptr[1] = (p_VOID) (&pri->pref_dict);
    pri->rc.vocptr[2] = (p_VOID) (&pri->suff_dict);
    pri->rc.vocptr[3] = (p_VOID) (&pri->user_dict);
    pri->rc.tr_ptr = (p_VOID) (&pri->p_trh);
    pri->rc.cflptr = (p_VOID) pLDB;
    pri->rc.lang = language;

    pri->wsr.pwsa = &(pri->w_str);
    pri->rc.p_ws_wi = (p_VOID) &pri->wswi;

    *ppri = pri;

    return 0;
    err: if (pri)
        HWRMemoryFree(pri);
    return 1;
}

/* ************************************************************************ */
/* *   Free recognition instance                                          * */
/* ************************************************************************ */
static _INT HWR_RecClose(p_rec_inst_type _PTR ppri)
{
    p_rec_inst_type pri = *ppri;

    if (pri == _NULL)
        return 1;

    pri->wsc.flags |= WS_FL_CLOSE;
    WordStrokes(_NULL, &pri->wsc, &pri->wsr);

    FreeXrdata(&pri->xrdata);
    dti_unload((p_VOID _PTR) &(pri->p_dtih));

    HWRMemoryFree(pri);
    *ppri = _NULL;
    return 0;
}

/* ************************************************************************ */
/* *   Free some meory and cleanup session                                * */
/* ************************************************************************ */

static _INT HWR_CleanUpContext(p_rec_inst_type pri)
{
    if (pri == _NULL)
        goto err;

    FreeInkInfo(&pri->ink_info);

    // pri->num_word      = 0;
    pri->g_stroke_num = 0;
    pri->baseline.size = 0;

    if (pri->recres)
        HWRMemoryFree(pri->recres);

    pri->recres = _NULL;
    pri->rr_alloc_size = 0;
    pri->rr_filled_size = 0;
    pri->rr_num_answers = 0;
    pri->rr_num_finished_answers = 0;
    pri->num_tentative_words = 0;

    if (pri->rc.p_xd_data)
        XrlvDealloc((p_xrlv_data_type _PTR) (&pri->rc.p_xd_data));

    // pri->rc.f_xd_data   = 0;
    // pri->unfinished_data.pxd = 0;

    return 0;
err:
    return 1;
}

/* ************************************************************************** */
/* *  Add word list to RecResult                                            * */
/* ************************************************************************** */
static _INT HWR_AddToAnsw(p_rec_inst_type pri, p_UCHAR answ, p_INT weights, _INT ns,
        p_INT stroke_ids)
{
    _INT i;
    _INT len_a, len_w, len_s, len, na;
    p_VOID ptr;
    p_INT iptr;
    p_UCHAR cptr;

    if (answ == _NULL || pri == _NULL || !pri->ok)
        goto err;

    // ------------ Estimate memory ---------------------------------

    len_a = HWRStrLen((_STR) answ) + 1;
    for (i = na = 0; i < len_a; i++)
        if (answ[i] <= PM_ALTSEP)
            na++;
    len_w = na;
    len_s = ns;

    len = len_a + (len_w + len_s + 3 + 1 + 1) * sizeof(_INT);

    // ------------- Alloc/realloc mmeory ----------------------------

    if (pri->rr_alloc_size < pri->rr_filled_size + len)
    {
        if (pri->recres == _NULL)
        {
            if ((pri->recres = (p_UCHAR) HWRMemoryAlloc(len + PR_ANSW_EXTALLOC))
                    == _NULL)
                goto err;
            pri->rr_alloc_size = len + PR_ANSW_EXTALLOC;
        }
        else
        {
            if ((ptr = HWRMemoryAlloc(pri->rr_filled_size + len
                    + PR_ANSW_EXTALLOC)) == _NULL)
                goto err;
            HWRMemCpy(ptr, pri->recres, pri->rr_alloc_size);
            HWRMemoryFree(pri->recres);
            pri->rr_alloc_size = pri->rr_filled_size + len + PR_ANSW_EXTALLOC;
            pri->recres = (p_UCHAR) ptr;
        }
    }

    // --------------- Put answer strings in answer buffer -----------------------

    iptr = (p_INT) (&pri->recres[pri->rr_filled_size]);
    *iptr = 1 + ((len_a + 3) >> 2);
    pri->rr_filled_size += (*iptr) * sizeof(_INT);
    HWRMemSet(iptr + (*iptr) - 1, 1, sizeof(_INT)); // String end padding
    HWRStrCpy((_STR) (iptr + 1), (_STR) answ);

    for (i = 0, cptr = (p_UCHAR) (iptr + 1); i < len_a; i++)
    {
        if (cptr[i] <= PM_ALTSEP)
            cptr[i] = 0;
    }
    // --------------- Put weights ------ in answer buffer -----------------------
    {
        _USHORT xrsum;
        p_xrd_el_type xrd = &((*pri->xrdata.xrd)[0]);
        // ------------- For tester we place xrsum after the weights ---------------
        for (i = xrsum = 0; i < pri->xrdata.len; i++, xrd++)
            xrsum += (_USHORT) (xrd->xr.type + xrd->xr.attrib + xrd->xr.penalty
                    + xrd->xr.height + xrd->xr.shift + xrd->xr.orient
                    + xrd->xr.depth);

        iptr = (p_INT) (&pri->recres[pri->rr_filled_size]);
        *iptr = 1 + 1 + len_w;
        pri->rr_filled_size += (*iptr) * sizeof(_INT);
        if (weights)
            HWRMemCpy(iptr + 1, weights, len_w * sizeof(_INT));
        *(iptr + 1 + len_w) = xrsum;
    }

    // --------------- Put strokes ids -- in answer buffer -----------------------

    iptr = (p_INT) (&pri->recres[pri->rr_filled_size]);
    *iptr = 1 + len_s;
    pri->rr_filled_size += (*iptr) * sizeof(_INT);
    if (ns && stroke_ids)
        HWRMemCpy(iptr + 1, stroke_ids, len_s * sizeof(_INT));

    if (len_s > 1) // Sort strokes -- for the sake of NetClient test accuracy
    {
        _INT all_sorted = 0, ti;
        while (!all_sorted)
        {
            for (i = 1, all_sorted = 1; i < len_s; i++)
            {
                if (iptr[i] > iptr[i + 1])
                {
                    ti = iptr[i];
                    iptr[i] = iptr[i + 1];
                    iptr[i + 1] = ti;
                    all_sorted = 0;
                }
            }
        }
    }

    pri->rr_num_answers++;
    if (!(pri->wswi.flags & WS_FL_TENTATIVE))
        pri->rr_num_finished_answers++;

    return 0;

    err: return 1;
}

/* ************************************************************************** */
/* *  Resets counter of tentative storage to finished state                 * */
/* ************************************************************************** */

static _INT HWR_ResetTentativeStorage(_INT st_index, p_rec_inst_type pri)
{
    _INT i;
    p_INT iptr;
    _INT num_answ, n_tt;

    for (i = n_tt = 0; i < st_index; i++)
        n_tt += pri->tentative_list[i].nparts;

    num_answ = pri->rr_num_finished_answers + n_tt;

    if (pri->rr_num_answers > num_answ)
    { // Reset free space pointer to the place of the tentatve words
        if ((iptr = HWR_GetAnswerBlockPtr(num_answ, pri)) == _NULL)
            goto err;
        pri->rr_filled_size = (_INT) ((p_CHAR) iptr - (p_CHAR) &pri->recres[0]);
        pri->rr_num_answers = num_answ;
    }

    pri->num_tentative_words = st_index;

    return 0;
    err: return 1;
}

/* ************************************************************************** */
/* *  Resets counter of tentative storage to finished state                 * */
/* ************************************************************************** */

static _INT HWR_ValidateNextTentativeWord(_INT nparts, p_rec_inst_type pri)
{
    pri->rr_num_finished_answers += nparts;

    if (pri->rr_num_finished_answers > pri->rr_num_answers)
    {
        // Something's wrong here!
        pri->rr_num_answers = pri->rr_num_finished_answers;
        goto err;
    }
    return 0;

    err: return 1;
}

/* ************************************************************************** */
/* *  Checks if word could contain incorrectly recognized 'funny' chars     * */
/* ************************************************************************** */

static _UINT DN_CheckForReplacements( p_rec_inst_type pri, _UCHAR * pWordIn )
{
    _UINT uReplacements = 0;
    _UINT x = 1;
    _UCHAR *p = pWordIn;
    int bFixO = 0;

    while( *p )
    {
        if ( *p != '0' )
        {
            if ( IsDigit( *p ) )
            return 0;
        }
        if ( *p == 's' || *p == 'a' || *p == 'S' || *p == 'A' || *p == 'o' || *p == 'O' || *p == '0' )
        {
            if ( *(p+1) == 'c' || *(p+1) == 'e' || *(p+1) == 'C' || *(p+1) == 'E' )
            {
                uReplacements |= x;
                x <<= 1;
                p++;
            }
            else if ( (*p == 'o' || *p == 'O' || *p == '0') &&
                    (*(p+1) == '/' || *(p+1) == '1' || *(p+1) == '(' || *(p+1) == ')' || *(p+1) == '|') )
            {
                bFixO = 1;
                p++;
            }
            else if (*(p+1) == 'i' || *(p+1) == 'I' || *(p+1) == 'l' || *(p+1) == 'L' )
            {
                uReplacements |= x;
                x <<= 1;
                p++;
            }
        }
        p++;
    }
    if ( bFixO && HWRStrLen( (_STR)pWordIn ) > 2 )
    {
        p = pWordIn;
        while( *p )
        {
            if ( *p == 'o' || *p == 'O' || *p == '0' )
            {
                if ( *(p+1) == '/' || *(p+1) == '1' || *(p+1) == '(' || *(p+1) == ')' || *(p+1) == '|' )
                {
                    _UCHAR * p1 = p;
                    if ( (pri->flags & HW_RECFL_CAPSONLY) || *p == 'O' )
                    *p1 = OS_O_crossed;
                    else
                    *p1 = OS_o_crossed;
                    p1++;
                    while( *(p1+1) )
                    {
                        *p1 = *(p1+1);
                        p1++;
                    }
                    *p1 = 0;
                    p++;
                }
            }
            p++;
        }
    }
    return uReplacements;
}

/* ************************************************************************** */
/* *   Check if the word is in the dictionary                               * */
/* ************************************************************************** */

static int _CheckWordInDict( _UCHAR * inp_word, void *h_dict )
{
    int len, res = 1;
    _UCHAR status = 0, attr = 0;

    if (h_dict == NULL)
    return 1;
    if ( (len = HWRStrLen( (_STR)inp_word) ) >= HW_MAXWORDLEN )
    return 1;

    res = PZDictCheckWord( (p_UCHAR)inp_word, &status, &attr, h_dict );
    if ( status == XRWD_MIDWORD )
    res = 1;

    if ( res )
    {
        // if the word has mixed capitalization, convert to lower case and try again
        int i;
        for ( i = 0; i < len; i++ )
        {
            if ( IsUpper( inp_word[i] ) )
            {
                _UCHAR word[HW_MAXWORDLEN] = "";
                HWRStrCpy( (_STR)word, (_STR)inp_word );
                for (; i < len; i++ )
                {
                    word[i] = (_UCHAR)ToLower( word[i] );
                }
                res = PZDictCheckWord( (p_UCHAR)word, &status, &attr, h_dict );
                if ( status == XRWD_MIDWORD )
                res = 1;
                return res;
            }
        }
    }
    return res;
}

/* ************************************************************************** */
/* *  Checks if the word in the dictionary: 0 - not, 1 - main, 2 - user     * */
/* ************************************************************************** */

static _INT DN_IsDictionaryWord( p_rec_inst_type pri, _UCHAR * pWord )
{
    _INT res = 0;
    if ( pri->user_dict.hvoc_dir != NULL )
    {
        if ( 0 == _CheckWordInDict( pWord, pri->user_dict.hvoc_dir ) )
        res = 2;
    }
    if ( (0 == res) && pri->main_dict.hvoc_dir != NULL)
    {
        if ( 0 == _CheckWordInDict( pWord, pri->main_dict.hvoc_dir ) )
        res = 1;
    }
    return res;
}

/* ************************************************************************** */
/* *  Converts suspected character sequence to 'funny' char and             * */
/* *  verifies if the word in dict. Returns >0 if word should be replaced   * */
/* ************************************************************************** */

static _INT DN_ConvertWord( p_rec_inst_type pri, _UCHAR * pWordIn, _UINT uReplacements, _UCHAR * pWordOut )
{
    _INT resIn = DN_IsDictionaryWord( pri, pWordIn ); // check if the recognized word in the dictionary
    _INT resOut = 0;
    _UINT nRepl = uReplacements;

    while ( nRepl > 0 )
    {
        _INT k = 0;
        _UCHAR * pWord = pWordIn;
        _UINT r = 1;
        while( *pWord )
        {
            if ( *pWord == 's' || *pWord == 'a' || *pWord == 'S' ||
                    *pWord == 'A' || *pWord == 'o' || *pWord == 'O' || *pWord == '0' )
            {
                if ( *(pWord+1) == 'c' || *(pWord+1) == 'e' || *(pWord+1) == 'C' || *(pWord+1) == 'E' )
                {
                    if ( 0 != (r & nRepl) )
                    {
                        if ( (pri->flags & HW_RECFL_CAPSONLY) || (IsUpper( *pWord ) && IsUpper( *(pWord+1) )) )
                        pWordOut[k++] = OS_AE_letter;
                        else
                        pWordOut[k++] = OS_ae_letter;
                        pWord += 2;
                    }
                    else
                    {
                        pWordOut[k++] = *pWord++;
                        pWordOut[k++] = *pWord++;
                    }
                    r <<= 1;
                }
                else if ( (*pWord == 'o' || *pWord == 'O' || *pWord == '0') &&
                        (*(pWord+1) == 'i' || *(pWord+1) == 'I' || *(pWord+1) == 'l' ||
                                *(pWord+1) == 'L' || *(pWord+1) == '|' || *(pWord+1) == '/') )
                {
                    if ( 0 != (r & nRepl) )
                    {
                        // try to replace o/ with crosses o and check against dictionary
                        if ( (pri->flags & HW_RECFL_CAPSONLY) || IsUpper( *pWord ) )
                        pWordOut[k++] = OS_O_crossed;
                        else
                        pWordOut[k++] = OS_o_crossed;
                        pWord += 2;
                    }
                    else
                    {
                        pWordOut[k++] = *pWord++;
                        pWordOut[k++] = *pWord++;
                    }
                    r <<= 1;
                }
            }
            if ( *pWord )
            pWordOut[k++] = *pWord++;
        }
        pWordOut[k] = 0;
        resOut = DN_IsDictionaryWord( pri, pWordOut ); // check if the recognized word in the dictionary
        if ( resOut > 0 )
        {
            if ( 0 == resIn )
            return 1;
            else if ( resOut > resIn )
            return 2;
            else
            return 3;
        }
        nRepl--;
    }
    return 0;
}

/* ************************************************************************** */
/* *  Add word list to RecResult                                            * */
/* ************************************************************************** */

static _INT HWR_RegNewAnsw(p_rec_inst_type pri, _INT er)
{
    _INT lang = pri->rc.lang;
    _INT i, j, k, n, m, f;
    _INT ns, np, len;
    _TRACE p_tr;
    _INT stroke_ids[WS_MAX_STROKES] = {0};
    _INT str_sts[WS_MAX_STROKES] = {0};
    _UCHAR answers[w_lim * (NUM_RW) + NUM_RW] = {0};
    _UCHAR vars[w_lim * (NUM_RW) + NUM_RW] = {0};
    _UCHAR word[w_lim + 1] = {0};
    _INT weights[NUM_RW] = { 0 };
    p_RWS_type prws;
    _UCHAR parts[w_lim] = {0};
    p_xrdata_type xrdata = &(pri->xrdata);

    // ------------ Write down stroke lineout ----------------------
    for (i = 1, ns = 0, p_tr = pri->rc.trace; i < pri->rc.ii; i++)
    {
        if (p_tr[i].y < 0)
        {
            str_sts[ns] = i;
            stroke_ids[ns++] = p_tr[i].x;
        }
    }

    if (p_tr[0].x > 0)
    {
        str_sts[ns] = 0;
        stroke_ids[ns++] = p_tr[0].x;
    } // Save trashed carry dash -- if this id is more than 0, it means that there is salvaged dash ID.

    // --------------------- Store answers -----------------------------------------

    prws = (p_RWS_type) (pri->rwg.rws_mem);

    if (!er && prws != _NULL) // Normal asnwer registration
    {

        for (i = 1, np = 1, parts[0] = 0; i < pri->rwg.size; i++)
        {
            if (prws[i].type != RWST_SYM)
                break;

            if (prws[i].sym == ' ')
                parts[np++] = prws[i].xrd_beg;
        }

        parts[np] = (_UCHAR) xrdata->len;
        pri->rr_nparts = np;

        for (n = 0; n < np; n++)
        {
            for (i = j = k = 0; i < pri->rwg.size; i++)
            {
                if (prws[i].type == RWST_SYM)
                {
                    if ((pri->flags & HW_RECFL_CAPSONLY) && IsLower(prws[i].sym))
                    {
                        // this did not work too well...
                        //if ( pri->flags & HW_RECFL_PURE && prws[i].sym == 'l' )
                        //	prws[i].sym = L'I';
                        //else
                        prws[i].sym = (_UCHAR) ToUpper(prws[i].sym);
                    }

                    if (prws[i].xrd_beg >= parts[n] && prws[i].xrd_beg
                            < parts[n + 1] && prws[i].sym != ' ')
                    {
                        answers[j] = prws[i].sym;
                        vars[j++] = prws[i].nvar;
                        weights[k] = prws[i].weight;
                    }
                }
                else if (prws[i].type == RWST_NEXT)
                {
                    k++;
                    answers[j++] = PM_ALTSEP;
                }
            }
            
            answers[j] = 0;
            len = j + 1;

            PostConfirmCapitalLetters(answers, weights, vars, lang );

            // this code is here to correct OS_ae_letter and OS_o_crossed letters
            if ((lang == LANGUAGE_NORWEGIAN || lang == LANGUAGE_DANISH) 
                && 0 == (pri->flags & HW_RECFL_NUMONLY) )
            {
                _UCHAR alt[w_lim*(NUM_RW)+NUM_RW];
                _UCHAR word1[w_lim+1];
                _INT l, res = 0;
                _UINT repl = 0;
                _UINT cnt_rw = 1;
                _INT nAlt = 0;
                for ( j = k = i = 0; i <= len; i++ )
                {
                    if ( PM_ALTSEP >= answers[i] )
                    {
                        word[k] = 0;
                        if ( j > 0 )
                        alt[j++] = PM_ALTSEP;
                        nAlt++;
                        if ( k >= 2 && nAlt < 4 && (repl = DN_CheckForReplacements( pri, word )) &&
                                (res = DN_ConvertWord( pri, word, repl, word1 )) > 0 )
                        {
                            if ( res == 1 )
                            {
                                for ( l = 0; word1[l] != 0; l++ )
                                alt[j++] = word1[l];
                                if ( (++cnt_rw) >= NUM_RW )
                                break;
                            }
                            else if ( res == 2 )
                            {
                                for ( l = 0; word1[l] != 0; l++ )
                                alt[j++] = word1[l];
                                if ( (++cnt_rw) >= NUM_RW )
                                break;
                                alt[j++] = PM_ALTSEP;
                                for ( l = 0; word[l] != 0; l++ )
                                alt[j++] = word[l];
                                if ( (++cnt_rw) >= NUM_RW )
                                break;
                            }
                            else
                            {
                                for ( l = 0; word[l] != 0; l++ )
                                alt[j++] = word[l];
                                if ( (++cnt_rw) >= NUM_RW )
                                break;
                                alt[j++] = PM_ALTSEP;
                                for ( l = 0; word1[l] != 0; l++ )
                                alt[j++] = word1[l];
                                if ( (++cnt_rw) >= NUM_RW )
                                break;
                            }
                        }
                        else
                        {
                            for ( l = 0; word[l] != 0; l++ )
                            alt[j++] = word[l];
                            if ( (++cnt_rw) >= NUM_RW )
                            break;
                        }
                        alt[j] = 0;
                        k = 0;
                    }
                    else
                    {
                        word[k++] = answers[i];
                    }
                }
                alt[j] = 0;
                HWRMemCpy( answers, alt, j+1 );
                answers[j] = 0;
                len = j + 1;
            }
            
            for (i = j = 0; i <= len; i++) // Remove duplicates
            {
                if (answers[i] <= PM_ALTSEP)
                {
                    word[j] = 0;
                    for (k = m = f = 0; k < i - j; k++)
                    {
                        if (answers[k] == PM_ALTSEP)
                        {
                            if (!f)
                            {
                                HWRMemCpy(&answers[i - (j + 1)], &answers[i], len - i + 1);
                                i -= j + 1;
                                len -= j + 1;
                                break;
                            }
                            m = f = 0;
                        }
                        else if (word[m++] != answers[k])
                            f++;
                    }
                    j = 0;
                }
                else
                    word[j++] = answers[i];
            }
            
            
            // Ideotic fix for 1/I
            if ( answers[1] == PM_ALTSEP && (answers[0] == '1' || answers[0] == 'I') && 0 == (pri->flags & HW_RECFL_PURE) )
            {
                int isi = (NULL != HWRStrChr( (_CSTR)&answers[2], 'i' ));
                int is7 = (NULL != HWRStrChr( (_CSTR)&answers[2], '7' ));
                int is1 = (NULL != HWRStrChr( (_CSTR)&answers[2], '1' ));
                int isSL = (NULL != HWRStrChr( (_CSTR)&answers[2], '/' ));
                int isBS = (NULL != HWRStrChr( (_CSTR)&answers[2], '\\' ));
                
                if ( answers[0] == '1' && (0 != (pri->rc.enabled_cs & CS_ALPHA) || pri->rc.enabled_cs == 0) )
                {
                    k = 2;
                    if ( is7 )
                    {
                        answers[k++] = '7';
                        answers[k++] = PM_ALTSEP;
                    }
                    answers[k++] = 'I';
                    answers[k++] = PM_ALTSEP;
                    if ( 0 == (pri->flags & HW_RECFL_CAPSONLY) )
                    {
                        if ( isi )
                        {
                            answers[k++] = 'i';
                            answers[k++] = PM_ALTSEP;
                        }
                        answers[k++] = 'l';
                        answers[k++] = PM_ALTSEP;
                    }
                    if ( 0 != (pri->rc.enabled_cs & CS_MATH ) )
                    {						
                        answers[k++] = '|';
                        answers[k++] = PM_ALTSEP;
                        if ( isSL )
                        {
                            answers[k++] = '/';
                            answers[k++] = PM_ALTSEP;
                        }
                        if ( isBS )
                        {
                            answers[k++] = '\\';
                            answers[k++] = PM_ALTSEP;
                        }
                    }
                    answers[k++] = 0;
                }
                else if ( answers[0] == 'I' && (0 != (pri->rc.enabled_cs & CS_NUMBER) || pri->rc.enabled_cs == 0) )
                {
                    k = 2;
                    if ( is1 )
                    {
                        answers[k++] = '1';
                        answers[k++] = PM_ALTSEP;
                    }
                    if ( 0 == (pri->flags & HW_RECFL_CAPSONLY) )
                    {
                        if ( isi )
                        {
                            answers[k++] = 'i';
                            answers[k++] = PM_ALTSEP;
                        }
                        answers[k++] = 'l';
                        answers[k++] = PM_ALTSEP;
                    }
                    if ( 0 != (pri->rc.enabled_cs & CS_MATH ) )
                    {						
                        answers[k++] = '|';
                        answers[k++] = PM_ALTSEP;
                        if ( isSL )
                        {
                            answers[k++] = '/';
                            answers[k++] = PM_ALTSEP;
                        }
                        if ( isBS )
                        {
                            answers[k++] = '\\';
                            answers[k++] = PM_ALTSEP;
                        }
                    }
                    answers[k++] = 0;
                }
            }
            
            if (np > 1) // Separate stroke belongings
            {
                _INT s = 0;
                _INT sis[WS_MAX_STROKES] =
                { 0 };

                // Go through xr elems and register strokes
                for (i = parts[n]; i < parts[n + 1]; i++)
                {
                    j = (*xrdata->xrd)[i].begpoint;
                    for (k = 0; k < ns; k++) // Find which stroke the xr belongs to
                    {
                        if (j < str_sts[k])
                        {
                            if (stroke_ids[k] < 0)
                                break; // Already used

                            sis[s++] = stroke_ids[k];
                            stroke_ids[k] = -1;
                            break;
                        }
                    }
                }

                if (n == np - 1) // Check if there are loose strokes left -- attach them to last word
                {
                    for (k = 0; k < ns; k++)
                    {
                        if (stroke_ids[k] >= 0)
                            sis[s++] = stroke_ids[k];
                    }
                }

                HWR_AddToAnsw(pri, (p_UCHAR) answers, weights, s, &(sis[0]));
            }
            else
            {
                HWR_AddToAnsw(pri, (p_UCHAR) answers, weights, ns, &(stroke_ids[0]));
            }
        }
    }
    else // Error return -- label it as such
    {
        pri->rr_nparts = 1;

        if (pri->flags & HW_RECFL_PURE)
            HWRStrCpy((p_CHAR) answers, (_STR) "");
        else
            HWRStrCpy((p_CHAR) answers, (_STR) gError);

        HWR_AddToAnsw(pri, (p_UCHAR) answers, weights, ns, &(stroke_ids[0]));
    }

    return 0;
}


