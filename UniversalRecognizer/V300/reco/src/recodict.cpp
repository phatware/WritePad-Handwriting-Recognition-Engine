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

#include <stdlib.h>
#include <string.h>

#include "hwr_sys.h"
#include "zctype.h"

#include "ams_mg.h"
#include "xrwdict.h"

#include "reco.h"
#include "pydict.h"

#define XRWD_MIDWORD 2

int HWR_CheckWordInDict(char *inp_word, void *h_dict);

// -------------------- Defines and globals --------------------------

/* ************************************************************************* */
/* *  Load user dictionary from memory image or create a new one           * */
/* ************************************************************************* */

int RecoLoadDict(const char *store, RECOHDICT *phDict)
{
#ifdef RECODICT
	if (store == _NULL)
		return PZDictCreateDict(phDict);
	else
		return PZDictLoadDict((p_UCHAR) store, phDict);
#else
	return PYDICT_ERR;
#endif
}

int RecoLoadDictFromFile(const char *name, RECOHDICT *phDict)
{
#ifdef RECODICT
	if ( name == _NULL )
		return PZDictCreateDict( phDict );
	else
		return PYDictLoadDictFromFile((p_UCHAR)name, phDict);
#else
	return PYDICT_ERR;
#endif
}

/* ************************************************************************** */
/* *  Save dictionary to memory image                                       * */
/* ************************************************************************** */

int RecoSaveDict(char * store, RECOHDICT h_dict)
{
#ifdef RECODICT
	if (h_dict == _NULL || store == _NULL)
		goto err;

	return PZDictSaveDict((p_UCHAR) store, h_dict);
err:
#endif
	return PYDICT_ERR;
}

int RecoSaveDictToFile( const char * name, RECOHDICT h_dict )
{
#ifdef RECODICT
	if (h_dict == _NULL || name == _NULL)
		goto err;
    
	return PYDictSaveDictToFile((p_UCHAR)name, h_dict);
err:
#endif
	return PYDICT_ERR;
}

/* ************************************************************************** */
/* *  Free dictionary    memory image                                       * */
/* ************************************************************************** */
int RecoFreeDict(RECOHDICT * h_dict)
{
#ifdef RECODICT
	if (*h_dict == _NULL)
		goto err;

	return PZDictFreeDict(h_dict);
	err:
#endif
	return PYDICT_ERR;
}

/* ************************************************************************** */
/* *  Get user dictionary length and changed status                         * */
/* ************************************************************************** */
int RecoGetDictStatus(int * plen, RECOHDICT h_dict)
{
#ifdef RECODICT
	if (h_dict == _NULL)
		goto err;

	return PZDictGetDictStatus(plen, h_dict);
	err:
#endif
	return 0;
}

/* ************************************************************************** */
/* *  Adds word to user dictionary                                          * */
/* ************************************************************************** */
#ifdef RECODICT

#if HW_RECINT_UNICODE

int RecoAddWordToDict(CUCHR * word, RECOHDICT * h_dict)
{
	char  wrd[PZDICT_MAX_WORDLEN+4];

#ifdef UNICODE_CONVERT
	UNICODEtoStr( wrd, word, PZDICT_MAX_WORDLEN+1);
#else
	_INT i;
	for (i = 0; i < PZDICT_MAX_WORDLEN && word[i] != 0; i ++)
        wrd[i] = (_UCHAR)(word[i]&0xFF);
    wrd[i] = 0;
#endif //ifdef UNICODE_CONVERT
    return RecoAddWordToDictA( wrd, h_dict );
}

#else
int RecoAddWordToDict(CUCHR * word, RECOHDICT * h_dict)
{
    return RecoAddWordToDictA( word, h_dict );
}

#endif

int RecoAddWordToDictA( const char * word, RECOHDICT * h_dict)
{
	p_UCHAR wrd = (p_UCHAR) word;

	if (*h_dict == _NULL)
		goto err;
	if (HWRStrLen((_CSTR) wrd) >= PZDICT_MAX_WORDLEN)
		goto err;

	return PZDictAddWord(wrd, 0, h_dict);
err:
	return PYDICT_ERR;
}

#else // RECODICT 
int RecoAddWordToDict( CUCHR * word, RECOHDICT * h_dict)
{
	return 1;
}
#endif // RECODICT

/* ************************************************************************** */
/* *  Check if word is present in any dictionary                            * */
/* ************************************************************************** */

#ifdef RECODICT

int RecoCheckWordInDicts( CUCHR * word, RECOHDICT h_main_dict, RECOHDICT h_user_dict)
{
#if HW_RECINT_UNICODE
	int res = 1;
	_CHAR wrd[PZDICT_MAX_WORDLEN+4];

#ifdef UNICODE_CONVERT
	UNICODEtoStr((char *)&wrd[0], word, PZDICT_MAX_WORDLEN+1);
#else
	_INT i;
	for (i = 0; i < PZDICT_MAX_WORDLEN && word[i] != 0; i ++)
		wrd[i] = (_UCHAR)(word[i]&0xFF);
	wrd[i] = 0;
#endif //ifdef UNICODE_CONVERT
#else
	int res = 1;
	p_CHAR wrd = (p_CHAR) word;
#endif // #if HW_RECINT_UNICODE 

	if (h_user_dict != NULL)
	{
		res = HWR_CheckWordInDict(wrd, h_user_dict);
	}
	if (res && h_main_dict != NULL)
	{
		res = HWR_CheckWordInDict(wrd, h_main_dict);
	}
	return res;
}

#else // RECODICT 
#if HW_RECINT_UNICODE

int RecoCheckWordInDicts( CUCHR * word, RECOHDICT h_main_dict, RECOHDICT h_user_dict)
{
	return 1;
}

#else

int RecoCheckWordInDicts( CUCHR * word, RECOHDICT h_main_dict, RECOHDICT h_user_dict)
{
	return 1;
}

#endif // #if HW_RECINT_UNICODE
#endif // RECODICT
#ifndef _PSION_DLL

/* ************************************************************************** */
/* *  Spell check word to dictionaries and provide list of alternatives     * */
/* ************************************************************************** */

#ifdef RECODICT

int RecoSpellCheckWord(CUCHR * wrd, UCHR * ans, int buf_len,
		RECOHDICT h_main_dict, RECOHDICT h_user_dict, int flags)
{
	int i, j, k, l, m, n, ll, u;
	_UCHAR * ptr;
	// _INT weights[HW_MAX_SPELL_NUM_ALTS];
	spc_answer_type u_list, m_list;

#if HW_RECINT_UNICODE 
	_UCHAR a[HW_MAX_SPELL_NUM_ALTS*PZDICT_MAX_WORDLEN+HW_MAX_SPELL_NUM_ALTS];
	_UCHAR * answ = &a[0];
	_UCHAR word[PZDICT_MAX_WORDLEN+4];

	if ( ans == _NULL )
        goto err;
	if ( wrd == _NULL )
        goto err;
	if ( buf_len < PZDICT_MAX_WORDLEN )
        goto err;

#ifdef UNICODE_CONVERT
	UNICODEtoStr((char *)&word[0], &wrd[0], PZDICT_MAX_WORDLEN+1);
#else
	for (i = 0; i < PZDICT_MAX_WORDLEN && wrd[i] != 0; i ++)
        word[i] = (_UCHAR)(wrd[i]&0xFF);
	word[i] = 0;
#endif //ifdef UNICODE_CONVERT
	if ( word[0] == 0 )
        goto err;
#else
	p_UCHAR answ = (p_UCHAR) ans;
	p_UCHAR word = (p_UCHAR) wrd;

	if (ans == _NULL)
		goto err;
	if (wrd == _NULL)
		goto err;
	if (word[0] == 0)
		goto err;
	if (buf_len < PZDICT_MAX_WORDLEN)
		goto err;
#endif

	u_list.nansw = m_list.nansw = 0;

	// HWR_CleanUpContext(pri);

	// --------------------- Get alternatives from both dicts ----------------------
	if (h_user_dict != NULL)
		SpellCheckWord((p_UCHAR) word, &u_list, h_user_dict, flags);
	if (h_main_dict != NULL)
		SpellCheckWord((p_UCHAR) word, &m_list, h_main_dict, flags);

	//  HWRMemSet(answ, 0, buf_len);
	l = HWRStrLen((_STR) word);

	// try to splity word into two
	if (l > 3 && l < PZDICT_MAX_WORDLEN - 1)
	{
		_UCHAR sz[PZDICT_MAX_WORDLEN];

		HWRStrCpy((_STR) sz, (_STR) word);

		for (i = 2; m_list.nansw < PZDICT_MAX_ALTS && i < l - 1; i++)
		{
			sz[i] = 0;

			if ((h_main_dict != NULL && HWR_CheckWordInDict((_STR) sz,
					h_main_dict) == 0) || (h_user_dict != NULL
					&& HWR_CheckWordInDict((_STR) sz, h_user_dict) == 0))
			{
				sz[i] = word[i];

				if ((h_main_dict != NULL && HWR_CheckWordInDict(
						(_STR) (sz + i), h_main_dict) == 0) || (h_user_dict
						!= NULL && HWR_CheckWordInDict((_STR) (sz + i),
						h_user_dict) == 0))
				{
					m_list.weights[m_list.nansw] = 50;
					HWRStrCpy((_STR) m_list.list[m_list.nansw], (_STR) sz);
					m_list.list[m_list.nansw][i] = ' ';
					HWRStrCpy((_STR) m_list.list[m_list.nansw] + i + 1,
							(_STR) (sz + i));
					m_list.nansw++;
				}
			}
			else
			{
				sz[i] = word[i];
			}
		}
	}

	l++;

	// --------------------- Store answers -----------------------------------------
	if (l >= PZDICT_MAX_WORDLEN)
		goto err;
	else
		HWRStrCpy((_STR) answ, (_STR) word);

	for (i = 1, j = k = 0; i < HW_MAX_SPELL_NUM_ALTS;)
	{
		m = u = 0;
		ptr = 0;
		if (j < u_list.nansw)
			u = 1; // Try user
		if (k < m_list.nansw)
			m = 1; // Try main
		if (m && u)
		{
			if (u_list.weights[j] > m_list.weights[k])
				u = 0;
			else
				m = 0;
		}
		if (u)
		{
			// weights[i] = 100 - u_list.weights[j];
			ptr = (p_UCHAR) &u_list.list[j++][0];
		}
		if (m)
		{
			// weights[i] = 100 - m_list.weights[j];
			ptr = (p_UCHAR) &m_list.list[k++][0];
		}

		if (ptr == 0)
			break;

		for (n = ll = 0; n < i; n++) // Search for duplicates
		{
			if (HWRStrCmp((_STR) &answ[ll], (_STR) ptr) == 0)
				break;
			else
				ll += HWRStrLen((_STR) &answ[ll]) + 1;
		}

		if (n == i)
		{
			n = HWRStrLen((_STR) ptr) + 1;
			if (l + n >= buf_len)
				break;
			HWRStrCpy((_STR) &answ[l], (_STR) ptr);
			l += n;
			i++;
		}
	}

	answ[l++] = 0;

#ifdef UNICODE_CONVERT
	for (i = 0; i < buf_len && i < l; i ++)
	{
		if ( answ[i] == 0 )
		ans[i] = PM_ALTSEP;
		else
		ans[i] = CharToUNICODE(answ[i]);
	}
	ans[i] = 0;
#else
	for (i = 0; i < buf_len && i < l; i++)
	{
		if (answ[i] == 0)
			ans[i] = PM_ALTSEP;
		else
			ans[i] = (UCHR) answ[i];
	}
	ans[i] = 0;
#endif //ifdef UNICODE_CONVERT

	return 0;

	err:
    return 1;
}

#else // RECODICT 

int RecoSpellCheckWord( CUCHR * wrd, UCHR * ans, int buf_len, RECOHDICT h_main_dict, RECOHDICT h_user_dict, int flags)
{
	return 1;
}

#endif // RECODICT
#endif /* _PSION_DLL */

// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef RECODICT
/* ************************************************************************** */
/* *  Check if word is present in given dictionary with some smarts         * */
/* ************************************************************************** */
    
int HWR_CheckWordInDict(char *inp_word, void *h_dict)
{
	int len, res;
	_UCHAR status, attr;
	_UCHAR word[PZDICT_MAX_WORDLEN];

	if ((len = HWRStrLen(inp_word)) >= PZDICT_MAX_WORDLEN)
		goto err;
	if (h_dict == NULL)
		goto err;

	res = PZDictCheckWord((p_UCHAR) inp_word, &status, &attr, h_dict);
	if (status == XRWD_MIDWORD)
		res = 1;

	if (res && IsUpper(inp_word[0])) // Not found directly
	{
		HWRStrCpy((_STR) word, inp_word);
		word[0] = (_UCHAR) ToLower(word[0]);
		res = PZDictCheckWord((p_UCHAR) word, &status, &attr, h_dict);
		if (status == XRWD_MIDWORD)
			res = 1;
	}

	// ------------ Let's remove punctuation -----------------------------------------------------

	if (res)
	{
		_INT i;
		_INT sp_len, ep_len;
		_UCHAR ep[PZDICT_MAX_WORDLEN];

		HWRStrCpy((_STR) word, (_STR) inp_word);

		for (i = sp_len = 0; i < len; i++)
		{
			if (IsPunct(word[i]))
				sp_len++;
			else
				break;
		}

		if (sp_len == len)
			goto err;

		for (i = len - 1, ep_len = 0; i >= 0; i--)
		{
			if (IsPunct(word[i]))
				ep[ep_len++] = word[i];
			else
				break;
		}
		ep[ep_len] = 0;
		if (ep_len)
			HWRStrRev((_STR) ep);

		if (ep_len + sp_len == 0)
			goto err;

		if (sp_len)
		{
			HWRMemCpy((_STR) &word[0], (_STR) &word[sp_len], len);
			len -= sp_len;
		}
		if (ep_len)
		{
			word[len - ep_len] = 0;
			len -= ep_len;
		}

		if (len <= 0)
			goto err;

		res = PZDictCheckWord((p_UCHAR) word, &status, &attr, h_dict);
		if (status == XRWD_MIDWORD)
			res = 1;

		if (res && IsUpper(word[0])) // Not found directly
		{
			word[0] = (_UCHAR) ToLower(word[0]);
			res = PZDictCheckWord((p_UCHAR) word, &status, &attr, h_dict);
			if (status == XRWD_MIDWORD)
				res = 1;
		}

	}

	return res;
	err: return 1;
}

/* ************************************************************************** */
/* *   EnumerateWords                                                       * */
/* ************************************************************************** */

#define MAXNEXTSYMS		256

bool EnumerateWords(PRECO_ONGOTWORD fnCallback, void *pParam, int depth,
		int &nWordCount, unsigned char *szWord, _INT fbn, p_fw_buf_type fwb,
		RECOHDICT hDict)
{
	int i, fbnNew;
	p_fw_buf_type fwbNew;

	if ((depth + 1) >= PZDICT_MAX_WORDLEN)
		return false;

	for (i = 0; i < fbn; i++)
	{
		szWord[depth] = fwb[i].sym;
		szWord[depth + 1] = 0;

		if (fwb[i].l_status >= XRWD_WORDEND)
		{
			//we have the next word.  Let's add it to the listbox:
			if (fnCallback)
			{
#if HW_RECINT_UNICODE
                UCHR * chr = new UCHR[HWRStrLen((char *)szWord)+2];
                StrToUNICODE( chr, (const char *)szWord, HWRStrLen((char *)szWord)+1);
				if (!fnCallback((const UCHR *) chr, pParam))
					return false;
                delete [] chr;
#else
				if (!fnCallback((const UCHR *) szWord, pParam))
					return false;
#endif // HW_RECINT_UNICODE
			}
			nWordCount++;
		}

		if (fwb[i].l_status < XRWD_BLOCKEND)
		{
			fwbNew = fwb + MAXNEXTSYMS;
			fbnNew = PZDictGetNextSyms(&fwb[i], fwbNew, hDict);
			if (!EnumerateWords(fnCallback, pParam, depth + 1, nWordCount, szWord, fbnNew, fwbNew, hDict))
				return false;
		}
	}
	return true;
}

int DictEnumWords(RECOHDICT hDict, PRECO_ONGOTWORD fnCallback, void *pParam)
{
	int fbn;
	fw_buf_type *pfwb = NULL;
	unsigned char szWord[PZDICT_MAX_WORDLEN];
	int nWordCount = 0;

	pfwb = (fw_buf_type *) malloc(MAXNEXTSYMS * sizeof(fw_buf_type) * (PZDICT_MAX_WORDLEN + 1));

	if (pfwb != NULL)
	{
		fbn = PZDictGetNextSyms( 0, pfwb, hDict );
		EnumerateWords(fnCallback, pParam, 0, nWordCount, szWord, fbn, pfwb, hDict );
		free((void *) pfwb);
	}
	return nWordCount;
}

#endif //   #ifdef RECODICT
