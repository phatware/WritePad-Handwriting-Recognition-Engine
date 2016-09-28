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

#define _REQ_WIN
#include "ams_mg.h"
#include "wginteractivelrn.h"
#include "resource.h"

#include "WGTemplate.h"
#include "wggbl.h"
#include "WGDTL.H"
#include "hwr_sys.h"
#include "rec-main.h"


#include "wghlv.h"
#include "pensub.h32"
#include "wgtrc.h"
#include "wgrec.h"

#include "xrword.h"

static xrinp_type _xrinp[XRINP_SIZE];
static int _nxr;

struct ListItem
{
	int score;
	int var;
};

BOOL MatchLetter(rec_info_type _PTR pri, wchar_t code, int* score, int* var)
{
	_SHORT use_v_inf;
	xrcm_type _PTR xrcm;
	_SHORT m_cm, m_ba;

	p_xrdata_type xrdata = (p_xrdata_type) pri->pxrdata;
	int len = xrdata->len;
	m_cm = RIRCX(pri)->caps_mode;
	m_ba = RIRCX(pri)->bad_amnesty;
	dti_lock(RIRCX(pri)->dtiptr);
	if (xrmatr_alloc(RIRCX(pri), xrdata, &xrcm) != 0)
	{
		dti_unlock(RIRCX(pri)->dtiptr);
		return false;
	}

	xrcm->word[0] = (_UCHAR) code;
	xrcm->word[1] = '\0';

	//look = NOPRINT;
	use_v_inf |= XRMC_DOTRACING;
	SetInitialLine(DTI_XR_SIZE / 3, xrcm);
	CountWord(xrcm->word, xrcm->caps_mode, use_v_inf, xrcm);

	*score = (*xrcm->s_out_line)[len - 1];
	*var = xrcm->nvar_vect[len - 1];

	xrmatr_dealloc(&xrcm);
	RIRCX(pri)->caps_mode = m_cm;
	RIRCX(pri)->bad_amnesty = m_ba;
	dti_unlock(RIRCX(pri)->dtiptr);
	return true;
}

BOOL LrnMatchLetter(HWND hWnd)
{
	rec_info_type _PTR pri = (rec_info_type _PTR)recGetRCinfo();
	char strbuf[10];
	GetDlgItemText(hWnd, IDC_ED_TEMPLATE_CODE, strbuf, 10);
	wchar_t code = (wchar_t) htoi(strbuf);
	int score, var;
	MatchLetter(pri, code, &score, &var);

	return true;
}

static BOOL AddTemplate(HWND hWnd)
{
	rec_info_type _PTR pri = (rec_info_type _PTR)recGetRCinfo();
	char strbuf[10];
	GetDlgItemText(hWnd, IDC_ED_TEMPLATE_CODE, strbuf, 10);
	wchar_t code = (wchar_t) htoi(strbuf);
	AddTemplateAndConfirm(hWnd, code, (xrdata_type*) pri->pxrdata);
	return TRUE;
}

LRESULT CALLBACK InteractiveLrnDlgProc(HWND hWnd, UINT message, WPARAM wParam,
                                       LPARAM lParam)
{
	INT_PTR result;
	static char buf[10];
	wchar_t code;
	switch (message)
	{
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BUTTON_LRNSAVE:
					GetDlgItemText(hWnd, IDC_ED_TEMPLATE_CODE, buf, 10);
					code = (wchar_t) htoi(buf);
					result = code << 16;
					result |= IDOK;
					EndDialog(hWnd, result);
					return TRUE;

				case IDC_LRNADDTEMPLATE:
					return AddTemplate(hWnd);
				case IDC_LRNMATCHTEST:
					LrnMatchLetter(hWnd);
					return TRUE;

				case IDC_LRNADDFEATURE:
				case IDCANCEL:
					EndDialog(hWnd, 0);
					return FALSE;
			}
	}
	return FALSE;
}

void SetXrData(LrnParam* param)
{
	p_LAB_XRDATA plab = param->xrs;
	int i;

	p_rec_info_type pri = recGetRCinfo();
	p_xrdata_type pxrdata = (p_xrdata_type) (pri->pxrdata);
	xrd_el_type _PTR xrel = (p_xrd_el_type) (((p_xrdata_type) pri->pxrdata)->xrd);

	pxrdata->len = param->n_xrs;
	for (i = 0; i < param->n_xrs; i++, ++plab, ++xrel)
	{
		xrel->xr.type = plab->xdxr.xnxr;
		xrel->xr.attrib = plab->xdxr.xna;
		xrel->xr.penalty = plab->xdxr.xnp;
		xrel->xr.height = plab->xdxr.xnh;

		xrel->xr.shift = plab->xdxr.xns;
		xrel->xr.orient = plab->xdxr.xno;
		xrel->xr.depth = plab->xdxr.xnd;

		xrel->hotpoint = plab->xdhotpt;
		xrel->begpoint = plab->xdbegpt;
		xrel->endpoint = plab->xdendpt;
		xrel->box_left = 0;
		xrel->box_up = 0;
		xrel->box_right = 0;
		xrel->box_down = 0;
	}
}

void createInteractiveLearn(HWND hWnd, LrnParam* param)
{
	SetXrData(param);

	INT_PTR result = DialogBox(hInst, MAKEINTRESOURCE(IDD_DLG_REGISTER_TEMPLATE), hWnd,
	                           (DLGPROC) MakeProcInstance((FARPROC) InteractiveLrnDlgProc, hInst));
}
