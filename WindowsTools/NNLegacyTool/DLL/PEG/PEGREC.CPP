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
#include "zctype.h"

#include "ams_mg.h"
#include "lowlevel.h"
#include "xrword.h"
#include "ldbtypes.h"
#include "xrlv.h"
#include "ws.h"
#include "elk.h"

#include "precutil.h"
#include "peg_util.h"
#include "PegRec.h"
#include "PegRec_p.h"
#include "ligstate.h"
#include "dti_lrn.h"
// -------------------- Defines and globals --------------------------

#define CGR_VER_ID     "WritePad v.1.0"

#ifdef FOR_FRENCH
#define CGR_LANG_STRING "f"
#elif  defined FOR_GERMAN
#define CGR_LANG_STRING "g"
#elif  defined FOR_INTERNATIONAL
#define CGR_LANG_STRING "i"
#else
#define CGR_LANG_STRING "e"
#endif

#ifdef DTI_COMPRESSED_ON
#define CGR_DTI_STRING "c"
#else
#define CGR_DTI_STRING "u"
#endif

#ifdef SNN_FAT_NET_ON
#define CGR_MLP_STRING "f"
#else
#define CGR_MLP_STRING "s"
#endif

#define CGR_ID_STRING CGR_VER_ID CGR_LANG_STRING CGR_DTI_STRING CGR_MLP_STRING

#define PR_ANSW_EXTALLOC       256
#define PR_TOOMANYWORDSWAITING   5
#define PR_TENTATIVELIST_SIZE   10

#define WST_DEF_H_LINE          80
#define WST_DEF_S_DELAY          3
#define WST_DEF_UNSURE_LEVEL    50

#define PM_ALTSEP                1 /* Recognized word list alternatives separator */
#define PM_LISTSEP               2 /* Recognized word list wordlist separator */
#define PM_LISTEND               0 /* Recognized word list end */

// ------------------------- Structures -------------------------------------

typedef struct
{
	_INT             nword;
	_INT             nparts;
	_INT             len;
} tentative_list_type, *p_tentative_list_type;

typedef struct
{
	_INT             ok;
	_INT             flags;
	_INT             sp_vs_q;

	rc_type          rc;
	xrdata_type      xrdata;
	RWG_type         rwg;

	_INT             g_stroke_num;

	word_strokes_type w_str[WS_MAX_WORDS];
	ws_results_type  wsr;
	ws_word_info_type wswi;
	ink_info_type     ink_info;
	ws_control_type   wsc;
	CGR_baseline_type baseline;
	tentative_list_type tentative_list[PR_TENTATIVELIST_SIZE];
	_INT             num_tentative_words;

	tentative_list_type unfinished_data;   // Storage of infinished recognition attr

	_INT             rr_alloc_size;       // Allocated Results buffer
	_INT             rr_filled_size;      // Butes used there
	_INT             rr_num_answers;      // Num of all registered answers
	_INT             rr_num_finished_answers;  // Num of non-tentative answers
	_INT             rr_nparts;           // Number of parts in last recognized and registered answer
	p_UCHAR          recres;

	vocptr_type      main_dict;
	vocptr_type      pref_dict;
	vocptr_type      suff_dict;
	vocptr_type      user_dict;

	info_func_type   InfoCallBack;
	p_VOID           ICB_param;

	p_dti_descr_type p_dtih;
	tr_descr_type    p_trh;

#ifdef PEG_RECINT_UNICODE
	UCHR             uans_buf[w_lim];
#endif
} rec_inst_type, _PTR p_rec_inst_type;


// ------------------------- Prototypes -------------------------------------

_INT  PegRecWord(p_rec_inst_type pri);
_INT  PegRecSymbol(p_rec_inst_type pri, p_UCHAR answ);
_INT  PegRecInit(p_rec_inst_type _PTR pri);
_INT  PegRecClose(p_rec_inst_type _PTR pri);
_INT  PegAddToAnsw(p_rec_inst_type pri, p_UCHAR answ, p_INT weights, _INT ns, p_INT stroke_ids);
_INT  PegRegNewAnsw(p_rec_inst_type pri, _INT er);
_INT  PegResetTentativeStorage(_INT st_index, p_rec_inst_type pri);
_INT  PegValidateNextTentativeWord(_INT nparts, p_rec_inst_type pri);
_INT  PegCleanUpContext(p_rec_inst_type pri);
_INT  PegCheckWordInDict(p_CHAR word, CGRHDICT h_dict);
p_INT PegGetAnswerBlockPtr(_INT nw, p_rec_inst_type pri);

#ifdef DEBUG
#define PEGREC_DEBUG       /* Allows to print out log of recognition proceedings */
#endif

#ifdef PEGREC_DEBUG
// #define TRACE_DMP_DBG
void PegDebugPrintf(char * format, ...);
int  DebTraceRec(CGRCTX context);
#endif

PRP_01

ROM_DATA_EXTERNAL _ULONG img_voc [];
ROM_DATA_EXTERNAL _ULONG img_vocpref [];
ROM_DATA_EXTERNAL _ULONG img_vocsuff [];
ROM_DATA_EXTERNAL _UCHAR sp_vs_q_ts [];
ROM_DATA_EXTERNAL _UCHAR sp_vs_q_bd [];

ROM_DATA_EXTERNAL _UCHAR alpha_charset [];
#if defined (FOR_SWED)
ROM_DATA_EXTERNAL _UCHAR alpha_charset_swe[];
ROM_DATA_EXTERNAL _UCHAR alpha_charset_eng[];
#elif defined (FOR_INTERNATIONAL)
ROM_DATA_EXTERNAL _UCHAR alpha_charset_eng[];
#endif /*FOR_SWED*/
ROM_DATA_EXTERNAL _UCHAR lpunct_charset [];
ROM_DATA_EXTERNAL _UCHAR epunct_charset [];
ROM_DATA_EXTERNAL _UCHAR other_charset [];
ROM_DATA_EXTERNAL _UCHAR num_charset [];
ROM_DATA_EXTERNAL _UCHAR math_charset [];

ROM_DATA_EXTERNAL tr_descr_type  img_trd_header;
ROM_DATA_EXTERNAL _ULONG         img_trd_body [];

#if VER_RECPROTECTED
#include <windows.h> // For GetTickCount
int g_rec_protect_locked = 2;
#endif

#ifdef __cplusplus
extern "C"
#endif
const _ULONG _PTR GetLDBImgBody(_INT index);

/* *************************************************************** */
/* *  Get info string and capabilities of the recognizer         * */
/* *************************************************************** */

int DLLEXP CgrGetRecID(p_CGR_ID_type p_inf)
{
	p_UCHAR idstr = (p_UCHAR) CGR_ID_STRING;

	if (p_inf == 0)
	{
		goto err;
	}

#if VER_RECPROTECTED
	if (g_rec_protect_locked == 2)
	{
		if ((GetTickCount() & 0x0FFFFFFF) - (p_inf->capabilities&0x0FFFFFFF) < 1000)
		{
			g_rec_protect_locked = 0;
		}
		else
		{
			g_rec_protect_locked = 1;    // remain locked
		}
	}
#endif //   #if VER_RECPROTECTED

	p_inf->capabilities = PEG_CPFL_CURS | PEG_CPFL_SPVSQ;

#if defined FOR_INTERNATIONAL
	p_inf->capabilities |= PEG_CPFL_INTER;
#endif

#ifdef PEG_RECINT_UNICODE
	{
		_INT i;
		for (i = 0; idstr[i] != 0; i ++)
		{
			p_inf->id_string[i] = (UCHR)idstr[i];
		}
		p_inf->id_string[i] = 0;
	}
#else
	HWRStrCpy(p_inf->id_string, (_STR) idstr);
#endif

	return PEG_RECINT_ID_001;
err:
	return 0;
}

/* *************************************************************** */
/* *  Create context for recognition                             * */
/* *************************************************************** */

CGRCTX DLLEXP CgrCreateContext(void)
{
	p_rec_inst_type pri;


	// ---------- Let's allocate and init what's needed -----------------------

	if (PegRecInit(&pri))
	{
		goto err;
	}
	if (pri == _NULL)
	{
		goto err;
	}

	PegCleanUpContext(pri);

#ifdef PEGREC_DEBUG
	PegDebugPrintf("CreateContext: Ok. pri = %x", pri);
#endif

	return (CGRCTX) pri;
err:
	return _NULL;
}

/* *************************************************************** */
/* *  Close recognition context and free all the memory          * */
/* *************************************************************** */

int DLLEXP CgrCloseContext(CGRCTX context)
{
	p_rec_inst_type pri = (p_rec_inst_type) (context);

	if (pri == _NULL)
	{
		goto err;
	}

	CgrCloseSession(context);
	PegCleanUpContext(pri);
	PegRecClose(&pri);

#ifdef PEGREC_DEBUG
	PegDebugPrintf("CloseContext:\n PRI: %x", pri);
#endif
	return 0;
err:
	return 1;
}
/* *************************************************************** */
/* *  Open recognition session and init context to default state * */
/* *************************************************************** */

int DLLEXP CgrOpenSession(p_CGR_control_type ctrl, CGRCTX context)
{
	p_rec_inst_type pri = (p_rec_inst_type) (context);
	p_rc_type       prc = &pri->rc;

	// ----------------- Absorb cotrol values --------------------------

	pri->flags = ctrl->flags;
	pri->sp_vs_q = ctrl->sp_vs_q;
	pri->InfoCallBack = ctrl->InfoCallBack;
	pri->ICB_param = ctrl->ICB_param;

	// ---------------- RC defaults -------------------------------------

	prc->enabled_cs = CS_ALPHA | CS_NUMBER | CS_LPUNCT | CS_EPUNCT | CS_OTHER | CS_MATH;
	prc->enabled_ww = WW_PALMER | WW_BLOCK | WW_GENERAL;
#if  defined(FOR_ENGLISH)
	prc->enabled_languages = EL_ENGLISH;
#elif  defined(FOR_FRENCH)
	prc->enabled_languages = EL_FRENCH;
#elif  defined(FOR_GERMAN)
	prc->enabled_languages = EL_GERMAN;
#else
	prc->enabled_languages = EL_ENGLISH|EL_GERMAN|EL_FRENCH|EL_SWEDISH;
#endif /* !ENGLISH */

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

	prc->ws_handle = 0; // <- auto WS segmentation with learning, was 6;

	prc->use_vars_inf = 1;

	// ------------------ Modifiers & pointers ----------------------------

#ifdef INTERNAL_DICT
	pri->main_dict.hvoc_dir   = (p_VOID)((img_voc[0] == 0) ? 0:(&img_voc[0]));
	pri->user_dict.hvoc_dir   = ctrl->h_user_dict;
	pri->pref_dict.hvoc_dir   = (p_VOID)((img_vocpref[0] == 0) ? 0:(&img_vocpref[0]));
	pri->suff_dict.hvoc_dir   = (p_VOID)((img_vocsuff[0] == 0) ? 0:(&img_vocsuff[0]));
#else
	pri->main_dict.hvoc_dir = ctrl->h_main_dict;
	pri->user_dict.hvoc_dir = ctrl->h_user_dict;
	pri->pref_dict.hvoc_dir = (p_VOID) ((img_vocpref[0] == 0) ? 0 : (&img_vocpref[0]));
	pri->suff_dict.hvoc_dir = (p_VOID) ((img_vocsuff[0] == 0) ? 0 : (&img_vocsuff[0]));
#endif

	if (pri->sp_vs_q < 0)
	{
		pri->sp_vs_q = 0;
	}
	if (pri->sp_vs_q > 9)
	{
		pri->sp_vs_q = 9;
	}
	prc->xrw_tag_size = sp_vs_q_ts[pri->sp_vs_q];
	prc->bad_distance = sp_vs_q_bd[pri->sp_vs_q];

	if (pri->flags & PEG_RECFL_DICTONLY)
	{
		prc->xrw_mode &= ~(XRWM_LD | XRWM_CS | XRWM_TRIAD);
	}
	if (pri->flags & PEG_RECFL_SEPLET)
	{
		prc->corr_mode = XRCM_SEPLET;
	}
	if (pri->flags & PEG_RECFL_NUMONLY)
	{
		prc->xrw_mode = XRWM_LD | XRWM_CS;
		prc->enabled_cs = CS_NUMBER | CS_OTHER | CS_MATH;
	}

	if (pri->main_dict.hvoc_dir != 0 && (prc->xrw_mode & XRWM_MWORD)) // Dictionary segmentation makes sense only in presence of dictionary!
	{
		prc->lrn_class_level = 92; //WS_SURE_LEVEL;
		prc->lrn_min_class = 92; //WS_SURE_LEVEL;
	}
	else
	{
		prc->lrn_class_level = 0; //WS_SURE_LEVEL;
		prc->lrn_min_class = 0; //WS_SURE_LEVEL;
	}

#if defined (FOR_SWED)
	if (pri->flags & PEG_RECFL_INTL_CS)
	{
		prc->alpha_charset = (p_UCHAR)alpha_charset_swe;
	}
	else
	{
		prc->alpha_charset = (p_UCHAR)alpha_charset_eng;
	}
#elif defined (FOR_FRENCH)
	prc->alpha_charset = (p_UCHAR)alpha_charset;
#elif defined (FOR_INTERNATIONAL)
	if (pri->flags & PEG_RECFL_INTL_CS)
	{
		prc->alpha_charset = (p_UCHAR)alpha_charset;
	}
	else
	{
		prc->alpha_charset = (p_UCHAR)alpha_charset_eng;
	}
#else
	prc->alpha_charset = (p_UCHAR) alpha_charset;
#endif

	prc->num_charset = (p_UCHAR) num_charset;
	prc->math_charset = (p_UCHAR) math_charset;
	prc->lpunct_charset = (p_UCHAR) lpunct_charset;
	prc->epunct_charset = (p_UCHAR) epunct_charset;
	prc->other_charset = (p_UCHAR) other_charset;

	pri->wsc.num_points = 0;
	pri->wsc.flags = 0;
	pri->wsc.sure_level = prc->lrn_class_level; // WST_DEF_UNSURE_LEVEL;
	pri->wsc.word_dist_in = prc->ws_handle; //+1;
	pri->wsc.line_dist_in = 0;
	pri->wsc.def_h_line = WST_DEF_H_LINE;
	pri->wsc.x_delay = WST_DEF_S_DELAY; // 0; // Delay in 'letters' or 0 -- only on line end

	PegCleanUpContext(pri);

	pri->ok = 1;


#ifdef PEGREC_DEBUG
	PegDebugPrintf("OpenSession: Flags: %x SP_VS_Q: %d MainVoc: %x UserVoc: %x Yield: %x",
	               (int)pri->flags, (int)pri->sp_vs_q, pri->main_dict.hvoc_dir, pri->user_dict.hvoc_dir, pri->InfoCallBack);
#endif

#ifdef TRACE_DMP_DBG
	pri->InfoCallBack = 0;
	DebTraceRec((CGRCTX*)pri);
#endif

	return 0;
}
/* *************************************************************** */
/* *  Recognizes incoming strokes and sends results as ready     * */
/* *************************************************************** */
int DLLEXP CgrCloseSession(CGRCTX context)
{
	int err = 1;
	p_rec_inst_type pri = (p_rec_inst_type) (context);

	if (pri == _NULL)
	{
		goto err;
	}
	if (!pri->ok)
	{
		goto err;
	}

	err = CgrRecognize(0, 0, context);

	FreeInkInfo(&pri->ink_info);

	pri->g_stroke_num = 0;
	pri->baseline.size = 0;
	pri->num_tentative_words = 0;

	pri->ok = 0;

#ifdef PEGREC_DEBUG
	PegDebugPrintf("CloseSession: PRI: %x", pri);
#endif

err:
	return err;
}
/* *************************************************************** */
/* *  Recognizes incoming strokes and sends results as ready     * */
/* *************************************************************** */
int DLLEXP CgrRecognize(int npoints, p_CGR_point_type strokes_win, CGRCTX context)
{
	p_PS_point_type strokes = (p_PS_point_type) strokes_win;
	_INT          i, j, k;
	_INT          num_strokes, prev_nstrokes, len, f, cur_nstrokes;
	_INT          er = 0, skip, min_next_word, cur_tt_word;
	PS_point_type _PTR p_tr = _NULL;
	PS_point_type _PTR stroke;
	p_rec_inst_type pri = (p_rec_inst_type) (context);

	if (pri == _NULL)
	{
		goto err;
	}
	if (!pri->ok)
	{
		goto err;
	}

	if ((p_tr = (PS_point_type _PTR)HWRMemoryAlloc(sizeof(PS_point_type)*(pri->ink_info.num_points + npoints + 4))) == _NULL)
	{
		goto err;
	}

	prev_nstrokes = num_strokes = pri->ink_info.num_strokes;
	cur_nstrokes = 0;

	pri->wsc.x_delay = WST_DEF_S_DELAY; // 0; // Delay in 'letters' or 0 -- only on line end
	if (pri->InfoCallBack != _NULL)
		if ((*pri->InfoCallBack)(pri->ICB_param) == 1)
		{
			pri->wsc.x_delay = 0;    // Do not attempt segmentation to words if there are more strokes coming (save time)
		}

	PRP_02

	if (npoints > 0) // --------- Add current stroke(s) to ink storage -----------------------
	{
		// ------------------- Label new strokes ------------------------------
		HWRMemCpy(p_tr, strokes, npoints*sizeof(PS_point_type));
		len = npoints;
		if (p_tr[len - 1].y >= 0)
		{
			p_tr[len++].y = -1;    // Terminate stroke if was not terminated
		}
		for (i = 0, stroke = p_tr; i < len; i++, stroke++) if (stroke->y < 0)
			{
				stroke->x = (_SHORT) (pri->g_stroke_num + cur_nstrokes++);
			}

		// ------------------- Put them in store ------------------------------
		num_strokes = CreateInkInfo(p_tr, len, &pri->ink_info);
		if (num_strokes == 0)
		{
			goto err;
		}

		if (num_strokes < 0) // Stroke storage is overfilled!!!
		{
			if (pri->wsr.num_finished_words == 0)
			{
				goto err;    // Can't help if nothing developed yet ...
			}

			// ------- Get all unused strokes --------------------------

			for (i = len = 0; i < prev_nstrokes; i++)
			{
				for (j = f = 0; !f && j < pri->wsr.num_words; j++)
				{
					if (((*pri->wsr.pwsa)[j].flags & WS_FL_PROCESSED) == 0)
					{
						continue;
					}
					for (k = 0; !f && k < (*pri->wsr.pwsa)[j].num_strokes; k++)
						if (pri->wsr.stroke_index[k + (*pri->wsr.pwsa)[j].first_stroke_index] == i)
						{
							f = 1;
						}
				}
				if (f)
				{
					continue;
				}

				GetInkStrokeCopy(i, p_tr + len, &pri->ink_info);
				len += GetInkStrokeLen(i, &pri->ink_info);
			}

			// ------- Destroy old store ------------------------------

			FreeInkInfo(&pri->ink_info);
			prev_nstrokes = num_strokes = 0;

			// ------------------- Label new strokes ------------------------------

			HWRMemCpy(p_tr + len, strokes, npoints*sizeof(PS_point_type));
			if (p_tr[len + npoints - 1].y >= 0)
			{
				p_tr[len + (npoints++)].y = -1;    // Terminate stroke if was not terminated
			}
			for (i = cur_nstrokes = 0, stroke = p_tr + len; i < npoints; i++, stroke++) if (stroke->y < 0)
				{
					stroke->x = (_SHORT) (pri->g_stroke_num + cur_nstrokes++);
				}
			len += npoints;

			// ------- Recreate stroke storage -------------------------

			if (len)
			{
				if ((num_strokes = CreateInkInfo(p_tr, len, &pri->ink_info)) <= 0)
				{
					goto err;
				}
			}
			if (num_strokes <= 0)
			{
				goto err;
			}

			// ------- Close previous session of segmentation ----------

			pri->wsc.flags |= WS_FL_LAST;
			pri->wsc.num_points = 0;
			if (WordStrokes(_NULL, &pri->wsc, &pri->wsr))
			{
				goto err;
			}
			pri->wsr.num_words = pri->wsr.num_finished_words = pri->wsr.num_finished_strokes = 0;
		}

		pri->g_stroke_num += cur_nstrokes;
	}

	// ---------------- Process Trace ----------------------------------------------

	if ((pri->flags & PEG_RECFL_NSEG) == 0)  // If segmentation enabled
	{
		// --------- Feed new info to segmentation ------------------------------

		for (i = prev_nstrokes; i <= num_strokes; i++)
		{
			if (i < num_strokes)
			{
				stroke = (p_PS_point_type) GetInkStrokePtr(i, &pri->ink_info);
				len = GetInkStrokeLen(i, &pri->ink_info);

				if (stroke == _NULL || len == 0)
				{
					goto err;
				}

				pri->wsc.flags = 0; // WS_FL_SPGESTURE;
				pri->wsc.num_points = len;
				if (WordStrokes(stroke, &pri->wsc, &pri->wsr))
				{
					break;
				}
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

		if (((pri->flags & PEG_RECFL_NCSEG) == 0) || (pri->wsc.flags & WS_FL_LAST))
		{
			skip = 0;     // If there are strokes waiting, let's get some more before recognizing
			if (pri->InfoCallBack != _NULL) if ((*pri->InfoCallBack)(pri->ICB_param) == 1)
				{
					skip = 1;
				}

			for (i = k = 0; i < pri->wsr.num_words && i < WS_MAX_WORDS - 1; i++)
				if (((*pri->wsr.pwsa)[i].flags & WS_FL_PROCESSED) == 0)
				{
					k++;
				}

			if (k > PR_TOOMANYWORDSWAITING)
			{
				skip = 0;    // Too many words in line, let's recognize them
			}
			if (pri->wsc.flags & WS_FL_LAST)
			{
				skip = 0;    // Need to recognize on last
			}

			if (!skip) // Let's recognize all ready words and try tentative one
			{
				cur_tt_word = min_next_word = 0;
				while ((len = GetNextWordInkCopy(RM_COMBINE_CARRY, min_next_word, &pri->wsr, p_tr, &pri->ink_info, &pri->wswi)) > 0)
				{
					if (pri->wswi.flags & WS_FL_TENTATIVE)
					{
						if (!(pri->flags & PEG_RECFL_TTSEG))
						{
							break;    // If not allowed to work with tentative words
						}

						if (cur_tt_word < pri->num_tentative_words &&  // If there was appropriate tentative word, skip to next
						        pri->tentative_list[cur_tt_word].nword == pri->wswi.nword &&
						        pri->tentative_list[cur_tt_word].len == len)
						{
							cur_tt_word++;
							min_next_word = pri->wswi.nword + 1; // Advance to the next tentative word
							PRP_03
							continue;
						}

						pri->rc.pFuncYield = pri->InfoCallBack; // Allow interrupting of recognition
						pri->rc.FY_param = pri->ICB_param;
						PegResetTentativeStorage(cur_tt_word, pri); // Tentative did not prove itself from this point, remove
					}
					else
					{
						if (pri->num_tentative_words > 0 &&  // If there was appropriate tentative word
						        pri->tentative_list[0].nword == pri->wswi.nword &&
						        pri->tentative_list[0].len == len)
						{
							if (PegValidateNextTentativeWord(pri->tentative_list[0].nparts, pri))
							{
								break;
							}
							HWRMemCpy(&pri->tentative_list[0], &pri->tentative_list[1], sizeof(tentative_list_type)*(PR_TENTATIVELIST_SIZE - 1));
							pri->num_tentative_words--;
							PRP_04
							continue;
						}
						else
						{
							PegResetTentativeStorage(0, pri);    // Tentative did not prove itself from this point, remove
						}

						pri->rc.pFuncYield = _NULL;  // No interruption on real word recognition
					}

					if (pri->rc.p_xd_data) // If there is unfinished word, restore XD
					{
						if (pri->unfinished_data.nword == pri->wswi.nword && pri->unfinished_data.len == len)
						{
							// If word matches current segmentation, use prev XD.
							PRP_05
						}
						else // If word does not match, release unfinished context
						{
							XrlvDealloc((p_xrlv_data_type _PTR)&pri->rc.p_xd_data);
							PRP_06
						}
					}

					PRP_07

					pri->rc.trace = p_tr;
					pri->rc.ii = (_SHORT) len;

					er = PegRecWord(pri);  // < ------ Call recognizer

					PRP_08

					if (pri->rc.p_xd_data) // Save XD for possible next do-recognition
					{
						pri->unfinished_data.nword = pri->wswi.nword;
						pri->unfinished_data.len = len;
						//            pri->unfinished_data.pxd   = pri->rc.p_xd_data;
					}

					if (er == XRLV_YIELD_BREAK)
					{
						break;    // Recognition was aborted
					}

					if (pri->wswi.flags & WS_FL_TENTATIVE)
					{
						i = pri->num_tentative_words;
						if (i >= PR_TENTATIVELIST_SIZE)
						{
							break;    // If too many tentative words, enough!
						}
						pri->tentative_list[i].nword = pri->wswi.nword;
						pri->tentative_list[i].nparts = pri->rr_nparts;
						pri->tentative_list[i].len = len;
						pri->num_tentative_words++;
						cur_tt_word++;

						min_next_word = pri->wswi.nword + 1; // Advance to the next tentative word
					}

					if (pri->InfoCallBack != _NULL) // If there are more strokes waiting, let's go get them
						if ((*pri->InfoCallBack)(pri->ICB_param) == 1)
						{
							break;
						}
				}
			}
		}
	}
	else  // ------------------- If segmentation was off ------------------------
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

			er = PegRecWord(pri);      // Call recognizer
		}
	}

	// ----------- Free memory and say chao ---------------------------------

	if (p_tr)
	{
		HWRMemoryFree(p_tr);
	}

	return 0;

err:
	if (p_tr)
	{
		HWRMemoryFree(p_tr);
	}
	return 1;
}

/* ************************************************************************** */
/* *  Recognize one symbol                                                  * */
/* ************************************************************************** */
int DLLEXP CgrRecognizeSymbol(int npoints, p_CGR_point_type strokes_win, p_CGR_baseline_type baseline, CGRCTX context)
{
	p_PS_point_type  strokes = (p_PS_point_type) strokes_win;
	_INT             err = 0;
	_INT             mwl;
	p_rec_inst_type  pri = (p_rec_inst_type) (context);


	if (pri == _NULL)
	{
		goto err;
	}

	PegCleanUpContext(pri);

	pri->rc.trace = strokes;
	pri->rc.ii = (_SHORT) npoints;

	pri->baseline = *baseline;

	mwl = pri->rc.xrw_max_wlen;
	pri->rc.xrw_max_wlen = 1;

	// -------------- Recognize it! ------------------------------------------------

	err += PegRecWord(pri);      // Call recognizer

	// --------------- Simple! Da? --------------------------------------------------

	pri->rc.xrw_max_wlen = (_SHORT) mwl;

	if (err || pri->recres == _NULL)
	{
		goto err;
	}

	return 0;
err:
	PegCleanUpContext(pri);
	return 1;
}

/* ************************************************************************** */
/* *  Returns current recognized words & info                               * */
/* ************************************************************************** */
long DLLEXP CgrGetAnswers(int what, int nw, int na, CGRCTX context)
{
	_INT   i, j;
	_ULONG result = 0;
	p_INT  iptr;
	p_UCHAR ptr;
	p_rec_inst_type pri = (p_rec_inst_type) (context);

	if (pri == _NULL)
	{
		goto err;
	}
	if (nw >= pri->rr_num_answers)
	{
		goto err;
	}

	switch (what)
	{
		case CGA_NUM_ANSWERS:
			result = pri->rr_num_answers;
			break;

		case CGA_NUM_ALTS:
			if ((iptr = PegGetAnswerBlockPtr(nw, pri)) == _NULL)
			{
				goto err;
			}
			ptr = (p_UCHAR) (iptr + 1);
			for (i = j = 0; i < ((*iptr - 1) << 2); i++) if (ptr[i] == 0)
				{
					j++;
				}
			result = j;
			break;

		case CGA_ALT_WORD:
			if ((iptr = PegGetAnswerBlockPtr(nw, pri)) == _NULL)
			{
				goto err;
			}
			ptr = (p_UCHAR) (iptr + 1);
			for (i = j = 0; i < ((*iptr - 1) << 2); i++)
			{
				if (ptr[i] == 0 || i == 0)
				{
					if (j == na)
					{
						_INT n;
						if (i)
						{
							n = i + 1;
						}
						else
						{
							n = i;    // Move from prev zero
						}
#ifdef PEG_RECINT_UNICODE
						for (k = 0; ptr[n+k] != 0; k ++)
						{
							pri->uans_buf[k] = (UCHR)ptr[n+k];
						}
						pri->uans_buf[k] = 0;
						result = (_ULONG)(&pri->uans_buf[0]);
#else
						result = (_ULONG) (&ptr[n]);
#endif
						break;
					}
					else
					{
						j++;
					}
				}
			}

			break;

		case CGA_ALT_WEIGHT:
			if ((iptr = PegGetAnswerBlockPtr(nw, pri)) == _NULL)
			{
				goto err;
			}
			iptr += *iptr; // Advance to weights block
			result = (_ULONG) (*(iptr + 1 + na));
			break;

		case CGA_ALT_NSTR:
			if ((iptr = PegGetAnswerBlockPtr(nw, pri)) == _NULL)
			{
				goto err;
			}
			iptr += *iptr; // Advance to weights block
			iptr += *iptr; // Advance to strokes block
			result = (_ULONG) ((*iptr) - 1);
			break;

		case CGA_ALT_STROKES:
			if ((iptr = PegGetAnswerBlockPtr(nw, pri)) == _NULL)
			{
				goto err;
			}
			iptr += *iptr; // Advance to weights block
			iptr += *iptr; // Advance to strokes block
			result = (_ULONG) (iptr + 1);
			break;

		default:
			goto err;
	}

	return result;
err:
	return _NULL;
}

/* ************************************************************************** */
/* *  Interface to get and set vexes via letter pictures                    * */
/* ************************************************************************** */
int DLLEXP CgrGetSetPicturesWeights(int operation, void * buf, CGRCTX context)
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

/* ************************************************************************** */
/* *  Get pointer to requested answer block                                 * */
/* ************************************************************************** */
p_INT PegGetAnswerBlockPtr(_INT nw, p_rec_inst_type pri)
{
	_INT   i;
	p_INT  iptr;

	if (pri->recres == _NULL)
	{
		goto err;
	}
	if (nw > pri->rr_num_answers)
	{
		goto err;
	}

	for (i = 0, iptr = (p_INT) pri->recres; i < nw * 3; i++)
	{
		iptr += *iptr;
	}

	return iptr;
err:
	return _NULL;
}

/* ************************************************************************ */
/* *   Main recognition function for one word                             * */
/* ************************************************************************ */
_INT PegRecWord(p_rec_inst_type pri)
{
	_INT  size, pos, nl, er = 0;
	RCB_inpdata_type rcb = { 0 };

	if (!pri->ok)
	{
		goto err;
	}

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
			rcb.prv_size =
			    rcb.prv_dn_pos =
			        rcb.prv_size_sure =
			            rcb.prv_pos_sure = 0;
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
			{
				rcb.flags |= (_SHORT) (RCBF_NEWLINE);
			}
		}

		if (pri->baseline.size)
		{
			rcb.bx_size = (_SHORT) (pri->baseline.size);
			rcb.bx_dn_pos = (_SHORT) (pri->baseline.base);
			rcb.flags |= (_SHORT) RCBF_BOXBORD;
		}

		SetRCB(&rcb, &(pri->rc.stroka));

		er = (low_level(pri->rc.trace, &pri->xrdata, &pri->rc) != SUCCESS);

		SetMultiWordMarksDash(&pri->xrdata);
		SetMultiWordMarksWS(pri->rc.lrn_min_class, &pri->xrdata, &pri->rc);
	}

	// --------------------- Protection scramble -------------------------------------

#if VER_RECPROTECTED
	{
		for (int i = 0; g_rec_protect_locked && i < pri->xrdata.len; i ++)
		{
			(*pri->xrdata.xrd)[i].xr.type += 1;
		}
	}
#endif

	if (!er)
	{
		er = xrlv(&pri->xrdata, &pri->rwg, &pri->rc);
	}

	PegRegNewAnsw(pri, er);

	FreeRWGMem(&pri->rwg);

	return er;
err:
	PegRegNewAnsw(pri, 1);
	FreeRWGMem(&pri->rwg);
	return 1;
}

/* ************************************************************************ */
/* *   Init recognition instance                                          * */
/* ************************************************************************ */
_INT PegRecInit(p_rec_inst_type _PTR ppri)
{
	p_rec_inst_type pri;
	_INT            i;
	_INT            nLdb;
	Automaton       am;
	p_Ldb           pLDB;


	// Count LDB headers.
	for (nLdb = 0; GetLDBImgBody(nLdb) != _NULL; nLdb++) {}

	if ((pri = (p_rec_inst_type) HWRMemoryAlloc(sizeof(rec_inst_type) + sizeof(Ldb) * nLdb)) == _NULL)
	{
		goto err;
	}

	HWRMemSet((p_VOID) (pri), 0, sizeof(rec_inst_type));

	// Cahin LDBs.
	if (nLdb > 0)
	{
		pLDB = (p_Ldb) (pri + 1);
		for (i = 0; (am = (Automaton) GetLDBImgBody(i)) != _NULL; i++)
		{
			pLDB[i].am = am;
			if (i != 0)
			{
				pLDB[i - 1].next = pLDB + i;
			}
		}
		pLDB[i - 1].next = _NULL;
	}
	else
	{
		pLDB = _NULL;
	}

	if (AllocXrdata(&pri->xrdata, XRINP_SIZE))
	{
		goto err;
	}

	dti_load(0, DTI_DTE_REQUEST, (p_VOID _PTR)&(pri->p_dtih));

	HWRMemCpy((p_VOID) (&pri->p_trh), (p_VOID) (&img_trd_header), sizeof(pri->p_trh));
	pri->p_trh.p_tr = (p_UCHAR) (&img_trd_body[0]);

	pri->rc.dtiptr = (p_VOID) (pri->p_dtih);
	pri->rc.vocptr[0] = (p_VOID) (&pri->main_dict);
	pri->rc.vocptr[1] = (p_VOID) (&pri->pref_dict);
	pri->rc.vocptr[2] = (p_VOID) (&pri->suff_dict);
	pri->rc.vocptr[3] = (p_VOID) (&pri->user_dict);
	pri->rc.tr_ptr = (p_VOID) (&pri->p_trh);
	pri->rc.cflptr = (p_VOID) pLDB;

	pri->wsr.pwsa = &(pri->w_str);
	pri->rc.p_ws_wi = (p_VOID) &pri->wswi;

	*ppri = pri;

	return 0;
err:
	if (pri)
	{
		HWRMemoryFree(pri);
	}
	return 1;
}

/* ************************************************************************ */
/* *   Free recognition instance                                          * */
/* ************************************************************************ */
_INT PegRecClose(p_rec_inst_type _PTR ppri)
{
	p_rec_inst_type pri = *ppri;

	if (pri == _NULL)
	{
		goto err;
	}

	pri->wsc.flags |= WS_FL_CLOSE;
	WordStrokes(_NULL, &pri->wsc, &pri->wsr);

	FreeXrdata(&pri->xrdata);
	dti_unload((p_VOID _PTR)&(pri->p_dtih));

	HWRMemoryFree(pri);
	*ppri = _NULL;

	return 0;
err:
	return 1;
}

/* ************************************************************************ */
/* *   Free some meory and cleanup session                                * */
/* ************************************************************************ */
_INT PegCleanUpContext(p_rec_inst_type pri)
{
	if (pri == _NULL)
	{
		goto err;
	}

	FreeInkInfo(&pri->ink_info);
	pri->g_stroke_num = 0;
	pri->baseline.size = 0;

	if (pri->recres)
	{
		HWRMemoryFree(pri->recres);
	}
	pri->recres = _NULL;
	pri->rr_alloc_size = 0;
	pri->rr_filled_size = 0;
	pri->rr_num_answers = 0;
	pri->rr_num_finished_answers = 0;
	pri->num_tentative_words = 0;

	if (pri->rc.p_xd_data)
	{
		XrlvDealloc((p_xrlv_data_type _PTR)(&pri->rc.p_xd_data));
	}

	return 0;
err:
	return 1;
}

/* ************************************************************************** */
/* *  Add word list to RecResult                                            * */
/* ************************************************************************** */
_INT PegAddToAnsw(p_rec_inst_type pri, p_UCHAR answ, p_INT weights, _INT ns, p_INT stroke_ids)
{
	_INT    i;
	_INT    len_a, len_w, len_s, len, na;
	p_VOID  ptr;
	p_INT   iptr;
	p_UCHAR cptr;

	if (answ == _NULL || pri == _NULL || !pri->ok)
	{
		goto err;
	}

	// ------------ Estimate memory ---------------------------------

	len_a = HWRStrLen((_STR) answ) + 1;
	for (i = na = 0; i < len_a; i++) if (answ[i] <= PM_ALTSEP)
		{
			na++;
		}
	len_w = na;
	len_s = ns;

	len = len_a + (len_w + len_s + 3 + 1 + 1)*sizeof(_INT);

	// ------------- Alloc/realloc mmeory ----------------------------

	if (pri->rr_alloc_size < pri->rr_filled_size + len)
	{
		if (pri->recres == _NULL)
		{
			if ((pri->recres = (p_UCHAR) HWRMemoryAlloc(len + PR_ANSW_EXTALLOC)) == _NULL)
			{
				goto err;
			}
			pri->rr_alloc_size = len + PR_ANSW_EXTALLOC;
		}
		else
		{
			if ((ptr = HWRMemoryAlloc(pri->rr_filled_size + len + PR_ANSW_EXTALLOC)) == _NULL)
			{
				goto err;
			}
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
	for (i = 0, cptr = (p_UCHAR) (iptr + 1); i < len_a; i++) if (cptr[i] <= PM_ALTSEP)
		{
			cptr[i] = 0;
		}

	// --------------- Put weights ------ in answer buffer -----------------------
	{
		_USHORT xrsum;
		p_xrd_el_type xrd = &((*pri->xrdata.xrd)[0]);
		// ------------- For tester we place xrsum after the weights ---------------
		for (i = xrsum = 0; i < pri->xrdata.len; i++, xrd++)
		{
			xrsum += (_USHORT) (xrd->xr.type + xrd->xr.attrib + xrd->xr.penalty + xrd->xr.height + xrd->xr.shift + xrd->xr.orient + xrd->xr.depth);
		}

		iptr = (p_INT) (&pri->recres[pri->rr_filled_size]);
		*iptr = 1 + 1 + len_w;
		pri->rr_filled_size += (*iptr) * sizeof(_INT);
		if (weights)
		{
			HWRMemCpy(iptr + 1, weights, len_w * sizeof(_INT));
		}
		*(iptr + 1 + len_w) = xrsum;
	}

	// --------------- Put strokes ids -- in answer buffer -----------------------

	iptr = (p_INT) (&pri->recres[pri->rr_filled_size]);
	*iptr = 1 + len_s;
	pri->rr_filled_size += (*iptr) * sizeof(_INT);
	if (ns && stroke_ids)
	{
		HWRMemCpy(iptr + 1, stroke_ids, len_s * sizeof(_INT));
	}

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
	{
		pri->rr_num_finished_answers++;
	}

	return 0;
err:
	return 1;
}

/* ************************************************************************** */
/* *  Resets counter of tentative storage to finished state                 * */
/* ************************************************************************** */
_INT PegResetTentativeStorage(_INT st_index, p_rec_inst_type pri)
{
	_INT  i;
	p_INT iptr;
	_INT  num_answ, n_tt;

	for (i = n_tt = 0; i < st_index; i++)
	{
		n_tt += pri->tentative_list[i].nparts;
	}

	num_answ = pri->rr_num_finished_answers + n_tt;

	if (pri->rr_num_answers > num_answ)
	{
		// Reset free space pointer to the place of the tentatve words
		if ((iptr = PegGetAnswerBlockPtr(num_answ, pri)) == _NULL)
		{
			goto err;
		}
		pri->rr_filled_size = (_INT) ((p_CHAR) iptr - (p_CHAR) &pri->recres[0]);
		pri->rr_num_answers = num_answ;
	}

	pri->num_tentative_words = st_index;

	return 0;
err:
	return 1;
}

/* ************************************************************************** */
/* *  Resets counter of tentative storage to finished state                 * */
/* ************************************************************************** */
_INT PegValidateNextTentativeWord(_INT nparts, p_rec_inst_type pri)
{

	pri->rr_num_finished_answers += nparts;

	if (pri->rr_num_finished_answers > pri->rr_num_answers)
	{
		// Something's wrong here!
		pri->rr_num_answers = pri->rr_num_finished_answers;
		goto err;
	}

	return 0;
err:
	return 1;
}


/* ************************************************************************** */
/* *  Add word list to RecResult                                            * */
/* ************************************************************************** */
_INT PegRegNewAnsw(p_rec_inst_type pri, _INT er)
{
	_INT    i, j, k, n, m, f;
	_INT    ns, np, len;
	_TRACE  p_tr;
	_INT    stroke_ids[WS_MAX_STROKES];
	_INT    str_sts[WS_MAX_STROKES];
	_UCHAR  answers[w_lim*NUM_RW + NUM_RW];
	_UCHAR  word[w_lim];
	_INT    weights[NUM_RW] = { 0 };
	p_RWS_type prws;
	_UCHAR  parts[w_lim];
	p_xrdata_type xrdata = &(pri->xrdata);

	// ------------ Write down stroke lineout ----------------------

	for (i = 1, ns = 0, p_tr = pri->rc.trace; i < pri->rc.ii; i++)
		if (p_tr[i].y < 0)
		{
			str_sts[ns] = i;
			stroke_ids[ns++] = p_tr[i].x;
		}

	if (p_tr[0].x > 0)
	{
		str_sts[ns] = 0;    // Save trashed carry dash -- if this id is more than 0, it means that there is salvaged dash ID.
		stroke_ids[ns++] = p_tr[0].x;
	}

	// --------------------- Store answers -----------------------------------------

	prws = (p_RWS_type) (pri->rwg.rws_mem);

	if (!er && prws != _NULL) // Normal asnwer registration
	{

		for (i = 1, np = 1, parts[0] = 0; i < pri->rwg.size; i++)
		{
			if (prws[i].type != RWST_SYM)
			{
				break;
			}
			if (prws[i].sym == ' ')
			{
				parts[np++] = prws[i].xrd_beg;
			}
		}
		parts[np] = (_UCHAR) xrdata->len;
		pri->rr_nparts = np;

		for (n = 0; n < np; n++)
		{
			for (i = j = k = 0; i < pri->rwg.size; i++)
			{
				if (prws[i].type == RWST_SYM)
				{
					if ((pri->flags & PEG_RECFL_CAPSONLY) && IsLower(prws[i].sym))
					{
						prws[i].sym = (_UCHAR) ToUpper(prws[i].sym);
					}
					if (prws[i].xrd_beg >= parts[n] && prws[i].xrd_beg < parts[n + 1] && prws[i].sym != ' ')
					{
						answers[j++] = prws[i].sym;
						weights[k] = prws[i].weight;
					}
				}
				else
					if (prws[i].type == RWST_NEXT)
					{
						k++;
						answers[j++] = PM_ALTSEP;
					}
			}
			answers[j] = 0;
			len = j + 1;

			for (i = j = 0; i <= len; i++)   // Remove duplicates
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
						else
							if (word[m++] != answers[k])
							{
								f++;
							}
					}

					j = 0;
				}
				else
				{
					word[j++] = answers[i];
				}
			}

			if (np > 1) // Separate stroke belongings
			{
				_INT  s = 0;
				_INT  sis[WS_MAX_STROKES] = { 0 };

				for (i = parts[n]; i < parts[n + 1]; i++) // Go through xr elems and register strokes
				{
					j = (*xrdata->xrd)[i].begpoint;
					for (k = 0; k < ns; k++) // Find which stroke the xr belongs to
					{
						if (j < str_sts[k])
						{
							if (stroke_ids[k] < 0)
							{
								break;    // Already used
							}
							sis[s++] = stroke_ids[k];
							stroke_ids[k] = -1;
							break;
						}
					}
				}

				if (n == np - 1) // Check if there are loose strokes left -- attach them to last word
				{
					for (k = 0; k < ns; k++) if (stroke_ids[k] >= 0)
						{
							sis[s++] = stroke_ids[k];
						}
				}

				PegAddToAnsw(pri, (p_UCHAR) answers, weights, s, &(sis[0]));
			}
			else
			{
				PegAddToAnsw(pri, (p_UCHAR) answers, weights, ns, &(stroke_ids[0]));
			}
		}
	}
	else // Error return -- label it as such
	{
		pri->rr_nparts = 1;
		HWRStrCpy((p_CHAR) answers, "???");
		PegAddToAnsw(pri, (p_UCHAR) answers, weights, ns, &(stroke_ids[0]));
	}

	return 0;
}

/* ************************************************************************** */
/* *  Debug function of PegRec                                              * */
/* ************************************************************************** */

#ifdef PEGREC_DEBUG

/* ************************************************************************** */
/* *  Debug printf                                                          * */
/* ************************************************************************** */
#ifdef _WIN32_WCE

#include <windows.h>
#include <tchar.h>

///int UNICODEtoStr(char * str, TCHAR * tstr, int cMax)
// {
//  int i;
//
// for (i = 0; i < cMax && tstr[i] != 0; i ++) str[i] = ((unsigned char)tstr[i]);
//  str[i] = 0;
//
//  return i;
// }

void PegDebugPrintf(char * form, ...)
{
	char str[256];
	TCHAR buffer[256];
	TCHAR format[256];
	va_list marker;
	int iLen;
	DWORD dw;
	static HANDLE hFile = NULL;

	if(!hFile)
		hFile = CreateFile(L"\\crgdebug.txt", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
		                   FILE_ATTRIBUTE_NORMAL, NULL);
	else
		hFile = CreateFile(L"\\crgdebug.txt", GENERIC_WRITE, 0, NULL, OPEN_EXISTING,
		                   FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile==NULL)
	{
		return;
	}

	SetFilePointer( hFile, 0, NULL,  FILE_END);

	for (int i = 0; i < 255 && form[i] != 0; i ++)
	{
		format[i] = form[i];
	}
	format[i] = 0;

	va_start(marker, form);
	wvsprintf(buffer, format, marker);
	va_end(marker);
	iLen = _tcslen(buffer);
	buffer[iLen++] = 0x0d;
	buffer[iLen++] = 0x0a;
	buffer[iLen] = 0;

	UNICODEtoStr(&str[0], buffer, 254);
	WriteFile(hFile, (LPCVOID)&str[0], iLen/*sizeof(buffer[0])*/, &dw, NULL);
	CloseHandle(hFile);
}

#else

#include <stdio.h>
#include <stdarg.h>

void PegDebugPrintf(char * format, ...)
{
	char buffer[255];
	va_list marker;
	FILE * file = 0;

	va_start(marker, format);
	vsprintf(buffer, format, marker);
	va_end(marker);

	if ((file = fopen("\\tentatve.log", "a+t")) != 0)
	{
		fprintf(file, "%s\n", buffer);
		fclose (file);
	}
}
#endif

#endif // PEGREC_DEBUG


/* ************************************************************************** */
/* *  Psion stuff                                                           * */
/* ************************************************************************** */
#ifdef _PSION_DLL
#include <e32std.h>
GLDEF_C TInt E32Dll(TDllReason /*aReason*/)
// DLL entry point
{
	return(KErrNone);
}
#endif /* _PSION_DLL */
