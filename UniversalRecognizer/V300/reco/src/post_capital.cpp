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

#include "post_capital.h"
#include "post_cap_tbl.h"
#include "recodefs.h"
#include "hwr_sys.h"
#include "ams_mg.h"
#include "zctype.h"
#include "langid.h"
#include "reco.h"

enum LetterType
{
    SURE_UPPER,
    SURE_LOWER,
    AMBIGUOUS_CASE,
    UNKNOWN_LETTER
};

struct AnsScorePair
{
    _UCHAR word[w_lim + 1];
    _UCHAR* write_style_id;
    _INT weight;
    _BOOL need_modify;
    LetterType letter_type[w_lim];
};

struct AnsInfo
{
    _UCHAR* answer;
    _INT* weights;
    _UCHAR* var;
    _INT num_alts;
};

struct PostCapSession
{
    _INT lang_id;
    AnsScorePair* candidates;
};

static _INT PostGetAltNumber(_UCHAR* answer)
{
    _UCHAR* pans = answer;
    _INT num_alts = 0;
    while (*pans)
    {
        if (*pans == PM_ALTSEP)
        {
            num_alts++;
        }
        ++pans;
    }
    num_alts++;
    return num_alts;
}

static _BOOL PostBuildAnsScorePair(AnsInfo* ans_info, AnsScorePair* anscore_pair)
{
    _UCHAR* pans = ans_info->answer;
    _UCHAR* pvar = ans_info->var;
    _INT alt_id = 0;
    _UCHAR* pans_prev = pans;
    
    anscore_pair[alt_id].weight = ans_info->weights[alt_id];
    anscore_pair[alt_id].write_style_id = pvar;
    anscore_pair[alt_id].need_modify = _FALSE;
    HWRMemSet(anscore_pair[alt_id].letter_type, 0, sizeof(anscore_pair[alt_id].letter_type));
    while (*pans)
    {
        if (*pans == PM_ALTSEP)
        {
            *pans = '\0';
            HWRStrCpy((_STR)anscore_pair[alt_id].word, (_STR)pans_prev);
            pans_prev = pans + 1;
            alt_id++;
            anscore_pair[alt_id].weight = ans_info->weights[alt_id];
            anscore_pair[alt_id].write_style_id = pvar + 1;
            anscore_pair[alt_id].need_modify = _FALSE;
            HWRMemSet(anscore_pair[alt_id].letter_type, 0, sizeof(anscore_pair[alt_id].letter_type));
        }
        ++pans;
        ++pvar;
    }
    HWRStrCpy((_STR)anscore_pair[alt_id].word, (_STR)pans_prev);
    return _TRUE;
}

static LetterType PostGetLetterType(_INT lang, _UCHAR code, _UCHAR style_id)
{
    if (style_id >= AMBIGUOUS_ID)
    {
        return UNKNOWN_LETTER;
    }
    
    _LONG style_data;
    if (IsUpper(code))
    {
        style_data = ST_NONE;
        if (PostCapGetCapStyle(lang, ToLower(code), &style_data))
        {
            if (style_data & ST_AMBIGUOUS)
            {
                return AMBIGUOUS_CASE;
            }
        }
        return SURE_UPPER;
    }
    else if (IsLower(code))
    {
        style_data = ST_NONE;
        if (PostCapGetCapStyle(lang, code, &style_data))
        {
            if (style_data & ST_AMBIGUOUS)
            {
                return AMBIGUOUS_CASE;
            }
            else if ((ST_BIT(style_id) & style_data) != 0)
            {
                return SURE_UPPER;
            }
        }
        return SURE_LOWER;
    }
    
    return UNKNOWN_LETTER;
}

static _BOOL PostCheckNeedToModify(AnsScorePair* anscore_pair)
{
    _BOOL result = _FALSE;
    
    LetterType* ptype = anscore_pair->letter_type;
    _UCHAR* pstr = anscore_pair->word;
    
    //find first alphabet letter
    while (*pstr)
    {
        if (IsUpper(*pstr) || IsLower(*pstr))
        {
            break;
        }
        ++pstr;
    }
    
    _INT len = HWRStrLen((_STR)anscore_pair->word);
    //check if first letter should be uppercase
    if (IsLower(pstr[0]) && ptype[0] == SURE_UPPER)
    {
        pstr[0] = ToUpper(pstr[0]);
        result = _TRUE;
    }
    
    //check if all of remaining letters should be uppercase
    if (IsUpper(pstr[0]) || ptype[0] == AMBIGUOUS_CASE)
    {
        _INT i, num_upper = 0, num_lower =0, num_alpha = 0;
        for (i = 1; i < len; i++)
        {
            if (IsUpper(pstr[i]) || IsLower(pstr[i]))
            {
                num_alpha++;
            }
            
            if (ptype[i] == SURE_UPPER)
            {
                num_upper++;
            }
            else if (ptype[i] == SURE_LOWER)
            {
                num_lower++;
            }
            else if (ptype[i] == AMBIGUOUS_CASE)
            {
                num_upper++;
                num_lower++;
            }
        }
        
        if (num_alpha == num_upper && num_upper > num_lower)
        {
            for (i = 0; i < len; i++)
            {
                pstr[i] = ToUpper(pstr[i]);
            }
            result = _TRUE;
        }
    }
    
    return result;
}

static _BOOL PostIsCapitalLetters(_INT lang, AnsScorePair* anscore_pair)
{
    _UCHAR* pstr = anscore_pair->word;
    _UCHAR* pstyle_id = anscore_pair->write_style_id;
    LetterType* ptype = anscore_pair->letter_type;
    
    while (*pstr)
    {
        *ptype = PostGetLetterType(lang, *pstr, *pstyle_id);
        ++ptype;
        ++pstyle_id;
        ++pstr;
    }
    
    if (PostCheckNeedToModify(anscore_pair))
    {
        return _TRUE;
    }
    
    return _FALSE;
}

static _BOOL PostInsertCapitalLetters(PostCapSession* session, _INT num_pair, _INT* num_inserted)
{
    _INT i;
    AnsScorePair* pBuf = session->candidates + num_pair;
    *num_inserted = 0;
    for (i = 0; i < num_pair; i++)
    {
        *pBuf = session->candidates[i];
        if (PostIsCapitalLetters(session->lang_id, pBuf))
        {
            (*num_inserted)++;
            pBuf->need_modify = _TRUE;
            ++pBuf;
        }
    }
    return _TRUE;
}

static _BOOL PostSortAnscores(AnsScorePair* anscore_pair, _INT num_pair)
{
    _INT i, j;
    AnsScorePair tmp;
    for (i = 0; i < num_pair - 1; i++)
    {
        for (j = i + 1; j < num_pair; j++)
        {
            if (anscore_pair[i].weight < anscore_pair[j].weight ||
                (anscore_pair[i].weight == anscore_pair[j].weight &&
                 anscore_pair[i].need_modify == _FALSE && anscore_pair[j].need_modify == _TRUE))
            {
                tmp = anscore_pair[i];
                anscore_pair[i] = anscore_pair[j];
                anscore_pair[j] = tmp;
            }
        }
    }
    return _TRUE;
}

static _BOOL PostRemoveDupAns(AnsScorePair* anscore_pair, _INT num_pair, _INT* num_removed)
{
    _INT i, j;
    *num_removed = 0;
    
    for (i = 0; i < num_pair - 1; i++)
    {
        if (anscore_pair[i].weight == 0)
        {
            continue;
        }
        for (j = i + 1; j < num_pair; j++)
        {
            if (anscore_pair[j].weight == 0)
            {
                continue;
            }
            if (HWRStrCmp((_STR)anscore_pair[i].word, (_STR)anscore_pair[j].word) == 0)
            {
                anscore_pair[j].weight = 0;
                (*num_removed)++;
            }
        }
    }
    return _TRUE;
}

static _BOOL PostRebuildAnswer(AnsScorePair* anscore_pair, _INT num_pair, _UCHAR* answer, _INT* weights)
{
    _INT i, j, len;
    AnsScorePair* pair = anscore_pair;
    _UCHAR* pstr = answer;
    _INT* pweight = weights;
    
    for (i = 0; i < num_pair; i++, ++pair, ++pweight)
    {
        len = HWRStrLen((_STR)pair->word);
        *pweight = pair->weight;
        for (j = 0; j < len; j++)
        {
            *pstr = pair->word[j];
            ++pstr;
        }
        
        if (i < num_pair - 1)
        {
            *pstr = PM_ALTSEP;
            ++pstr;
        }
    }
    *pstr = 0;
    return _TRUE;
}

static _BOOL PostGetPostSession(PostCapSession* session, _INT num_alts, _INT lang )
{
    session->lang_id = lang;
    session->candidates = (AnsScorePair*)HWRMemoryAlloc(sizeof(AnsScorePair) * num_alts * 2);
    if (session->candidates == _NULL)
    {
        return _FALSE;
    }
    return _TRUE;
}

static void PostReleasePostSession(PostCapSession* session)
{
    if (session->candidates)
    {
        HWRMemoryFree(session->candidates);
        session->candidates = _NULL;
    }
}

_BOOL PostConfirmCapitalLetters(_UCHAR* answer, _INT* weights, _UCHAR* var, _INT lang )
{
    _INT num_alts = PostGetAltNumber(answer);
    if (num_alts == 0)
    {
        return _TRUE;
    }
    
    PostCapSession session = {0, };
    if (!PostGetPostSession(&session, num_alts, lang ))
    {
        return _FALSE;
    }
    
    _UCHAR* answer_buf = (_UCHAR*)HWRMemoryAlloc(sizeof(_UCHAR) * HWRStrLen((_STR)answer));
    if (answer_buf == NULL)
    {
        PostReleasePostSession(&session);
        return _FALSE;
    }
    
    HWRStrCpy((_STR)answer_buf, (_STR)answer);
    AnsInfo ans_info;
    ans_info.answer = answer_buf;
    ans_info.weights = weights;
    ans_info.var = var;
    ans_info.num_alts = num_alts;
    
    PostBuildAnsScorePair(&ans_info, session.candidates);
    // AnsScorePair (* pat)[20] = (AnsScorePair (*)[20])session.candidates; //for debug
    
    _INT num_inserted = 0;
    PostInsertCapitalLetters(&session, num_alts, &num_inserted);
    
    if (num_inserted > 0)
    {
        _INT num_pair = num_alts + num_inserted;
        _INT num_removed;
        PostSortAnscores(session.candidates, num_pair);
        PostRemoveDupAns(session.candidates, num_pair, &num_removed);
        if (num_removed > 0)
        {
            PostSortAnscores(session.candidates, num_pair);
            num_pair -= num_removed;
        }
        
        PostRebuildAnswer(session.candidates, num_pair, answer, weights);
    }
    
    HWRMemoryFree(answer_buf);
    PostReleasePostSession(&session);
    return _TRUE;
}
