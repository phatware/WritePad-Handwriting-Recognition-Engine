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

#include "bastypes.h"
#include <windows.h>
#include <windowsx.h>
#ifdef _WIN32
#include <winbase.h>
#endif
#include <limits.h>
#include <string.h>
#include <stdio.h>
#include <io.h>
#include <errno.h>
//#include <ams_mg.h>
//#include <xrword.h>
#ifdef _PENWIN
#include <penwin.h>
#include <penwoem.h>
#include <avprec.h> /* additional_data_for main_buf_type and des files processing */
#else
//  #include <rec-main.h> /* additional_data_for main_buf_type and des files processing */
#endif
#include <tap.h>
#define _NOHWR
#include "deslib.h"
#include "wg_stuff.h"
#include "wgdbg.h"
//#define  OLD_RECOGNIZER 1

/* *************************************************************** */
#define DR_HEAD 0
#define DR_TAIL 1
#define DR_CRAZY 3

#define DW_HEAD 0
#define DW_TAIL 1

/* *************************************************************** */

_INT _read_des
(FILE * hf, p_DES_DATA b, _INT n, _UINT mode, _SHORT m);
_INT _write_des
(FILE * hf, p_DES_DATA b, _INT n, _UINT mode, _SHORT m);

_ULONG _item_add(_ULONG d, void _PTR p, _UINT n);
_ULONG _item_del(_ULONG d, _UINT n);
_ULONG _item_count(_ULONG d);
_UINT _item_get(_ULONG d, void _PTR p, _UINT n);
_UINT _item_seq(_ULONG d, void _PTR p);

/* *************************************************************** */
_ULONG _des_file(_INT command, _ULONG param1, _ULONG param2)
{
	_USHORT m, n;
	_INT s;
	_ULONG  u, ud, dd;
	FILE * hf;
	static DES_DATA_TYPE mb;

	switch (command)
	{
		case DES_OPEN:
			u = _NULL;
			if (_access((char*) param1, 0) == 0)
			{
				hf = fopen((char*) param1, "rb");
				if (!hf)
				{
					return _NULL;
				}
				while (_read_des(hf, &mb, sizeof(DES_DATA_TYPE), DR_HEAD, -1))
				{
					ud = _item_add(u, &mb, sizeof(DES_DATA_TYPE));
					if (ud)
					{
						u = ud;
					}
				}
				if (u)
				{
					/* count des sections */
					s = (_INT) _des_file(DES_COUNT, u, 0L)*_read_des(_NULL, _NULL, 0, DR_CRAZY, 0);
					dd = u;
					u = _NULL;
					while (_des_file(DES_GET, dd, (_ULONG) (p_DES_DATA) &mb))
					{
						mb.ofs_of_xrattr += s;
						mb.ofs_of_vars += s;
						_read_des(hf, (p_DES_DATA) &mb, sizeof(DES_DATA_TYPE), DR_TAIL, -1);
						ud = _item_add(u, (void _PTR)&mb, sizeof(DES_DATA_TYPE));
						if (ud)
						{
							u = ud;
						}
					}
					_item_del(dd, 0);
				}
				fclose(hf);
			}
			return u;

		case DES_GET:
			return (_ULONG) _item_seq(param1, (void _PTR)param2);

		case DES_PUT:
			return _item_add(param1, (void _PTR)param2, sizeof(DES_DATA_TYPE));

		case DES_CLOSE:
			if (param2)
			{
				hf = fopen((char*) param2, "wb+");
				if (hf)
				{
					/* count des sections */
					m = (_UINT) _des_file(DES_COUNT, param1, 0L);
					n = m;
					/* reserve space for des head */
					while (m--)
					{
						_write_des(hf, (p_DES_DATA) &mb, sizeof(DES_DATA_TYPE), DW_HEAD, -1);
					}
					/* write separator */
					m = 0;
					fwrite(&m, sizeof(_USHORT), 1, hf);
					while (_des_file(DES_GET, param1, (_ULONG) (p_DES_DATA) &mb))
					{
						s = _write_des(hf, (p_DES_DATA) &mb, sizeof(DES_DATA_TYPE), DW_TAIL, m);
						if (s != 0)
						{
							mb.num = m + 1;
							mb.ofs_of_xrattr -= n*s;
							mb.ofs_of_vars -= n*s;
							_write_des(hf, (p_DES_DATA) &mb, sizeof(DES_DATA_TYPE), DW_HEAD, m);
						}
						m++;
					}
					fclose(hf);
				}
			}
			_item_del(param1, 0);
			if (hf == NULL)
			{
				return FALSE;
			}
			return TRUE;

		case DES_COUNT:
			return _item_count(param1);

		default:
			break;
	}
	return 0L;
}

/* *************************************************************** */
_INT _read_des
(FILE * hf, p_DES_DATA b, _INT n, _UINT mode, _SHORT m)
{
	p_UCHAR p;
	switch (mode)
	{
		case DR_HEAD:
			if (!hf)
			{
				return 0;
			}
			n = _read_des(_NULL, _NULL, 0, DR_CRAZY, 0);
			if (m == -1)
			{
				if (fread(b, n, 1, hf) != 1)
				{
					return 0;
				}
				if (b->num)
				{
					return n;
				}
			}
			return 0;
		case DR_TAIL:
			if (!hf)
			{
				return 0;
			}
			fseek(hf, b->ofs_of_xrattr, SEEK_SET);
#if ! OLD_RECOGNIZER
			for (m = 0; m < LAB_XRINP_SIZE; m++)
			{
				if (fread(&(b->xrinp[m]), sizeof(DES_XRNODE_TYPE), 1, hf) != 1)
				{
					return 0;
				}
				if (b->xrinp[m].dxrid == 0)
				{
					break;
				}
			}
#else
			for (m = 0; m < XRINP_SIZE; m++)
			{
				if (fread(&(b->xrinp[m]), sizeof(xrvoc_type), 1, hf) != 1)
				{
					return 0;
				}
				if (b->xrinp[m].dxrid == 0)
				{
					break;
				}
			}
#endif
			fseek(hf, b->ofs_of_vars, SEEK_SET);
			for (m = 0; m < b->num_of_vars; m++)
			{
				p = b->w_vars[m];
				do
				{
					//          *p = (_UCHAR)fgetc(hf);
					if (fread(p, sizeof(_UCHAR), 1, hf) != 1)
					{
						return 0;
					}
				}
				while (*p != 0 && p++);
			}
			return n;
		case DR_CRAZY:
			/* some crazy or stupid code */
			/* because one upon a time XR was an 2-BYTES long*/
			return (sizeof(DES_DATA_TYPE) - sizeof(DES_VARS_TYPE) - LAB_XRINP_SIZE * 2);

	}
	return 0;
}
/* *************************************************************** */
_INT _write_des(FILE * hf, p_DES_DATA b, _INT n, _UINT mode, _SHORT m)
{
	_INT  bm;
	_LONG fl;

	bm = _read_des(_NULL, _NULL, 0, DR_CRAZY, 0);
	switch (mode)
	{
		case DW_HEAD:
			if (!hf)
			{
				return 0;
			}
			if (m == -1)
			{
				/* sequential write */
				if (fwrite(b, bm, 1, hf) != 1)
				{
					return 0;
				}
				else
				{
					return (_INT) n;
				}
			}
			else
			{
				/* random write */
				fseek(hf, 0, SEEK_END);
				fl = ftell(hf);
				if (fl < (bm*m))
				{
					/* additional space required */
					fl = bm*m - fl + 1;
					if (fwrite(&bm, sizeof(_UCHAR), (size_t) fl, hf) != (size_t) fl)
					{
						return 0;
					}
				}
				else
				{
					fseek(hf, bm*m, SEEK_SET);
				}
				if (fwrite(b, bm, 1, hf) != 1)
				{
					return 0;
				}
				else
				{
					return (_INT) n;
				}
			}
		case DW_TAIL:
			if (!hf)
			{
				return 0;
			}
			b->num = m + 1;
			fseek(hf, 0, SEEK_END);
			fl = ftell(hf);
			for (m = 0; m < LAB_XRINP_SIZE; m++)
			{
				if (fwrite(&(b->xrinp[m]), sizeof(DES_XRNODE_TYPE), 1, hf) != 1)
				{
					return 0;
				}
				if (b->xrinp[m].dxrid == 0)
				{
					break;
				}
			}
			*(_LONG _PTR)&(b->ofs_of_xrattr) = fl;
			fseek(hf, 0, SEEK_END);
			fl = ftell(hf);
			for (m = 0; m < b->num_of_vars; m++)
			{
				if (fwrite(b->w_vars[m], strlen((char*) (b->w_vars[m])) + 1, 1, hf) != 1)
				{
					return 0;
				}
			}
			*(_LONG _PTR)&(b->ofs_of_vars) = fl;
			return bm;
	}
	return 0;
}

/* *************************************************************** */

typedef struct _LIKEMEM
{
	_ULONG m_next;
	_ULONG m_prev;
	_UINT  m_items;
	_UINT  m_icount;
	_UINT  m_isize;
	_UINT  m_flag;
} LIKEMEM;
typedef LIKEMEM _PTR pLIKEMEM;

#define _86SEGSIZE 0xfff0

/* *************************************************************** */
_ULONG _item_add(_ULONG d, void _PTR p, _UINT n)
{
	_UINT m;
	pLIKEMEM l;
	_ULONG z;
	p_UCHAR u;

	if (!p)
	{
		return _NULL;
	}
	if (!d)
	{
		m = (_86SEGSIZE - sizeof(LIKEMEM)) / n;
		if (m == 0)
		{
			return _NULL;
		}
		d = (_ULONG) DebugAllocPtr(GPTR, (m*n) + sizeof(LIKEMEM), "_item_add32");
		if (!d)
		{
			return _NULL;
		}
		((pLIKEMEM) d)->m_next = _NULL;
		((pLIKEMEM) d)->m_prev = _NULL;
		((pLIKEMEM) d)->m_items = m;
		((pLIKEMEM) d)->m_icount = 0;
		((pLIKEMEM) d)->m_isize = n;
		((pLIKEMEM) d)->m_flag = 0;
	}
	l = (pLIKEMEM) d;
	while (l->m_next != _NULL)
	{
		l = (pLIKEMEM) l->m_next;
	}
	if (l->m_icount == l->m_items)
	{
		m = (_86SEGSIZE - sizeof(LIKEMEM)) / n;
		if (m == 0)
		{
			return _NULL;
		}
		z = (_ULONG) DebugAllocPtr(GPTR, (m*n) + sizeof(LIKEMEM), "_item_add32 2");
		if (!z)
		{
			return _NULL;
		}
		((pLIKEMEM) z)->m_next = _NULL;
		((pLIKEMEM) z)->m_prev = (_ULONG) l;
		((pLIKEMEM) z)->m_items = m;
		((pLIKEMEM) z)->m_icount = 0;
		((pLIKEMEM) z)->m_isize = n;
		((pLIKEMEM) z)->m_flag = 0;
		l->m_next = z;
		l = (pLIKEMEM) z;
	}
	u = (p_UCHAR) l;
	u += sizeof(LIKEMEM);
	m = l->m_icount;
	while (m--)
	{
		u += l->m_isize;
	}
	memcpy(u, p, n);
	l->m_icount++;
	return d;
}


/* *************************************************************** */
_ULONG _item_del(_ULONG d, _UINT n)
{
	_ULONG l;

	while (d != _NULL)
	{
		l = ((pLIKEMEM) d)->m_next;
		DebugFreePtr((void _PTR)d, "_item_del32");
		d = l;
	}
	return _NULL;
}

/* *************************************************************** */
_ULONG _item_count(_ULONG d)
{
	_ULONG m;

	m = 0;
	while (d != _NULL)
	{
		m += ((pLIKEMEM) d)->m_icount;
		d = ((pLIKEMEM) d)->m_next;
	}
	return m;
}

/* *************************************************************** */
_UINT _item_get(_ULONG d, void _PTR p, _UINT n)
{
	_UINT m;
	pLIKEMEM l;
	p_UCHAR u;

	if (!d)
	{
		return 0;
	}
	m = 0;
	do
	{
		if (m <= n && n < (m + ((pLIKEMEM) d)->m_icount))
		{
			l = (pLIKEMEM) d;
			u = (p_UCHAR) l;
			u += sizeof(LIKEMEM);
			while (n--)
			{
				u += l->m_isize;
			}
			if (p)
			{
				memcpy(p, u, l->m_isize);
			}
			return l->m_isize;
			break;
		}
		m += ((pLIKEMEM) d)->m_icount;
		d = ((pLIKEMEM) d)->m_next;
	}
	while (d != _NULL);
	return 0;
}

/* *************************************************************** */
_UINT _item_seq(_ULONG d, void _PTR p)
{
	_UINT m;
	pLIKEMEM l;

	if (!d)
	{
		return 0;
	}
	l = (pLIKEMEM) d;
	if (!(m = _item_get(d, p, l->m_flag++)))
	{
		l->m_flag = 0;
	}
	return m;
}

/* *************************************************************** */
_ULONG _tap_file(_UINT command, _ULONG param1, _ULONG param2, _UINT n)
{
	_TAPCOUNT tc;
	_INT npwb, npwe, i, np;

	if (param1 == 0L)
	{
		return 0L;
	}
	memset(&tc, 0, sizeof(_TAPCOUNT));
	switch (command)
	{
		//??SD looks like not used code
		case TAP_COUNT:
			TapCount((_HTAP) param1, &tc, TAP_MODE_PAGE);
			return tc.nWords;
		case TAP_CMP:
			if (!TapSeek((_HTAP) param1, 0, TAP_MODE_PAGE | TAP_SEEK_SET))
			{
				return 0L;
			}
			if (TapCount((_HTAP) param1, &tc, TAP_MODE_PAGE) == RC_TAP_ERROR)
			{
				return 0L;
			}
			np = tc.nPages;
			npwb = 0;
			npwe = 0;
			for (i = 0; i < np; i++)
			{
				if (!TapSeek((_HTAP) param1, i, TAP_MODE_PAGE | TAP_SEEK_SET))
				{
					return 0L;
				}
				if (TapCount((_HTAP) param1, &tc, TAP_MODE_WORD) == RC_TAP_ERROR)
				{
					return 0L;
				}
				npwe += tc.nWords;
				if (npwb <= (_INT) n && (_INT) n <= npwe)
				{
					if (TapSeek((_HTAP) param1, n - npwb, TAP_MODE_WORD | TAP_SEEK_SET))
					{
						*((p_UCHAR) param2) = 0;
						if (param2)
						{
							TapGetText((_HTAP) param1, (p_CHAR) param2, UCHAR_MAX);
						}
						return (_ULONG) strlen((p_CHAR) param2);
					}
					else
					{
						return 0L;
					}
				}
				npwb += npwe;
			}
			return 0L;
		case TAP_TRACE:
			TapCount((_HTAP) param1, &tc, TAP_MODE_PAGE);
			return 0L;
	}
	return 0L;
}
