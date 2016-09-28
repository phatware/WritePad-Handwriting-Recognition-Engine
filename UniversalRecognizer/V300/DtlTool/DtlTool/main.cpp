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
#include <stdlib.h>
#include <string.h>

#ifndef __DONT_USE_WINDOWS_H

#undef LANG_DUTCH
#undef LANG_ENGLISH
#undef LANG_FINNISH
#undef LANG_GERMAN
#undef LANG_ITALIAN
#undef LANG_PORTUGUESE
#undef LANG_SPANISH 
#undef LANG_FRENCH

#include <windows.h>
#include <commdlg.h>
#endif // __DONT_USE_WINDOWS_H

#include <iostream>
#include <fstream>

#pragma hdrstop

#include <time.h>

#include "hwr_sys.h"
#include "ams_mg.h"
#include "langid.h"

#include "dti.h"
#include "dtp.h"

using namespace std;

// --------------- Defines -----------------------------------------------------

#define DTL_ID_STR     "DTL converter. Ver. 1.05."
#define DTC_NAME_SIZE  300
#define DEF_DTI_SUF    ".dtn"
#define DEF_DTL_SUF    ".dtl"
#define DEF_DTC_SUF    ".dtc"

#define DTC_PLT_SIZE  (DTI_XT_PALETTE_SIZE*2)
//#define DTC_CONV_LIMIT 128

#define DC_ABS(x) (((x) > 0) ? (x) : (-(x)))

// --------------- Structures --------------------------------------------------

typedef struct {
	_INT     epoch;
	
	_INT     dif_t;
	_INT     dif_h;
	_INT     dif_s;
	_INT     dif_z;
	_INT     dif_o;
	
	_USHORT  xt_palette[DTC_PLT_SIZE][XT_COUNT+1];
	_USHORT  xh_palette[DTC_PLT_SIZE][XH_COUNT+1];
	_USHORT  xs_palette[DTC_PLT_SIZE][XS_COUNT+1];
	_USHORT  xz_palette[DTC_PLT_SIZE][XZ_COUNT+1];
	_USHORT  xo_palette[DTC_PLT_SIZE][XO_COUNT+1];
} s_palette_type, * p_s_palette_type;

//typedef struct {
//                _UCHAR   xt_palette[DTI_XT_PALETTE_SIZE][XT_COUNT];
//                _UCHAR   xh_palette[DTI_XH_PALETTE_SIZE][XH_COUNT];
//                _UCHAR   xs_palette[DTI_XS_PALETTE_SIZE][XS_COUNT];
//                _UCHAR   xz_palette[DTI_XZ_PALETTE_SIZE][XZ_COUNT];
//                _UCHAR   xo_palette[DTI_XO_PALETTE_SIZE][XO_COUNT];
//                _USHORT  xt_sum[DTI_XT_PALETTE_SIZE][XT_COUNT+1];
//                _USHORT  xh_sum[DTI_XH_PALETTE_SIZE][XH_COUNT+1];
//                _USHORT  xs_sum[DTI_XS_PALETTE_SIZE][XS_COUNT+1];
//                _USHORT  xz_sum[DTI_XZ_PALETTE_SIZE][XZ_COUNT+1];
//                _USHORT  xo_sum[DTI_XO_PALETTE_SIZE][XO_COUNT+1];
//               } s_palette_type, * p_s_palette_type;

// --------------- Globals -----------------------------------------------------

char    version_id[] = "WritePad SDK DTL to DTN converter.";

#ifndef __DONT_USE_WINDOWS_H

static char    *szFilterSpec    = "DTL files\0*.dtl\0All files\0*.*\0\0";
static char    szFileName[MAX_PATH]  = {0};
static char    szFileTitle[MAX_PATH] = {0};
static char    szWindowOpen[MAX_PATH] = {0};

static OPENFILENAMEA * GetOfn()
{
	static OPENFILENAMEA        ofn = { 0 };
	
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner         = NULL;
	ofn.lpstrFilter       = szFilterSpec;
	ofn.lpstrCustomFilter = NULL;
	ofn.nMaxCustFilter    = 0;
	ofn.nFilterIndex      = 1;
	ofn.lpstrFile         = szFileName;
	ofn.nMaxFile          = MAX_PATH;
	ofn.lpstrInitialDir   = NULL;
	ofn.lpstrFileTitle    = szFileTitle;
	ofn.nMaxFileTitle     = MAX_PATH;
	ofn.lpstrTitle        = szWindowOpen;
	ofn.lpstrDefExt       = NULL;
	ofn.Flags             = 0;
	ofn.lpstrInitialDir   = "";
	
	return &ofn;
}

#endif // __DONT_USE_WINDOWS_H

static int      gdif_lev  = 3;
static int      gnum_vect = DTI_XT_PALETTE_SIZE*2;
static int      gtemp;

static _INT     langID = LANGUAGE_ENGLISH;

// --------------- Prototypes --------------------------------------------------

static int    MainDtlToDtn(int mode);
static int    MainDtiConv(p_VOID pdtp, p_VOID _PTR dtl, _INT inum, p_CHAR dtn_name);
static int    FillDtn(p_dti_descr_type pdp, p_dtp_descr_type dtp, p_s_palette_type sp);
static int    GetSetPalette(p_UCHAR xrs, p_UCHAR pls, _INT size, p_USHORT spv, p_INT dif);
static int    UpdatePalettes(p_dte_index_type pdp, p_s_palette_type sp);
static int    MainDtnToDtl(int mode);
static int    MainDtnConv(p_VOID pdtp, p_VOID _PTR dtl);
static int    SetXrVect(p_UCHAR xrs, _INT lsb, p_UCHAR pls, _INT size);
static int    ToBeConverted(int sym);

#define  PALETTE_SIZE  512

typedef struct
{
	_INT    use[PALETTE_SIZE];
	_UCHAR  let[PALETTE_SIZE];
	_UCHAR  var[PALETTE_SIZE];
	_UCHAR  iXr[PALETTE_SIZE]; //in the "var" of "let" for which it is used
}
PALE_INFO, *pPALE_INFO;

#define  SOME_JUNK_XR     ((char)108)
#define  XR_TO_PRINT(xr)  ((char)((xr)==0? SOME_JUNK_XR:((xr)+32) ))

#define  XR_TYPES_COUNT 80
_SHORT  type_use[PALETTE_SIZE][XR_TYPES_COUNT] = {0};

_VOID  PrintPaletteStats( p_UCHAR pal, _INT size, pPALE_INFO ppi,
						 p_CHAR dti_fname, p_CHAR header, _BOOL bRewind );
_VOID  PrintXrPalette( p_UCHAR pal, _INT size, pPALE_INFO ppi,
					  _SHORT  ind_use[PALETTE_SIZE][XR_TYPES_COUNT],
					  _INT ind_size,
					  p_CHAR dti_fname, p_CHAR header, _BOOL bRewind );
_INT  FindClosestPalIndex( p_UCHAR pal, _INT size, _INT iPal );


_INT dtp_load(p_CHAR dtpname, _INT what_to_load, _VOID _PTR _PTR dp);
_INT dtp_unload(p_VOID _PTR dp);
_INT dti_unload(p_VOID _PTR dp);
_INT dti_save(p_CHAR fname, _INT what_to_save, p_VOID dp);

/* **************************************************************************** */
/* *         Main entry point                                                 * */
/* **************************************************************************** */

int main(int argc, char *argv[])
{
	_UCHAR key = '?';

	cout << version_id << endl;
    cout << "Copyright (c) 1997-2016 PhatWare Corp. All rights reserved." << endl;
	
#ifndef __DONT_USE_WINDOWS_H
	 // for file open dialog
	 CoInitializeEx( NULL, COINIT_APARTMENTTHREADED );
#endif // __DONT_USE_WINDOWS_H
	
	while (key != 0)
	{
		cout << endl;
		cout << "Convert DTL to DTN   - c"    << endl;
		cout << "Convert DTN to DTL   - u"    << endl;
		cout << "Exit                 - e, q" << endl;
		
		cout << "> ";
		cin  >> key;
		cout << endl;
		
		try
		{
			switch (key)
			{
				case 'C':
				case 'c':
					MainDtlToDtn(0);
					break;
					
				case 'U':
				case 'u':
					MainDtnToDtl(0);
					break;
					
				case 'Q':
				case 'q':
				case 'E':
				case 'e':
					goto err;
			}
		}
		
		catch (int code)
		{
			cout << "\n\n*** ";
			
			switch (code)
			{
				case 0x0001:
                    cout << " !!!";
                    break;
				case 0x0002:
                    cout << " No xrs in var !!!";
                    break;
				case 0x0003:
                    cout << " Too many xrs in var !!!";
                    break;
				default:
                    cout << "Something went wrong here. Error Code: " << code;
                    break;
			}
			
			cout  << " *** \n\n";
		}
		
	}
	
	
	return 0;
err:
	return 1;
}

/* **************************************************************************** */
/* *        Convert type 4 to type 5                                          * */
/* **************************************************************************** */

int MainDtlToDtn(int mode)
{
	p_CHAR   ptr;
	_CHAR    dtl_name[DTC_NAME_SIZE];
	_CHAR    dtn_name[DTC_NAME_SIZE];
	p_VOID   pdtp, pdtl;
	_INT     inum;
	
	
	cout << "Input DTL file name or ? for selector." << endl;
	cin  >> dtl_name;
	
#ifndef __DONT_USE_WINDOWS_H
	if (dtl_name[0] == '?')
	{
		strcpy(szWindowOpen, "Select DTL file");
		strcpy(szFileName, "*" DEF_DTL_SUF);
		if (!GetOpenFileNameA( GetOfn() ) )
		{
			cout << " No file selected." << endl;
			goto err;
		}
		strcpy((char *)dtl_name, (char *)szFileName);
	}
#endif
	
	strcpy((char *)dtn_name, (char *)dtl_name);
	if ((ptr = strrchr((char *)dtn_name, '.')) != 0) *ptr = 0;
	strcat((char *)dtn_name, DEF_DTI_SUF);
	
	if (dtp_load(dtl_name, DTI_DTE_REQUEST, &pdtp))
	{
		printf("Can't open %s", dtl_name);
		goto err;
	}
	
	printf("\n DTL file loaded\n");
	
	cout << endl << "How many iterations? (0 for cloning) > ";
	cin  >> inum;
	cout << endl;
	
	printf("\n Converting ...\n");
	
	if (MainDtiConv(pdtp, &pdtl, inum, dtn_name))
	{
		printf("Can't convert %s", dtl_name);
		goto err;
	}
	
	if (dti_save(dtn_name, DTI_DTE_REQUEST, pdtl))
	{
		printf("Can't save %s", dtn_name);
		goto err;
	}
	
	printf("\n DTN file saved as %s \n", dtn_name);
	
	dti_unload(&pdtl);
	dtp_unload(&pdtp);
	
	return 0;
err:
	return 1;
}


/* **************************************************************************** */
/* *        Converts DTL to DTN format                                        * */
/* **************************************************************************** */

int MainDtiConv(p_VOID pdtp, p_VOID _PTR dtl, _INT inum, p_CHAR dtn_name)
{
	int i, j, n, m, step;
	int msize, best = 0, start = 0, reset = 1;
	int prev = 0, pprev = 0;
	int dif_t, dif_h, dif_s, dif_z, dif_o;
	p_dtp_descr_type dtp  = (p_dtp_descr_type)pdtp;
	p_let_table_type ltbl;
	p_dtp_sym_header_type psh;
	p_dti_descr_type dti, dtb;
	p_dte_index_type dindex, bindex;
	s_palette_type   sp;
	FILE *file;
	
	dtp_lock(dtp);
	ltbl = (p_let_table_type)dtp->p_dtp;
	
	// ---------- Let's calculate size of new DTL ----------------------------------
	
	msize = sizeof(*dti) + sizeof(*dindex);
	
	for (i = 0; i < 256; i ++)
	{
		if (!ToBeConverted(i))
            continue;
		if ((*ltbl)[i] == 0)
            continue;
		
		msize += sizeof(dte_sym_header_type);
		psh    = (p_dtp_sym_header_type)((p_UCHAR)ltbl + (*ltbl)[i]);
		for (j = 0; j < psh->num_vars; j ++)
			msize += sizeof(dte_var_header_type) + (psh->var_lens[j]-1) * sizeof(xrp_type);
	}
	
	// -------------- Alloc and init -----------------------------------------------
	
	if ((dti = (p_dti_descr_type)HWRMemoryAlloc(msize)) == _NULL)
		goto err;
	HWRMemSet(dti, 0, msize);
	if ((dtb = (p_dti_descr_type)HWRMemoryAlloc(msize)) == _NULL)
		goto err;
	HWRMemSet(dtb, 0, msize);
	
	dindex = (p_dte_index_type)((p_UCHAR)dti + sizeof(*dti));
	bindex = (p_dte_index_type)((p_UCHAR)dtb + sizeof(*dtb));
	
	if ((file = fopen("start.dtn", "rb")) != 0)
	{
		fseek(file, sizeof(dti_header_type), SEEK_SET);
		fread(dindex, sizeof(*dindex), 1, file);
		fclose(file);
		printf("\n Initial palettes read from start.dtn.\n\n");
		start = 6;
		reset = 0;
	}
	else
	{
		if (inum == 0) // Cloning was requested, need start dtn!
		{
			printf("\n Can't read initial palettes from start.dtn!!!\n\n");
			goto err;
		}
		else
        {
            printf("\n Initial palettes were NOT read from start.dtn.\n\n");
        }
    }
    
	HWRStrCpy( dti->dti_fname, "DtlConv.dtl" );
	HWRMemCpy( (p_VOID)dti->object_type, (p_VOID)DTI_DTI_OBJTYPE, DTI_ID_LEN);
	HWRMemCpy( (p_VOID)dti->type, (p_VOID)DTI_DTI_TYPE, DTI_ID_LEN);
	HWRMemCpy( (p_VOID)dti->version, (p_VOID)DTI_DTI_VER, DTI_ID_LEN);
	
	dti->h_dte = 0;
	dti->p_dte = (p_UCHAR)dti + sizeof(*dti);
	
	dindex->len = msize - sizeof(*dti);
	
	HWRMemCpy(dtb, dti, msize);
	
	// -------------- Now let's convert it -----------------------------------------
	
	// randomize();  // ??
	dif_t = dif_h = dif_s = dif_z = dif_o = 0;
	
	for (step = 0; step < inum; step ++)
	{
		if (reset) HWRMemSet(dindex, 0, sizeof(*dindex));
		printf("Step %d, best dif: %d\n", step+1, best);
		reset = 1;
		
		for (m = start; m < 100; m ++) // Iterate it some times
		{
			HWRMemSet(&sp, 0, sizeof(sp));
			
			printf(" %d:%d - ", step+1, m+1);
			gtemp = m;
			sp.epoch = m;
			
			FillDtn(dti, dtp, &sp);
			
			n = UpdatePalettes(dindex, &sp);
			
			j = sp.dif_t + sp.dif_h + sp.dif_s + sp.dif_z + sp.dif_o;
			printf("SumCh:%d, D: t:%d, h:%d, s:%d, z:%d, o:%d, a:%d\n", n, sp.dif_t, sp.dif_h, sp.dif_s, sp.dif_z, sp.dif_o, j);
			
			//    if (gdif_lev > 0) gdif_lev --;
			//    if (n == 0 && gdif_lev > 2) {gdif_lev = 3; gnum_vect = 256;}
			if (n == 0)
				break;
			if (n == pprev && m > 20)
				break;
			pprev = prev;
			prev = n;
		}
		
		if (dif_t == 0 || sp.dif_t < dif_t || sp.dif_h < dif_h ||
			sp.dif_s < dif_s || sp.dif_z < dif_z || sp.dif_o < dif_o)
		{
			if (dif_t == 0 || sp.dif_t < dif_t)
			{
				dif_t = sp.dif_t;
				HWRMemCpy( bindex->xt_palette, dindex->xt_palette, sizeof(bindex->xt_palette));
			}
			if (dif_h == 0 || sp.dif_h < dif_h)
			{
				dif_h = sp.dif_h;
				HWRMemCpy(bindex->xh_palette, dindex->xh_palette, sizeof(bindex->xh_palette));
			}
			if (dif_s == 0 || sp.dif_s < dif_s)
			{
				dif_s = sp.dif_s;
				HWRMemCpy(bindex->xs_palette, dindex->xs_palette, sizeof(bindex->xs_palette));
			}
			if (dif_z == 0 || sp.dif_z < dif_t)
			{
				dif_z = sp.dif_z;
				HWRMemCpy(bindex->xz_palette, dindex->xz_palette, sizeof(bindex->xz_palette));
			}
			if (dif_o == 0 || sp.dif_o < dif_o)
			{
				dif_o = sp.dif_o;
				HWRMemCpy(bindex->xo_palette, dindex->xo_palette, sizeof(bindex->xo_palette));
			}
			
			HWRMemSet(&sp, 0, sizeof(sp));
			sp.epoch = 100;
			
			FillDtn(dtb, dtp, &sp);
			
			j = best = sp.dif_t + sp.dif_h + sp.dif_s + sp.dif_z + sp.dif_o;
			printf("Res: D: t:%d, h:%d, s:%d, z:%d, o:%d, a:%d\n", sp.dif_t, sp.dif_h, sp.dif_s, sp.dif_z, sp.dif_o, j);
			
			dtb->p_dte = (p_UCHAR)bindex;
			if (dti_save(dtn_name, DTI_DTE_REQUEST, dtb))
			{
				printf("Can't save %s", dtn_name);
				goto err;
			}
			printf("* DTN file saved as %s \n", dtn_name);
		}
	}
	
	// --------------- Cloning of DTN ----------------------------------------------
	
	if (inum == 0)
	{
		HWRMemSet(&sp, 0, sizeof(sp));
		sp.epoch = 100;
		
		FillDtn(dtb, dtp, &sp);
		
		n = UpdatePalettes(bindex, &sp);
		
		j = best = sp.dif_t + sp.dif_h + sp.dif_s + sp.dif_z + sp.dif_o;
		printf("Res: D: t:%d, h:%d, s:%d, z:%d, o:%d, a:%d\n", sp.dif_t, sp.dif_h, sp.dif_s, sp.dif_z, sp.dif_o, j);
		
		dtb->p_dte = (p_UCHAR)bindex;
	}
	
	// -------------- That's it ! --------------------------------------------------
	
	printf("\nBest dif was: %d\n", best);
	
	HWRMemoryFree(dti);
	
	*dtl = (p_VOID)dtb;
	
	return 0;
err:
	return 1;
}

/* **************************************************************************** */
/* *        Update sums and counters for Xr                                   * */
/* **************************************************************************** */

_INT FillDtn(p_dti_descr_type dti, p_dtp_descr_type dtp, p_s_palette_type sp)
{
	int i, j, n, loc, k, v, nxr;
	p_let_table_type ltbl;
	p_dtp_sym_header_type psh;
	p_dte_sym_header_type pish;
	p_dte_var_header_type pvh;
	p_dte_index_type dindex;
	p_xrpp_type      pxrp;
	p_xrp_type       pxr;
	
	static PALE_INFO pi_t;
	static PALE_INFO pi_s;
	static PALE_INFO pi_o;
	static PALE_INFO pi_z; //links
	static PALE_INFO pi_h;
	
	HWRMemSet( pi_t.use, 0, sizeof(pi_t.use) );
	HWRMemSet( pi_s.use, 0, sizeof(pi_s.use) );
	HWRMemSet( pi_o.use, 0, sizeof(pi_o.use) );
	HWRMemSet( pi_z.use, 0, sizeof(pi_z.use) );
	HWRMemSet( pi_h.use, 0, sizeof(pi_h.use) );
	
	
	ltbl   = (p_let_table_type)dtp->p_dtp;
	dindex = (p_dte_index_type)((p_UCHAR)dti + sizeof(*dti));
	
	loc = sizeof(*dindex);
	for (n = 0; n < 256; n ++)
	{
		//    if (n == 0) loc = sizeof(*dindex); // Restore position after extra pass ...
		//    if (n < 0) i = 'z' + n;
		//     else i = n;
		i = n;
		if (!ToBeConverted(i)) continue;
		if ((*ltbl)[i] == 0) continue;
		
		dindex->sym_index[i] = (_USHORT)(loc >> 2);
		loc += sizeof(dte_sym_header_type);
		pish = (p_dte_sym_header_type)((p_UCHAR)dindex + ((_INT)dindex->sym_index[i]<<2));
		psh  = (p_dtp_sym_header_type)((p_UCHAR)ltbl + (*ltbl)[i]);
		
		pish->num_vars = psh->num_vars;
		pish->loc_vs_border = psh->loc_vs_border;
		pish->let      = psh->let;
		pish->language = psh->language;
		
		//    HWRMemCpy(pish, psh, sizeof(*pish));
		pxrp = (p_xrpp_type)((p_UCHAR)psh + sizeof(*psh));
		pvh  = (p_dte_var_header_type)((p_UCHAR)pish + sizeof(*pish));
		for (j = 0; j < psh->num_vars; j ++)
		{
			nxr  = psh->var_lens[j];
			if (nxr < 1)
				throw(0x0002);
			if (nxr > DTI_XR_SIZE)
				throw(0x0003);
			
			pvh->nx_and_vex = (_UCHAR)(nxr | (psh->var_vexs[j] << DTI_VEX_OFFS));
			pvh->veis       = psh->var_veis[j];
			pvh->pos        = psh->var_pos[j];
			pvh->size       = psh->var_size[j];
			
			pxr  = pvh->xrs;
			for (k = 0; k < nxr; k ++, pxrp ++, pxr ++)
			{
				if (sp->epoch < 5 && rand() > RAND_MAX/10) continue;
				
				pxr->type = (_UCHAR)(pxrp->type << 5);
				pxr->penl = (_UCHAR)(pxrp->penl); if (pxr->penl > 0x0F) pxr->penl = 0x0F;
				pxr->penl|= (_UCHAR)((pxrp->type << 1) & 0xF0);
				pxr->attr = pxrp->attr;
				
				v = GetSetPalette(pxrp->xtc, &dindex->xt_palette[0][0], XT_COUNT, &sp->xt_palette[0][0], &sp->dif_t);
				pxr->xtc  = (_UCHAR)(v >> 1);
				if (v&1)
					pxr->type |= DTI_XT_LSB;
				pi_t.use[v]++;
				pi_t.let[v] = n;
				pi_t.var[v] = j;
				pi_t.iXr[v] = k;
				type_use[v][pxrp->type]++;
				
				v = GetSetPalette(pxrp->xhc, &dindex->xh_palette[0][0], XH_COUNT, &sp->xh_palette[0][0], &sp->dif_h);
				pxr->xhc  = (_UCHAR)(v >> 1);
				if (v&1)
					pxr->type |= DTI_XH_LSB;
				pi_h.use[v]++;
				pi_h.let[v] = n;
				pi_h.var[v] = j;
				pi_h.iXr[v] = k;
				
				v = GetSetPalette(pxrp->xsc, &dindex->xs_palette[0][0], XS_COUNT, &sp->xs_palette[0][0], &sp->dif_s);
				pxr->xsc  = (_UCHAR)(v >> 1);
				if (v&1)
					pxr->type |= DTI_XS_LSB;
				pi_s.use[v]++;
				pi_s.let[v] = n;
				pi_s.var[v] = j;
				pi_s.iXr[v] = k;
				
				v = GetSetPalette(pxrp->xzc, &dindex->xz_palette[0][0], XZ_COUNT, &sp->xz_palette[0][0], &sp->dif_z);
				pxr->xzc  = (_UCHAR)(v >> 1);
				if (v&1)
					pxr->type |= DTI_XZ_LSB;
				pi_z.use[v]++;
				pi_z.let[v] = n;
				pi_z.var[v] = j;
				pi_z.iXr[v] = k;
				
				v = GetSetPalette(pxrp->xoc, &dindex->xo_palette[0][0], XO_COUNT, &sp->xo_palette[0][0], &sp->dif_o);
				pxr->xoc  = (_UCHAR)(v >> 1);
				if (v&1)
					pxr->type |= DTI_XO_LSB;
				pi_o.use[v]++;
				pi_o.let[v] = n;
				pi_o.var[v] = j;
				pi_o.iXr[v] = k;
			}
			
			pvh = (p_dte_var_header_type)((p_UCHAR)(pvh) + sizeof(*pvh) + sizeof(*pxr)*(nxr-1));
			loc += sizeof(*pvh) + sizeof(*pxr)*(nxr-1);
		}
	}
	
	PrintXrPalette( &dindex->xt_palette[0][0], XT_COUNT,
				   &pi_t, type_use, XR_TYPES_COUNT,
				   dti->dti_fname, (p_CHAR)"XR palette", _TRUE );
	
	PrintPaletteStats( &dindex->xt_palette[0][0], XT_COUNT,
					  &pi_t, dti->dti_fname, (p_CHAR)"XR palette", _TRUE );
	PrintPaletteStats( &dindex->xh_palette[0][0], XH_COUNT,
					  &pi_h, dti->dti_fname, (p_CHAR)"Heights palette", _FALSE );
	PrintPaletteStats( &dindex->xs_palette[0][0], XS_COUNT,
					  &pi_s, dti->dti_fname, (p_CHAR)"Shifts palette", _FALSE );
	PrintPaletteStats( &dindex->xz_palette[0][0], XZ_COUNT,
					  &pi_z, dti->dti_fname, (p_CHAR)"Links palette", _FALSE );
	PrintPaletteStats( &dindex->xo_palette[0][0], XO_COUNT,
					  &pi_o, dti->dti_fname, (p_CHAR)"Orients palette", _FALSE );
	
	return 0;
}

/* **************************************************************************** */

_INT  DifVal( _INT v, _INT vVect )
{
	_INT  dv = DC_ABS(v - vVect);
	
	if  ( dv > v  ||  dv > vVect )
		dv += 1; //  for hand-tuned vars, those that rely on small
	//corrs. for some auxiliary XRs
	
	if  ( (v==0 || vVect==0) && dv!=0 )
		dv += 1; //zero corr is special one
	
	return  dv;
	/*
	 if  ( dv>=3  &&  dv<=6 )
	 dv += 1; //for hand-tuned vars
	 
	 return  dv*dv;
	 */
}
/**************************************/

_INT  GetPalVal( p_UCHAR pal, _INT size, _INT iPal, _INT index )
{
	_INT  val = pal[ (iPal>>1)*size + index ];
	
	if  ( (iPal&1) == 0 )
		val &= 0x0F;
	else
		val >>= 4;
	
	return  val;
}
/**************************************/

_VOID  PrintPaletteStats( p_UCHAR pal, _INT size, pPALE_INFO ppi, p_CHAR dti_fname, p_CHAR header, _BOOL bRewind )
{
	_CHAR   stat_name[DTC_NAME_SIZE];
	FILE    *stat = NULL;
	p_CHAR  pPt;
	_INT    iPal, ind, iPalClosest;
	
	cout << "Writing statistics: " << (header? header:"???");
	
	//Prepare file for writing:
	strcpy( stat_name, dti_fname );
	pPt = strrchr( stat_name, '.' );
	if  ( pPt )
		*pPt = 0;
	strcat( stat_name, ".PST" );
	
	stat = fopen( stat_name, bRewind? "wt":"at" );
	if  ( !stat )  {
		cout << "\nCan't open output file " << stat_name;
		goto  EXIT_ACTIONS;
	}
	
	//Write the header info:
	if  ( !bRewind )
		fprintf( stat, "\n\n-----------------------------------------------------------\n" );
	fprintf( stat,
			"\n--- %s ---  Length=%d\n\n"
			"Legend: palette_type palette_index(usage)(letter,iVar,iXr) {same for closest match} ...\n"
			"                                        ",
			header? header:"No title", size );
	for  ( ind=0;  ind<size;  ind++ )
		fprintf( stat, " %2d", ind );
	fprintf( stat, "\n" );
	
	//Write the tables' info:
	
	for  ( iPal=0;  iPal<PALETTE_SIZE;  iPal++ )  { /*30*/
		
		//if  ( ppi->use[iPal] > 1 )
		//  continue;  //to show only barely used lines
		
		iPalClosest = FindClosestPalIndex( pal, size, iPal );
		
		fprintf( stat, "\n%c%3d(%3d)(%c,%2d,%2d) ",
				header? header[0]:' ', iPal, ppi->use[iPal],
				ppi->let[iPal], (int)ppi->var[iPal], (int)ppi->iXr[iPal] );
		
		fprintf( stat, " {%3d(%3d)(%c,%2d,%2d)} ",
				iPalClosest, ppi->use[iPalClosest],
				ppi->let[iPalClosest], (int)ppi->var[iPalClosest],
				(int)ppi->iXr[iPalClosest] );
		
		for  ( ind=0;  ind<size;  ind++ )
			fprintf( stat, " %2d", GetPalVal( pal, size, iPal, ind ) );
		
	} /*30*/
	
	
EXIT_ACTIONS:;
	if  ( stat )
		fclose( stat );
	
	cout << "\n";
	
} /*PrintPaletteStats*/
/* **************************************************************************** */

//CHE:
_VOID  PrintXrPalette( p_UCHAR pal, _INT size, pPALE_INFO ppi,
					  _SHORT  ind_use[PALETTE_SIZE][XR_TYPES_COUNT],
					  _INT ind_size,
					  p_CHAR dti_fname, p_CHAR header, _BOOL bRewind )
{
	_CHAR   stat_name[DTC_NAME_SIZE];
	FILE    *stat = NULL;
	p_CHAR  pPt;
	_INT    iPal, ind; //, iPalClosest;
	_INT    i1st, i2nd;
	
	
	cout << "Writing " << (header? header:"???");
	
	//Prepare file for writing:
	strcpy( stat_name, dti_fname );
	pPt = strrchr( stat_name, '.' );
	if  ( pPt )
		*pPt = 0;
	strcat( stat_name, ".PLT" );
	
	stat = fopen( stat_name, bRewind? "wt":"at" );
	if  ( !stat )  {
		cout << "\nCan't open output file " << stat_name;
		goto  EXIT_ACTIONS;
	}
	
	//Write the header info:
	if  ( !bRewind )
		fprintf( stat, "\n\n-----------------------------------------------------------\n" );
	fprintf( stat,
			"\n--- %s ---  Length=%d\n\n"
			" index usage most_often_used_type 2nd_most_often_used_type percent_2nd_to_1st\n"
			"- - - - - - - -                   ",
			header? header:"No title", size );
	for  ( ind=0;  ind<size;  ind++ )
		fprintf( stat, " %2d", ind );
	fprintf( stat, "\n- - - - - - - -                   " );
	for  ( ind=0;  ind<size;  ind++ )
		fprintf( stat, " %2c", XR_TO_PRINT(ind) );
	fprintf( stat, "\n" );
	
	//Write the tables' info:
	
	for  ( iPal=0;  iPal<PALETTE_SIZE;  iPal++ )  { /*30*/
		
		fprintf( stat, "\n %3d %3d ", iPal, ppi->use[iPal] );
		
		//Print the XR index this palette entry corresponds to:
		i1st = i2nd = 0;
		for  ( ind=0;  ind<ind_size;  ind++ )  {
			if  ( ind_use[iPal][ind] >= ind_use[iPal][i1st] )  {
				i2nd = i1st;
				i1st = ind;
			}
			else  if  ( ind_use[iPal][ind] > ind_use[iPal][i2nd] )  {
				i2nd = ind;
			}
		}
		
		if  ( ind_use[iPal][i1st] > 0 )
			fprintf( stat, "%2d %2d %3d  %c %c  ",
					i1st, ind_use[iPal][i2nd]>0? i2nd:0,
					(100*ind_use[iPal][i2nd])/ind_use[iPal][i1st],
					XR_TO_PRINT(i1st),
					(ind_use[iPal][i2nd]>0? XR_TO_PRINT(i2nd):SOME_JUNK_XR) );  //for non-used just print some junk
		else
			fprintf( stat, "-- -- ---  %c %c  ", SOME_JUNK_XR, SOME_JUNK_XR );   //for non-used just print some junk
		
		
		//Print palette values:
		for  ( ind=0;  ind<size;  ind++ )
			fprintf( stat, " %2d", GetPalVal( pal, size, iPal, ind ) );
		
		/*
		 //Print additional info (as in "PrintPaletteStats"):
		 iPalClosest = FindClosestPalIndex( pal, size, iPal );
		 
		 fprintf( stat, "%c%3d(%3d)(%c,%2d,%2d) ",
		 header? header[0]:' ', iPal, ppi->use[iPal],
		 ppi->let[iPal], (int)ppi->var[iPal], (int)ppi->iXr[iPal] );
		 
		 fprintf( stat, " {%3d(%3d)(%c,%2d,%2d)} ",
		 iPalClosest, ppi->use[iPalClosest],
		 ppi->let[iPalClosest], (int)ppi->var[iPalClosest],
		 (int)ppi->iXr[iPalClosest] );
		 */
		
	} /*30*/
	
	
EXIT_ACTIONS:;
	if  ( stat )
		fclose( stat );
	
	cout << "\n";
	
} /*PrintXrPalette*/
/* **************************************************************************** */

_INT  FindClosestPalIndex( p_UCHAR pal, _INT size, _INT iPal )
{
	_INT  d, dMin;
	_INT  i, ind, iClosest;
	
	dMin     = 32767;
	iClosest = 0;
	
	for  ( i=0;  i<PALETTE_SIZE;  i++ )  {
		
		if  ( i == iPal )
			continue;
		
		d = 0;
		for  ( ind=0;  ind<size;  ind++ )
			d += ( GetPalVal(pal,size,iPal,ind), GetPalVal(pal,size,i,ind) );
		
		if  ( d < dMin )  {
			dMin     = d;
			iClosest = i;
		}
		
	}
	
	return  iClosest;
	
} /*FindClosestPalIndex*/
/* **************************************************************************** */

/* **************************************************************************** */
/* *        Converts one xr                                                   * */
/* **************************************************************************** */

int GetSetPalette(p_UCHAR xrs, p_UCHAR pls, _INT size, p_USHORT spv, p_INT mdif)
{
	_INT   i, j, k;
	_INT   v;
	_INT   dif, min, min_dif, pos, pos0, sum;
	_UCHAR vect[XT_COUNT];
	
	// ------------- Let's copy and clean up xr vector -----------------------------
	
	for (i = 0, min = 255, sum = 0; i < size; i ++)
	{
		sum += vect[i] = (_UCHAR)((i%2 == 0) ? (xrs[i/2] >> 4) : (xrs[i/2] & 0x0F));
		if (vect[i] < min)
			min = vect[i];
	}
	
	if (sum == 0)
		vect[0] = 1;  // To Escape all-zero-vector prolbem
	
	//  if (min > 0) for (i = 0; i < size; i ++) if (vect[i] == min) vect[i] = 0;
	
	// ------------- Let's find best matching and update it ------------------------
	
	for (i = pos = pos0 = 0, min_dif = size*12; i < gnum_vect; i ++)
	{
		sum = dif = 0;
		
		
		for (j = 0, k = (i>>1)*size; j < size; j ++, k ++)
		{
			if ((i & 1) == 0)
				v = pls[k] & 0x0f;
			else
				v = pls[k] >> 4;
			dif += DifVal( v, vect[j] ); //DC_ABS(v-vect[j]);
			sum += v;
		}
		
		if (sum == 0)
		{
			pos0 = i;
			break;
		}
		if (min_dif > dif)
		{
			pos = i;
			min_dif = dif;
		}
	}
	
	if (min_dif < gdif_lev || sum > 0)
	{                         // Merge vectors
		gtemp = 5;// Ooops ...
	}
	else
	{                      // Write down vector
		pos = pos0;
		for (j = 0, k = (pos>>1)*size; j < size; j ++, k ++)
			pls[k] |= (_UCHAR)((pos&1) ? vect[j] << 4 : vect[j]);
	}
	
	for (i = 0, k = pos*(size+1), spv[k+size] ++; i < size; i ++, k ++)
		spv[k] += vect[i];
	
	*mdif += min_dif;
	return pos;
}

/* **************************************************************************** */
/* *        Update sums and counters for Xr                                   * */
/* **************************************************************************** */
_INT UpdatePalettes(p_dte_index_type pdp, p_s_palette_type sp)
{
	_INT   i, j, k;
	_UCHAR c;
	_INT   sum = 0;
	_UCHAR sv[XT_COUNT];
	
	for (i = k = 0; i < DTI_XT_PALETTE_SIZE; i ++, k += 2)
	{
		HWRMemCpy(sv, &pdp->xt_palette[i][0], XT_COUNT);
		HWRMemSet(&pdp->xt_palette[i][0], 0, XT_COUNT);
		
		for (j = 0; sp->xt_palette[k][XT_COUNT] && j < XT_COUNT; j ++)
		{
			c = (_UCHAR)((sp->xt_palette[k][j]+sp->xt_palette[k][XT_COUNT]/2)/sp->xt_palette[k][XT_COUNT]);
			sum += DC_ABS((sv[j] & 0x0f) - c);
			pdp->xt_palette[i][j] = c;
		}
		for (j = 0; sp->xt_palette[k+1][XT_COUNT] && j < XT_COUNT; j ++)
		{
			c = (_UCHAR)((sp->xt_palette[k+1][j]+sp->xt_palette[k+1][XT_COUNT]/2)/sp->xt_palette[k+1][XT_COUNT]);
			sum += DC_ABS((sv[j] >> 4)- c);
			pdp->xt_palette[i][j] |= (_UCHAR)(c << 4);
		}
	}
	
	for (i = k = 0; i < DTI_XH_PALETTE_SIZE; i ++, k += 2)
	{
		HWRMemCpy(sv, &pdp->xh_palette[i][0], XH_COUNT);
		HWRMemSet(&pdp->xh_palette[i][0], 0, XH_COUNT);
		
		for (j = 0; sp->xh_palette[k][XH_COUNT] && j < XH_COUNT; j ++)
		{
			c = (_UCHAR)((sp->xh_palette[k][j]+sp->xh_palette[k][XH_COUNT]/2)/sp->xh_palette[k][XH_COUNT]);
			sum += DC_ABS((sv[j] & 0x0f) - c);
			pdp->xh_palette[i][j] = c;
		}
		for (j = 0; sp->xh_palette[k+1][XH_COUNT] && j < XH_COUNT; j ++)
		{
			c = (_UCHAR)((sp->xh_palette[k+1][j]+sp->xh_palette[k+1][XH_COUNT]/2)/sp->xh_palette[k+1][XH_COUNT]);
			sum += DC_ABS((sv[j]>>4) - c);
			pdp->xh_palette[i][j] |= (_UCHAR)(c << 4);
		}
	}
	
	for (i = k = 0; i < DTI_XS_PALETTE_SIZE; i ++, k += 2)
	{
		HWRMemCpy(sv, &pdp->xs_palette[i][0], XS_COUNT);
		HWRMemSet(&pdp->xs_palette[i][0], 0, XS_COUNT);
		
		for (j = 0; sp->xs_palette[k][XS_COUNT] && j < XS_COUNT; j ++)
		{
			c = (_UCHAR)((sp->xs_palette[k][j]+sp->xs_palette[k][XS_COUNT]/2)/sp->xs_palette[k][XS_COUNT]);
			sum += DC_ABS((sv[j] & 0x0f) - c);
			pdp->xs_palette[i][j] = c;
		}
		for (j = 0; sp->xs_palette[k+1][XS_COUNT] && j < XS_COUNT; j ++)
		{
			c = (_UCHAR)((sp->xs_palette[k+1][j]+sp->xs_palette[k+1][XS_COUNT]/2)/sp->xs_palette[k+1][XS_COUNT]);
			sum += DC_ABS((sv[j] >> 4)- c);
			pdp->xs_palette[i][j] |= (_UCHAR)(c << 4);
		}
	}
	
	for (i = k = 0; i < DTI_XZ_PALETTE_SIZE; i ++, k += 2)
	{
		HWRMemCpy(sv, &pdp->xz_palette[i][0], XZ_COUNT);
		HWRMemSet(&pdp->xz_palette[i][0], 0, XZ_COUNT);
		
		for (j = 0; sp->xz_palette[k][XZ_COUNT] && j < XZ_COUNT; j ++)
		{
			c = (_UCHAR)((sp->xz_palette[k][j]+sp->xz_palette[k][XZ_COUNT]/2)/sp->xz_palette[k][XZ_COUNT]);
			sum += DC_ABS((sv[j]&0x0f) - c);
			pdp->xz_palette[i][j] = c;
		}
		for (j = 0; sp->xz_palette[k+1][XZ_COUNT] && j < XZ_COUNT; j ++)
		{
			c = (_UCHAR)((sp->xz_palette[k+1][j]+sp->xz_palette[k+1][XZ_COUNT]/2)/sp->xz_palette[k+1][XZ_COUNT]);
			sum += DC_ABS((sv[j] >> 4) - c);
			pdp->xz_palette[i][j] |= (_UCHAR)(c << 4);
		}
	}
	
	for (i = k = 0; i < DTI_XO_PALETTE_SIZE; i ++, k += 2)
	{
		HWRMemCpy(sv, &pdp->xo_palette[i][0], XO_COUNT);
		HWRMemSet(&pdp->xo_palette[i][0], 0, XO_COUNT);
		
		for (j = 0; sp->xo_palette[k][XO_COUNT] && j < XO_COUNT; j ++)
		{
			c = (_UCHAR)((sp->xo_palette[k][j]+sp->xo_palette[k][XO_COUNT]/2)/sp->xo_palette[k][XO_COUNT]);
			sum += DC_ABS((sv[j] & 0x0f) - c);
			pdp->xo_palette[i][j] = c;
		}
		for (j = 0; sp->xo_palette[k+1][XO_COUNT] && j < XO_COUNT; j ++)
		{
			c = (_UCHAR)((sp->xo_palette[k+1][j]+sp->xo_palette[k+1][XO_COUNT]/2)/sp->xo_palette[k+1][XO_COUNT]);
			sum += DC_ABS((sv[j] >> 4) - c);
			pdp->xo_palette[i][j] |= (_UCHAR)(c << 4);
		}
	}
	
	return sum;
}

/* **************************************************************************** */
/* *        UnConvert type 5 to type 4                                        * */
/* **************************************************************************** */

int MainDtnToDtl(int mode)
{
	p_CHAR   ptr;
	_CHAR    dtl_name[DTC_NAME_SIZE];
	_CHAR    dtn_name[DTC_NAME_SIZE];
	p_VOID   pdtp, pdtl;
	
	
	cout << "Input DTN file name or ? for selector." << endl;
	cin  >> dtn_name;
	
#ifndef __DONT_USE_WINDOWS_H
	if (dtn_name[0] == '?')
	{
		strcpy(szWindowOpen, "Select DTN file");
		strcpy(szFileName, "*" DEF_DTI_SUF);
		if (!GetOpenFileNameA( GetOfn() ) )
		{
			cout << " No file selected." << endl;
			goto err;
		}
		strcpy((char *)dtn_name, (char *)szFileName);
	}
#endif

	strcpy((char *)dtl_name, (char *)dtn_name);
	if ((ptr = strrchr((char *)dtl_name, '.')) != 0) *ptr = 0;
	strcat((char *)dtl_name, DEF_DTC_SUF);
	
	if ( dti_load( dtn_name, DTI_DTE_REQUEST, &pdtp, langID ) )
	{
		printf("Can't open %s", dtn_name);
		goto err;
	}
	
	printf("\n DTN file loaded, \n\n Converting ...\n");
	
	if (MainDtnConv(pdtp, &pdtl))
	{
		printf("Can't convert %s", dtn_name);
		goto err;
	}
	
	if ( dtp_save(dtl_name, DTI_DTE_REQUEST, pdtl, langID ) )
	{
		printf("Can't save %s", dtl_name);
		goto err;
	}
	
	printf("\n DTL file saved as %s \n", dtl_name);
	
	dti_unload(&pdtp);
	dtp_unload(&pdtl);
	
	return 0;
err:
	return 1;
}

/* **************************************************************************** */
/* *        Converts DTN to DTL format                                        * */
/* **************************************************************************** */

int MainDtnConv(p_VOID pdtp, p_VOID _PTR dtl)
{
	int i, j, k;
	int msize, loc, nxr;
	p_dtp_descr_type dtp;
	p_let_table_type ltbl;
	p_dtp_sym_header_type psh;
	p_dte_sym_header_type pish;
	p_dte_var_header_type pvh;
	p_dti_descr_type dti = (p_dti_descr_type)pdtp;
	p_dte_index_type dindex;
	p_xrpp_type      pxrp;
	p_xrp_type       pxr;
	
	
	dti_lock(dti);
	dindex = (p_dte_index_type)dti->p_dte;
	
	// ---------- Let's calculate size of new DTL ----------------------------------
	
	msize = sizeof(*dtp) + sizeof(let_table_type);
	
	for (i = 0; i < 256; i ++)
	{
		if (!ToBeConverted(i))
            continue;
		if (dindex->sym_index[i] == 0)
            continue;
		
		msize += sizeof(dtp_sym_header_type);
		pish   = (p_dte_sym_header_type)((p_UCHAR)dindex + ((_INT)dindex->sym_index[i] << 2));
		pvh    = (p_dte_var_header_type)((p_UCHAR)pish + sizeof(*pish));
		for (j = 0; j < pish->num_vars; j ++)
		{
			nxr = pvh->nx_and_vex & DTI_NXR_MASK;
			msize += sizeof(xrpp_type)*nxr;
			pvh = (p_dte_var_header_type)((p_UCHAR)pvh + sizeof(*pvh) + sizeof(xrp_type)*(nxr-1));
		}
	}
	
	// -------------- Alloc and init -----------------------------------------------
	
	if ((dtp = (p_dtp_descr_type)HWRMemoryAlloc(msize)) == _NULL)
		goto err;
	HWRMemSet(dtp, 0, msize);
	
	HWRStrCpy( dtp->dtp_fname, "DtlConv.dtl");
	HWRMemCpy( (p_VOID)dtp->object_type, (p_VOID)DTP_DTP_OBJTYPE, DTP_ID_LEN);
	HWRMemCpy( (p_VOID)dtp->type, (p_VOID)"EngM", DTP_ID_LEN);
	HWRMemCpy( (p_VOID)dtp->version, (p_VOID)DTP_DTP_VER, DTP_ID_LEN);
	
	dtp->h_dtp = 0;
	dtp->p_dtp = (p_UCHAR)dtp + sizeof(*dtp);
	
	// -------------- Now let's convert it -----------------------------------------
	
	ltbl = (p_let_table_type)((p_UCHAR)dtp + sizeof(*dtp));
	loc  = sizeof(let_table_type);
	dtp->xrt_chsum = msize - sizeof(*dtp); // Temp transfer param to SaveDti
	
	for (i = 0; i < 256; i ++)
	{
		if (!ToBeConverted(i))
			continue;
		if (dindex->sym_index[i] == 0)
			continue;
		
		(*ltbl)[i] = loc;
		loc += sizeof(dtp_sym_header_type);
		pish = (p_dte_sym_header_type)((p_UCHAR)dindex + ((_INT)dindex->sym_index[i]<<2));
		psh  = (p_dtp_sym_header_type)((p_UCHAR)ltbl + (*ltbl)[i]);
		//    HWRMemCpy(psh, pish, sizeof(*psh));
		
		psh->num_vars = pish->num_vars;
		psh->loc_vs_border = pish->loc_vs_border;
		psh->let      = pish->let;
		psh->language = pish->language;
		
		pxrp = (p_xrpp_type)((p_UCHAR)psh + sizeof(*psh));
		pvh  = (p_dte_var_header_type)((p_UCHAR)pish + sizeof(*pish));
		for (j = 0; j < pish->num_vars; j ++)
		{
			nxr  = pvh->nx_and_vex & DTI_NXR_MASK; if (nxr < 1) throw(0x0002); if (nxr > DTI_XR_SIZE) throw(0x0003);
			pxr  = pvh->xrs;
			
			psh->var_lens[j] = (_UCHAR)(nxr);
			psh->var_vexs[j] = (_UCHAR)(pvh->nx_and_vex >> DTI_VEX_OFFS);
			psh->var_veis[j] = (_UCHAR)(pvh->veis);
			psh->var_pos[j]  = (_UCHAR)(pvh->pos);
			psh->var_size[j] = (_UCHAR)(pvh->size);
			
			for (k = 0; k < nxr; k ++, pxrp ++, pxr ++, loc += sizeof(*pxrp))
			{
				pxrp->type   = (_UCHAR)((pxr->type >> 5) | ((pxr->penl & 0xF0) >> 1));
				pxrp->height = (_UCHAR)(1);
				pxrp->attr   = pxr->attr;
				pxrp->penl   = (_UCHAR)(pxr->penl & 0x0F);
				SetXrVect(pxrp->xtc, (pxr->type & DTI_XT_LSB), &dindex->xt_palette[pxr->xtc][0], XT_COUNT);
				SetXrVect(pxrp->xhc, (pxr->type & DTI_XH_LSB), &dindex->xh_palette[pxr->xhc][0], XH_COUNT);
				SetXrVect(pxrp->xsc, (pxr->type & DTI_XS_LSB), &dindex->xs_palette[pxr->xsc][0], XS_COUNT);
				SetXrVect(pxrp->xzc, (pxr->type & DTI_XZ_LSB), &dindex->xz_palette[pxr->xzc][0], XZ_COUNT);
				SetXrVect(pxrp->xoc, (pxr->type & DTI_XO_LSB), &dindex->xo_palette[pxr->xoc][0], XO_COUNT);
			}
			
			pvh = (p_dte_var_header_type)((p_UCHAR)(pvh) + sizeof(*pvh) + sizeof(*pxr)*(nxr-1));
		}
	}
	
	// -------------- That's it ! --------------------------------------------------
	
	*dtl = (p_VOID)dtp;
	
	return 0;
err:
	return 1;
}

/* **************************************************************************** */
/* *        Converts one xr                                                   * */
/* **************************************************************************** */

_INT SetXrVect(p_UCHAR xrs, _INT lsb, p_UCHAR pls, _INT size)
{
	_INT   i;
	_INT   a, b;
	
	// ------------- Let's copy xr vector ------------------------------------------
	
	for (i = 0; i < size; i += 2)
	{
		a = (lsb) ? pls[i] >> 4 : pls[i] & 0x0F;
		b = (lsb) ? pls[i+1] >> 4 : pls[i+1] & 0x0F;
		xrs[i/2] = (_UCHAR)((a << 4) | b);
	}
	
	return 0;
}


/* **************************************************************************** */
/* *        error mesage printing routine                                     * */
/* **************************************************************************** */

_VOID err_msg (_CHAR _PTR msg)
{
	_UCHAR  szMsg[140];
	
	if  ( msg != _NULL )
	{
		sprintf((p_CHAR)szMsg, (p_CHAR)"ERR_MSG> %s\n", (p_CHAR)msg);
		cout << szMsg;
	}
}
/* **************************************************************************** */
/* *        Set range of the convertion                                       * */
/* **************************************************************************** */

int ToBeConverted(int sym)
{
	if ((sym > 32 && sym < 128) ||
		sym == REC_Yenn_sign || sym == REC_Copyright_sign ||
		sym == REC_TradeName_sign || sym == REC_DIV_sign)
	{
		return 1;
	}
	return 0;
}

