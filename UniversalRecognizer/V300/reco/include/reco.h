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

#ifndef PR_HWREC_H_INCLUDED
#define PR_HWREC_H_INCLUDED

#include "reco_ver.h"
#include "recodefs.h"
#include "bastypes.h"
#include "RecognizerApi.h"

typedef int  (__cdecl *info_func_type)(void *);		/* Type of the callback returning parent status */

typedef struct
{                                           /* Control structure for initializing recognition */
	int    flags;							/* place for the HW_RECFL_ flags */
	int    sp_vs_q;							/* Parameter of speed-quality tradeof */
	RECOHDICT h_main_dict;					/* Handle to user dictionary loaded by Dictionary functions */
	RECOHDICT h_user_dict;					/* Handle to user dictionary loaded by Dictionary functions */
	info_func_type InfoCallBack;			/* CallBack for interrupting recognizer operation */
	char *	customPunct;					/* custom punctuation characters */
	char *	customNumbers;					/* custom numbers */
	void *	ICB_param;						/* Parameter, which will be passed to InfoCallBack */
} RECO_control_type, * p_RECO_control_type;

typedef struct
{
	short  x;
	short  y;
} RECO_point_type, * p_RECO_point_type;

typedef struct
{                                           /* Baseline definition for SymbRecognize */
	int    size;
	int    base;
} RECO_baseline_type, * p_RECO_baseline_type;

typedef union _reco_answer
{
    UCHR * word;
    int * array;
    int value;
} RECO_ANSWER;

typedef struct
{
    int    capabilities;                    /* Bits (HW_CPFL_) describing what type of recognizer it is */
    _CHAR  id_string[HW_RECID_MAXLEN];      /* Name of the recognizer */
} RECO_ID_type, * p_RECO_ID_type;

/* ------------------------- Prototypes ------------------------------------- */

#ifdef __cplusplus
extern "C"
{
#endif
		
// --------------- Recognition API functions ----------------------------------- 

int      RecoGetRecID(p_RECO_ID_type p_inf);
void	 RecoSetErrorText( CUCHR * pErrorText );

RECOCTX  RecoCreateContext( _INT language );
int      RecoCloseContext(RECOCTX context);

int      RecoOpenSession(p_RECO_control_type ctrl, RECOCTX context);
int      RecoCloseSession(RECOCTX context);

int      RecoRecognize(int npoints, p_RECO_point_type strokes, RECOCTX context);
int      RecoRecognizeSymbol(int npoints, p_RECO_point_type strokes, p_RECO_baseline_type baseline, RECOCTX context);

int		 RecoGetRecLangID( RECOCTX context );

RECO_ANSWER  RecoGetAnswers(int what, int nw, int na, RECOCTX context);

// -------------- Dictionary functions -----------------------------------------
 
int      RecoLoadDict( const char * store, RECOHDICT *phDict);
int      RecoSaveDict( char * store, RECOHDICT h_dict);
int      RecoFreeDict(RECOHDICT * h_dict);
int      RecoGetDictStatus(int * plen, RECOHDICT h_dict);
int      DictEnumWords( RECOHDICT hDict, PRECO_ONGOTWORD fnCallback, void *pParam );

int      RecoSpellCheckWord( CUCHR * wrd, UCHR * ans, int buf_len, RECOHDICT h_main_dict, RECOHDICT h_user_dict, int flags );
int      RecoAddWordToDict( CUCHR * word, RECOHDICT * h_dict);
int      RecoCheckWordInDicts( CUCHR * word, RECOHDICT h_main_dict, RECOHDICT h_user_dict);
int      RecoAddWordToDictA( const char * word, RECOHDICT * h_dict);
    
int      RecoSaveDictToFile( const char * name, RECOHDICT h_dict);
int      RecoLoadDictFromFile(const char *name, RECOHDICT *phDict);
    
// -------------- Letter Shape selector functions ------------------------------

int      RecoGetSetPicturesWeights(int operation, void * buf, RECOCTX context); // Private API call

// -----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif


#endif /* PR_HWREC_H_INCLUDED */
/* ************************************************************************** */
/* *   Head functions of Embedded recognizer header end                     * */
/* ************************************************************************** */

