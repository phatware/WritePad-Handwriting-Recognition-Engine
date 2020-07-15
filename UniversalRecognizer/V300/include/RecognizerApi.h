/***************************************************************************************
 *
 *  WRITEPAD(r): Handwriting Recognition Engine (HWRE) and components.
 *  Copyright (c) 2001-2017 PhatWare (r) Corp. All rights reserved.
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

#ifndef __RecognizerAPI_h__
#define __RecognizerAPI_h__

#include "gestures.h"
#include "recotypes.h"

#define RECMODE_GENERAL			0          // Normal recognition -- all sybols allowed
#define RECMODE_CAPS			1          // All recognized text converted to capitals
#define RECMODE_NUM				2          // Numeric and Lex DB recognition mode
#define RECMODE_WWW				3          // internet address mode
#define RECMODE_NUMBERSPURE		4		   // pure numeric mode, no alpha or punctuation, recognizes 0123456789 only
#define RECMODE_CUSTOM			5		   // custom charset for numbers and punctuation, no alpha
#define RECMODE_ALPHAONLY		6		   // Alpha characters only, no punctuation or numbers
#define RECMODE_INVALID			(-1)

#define MAX_TRACE_LENGTH		4096
#define TRACE_BREAK_LENGTH		200

#define MAX_STRING_BUFFER		2048

#define FLAG_SEPLET				0x00000001
#define FLAG_USERDICT			0x00000002
#define FLAG_MAINDICT			0x00000004
#define FLAG_ONLYDICT			0x00000008
#define FLAG_STATICSEGMENT		0x00000010
#define FLAG_SINGLEWORDONLY		0x00000020
#define FLAG_INTERNATIONAL		0x00000040
#define FLAG_SUGGESTONLYDICT	0x00000080
#define FLAG_ANALYZER			0x00000100
#define FLAG_CORRECTOR			0x00000200
#define FLAG_SPELLIGNORENUM		0x00000400
#define FLAG_SPELLIGNOREUPPER	0x00000800
#define FLAG_NOSINGLELETSPACE	0x00001000
#define FLAG_ENABLECALC			0x00002000
#define FLAG_NOSPACE			0x00004000
#define FLAG_ALTDICT			0x00008000
#define FLAG_USECUSTOMPUNCT		0x00010000
#define FLAG_SMOOTHSTROKES      0x00020000

#define FLAG_ERROR				0xFFFFFFFF

#define READ_FLAG                   0x01
#define MEM_STREAM_FLAG             0x02
#define INK_FMT_MASK                0x3C

#define INK_RAW                     0x01
#define INK_CALCOMP                 0x02
#define INK_PWCOMP                  0x03
#define INK_JPEG                    0x04
#define INK_DATA                    0x05
#define INK_PNG                     0x06

#define IGNORE_LAST_STROKE          0x0001000
#define SORT_STROKES                0x0002000
#define SAVE_PRESSURE				0x0004000

#define MAKE_READ_FMT( dwDataFmt, bMemStream )  ( ((dwDataFmt) << 2L) | ((bMemStream)?MEM_STREAM_FLAG:0) | READ_FLAG )
#define MAKE_WRITE_FMT( dwDataFmt, bMemStream ) ( ((dwDataFmt) << 2L) | ((bMemStream)?MEM_STREAM_FLAG:0) )
#define INK_DATA_FMT( dwFlags )                 ( ((dwFlags) & INK_FMT_MASK) >> 2L )
#define INK_READ( dwFlags )                     ( (dwFlags) & READ_FLAG )
#define INK_WRITE( dwFlags )                    ( ((dwFlags) & READ_FLAG) == 0 )
#define IS_MEM_STREAM( dwFlags )                ( (dwFlags) & MEM_STREAM_FLAG )
#define IS_FILE_STREAM( dwFlags )               ( ((dwFlags) & MEM_STREAM_FLAG) == 0 )

/* ------------------------- Defines ---------------------------------------- */


#define HW_RECINT_ID_001        0x01000002  /* Rec Interface ID */
#define HW_MAX_SPELL_NUM_ALTS    10            /* How many variants will be out by the SpellCheck func */
#define HW_RECID_MAXLEN            32            /* Max length of the RecID string */
#define HW_MAX_FILENAME            128            /* Limit for filename buffer */

// Recognizer Control Falgs
#define HW_RECFL_NSEG            0x0001      /* Do not perform segmentation at all*/
#define HW_RECFL_NCSEG            0x0002      /* Do not allow segm not waiting for final stroke. (No results on the go) */
#define HW_RECFL_TTSEG            0x0004      /* Perform read-ahead of tentative segmented words */
#define HW_RECFL_INTL_CS        0x0010      /* Enables international charsets */
#define HW_RECFL_ALPHAONLY        0x0020      /* Enables international charsets */
#define HW_RECFL_CUSTOM_WITH_ALPHA    0x0040    /* Alpha with custom punctuation */
#define HW_RECFL_SEPLET            0x0100      /* Enables separate letter mode */
#define HW_RECFL_DICTONLY        0x0200      /* Restricts dictionary words only recognition */
#define HW_RECFL_NUMONLY        0x0400      /* NUMBERS only  */
#define HW_RECFL_CAPSONLY        0x0800      /* CAPITALS only */
#define HW_RECFL_PURE            0x1000      /* NUMBERS and CAPITALS modes do not use any other chars */
#define HW_RECFL_INTERNET        0x2000      /* Internet address mode */
#define HW_RECFL_STATICSEG        0x4000        /* Static segmentation */
#define HW_RECFL_CUSTOM            0x8000        /* use custom charset */

// Bits of recognizer capabilities

#define HW_CPFL_CURS            0x0001      /* Cursive capable */
#define HW_CPFL_TRNBL            0x0002      /* Training capable */
#define HW_CPFL_SPVSQ            0x0004      /* Speed VS Quality control capable */
#define HW_CPFL_INTER            0x0008      /* International support capable */

#define HW_MAXWORDLEN            50            /* maximum word length */

#define HW_SPELL_CHECK            0x0000      /* SpellCheck flag: do spell checking */
#define HW_SPELL_LIST            0x0001      /* SpellCheck flag: list continuations */
#define HW_SPELL_USERDICT        0x0002        /* SpellCheck flag: use user dictionary */
#define HW_SPELL_USEALTDICT        0x0004        /* SpellCheck flag: use alternative dictionary */
#define HW_SPELL_IGNORENUM        0x0008      /* SpellCheck flag: ignore words containing numbers */
#define HW_SPELL_IGNOREUPPER    0x0010      /* SpellCheck flag: ignore words in UPPER case */

#define HW_NUM_ANSWERS            1            /* Request to get number of recognized words */
#define HW_NUM_ALTS                2            /* Request number of alternatives for given word */
#define HW_ALT_WORD                3            /* Requestto get pointer to a given word alternative */
#define HW_ALT_WEIGHT            4            /* Request to get weight of a give word alternative */
#define HW_ALT_NSTR                5            /* Request to get number of strokes used for a given word alternative */
#define HW_ALT_STROKES            6            /* Request to get a pointer to a given word alternative stroke ids */

#define MIN_RECOGNITION_WEIGHT  51            /* Minimum recognition quality */
#define MAX_RECOGNITION_WEIGHT  100            /* Maximum recognition quality */
#define AVE_RECOGNITION_WEIGHT    ((MIN_RECOGNITION_WEIGHT+MAX_RECOGNITION_WEIGHT)/2)

#define LRN_WEIGHTSBUFFER_SIZE    448
#define LRN_SETDEFWEIGHTS_OP    0            /* LEARN interface commands for RecoGetSetPictWghts func */
#define LRN_GETCURWEIGHTS_OP    1
#define LRN_SETCURWEIGHTS_OP    2

#define PM_ALTSEP               1            /* Recognized word list alternatives separator */
#define PM_LISTSEP              2            /* Recognized word list wordlist separator */
#define PM_LISTEND              0            /* Recognized word list end */

#define PM_NUMSEPARATOR            (-1)

#endif 



