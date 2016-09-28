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

#include "elk.h"
#include "PegRec.h"



#ifdef PEG_RECINT_UNICODE
#define UNICODE_CONVERT
int     StrToUNICODE(UCHR * tstr, char * str, int cMax);
int     UNICODEtoStr(char * str, UCHR * tstr, int cMax);
UCHR    CharToUNICODE(char ch);
#endif //  

//#include "xrwdict.h"
#define XRWD_MIDWORD 2

int PegCheckWordInDict(char *inp_word, void *h_dict);

/* ************************************************************************** */
/* *  Load user dictionary from memory image or create a new one            * */
/* ************************************************************************** */
int DLLEXP CgrLoadDict(char * store, CGRHDICT *phDict)
{

#ifdef PEGDICT
	if (store == _NULL)
	{
		return ElkCreateDict(phDict);
	}
	else
	{
		return ElkLoadDict((p_UCHAR)store, phDict);
	}
#else
	return 1;
#endif
}

/* ************************************************************************** */
/* *  Save dictionary to memory image                                       * */
/* ************************************************************************** */
int  DLLEXP CgrSaveDict(char * store, CGRHDICT h_dict)
{
#ifdef PEGDICT
	if (h_dict == _NULL || store == _NULL)
	{
		goto err;
	}

	return ElkSaveDict((p_UCHAR)store, h_dict);
err:
#endif
	return 1;
}

/* ************************************************************************** */
/* *  Free dictionary    memory image                                       * */
/* ************************************************************************** */
int  DLLEXP CgrFreeDict(CGRHDICT * h_dict)
{
#ifdef PEGDICT
	if (*h_dict == _NULL)
	{
		goto err;
	}

	return ElkFreeDict(h_dict);
err:
#endif
	return 1;
}

/* ************************************************************************** */
/* *  Get user dictionary length and changed status                         * */
/* ************************************************************************** */
int  DLLEXP CgrGetDictStatus(int * plen, CGRHDICT h_dict)
{
#ifdef PEGDICT
	if (h_dict == _NULL)
	{
		goto err;
	}

	return ElkGetDictStatus(plen, h_dict);
err:
#endif
	return 0;
}

/* ************************************************************************** */
/* *  Adds word to user dictionary                                          * */
/* ************************************************************************** */
#ifdef PEGDICT
#ifdef PEG_RECINT_UNICODE
int  DLLEXP CgrAddWordToDict(UCHR * word, CGRHDICT * h_dict)
{
	_INT i;
	_UCHAR wrd[ELK_MAX_WORDLEN+4];

#ifdef UNICODE_CONVERT
	i = UNICODEtoStr((char *)&wrd[0], word, ELK_MAX_WORDLEN+1);
#else
	for (i = 0; i < ELK_MAX_WORDLEN && word[i] != 0; i ++)
	{
		wrd[i] = (_UCHAR)(word[i]&0xFF);
	}
	wrd[i] = 0;
#endif //ifdef UNICODE_CONVERT

#else
int  DLLEXP CgrAddWordToDict(char * word, CGRHDICT * h_dict)
{
	p_UCHAR wrd = (p_UCHAR)word;
#endif

	if (*h_dict == _NULL)
	{
		goto err;
	}
	if (HWRStrLen((_STR)wrd) >= ELK_MAX_WORDLEN)
	{
		goto err;
	}

	return !(ElkAddWord(wrd, 0, h_dict));

err:
	return 1;
}
#else // PEGDICT 
int  DLLEXP CgrAddWordToDict(char * word, CGRHDICT * h_dict)
{
	return 1;
}
#endif // PEGDICT

/* ************************************************************************** */
/* *  Check if word is present in any dictionary                            * */
/* ************************************************************************** */
#ifdef PEGDICT
#ifdef PEG_RECINT_UNICODE
int  DLLEXP CgrCheckWordInDicts(UCHR * word, CGRHDICT h_main_dict, CGRHDICT h_user_dict)
{
	_INT i;
	int res = 1;
	_CHAR wrd[ELK_MAX_WORDLEN+4];

#ifdef UNICODE_CONVERT
	i = UNICODEtoStr((char *)&wrd[0], word, ELK_MAX_WORDLEN+1);
#else
	for (i = 0; i < ELK_MAX_WORDLEN && word[i] != 0; i ++)
	{
		wrd[i] = (_UCHAR)(word[i]&0xFF);
	}
	wrd[i] = 0;
#endif //ifdef UNICODE_CONVERT
#else
int  DLLEXP CgrCheckWordInDicts(char * word, CGRHDICT h_main_dict, CGRHDICT h_user_dict)
{
	int res = 1;
	p_CHAR wrd = (p_CHAR)word;
#endif // #if PEG_RECINT_UNICODE 

	if (h_user_dict != NULL)
	{
		res = PegCheckWordInDict(wrd, h_user_dict);
	}
	if (res && h_main_dict != NULL)
	{
		res = PegCheckWordInDict(wrd, h_main_dict);
	}

	return res;
}

#else // PEGDICT 

#ifdef PEG_RECINT_UNICODE
int  DLLEXP CgrCheckWordInDicts(UCHR * word, CGRHDICT h_main_dict, CGRHDICT h_user_dict)
{
	return 1;
}
#else
int  DLLEXP CgrCheckWordInDicts(char * word, CGRHDICT h_main_dict, CGRHDICT h_user_dict)
{
	return 1;
}
#endif // #if PEG_RECINT_UNICODE 

#endif // PEGDICT


#ifndef _PSION_DLL
/* ************************************************************************** */
/* *  Spell check word to dictionaries and provide list of alternatives     * */
/* ************************************************************************** */
#ifdef PEGDICT

int DLLEXP CgrSpellCheckWord(UCHR * wrd, UCHR * ans, int buf_len, CGRHDICT h_main_dict, CGRHDICT h_user_dict, int flags)

#ifdef PEG_RECINT_UNICODE
{
	int i, j, k, l, m, n, ll, u;
	_UCHAR * ptr;
	_INT   weights[PEG_MAX_SPELL_NUM_ALTS];
	//  _UCHAR answ[NUM_RW*ELK_MAX_WORDLEN];
	//  p_rec_inst_type pri = (p_rec_inst_type)(context);
	spc_answer_type u_list, m_list;
	_UCHAR  a[PEG_MAX_SPELL_NUM_ALTS*ELK_MAX_WORDLEN+PEG_MAX_SPELL_NUM_ALTS];
	_UCHAR * answ = &a[0];
	_UCHAR word[ELK_MAX_WORDLEN+4];


	if (ans  == _NULL)
	{
		goto err;
	}
	if (wrd  == _NULL)
	{
		goto err;
	}
	if (buf_len < ELK_MAX_WORDLEN)
	{
		goto err;
	}

#ifdef UNICODE_CONVERT
	i = UNICODEtoStr((char *)&word[0], &wrd[0], ELK_MAX_WORDLEN+1);
#else
	for (i = 0; i < ELK_MAX_WORDLEN && wrd[i] != 0; i ++)
	{
		word[i] = (_UCHAR)(wrd[i]&0xFF);
	}
	word[i] = 0;
#endif //ifdef UNICODE_CONVERT

	if (word[0] == 0)
	{
		goto err;
	}

#else
{
	int i, j, k, l, m, n, ll, u;
	_UCHAR * ptr;
	_INT   weights[PEG_MAX_SPELL_NUM_ALTS];
	//  _UCHAR answ[NUM_RW*ELK_MAX_WORDLEN];
	//  p_rec_inst_type pri = (p_rec_inst_type)(context);
	spc_answer_type u_list, m_list;
	p_UCHAR answ = (p_UCHAR)ans;
	p_UCHAR word = (p_UCHAR)wrd;

	if (ans  == _NULL)
	{
		goto err;
	}
	if (wrd  == _NULL)
	{
		goto err;
	}
	if (word[0] == 0)
	{
		goto err;
	}
	if (buf_len < ELK_MAX_WORDLEN)
	{
		goto err;
	}
#endif


	u_list.nansw = m_list.nansw = 0;

	//  PegCleanUpContext(pri);

	// --------------------- Get alternatives from both dicts ----------------------

	if (h_user_dict != NULL)
	{
		SpellCheckWord((p_UCHAR)word, &u_list, h_user_dict, flags);
	}
	if (h_main_dict != NULL)
	{
		SpellCheckWord((p_UCHAR)word, &m_list, h_main_dict, flags);
	}

	// --------------------- Store answers -----------------------------------------

	//  HWRMemSet(answ, 0, buf_len);
	l = HWRStrLen((_STR)word)+1;
	if (l >= ELK_MAX_WORDLEN)
	{
		goto err;
	}
	else
	{
		HWRStrCpy((_STR)answ, (_STR)word);
	}

	for (i = 1, j = k = 0; i < PEG_MAX_SPELL_NUM_ALTS;)
	{
		m = u = 0;
		ptr = 0;
		if (j < u_list.nansw)
		{
			u = 1;    // Try user
		}
		if (k < m_list.nansw)
		{
			m = 1;    // Try main
		}
		if (m && u)  if (u_list.weights[j] > m_list.weights[k])
			{
				u = 0;
			}
			else
			{
				m = 0;
			}

		if (u)
		{
			weights[i] = 100 - u_list.weights[j];
			ptr = (p_UCHAR)&u_list.list[j++][0];
		}
		if (m)
		{
			weights[i] = 100 - m_list.weights[j];
			ptr = (p_UCHAR)&m_list.list[k++][0];
		}

		if (ptr == 0)
		{
			break;
		}

		for (n = ll = 0; n < i; n ++) // Search for duplicates
		{
			if (HWRStrCmp((_STR)&answ[ll], (_STR)ptr) == 0)
			{
				break;
			}
			else
			{
				ll += HWRStrLen((_STR)&answ[ll])+1;
			}
		}

		if (n == i)
		{
			n = HWRStrLen((_STR)ptr)+1;
			if (l+n >= buf_len)
			{
				break;
			}
			HWRStrCpy((_STR)&answ[l], (_STR)ptr);
			l += n;
			i ++;
		}
	}

	answ[l++] = 0;

#ifdef PEG_RECINT_UNICODE
#ifdef UNICODE_CONVERT
	//StrToUNICODE(ans, (char *)answ, buf_len);
	for (i = 0; i < buf_len && i < l; i ++)
	{
		ans[i] = CharToUNICODE(answ[i]);
	}
	ans[buf_len-1] = 0;
#else
	for (i = 0; i < buf_len && i < l; i ++)
	{
		ans[i] = (UCHR)answ[i];
	}
	ans[buf_len-1] = 0;
#endif //ifdef UNICODE_CONVERT
#endif

	//  for (i = 0; i < l-1; i ++) if (answ[i] == 0) answ[i] = PM_ALTSEP;
	//  PegAddToAnsw(pri, answ, weights, 0, 0);

	return 0;

err:
	return 1;
}

#else // PEGDICT 

#ifdef PEG_RECINT_UNICODE
int  DLLEXP CgrSpellCheckWord(UCHR * wrd, UCHR * ans, int buf_len, CGRHDICT h_main_dict, CGRHDICT h_user_dict, int flags)
{
	return 1;
}
#else // PEG_RECINT_UNICODE 
int  DLLEXP CgrSpellCheckWord(char * wrd, char * ans, int buf_len, CGRHDICT h_main_dict, CGRHDICT h_user_dict, int flags)
{
	return 1;
}
#endif // PEG_RECINT_UNICODE 
#endif // PEGDICT
#endif /* _PSION_DLL */


// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

#ifdef PEGDICT
/* ************************************************************************** */
/* *  Check if word is present in given dictionary with some smarts         * */
/* ************************************************************************** */
int PegCheckWordInDict(char *inp_word, void *h_dict)
{
	int len, res;
	_UCHAR status, attr;
	_UCHAR word[ELK_MAX_WORDLEN];

	if ((len = HWRStrLen(inp_word)) >= ELK_MAX_WORDLEN)
	{
		goto err;
	}
	if (h_dict == NULL)
	{
		goto err;
	}

	res = ElkCheckWord((p_UCHAR)inp_word, &status, &attr, h_dict);
	if (status == XRWD_MIDWORD)
	{
		res = 1;
	}

	if (res && IsUpper(inp_word[0])) // Not found directly
	{
		HWRStrCpy((_STR)word, inp_word);
		word[0] = (_UCHAR)ToLower(word[0]);
		res = ElkCheckWord((p_UCHAR)word, &status, &attr, h_dict);
		if (status == XRWD_MIDWORD)
		{
			res = 1;
		}
	}

	// ------------ Let's remove punctuation -----------------------------------------------------

	if (res)
	{
		_INT i;
		_INT sp_len, ep_len;
		_UCHAR sp[ELK_MAX_WORDLEN];
		_UCHAR ep[ELK_MAX_WORDLEN];

		HWRStrCpy((_STR)word, (_STR)inp_word);

		for (i = sp_len = 0; i < len; i ++) if (IsPunct(word[i]))
			{
				sp[sp_len++] = word[i];
			}
			else
			{
				break;
			}
		sp[sp_len]  = 0;

		if(sp_len==len) //krav
		{
			goto err;    //krav
		}

		for (i = len-1, ep_len = 0; i >= 0; i --) if (IsPunct(word[i]))
			{
				ep[ep_len++] = word[i];
			}
			else
			{
				break;
			}
		ep[ep_len]  = 0;
		if (ep_len)
		{
			HWRStrRev((_STR)ep);
		}

		if (ep_len + sp_len == 0)
		{
			goto err;
		}

		if (sp_len)
		{
			HWRMemCpy((_STR)&word[0], (_STR)&word[sp_len], len);
			len -= sp_len;
		}
		if (ep_len)
		{
			word[len-ep_len] = 0;
			len -= ep_len;
		}

		if (len <= 0)
		{
			goto err;
		}

		res = ElkCheckWord((p_UCHAR)word, &status, &attr, h_dict);
		if (status == XRWD_MIDWORD)
		{
			res = 1;
		}

		if (res && IsUpper(word[0])) // Not found directly
		{
			word[0] = (_UCHAR)ToLower(word[0]);
			res = ElkCheckWord((p_UCHAR)word, &status, &attr, h_dict);
			if (status == XRWD_MIDWORD)
			{
				res = 1;
			}
		}

	}

	return res;
err:
	return 1;
}


#if defined(REC_DLL) && defined(PEG_RECINT_UNICODE)

#define MAX_XU_CONVERTS 5

static int _iHexes[MAX_XU_CONVERTS] = {0x08a, 0x08c, 0x09a, 0x09c, 0x09f};
static int _iUnicodes[MAX_XU_CONVERTS] = {352, 338, 353, 339, 376};

/* ************************************************************************** */
/* *   Convert char string to UNICODE                                       * */
/* ************************************************************************** */
int StrToUNICODE(TCHAR * tstr, char * str, int cMax)
{
	int i, j;

	for (i = 0; i < cMax-1 && str[i] != 0; i ++)
	{
		tstr[i] = (TCHAR)(TCHAR)(unsigned char)str[i];
		if((unsigned char)str[i]>=0x08a && (unsigned char)str[i]<=0x09f)
			for (j=0; j<MAX_XU_CONVERTS; j++)
				if((unsigned char)str[i]==_iHexes[j])
				{
					tstr[i] = _iUnicodes[j];
				}
	}

	tstr[i] = 0;


	return i;
}

/* ************************************************************************** */
/* *   Convert UNICODE string to char                                       * */
/* ************************************************************************** */
int UNICODEtoStr(char * str, TCHAR * tstr, int cMax)
{
	int i, j;

	for (i = 0; i < cMax && tstr[i] != 0; i ++)
	{
		str[i] = ((unsigned char)tstr[i]);
		if(tstr[i]>=338 && tstr[i]<=376)
			for (j=0; j<MAX_XU_CONVERTS; j++)
				if(tstr[i]==_iUnicodes[j])
				{
					str[i] = (unsigned char)_iHexes[j];
				}
	}

	str[i] = 0;


	return i;
}

/* ************************************************************************** */
/* *   Convert char to UNICODE                                              * */
/* ************************************************************************** */
TCHAR CharToUNICODE(char ch)
{
	int i;
	if((unsigned char)ch>=0x08a && (unsigned char)ch<=0x09f)
		for(i=0; i<MAX_XU_CONVERTS; i++)
			if((unsigned char)ch==_iHexes[i])
			{
				return _iUnicodes[i];
			}
	return (TCHAR)(unsigned char)ch;
}


#endif // REC_DLL


#endif //   #ifdef PEGDICT
