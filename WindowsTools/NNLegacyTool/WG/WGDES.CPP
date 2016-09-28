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

#define STRICT
#define _REQ_WIN
#include <windows.h>
#include <windowsx.h>
#ifndef _PENWIN
#include "pensub.h32"
#else
#include <penwin.h>
#include <penwoem.h>
#include <bastypes.h>
#include <hwr_sys.h>
#include <ams_mg.h>
#include <xrword.h>
#include <learn.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <io.h>
#ifdef _PENWIN
#include <avprec.h> /* additional_data_for DES_DATA_TYPE and des files processing */
#endif

#include <bastypes.h>
#include <wg_stuff.h>
#include "wggbl.h"
#include "wgidm.h"
#include "wgtrc.h"
#include "wgtap.h"
#include "wgmsg.h"
#include "wgbat.h"
#include "wgdbg.h"
#include "wgdes.h"
#include "wggbl.h"
#define _NOHWR
#include "deslib.h"

#define     MAX_TAPWORDS   70

#ifdef _PENWIN
int    word_descriptor(p_DES_DATA buf, int w_num, int command);
#endif
#if _HWR_NEW
_SHORT StoreAliases(p_CHAR mem, p_USHORT am_loc, p_ALS_NODE /*p_aliased_vars_type*/ av);
#endif
void   err_msg(p_CHAR str);
int    SaveAWLFile(p_DES_DATA desbuf, int count, char * name);



char *ver_h_DES = "WritePad Recognizer Comparator";
char def_aliases_ext [] = ".als";
//char file_name[_MAX_PATH];

DES_DATA_TYPE       buf;
p_DES_DATA          lpBuf = _NULL;
DES_DATA_TYPE       local_buf;
DES_DATA_TYPE(_PTR cache_buf) [];
LPSTR               lpChn = _NULL;
int                 nWords = 0;
int                 nCount = 0;

p_CHAR              aliases_mem = _NULL;       /* Allocated array for Xr aliases */
_USHORT             aliases_mem_loc = 0;       /* Location in this array */

//??SD
FILE  _PTR dbuf = NULL, *afile = _NULL;
int  num_descriptors = 0;
char descr_fname[_MAX_PATH];
int  cb_first = 0;
int  cb_last = 0;
long cache_buf_size = 0;

_SHORT num_word = 0;              // Number of word in file
_SHORT new_file = 1;


/************************************************************************/
BOOL FAR desSaveDESFile(void)
{
	int       n, result = TRUE;
	int       strokes_count;
	_ULONG    d, dd;
#if _HWR_NEW
	static char     buff[_MAX_PATH];
#endif

	bSaveDesFile = FALSE;
	strokes_count = 0;
	d = 0L;
	for (n = 0; n < nCount; n++)
	{
		lpBuf[n].page = n / 8;
		lpBuf[n].line = n % 8;
		lpBuf[n].word = 0;
		lpBuf[n].s_stroke_num = strokes_count;
		strokes_count += lpBuf[n].num_strokes;
		dd = _des_file(DES_PUT, d, (_ULONG) &lpBuf[n]);
		if (dd)
		{
			d = dd;
		}
	}
	result = _des_file(DES_CLOSE, d, (_ULONG) (LPSTR) descr_fname);

	SaveAWLFile(lpBuf, nCount, descr_fname);

	DebugFreePtr(lpBuf, "WGDES desSaveDESFile");
	lpBuf = _NULL;

#if DEBUG_CHUNK_PROCESSOR
	desSaveChunkDataInDesFile(descr_fname);
	DebugFreePtr(lpChn, "WGDES desSaveDESFile");
	lpChn = _NULL;
#endif
	// save alias file
#if _HWR_NEW
	if (gPenLabBatchMode)
	{
		//if (!batSaveAliasFile())
		//   goto out;
	}
	lstrcpy(buff, (LPSTR) descr_fname/*file_name*/);
	if (_fstrrchr(buff, '.') != NULL)
	{
		*(_fstrrchr(buff, '.')) = 0;
	}
	lstrcat(buff, def_aliases_ext);

	if (aliases_mem_loc > 0 && (afile = fopen(buff, "wb")) != NULL)
	{
		fwrite(aliases_mem, 1, aliases_mem_loc, afile);
		fclose(afile);
	}
#endif // _HWR_NEW
	if (aliases_mem != NULL)
	{
		DebugFreePtr(aliases_mem, "WGDES desSaveDESFile");
		aliases_mem = NULL;
	}
	return result;
} /* end of desSaveDESFile */

/************************************************************************/
BOOL FAR desSaveRCresult(void FAR *lpRCResult)
{
#ifdef _PENWIN
	LPRCRESULT          lpRC = (LPRCRESULT)lpRCResult;
	p_shipped_data_type s_data = (p_VOID)lpRC->syg.lpsye->lRecogVal;

	if (nCount == MAX_TAPWORDS)
	{
		return FALSE;
	}
	_fmemcpy((p_VOID)&(lpBuf[nCount]), (p_VOID)s_data->buf, sizeof(buf));
	nCount++;
#if _HWR_NEW
	StoreAliases(aliases_mem, &aliases_mem_loc, s_data->a_vars);
#endif
	return TRUE;
} /* end of desSaveRCresult */
#else
	rec_info_type FAR *lpRC = (rec_info_type FAR *)lpRCResult;

	if (nCount == MAX_TAPWORDS)
	{
		return FALSE;
	}
	_fmemcpy((p_VOID) &(lpBuf[nCount]), (p_VOID) lpRC->pmbuf, sizeof(DES_DATA_TYPE));
	nCount++;
#if _HWR_NEW
	StoreAliases(aliases_mem, &aliases_mem_loc, (p_ALS_NODE) lpRC->pavars);
#endif
	return TRUE;
} /* end of desSaveRCresult */
#endif
/************************************************************************/
BOOL FAR desPrepareBuffer(LPSTR DesFileName)
{
	char szTapName[_MAX_PATH];
	int  i;

	tapGetTapFileName(hLastTAPWnd, szTapName, &nWords);
	batGetDesFileName(descr_fname, szTapName);
	lstrcpy(DesFileName, descr_fname);
	i = sizeof(DES_DATA_TYPE);
	lpBuf = (p_DES_DATA)
	        DebugAllocPtr(GHND, (long)sizeof(DES_DATA_TYPE) * MAX_TAPWORDS, "WGDES desPrepareBuffer");
	if (lpBuf == NULL)
	{
		return FALSE;
	}

#if DEBUG_CHUNK_PROCESSOR
	lpChn =
	    DebugAllocPtr(GHND, (long)sizeof(CHUNK_DES_LINE_TYPE) * MAX_TAPWORDS + sizeof(CHUNK_DES_HEAD_TYPE), "WGDES desPrepareBuffer");
	if (lpChn == NULL)
	{
		DebugFreePtr(lpBuf, "WGDES desPrepareBuffer");
		return FALSE;
	}
#endif
#if _HWR_NEW
	aliases_mem = (p_CHAR) DebugAllocPtr(GHND, MAX_ALIASES_MEM, "WGDES desPrepareBuffer");
	if (aliases_mem == NULL)
	{
		DebugFreePtr(lpBuf, "WGDES desPrepareBuffer");
		lpBuf = _NULL;
		return FALSE;
	}
#endif //_HWR_NEW
	//??SD
	dbuf = NULL;
	cb_first = 0;
	cb_last = 0;
	cache_buf_size = 0;
	num_word = 0;
	new_file = 1;
	aliases_mem_loc = 0;
	nCount = 0;
	return TRUE;
} /* end of desPrepareBuffer */

/************************************************************************/
void err_msg_win(p_CHAR str)
{
	MessageBox(NULL, (LPSTR) str, "Low Level Processing error", MB_ICONEXCLAMATION);
}

#ifdef _PENWIN
#if _HWR_NEW
/************************************************************************/
int  word_descriptor(p_DES_DATA buf, int w_num, int command)
{
	/* Return current number of descriptors */
	int     i,j,l;
	int     main_block_size;
	int     attrib;
	main_block_size = sizeof(DES_DATA_TYPE)-sizeof(w_vars_type)-XRINP_SIZE*2;

	switch (command)
	{
		case NEW:
		{
			command = NEW;
			goto save_cache;
save_cache_new_ret:

			num_descriptors = 0;
			strcpy(descr_fname, file_name);
			if (strrchr(descr_fname, '.') != NULL)
			{
				*(strrchr(descr_fname, '.')) = 0;
			}
			strcat(descr_fname, ".des");
			if (dbuf != NULL)
			{
				fclose(dbuf);
			}

			if ((dbuf = fopen(descr_fname,"wb")) != NULL)
			{
				fputc(0, dbuf);
				fputc(0, dbuf);                                     /* Ending marker */
				if (cache_buf != NULL)
				{
					HWRMemoryFree(cache_buf);
					cache_buf = NULL;
				}
				cb_first = 0;
				cb_last = 0;
				fclose(dbuf);
				if ((dbuf = fopen(descr_fname,"rb+w")) == NULL)
				{
					goto errors;
				}
				local_buf.num = -1;
			}
			else
			{
				goto errors;
			}
			break;
		}

		case ADD:                              /* Add new descriptor to the file */
		{
			goto save_cache;
save_cache_add_ret:

			if (dbuf == NULL)
			{
				goto errors;
			}

			fseek(dbuf, (long)main_block_size*(long)num_descriptors, SEEK_SET);
			num_descriptors ++;
			buf->num = num_descriptors;
			memmove(&local_buf, buf, sizeof(local_buf));         /* Put in cache */
			fwrite(buf, 1, (size_t)main_block_size, dbuf);
			fputc(0, dbuf);                                     /* Ending marker */
			fputc(0, dbuf);                                     /* Ending marker */

			local_buf.num = -1;
			break;
		}

		case PUT:
		{
			if (dbuf == NULL)
			{
				goto errors;
			}
			if (abs(w_num) > num_descriptors)
			{
				goto errors;
			}

			buf->num = w_num;
			memcpy(&local_buf, buf, sizeof(DES_DATA_TYPE));

			if (w_num == 0)
			{
				break;
			}

			if (cache_buf != NULL && w_num != 0 &&
			        w_num >= cb_first && w_num <= cb_last)           /* In page cache ? */
			{
				for (i = 0; i < cb_last-cb_first+1; i ++)
				{
					if (w_num == (*cache_buf)[i].num)
					{
						goto put_cache;
					}
				}
				goto errors;

put_cache:
				attrib  = 0;
				if (memcmp(buf, &(*cache_buf)[i], main_block_size) != 0)
				{
					attrib |= 0x0004;
				}
				if (memcmp(buf->xrinp, (*cache_buf)[i].xrinp, XRINP_SIZE*sizeof(xrinp_type)) != 0)
				{
					attrib |= 0x0008;
				}
				if (memcmp(buf->w_vars, (*cache_buf)[i].w_vars, sizeof(w_vars_type)) != 0)
				{
					attrib |= 0x0010;
				}
				if (attrib & 0x001c)                          /* something changed */
				{
					attrib |= ((*cache_buf)[i].attrib & 0x001c);
					buf->attrib |= attrib;
					memcpy(&(*cache_buf)[i], buf, sizeof(DES_DATA_TYPE)); /* Write to cache  */
				}
				break;
			}

			fseek(dbuf, 0, SEEK_END);
			buf->ofs_of_xrattr = ftell(dbuf) - (long)main_block_size * (long)num_descriptors;
			local_buf.ofs_of_xrattr = buf->ofs_of_xrattr;
			fseek(dbuf, 0, SEEK_END);

			for (i = 0; i < XRINP_SIZE; i ++)
			{
				fwrite(&local_buf.xrinp[i], sizeof(xrinp_type), 1, dbuf);
				if (local_buf.xrinp[i].xr == 0)
				{
					break;
				}
			}

			fseek(dbuf, 0, SEEK_END);
			buf->ofs_of_vars = ftell(dbuf) - (long)main_block_size * (long)num_descriptors;
			local_buf.ofs_of_vars = buf->ofs_of_vars;
			for (i = 0; i < local_buf.num_of_vars && i < 10; i ++)
			{
				fprintf(dbuf, "%s%c", local_buf.w_vars[i],0);
			}

			fseek(dbuf, (long)main_block_size*(long)(w_num-1), SEEK_SET);
			fwrite(&local_buf, 1, (size_t)main_block_size, dbuf);

			break;
		}
		case GET:
		{
			if (dbuf == NULL)
			{
				goto errors;
			}
			if (abs(w_num) > num_descriptors)
			{
				goto errors;
			}

			if (w_num == local_buf.num)
			{
				memmove(buf, &local_buf, sizeof(DES_DATA_TYPE)); /* Get from STORE */
				break;
			}

			if (w_num == 0)
			{
				memset(buf, 0, sizeof(DES_DATA_TYPE));
				goto err;
			}
			if (cache_buf != NULL && w_num != 0 &&
			        w_num >= cb_first && w_num <= cb_last)           /* In page cache ? */
			{
				for (i = 0; i < cb_last-cb_first+1; i ++)
				{
					if (w_num == (*cache_buf)[i].num)
					{
						goto get_cache;
					}
				}
				goto errors;

get_cache:
				memcpy(buf, &(*cache_buf)[i], sizeof(DES_DATA_TYPE)); /* Get from cache  */
				break;
			}

			if (w_num == 0)
			{
				memset(buf, 0, sizeof(DES_DATA_TYPE));
				goto errors;
			}
			memset(buf, 0, sizeof(DES_DATA_TYPE));

			fseek(dbuf, (long)main_block_size*(long)(w_num-1), SEEK_SET);
			fread(buf, 1, (size_t)main_block_size, dbuf);

			fseek(dbuf, (long)main_block_size*(long)num_descriptors +
			      buf->ofs_of_xrattr, SEEK_SET);

			for (j = 0; j < XRINP_SIZE; j ++)
			{
				fread(&buf->xrinp[j], sizeof(xrinp_type), 1, dbuf);
				if (buf->xrinp[j].xr == 0)
				{
					break;
				}
			}

			fseek(dbuf, (long)main_block_size*(long)num_descriptors +
			      buf->ofs_of_vars, SEEK_SET);

			for (i = 0; i < buf->num_of_vars && i < 10; i ++)
			{
				for (j=0; j < w_lim && (l = fgetc(dbuf)) > 0; j++)
				{
					buf->w_vars[i][j] = (uchar)l;
				}
				buf->w_vars[i][j] = 0;                        /* Put condom on it */
			}

			memmove(&local_buf, buf, sizeof(DES_DATA_TYPE));    /* Put to cache */

			break;
		}
		case CLOSE:
		{
			goto save_cache;
save_cache_close_ret:

			if (dbuf != NULL)
			{
				fclose(dbuf);
				dbuf = NULL;
			}
			break;
		}

		default:
		{
			goto errors;
		}
	}
	return num_descriptors;
errors:
	err_msg_win("Word Descriptor:\nSome error found!");
err:
	return 0;

	/* -------------------- Read Cache ------------------------------------------ */
	/* -------------------- Save Cache ------------------------------------------ */
save_cache:

	if (cache_buf != NULL)
	{
		for (i = 0; i <= cb_last-cb_first; i ++)        /* Put Cache info in f.  */
		{
			if ((*cache_buf)[i].attrib & 0x0008)               /* Was not changed */
			{
				fseek(dbuf, 0, SEEK_END);
				(*cache_buf)[i].ofs_of_xrattr = ftell(dbuf) - (long)main_block_size * (long)num_descriptors;
				fseek(dbuf, 0, SEEK_END);

				//       fprintf(dbuf,"%s%c%s%c", (*cache_buf)[i].xrinp,0,(*cache_buf)[i].attrinp,0);

				for (j = 0; j < XRINP_SIZE; j ++)
				{
					fwrite(&(*cache_buf)[i].xrinp[j], sizeof(xrinp_type), 1, dbuf);
					if ((*cache_buf)[i].xrinp[j].xr == 0)
					{
						break;
					}
				}

			}
			if ((*cache_buf)[i].attrib & 0x0010)               /* Was not changed */
			{
				fseek(dbuf, 0, SEEK_END);
				(*cache_buf)[i].ofs_of_vars = ftell(dbuf) - (long)main_block_size * (long)num_descriptors;
				for (j = 0; j < (*cache_buf)[i].num_of_vars && j < 10; j ++)
				{
					fprintf(dbuf, "%s%c", (*cache_buf)[i].w_vars[j],0);
				}
			}
		}                                              /* Put standart info */
		for (i = 0; i <= cb_last-cb_first; i ++)        /* Put Cache info in f.  */
		{
			if (!((*cache_buf)[i].attrib & 0x001c))
			{
				continue;    /* Was not changed */
			}

			(*cache_buf)[i].attrib &= 0xffe3;                          /* Reset "changed" bit */

			fseek(dbuf, (long)main_block_size*(long)((*cache_buf)[i].num-1), SEEK_SET);
			fwrite(&(*cache_buf)[i], 1, (size_t)main_block_size, dbuf);
		}
	}
	switch (command)
	{
		case NEW:
			goto save_cache_new_ret;
		case ADD:
			goto save_cache_add_ret;
		case CLOSE:
			goto save_cache_close_ret;
	}

	return 0;
}
#endif /* _HWR_NEW */
#endif //ifndef _WIN32

/************************************************************************ */
#if _HWR_NEW


typedef struct
{
	_CHAR    id_string[4];           // ID - must be ALIS
	_SHORT   num_entries;            // Number of words in file
	_CHAR    creator_signature[40];  // Some descr string
	_LONG    reserved[10];           // For future use
} aliases_file_hdr_type, *p_aliases_file_hdr_type;

typedef ALS_NODE_TYPE   aliased_vars_type;
typedef p_ALS_NODE      p_aliased_vars_type;

_SHORT StoreAliases(p_CHAR mem, p_USHORT am_loc, p_aliased_vars_type av)
{
	_SHORT                  i, j, len;
	_USHORT                 loc;
	p_aliases_file_hdr_type af_hdr;

	loc = *am_loc;
	af_hdr = (p_aliases_file_hdr_type) mem;

	num_word++;

	if (loc == 0 && new_file)
	{
		num_word = 0;
		new_file = 0;
	}

	if (av->alias[0] == 0)
	{
		goto err;    // Nothing written
	}

	if (loc > MAX_ALIASES_MEM - sizeof(aliased_vars_type))
	{
		goto err;
	}

	if (loc == 0)                                       // First entry in file
	{
		strcpy(af_hdr->id_string, "ALIS");
		strcpy(af_hdr->creator_signature, ver_h_DES);
		af_hdr->num_entries = 0;

		loc += sizeof(aliases_file_hdr_type);
		new_file = 1;
	}

	len = strlen((_STR) av->aword);

	*((p_SHORT) (mem + loc)) = num_word;
	loc += sizeof(_SHORT) + 1;

	_fmemcpy(mem + loc, av->aword, len);
	loc += len + 1;
	_fmemcpy(mem + loc, av->alias, len);
	loc += len + 1;

	for (i = 0; i < LAB_XRINP_SIZE && av->axrinp[i].dxrid != 0; i++)// Write XrInp string
	{
		*((DES_XRNODE_TYPE*) (mem + loc)) = av->axrinp[i];
		loc += sizeof(DES_XRNODE_TYPE);
	}
	if (i < LAB_XRINP_SIZE)
	{
		loc += sizeof(DES_XRNODE_TYPE);
	}

	for (i = 0; i < len; i++)                          // Write XrVoc vars
	{
		for (j = 0; j < LAB_XR_SIZE && av->axrdte[i][j].dxrid != 0; j++)
		{
			*((DES_XRNODE_TYPE*) (mem + loc)) = av->axrdte[i][j];
			loc += sizeof(DES_XRNODE_TYPE);
		}
		if (j < LAB_XR_SIZE)
		{
			loc += sizeof(DES_XRNODE_TYPE);
		}
	}

	*am_loc = loc;
	af_hdr->num_entries++;

	return 0;
err:
	return 1;
}
#endif // _HWR_NEW
/*************************************************************************** */
#if DEBUG_CHUNK_PROCESSOR
void FAR desStoreChunkData(p_LAB_CHUNK_DATA lpChunk)
{
	p_CHUNK_DES_HEAD_TYPE   lpHead;
	CHUNK_DES_LINE_TYPE     Line;
	p_CHUNK_DES_LINE_TYPE   lpLine;
	if (lpChn != NULL)
	{
		_fmemset(&Line, 0, sizeof(CHUNK_DES_LINE_TYPE));
		lstrcpy(Line.szAnswer, lpChunk->szAnswer);
		Line.fIsNumb = lpChunk->fIsNumber;
		Line.nSymb   = lstrlen(Line.szAnswer);
		lpHead = (p_CHUNK_DES_HEAD_TYPE)lpChn;
		lpLine = (p_CHUNK_DES_LINE_TYPE)(lpChn + sizeof(CHUNK_DES_HEAD_TYPE));
		lpLine += lpHead->nLines;
		_fmemcpy(lpLine, &Line, sizeof(CHUNK_DES_LINE_TYPE));
		lpHead->nLines += 1;
	}
} /* end of desStoreChunkData */

/**************************************************************************** */
void FAR desSaveChunkDataInDesFile(LPSTR DesFileName)
{
	p_CHUNK_DES_HEAD_TYPE   lpHead;
	HFILE                   hFile;
	LONG                    Offset;
	DES_DATA_TYPE           buf;

	hFile = _lopen(DesFileName, READ_WRITE);
	if (hFile != HFILE_ERROR && lpChn != NULL)
	{
		Offset = _llseek(hFile, 0, 2);
		lpHead = (p_CHUNK_DES_HEAD_TYPE)lpChn;
		_lwrite(hFile, lpChn,
		        sizeof(CHUNK_DES_HEAD_TYPE) + lpHead->nLines*sizeof(CHUNK_DES_LINE_TYPE));
		_llseek(hFile, 0, 0);
		_lread(hFile, &buf, sizeof(DES_DATA_TYPE));
		buf.ams2 = LOWORD(Offset);
		_llseek(hFile, 0, 0);
		_lwrite(hFile, (LPCSTR)&buf, sizeof(DES_DATA_TYPE));
		_lclose(hFile);
	}
} /* end of desSaveChunkDataInDesFile */

#endif //DEBUG_CHUNK_PROCESSOR

int  SaveAWLFile(p_DES_DATA desbuf, int count, char * name)
{
	int i, n;
	char * p;
	char fname[128];
	char cmpw[128];
	FILE * file;

	strcpy(fname, name);
	if ((p = strrchr(fname, '.')) != 0)
	{
		*p = 0;
	}
	strcat(fname, ".awl");

	if ((file = fopen(fname, "wt")) == NULL)
	{
		goto err;
	}

	for (n = 0; n < count; n++)
	{
		tapGetCMPWordByNum(hLastTAPWnd, (LPSTR) cmpw, n);
		fprintf(file, "%-10s /", cmpw);

		for (i = 0; i < 5 && i < desbuf[n].num_of_vars && desbuf[n].w_vars_weight[i] > 0; i++)
		{
			fprintf(file, "/ %s / %d ", desbuf[n].w_vars[i], desbuf[n].w_vars_weight[i]);
		}
		fprintf(file, "\n");
	}

	fclose(file);

	return 0;
err:
	return 1;
}


