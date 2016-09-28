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

#include "resource.h"
#include "ams_mg.h"
#include "WGTemplate.h"
#include "wggbl.h"
#include "WGDTL.H"
#define _REQ_WIN
#include "wg_stuff.h"
#include "hwr_sys.h"

using namespace wp_engine;

static DTLTemplate g_template;
static char _szBuf[256];


LRESULT CALLBACK templateDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
p_rec_info_type recGetRCinfo(void);

static wchar_t _symbol = 0;
static int _nvar = 0;
static int _time = 0;
static HWND htemplate = NULL;

void tplCreateTemplateDialog(HWND hWnd, wchar_t symbol, int nvar, int time)
{
	g_template.Clear();
	p_rec_info_type rc = recGetRCinfo();
	rc_type* prc = (rc_type*) (((p_rec_info_type) rc)->prc);

	p_dti_descr_type dscr = (p_dti_descr_type) prc->dtiptr;

	g_template.Read(*dscr);
	_symbol = symbol;
	_nvar = nvar;
	_time = time;
	if (htemplate == NULL)
	{
		htemplate = CreateDialog(hInst, MAKEINTRESOURCE(IDD_TEMPLATE), hWnd,
		                         (DLGPROC) MakeProcInstance((FARPROC) templateDlgProc, hInst));
	}
	else
	{
		SendMessage(htemplate, WM_REFRESH_DLG, 0, 0);
	}
	ShowWindow(htemplate, SW_SHOW);
}

char* Hex2String(const char* mem, int size, char* buf)
{
	int i, col = 0;
	char charbuf[10];
	static const char symtbl [] = "0123456789ABCDEF";
	buf[0] = 0;
	for (i = 0; i < size; i++)
	{
		sprintf(charbuf, "%c", symtbl[(unsigned char) (mem[i]) >> 4]);
		strcat(buf, charbuf);
		if (col < size * 2 - 1 && (col + 1) % 5 == 0)
		{
			strcat(buf, " ");
		}
		col++;
		sprintf(charbuf, "%c", symtbl[mem[i] & 0x0f]);
		strcat(buf, charbuf);
		if (col < size * 2 - 1 && (col + 1) % 5 == 0)
		{
			strcat(buf, " ");
		}

		col++;
	}
	return buf;
}

int htoi(const char* str)
{
	int res = 0;
	int val;
	while (*str)
	{
		val = 0;
		if (*str >= 'a' && *str <= 'z')
		{
			val = *str - 'a' + 10;
		}
		else
			if (*str >= 'A' && *str <= 'Z')
			{
				val = *str - 'A' + 10;
			}
			else
				if (*str >= '0' && *str <= '9')
				{
					val = *str - '0';
				}
				else
				{
					break;
				}

		res *= 16;
		res += val;
		++str;
	}
	return res;
}

static char* String2Mem(const char* str, char* dstmem, int size)
{
	char* mem = new char[size];
	char onebyte;
	bool size_invalid = false;
	const char* pstr = str;
	char* dst = mem;
	int cnt = 0;
	int nwrote = 0;
	*dst = 0;
	while (*pstr)
	{
		if (nwrote >= size)
		{
			size_invalid = true;
			break;
		}

		if (*pstr >= 'a' && *pstr <= 'f')
		{
			onebyte = *pstr - 'a' + 10;
		}
		else
			if (*pstr >= 'A' && *pstr <= 'F')
			{
				onebyte = *pstr - 'A' + 10;
			}
			else
				if (*pstr >= '0' && *pstr <= '9')
				{
					onebyte = *pstr - '0';
				}
				else
				{
					++pstr;
					continue;
				}

		if (cnt & 1)
		{
			*dst |= onebyte;
			++nwrote;
			++dst;
		}
		else
		{
			*dst = onebyte << 4;
		}

		++cnt;
		++pstr;
	}

	if (size_invalid || nwrote != size)
	{
		delete [] mem;
		return NULL;
	}

	memcpy(dstmem, mem, size);
	return dstmem;
}


static bool GetTemplatePos(HWND hWnd, _UCHAR& symbol, int& nvar, int& timestep)
{
	GetDlgItemText(hWnd, IDC_ED_SYMBOL, _szBuf, 256);
	symbol = htoi(_szBuf);

	GetDlgItemText(hWnd, IDC_ED_NUMVAR, _szBuf, 256);
	nvar = atoi(_szBuf);

	GetDlgItemText(hWnd, IDC_ED_TIMESTEP, _szBuf, 256);
	timestep = atoi(_szBuf);
	return true;
}

static bool SaveTemplate(HWND hWnd)
{
	_UCHAR symbol;
	int nvar, timestep;
	GetTemplatePos(hWnd, symbol, nvar, timestep);
	wp_engine::DTLTemplate::Varience varience;
	wp_engine::VarienceAttr attr;
	if (g_template.GetVarience(symbol, nvar, varience, attr) == false ||
	        varience.empty() || timestep >= varience.size())
	{
		MessageBox(hWnd, "invalid symbol/varience/timestep setting", NULL, MB_OK);
		return false;
	}

	GetDlgItemText(hWnd, IDC_ED_TYPE, _szBuf, 256);
	if (String2Mem(_szBuf, (char*) (varience[timestep].xtc), DTI_XTB_SIZE) == NULL)
	{
		MessageBox(hWnd, "invalid type string", NULL, MB_OK);
		return false;
	}

	GetDlgItemText(hWnd, IDC_ED_HEIGHT, _szBuf, 256);
	if (String2Mem(_szBuf, (char*) (varience[timestep].xhc), DTI_XHB_SIZE) == NULL)
	{
		MessageBox(hWnd, "invalid height string", NULL, MB_OK);
		return false;
	}

	GetDlgItemText(hWnd, IDC_ED_ORIENT, _szBuf, 256);
	if (String2Mem(_szBuf, (char*) (varience[timestep].xoc), DTI_XOB_SIZE) == NULL)
	{
		MessageBox(hWnd, "invalid orient string", NULL, MB_OK);
		return false;
	}

	GetDlgItemText(hWnd, IDC_ED_DEPTH, _szBuf, 256);
	if (String2Mem(_szBuf, (char*) (varience[timestep].xzc), DTI_XZB_SIZE) == NULL)
	{
		MessageBox(hWnd, "invalid depth string", NULL, MB_OK);
		return false;
	}

	GetDlgItemText(hWnd, IDC_ED_SHIFT, _szBuf, 256);
	if (String2Mem(_szBuf, (char*) (varience[timestep].xsc), DTI_XSB_SIZE) == NULL)
	{
		MessageBox(hWnd, "invalid shift string", NULL, MB_OK);
		return false;
	}

	GetDlgItemText(hWnd, IDC_ED_MAINTYPE, _szBuf, 256);
	varience[timestep].type = _szBuf[0] - ' ';

	GetDlgItemText(hWnd, IDC_ED_MAINHEIGHT, _szBuf, 256);
	varience[timestep].height = (_UCHAR) atoi(_szBuf);

	GetDlgItemText(hWnd, IDC_ED_PENALTY, _szBuf, 256);
	varience[timestep].penl = (_UCHAR) atoi(_szBuf);

	GetDlgItemText(hWnd, IDC_ED_MAINATTR, _szBuf, 256);
	varience[timestep].attr = htoi(_szBuf);

	GetDlgItemText(hWnd, IDC_ED_VEIS, _szBuf, 256);
	attr.veis = (_UCHAR) htoi(_szBuf);

	GetDlgItemText(hWnd, IDC_ED_VEXS, _szBuf, 256);
	attr.vexs = (_UCHAR) htoi(_szBuf);

	GetDlgItemText(hWnd, IDC_ED_HEAD_POS, _szBuf, 256);
	attr.pos = htoi(_szBuf);

	GetDlgItemText(hWnd, IDC_ED_HEAD_SIZE, _szBuf, 256);
	attr.size = htoi(_szBuf);

	GetDlgItemText(hWnd, IDC_ED_SYMLANG, _szBuf, 256);
	attr.lang = htoi(_szBuf);

	GetDlgItemText(hWnd, IDC_ED_SYMLOC, _szBuf, 256);
	attr.loc = htoi(_szBuf);

	if (g_template.SetVarience(symbol, nvar, varience, attr) == false)
	{
		MessageBox(hWnd, "error setvarience", NULL, MB_OK);
		return false;
	}
	return true;
}

static bool CommitTemplate(HWND hWnd)
{
	p_rec_info_type rc = recGetRCinfo();
	rc_type* prc = (rc_type*) (((p_rec_info_type) rc)->prc);
	p_dti_descr_type dscr = (p_dti_descr_type) prc->dtiptr;
	HWRMemoryFreeHandle(dscr->h_dte);

	int size;
	g_template.SizeOfWrite(&size);

	dscr->h_dte = HWRMemoryAllocHandle(size);
	dscr->p_dte = (p_UCHAR) HWRMemoryLockHandle(dscr->h_dte);

	g_template.Write((char*) dscr->p_dte, size);
	HWRMemoryUnlockHandle(dscr->h_dte);
	dscr->p_dte = NULL;
	return true;
}

static bool UpdateDlgItems(HWND hWnd, _UCHAR sym = 0, int var_id = 0, int time = 0)
{
	_UCHAR symbol;
	int nvar, timestep;

	if (sym == 0)
	{
		GetTemplatePos(hWnd, symbol, nvar, timestep);
	}
	else
	{
		symbol = sym;
		nvar = var_id;
		timestep = time;

		_itoa(symbol, _szBuf, 16);
		SetWindowText(GetDlgItem(hWnd, IDC_ED_SYMBOL), _szBuf);

		_itoa(nvar, _szBuf, 10);
		SetWindowText(GetDlgItem(hWnd, IDC_ED_NUMVAR), _szBuf);

		_itoa(timestep, _szBuf, 10);
		SetWindowText(GetDlgItem(hWnd, IDC_ED_TIMESTEP), _szBuf);
	}

	wp_engine::DTLTemplate::Varience varience;
	wp_engine::VarienceAttr attr;
	if (g_template.GetVarience(symbol, nvar, varience, attr) == false ||
	        varience.empty() || timestep >= varience.size())
	{
		MessageBox(hWnd, "invalid symbol/varience/timestep setting", NULL, MB_OK);
		return false;
	}

	Hex2String((char*) varience[timestep].xtc, DTI_XTB_SIZE, _szBuf);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_TYPE), _szBuf);

	Hex2String((char*) varience[timestep].xhc, DTI_XHB_SIZE, _szBuf);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_HEIGHT), _szBuf);

	Hex2String((char*) varience[timestep].xoc, DTI_XOB_SIZE, _szBuf);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_ORIENT), _szBuf);

	Hex2String((char*) varience[timestep].xsc, DTI_XSB_SIZE, _szBuf);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_SHIFT), _szBuf);

	Hex2String((char*) varience[timestep].xzc, DTI_XZB_SIZE, _szBuf);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_DEPTH), _szBuf);

	int max_time = varience.size();
	int max_var = g_template.GetNumVarience(symbol);
	_itoa(max_var, _szBuf, 10);
	SetWindowText(GetDlgItem(hWnd, IDC_TEXT_MAXVAR), _szBuf);

	_itoa(max_time, _szBuf, 10);
	SetWindowText(GetDlgItem(hWnd, IDC_TEXT_MAXTIME), _szBuf);

	_itoa(varience[timestep].attr, _szBuf, 16);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_MAINATTR), _szBuf);

	_itoa(varience[timestep].height, _szBuf, 10);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_MAINHEIGHT), _szBuf);

	_itoa(varience[timestep].penl, _szBuf, 10);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_PENALTY), _szBuf);

	_szBuf[0] = ' ' + varience[timestep].type;
	_szBuf[1] = 0;
	SetWindowText(GetDlgItem(hWnd, IDC_ED_MAINTYPE), _szBuf);

	_itoa(attr.pos, _szBuf, 16);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_HEAD_POS), _szBuf);

	_itoa(attr.size, _szBuf, 16);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_HEAD_SIZE), _szBuf);

	_itoa(attr.veis, _szBuf, 16);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_VEIS), _szBuf);

	_itoa(attr.lang, _szBuf, 16);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_SYMLANG), _szBuf);

	_itoa(attr.loc, _szBuf, 16);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_SYMLOC), _szBuf);

	_itoa(attr.vexs, _szBuf, 16);
	SetWindowText(GetDlgItem(hWnd, IDC_ED_VEXS), _szBuf);

	return true;
}

static void MoveVarNum(HWND hWnd, int inc)
{
	GetDlgItemText(hWnd, IDC_ED_NUMVAR, _szBuf, 256);
	int var = atoi(_szBuf);
	GetDlgItemText(hWnd, IDC_TEXT_MAXVAR, _szBuf, 256);
	int maxvar = atoi(_szBuf);

	var += inc;
	if (var >= 0 && var < maxvar)
	{
		_itoa(var, _szBuf, 10);
		SetWindowText(GetDlgItem(hWnd, IDC_ED_NUMVAR), _szBuf);
		UpdateDlgItems(hWnd);
	}
}

static void MoveTimeStep(HWND hWnd, int inc)
{
	GetDlgItemText(hWnd, IDC_ED_TIMESTEP, _szBuf, 256);
	int timestep = atoi(_szBuf);
	GetDlgItemText(hWnd, IDC_TEXT_MAXTIME, _szBuf, 256);
	int maxstep = atoi(_szBuf);

	timestep += inc;
	if (timestep >= 0 && timestep < maxstep)
	{
		_itoa(timestep, _szBuf, 10);
		SetWindowText(GetDlgItem(hWnd, IDC_ED_TIMESTEP), _szBuf);
		UpdateDlgItems(hWnd);
	}
}

void AddTimestep(HWND hWnd)
{
	DTLTemplate::Varience var;
	VarienceAttr attr;

	_UCHAR symbol;
	int nvar, timestep;
	GetTemplatePos(hWnd, symbol, nvar, timestep);

	if (g_template.GetVarience(symbol, nvar, var, attr) == false ||
	        var.empty() || timestep >= var.size())
	{
		MessageBox(hWnd, "current timestep is invalid", NULL, MB_OK);
		return;
	}
	DTLTemplate::Feature feature = var[timestep];
	var.insert(var.begin() + timestep + 1, feature);
	g_template.SetVarience(symbol, nvar, var, attr);
	UpdateDlgItems(hWnd);
}

void RemoveTimestep(HWND hWnd)
{
	DTLTemplate::Varience var;
	VarienceAttr attr;

	_UCHAR symbol;
	int nvar, timestep;
	GetTemplatePos(hWnd, symbol, nvar, timestep);

	if (g_template.GetVarience(symbol, nvar, var, attr) == false ||
	        var.empty() || timestep >= var.size())
	{
		MessageBox(hWnd, "current timestep is invalid", NULL, MB_OK);
		return;
	}

	var.erase(var.begin() + timestep);
	g_template.SetVarience(symbol, nvar, var, attr);
	UpdateDlgItems(hWnd);
}

BOOL AddTemplateAndConfirm(HWND hWnd, wchar_t code, xrdata_type* xrdata)
{
	BOOL  result;
	g_template.Clear();
	p_rec_info_type rc = recGetRCinfo();
	rc_type* prc = (rc_type*) (((p_rec_info_type) rc)->prc);

	p_dti_descr_type dscr = (p_dti_descr_type) prc->dtiptr;

	g_template.Read(*dscr);

	DTLTemplate::Varience var;
	DTLTemplate::Feature feature;
	var.reserve(xrdata->len);
	p_xrd_type pxrd = xrdata->xrd;
	xrd_el_type* pxr = &(*pxrd)[0];
	for (int i = 0; i < xrdata->len; i++, ++pxr)
	{
		memset(&feature, 0, sizeof(feature));
		feature.attr = pxr->xr.attrib;
		feature.height = pxr->xr.height;
		feature.penl = pxr->xr.penalty;
		feature.type = pxr->xr.type;

		feature.xtc[pxr->xr.type / 2] = (pxr->xr.type & 1) ? 12 : 12 << 4;
		feature.xhc[pxr->xr.height / 2] = (pxr->xr.height & 1) ? 12 : 12 << 4;
		feature.xsc[pxr->xr.shift / 2] = (pxr->xr.shift & 1) ? 12 : 12 << 4;
		feature.xzc[pxr->xr.depth / 2] = (pxr->xr.depth & 1) ? 12 : 12 << 4;
		feature.xoc[pxr->xr.orient / 2] = (pxr->xr.orient & 1) ? 12 : 12 << 4;
		var.push_back(feature);
	}

	VarienceAttr attr;
	attr.lang = 0;
	attr.loc = 0x0f;
	attr.pos = 0x67;
	attr.size = 0x67;
	attr.veis = 0xc1;
	attr.vexs = 0;
	int id_var = g_template.GetNumVarience(code);
	if (g_template.AddVarience(code, var, attr) == true)
	{
		result = TRUE;
		_symbol = code;
		_nvar = id_var;
		_time = 0;
		DialogBox(hInst, MAKEINTRESOURCE(IDD_TEMPLATE), hWnd,
		          (DLGPROC) MakeProcInstance((FARPROC) templateDlgProc, hInst));
	}
	else
	{
		result = FALSE;
	}

	return result;
}

void AddVarience(HWND hWnd)
{
	DTLTemplate::Varience var;
	VarienceAttr attr;

	_UCHAR symbol;
	int nvar, timestep;
	GetTemplatePos(hWnd, symbol, nvar, timestep);

	if (g_template.GetVarience(symbol, nvar, var, attr) == false ||
	        var.empty() || timestep >= var.size())
	{
		MessageBox(hWnd, "current timestep is invalid", NULL, MB_OK);
		return;
	}

	g_template.AddVarience(symbol, var, attr);
	UpdateDlgItems(hWnd);
}

void RemoveVarience(HWND hWnd)
{
	_UCHAR symbol;
	int nvar, timestep;
	GetTemplatePos(hWnd, symbol, nvar, timestep);

	g_template.EraseVarience(symbol, nvar);
	UpdateDlgItems(hWnd);
}

LRESULT CALLBACK templateDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
		{
			UpdateDlgItems(hWnd, _symbol, _nvar, _time);
			return 0;
		}
		case WM_MOUSEWHEEL:
		{
			short zDelta;
			zDelta = (short) HIWORD(wParam);
			SaveTemplate(hWnd);
			if (zDelta > 0)
			{
				if (LOWORD(wParam) == MK_CONTROL)
				{
					MoveVarNum(hWnd, 1);
				}
				else
				{
					MoveTimeStep(hWnd, 1);
				}
			}
			else
			{
				if (LOWORD(wParam) == MK_CONTROL)
				{
					MoveVarNum(hWnd, -1);
				}
				else
				{
					MoveTimeStep(hWnd, -1);
				}
			}
			break;
		}
		case WM_REFRESH_DLG:
		{
			UpdateDlgItems(hWnd, _symbol, _nvar, _time);
			break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
				case IDC_BTN_SAVETIMESTEP:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						if (SaveTemplate(hWnd))
						{
							CommitTemplate(hWnd);
						}
						//EndDialog(hWnd, TRUE);
					}
					break;
				case IDC_BTN_LOADTIMESTEP:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						UpdateDlgItems(hWnd);
					}
					break;
				case IDC_BTN_TIMEADD:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						AddTimestep(hWnd);
					}
					break;
				case IDC_BTN_TIMEREMOVE:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						RemoveTimestep(hWnd);
					}
					break;
				case IDC_BTN_VARADD:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						AddVarience(hWnd);
					}
					break;
				case IDC_BTN_VARREMOVE:
					if (HIWORD(wParam) == BN_CLICKED)
					{
						RemoveVarience(hWnd);
					}
					break;
				case IDCANCEL:
					EndDialog(hWnd, FALSE);
					return TRUE;
			}
			break;
		default:
			break;
	}
	return FALSE;
}

static LPSTR GetSaveFilename(HWND hWnd)
{
	_szBuf[0] = '\0';
	OPENFILENAME ofn = { 0, };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = hWnd;
	ofn.hInstance = hInst;
	ofn.lpstrFilter = "DTL Files(*.dtl)\0*.dtl\0";
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter = 0;
	ofn.nFilterIndex = 1;
	ofn.nMaxFile = 255;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrFileTitle = NULL;
	ofn.lpstrFile = (LPSTR) _szBuf;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrTitle = NULL;
	ofn.lpstrDefExt = (LPSTR) "dtl";
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;

	if (!GetSaveFileName(&ofn))
	{
		_szBuf[0] = 0;
	}
	return _szBuf;
} /* flmGetSaveTapName */

void SaveTemplateFile(HWND hWnd)
{
	static char target_path[256] = { 0, };

	if (target_path[0] == '\0')
	{
		LPSTR outfile = GetSaveFilename(hWnd);
		strcpy_s(target_path, outfile);
	}

	if (target_path[0])
	{
		DTLTemplate tpl;
		tpl.Clear();
		p_rec_info_type rc = recGetRCinfo();
		rc_type* prc = (rc_type*) (((p_rec_info_type) rc)->prc);

		p_dti_descr_type dscr = (p_dti_descr_type) prc->dtiptr;

		tpl.Read(*dscr);
		if (tpl.WriteFile(target_path) == true)
		{
			MessageBox(hWnd, "Saved successfully", NULL, MB_OK);
		}
		else
		{
			MessageBox(hWnd, "failed saving", NULL, MB_OK);
		}
	}
}
