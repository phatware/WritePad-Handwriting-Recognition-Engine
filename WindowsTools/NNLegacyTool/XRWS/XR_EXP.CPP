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

#include "ams_mg.h"                           /* Most global definitions     */
#include "zctype.h"
#include "hwr_sys.h"

#if PG_DEBUG

#include <stdio.h>
#include <stdlib.h>

#include "def.h"
#include "lowlevel.h"
#include "xr_names.h"
#include "xrword.h"                           /* Definition for use as engine*/
#include "pg_debug.h"
#include "wg_stuff.h"
#include "xr_exp.h"
#include "pws.h"
#include "xrlv.h"
#include "polyco.h"
#include "bitmapco.h"


// ---------------- XrData prameters learning -----------------------------

#define XEM_XR 1
#define XEM_H  2
#define XEM_SH 3
#define XEM_SZ 4
#define XEM_OR 5

#define XEM_CUR XEM_SZ
#define xr_exp_snn xr_exp

#if XEM_CUR == XEM_XR
#define XE_BSIZE XR_COUNT
#define XE_TYPE   'X'
#endif
#if XEM_CUR == XEM_H
#define XE_BSIZE XH_COUNT
#define XE_TYPE   'H'
#endif
#if XEM_CUR == XEM_SH
#define XE_BSIZE XS_COUNT
#define XE_TYPE   'S'
#endif
#if XEM_CUR == XEM_SZ
#define XE_BSIZE XZ_COUNT
#define XE_TYPE   'L'
#endif
#if XEM_CUR == XEM_OR
#define XE_BSIZE XO_COUNT
#define XE_TYPE   'O'
#endif

typedef struct
{
	_INT x;
	_INT y;
} loc_type, _PTR p_loc_type;

typedef struct
{
	_UCHAR sym;
	_UCHAR nvar;
	_UCHAR pvoc;
	_UCHAR pinp;
} mtrace_type, _PTR p_mtrace_type;

class var_descr_class
{
protected:
	_INT var_stats[DTI_XR_SIZE][XE_BSIZE][2];

public:
	var_descr_class(void)
	{
		HWRMemSet(var_stats, 0, sizeof(var_stats));
	}

	inline _VOID IncXrStat(_INT pos, _INT loc)
	{
		var_stats[pos][loc][0] ++;
		return;
	}
	inline _INT  GetXrStat(_INT pos, _INT loc)
	{
		return var_stats[pos][loc][0];
	}
	inline _VOID SetXrStat(_INT pos, _INT loc, _INT val)
	{
		var_stats[pos][loc][0] = val;
		return;
	}

	inline _VOID IncXrAntiStat(_INT pos, _INT loc)
	{
		var_stats[pos][loc][1] ++;
		return;
	}
	inline _INT  GetXrAntiStat(_INT pos, _INT loc)
	{
		return var_stats[pos][loc][1];
	}
	inline _VOID SetXrAntiStat(_INT pos, _INT loc, _INT val)
	{
		var_stats[pos][loc][1] = val;
		return;
	}
};

class trace_descr_class
{
protected:
	mtrace_type trace[XRINP_SIZE];
	_INT num_cells;

public:
	trace_descr_class(void)
	{
		HWRMemSet(trace, 0, sizeof(trace));
		num_cells = 0;
	}

	_VOID AddTraceCell(_UCHAR sym, _UCHAR nvar, _UCHAR pvoc, _UCHAR pinp)
	{
		trace[num_cells].sym = sym;
		trace[num_cells].nvar = nvar;
		trace[num_cells].pvoc = pvoc;
		trace[num_cells].pinp = pinp;
		num_cells++;
		if (num_cells >= XRINP_SIZE)
		{
			num_cells--;
		}
		return;
	}

	_INT CheckTrace(_UCHAR sym, _UCHAR nvar, _UCHAR pvoc, _UCHAR pinp)
	{
		_INT i;
		_INT ret = 0;

		for (i = 0; i < num_cells; i++)
		{
			if (trace[i].sym == sym  && trace[i].nvar == nvar  &&
			        trace[i].pvoc == pvoc && trace[i].pinp == pinp)
			{
				ret = 1;
				break;
			}
		}

		return ret;
	}
};

_INT  SetVarStat(_INT pos, _INT sign, p_xrdata_type xrdata, p_RWG_type rwg, var_descr_class * (*cvs_table)[256][16], trace_descr_class * mtrace);
_INT  GetVarMeasures(_INT pos, p_xrdata_type xrdata, p_RWG_type rwg, p_rc_type rc);

_INT  PrintExpResults(var_descr_class * (*cvs_table)[256][16]);
_INT  ImportStatResults(var_descr_class * (*cvs_table)[256][16]);

p_rc_type grc;

/* ************************************************************************* */
/* *    XrWord experimental routines                                       * */
/* ************************************************************************* */
_INT xr_exp1(p_xrdata_type xrdata, p_RWG_type rwg, p_rec_w_type prw, rc_type  _PTR rc)
{
	_INT i;
	p_RWS_type     rws;

	if (prw->word[0] == 0)
	{
		goto err;
	}
	//  if (prw->word[0] == 0) HWRStrCpy((_STR)prw->word, "are");

	GetCMPAliases(xrdata, rwg, (_STR) prw->word, rc);

	rws = (p_RWS_type) (rwg->rws_mem);
	if (rws == _NULL)
	{
		goto err;
	}

	for (i = 0; i < rwg->size; i++)
	{
		if (rws[i].type != RWST_SYM)
		{
			continue;
		}

		GetVarMeasures(i, xrdata, rwg, rc);
	}


	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

_INT  SetCHLStat(_INT pos, p_xrdata_type xrdata, p_RWG_type rwg, _INT(*chl_table)[256][16][10], _INT(*chh_table)[256][16][4], p_rc_type);
_INT  PrintCHLResults(_INT(*chl_table)[256][16][10], _INT(*chh_table)[256][16][4]);

/* ************************************************************************* */
/* *    XrWord experimental routines                                       * */
/* ************************************************************************* */
_INT xr_exp2(p_xrdata_type xrdata, p_RWG_type rwg, p_rec_w_type prw, rc_type  _PTR rc)
{
	_INT i, j, nw;
	p_RWS_type     rws;
	rc_type rcl;
	RWG_type rwgl = { 0 };
	_CHAR words[NUM_RW][w_lim] = { { 0 } };
	_CHAR cmp_word[w_lim];
	trace_descr_class _PTR mtrace = _NULL;
	static var_descr_class * (*cvs_table)[256][16] = _NULL;
	static _INT(*chl_table)[256][16][10] = _NULL;
	static _INT(*chh_table)[256][16][4] = _NULL;

	if (rc == _NULL) // Command to put down results
	{
		PrintExpResults(cvs_table);
		PrintCHLResults(chl_table, chh_table);
		goto done;
	}

	if (xrdata->len < 3)
	{
		goto err;
	}

	grc = rc;

	HWRStrCpy(cmp_word, (_STR) prw->word);
	if (cmp_word[0] == 0)
	{
		goto err;
	}
	//  if (cmp_word[0] == '#') goto err;

	if (cvs_table == _NULL)
	{
		cvs_table = (var_descr_class* (*)[256][16])HWRMemoryAlloc(sizeof(*cvs_table));
		HWRMemSet(cvs_table, 0, sizeof(*cvs_table));
		ImportStatResults(cvs_table);
	}

	if (chl_table == _NULL)
	{
		chl_table = (_INT(*)[256][16][10])HWRMemoryAlloc(sizeof(*chl_table));
		HWRMemSet(chl_table, 0, sizeof(*chl_table));
		chh_table = (_INT(*)[256][16][4])HWRMemoryAlloc(sizeof(*chh_table));
		HWRMemSet(chh_table, 0, sizeof(*chh_table));
		//    ImportCHLResults(chl_table);
	}

	if (GetCMPAliases(xrdata, &rwgl, (_STR) prw->word, rc))
	{
		goto err;
	}

	rws = (p_RWS_type) (rwgl.rws_mem);
	if (rws == _NULL)
	{
		goto err;
	}
	if (rwgl.ppd_mem == _NULL)
	{
		goto err;
	}

	mtrace = new trace_descr_class();

	for (i = 0; i < rwgl.size; i++)
	{
		if (rws[i].type != RWST_SYM)
		{
			continue;
		}

		SetVarStat(i, 0, xrdata, &rwgl, cvs_table, mtrace);
		SetCHLStat(i, xrdata, &rwgl, chl_table, chh_table, rc);
	}

	if (rwgl.rws_mem != _NULL)
	{
		HWRMemoryFree(rwgl.rws_mem);
		rwgl.rws_mem = _NULL;
	}
	if (rwgl.ppd_mem != _NULL)
	{
		HWRMemoryFree(rwgl.ppd_mem);
		rwgl.ppd_mem = _NULL;
	}

	rcl = *rc;
	rcl.xrw_mode = XRWM_CS; //  | XRWM_LD;
	rcl.xrw_tag_size = 2;
	rcl.enabled_cs = CS_ALPHA | CS_NUMBER | CS_EPUNCT | CS_OTHER;

	if (xrlv(xrdata, rwg, &rcl))
	{
		goto err;
	}

	rws = (p_RWS_type) (rwg->rws_mem);
	if (rws == _NULL)
	{
		goto err;
	}

	j = 0;
	nw = 0;
	for (i = 0; i < rwg->size; i++)
	{
		if (rws[i].type != RWST_SYM)
		{
			if (j > 0)
			{
				words[nw][j] = 0;
				nw++;
				j = 0;
			}
		}
		else
		{
			words[nw][j++] = rws[i].sym;
		}
	}

	//  if (rwg->rws_mem != _NULL) {HWRMemoryFree(rwg->rws_mem); rwg->rws_mem = _NULL;}
	//  if (rwg->ppd_mem != _NULL) {HWRMemoryFree(rwg->ppd_mem); rwg->ppd_mem = _NULL;}


	for (i = 0; i < nw; i++)
	{
		if (HWRStrCmp(words[i], cmp_word) == 0)
		{
			continue;
		}

		HWRStrCpy((_STR) prw->word, words[i]);

		if (GetCMPAliases(xrdata, &rwgl, (_STR) prw->word, &rcl))
		{
			goto err;
		}

		rws = (p_RWS_type) (rwgl.rws_mem);
		if (rws == _NULL)
		{
			goto err;
		}

		for (j = 0; j < rwgl.size; j++)
		{
			if (rws[j].type != RWST_SYM)
			{
				continue;
			}
			SetVarStat(j, 1, xrdata, &rwgl, cvs_table, mtrace);
		}

		if (rwgl.rws_mem != _NULL)
		{
			HWRMemoryFree(rwgl.rws_mem);
			rwgl.rws_mem = _NULL;
		}
		if (rwgl.ppd_mem != _NULL)
		{
			HWRMemoryFree(rwgl.ppd_mem);
			rwgl.ppd_mem = _NULL;
		}
	}


	if (mtrace)
	{
		delete mtrace;
	}

done:
	return 0;
err:
	if (rwgl.rws_mem != _NULL)
	{
		HWRMemoryFree(rwgl.rws_mem);
		rwgl.rws_mem = _NULL;
	}
	if (rwgl.ppd_mem != _NULL)
	{
		HWRMemoryFree(rwgl.ppd_mem);
		rwgl.ppd_mem = _NULL;
	}
	if (mtrace)
	{
		delete mtrace;
	}
	return 1;
}

/* ************************************************************************* */
/* *    Get variant measurments                                            * */
/* ************************************************************************* */
_INT  SetVarStat(_INT pos, _INT sign, p_xrdata_type xrdata, p_RWG_type rwg, var_descr_class * (*cvs_table)[256][16], trace_descr_class * mtrace)
{
	_INT   i;
	_INT   nvar;
	_UCHAR sym;
	_INT   varlen;
	_INT   loc, val;
	p_RWS_type     rws = _NULL;
	p_RWG_PPD_type ppd = _NULL;
	var_descr_class * pvdc;
	xrd_el_type(_PTR xrd)[XRINP_SIZE] = xrdata->xrd;


	if (cvs_table == _NULL)
	{
		goto err;
	}

	rws = (p_RWS_type) (rwg->rws_mem);
	ppd = (p_RWG_PPD_type) rwg->ppd_mem;
	if (rws == _NULL)
	{
		goto err;
	}
	if (ppd == _NULL)
	{
		goto err;
	}

	sym = rws[pos].realsym;
	nvar = rws[pos].nvar;

	ppd += pos;

	for (varlen = 0; varlen < DTI_XR_SIZE && (*ppd)[varlen].type != 0; varlen++);

	if ((*cvs_table)[sym][nvar] == _NULL)
	{
		(*cvs_table)[sym][nvar] = new var_descr_class();
		if ((*cvs_table)[sym][nvar] == _NULL)
		{
			goto err;
		}
	}

	pvdc = (*cvs_table)[sym][nvar];

	for (i = 0; i < varlen; i++)
	{
		if ((*ppd)[i].type == XRWG_ALST_CR) // if there was corelation
		{
			loc = (*ppd)[i].alias;
			switch (XEM_CUR)
			{
				case XEM_XR:
					val = (*xrd)[loc].xr.type;
					break;
				case XEM_H:
					val = (*xrd)[loc].xr.height;
					break;
				case XEM_SH:
					val = (*xrd)[loc].xr.shift;
					break;
				case XEM_SZ:
					val = (*xrd)[loc].xr.depth;
					break;
				case XEM_OR:
					val = (*xrd)[loc].xr.orient;
					break;
			}

			if (sign == 0)
			{
				pvdc->IncXrStat(i, val);
				mtrace->AddTraceCell((_UCHAR) sym, (_UCHAR) nvar, (_UCHAR) i, (_UCHAR) loc);
			}
			else
			{
				if (mtrace->CheckTrace((_UCHAR) sym, (_UCHAR) nvar, (_UCHAR) i, (_UCHAR) loc) == 0)
				{
					pvdc->IncXrAntiStat(i, val);
				}
			}
		}
	}

	return 0;
err:
	return 1;
}
/* ************************************************************************* */
/* *    Print results of up-to-now run                                     * */
/* ************************************************************************* */
_INT  PrintExpResults(var_descr_class * (*cvs_table)[256][16])
{
	_INT i, j, k, m;
	_INT v;
	var_descr_class * pvdc;
	FILE * file;

	if (cvs_table == _NULL)
	{
		goto err;
	}

	if ((file = fopen("c:\\HWR\\results.stt", "wt")) == _NULL)
	{
		goto err;
	}

	fprintf(file, "WritePad Stat file v.3.10\n");
	fprintf(file, "%c\n\n", XE_TYPE);

	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 16; j++)
		{
			if ((*cvs_table)[i][j] != _NULL)
			{
				fprintf(file, "%c\x09%d\n", (_CHAR) i, j);
				for (k = 0; k < DTI_XR_SIZE; k++)
				{
					for (m = 0; m < XE_BSIZE; m++)
					{
						pvdc = (*cvs_table)[i][j];
						v = pvdc->GetXrStat(k, m);
						fprintf(file, "\x09%d", v);
					}
					fprintf(file, "\n");
					for (m = 0; m < XE_BSIZE; m++)
					{
						pvdc = (*cvs_table)[i][j];
						v = pvdc->GetXrAntiStat(k, m);
						fprintf(file, "\x09%d", v);
					}
					fprintf(file, "\n");
				}
				fprintf(file, "\n");
			}
		}
	}

	fclose(file);

	return 0;
err:
	return 1;
}
/* ************************************************************************* */
/* *    Print results of up-to-now run                                     * */
/* ************************************************************************* */
_INT  ImportStatResults(var_descr_class * (*cvs_table)[256][16])
{
	_INT   i, j;
	_INT   state = 0;
	_INT   nvar, nxr, len;
	_INT   set[XE_BSIZE];
	_UCHAR sym;
	_CHAR  str[1024];
	FILE *file = _NULL;
	var_descr_class * pvdc;

	if (cvs_table == _NULL)
	{
		goto err;
	}

	if ((file = fopen("c:\\HWR\\results.stt", "rt")) == _NULL)
	{
		goto done;
	}

	fgets(str, 1024, file);
	if (HWRStrnCmp(str, "WritePad Stat file v.3.10", 20) != _NULL)
	{
		goto err;
	}

	fgets(str, 1024, file);
	if (str[0] != XE_TYPE)
	{
		goto err;
	}

	printw("\nImporting prev statistics ...");

	while (!feof(file))
	{
		fgets(str, 1024, file);

		if ((len = HWRStrLen(str)) < 3)
		{
			state = 1;    // Start of new proto
			continue;
		}

		if (state == 1) // read header of proto
		{
			sscanf(str, "%c %d", &sym, &nvar);
			if (nvar < 0 || nvar > 15)
			{
				goto err;
			}
			state = 2;
			nxr = 0;
			continue;
		}

		if (state == 2) // reading records of the proto
		{
			for (i = 0, j = 0; i < XE_BSIZE; i++)
			{
				for (j; j < 1024; j++) if (IsDigit(str[j]))
					{
						break;
					}
				if (j >= len)
				{
					goto err;
				}

				sscanf(&str[j], "%d", &set[i]);

				for (j; j < 1024; j++) if (!IsDigit(str[j]))
					{
						break;
					}
				if (j >= len)
				{
					goto err;
				}
			}

			if ((*cvs_table)[sym][nvar] == _NULL)
			{
				(*cvs_table)[sym][nvar] = new var_descr_class();
				if ((*cvs_table)[sym][nvar] == _NULL)
				{
					goto err;
				}
			}

			for (i = 0; i < XE_BSIZE; i++)
			{
				pvdc = (*cvs_table)[sym][nvar];
				pvdc->SetXrStat(nxr, i, set[i]);
			}

			//      nxr ++;
			//      if (nxr > 12) goto err;

			state = 3;
			continue;
		}

		if (state == 3) // reading records of the proto
		{
			for (i = 0, j = 0; i < XE_BSIZE; i++)
			{
				for (j; j < 1024; j++) if (IsDigit(str[j]))
					{
						break;
					}
				if (j >= len)
				{
					goto err;
				}

				sscanf(&str[j], "%d", &set[i]);

				for (j; j < 1024; j++) if (!IsDigit(str[j]))
					{
						break;
					}
				if (j >= len)
				{
					goto err;
				}
			}

			if ((*cvs_table)[sym][nvar] == _NULL)
			{
				(*cvs_table)[sym][nvar] = new var_descr_class();
				if ((*cvs_table)[sym][nvar] == _NULL)
				{
					goto err;
				}
			}

			for (i = 0; i < XE_BSIZE; i++)
			{
				pvdc = (*cvs_table)[sym][nvar];
				pvdc->SetXrAntiStat(nxr, i, set[i]);
			}

			nxr++;
			if (nxr > 12)
			{
				goto err;
			}

			state = 2;
			continue;
		}
	}

	if (file)
	{
		fclose(file);
	}

	printw("\nImported.\n");

done:
	return 0;
err:
	if (file)
	{
		fclose(file);
		err_msg("!Stat results were not read properly!!!\n");
	}

	return 1;
}
/* ************************************************************************* */
/* *    Get variant measurments                                            * */
/* ************************************************************************* */
_INT  GetVarMeasures(_INT pos, p_xrdata_type xrdata, p_RWG_type rwg, p_rc_type rc)
{
	_INT   i;
	_INT   num;
	_INT   nvar;
	_UCHAR sym;
	_CHAR  word[w_lim];
	loc_type xl[DTI_XR_SIZE] = { { 0 } };
	p_RWS_type     rws = _NULL;
	p_RWG_PPD_type ppd = _NULL;
	FILE * file;
	xrd_el_type(_PTR xrd)[XRINP_SIZE] = xrdata->xrd;

	(void) rc;

	rws = (p_RWS_type) (rwg->rws_mem);
	ppd = (p_RWG_PPD_type) rwg->ppd_mem;
	if (rws == _NULL)
	{
		goto err;
	}
	if (ppd == _NULL)
	{
		goto err;
	}

	sym = rws[pos].sym;
	nvar = rws[pos].nvar;

	//  if (sym  != 'a') goto err;
	//  if (nvar != 3)   goto err;
	//  if (sym  != 'd') goto err;
	//  if (nvar != 0 && nvar != 1) goto err;
	if (sym != 'u')
	{
		goto err;
	}
	if (nvar > 3)
	{
		goto err;
	}

	ppd += pos;

	for (i = 0; i < DTI_XR_SIZE && (*ppd)[i].type != 0; i++)
	{
		num = (*ppd)[i].alias;
		xl[i].x = ((*xrd)[num].box_left + (*xrd)[num].box_right) / 2;
		xl[i].y = ((*xrd)[num].box_up + (*xrd)[num].box_down) / 2;
	}

	/* --------------- Write results to file --------------------------- */

	if ((file = fopen("c:\\HWR\\let_pos.xlh", "a+t")) == _NULL)
	{
		goto err;
	}

	for (i = 0; i < rwg->size && i < w_lim; i++)
	{
		word[i] = rws[i].sym;
	}
	word[i] = 0;

	fprintf(file, "%s\n", word);
	for (i = 0; i < DTI_XR_SIZE; i++)
	{
		fprintf(file, "%d\x09", xl[i].x);
	}
	fprintf(file, "\n");
	for (i = 0; i < DTI_XR_SIZE; i++)
	{
		fprintf(file, "%d\x09", xl[i].y);
	}
	fprintf(file, "\n");

	fclose(file);

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/* *    Set chl stat                                                       * */
/* ************************************************************************* */
_INT  SetCHLStat(_INT pos, p_xrdata_type xrdata, p_RWG_type rwg, _INT(*chl_table)[256][16][10], _INT(*chh_table)[256][16][4], p_rc_type rc)
{
	_INT          i, k;
	_INT          nvar;
	_UCHAR        sym;
	_INT          txb, txe, tmb, tme;
	_INT          base;
	_INT          st, end;
	_INT          set, setp;
	_INT          med;
	p_RWS_type    rws = _NULL;
	p_RWS_type    rwe;
	xrd_el_type(_PTR xrd)[XRINP_SIZE] = xrdata->xrd;
	p_xrd_el_type xel;
	_RECT         rect, rectp;
	static _INT   hh_coder[4] = { 50, 130, 250, 32000 };


	if (chl_table == _NULL)
	{
		goto err;
	}
	if (chh_table == _NULL)
	{
		goto err;
	}

	rws = (p_RWS_type) (rwg->rws_mem);
	if (rws == _NULL)
	{
		goto err;
	}
	rwe = &rws[pos];
	if (rwe->xrd_len < 2)
	{
		goto err;
	}

	sym = rwe->realsym;
	nvar = rwe->nvar;

	med = rc->stroka.size_out;
	txb = rc->stroka.box.left;   // Get medium base line coords
	txe = rc->stroka.box.right;
	tmb = rc->stroka.box.top + ((rc->stroka.box.bottom - rc->stroka.box.top)*
	                            ((rc->curv_bord[2] + rc->curv_bord[3]) / 2)) / 256;
	tme = rc->stroka.box.top + ((rc->stroka.box.bottom - rc->stroka.box.top)*
	                            ((rc->curv_bord[CB_NUM_VERTEX - 4] + rc->curv_bord[CB_NUM_VERTEX - 3]) / 2)) / 256;

	st = rwe->xrd_beg;
	end = rwe->xrd_beg + rwe->xrd_len;
	xel = &((*xrd)[st]);
	rect.top = rectp.top = 32000;
	rect.bottom = rectp.bottom = 0;
	rect.left = rectp.left = 32000;
	rect.right = rectp.right = 0;
	set = setp = 0;
	for (i = st; i < end; i++, xel++)
	{
		if (IsXrLink(xel))
		{
			continue;
		}
		if (GetXrMovable(xel))
		{
			if (rectp.top    > xel->box_up)
			{
				rectp.top = xel->box_up;
			}
			if (rectp.bottom < xel->box_down)
			{
				rectp.bottom = xel->box_down;
			}
			if (rectp.right < xel->box_right)
			{
				rectp.right = xel->box_right;
			}
			if (rectp.left > xel->box_left)
			{
				rectp.left = xel->box_left;
			}
			setp++;
		}
		else
		{
			if (rect.top > xel->box_up)
			{
				rect.top = xel->box_up;
			}
			if (rect.bottom < xel->box_down)
			{
				rect.bottom = xel->box_down;
			}
			if (rect.right < xel->box_right)
			{
				rect.right = xel->box_right;
			}
			if (rect.left > xel->box_left)
			{
				rect.left = xel->box_left;
			}
			set++;
		}
	}

	if (set < 2) // Less than 2 normal els in letter
	{
		if (!setp) // Nothing at alll???
		{
			rect.top = rect.bottom = (_SHORT) tmb;
			rect.left = rect.right = (_SHORT) txb;
		}
		else
		{
			if (set)
			{
				rect.top = HWRMin(rect.top, rectp.top);
				rect.bottom = HWRMax(rect.bottom, rectp.bottom);
				rect.left = HWRMin(rect.left, rectp.left);
				rect.right = HWRMax(rect.right, rectp.right);
			}
			else
			{
				rect = rectp;
			}
		}
	}


	k = (rect.left + rect.right) / 2;
	if (k <= 0)
	{
		goto err;    // Get position of baseline med in sym location
	}
	k = k - txb;
	if (k < 0)
	{
		goto err;
	}
	k = 1000 * k / (txe - txb);
	k = (k*(tme - tmb)) / 1000;
	base = tmb + k;


	if (base > rect.top && base < rect.bottom)
	{
		k = rect.bottom - rect.top;
		k = 10 * (base - rect.top) / k;
		(*chl_table)[sym][nvar][k] ++;
	}
	else
	{
		if (base <= rect.top)
		{
			(*chl_table)[sym][nvar][0] ++;
		}
		if (base >= rect.bottom)
		{
			(*chl_table)[sym][nvar][9] ++;
		}
	}

	k = (100 * (rect.bottom - rect.top)) / med; // Count box height info
	for (i = 0; i < 4; i++) if (k < hh_coder[i])
		{
			break;
		}
	if (i > 3)
	{
		i = 3;
	}
	(*chh_table)[sym][nvar][i] ++;


	return 0;
err:
	return 1;
}
/* ************************************************************************* */
/* *    Print results of up-to-now run                                     * */
/* ************************************************************************* */
_INT  PrintCHLResults(_INT(*chl_table)[256][16][10], _INT(*chh_table)[256][16][4])
{
	_INT i, j, k, m;
	FILE * file;

	if (chl_table == _NULL)
	{
		goto err;
	}

	if ((file = fopen("c:\\HWR\\chl_res.stt", "wt")) == _NULL)
	{
		goto err;
	}

	fprintf(file, "HWR CHL Stat file v.3.20\n");
	//  fprintf(file, "%c\n\n", XE_TYPE);
	fprintf(file, "\n\n");

	for (i = 0; i < 256; i++)
	{
		for (j = 0; j < 16; j++)
		{
			for (k = m = 0; k < 10; k++) if ((*chl_table)[i][j][k] > 0)
				{
					m++;
					break;
				}

			if (m)
			{
				fprintf(file, "%c\x09%d", (_CHAR) i, j);
				for (k = 0; k < 10; k++)
				{
					fprintf(file, "\x09%d", (*chl_table)[i][j][k]);
				}
				fprintf(file, "\x09");
				for (k = 0; k < 4; k++)
				{
					fprintf(file, "\x09%d", (*chh_table)[i][j][k]);
				}
				fprintf(file, "\n");
			}
		}
	}

	fclose(file);

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

#define XRE_NUMSYM  40

typedef struct
{
	_UCHAR st;
	_UCHAR w;
	_UCHAR p;
	_UCHAR nv;
} slw_type, _PTR p_slw_type;

_INT GetSymLW(_UCHAR sym, p_xrdata_type xrdata, rc_type _PTR rc, slw_type(_PTR slw)[XRINP_SIZE]);

/* ************************************************************************* */
/* *    Create table of letter weights for current xrdata                  * */
/* ************************************************************************* */
_INT xr_exp3(p_xrdata_type xrdata, p_RWG_type rwg, p_rec_w_type prw, rc_type  _PTR rc)
{
	_INT i;
	_INT j;
	xrd_el_type(_PTR xrd)[XRINP_SIZE] = xrdata->xrd;
	slw_type(*slw)[XRE_NUMSYM][XRINP_SIZE] = _NULL;
	_UCHAR symset[XRE_NUMSYM] = { 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j',
	                              'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't',
	                              'u', 'v', 'w', 'x', 'y', 'z',
	                              '0', '1', '2', '3', '4', '5', '6', '7', '8', '9'
	                            };

	if (xrdata == _NULL || xrdata->len < 3 || rc == _NULL)
	{
		goto err;
	}


	slw = (slw_type(*)[XRE_NUMSYM][XRINP_SIZE])HWRMemoryAlloc(sizeof(*slw));
	if (slw == _NULL)
	{
		goto err;
	}
	HWRMemSet(slw, 0, sizeof(*slw));

	printw("\n  ");
	for (i = 0; i < xrdata->len; i++)
	{
		put_xr((*xrd)[i].xr, 4);
		printw("  ");
	}

	for (i = 0; symset[i] != 0 && i < XRE_NUMSYM; i++)
	{
		if (GetSymLW(symset[i], xrdata, rc, &((*slw)[i])))
		{
			goto err;
		}

		printw("\n%c  ", symset[i]);
		for (j = 1; j < xrdata->len; j++)
		{
			printw("%3d", (*slw)[i][j].p);
		}

	}

	if (slw)
	{
		HWRMemoryFree(slw);
	}
	return 0;
err:
	if (slw)
	{
		HWRMemoryFree(slw);
	}
	return 1;
}

/* ************************************************************************* */
/* *    XrWord experimental routines                                       * */
/* ************************************************************************* */
_INT GetSymLW(_UCHAR sym, p_xrdata_type xrdata, rc_type _PTR rc, slw_type(_PTR slw)[XRINP_SIZE])
{
	_INT i;
	_INT base_w, base_len, p, w;
	_INT xrinp_len;
	_SHORT st_line[XRINP_SIZE];
	p_xrcm_type xrcm = _NULL;
	p_let_hdr_type plh;
	p_letlayout_hdr_type  plsym;
	p_tr_pos_type         trp;
	//  xrd_el_type (_PTR xrd)[XRINP_SIZE] = xrdata->xrd;

	xrinp_len = xrdata->len;

	if (xrinp_len < 3)
	{
		goto err;
	}
	if (xrinp_len > XRINP_SIZE - 5)
	{
		goto err;
	}

	/* ------ Allocate memory & init structures ------------------------------ */

	if (xrmatr_alloc(rc, xrdata, &xrcm) != 0)
	{
		goto err;
	}

	xrcm->word[0] = sym;
	xrcm->word[1] = 0;

	xrcm->flags |= XRMC_DOTRACING;
	xrcm->cmode = XCM_AL_DEFSIZE;
	xrcm->src_pos = xrcm->wwc_pos;
	xrcm->wwc_pos = 0;
	xrcm->let = sym;

	if (TraceAlloc(0, xrcm))
	{
		goto err;
	}

	plh = (p_let_hdr_type) (TDwordAdvance(sizeof(*plh), xrcm));

	if (plh == _NULL)
	{
		goto err;
	}
	HWRMemSet(plh, 0, sizeof(*plh));

	xrcm->p_htrace->lhdrs[0] = plh;
	xrcm->let_htr = plh;
	xrcm->cur_let_num = 0;

	for (i = 0; i < xrinp_len; i++)
	{
		st_line[i] = XRMC_CONST_CORR;
	}
	SetInpLine(st_line, 0, xrinp_len - 1, xrcm);

	if (CountLetter(xrcm))
	{
		goto err;
	}


	for (i = 1; i < xrinp_len; i++)
	{
		xrcm->v_end = i + 1;

		if (CreateLayout(xrcm))
		{
			goto err;
		}

		plsym = xrcm->p_hlayout->llhs[0];
		trp = &(plsym->trp[0]);

		p = w = 0;
		base_len = i - trp->inp_pos;
		base_len += (trp->vect == XRMC_T_CSTEP) ? 1 : 0;
		//    base_len -= (plsym->trp[plsym->len-1].vect != XRMC_T_CSTEP) ? 1:0;
		base_w = base_len * XRMC_DEF_CORR_VALUE;
		if (base_w > 0)
		{
			w = (*xrcm->s_out_line)[i] - XRMC_CONST_CORR;
			if (w < 0)
			{
				w = 0;
			}
			p = (100 * w) / base_w;
		}

		(*slw)[i].st = (_UCHAR) trp->inp_pos;
		(*slw)[i].w = (_UCHAR) w;
		(*slw)[i].p = (_UCHAR) p;
		(*slw)[i].nv = (_UCHAR) plsym->var_num;

		FreeLayout(xrcm);
	}

	TraceDealloc(xrcm);
	FreeLayout(xrcm);
	xrmatr_dealloc(&xrcm);
	return 0;
err:
	TraceDealloc(xrcm);
	FreeLayout(xrcm);
	xrmatr_dealloc(&xrcm);
	return 1;
}

/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */

#define ABS(x) (((x) >= 0) ? (x) : (-(x)))

class cmpwords_descr_class
{
protected:
	_INT   nwg;
	_UCHAR wg[WS_MAX_WORDS];

public:
	cmpwords_descr_class(p_ws_data_type pws_data)
	{
		_INT i, j;
		_INT max;
		_INT ns;
		_INT size;
		register p_ws_data_type pwsd = pws_data;

		nwg = 0;
		HWRMemSet(wg, 0, sizeof(wg));

		ns = 0;
		if (pwsd->cmp)
		{
			for (i = 0; (*pwsd->cmp)[i] != 0 && i < WS_MAX_WORDS; i++) // CMP Word step cycle
			{
				max = 0;
				size = 0;

				if (ns >= pwsd->global_cur_stroke)
				{
					break;
				}

				for (j = 0; j < (*pwsd->cmp)[i]; j++) // CMP word strokes cycle
				{
					if (ns >= pwsd->line_st_stroke)
					{
						if (max < pwsd->xstrokes[ns - pwsd->line_st_stroke].end)
						{
							max = pwsd->xstrokes[ns - pwsd->line_st_stroke].end;
						}
					}

					ns++;
				}

				if (max > 0)  // Got into line word
				{
					_INT ldist, rdist, dist, best_dist, best_num;

					best_dist = 32000;
					best_num = 0;
					for (j = 1; j < pwsd->line_ngaps; j++)
					{
						ldist = ABS(max - (*pwsd->gaps)[j].lst);
						rdist = ABS(max - ((*pwsd->gaps)[j].lst + (*pwsd->gaps)[j].low));
						dist = (ldist < rdist) ? ldist : rdist;

						if (dist < best_dist)
						{
							best_dist = dist;
							best_num = j;
						}
					}

					if (best_num)
					{
						wg[nwg] = (_UCHAR) best_num;
						nwg++;
					}
				}
			}
		}
	}

	_INT CheckCMPGap(_INT ngap, p_ws_data_type pwsd)
	{
		_INT i;
		_INT found = 0;

		for (i = 0; i < nwg; i++)
		{
			if (wg[i] == ngap)
			{
				found = 1;
				break;
			}
		}

		return found;
	}
};

#define XE_MAX_GAPS 50    // Max number of gaps per line

typedef struct
{
	_SHORT size;
	_SHORT blank;
	_SHORT low;
	_SHORT flag;
} xe_gaps_type, _PTR p_xe_gaps_type;

/* ************************************************************************* */
/* *    NN suport programs for WS                                          * */
/* ************************************************************************* */
_INT xrexp_nn(p_VOID pws_data)
{
	_INT i, fl;
	_INT in_max;
	_INT ou_min;
	_INT size;
	_INT xegs = 0;
	register p_ws_data_type pwsd = (p_ws_data_type) pws_data;
	cmpwords_descr_class _PTR cmp_descr;
	static FILE * nn_stt = _NULL;
	static _INT num = 1;
	xe_gaps_type xeg[XE_MAX_GAPS] = { 0 };

	if (pwsd->cmp == _NULL)
	{
		goto err;
	}
	if ((*pwsd->cmp)[0] == 0)
	{
		goto err;
	}

	cmp_descr = new cmpwords_descr_class(pwsd);

	in_max = 0;
	ou_min = 32000;
	for (i = 1; i < pwsd->line_ngaps - 1; i++)
	{
		size = (100 * (*pwsd->gaps)[i].size) / (pwsd->ws_inline_dist);

		if ((fl = cmp_descr->CheckCMPGap(i, pwsd)) == 0) // Betw word gap?
		{
			if (in_max < size && size < 250)
			{
				in_max = size;
			}
		}
		else
		{
			if (ou_min > size && size > 0)
			{
				ou_min = size;
			}
		}

		if (i <= XE_MAX_GAPS)
		{
			xeg[i - 1].size = (_SHORT) ((100 * (*pwsd->gaps)[i].psize) / (pwsd->ws_inline_dist));
			xeg[i - 1].blank = (_SHORT) ((100 * (*pwsd->gaps)[i].blank) / (pwsd->ws_inline_dist));
			xeg[i - 1].low = (_SHORT) ((100 * (*pwsd->gaps)[i].low) / (pwsd->ws_inline_dist));
			xeg[i - 1].flag = (_SHORT) (fl);
			xegs++;
		}
	}

	delete cmp_descr;

	if (pwsd->cmp)
	{
		pwsd->nn_cmp_min = in_max;
		pwsd->nn_cmp_max = ou_min;
	}
	else
	{
		pwsd->nn_cmp_min = pwsd->nn_cmp_max = 0;
	}

	if (nn_stt == _NULL)
	{
		nn_stt = fopen("c:\\HWR\\nn.stt", "a+t");
		if (nn_stt)
		{
			fprintf(nn_stt, "NN stat file v.1.00\n\n");
			fprintf(nn_stt, ";   > SSP NSSP BSP NBSP SL INWD NPIKS CMPMIN CMPMAX\n");
			fprintf(nn_stt, ";   * Num Size Blank Low ToBeSegmented(0/1)\n\n");
		}
	}

	if (nn_stt)
	{
		if (pwsd->global_cur_line == 1)
		{
			fprintf(nn_stt, "%d\n", (num++));
		}
		fprintf(nn_stt, ">\x09%d\x09%d\x09%d\x09%d\x09%d\x09%d\x09%d\x09%d\x09%d\n",
		        pwsd->nn_ssp, pwsd->nn_n_ssp, pwsd->nn_bsp, pwsd->nn_n_bsp, pwsd->nn_sl,
		        pwsd->nn_inw_dist, pwsd->nn_npiks, pwsd->nn_cmp_min, pwsd->nn_cmp_max);

		for (i = 0; i < xegs; i++)
		{
			fprintf(nn_stt, "*\x09%d\x09%d\x09%d\x09%d\x09%d\n",
			        i, (_INT) xeg[i].size, (_INT) xeg[i].blank, (_INT) xeg[i].low, (_INT) xeg[i].flag);
		}
	}

	return 0;
err:
	return 1;
}




/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* ************************************************************************* */
/* *   xr_exp_snn  ********************************************************* */
/* ************************************************************************* */
_INT g_snn_first_save = 1;
_INT xe_g_num_word = 0;

#define SNN_EXP_NCELLS w_lim

typedef struct
{
	_UCHAR sym;
	_UCHAR nvar;
	_UCHAR st;
	_UCHAR len;
	_SCHAR w;
	_UCHAR coeff[MLP_NET_NUMINPUTS];
} snn_pos_data_type, _PTR p_snn_pos_data_type;

typedef snn_pos_data_type snn_pos_data_array_type[XRLV_VARNUM];
typedef snn_pos_data_array_type _PTR p_snn_pos_data_array_type;

class snn_word_data_class
{
protected:
	_INT num_pos;
	_INT num_cells;
	_INT initialized_ok;
	p_snn_pos_data_array_type pos_ptrs[XRINP_SIZE];

public:
	snn_word_data_class(_VOID);
	~snn_word_data_class(_VOID);

	_INT   snn_CreateStorage(_INT np);
	_INT   snn_FillPosData(_INT pos, _INT st, _INT len, p_UCHAR coeff);
	p_RWS_type snn_CheckCorrect(_INT st, _INT len, p_RWG_type rwg);
	_INT   snn_SaveWordResults(_STR fname);
};

snn_word_data_class * g_snn_word_data = _NULL;
RWG_type rwgl;

/* ************************************************************************* */
/* *    Gather information for Sym NN                                      * */
/* ************************************************************************* */
_INT xr_exp_snn(p_xrdata_type xrdata, p_RWG_type rwg, p_rec_w_type prw, rc_type  _PTR rc)
{
	_INT   i, w, wp, self_weight;
	p_RWS_type     rws;
	_CHAR  cmp_word[w_lim];
	_UCHAR coeff[PC_NUM_COEFF + GBM_NCOEFF];

	if (rc == _NULL) // Command to put down results
	{
		goto done;
	}

	if (xrdata->len < 3)
	{
		goto err;
	}

	g_snn_word_data = new snn_word_data_class;

	if (!g_snn_word_data)
	{
		goto err;
	}

	// ---------------- Get CMP aliases ----------------------------------------

	HWRMemSet(&rwgl, 0, sizeof(rwgl));
	HWRStrCpy(cmp_word, (_STR) prw->word);
	if (cmp_word[0] == 0)
	{
		goto err;
	}

	if (GetCMPAliases(xrdata, &rwgl, (_STR) prw->word, rc))
	{
		goto err;
	}

	rws = (p_RWS_type) (rwgl.rws_mem);
	if (rws == _NULL)
	{
		goto err;
	}
	if (rwgl.ppd_mem == _NULL)
	{
		goto err;
	}

	self_weight = (xrdata->len - 1)*XRMC_DEF_CORR_VALUE;

	for (i = w = 0; i < rwgl.size; i++)
	{
		if (rws[i].type != RWST_SYM)
		{
			continue;
		}
		w += rws[i].letw;
	}

	wp = (100 * w) / self_weight;

	if (wp < 40)
	{
		goto err;    // Do not consider too bad layouts
	}

	// ------------- Call XRLV and gather data -------------------------------------

	if (xrlv(xrdata, rwg, rc))
	{
		goto err;
	}

	// ------------- Store correct layout information ------------------------------

	for (i = 0; i < rwgl.size; i++)
	{
		if (rws[i].type != RWST_SYM)
		{
			continue;
		}
		if (GetPolyCo(rws[i].xrd_beg, rws[i].xrd_len, xrdata, rc->trace, coeff))
		{
			continue;
		}
		g_snn_word_data->snn_FillPosData(-1, rws[i].xrd_beg, rws[i].xrd_len, coeff);
	}

	// -------------- Prepare to exit ------------------------------------------

	if (g_snn_word_data)
	{
		g_snn_word_data->snn_SaveWordResults("c:\\HWR\\snn.stn");
	}

	FreeRWGMem(&rwgl);

done:
	if (g_snn_word_data)
	{
		delete g_snn_word_data;
		g_snn_word_data = _NULL;
	}
	return 0;
err:
	FreeRWGMem(&rwgl);
	if (g_snn_word_data)
	{
		delete g_snn_word_data;
		g_snn_word_data = _NULL;
	}
	return 1;
}

/* ************************************************************************* */
/* *    snn_word_data_class: Constructor                                   * */
/* ************************************************************************* */
snn_word_data_class::snn_word_data_class(_VOID)
{
	num_pos = 0;
	num_cells = 0;
	initialized_ok = 0;

	HWRMemSet(pos_ptrs, 0, sizeof(pos_ptrs));
}

/* ************************************************************************* */
/* *    snn_word_data_class: Destructor                                    * */
/* ************************************************************************* */
snn_word_data_class::~snn_word_data_class(_VOID)
{
	_INT i;

	initialized_ok = 0;

	for (i = 0; i < num_pos && i < XRINP_SIZE; i++)
	{
		if (pos_ptrs[i])
		{
			HWRMemoryFree(pos_ptrs[i]);
		}
	}
}

/* ************************************************************************* */
/* *    snn_word_data_class: Create data structures                        * */
/* ************************************************************************* */
_INT snn_word_data_class::snn_CreateStorage(_INT np)
{
	_INT i;
	_INT err = 0;

	num_pos = np;
	num_cells = SNN_EXP_NCELLS;

	for (i = 0; i < num_pos && i < XRINP_SIZE; i++)
	{
		pos_ptrs[i] = (p_snn_pos_data_array_type) HWRMemoryAlloc(num_cells*sizeof(snn_pos_data_type));
		if (pos_ptrs[i] == _NULL)
		{
			err++;
			break;
		}
		HWRMemSet(pos_ptrs[i], 0, num_cells*sizeof(snn_pos_data_type));
	}

	if (!err)
	{
		initialized_ok = 1;
	}

	return 0;
}

_INT SnnCreateStorage(_INT np)
{
	if (g_snn_word_data)
	{
		return g_snn_word_data->snn_CreateStorage(np);
	}
	else
	{
		return 1;
	}
}

/* ************************************************************************* */
/* *    snn_word_data_class: Fill in position data                         * */
/* ************************************************************************* */
_INT snn_word_data_class::snn_FillPosData(_INT pos, _INT st, _INT len, p_UCHAR coeff)
{
	_INT j, w;
	_UCHAR sym;
	_INT  weight, nvar;
	p_snn_pos_data_array_type snpa;
	p_snn_pos_data_type       tsnp;
	p_RWS_type                prws;

	//  p_UCHAR                   po = xd->order;

	if (!initialized_ok)
	{
		goto err;
	}


	prws = g_snn_word_data->snn_CheckCorrect(st, len, &rwgl);
	if (prws == 0)
	{
		if (rand() < RAND_MAX / 20)  // From time to time write down nonsence
		{
			sym = '`';
			nvar = 0;
			weight = 0;
		}
		else
		{
			goto err;
		}
	}
	else
	{
		if (pos < 0) // Register only direct counted correct letters
		{
			sym = prws->sym;
			nvar = prws->nvar;
			weight = prws->letw;
		}
		else
		{
			goto err;
		}
	}

	if (pos < 0)
	{
		pos = 0;
	}

	snpa = pos_ptrs[pos];

	for (j = 0, tsnp = &(*snpa)[0]; j < SNN_EXP_NCELLS && tsnp->sym != 0; j++, tsnp++);

	if (j < SNN_EXP_NCELLS)
	{
		tsnp->sym = sym;
		tsnp->nvar = (_UCHAR) nvar;
		tsnp->st = (_UCHAR) st;
		tsnp->len = (_UCHAR) len;
		w = weight;
		if (w < -9)
		{
			w = -9;
		}
		if (w > 99)
		{
			w = 99;
		}
		tsnp->w = (_SCHAR) w;
		HWRMemCpy(tsnp->coeff, coeff, sizeof(tsnp->coeff));
	}
	else
	{
		pos_ptrs[pos] = snpa;  // Just for debug breakpoints ....
	}

	return 0;
err:
	return 1;
}

_INT SnnFillPosData(_INT pos, _INT st, _INT len, p_UCHAR coeff)
{
	if (g_snn_word_data)
	{
		return g_snn_word_data->snn_FillPosData(pos, st, len, coeff);
	}
	else
	{
		return 1;
	}
}

/* ************************************************************************* */
/* *    snn_word_data_class: Check which of gathered variants are correct  * */
/* ************************************************************************* */
p_RWS_type snn_word_data_class::snn_CheckCorrect(_INT st, _INT len, p_RWG_type rwg)
{
	_INT i;
	//  _UCHAR correct = 0;
	//  p_snn_pos_data_array_type snpa;
	//  p_snn_pos_data_type       snp;
	p_RWS_type rws, trws, rrws = 0;


	if (!initialized_ok)
	{
		goto err;
	}

	rws = (p_RWS_type) (rwg->rws_mem);
	if (rws == _NULL)
	{
		goto err;
	}

	for (i = 0, trws = rws; i < rwg->size; i++, trws++)
	{
		if (trws->type != RWST_SYM)
		{
			continue;
		}

		if (trws->xrd_beg == st && trws->xrd_len == len)
		{
			rrws = trws;
			break;
		}
	}

	return rrws;
err:
	return 0;
}

/* ************************************************************************* */
/* *    snn_word_data_class: SaveWordData                                  * */
/* ************************************************************************* */
int snn_word_data_class::snn_SaveWordResults(_STR fname)
{
	_INT j;
	_INT pos, nc, v;
	//  _INT found, dif;
	FILE * file;
	p_snn_pos_data_array_type snpa;
	p_snn_pos_data_type       snp;//, tsnp;
	extern char RealTapFileName[256];
	extern p_rec_info_type prig;



	if (!initialized_ok)
	{
		goto err;
	}

	if (g_snn_first_save)
	{
		if ((file = fopen(fname, "wt")) == _NULL)
		{
			goto err;
		}

		fprintf(file, "SNN data file Ver. 1.7. \n   WritePad 2014\n\n");
		g_snn_first_save = 0;
	}
	else
	{
		if ((file = fopen(fname, "at")) == _NULL)
		{
			goto err;
		}
	}

	fprintf(file, "\n> %s %d %s", RealTapFileName, xe_g_num_word++, prig->cmp_word);

	for (pos = 0; pos < num_pos; pos++)
	{
		snpa = pos_ptrs[pos];
		snp = &(*snpa)[0];

		for (nc = 0; nc < num_cells && snp->sym != 0; nc++, snp++)
		{
			fprintf(file, "\n*");
			for (j = 0; j < PC_NUM_COEFF; j++)
			{
				fprintf(file, " %d", (_INT) snp->coeff[j]);
			}
			for (j = 0; j < GBM_NCOEFF; j++)
			{
				if (j % 16 == 0)
				{
					v = 0;
				}
				v |= ((_ULONG) (snp->coeff[j + PC_NUM_COEFF] >> 6) << ((j % 16) * 2));
				if (j % 16 == 15)
				{
					fprintf(file, " %lu", v);
				}
			}
			if ((j - 1) % 16 != 15)
			{
				fprintf(file, "%lu", v);
			}

			fprintf(file, "\n");
			fprintf(file, "%c%x%1d%02d ", snp->sym, snp->nvar, ((snp->sym == '`') ? 0 : 1), snp->w);
		}
	}

	fprintf(file, "\n");

	fclose(file);

	return 0;
err:
	return 1;
}

/* ************************************************************************* */
/* ************************************************************************* */

#endif // PG_DEBUG

/* ************************************************************************* */
/* *    END OF ALLLLLLLLLLLLL                                              * */
/* ************************************************************************* */

