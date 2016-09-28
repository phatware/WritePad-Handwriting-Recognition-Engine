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

/* ************************************************************************************* */
/* *    PhatWare WritePad handwriting recognition engine                               * */
/* *    Copyright (c) 1997-2014 PhatWare(r) Corp. All rights reserved.                 * */
/* ************************************************************************************* */

/* ************************************************************************************* *
 *
 * File: post_cap_tbl.cpp
 * Post processing capial letters (tables)
 *
 * Unauthorized distribution of this code is prohibited.
 * Contractor/manufacturer is PhatWare Corp.
 * 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
 *
 * ************************************************************************************* */

#include "post_cap_tbl.h"
#include "langid.h"
#include "dti.h"

struct StyleInfo
{
    _UCHAR code;
    _LONG cap_ids;
};

static const StyleInfo _international_capital_ids[] =
{
    'a', ST_BIT(4) | ST_BIT(5) | ST_BIT(6) | ST_BIT(8),
    'b', ST_BIT(0),
    'c', ST_AMBIGUOUS,
    'd', ST_BIT(1) | ST_BIT(2),
    'e', ST_BIT(0) | ST_BIT(8) | ST_BIT(10) | ST_BIT(11),
    'f', ST_BIT(6),
    'g', ST_BIT(2) | ST_BIT(3),
    'h', ST_BIT(0) | ST_BIT(1) | ST_BIT(4),
    'i', ST_BIT(2) | ST_BIT(6),
    'k', ST_AMBIGUOUS,
    'l', ST_BIT(1),
    'm', ST_BIT(3),
    'n', ST_BIT(2),
    'o', ST_AMBIGUOUS,
    'p', ST_AMBIGUOUS,
    'q', ST_BIT(1),
    'r', ST_BIT(0) | ST_BIT(1),
    's', ST_AMBIGUOUS,
    't', ST_BIT(6) | ST_BIT(8),
    'u', ST_AMBIGUOUS,
    'v', ST_AMBIGUOUS,
    'w', ST_AMBIGUOUS,
    'x', ST_AMBIGUOUS,
    'y', ST_AMBIGUOUS,
    'z', ST_AMBIGUOUS,
    REC_a_umlaut, ST_BIT(11) | ST_BIT(4) | ST_BIT(6) | ST_BIT(5),
    REC_a_angstrem, ST_BIT(9) | ST_BIT(8),
    REC_o_crossed, ST_AMBIGUOUS,
    REC_o_umlaut, ST_AMBIGUOUS,
    REC_o_circumflex, ST_AMBIGUOUS,
    REC_u_grave, ST_AMBIGUOUS,
    REC_u_umlaut, ST_AMBIGUOUS,
    REC_u_circumflex, ST_AMBIGUOUS,
    0, 0
};

static const StyleInfo _french_capital_ids[] =
{
    'c', ST_AMBIGUOUS,
    'k', ST_AMBIGUOUS,
    'o', ST_AMBIGUOUS,
    'p', ST_AMBIGUOUS,
    's', ST_AMBIGUOUS,
    'u', ST_AMBIGUOUS,
    'v', ST_AMBIGUOUS,
    'w', ST_AMBIGUOUS,
    'x', ST_AMBIGUOUS,
    'y', ST_AMBIGUOUS,
    'z', ST_AMBIGUOUS,
    REC_o_circumflex, ST_AMBIGUOUS,
    REC_o_umlaut, ST_AMBIGUOUS,
    REC_u_grave, ST_AMBIGUOUS,
    REC_u_circumflex, ST_AMBIGUOUS,
    REC_u_umlaut, ST_AMBIGUOUS,
    REC_c_cedilla, ST_AMBIGUOUS,
    0, 0
};

static const StyleInfo _german_capital_ids[] =
{
    'c', ST_AMBIGUOUS,
    'e', ST_BIT(0),
    'k', ST_AMBIGUOUS,
    'o', ST_AMBIGUOUS,
    'p', ST_AMBIGUOUS,
    's', ST_AMBIGUOUS,
    'u', ST_AMBIGUOUS,
    'v', ST_AMBIGUOUS,
    'w', ST_AMBIGUOUS,
    'x', ST_AMBIGUOUS,
    'y', ST_AMBIGUOUS,
    'z', ST_AMBIGUOUS,
    REC_o_umlaut, ST_AMBIGUOUS,
    REC_u_umlaut, ST_AMBIGUOUS,
    0, 0
};

static _BOOL PostCapSearchTable(const StyleInfo* table, _UCHAR code, _LONG* cap_ids, _INT lang )
{
    _INT rec_code = OSToRec(code, lang);
    
    const StyleInfo* item = table;
    while (item->code)
    {
        if (rec_code == item->code)
        {
            *cap_ids = item->cap_ids;
            return _TRUE;
        }
        ++item;
    }
    return _FALSE;
}

_BOOL PostCapGetCapStyle(_INT lang, _UCHAR code, _LONG* styles)
{
    _BOOL result = _FALSE;
    
    const StyleInfo* table = NULL;
    
    switch (lang)
    {
        case LANGUAGE_FRENCH :
        case LANGUAGE_ITALIAN :
        case LANGUAGE_PORTUGUESE :
            table = _french_capital_ids;
            break;
            
        case LANGUAGE_GERMAN :
            table = _german_capital_ids;
            break;
            
        case LANGUAGE_ENGLISH :
        case LANGUAGE_ENGLISHUK :
        case LANGUAGE_SPANISH :
        case LANGUAGE_DUTCH :
        case LANGUAGE_FINNISH :
        case LANGUAGE_SWEDISH :
        case LANGUAGE_NORWEGIAN :
        case LANGUAGE_DANISH :
        case LANGUAGE_INDONESIAN :
            table = _international_capital_ids;
            break;
    }
    
    if (table)
    {
        result = PostCapSearchTable(table, code, styles, lang);
    }
    return result;
}
