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

#include <jni.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <android/log.h>

#include "InkWrapper.h"
#include "RecognizerWrapper.h"
#include "ligstate.h"
#include "letimg.h"
#include "ShapeAPI.h"

#define MAX_PATH	260


//#define USER_DICTIONARY						"WritePad_User.dct"
//#define USER_STATISTICS						"WritePad_Stat.lrn"
//#define USER_CORRECTOR						"WritePad_Corr.cwl"
//#define DEFAULT_DICTIONARY					"English"

#ifndef false
#define false	0
#endif
#ifndef true
#define true	1
#endif


static INK_DATA_PTR _inkData = NULL;
static RECOGNIZER_PTR _recognizer = NULL;
static int _currentStroke = -1;
static char _szPath[MAX_PATH] = { 0 };
    
#define MAX_XU_CONVERTS 5
#define MAX_CMD_WORDS   4
#define MAX_CMD_LENGHT  16


static int _iHexes[MAX_XU_CONVERTS] = { 0x08a, 0x08c, 0x09a, 0x09c, 0x09f };
static int _iUnicodes[MAX_XU_CONVERTS] = { 352, 338, 353, 339, 376 };

static int u_strlen(const unsigned short * str) {
	register int i = 0;
	for (i = 0; str[i] != 0 && i < 2048; i++)
		;
	return i;
}

// JNI BUG: if the first character of the UTF-8 string has high bit set to 1 (ie char code > 127)
// JNI crashes in this implementation. One way to avoid it is to add an extra character in front
// which code is less than 128

/* ************************************************************************** */
/* *   Convert char string to UNICODE                                       * */
/* ************************************************************************** */

static int StrToUNICODE(unsigned short * tstr, const char * str, int cMax) {
	register int i, j;

	for (i = 0; i < cMax && str[i] != 0; i++) {

		tstr[i] = (unsigned short) (unsigned char) str[i];
		/* this causes crash when translating to UTF string; do not use may relate to the bug
		 described above
		 if((unsigned char)str[i]>=0x08a && (unsigned char)str[i]<=0x09f)
		 {
		 for (j=0; j<MAX_XU_CONVERTS; j++)
		 {
		 if((unsigned char)str[i]==_iHexes[j])
		 tstr[i] = _iUnicodes[j];
		 }
		 }
		 */
	}

	tstr[i] = 0;
	return i;
}

/* ************************************************************************** */
/* *   Convert UNICODE string to char                                       * */
/* ************************************************************************** */

static int UNICODEtoStr(char * str, const unsigned short * tstr, int cMax)
{
	register int i, j;
    
	for (i = 0; i < cMax && tstr[i] != 0; i++)
    {
		if (tstr[i] < 0xff)
			str[i] = ((unsigned char) tstr[i]);
		else
			str[i] = '?';
	}
	str[i] = 0;
	return i;
}

static const unsigned short * UTF8ToUnicode(const unsigned char *Src) 
{
	if (Src == NULL || *Src == 0)
		return NULL;
    
	int i = 0;
	int outputlen = 0;
	int SrcLen = strlen((const char *) Src);
    
	// unicode will be the same or shorter
	int DestLen = SrcLen + 2;
	unsigned short * strDest = (unsigned short *) malloc(DestLen
                                                         * sizeof(unsigned short));
	if (NULL == strDest)
		return NULL;
    
	for (i = 0; i < SrcLen;) {
		if (outputlen >= DestLen - 1) {
			//overflow detected
			break;
		}
        
		else if ((0xe0 & Src[i]) == 0xe0) {
			strDest[outputlen++] = (unsigned short) ((((int) Src[i] & 0x0f)
                                                      << 12) | (((int) Src[i + 1] & 0x3f) << 6) | (Src[i + 2]
                                                                                                   & 0x3f));
			i += 3;
		} else if ((0xc0 & Src[i]) == 0xc0) {
			strDest[outputlen++] = (unsigned short) (((int) Src[i] & 0x1f) << 6
                                                     | (Src[i + 1] & 0x3f));
			i += 2;
		} else {
			strDest[outputlen++] = (unsigned short) Src[i];
			++i;
		}
	}
	strDest[outputlen] = '\0';
	return strDest;
}

static const char * UnicodeToUTF8(const unsigned short *Src) 
{
	if (Src == NULL || *Src == 0)
		return NULL;
    
	int i = 0;
	int outputlen = 0; /*bytes */
	int SrcLen = u_strlen(Src);
	int DstLen = 2 + 3 * SrcLen;
    
	unsigned char * strDest = (unsigned char *) malloc(DstLen);
	if (NULL == strDest)
		return NULL;
    
	for (i = 0; i < SrcLen; i++)
	{
		if (outputlen >= DstLen - 1) {
			//overflow detected
			break;
		}
        
		if (0x0800 <= Src[i]) {
			strDest[outputlen++] = (((Src[i] >> 12) & 0x0f) | 0xe0);
			strDest[outputlen++] = (((Src[i] >> 6) & 0x3f) | 0x80);
			strDest[outputlen++] = ((Src[i] & 0x3f) | 0x80);
		} else if (0x800 > Src[i] && 0x80 <= Src[i]) {
			strDest[outputlen++] = (((Src[i] >> 6) & 0x1f) | 0xc0);
			strDest[outputlen++] = ((Src[i] & 0x3f) | 0x80);
		} else if (0x80 > Src[i]) {
			strDest[outputlen++] = (unsigned char) Src[i];
		}
	}
	strDest[outputlen] = 0;
	return (const char *) strDest;
}
static jstring StringToJstringA(JNIEnv* env, const char * string)
{
	jstring result = NULL;
	int len = strlen(string);
    unsigned short *  uResult = (unsigned short *) malloc((len + 2) * sizeof( unsigned short ));
	if (uResult == NULL)
		return NULL;
	StrToUNICODE(uResult, string, len);
	const char * utfResult = UnicodeToUTF8(uResult);
	free((void *) uResult);
    
	if (NULL != utfResult)
    {
		result = (*env)->NewStringUTF(env, utfResult);
		free((void *) utfResult);
	}
    else
    {
		result = (*env)->NewStringUTF( env, string);
	}
	return result;
}


static jstring StringToJstring(JNIEnv* env, const UCHR * string)
{
	jstring result = NULL;
#ifdef HW_RECINT_UNICODE
	const char * utfResult = UnicodeToUTF8(string);
#else
	unsigned short * uResult;
	int len = strlen(string);
    uResult = (unsigned short *) malloc((len + 2) * sizeof( unsigned short ));
	if (uResult == NULL)
		return NULL;
	StrToUNICODE(uResult, string, len);
	const char * utfResult = UnicodeToUTF8(uResult);
	free((void *) uResult);
#endif  // HW_RECINT_UNICODE
    
	if (NULL != utfResult)
    {
		result = (*env)->NewStringUTF(env, utfResult);
		free((void *) utfResult);
	}
#ifndef HW_RECINT_UNICODE
    else
    {
		result = (*env)->NewStringUTF( env, string);
	}
#endif // HW_RECINT_UNICODE
	return result;
}

static const char * JstringToStringA(JNIEnv* env, jstring jstr)
{
    jboolean isCopy = JNI_FALSE;
	const char * string = (*env)->GetStringUTFChars(env, jstr, &isCopy );
	const char * result = NULL;
    
	const unsigned short * uString = UTF8ToUnicode((const unsigned char *)string);
	if (uString != NULL)
    {
		int len = u_strlen(uString);
		char * cString = (char *) malloc(len + 1);
		UNICODEtoStr(cString, uString, len);
		result = cString;
		free((void *) uString);
	}
    else
    {
		result = strdup(string);
	}
    if ( string && isCopy == JNI_TRUE)
        (*env)->ReleaseStringUTFChars(env, jstr, string);
	return result;
}

static const UCHR * JstringToString(JNIEnv* env, jstring jstr)
{
    jboolean isCopy = JNI_FALSE;
	const char *string = (*env)->GetStringUTFChars(env, jstr, &isCopy );
	const UCHR * result = NULL;
    
	const unsigned short * uString = UTF8ToUnicode((const unsigned char *)string);
#ifdef HW_RECINT_UNICODE
    result = (const UCHR *)uString;
#else
	if (uString != NULL)
    {
		int len = u_strlen(uString);
		char * cString = (char *) malloc(len + 1);
		UNICODEtoStr(cString, uString, len);
		result = cString;
		free((void *) uString);
	}
    else
    {
		result = strdup(string);
	}
#endif // HW_RECINT_UNICODE
    if ( string && isCopy == JNI_TRUE)
        (*env)->ReleaseStringUTFChars(env, jstr, string);
	return result;
}

static long _getTime(void) {
	struct timeval now;

	gettimeofday(&now, NULL);
	return (long) (now.tv_sec * 1000 + now.tv_usec / 1000);
}

static char userDict[MAX_PATH] = {0};
static char learner[MAX_PATH] = {0};
static char corrector[MAX_PATH] = {0};


jint Java_com_phatware_writepad_WritePadAPI_recognizerInit(JNIEnv* env, jobject thiz, jstring jpath, jint languageId, jbyteArray data, jstring pUserDict, jstring pLearner, jstring pCorrector)
{
    jboolean isCopy = JNI_FALSE;
	const char * path = (*env)->GetStringUTFChars(env, jpath, &isCopy);

	userDict[0] = 0;
	learner[0] = 0;
	corrector[0] = 0;

    // properly release recognizer
    // Java_com_phatware_writepad_WritePadAPI_freeRecognizer( env, thiz, pUserDict, pLearner, pCorrector );
    
	if (path != NULL)
    {
		strcpy(_szPath, path);
		strcat(_szPath, "/");
		strcpy(userDict, _szPath);
		strcpy(learner, _szPath);
		strcpy(corrector, _szPath);
	}

    const char * t = JstringToStringA(env, pUserDict);
    if ( NULL != t )
    {
        strcat(userDict, t );
        free( (void *)t );
    }
    t = JstringToStringA(env, pLearner);
    if ( NULL != t )
    {
        strcat(learner, t );
        free( (void *)t );
    }
    t = JstringToStringA(env, pCorrector);
    if ( NULL != t )
    {
        strcat(corrector, t );
        free( (void *)t );
    }
    
    if ( isCopy == JNI_TRUE && path )
        (*env)->ReleaseStringUTFChars(env, jpath, path);

	int flags = -1;
	_inkData = INK_InitData();
	if (NULL == _inkData)
    {
		return -1;
	}
	_recognizer = HWR_InitRecognizer(NULL, userDict, learner, corrector, languageId, &flags);
	if (NULL == _recognizer)
    {
        INK_FreeData( _inkData );
        _inkData = NULL;
        return -1;
    }

	_currentStroke = -1;

    isCopy = JNI_FALSE;
    int dataSize = (*env)->GetArrayLength(env, data);
    jboolean result;
    jbyte * pMem = (*env)->GetByteArrayElements(env, data, &isCopy);

    if ( dataSize > 0 )
    {
        result = HWR_SetLetterShapes( _recognizer, (const unsigned char *)pMem );
    }
    else
    {
        result = HWR_SetDefaultShapes( _recognizer );
    }

    if ( isCopy == JNI_TRUE && pMem )
        (*env)->ReleaseByteArrayElements(env, data, pMem, JNI_ABORT);

	return flags;
}

jboolean  Java_com_phatware_writepad_WritePadAPI_resetLearner(JNIEnv* env) {
	if (_recognizer != NULL) {
		return HWR_ResetLearner(_recognizer, learner);
	}
	return false;
}

jboolean  Java_com_phatware_writepad_WritePadAPI_reloadLearner(JNIEnv* env) {
	if (_recognizer != NULL) {
		return HWR_ReloadLearner(_recognizer, learner);
	}
	return false;
}

jboolean Java_com_phatware_writepad_WritePadAPI_saveLearner(JNIEnv* env) {
	if (_recognizer != NULL) {
		return HWR_SaveLearner(_recognizer, learner);
	}
	return false;
}


jboolean Java_com_phatware_writepad_WritePadAPI_resetAutocorrector(JNIEnv* env) {
	if (_recognizer != NULL) {
	    BOOL result = HWR_ResetAutoCorrector(_recognizer, corrector);
		return result;
	}
	return false;
}

jboolean Java_com_phatware_writepad_WritePadAPI_reloadAutocorrector(JNIEnv* env) {
	if (_recognizer != NULL) {
	    BOOL result = HWR_ReloadAutoCorrector(_recognizer, corrector);
		return HWR_ReloadAutoCorrector(_recognizer, corrector);
	}
	return false;
}

jboolean Java_com_phatware_writepad_WritePadAPI_saveWordList(JNIEnv* env) {
	if (_recognizer != NULL) {
		return HWR_SaveWordList(_recognizer, corrector);
	}
	return false;
}



jboolean Java_com_phatware_writepad_WritePadAPI_resetUserDict(JNIEnv* env) {
	if (_recognizer != NULL) {
	    BOOL result = HWR_ResetUserDict(_recognizer, userDict);
		return result;
	}
	return false;
}

jboolean Java_com_phatware_writepad_WritePadAPI_reloadUserDict(JNIEnv* env) {
	if (_recognizer != NULL) {
	    BOOL result = HWR_ReloadUserDict(_recognizer, userDict);
		return result;
	}
	return false;
}

jboolean Java_com_phatware_writepad_WritePadAPI_isPointStroke(JNIEnv* env, jobject thiz, jint nStroke )
{
	if ( _inkData != NULL )
	{
		CGRect rect = {0,0,0,0};
		if ( nStroke < 0 )
		    nStroke = INK_StrokeCount( _inkData, false ) - 1;
		if ( nStroke < 0 )
			return false;
		if ( INK_GetStrokeRect( _inkData, nStroke, &rect, false) )
		{
			if ( rect.size.width <= 2.0f && rect.size.height <= 2.0f )
				return true;
		}
	}
	return false;
}

jint Java_com_phatware_writepad_WritePadAPI_checkStrokeNewLine(JNIEnv* env, jobject thiz, jint nStroke )
{
	int result = GEST_NONE;
	if ( _inkData != NULL )
	{
		CGRect      rect = {0,0,0,0};
		CGStroke    pStroke = NULL;
		float         width = 3.0f;
        
		if ( nStroke < 0 )
			nStroke = INK_StrokeCount( _inkData, false ) - 1;
		if ( nStroke < 0 )
			return 0;
		if ( ! INK_GetStrokeRect( _inkData, nStroke, &rect , false) )
			return 0;
		int len = INK_GetStrokeP( _inkData, nStroke, &pStroke, &width, NULL);
		if (len > 5)
		{
			result = HWR_CheckGesture( GEST_DELETE | GEST_BACK, pStroke, len, 1, 5 );
		}
		if (pStroke != NULL)
			free((void *) pStroke);
		if ( result == GEST_DELETE || result == GEST_BACK )
			return 0;
		if ( rect.size.width < width && rect.size.height < width )
			return 0;
		if ( rect.size.width > 2.0f * rect.size.height && rect.size.height <= 2.0f * width )
			return 0;
		
		int xx = (int)rect.origin.x;
		int yy = (int)rect.origin.y;
		return (jint)((xx & 0xffff) | ((yy << 16) &0xffff0000));
	}
	return 0;
}

jint Java_com_phatware_writepad_WritePadAPI_getInkRightCoordinate(JNIEnv* env, jobject thiz )
{
	if ( _inkData != NULL )
	{
		CGRect      rect = {0,0,0,0};
        
        if ( INK_GetDataRect( _inkData, &rect, false ) )
        {
            return rect.origin.x + rect.size.width;
        }
    }
    return 0;
}

jboolean Java_com_phatware_writepad_WritePadAPI_saveUserDict(JNIEnv* env)
{
	if (_recognizer != NULL)
    {
		return HWR_SaveUserDict(_recognizer, userDict);
	}
	return false;
}

jboolean Java_com_phatware_writepad_WritePadAPI_setDictionaryData(JNIEnv* env, jobject thiz, jbyteArray buff)
{
	jbyte * data = NULL;
	jboolean result = false;
	if (_recognizer == NULL)
		return false;

    jboolean isCopy = JNI_FALSE;
	data = (*env)->GetByteArrayElements(env, buff, &isCopy );
	if (data != NULL)
    {
		result = HWR_SetDictionaryData(_recognizer, (const char *)data, FLAG_MAINDICT);
		(*env)->ReleaseByteArrayElements(env, buff, data, JNI_ABORT);
        if ( isCopy == JNI_TRUE )
            (*env)->ReleaseByteArrayElements(env, buff, data, JNI_ABORT);
	}
	return result;
}



void Java_com_phatware_writepad_WritePadAPI_freeRecognizer(JNIEnv* env,  jobject thiz, jstring pUserDict, jstring pLearner, jstring pCorrector)
{
	if (_recognizer != NULL)
    {
        if ( _szPath[0] == 0 )
        {
            HWR_FreeRecognizer(_recognizer, NULL, NULL, NULL );
        }
        else
        {
            char userDict[MAX_PATH];
            char learner[MAX_PATH];
            char corrector[MAX_PATH];

            userDict[0] = 0;
            learner[0] = 0;
            corrector[0] = 0;
            strcpy(userDict, _szPath);
            strcpy(learner, _szPath);
            strcpy(corrector, _szPath);

            const char * t = JstringToStringA(env, pUserDict);
            if ( NULL != t )
            {
                strcat(userDict, t );
                free( (void *)t );
            }
            t = JstringToStringA(env, pLearner);
            if ( NULL != t )
            {
                strcat(learner, t );
                free( (void *)t );
            }
            t = JstringToStringA(env, pCorrector);
            if ( NULL != t )
            {
                strcat(corrector, t );
                free( (void *)t );
            }
            
            HWR_FreeRecognizer(_recognizer, userDict, learner, corrector);
        }
		_recognizer = NULL;
	}
	if (_inkData != NULL)
    {
		INK_FreeData(_inkData);
		_inkData = NULL;
	}
	_currentStroke = -1;
}



jint  Java_com_phatware_writepad_WritePadAPI_getRecognizerFlags(JNIEnv * env) {
	jint result = 0;
	if (_recognizer != NULL) {
		result = HWR_GetRecognitionFlags(_recognizer);
	}
	return result;
}



void Java_com_phatware_writepad_WritePadAPI_setRecognizerFlags(JNIEnv * env, jobject thiz, jint flags) {
	if (_recognizer != NULL) {
            HWR_SetRecognitionFlags(_recognizer, (unsigned int) flags);
	}
}


jint Java_com_phatware_writepad_WritePadAPI_getRecognizerMode(JNIEnv * env) {
	jint result = 0;
	if (_recognizer != NULL) {
		result = HWR_GetRecognitionMode(_recognizer);
	}
	return result;
}

void Java_com_phatware_writepad_WritePadAPI_setRecognizerMode(JNIEnv * env, jobject thiz, jint mode) {
	if (_recognizer != NULL) {
		HWR_SetRecognitionMode(_recognizer, mode);
	}
}

void Java_com_phatware_writepad_WritePadAPI_stopRecognizer(JNIEnv* env) {
	if (_recognizer != NULL) {
		HWR_StopAsyncReco(_recognizer);
	}
}

jstring Java_com_phatware_writepad_WritePadAPI_recognizeInkData(JNIEnv* env, jobject thiz, jint nDataLen, jboolean bAsync, jboolean bFlipY, jboolean bSort )
{
	jstring result = NULL;
	const UCHR * recognizedText = NULL;
	if (_recognizer == NULL || _inkData == NULL || INK_StrokeCount(_inkData, false) < 1)
		return NULL;
	INK_DATA_PTR inkCopy = NULL;
	if (bAsync)
    {
		// create ink copy before starting recognizer.
		inkCopy = INK_CreateCopy(_inkData);
	}
	recognizedText = HWR_RecognizeInkData(_recognizer,
			(inkCopy == NULL) ? _inkData : inkCopy, 0, nDataLen, bAsync, bFlipY,
			bSort, false);
	if (NULL != inkCopy)
    {
		INK_FreeData(inkCopy);
		inkCopy = NULL;
	}
	if (recognizedText == NULL || *recognizedText == 0)
		return NULL;

	result = StringToJstring(env, recognizedText);
	if ((long) result == (-1))
		return NULL;
	return result;
}

jboolean Java_com_phatware_writepad_WritePadAPI_preRecognizeInkData(JNIEnv* env, jobject thiz, jint nDataLen) {
	const char * recognizedText = NULL;
	if (_recognizer == NULL || _inkData == NULL || INK_StrokeCount(_inkData, false) < 1)
		return false;
	return HWR_PreRecognizeInkData(_recognizer, _inkData, nDataLen, false);
}

jboolean Java_com_phatware_writepad_WritePadAPI_newStroke(JNIEnv * env, jobject thiz, jfloat width, jint color) {
	jint result = -1;
	if (NULL != _inkData) {
		result = INK_AddEmptyStroke(_inkData, width, (COLORREF) color);
	}
	_currentStroke = (int) result;
	return result;
}


jint Java_com_phatware_writepad_WritePadAPI_getStrokeCount(JNIEnv * env) {
	jint result = -1;
	if (NULL != _inkData) {
		result = INK_StrokeCount(_inkData, false);
	}
	return result;
}

jfloatArray Java_com_phatware_writepad_WritePadAPI_getStrokePoint( JNIEnv* env, jobject thiz, jint nStroke, jint nPixels )
{
    jfloatArray result = NULL;
	if ( _inkData != NULL )
	{
        float   ret[2];
        int     pressure = 0;
        jboolean r = INK_GetStrokePointP( _inkData, nStroke, nPixels, &ret[0], &ret[1], &pressure );
        if ( r )
        {
            result = (*env)->NewFloatArray(env, 2);
            (*env)->SetFloatArrayRegion( env, result, 0, 2, ret);
        }
    }   
    return result;
}

jboolean Java_com_phatware_writepad_WritePadAPI_deleteLastStroke(JNIEnv * env) {
	jboolean result = false;
	if (NULL != _inkData) {
		result = INK_DeleteStroke(_inkData, -1);
	}
	return result;
}

jboolean Java_com_phatware_writepad_WritePadAPI_deleteStroke(JNIEnv * env, jobject thiz, jint nStroke )
{
	jboolean result = false;
	if (NULL != _inkData) {
		result = INK_DeleteStroke(_inkData, nStroke );
	}
	return result;
}

jint Java_com_phatware_writepad_WritePadAPI_detectGesture(JNIEnv* env, jobject thiz, jint type)
{
	jint result = GEST_NONE;

	if (NULL != _inkData)
    {
		int nCnt = INK_StrokeCount(_inkData, false);
		if (nCnt > 0)
        {
			CGStroke pStroke = NULL;
			int len = INK_GetStrokeP(_inkData, nCnt - 1, &pStroke, NULL, NULL);
			if (len > 5) {
				result = HWR_CheckGesture((GESTURE_TYPE) type, pStroke, len, 1, 100);
			}
			if (pStroke != NULL)
				free((void *) pStroke);
		}
	}
	return result;
}

jint Java_com_phatware_writepad_WritePadAPI_addPixelToStroke(JNIEnv * env, jobject thiz, jint stroke, jfloat x, jfloat y) {
	jint result = -1;
	if (NULL != _inkData) {
		result = INK_AddPixelToStroke(_inkData, stroke, x, y, DEFAULT_PRESSURE );
	}
	return result;
}

jboolean Java_com_phatware_writepad_WritePadAPI_resetRecognizer(JNIEnv * env) {
	if (NULL != _recognizer) {
		HWR_Reset(_recognizer);
		return true;
	}
	return false;
}

jboolean Java_com_phatware_writepad_WritePadAPI_resetInkData(JNIEnv * env) {
	if (NULL != _inkData) {
		INK_Erase(_inkData);
		_currentStroke = -1;
		return true;
	}
	return false;
}

jstring Java_com_phatware_writepad_WritePadAPI_languageName(JNIEnv* env, jobject thiz)
{
	jstring result = StringToJstringA(env, HWR_GetLanguageName(_recognizer));
	return result;
}

jint Java_com_phatware_writepad_WritePadAPI_languageID(JNIEnv * env)
{
	return HWR_GetLanguageID(_recognizer);
}

jboolean Java_com_phatware_writepad_WritePadAPI_newUserDict(JNIEnv * env)
{
	if (NULL != _recognizer) {
		return HWR_NewUserDict(_recognizer);
	}
	return false;
}

jboolean Java_com_phatware_writepad_WritePadAPI_addWordToUserDict(JNIEnv * env, jobject thiz, jstring jword) {
	jboolean result = false;
	if (NULL != _recognizer)
    {
		const UCHR * strWord = JstringToString(env, jword);
		if (strWord != NULL)
        {
			result = HWR_AddUserWordToDict(_recognizer, strWord, true);
			free((void *) strWord);
		}
	}
	return result;
}

jboolean Java_com_phatware_writepad_WritePadAPI_resetResult(JNIEnv * env, jobject thiz) {
	jboolean result = false;
	if (NULL != _recognizer) {
		result = HWR_EmptyWordList(_recognizer);
	}
	return result;
}

jboolean Java_com_phatware_writepad_WritePadAPI_isWordInDict(JNIEnv * env, jobject thiz, jstring jword) {
	jboolean result = false;
	if (_recognizer != NULL) {
		const UCHR * word = JstringToString(env, jword);
		if (word != NULL) {
			result = HWR_IsWordInDict(_recognizer, word);
			free((void *) word);
		}
	}
	return result;
}

jint Java_com_phatware_writepad_WritePadAPI_getResultColumnCount(JNIEnv * env, jobject thiz) {
	jint result = 0;
	if (NULL != _recognizer) {
		result = HWR_GetResultWordCount(_recognizer);
	}
	return result;
}

jint Java_com_phatware_writepad_WritePadAPI_getResultRowCount(JNIEnv * env, jobject thiz, jint col) {
	jint result = 0;
	if (NULL != _recognizer) {
		result = HWR_GetResultAlternativeCount(_recognizer, col);
	}
	return result;
}


jstring Java_com_phatware_writepad_WritePadAPI_getRecognizedWord(JNIEnv * env, jobject thiz, jint col, jint row) {
	jstring result = NULL;
	if (NULL != _recognizer)
    {
		const UCHR * word = HWR_GetResultWord(_recognizer, col, row);
		if (word != NULL) 
        {
			result = StringToJstring(env, word);
			if ((long) result == (-1))
				result = NULL;
		}
	}
	return result;
}

jstring Java_com_phatware_writepad_WritePadAPI_getAutocorrectorWord(JNIEnv * env, jobject thiz, jstring inWord)
{
	jstring result = NULL;
	if (_recognizer != NULL)
    {
		const UCHR * word = JstringToString(env, inWord);
		if (word != NULL)
        {
			const UCHR * outWord = HWR_AutocorrectWord(_recognizer, word);
			if (NULL != outWord)
            {
				result = StringToJstring(env, outWord);
				if ((long) result == (-1))
					result = NULL;
			}
			free((void *) word);
		}
	}
	return result;
}

jint Java_com_phatware_writepad_WritePadAPI_getRecognizedWeight(JNIEnv * env, jobject thiz, jint col, jint row) {
	jint result = 0;
	if (NULL != _recognizer) {
		result = HWR_GetResultWeight(_recognizer, col, row);
	}
	return result;
}

jboolean Java_com_phatware_writepad_WritePadAPI_learnerAddNewWord(JNIEnv * env, jobject thiz, jstring jword, jint weight) {
	jboolean result = false;
	if (NULL != _recognizer) {
		const UCHR * word = JstringToString(env, jword);
		if (word != NULL) {
			result = HWR_LearnNewWord(_recognizer, word, weight);
			free((void *) word);
		}
	}
	return result;
}

jboolean Java_com_phatware_writepad_WritePadAPI_learnerReplaceWord(JNIEnv * env, jobject thiz, jstring jword1, jint weight1,
		jstring jword2, jint weight2) {
	jboolean result = false;
	if (NULL != _recognizer)
    {
		const UCHR * word1 = JstringToString(env, jword1);
		const UCHR * word2 = JstringToString(env, jword2);
		if (word1 != NULL && word2 != NULL)
        {
			result = HWR_ReplaceWord(_recognizer, word1, weight1, word2, weight2);
			free((void *) word1);
			free((void *) word2);
		}
	}
	return result;
}

jboolean Java_com_phatware_writepad_WritePadAPI_autocorrectorLearnWord(JNIEnv * env, jobject thiz, jstring jword1, jstring jword2, jint flags,
		jboolean bReplace) {
	jboolean result = false;
	if (NULL != _recognizer)
    {
		const UCHR * word1 = JstringToString(env, jword1);
		const UCHR * word2 = JstringToString(env, jword2);
		if (word1 != NULL && word2 != NULL)
        {
			result = HWR_AddWordToWordList(_recognizer, word1, word2, flags, bReplace);
			free((void *) word1); 
			free((void *) word2);
		}
	}
	return result;
}

jstring Java_com_phatware_writepad_WritePadAPI_spellCheckWord(JNIEnv * env, jobject thiz, jstring jword, jboolean showlist )
{
    const UCHR * strWord = JstringToString(env, jword);
    if (strWord == NULL)
    {
        return jword;
    }
	jstring newWordFrom = NULL;
	UCHR * pWordList = malloc( MAX_STRING_BUFFER * sizeof( UCHR ) );
    if ( NULL != pWordList )
    {
        memset( pWordList, 0, MAX_STRING_BUFFER * sizeof( UCHR ) );
        
        int flag = (showlist) ? HW_SPELL_LIST : 0;
        if ( HWR_SpellCheckWord( _recognizer, strWord, pWordList, MAX_STRING_BUFFER - 1, flag ) == 0 )
        {
            newWordFrom = StringToJstring(env, &pWordList[0]);
        }
        free( (void *) pWordList );
        free((void *) strWord);
    }
    if ( newWordFrom == NULL )
        return jword;
    return newWordFrom;
}


JNIEnv* myEnv;

const char *kInterfacePath = "com/phatware/writepad/WritePadAPI";


static JavaVM* gJavaVM;
static jobject gInterfaceObject;

int status;
int isAttached;

jclass interfaceClass;
jmethodID method;

 int EnumWordListCallback(const UCHR * szWordFrom, const UCHR * szWordTo, unsigned int nFlags, void * pParam) {
	JNIEnv *env;
	isAttached = 0;

	status = (*gJavaVM)->GetEnv(gJavaVM, (void **) &env, JNI_VERSION_1_4);
	if (status < 0) {
		status = (*gJavaVM)->AttachCurrentThread(gJavaVM, &env, NULL);
		if (status < 0) {
			return 0;
		}
		isAttached = 1;
	}
    if (interfaceClass  == NULL){
	    interfaceClass = (*env)->GetObjectClass(env, gInterfaceObject);
	}

	if (!interfaceClass) {
		// __android_log_print(ANDROID_LOG_INFO, "callback_handler"," failed to get class reference");
		if (isAttached == 1)
			(*gJavaVM)->DetachCurrentThread(gJavaVM);
		return 0;
	}
	/* Find the callBack method ID */

	if (method == NULL)
	{
	    method = (*env)->GetStaticMethodID(env, interfaceClass, "onEnumWord", "(Ljava/lang/String;Ljava/lang/String;I)V");
	}

	if (!method) {
		// __android_log_print(ANDROID_LOG_INFO, "callback_handler"," failed to get method ID");
		if (isAttached == 1) {
			(*gJavaVM)->DetachCurrentThread(gJavaVM);
		}
		return 0;
	}

    jstring newWordFrom = StringToJstring(env, szWordFrom);
    jstring newWordTo = StringToJstring(env, szWordTo);

    (*env)->CallStaticVoidMethod(env, interfaceClass, method, newWordFrom, newWordTo, nFlags);
    (*env)->DeleteLocalRef(env, newWordFrom);
    (*env)->DeleteLocalRef(env, newWordTo);

	if (isAttached == 1) {
		(*gJavaVM)->DetachCurrentThread(gJavaVM);
	}

	return 1;
}

int EnumUserWordsCallback(const UCHR * szWord, void * pParam) {
	JNIEnv *env;
	isAttached = 0;

	status = (*gJavaVM)->GetEnv(gJavaVM, (void **) &env, JNI_VERSION_1_4);
	if (status < 0) {
		//__android_log_print(ANDROID_LOG_INFO, "callback_handler: failed to get JNI environment, assuming native thread");
		status = (*gJavaVM)->AttachCurrentThread(gJavaVM, &env, NULL);
		if (status < 0) {
			//__android_log_print(ANDROID_LOG_INFO, "callback_handler: failed to attach ", "current thread");
			return 0;
		}
		isAttached = 1;
	}
    if (interfaceClass == NULL){
	    interfaceClass = (*env)->GetObjectClass(env, gInterfaceObject);
	}

	if (!interfaceClass) {
		//__android_log_print(ANDROID_LOG_INFO, "callback_handler"," failed to get class reference");
		if (isAttached == 1)
			(*gJavaVM)->DetachCurrentThread(gJavaVM);
		return 0;
	}

	if (method == NULL)
	{
	    method = (*env)->GetStaticMethodID(env, interfaceClass, "onEnumUserWords", "(Ljava/lang/String;)V");
	}

	if (!method) {
		// __android_log_print(ANDROID_LOG_INFO, "callback_handler"," failed to get method ID");
		if (isAttached == 1) {
			(*gJavaVM)->DetachCurrentThread(gJavaVM);
		}
		return 0;
	}

    jstring newWord = StringToJstring(env, szWord);
    (*env)->CallStaticVoidMethod(env, interfaceClass, method, newWord);
    (*env)->DeleteLocalRef(env, newWord);

	if (isAttached == 1) {
		(*gJavaVM)->DetachCurrentThread(gJavaVM);
	}

	return 1;
}

jint Java_com_phatware_writepad_WritePadAPI_getEnumUserWordsList(JNIEnv * env, jobject thiz, void * param) {
	jint result = 0;
	if (NULL != _recognizer) {
        interfaceClass = NULL;
        method = NULL;
		result = HWR_EnumUserWords(_recognizer, EnumUserWordsCallback, param);
	}
	return result;
}

jint Java_com_phatware_writepad_WritePadAPI_getEnumWordList(JNIEnv * env, jobject thiz, void * param) {
	jint result = 0;
	if (NULL != _recognizer) {
        interfaceClass = NULL;
        method = NULL;
		result = HWR_EnumWordList(_recognizer, EnumWordListCallback, param);
	}
	return result;
}


 jboolean Java_com_phatware_writepad_WritePadAPI_isLanguageSupported(JNIEnv * env, jobject thiz, jint langID) {
    return HWR_IsLanguageSupported(langID);
 }


 void initClassHelper(JNIEnv *env, const char *path, jobject *objptr) {
	jclass cls = (*env)->FindClass(env, path);
	if (!cls) {
		//__android_log_print(ANDROID_LOG_INFO, "initClassHelper: failed to get %s class reference", path);
		return;
	}
	jmethodID constr = (*env)->GetMethodID(env, cls, "<init>", "()V");
	if (!constr) {
		//__android_log_print(ANDROID_LOG_INFO, "initClassHelper: failed to get %s constructor", path);
		return;
	}
	jobject obj = (*env)->NewObject(env, cls, constr);
	if (!obj) {
		//__android_log_print(ANDROID_LOG_INFO, "initClassHelper: failed to create a %s object", path);
		return;
	}
	(*objptr) = (*env)->NewGlobalRef(env, obj);
}

jint JNI_OnLoad(JavaVM* vm, void* reserved) {
	JNIEnv *env;
	gJavaVM = vm;
	if ((*vm)->GetEnv(vm, (void**) &env, JNI_VERSION_1_4) != JNI_OK)
    {
		//__android_log_print(ANDROID_LOG_INFO, "(Failed"," to get the environment using GetEnv()");
		return -1;
	}

	initClassHelper(env, kInterfacePath, &gInterfaceObject);
	return JNI_VERSION_1_4;
}


const LIDBType * _lidb;
LIGStatesType		_groupstates;
LILetImgDrawType	_lidraw;
int width;
int height;
int offsetX = -11;
int offsetY = 80;

jobject Java_com_phatware_writepad_WritePadAPI_getLetterShapes(JNIEnv * env, jobject thiz, jstring letter, int right, int bottom)
{
	const char *classLetterManager = "com/phatware/writepad/preference/lettershapes/LetterManager";
    jclass clsLetterManager =              (*env)->FindClass(env, classLetterManager);
    jmethodID constructorClsLetterManager = (*env)->GetMethodID(env, clsLetterManager, "<init>", "()V");

    jmethodID beginLetterGroupCollections    = (*env)->GetMethodID(env, clsLetterManager,  "beginLetterGroupCollections", "(Ljava/lang/String;)V");
    jmethodID endLetterGroupCollections      = (*env)->GetMethodID(env, clsLetterManager,  "endLetterGroupCollections",   "()V");
    jmethodID beginStroke                    = (*env)->GetMethodID(env, clsLetterManager,  "beginStroke",            "()V");
    jmethodID endStroke                      = (*env)->GetMethodID(env, clsLetterManager,  "endStroke",              "()V");
    jmethodID beginLetterGroup               = (*env)->GetMethodID(env, clsLetterManager,  "beginLetterGroup",       "(IIFFFF)V");
    jmethodID endLetterGroup                 = (*env)->GetMethodID(env, clsLetterManager,  "endLetterGroup",         "()V");
    jmethodID beginLetter                    = (*env)->GetMethodID(env, clsLetterManager,  "beginLetter",            "()V");
    jmethodID endLetter                      = (*env)->GetMethodID(env, clsLetterManager,  "endLetter",              "()V");
    jmethodID add                            = (*env)->GetMethodID(env, clsLetterManager,  "add",                    "(FF)V");

    jobject letterManager = (*env)->NewObject(env, clsLetterManager, constructorClsLetterManager);

	LIRectType	ur, lr;
    lr.left = 0;
    lr.top = 0;
    lr.right = right;
    lr.bottom = bottom;
    lr.left = ORIGIN_LEFT(lr) + LI_LET_IMG_X;
    lr.right -= LI_LET_IMG_X_FROM_RIGHT;
    lr.top = LI_LET_IMG_Y;

    const UCHR * sLetter = JstringToString(env, letter);
    if ( sLetter != NULL )
    {
        CalcLetterLayout( &lr, sLetter[0], &_lidraw, _lidb, (const LIGStatesType *)&_groupstates );
        free( (void *)sLetter );
    }
    int i;
    LILayoutType *unsellt;

	for ( i = 0, unsellt = _lidraw.letimg; i < LI_ARRAY_LENGTH(_lidraw.letter) && _lidraw.letter[i] != 0; i++ )
	{
		char ch[3] = {0,')',0};
		ch[0] = (char)_lidraw.letter[i];

        int len = strlen(ch);
        unsigned short * uResult = (unsigned short *) malloc((len + 2) * sizeof(unsigned short));

	    if (uResult == NULL)
		    return NULL;

    	StrToUNICODE(uResult, ch, len);
    	const char * utfResult = UnicodeToUTF8(uResult);
	    (*env)->CallVoidMethod(env, letterManager, beginLetterGroupCollections, (*env)->NewStringUTF(env, utfResult));


		const LIInfoType *leti;
    	const LIVarType	 *letv;
    	const LILayoutType inLayout = _lidraw.letimg[i];
    	register int k;

/*--------------------------------draw letter layout-----------------------------------*/
        if ((leti = LIGetLetterInfo( _lidb, inLayout.letter)) == 0)
        {
            continue;
        }
        int previusGroup = -1;
        for ( k = 0; k < inLayout.numVar; k++)  //loop letters
        {
            (*env)->CallVoidMethod(env, letterManager, beginLetter);
            if ((letv = LIGetVariantInfo( _lidb, leti, inLayout.letterVar[k])) == 0)
            {
                continue;
            }

    /*-------------------------------------draw letter image---------------------------------------------------------------------------*/
            const LIRectType inDestRect = inLayout.letterRect[k];
            enum E_LI_LETSTATE  inLetState  = inLayout.letState[k];

            LIRectType  bbox;
            LIRectType  dest;
            LIRectType  screenRect;
            LIRectType  baseLineRect;
            LIPointType pt = { 0,0 };

            if ( letv == 0 || LIGetVariantBBox( _lidb, letv, &bbox ) == LIError)
            {
                continue;
            }

            int _width = LETTER_PEN_SIZE;

            dest = inDestRect;
            dest.left   += _width;
            dest.top    += _width;
            dest.right  -= _width;
            dest.bottom -= _width;

            if (CalculateScreenRect( &bbox, &dest, &screenRect ) == LIError || LIGetVariantBaseLine( _lidb, letv, &baseLineRect ) == LIError)
            {
                continue;
            }

        /*-------------------------------------draw single letter---------------------------------------------------------------------------*/


            int nstrk = LIGetNumStrokes(letv);
            int i;
            float letterLastX, letterLastY;
            for (  i = 0; i < nstrk; i++) //loop strokes
            {
                (*env)->CallVoidMethod(env, letterManager, beginStroke);
                const LIStokeType * lets;
                if ( (lets = LIGetStrokeInfo( _lidb, letv, i)) == 0 )
                {
                    continue;
                }
                int npts = LIGetNumPoints(lets);
                CGPoint * pts = (CGPoint * )malloc( sizeof( CGPoint ) * (npts+1) );
                int j;
                for ( j = 0; j < npts; j++)  // loop points
                {
                    LIPointType pt;
                    if ((pt.x = (int)LIGetPointX(lets,j)) < 0 || (pt.y = (int)LIGetPointY(lets,j)) < 0 || ConvertToScreenCoord( &pt, &bbox, &screenRect) == LIError)
                    {
                        continue;
                    }
                    letterLastX = pts[j].x = (CGFloat)pt.x;
                    letterLastY = pts[j].y = (CGFloat)pt.y;
                    (*env)->CallVoidMethod(env, letterManager, add, pts[j].x + offsetX, pts[j].y + offsetY);

                }
        		(*env)->CallVoidMethod(env, letterManager, endStroke);
        		free( (void *)pts );
            }
            int group;
            BOOL needToCloseGroup;


            if ((group = LIHitTestLetterLayout( &inLayout, letterLastX,letterLastY, 1)) != previusGroup){
                LIRectType r = inLayout.groupRect[group];
                CGRect groupRect = { r.left + offsetX, r.top + offsetY, r.right + offsetX, r.bottom + offsetY};
                if (LIGetSelectedGroup(unsellt) >= 0)
                {
                    LISelelectGroup(unsellt, -1);
                }

                int groupState = inLetState == LI_OFTEN ? 0 : inLetState == LI_SOMETIMES ? 1 : inLetState == LI_RARE ? 2 : inLetState;
                (*env)->CallVoidMethod(env, letterManager, beginLetterGroup, group, groupState, groupRect.origin.x, groupRect.origin.y, groupRect.size.width, groupRect.size.height);
                previusGroup = group;
                needToCloseGroup = previusGroup > - 1;

            } else {
                needToCloseGroup = false;
            }

            (*env)->CallVoidMethod(env, letterManager, endLetter);
            if (needToCloseGroup){
                (*env)->CallVoidMethod(env, letterManager, endLetterGroup);
            }


        }
        (*env)->CallVoidMethod(env, letterManager, endLetterGroupCollections);
    }
    return letterManager;
}

void Java_com_phatware_writepad_WritePadAPI_initLetterShapes(JNIEnv * env, jobject thiz)
{
     _lidb = (LIDBType *)LIGetLetterDB(  HWR_GetLanguageID( _recognizer )  );
     const unsigned char * shapes = HWR_GetLetterShapes( _recognizer );
     if ( NULL != shapes ){
            memcpy( _groupstates, shapes, LIG_STATES_SIZE );
     }
     memset( &_lidraw, 0, sizeof( _lidraw ) );
}

jstring Java_com_phatware_writepad_WritePadAPI_analyzeWordList(JNIEnv * env, jobject thiz, jstring jword)
{
    const UCHR * strWord = JstringToString(env, jword);
    if (strWord == NULL)
    {
        return jword;
    }
    int len = u_strlen( strWord );
	jstring newWordFrom = NULL;
    UCHR * pWordList = malloc( MAX_STRING_BUFFER * sizeof( UCHR ));
    if ( NULL != pWordList )
    {
        memset( pWordList, 0, MAX_STRING_BUFFER * sizeof( UCHR ) );
        
        if ( HWR_AnalyzeWordList( _recognizer, strWord, pWordList ) == 0 )
        {
            newWordFrom = StringToJstring(env, &pWordList[0]);
        }
        free( (void *) pWordList );
    }
    free((void *) strWord);
    if ( newWordFrom == NULL )
        return jword;
    return newWordFrom;
}

jstring Java_com_phatware_writepad_WritePadAPI_wordEnsureLowerCase(JNIEnv * env, jobject thiz, jstring inWord)
{
    const UCHR * cword = JstringToString(env, inWord);
    jstring result = StringToJstring( env, HWR_WordEnsureLowerCase(_recognizer, cword) );
    return result;
}

jbyteArray Java_com_phatware_writepad_WritePadAPI_getLetterGroupStates(JNIEnv * env, jobject thiz)
{
    jbyte * groupDataArray = malloc( LIG_STATES_SIZE );
    if ( NULL != groupDataArray )
    {
        jbyteArray data = (*env)->NewByteArray(env, LIG_STATES_SIZE );
        memmove( groupDataArray, (const void *)_groupstates, LIG_STATES_SIZE );
        (*env)->SetByteArrayRegion( env, data, 0,  LIG_STATES_SIZE, groupDataArray );
        free( (void *)groupDataArray );
        return data;
    }
    return NULL;
}

jboolean Java_com_phatware_writepad_WritePadAPI_saveShapes(JNIEnv * env, jobject thiz)
{
    return HWR_SetLetterShapes( _recognizer, _groupstates );
}

void Java_com_phatware_writepad_WritePadAPI_touchGroupLetter(JNIEnv * env, jobject thiz, jfloat x, jfloat y)
{

  	int		k, i, group = -1;
  	LILayoutType * layout;
  	x -= offsetX;
  	y -= offsetY;

  	for ( i = 0, layout = _lidraw.letimg;  i < LI_ARRAY_LENGTH(_lidraw.letter) && _lidraw.letter[i] != 0; i++, layout++)
      {
  		if ( (group = LIHitTestLetterLayout( layout, (int)x, (int)y, 1)) >= 0 )
        {
  			break;
  		}
  	}

  	if ( group >= 0 )
  	{
  		if ( group == LIGetSelectedGroup( layout ) )
  		{
  			int letter = -1;
  			if ( (letter = LIHitTestLetterLayout( layout, (int)x, (int)y, 0)) >= 0 )
  			{
  				SelectNextGroupDtate( _lidb, &_lidraw, &_groupstates );

  			}
  		}
        else
        {
            LILayoutType   *unsellt;
            for ( k = 0, unsellt = _lidraw.letimg; k < LI_ARRAY_LENGTH(_lidraw.letter) && _lidraw.letter[k] != 0; k++, unsellt++)
            {
                if (LIGetSelectedGroup(unsellt) >= 0)
                {
                    LISelelectGroup(unsellt, -1);
                    break;
                }
            }

            LISelelectGroup( layout, group );
        }
  	}
}

jboolean  Java_com_phatware_writepad_WritePadAPI_exportWordList(JNIEnv* env, jobject thiz, jstring pExportFile)
{
    jboolean result = false;
	if (_recognizer != NULL)
    {
        jboolean isCopy = JNI_FALSE;
    	const char *fileName = (*env)->GetStringUTFChars(env, pExportFile, &isCopy);
		result = HWR_ExportWordList(_recognizer, fileName);
        if ( fileName && isCopy == JNI_TRUE)
            (*env)->ReleaseStringUTFChars(env, pExportFile, fileName);
	}
	return result;
}

jboolean Java_com_phatware_writepad_WritePadAPI_exportUserDictionary(JNIEnv* env, jobject thiz, jstring pExportFile)
{
    jboolean result = false;
	if (_recognizer != NULL)
    {
        jboolean isCopy = JNI_FALSE;
    	const char *fileName = (*env)->GetStringUTFChars(env, pExportFile, &isCopy );
		result = HWR_ExportUserDictionary(_recognizer, fileName);
        if ( fileName && isCopy == JNI_TRUE)
            (*env)->ReleaseStringUTFChars(env, pExportFile, fileName);
	}
	return result;
}

jboolean Java_com_phatware_writepad_WritePadAPI_importWordList(JNIEnv* env, jobject thiz, jstring pImportFile)
{
    jboolean result = false;
	if (_recognizer != NULL)
    {
        jboolean isCopy = JNI_FALSE;
    	const char *fileName = (*env)->GetStringUTFChars(env, pImportFile, &isCopy);
		result = HWR_ImportWordList(_recognizer, fileName);
        if ( fileName && isCopy == JNI_TRUE)
            (*env)->ReleaseStringUTFChars(env, pImportFile, fileName);
	}
	return result;
}

jboolean Java_com_phatware_writepad_WritePadAPI_importUserDictionary(JNIEnv* env, jobject thiz, jstring pImportFile)
{
    jboolean result = false;
	if (_recognizer != NULL)
    {
        jboolean isCopy = JNI_FALSE;
    	const char *fileName = (*env)->GetStringUTFChars( env, pImportFile, &isCopy);
		result = HWR_ImportUserDictionary(_recognizer, fileName);
        if ( fileName && isCopy == JNI_TRUE)
            (*env)->ReleaseStringUTFChars(env, pImportFile, fileName);
	}
	return result;
}

static BOOL stringcaseiqual( CUCHR * str1, CUCHR * str2 )
{
    while( *str1 != 0 && *str2 != 0 )
    {
        if ( tolower( *str1 ) != tolower( *str2 ) )
            return false;
        str1++;
        str2++;
    }
    return ( *str1 == *str2 ) ? true : false;
}

jint Java_com_phatware_writepad_WritePadAPI_isCommand( JNIEnv* env, jobject thiz, jstring jcommand )
{
    jint result = -1;
	if (NULL != _recognizer)
    {
		int cnt = HWR_GetResultWordCount( _recognizer );
        if ( cnt == 1 )
        {
            // if only single word was recognized
            result = 0;
            const UCHR * strCmd = JstringToString(env, jcommand);
            if ( strCmd != NULL )
            {
                int alt = HWR_GetResultAlternativeCount( _recognizer, 0 );
                if ( alt > MAX_CMD_WORDS )
                    alt = MAX_CMD_WORDS;
                int i;
                for ( i = 0; i < alt; i++ )
                {
                    const UCHR * word = HWR_GetResultWord( _recognizer, 0, i );
                    if ( NULL != word && stringcaseiqual( strCmd, word ) )
                    {
                        result = 1;
                        break;
                    }
                }
                free( (void *)strCmd );
            }
        }
	}
    return result;
}

