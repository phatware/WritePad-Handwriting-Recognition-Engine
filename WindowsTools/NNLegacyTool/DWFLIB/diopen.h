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

#ifndef __DIOPEN_H__
#define  __DIOPEN_H__

/***************************************************************************
 * Includes
 ***************************************************************************/

#if defined(UNDER_CE) || defined(_WINDOWS)
#ifndef  _WIN32
#define _WIN32
#endif
#endif /* UNDER_CE */

#include "dio_define.h"
#include "templatemodel.h"    // load model
/***************************************************************************
 * Local Definitions
 ***************************************************************************/
#ifdef _WIN32
#if defined(DIOPEN_EXPORTS) || defined(DIOPENCE_EXPORTS)
#define DIOPEN_API __declspec(dllexport)
#else /* DIOPEN_EXPORTS */
#ifdef  __cplusplus
#define DIOPEN_API  extern "C"
#else  /* __CPP__ */
#define DIOPEN_API
#endif  /* __CPP__ */
#endif /* DIOPEN_EXPORTS */
#else /* _WIN32 */
#ifdef  __cplusplus
#define DIOPEN_API  extern "C"
#else  /* __CPP__ */
#define DIOPEN_API  extern
#endif  /* __CPP__ */
#endif /* _WIN32 */

/*-------------------------------------------------------------------
  Constant Definition
  -------------------------------------------------------------------*/
// hjlee 2007.07.27  european 정의
#if defined __LATIN_1__ || defined __LATIN_EX_A__ || defined __GREEK__ \
  || defined __CYRILLIC__
#ifndef __EUROPEAN__
#define __EUROPEAN__
#endif
#endif //__LATIN_1__

#define  DIO_MAXPOINT      2500
#define  DIO_MAXSTROKE      99

#define  DIO_CHAR_MAXPOINT    500
#define  DIO_CHAR_MAXSTROKE    21

#ifdef _LINUX_   //FIORE
#define  DIO_MAX_CANDIDATE    10
#elif defined __EUROPEAN__
#define DIO_MAX_CANDIDATE    20    // 필터링을 하기전에 많은 후보를 받아둔다.
#elif __DIO_CANDIDATE_13__    // wihwang 2007/10/29 [Candidate_13]
#define DIO_MAX_CANDIDATE    13
#else
#define  DIO_MAX_CANDIDATE    10
#endif //__EUROPEAN__

#define DIO_MAX_CHARACTERS    100
#define DIO_MAX_CHARACTERS_KANA  10        // wihwang 2009/02/25 [Restrict the number of characters to recognize]
#define DIO_MAX_CHARACTERS_BPMF 5

#define  DIO_HAN_VOWEL      0x10000000
#define  DIO_HAN_CONSONANT    0x00000001
// Set Output Code
#define  DIO_CODE_KSC5601    0x00000002
#define  DIO_CODE_JOHAP      0x00000004
#define DIO_CODE_EXKS      0x20000000    // 확장 완성형
#define  DIO_CODE_UNICODE    0x00000008
// recog character type
#define  DIO_HANGUL_11172    0x00000010
#define  DIO_HANGUL        0x00000020
#define DIO_HANMODE        (DIO_HANGUL | DIO_HANGUL_11172)
#define DIO_HANMODE_ALL      (DIO_HANGUL | DIO_HANGUL_11172 | DIO_HAN_CONSONANT | DIO_HAN_VOWEL)
#define  DIO_UPPERCASE      0x00000040
#define  DIO_LOWERCASE      0x00000080
#define  DIO_NUMERIC        0x00000100
#define DIO_ENGLISH        0x000000c0    // UPPERCASE | LOWERCASE
#define  DIO_ALPHANUMERIC       0x000001c0    // UPPERCASE | LOWERCASE | NUMERIC
#define  DIO_SYMBOL        0x00000200
#define  DIO_ASCII        0x000003c0    // ALPHANUMERIC | SYMBOL
#define  DIO_GESTURE        0x00000400
#define  DIO_DEFSYMBOL      0x00000800
#define  DIO_ACCENTCHAR      0x00001000
#define  DIO_MODECHAR      0x00002000
#define DIO_TURKISHSYM      0x00008000
#define  DIO_ARABICSYM      0x00008000    // arabic symbol
#define DIO_DIALDIGIT_PW    0x00010000
#define DIO_DIALDIGIT      0x00100000
#define DIO_FLICK        0x00020000
#define DIO_ARABIC_FARSI    0x00080000    // dhlee 2008/08/04 [아랍어 페르시아어(Farsi)]
#define DIO_ARABIC_URDU      0x00004000    // dhlee 2009/04/07 [인도어 우르두어(Urdu)]
#define DIO_ARABIC        0x00200000
#define DIO_PUNCTUATION      0x02000000    // 문장부호 혼용 (mix Hangul(Eng) and Punctuation Marks)

//okjeong 2009/01/30 [kana_recog_onechar] {
#define DIO_HIRAGANA      0x04000000
#define DIO_KATAKANA      0x80000000
//okjeong 2009/01/30 [kana_recog_onechar] }
#define DIO_BPMF        0x00040000

// sentence recognition Flag 2002.07.25
#define  DIO_AUTOSPACING      0x00400000
#define  DIO_AUTOWRAPING      0x00800000

// hjlee 2007.11.26 사칙연산
#define  DIO_SIMP_CALC      0x00004000

#define DIO_EQUALITY      0x00000800    // The sign of equality and inequality Mode ( =, >, < )

#define DIO_CONS_RECOMMEND    0x01000000    // hjlee 2008/12/17
#define DIO_TRAIN_USER_MODEL  0x08000000    // hjlee 2009/02/27 [user_model]

// 제 2 의 인식 모드
// dioRC.nRecogMode2 에 들어가는 모드 {
#define DIO_EMOTICON      0x00000001    // ejjeon 2009/08/18 emoticon mode 추가
#define DIO_ENDPUNC        0x00000002    // okjeong 2009/09/01 [mix_han_emo] 단어의 끝에만 올 수 있는 문장부호 모드
#define DIO_HEBREW        0x00000004    // dhlee 2009/09/10 [hebrew mode 추가]
#define DIO_MIXLOWERCASE    0x00000008    // okjeong 2010/01/25
#define DIO_MIXUPPERCASE    0x00000010    // okjeong 2010/01/25
#define DIO_MIXENGLISH      (DIO_MIXLOWERCASE|DIO_MIXUPPERCASE)
#define DIO_MIXNUMERIC      0x00000020    // okjeong 2010/01/25
#define DIO_MIXPUNC        0x00000040
#define DIO_MIXSYMBOL      0x00000080
#define DIO_MIXHIRA        0x00000100
#define DIO_MIXKATA        0x00000200
#define DIO_ARABICNUM      0x00000400
#define DIO_RUSSIAN        0x00000800
#define DIO_MIXEDMODE      (DIO_MIXLOWERCASE | DIO_MIXUPPERCASE | DIO_MIXNUMERIC | DIO_MIXPUNC | DIO_MIXSYMBOL | DIO_MIXHIRA | DIO_MIXKATA)
#define DIO_SUPREMO_MODE   (DIO_RUSSIAN)
// dioRC.nRecogMode2 에 들어가는 모드 }

/* only use for unistroke and graffiti */
#define  DIO_EXTENDED      0x0800
#define  DIO_PALM_CONTROL    0x0400  //TEEJAY 2003.08.12
#define  DIO_STATE_NEW      0x0001
#define  DIO_STATE_CONTINUE    0x0002
#define  DIO_STATE_UPDATE    0x0004
#define  DIO_STATE_CAPSLOCK    0x0006
#define  DIO_STATE_NORMAL    0x0008
#define  DIO_STATE_NOTHING    0x0008
#define  DIO_STATE_EXTENDED    0x000a
#define  DIO_STATE_DOT      0x000c
#define  DIO_STATE_SHIFT      0x000e
#define  DIO_STATE_SHIFT2    0x000f
#define  DIO_STATE_FAIL      0x0000

/* definition of Gesture code */
#define  DIO_GESTURE_SPACE      0x20
#define  DIO_GESTURE_BACKSPACE    0x08
#define  DIO_GESTURE_DELETE      0x0c
#define  DIO_GESTURE_TAB        0x09
#define  DIO_GESTURE_RETURN      0x0d
#define  DIO_GESTURE_SYMBOL      0x01
#define DIO_GESTURE_MERGE      0x02  // hjlee 2009/12/30
#define  DIO_GESTURE_AWAY		0x05

#define  DIO_GESTURE_HELP      0x06  //draw to LEFT AND UP
#define  DIO_GESTURE_UPRIGHT      0x05  //draw to upper-right corner directly
#define  DIO_GESTURE_USER1      0x04  //shape as Graffiti A stroke
#define  DIO_GESTURE_USER2      0x0e  //draw to upper-left corner directly
#define  DIO_GESTURE_USER3      0x0f  //draw to upper-right corner and return to origin
#define  DIO_GESTURE_USER4      0x02  //draw to RIGHT and UP
#define  DIO_GESTURE_USER5      0x07  //draw the shape of delete gesture to reverse direction, Graffiti 2 Paste gesture

/* for Graffiti recognizer */
#define  DIO_GESTURE_GCAPS      DIO_GESTURE_SYMBOL
#define  DIO_GESTURE_GRETURN      0x03  //Graffiti return gesture
#define  DIO_GESTURE_OSPACE      0x0a  //draw to LEFT and return to origin, Graffiti 2 Undo

#define  DIO_GESTURE_BACKSLASH    0x0b  //to input Palm OS Graffiti 2 dot //2004.03.31
#define  DIO_GESTURE_SHORTCUT    0x17  //for Palm OS Shorcut stroke //2004.03.31

/* two byte symbol recognition code */
#define  DIO_SYMBOL_WON        0x0a
#define  DIO_SYMBOL_YEN        0x0b

/***************************************************************************
 * Local Type Definitions
 ***************************************************************************/

#ifndef _DIOINK_
#define _DIOINK_
typedef struct tagDIOINK
{
	DIO16    nStrokes;
	DIO16    nPoints;
	DIO16    nIndex[DIO_MAXSTROKE + 1];
	DIOPOINT  udCoord[DIO_MAXPOINT];
} DIOINK, *PDIOINK;
#endif

#ifndef _DIOINKC_
#define _DIOINKC_
typedef struct tagDIOINKC
{
	DIO16    nStrokes;
	DIO16    nPoints;
	DIO16    nIndex[DIO_CHAR_MAXSTROKE + 1];
	DIOPOINT  udCoord[DIO_CHAR_MAXPOINT];
} DIOINKC, *PDIOINKC;
#endif

#ifndef _DIORC_
#define _DIORC_
typedef struct tagDIORC
{
	DIO16    nCandidate;
	DIOU32    nRecogMode;
	DIOU32    nRecogMode2; // ejjeon 2009/08/18 제 2 의 인식 모드
	DIOU32    nSupremoMain; //okjeong 2011/01/06 [usermodel]
	DIOU32    nSupremoType; //okjeong 2011/01/06 [usermodel] {
	DIOU32    lEuroLangMode1;
	DIOU32    lEuroLangMode2;
#ifdef __LOAD_MODEL__
	DIOU8    *pModel[MOD_MAX_LANG];    // load file model
#endif
} DIORC, *PDIORC;
#endif

/* okjeong 2008.03.04 : boxinfo -> _writingArea 로 바뀌어 주석처리함
#ifndef _BOXINFO_
#define _BOXINFO_
typedef struct tagBOXINFO
{
DIO16    nOriginX, nOriginY;    //  origin of start box
DIO16    nBoxWidth, nBoxHeight;  //  width and height of box
DIO16    nHorzBox;        //  number of columns
DIO16    nVertBox;        //  number of rows, not used
} BOXINFO, *PBOXINFO;
#endif
*/

enum
{
	ALNUM_CHOSUNG_POS,
	JUNGSUNG_POS,
	JONGSUNG_POS,
	NUM_MODEL_POS
};

#ifndef _CANDINFO_
#define _CANDINFO_
typedef struct tagCANDINFO
{
	DIOU16      unCode;     /*  recognized code  */
	DIO16      nDistance;    /*  matching distance */
	DIOU16      uModelInfo;
	DIORECT    udRect;

} CANDINFO, *PCANDINFO;
#endif

#ifndef _DIORESULT_
#define _DIORESULT_
typedef struct tagDIORESULT
{
	DIO32   stroke;
	DIO16    nCand;
	CANDINFO  udCandList[DIO_MAX_CANDIDATE];
} DIORESULT, *PDIORESULT;
#endif

// ejjeon 2009/08/06 처음 사용자가 설정한 인식모드를 set, get 하는 함수 추가
DIOU32 dioGetRecogMode(void);
void dioSetRecogMode(DIOU32 dwRecogMode);

/*-------------------------------------------------------------------
  Function Definition
  -------------------------------------------------------------------*/
DIOBOOL dioRecognizeAlnumSup(PDIORC pudRC, PDIOINKC pudCData, PDIORESULT pudResult);
DIOPEN_API DIOBOOL dioRecognizeOneChar(PDIORC, PDIOINK, PDIORESULT);
DIOPEN_API DIO16 dioRecognizeSentence(PDIORC, PDIOINK, PDIORESULT, DIO16);
DIOPEN_API DIOBOOL RUNON_WordRecognition(PDIORC, PDIOINK, PDIORESULT, DIO16);
DIOPEN_API DIO16 dioGetConfidenceLevel(DIOU16, DIO16);
DIOPEN_API DIOBOOL dioRecognizeHangulJaso(PDIORC, PDIOINK, PDIORESULT);
void dioSortCandidate(PCANDINFO  pCandidate, int candNum); //okjeong 2009/08/20 [mix_han_sym] : 많이 쓰이는 함수 => 내부 디오펜 해더로 옮김

/* for unistroke */
//DIOPEN_API void dioInitStatus(PBOXINFO); //okjeong 2008.03.04 : 어느 곳에서도 호출하지 않는 함수
DIOPEN_API DIO32  dioRecognizeUnistroke(PDIORC, PDIOINK, PDIORESULT);  // hjlee 2008/01/08 change type 16->32
DIOPEN_API DIO16 dioRecognizeGraffiti(PDIORC, PDIOINK, PDIORESULT);
DIOPEN_API DIOU16 dioGetGraffitiState(void);
DIOPEN_API DIOU16 dioRecogHangulUnistroke(PDIORC, PDIOINK, DIOU16 * punResult);
DIOPEN_API void dioInitHanUnistroke(void);
/* for one 1 stroke recognizer */
DIOPEN_API DIO16 dioRecogUnistrokeAll(PDIOINK pudInk, DIOU8 *ubBuf, DIO8 nSize);
DIOPEN_API DIO16 dioGetUnistrokeState(DIO16 *pnState);

/* ETC */
DIOPEN_API DIOU16 *dioHangulToHanja(DIOU16 unCode, DIO16 *nLen, DIO16 nCodeFlag);

DIOPEN_API DIOBOOL dioSetDBFile(DIOU8 * pDB);
#endif /* __DIOPEN_H__ */

