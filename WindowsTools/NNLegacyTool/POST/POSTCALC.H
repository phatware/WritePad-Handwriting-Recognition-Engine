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

#ifndef POSTCALC_H
#define POSTCALC_H

#if !defined(DTE_EDITOR)
#define DTE_EDITOR 0
/* this macro was added by Andrew Guryanov to mark changes
   in compilation of PDF system for DTE editor */
#if DTE_EDITOR
#define USE_POSTPROC 1
#endif /*DTE_EDITOR*/
#endif /*DTE_EDITOR*/

#if (DTE_EDITOR==0)
#include "ams_mg.h"
#include "xrword.h"
#endif /*DTE_EDITOR */

#include "pdf_file.h"

/*  Set to 1 for the output of calculations results to .PRS-file.  */
#define POSTCALC_FILE_RESULTS  (1 && PG_DEBUG)

/*  Set to 1 for the verbose output of calculations results  */
/* to .PRS-file (with all details of rules calculating).     */
#define POSTCALC_FILE_VERBOSE  1

/*  Set to 1 if you want to output the real letters from CMP, */
/* which made the part of the trajectory we consider.         */
/*  WARNING: This setting  affects  the  trace  calculations: */
/* there's another word in graph.                             */
#define POSTCALC_FILE_CMP_LETTERS    1

/*  Set to 1 if you want to switch the postprocessing learning */
/* on. Right now, it is possible only if you have the POSTCALC */
/* _FILE_CMP_LETTERS switch turned to 1;  later it may  become */
/* possible to use this switch independently.                  */
#define POST_LEARNING_ON         0
#if !POSTCALC_FILE_CMP_LETTERS
#undef POST_LEARNING_ON
#define POST_LEARNING_ON         0
#endif  /*  POSTCALC_FILE_CMP_LETTERS  */

/*  If we are going to use the upper level at all.  */
#define USE_PP_IN_UPPER_LEVEL  1

/*  If we are going to use the side reasoning in upper level.  */
#define USE_REASONING_IN_UPPER_LEVEL  0

#if POSTCALC_FILE_RESULTS
#ifndef POST_CALCULATOR
extern
#endif  /*  POST_CALCULATOR  */
char PRSFileName[80];

#endif
#if PG_DEBUG || (defined(FORMULA) && !DEMO)
extern _SHORT mpr;
#endif

#if POSTCALC_FILE_RESULTS
#ifndef POST_CALCULATOR
extern
#endif  /*  POST_CALCULATOR  */
_BOOL bDumpPostResults;
#ifndef POST_CALCULATOR
extern
#endif  /*  POST_CALCULATOR  */
_BOOL bWriteInPRSFile;
#endif  /*  POSTCALC_FILE_RESULTS  */

/*  Set to 1 for the windows version - the file dump.prs */
/* is created in c:\.                                    */
#if HWR_SYSTEM == HWR_WINDOWS
#define WINDOWS_POSTCALC_DUMP  1
#else
#define WINDOWS_POSTCALC_DUMP  0
#endif

/*  This structure contains all parameters needed for the postprocessing */
/* learning and is passed to everybody who also needs them.              */
#if  POST_LEARNING_ON

#define PL_SELF  0
#define PL_OTHER 1
#define PL_TRASH 2

#define PL_ID_LEN  22

typedef struct _PL_STAT
{
	_LONG  curMinSelf;
	_LONG  curMaxSelf;
	_LONG  curMinOther;
	_LONG  curMaxOther;
	_LONG  curMinTrash;
	_LONG  curMaxTrash;
	_SHORT curAbortCountSelf;
	_SHORT curAbortCountOther;
	_SHORT curAbortCountTrash;
	_SHORT curCorrectCountSelf;
	_SHORT curIncorrectCountSelf;
	_SHORT curCorrectCountOther;
	_SHORT curIncorrectCountOther;
	_SHORT curSuccessCountTrash;
} PL_STAT, _PTR P_PL_STAT;

typedef struct _PL_COMPARISON
{
	_CHAR   ch;
	_CHAR   var;
	_CHAR   rule;
	_CHAR   cmpsNum;
	_LONG   rightSide;
	_SHORT  operation;
	_SHORT  nTargets;
	_CHAR   targetChar[8];
	_UCHAR  targetGroup[8];
	PL_STAT stat;
	_CHAR   id[PL_ID_LEN];
} PL_COMPARISON, _PTR P_PL_COMPARISON;

typedef struct _PL_LETTER
{
	_CHAR   ch;
	_CHAR   var;
	_SHORT  xrBeg;
	_SHORT  xrEnd;
} PL_LETTER, _PTR P_PL_LETTER;

typedef struct _PL_CMPS_IN_RULE
{
	_LONG   leftSide;
	_LONG   rightSide;
	_BOOL   bWasExecuted;
	_SHORT  operation;
	_SHORT  nTargets;
	_CHAR   targetChar[8];
	_UCHAR  targetGroup[8];
	_CHAR   id[PL_ID_LEN];
} PL_CMPS_IN_RULE, _PTR P_PL_CMPS_IN_RULE;

typedef struct _POST_LRN_STRUCT
{
	P_PDF_LOAD_HEADER pPDFHeader;
	p_VOID            datptr;
	_HTEXT            lrnOut;
	_BOOL             bDoNotUseIt;
	_CHAR             user[12];
	_CHAR             cmpWord[w_lim];
	_CHAR             guessWord[w_lim];
	_SHORT            thisChar;
	_SHORT            thisVar;
	_SHORT            ruleNumber;
	_SHORT            cmpNum;
	_LONG             leftSide;
	_SHORT            operation;
	_SHORT            xrBeg;
	_SHORT            xrEnd;
	_CHAR             correctLetters[w_lim];
	_USHORT           correctVar;
	_SHORT            nUsedRules;
	_SHORT            curRule;
	P_PL_COMPARISON   usedRules;
	_SHORT            nUsedInWord;
	P_PL_LETTER       usedInWord;
	_SHORT            nCmpsInRule;
	P_PL_CMPS_IN_RULE cmpsInRule;
	_BOOL             bDuplicateChar;
} POST_LRN_STRUCT, _PTR P_POST_LRN_STRUCT;

#ifndef POST_CALCULATOR
extern
#endif  /*  POST_CALCULATOR  */
POST_LRN_STRUCT postLrnStruct;  /*  The global learning structure. */

#endif  /*  POST_LEARNING_ON  */


/*  The values of "" in POST_PARAMS structure: */

typedef  enum
{
	PPD_DO_NOTHING         = 0,
	PPD_DO_OWN_RULES       = 1,
	PPD_DO_MUTUAL_RULES    = 2,
	PPD_DO_SIDE_LETTER     = 4,
	PPD_DO_SIDE_WORD       = 8,
	PPD_DO_RULES           = PPD_DO_OWN_RULES|PPD_DO_MUTUAL_RULES,
	PPD_DO_SIDE            = PPD_DO_SIDE_LETTER|PPD_DO_SIDE_WORD,
	PPD_DO_MUTUAL_AND_SIDE = PPD_DO_MUTUAL_RULES|PPD_DO_SIDE,
	PPD_DO_ALL             = PPD_DO_RULES|PPD_DO_SIDE
}
PPD_WHAT_TO_DO;


/* WARNING!! Don't do PPD_MAX_VARIABLES less than 10   !!!!! */
#define PPD_MAX_VARIABLES 10


/*  The structure for the postprocessing parameters passing.  */
typedef struct _POST_PARAMS
{
	P_PDF_LOAD_HEADER pPDFHeader;
	rc_type       _PTR rc;
	p_xrdata_type      xrdata;
	PS_point_type _PTR pTrace;
	p_SHORT            xTrace;
	p_SHORT            yTrace;
	p_RWG_type         pRWG;
	p_RWS_type         pRWS;
	RWG_PPD_type  _PTR pNewPPDData;
	p_SHORT            prevPPDIndex;
	p_SHORT            nextPPDIndex;
	p_CHAR             calcBuffer;

#if POST_LEARNING_ON
	P_POST_LRN_STRUCT  pLS;
#endif  /*  POST_LEARNING_ON  */

#if POSTCALC_FILE_RESULTS
	rec_w_type         correctWord;
	_HTEXT             resOut;
	_LONG              lWordNumberInTest;
	_SHORT             oldNumGroup;
	_SHORT             numGroup;
	_UCHAR             sz1stWord[w_lim];
	RWG_type           correctRWG;
	_INT               nRulesTotal;
#endif  /*  POSTCALC_FILE_RESULTS  */

	_BOOL              bPrevRule;
	_BOOL              bNextRule;
	_BOOL              bFirstChar;
	_BOOL              bLastChar;
	_SHORT             ppdIndex;
	_SHORT             nPoints;
	_SHORT             calcBufferMaxLen;
	_SHORT             rulesUsed;

	long               PPDInternalVars[PPD_MAX_VARIABLES];

	PPD_WHAT_TO_DO     WhatToDo;

	_UCHAR             cChgLet;        /*  Parameters for "ChangePPDLetter" */
	long               lChgLetResult;  /*      --"--                        */

	_BOOL              bUseGlobalFirstLast;  //for "PPGraph" to work
	_BOOL              bGlobalFirstChar;
	_BOOL              bGlobalLastChar;

	_BOOL              bUseFrom;  //1 to use "From" and "FatalFrom" functions
	//0 to always do the rules.

	_UCHAR             thisChar, prevChar, nextChar;

} POST_PARAMS, _PTR P_POST_PARAMS;

/*  External names to call.  */

#if (DTE_EDITOR==0)
void  EvaluateAndSortAnswers(rec_w_type *rec_words,
                             rc_type _PTR rc,
                             xrdata_type _PTR xrd,
                             p_RWG_type pRWG);
_BOOL  EvaluateAnswers(P_POST_PARAMS pParams,
                       rec_w_type _PTR rec_words,
                       p_POST_CONTROLS pPostCtrl,
                       p_BOOL pbLetterGraph);
_VOID EvaluateInUpperLevel(p_RWG_type pRwg);
_LONG EvaluateCharQuality(P_POST_PARAMS pParams);
long  CalculateQueueResult(P_POST_PARAMS pParams, char *interpBuffer,
                           short *ptRetcod);

#endif /* DTE_EDITOR*/

short AnalyseExpressionString(p_CHAR inputString, short inputLen,
                              p_CHAR interpBuffer,
                              short interpBufMaxLen, p_SHORT pInterpBufUsed,
                              short calcBufMaxLen,
                              p_SHORT ptRetPos);

/*  The characteristics of the comparing functions.  */
#define COMPARE_HEIGHT        20
#define COMPARE_HALF_WIDTH    50

/*  Suggested lengths of the interpreter result buffer and of */
/* calculation buffer.                                        */
#define CALC_BUF_SIZE         60
#define INTERP_BUFFER_SIZE   400

/*  Коды возврата, описывающие ошибки разбора.  */
#define SYNTAX_ERROR           1
#define EXPRESSION_TOO_LONG    2
#define NO_BRACKET             3
#define WRONG_ARGUMENTS_NUMBER 4
#define NESTING_TOO_DEEP       5

/*  Коды возврата, описывающие ошибки вычисления.  */
#define DIVIDE_ERROR          11
#define OVERFLOW_ERROR        12
#define DOMAIN_ERROR          13
#define FLOAT_STACK_OVERFLOW  14
#define UNKNOWN_ERROR         15
#define STACK_TOO_SMALL       16
#define ARGUMENT_NOT_FOUND    17
#define NO_MEMORY_FOR_CALC    18
#define CURV_MEASURE_ERROR    19
#define OTHER_DIRECTION_RULE  20
#define NO_PREVIOUS_CHARACTER 21
#define NO_NEXT_CHARACTER     22

#define BREAK_WHERE_SHOULDNT  23
#define NO_PTS_IN_TRAJECTORY  24
#define NO_PREV_FOR_SEPRLET   25
#define NO_NEXT_FOR_SEPRLET   26
#define NO_CORRELATED_XRS     27

#define WRONG_INDEX_OF_VARIABLE   28
#define LANGUAGE_IS_WRONG         29
#define NO_SUCH_LETTER_IN_LIST    30

#define UNDEFINED_FUNCTION        31

/*  Макросы, определяющие поля очереди. Ставятся в начале и конце поля,
  позволяя двигаться по очереди вперед-назад. Между ними - информационное
  поле.
*/

#define GLOBAL_END          0   /*  конец всей очереди.  Инф. поле: 0 */
#define GLOBAL_END_SIZE     2
#define UNEXTENDED_FIELD    1    /*  поле, вместо которого должно стоять  */
#define UNEXTENDED_FIELD_SIZE 6  /*  проанализированное выражение.
                                     Инф. поле: 2 байта - начало выр-я,
                                                2 байта - длина.*/
#define LOCAL_END           2   /*  локальный конец очереди при  */
#define LOCAL_END_SIZE      2   /*  разворачивании выражения. Инф. поле: 0 */
#define NUMBER              3   /*  число. Инф. поле: 4 байт. */
#define NUMBER_SIZE         6
#define OPERATION           4   /*  Операция. Инф. поле: 1 байт. */
#define OPERATION_SIZE      3
#define FUNCTION            5   /*  Функция. Инф. поле: 2 байта - номер.  */
#define FUNCTION_SIZE       4   /*  функции в массиве структур FUNCDEF. */
#define PLACEHOLDER         6   /*  Место для вставки очередной  */
#define PLACEHOLDER_SIZE    2   /*  проанализированной конструкции из
                                    распознаваемой строки. Инф. поле: 0. */
#define ARGUMENT            7   /*  Аргумент (xrXX[.X]). */
/*  Info field: 1 byte - xr number, */
/*              1 byte - field ID.  */
#define ARGUMENT_SIZE       4

/*  Compressed fields IDs an sizes.  */
#define Z_GLOBAL_END           8   /*  конец всей очереди.  Инф. поле: 0 */
#define Z_GLOBAL_END_SIZE      1
#define Z_NUMBER               9   /*  число. Инф. поле: 4 байт. */
#define Z_NUMBER_SIZE          5
#define Z_SHORT               10   /*  SHORT. Инф. поле: 2 байт. */
#define Z_SHORT_SIZE           3
#define Z_CHAR                11   /*  CHAR. Инф. поле: 1 байт. */
#define Z_CHAR_SIZE            2
#define Z_OPERATION           12   /*  Операция. Инф. поле: 1 байт. */
#define Z_OPERATION_SIZE       2
#define Z_FUNCTION            13   /*  Функция. Инф. поле: 2 байта - номер.  */
#define Z_FUNCTION_SIZE        3   /*  функции в массиве структур FUNCDEF. */
#define Z_SHORT_FUNCTION      14   /*  Функция. Инф. поле: 2 байта - номер.  */
#define Z_SHORT_FUNCTION_SIZE  2   /*  функции в массиве структур FUNCDEF. */
#define Z_ARGUMENT            15   /*  Аргумент (xrXX[.X]). */
/*  Info field: 1 byte - xr number, */
/*              1 byte - field ID.  */
#define Z_ARGUMENT_SIZE        3
#define RIGHT_SIDE         16   /* Comparison right side field. */
#define RIGHT_SIDE_SIZE    13   /*  1 byte - field size (always 13 for */
/* now).  2 bytes - index in the right */
/* sides array when executing  and the */
/* right  side itself when editing.  8 */
/* bytes - 4 pairs of target character */
/* of the rule and its group.  If some */
/* day we'll make this a variable-size */
/* field,  the number of pairs will be */
/* determined by the first  byte;  for */
/* now,  the  NULL character means the */
/* end of pairs.                       */

/*   XR field ID's.  Attention! The numbers are important! */
/* Some   modules  use  XRMOVE_TO_NEXT  or  XRMOVE_TO_PREV */
/* adding to  obtain  the  ..._NEXT_...  and  ..._PREV_... */
/* constants !                                             */
#define XRID_ITSELF                0
#define XRID_XL_FIELD              1
#define XRID_YD_FIELD              2
#define XRID_XR_FIELD              3
#define XRID_YU_FIELD              4
#define XRID_PREV_ITSELF           5
#define XRID_PREV_XL_FIELD         6
#define XRID_PREV_YD_FIELD         7
#define XRID_PREV_XR_FIELD         8
#define XRID_PREV_YU_FIELD         9
#define XRID_NEXT_ITSELF          10
#define XRID_NEXT_XL_FIELD        11
#define XRID_NEXT_YD_FIELD        12
#define XRID_NEXT_XR_FIELD        13
#define XRID_NEXT_YU_FIELD        14
#define XRMOVE_TO_PREV             5
#define XRMOVE_TO_NEXT            10

/*  Операции.  */
#define ADD_OP              0
#define SUBTRACT            1
#define MULTIPLY            2
#define DIVIDE              3
#define POW                 4
#define CHANGE_SIGN         5
#define LESS                6
#define GREATER             7
#define EQUALS              8

/*  Структура, описывающая очередь.  */

typedef struct
{
	p_CHAR data;
	int  maxlen;
	int  length;
	int  pointer;
} QUEUE;

/*  Отладочная функция распечатки очереди.  */
void PrintQueue(QUEUE *);

/*  Локальные функции анализатора.  */

int InsertUnextendedField(QUEUE *, int, int);
_BOOL UnextendedFieldPresent(QUEUE *);
int  MoveQueuePointer(QUEUE *, int);
_BOOL SkipSpaces(char **, char *, int);
int FindFieldSize(char);
void RemoveField(QUEUE *);
int InsertNumberField(QUEUE *, long);
int ShiftQueue(QUEUE *, char);
int InsertLocalEndField(QUEUE *);
int InsertPlaceholderField(QUEUE *);
int InsertArgumentField(QUEUE *, short, short);
int InsertFunctionField(QUEUE *, int);
int InsertOperationField(QUEUE *, char);
_BOOL FindClosingBracket(char **, char *, int, int *, int *, short, int *);
long FindNumberEnd(char **, char *, int);
int  FindKeyword(char **, char *, int, int *);
_BOOL FindLocalEnd(QUEUE *);
int ExtendField(QUEUE *, char *, short, int *);
int InsertPHAndOpCode(QUEUE *, char);
#if (DTE_EDITOR==0)
long CalculateFunction(P_POST_PARAMS pParams, QUEUE *, long *,
                       short *, int *);
long  CalculateGroupResult(P_POST_PARAMS pParams,
                           P_PDF_RULE_HEADER pRuleHeader,
                           _LONG lRuleAbsLimit);
_SHORT FindXrIndex(RWG_PPD_type _PTR pNewPPDData,
                   _SHORT ppdIndex, _INT xrNum);
#endif /* DTE_EDITOR */
long CalculateLess(long, long, int *);
long CalculateGreater(long, long, int *);
long CalculateEquals(long, long, int *);
long CalculateAdd(long, long, int *);
long CalculateSubtract(long, long, int *);
long CalculateMultiply(long, long, int *);
long CalculateDivide(long, long, int *);
long CalculatePow(long, long, int *);
long CalculateChangeSign(long, int *);
short  FindArgumentParameters(char **ppStr, short *pXrNumber,
                              short *pXrField);
short  DevelopMultiArguments(QUEUE *ptQueue, char *String,
                             short begIndex, short length, short numArgs);
short  FindQueueTotalLength(QUEUE *ptQueue);
_BOOL  RenameRuleXrs(P_PDF_RULE_HEADER pRuleHeader, _SHORT xrNum,
                     _SHORT delta);
_VOID  MergeTwoRecWordsSets(rec_w_type _PTR rec_words,
                            rec_w_type _PTR new_rec_words);
_VOID  MakeRecWordsFromWordGraph(p_RWG_type pRWG,
                                 rec_w_type _PTR rec_words,
                                 _INT self_weight);
_VOID  MakeRecWordsFromGraph(p_RWS_type pRWS, _USHORT totalSize,
                             rec_w_type _PTR rec_words, p_UCHAR workBuffer,
                             RWG_PPD_type _PTR pNewPPDData);
_VOID  FillRecWordsElement(rec_w_type _PTR rec_words, p_RWS_type pRWS,
                           _SHORT numWord, _SHORT numSymbol,
                           _SHORT indexInGraph);
_BOOL  MakeNewPath(p_RWS_type pRWS, _UCHAR (_PTR paths)[w_lim],
                   _USHORT nChars, _USHORT nPaths, _USHORT curPathNum,
                   p_SHORT pBestStepNum, p_SHORT pBestStepDiff);
void   SwapPrecompiledExpression(p_CHAR ptr);
_WORD  _FVPREFIX PostSPrintf(p_CHAR buffer, p_CHAR __format, ...);
_VOID  FillVirtualPPDElement(RWG_PPD_type _PTR pNewPPDData, _SHORT ppdIndex,
                             xrdata_type *xrd, _BOOL bFillPrev,
                             _BOOL bFillNext);
_BOOL CheckFirstLetter(rec_w_type _PTR rec_words, xrdata_type _PTR xrd,
                       rc_type _PTR rc, _SHORT yd);
_BOOL  StringsEqualIgnoreCase( p_CHAR s1, p_CHAR s2 );
_BOOL  StringsEqualIgnoreCaseOS( p_CHAR s1, p_CHAR s2 );

//CHE: moved it here since it is now used not only in "box":
_BOOL FindAverageLetterBoxes(rec_w_type _PTR rec_words, xrdata_type _PTR xrd,
                             p_RECT pSmallRect, p_RECT pBigRect,
                             p_SHORT pnSmallNumber, p_SHORT pnBigNumber,
                             p_RECT pFirstRect, p_SHORT pFirstLowTop,
                             p_RECT pMinMaxSmallRect, p_VOID dp);

/* CHE: Function for PPD in XrLet: */
_UCHAR  PPGraph( p_RWG_type pRWG, xrdata_type _PTR xrd, rc_type _PTR rc, _INT lf );
/* ayv 01/18/96*/
//for XRLW in swed mode
//_UCHAR RecodeDiacr(_UCHAR sym, p_SHORT xtrace,p_SHORT ytrace,
//                    _SHORT xst, _SHORT xend,
//                    xrdata_type (_PTR xrd)[XRINP_SIZE], rc_type _PTR rc);


/*  The learning functions prototypes. */
/* The  references  to  the  first two */
/* functions should be always resolved */
/* - learning or no learning.          */
_VOID InitPostLrnData(_VOID);
_VOID ClearPostLrnData(_VOID);
#if POST_LEARNING_ON
_BOOL InitPostLrnStruct(P_POST_LRN_STRUCT pLS);
_VOID ClearPostLrnStruct(P_POST_LRN_STRUCT pLS);
_VOID PostLrnNewWord(P_POST_PARAMS pParams);
_VOID PostLrnNewGuess(P_POST_LRN_STRUCT pLS, p_CHAR guessWord);
_VOID PostLrnNewCharVar(P_POST_LRN_STRUCT pLS, _SHORT newChar,
                        _SHORT newVar);
_VOID PostLrnNewRule(P_POST_LRN_STRUCT pLS, _SHORT ruleNumber,
                     p_CHAR objCode);
_VOID PostLrnNewComparison(P_POST_LRN_STRUCT pLS,
                           _SHORT operation, _LONG leftSide);
_VOID PostLrnCorrectLetters(P_POST_LRN_STRUCT pLS, p_CHAR correctLetters,
                            _USHORT correctVar, _SHORT xrBeg, _SHORT xrEnd);
_VOID PostLrnCalcRetcod(P_POST_LRN_STRUCT pLS, _SHORT calcRetcod,
                        _LONG ruleResult);
_VOID PostLrnSetUseFlag(P_POST_LRN_STRUCT pLS, _BOOL bUse);
_VOID AllocCmpsInRuleArray(P_POST_LRN_STRUCT pLS, p_CHAR objCode);
_VOID PostLrnDumpFinalStatistics(P_POST_LRN_STRUCT pLS);
#else
#define InitPostLrnStruct(a)
#define ClearPostLrnStruct(a)
#define PostLrnNewWord(a)
#define PostLrnNewGuess(a, b);
#define PostLrnNewCharVar(a, b, c)
#define PostLrnNewRule(a, b, c)
#define PostLrnNewComparison(a, b, c)
#define PostLrnCorrectLetters(a, b, c, d, e)
#define PostLrnCalcRetcod(a, b, c)
#define PostLrnSetUseFlag(a, b)
#endif  /*  POST_LEARNING_ON  */

#if POSTCALC_FILE_RESULTS

/*  Here are the prototypes of postprocessing dumping functions. */
_VOID  PostDumpNumberInQueue(p_CHAR name, _LONG number, _HTEXT resOut);
_VOID  PostDumpArgument(p_CHAR xrField, _LONG number, _HTEXT resOut);
_VOID  PostDumpFunction(p_CHAR xrField, _LONG number, _HTEXT resOut);
_VOID  PostDumpOperation(p_CHAR xrField, _LONG numberLeft,_LONG numberRight);
_VOID  PostDumpOperationResult(p_CHAR xrField, _LONG number, _HTEXT resOut);
_VOID  PostDumpShort2Var(p_CHAR strVerbose, p_CHAR strNonVerbose,
                         _SHORT number, _HTEXT resOut);
_VOID  PostDumpCorrectLetters(P_POST_PARAMS pParams);
_VOID  PostDumpString2Var(p_CHAR strVerbose, p_CHAR strNonVerbose,
                          _HTEXT resOut);
_VOID  PostDumpChar2Var(p_CHAR strVerbose, p_CHAR strNonVerbose,
                        _CHAR ch, _HTEXT resOut);
_VOID  PostDumpLong(p_CHAR str, _LONG number, _HTEXT resOut);
_VOID  PostDumpShort(p_CHAR str, _SHORT number, _HTEXT resOut);
_VOID  PostDumpQueueResult(_SHORT calcRetcod, _LONG ruleResult,
                           _HTEXT resOut);
_VOID  PostDumpInitWord(P_POST_PARAMS pParams, rec_w_type _PTR rec_words,
                        p_LONG pLWordNumberInTest);
_BOOL  PostDumpOpenFile(P_POST_PARAMS pParams);
#define OPEN_PRS_MACRO(a) if (!PostDumpOpenFile(a)) goto err;
_VOID  PostDumpString(p_CHAR format, p_CHAR str, _HTEXT resOut);
_VOID  PostDumpNewWord(P_POST_PARAMS pParams, _SHORT ppdIndex,
                       _SHORT numGroup, _SHORT groupLen,
                       _BOOL bInsideBrackets);
_VOID  PostDumpChar(p_CHAR str, _CHAR ch, _HTEXT resOut);
_VOID  PostDumpCharScore(p_CHAR str, _SHORT score, _SCHAR scoreScaled,
                         _SHORT rulesUsed, _HTEXT resOut);
_VOID  PostDumpEffect(_BOOL bApplied, P_POST_PARAMS pParams,
                      rec_w_type _PTR rec_words);
_VOID  PostDumpCloseFile(P_POST_PARAMS pParams);
_VOID  FillCMPWordAliases(p_RWG_type pRWG, rec_w_type _PTR rwIn,
                          rec_w_type _PTR rwOut, rc_type _PTR rc,
                          xrdata_type _PTR xrdata,
                          p_RWG_type pCorrectRWG);
_VOID  PostDumpSetFlags(P_POST_PARAMS pParams);

#else  /*  !POSTCALC_FILE_RESULTS  */

/*  Disable dumping calls if no dumping.  */
#define PostDumpNumberInQueue(a, b, c)
#define PostDumpArgument(a, b, c)
#define PostDumpFunction(a, b, c)
#define PostDumpOperation(a, b, c)
#define PostDumpOperationResult(a, b, c)
#define PostDumpShort2Var(a, b, c, d)
#define PostDumpCorrectLetters(a)
#define PostDumpString2Var(a, b, c)
#define PostDumpChar2Var(a, b, c, d)
#define PostDumpLong(a, b, c)
#define PostDumpShort(a, b, c)
#define PostDumpQueueResult(a, b, c)
#define PostDumpInitWord(a, b, c)
#define PostDumpOpenFile(a)         (_TRUE)
#define OPEN_PRS_MACRO(a)
#define PostDumpString(a, b, c)
#define PostDumpNewWord(a, b, c, d, e)
#define PostDumpChar(a, b, c)
#define PostDumpCharScore(a, b, c, d, e)
#define PostDumpEffect(a, b, c)
#define PostDumpCloseFile(a)
#define FillCMPWordAliases(a, b, c, d, e, f)
#define PostDumpSetFlags(a)

#endif  /*  POSTCALC_FILE_RESULTS  */

#endif  /*  POSTCALC_H  */
