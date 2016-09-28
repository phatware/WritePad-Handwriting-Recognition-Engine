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

#ifndef RECODICT_H
#define RECODICT_H

#ifndef _OS_LINUX
#include <windows.h>
#else // _OS_LINUX

#define FAR  
#define WINAPI
#define LPCTSTR unsigned short *
#define BOOL int 
#define UCHAR unsigned short
#define TCHAR unsigned short

#endif // _OS_LINUX

#include "reco.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef RECOHDICT (WINAPI RECO_DICTCREATE)( LPCTSTR szFileName, BOOL bMustExist );
typedef RECO_DICTCREATE FAR * LPRECO_DICTCREATE;

typedef BOOL (WINAPI RECO_DICTCLOSE)( RECOHDICT *phDict, LPCTSTR szFileName  );
typedef RECO_DICTCLOSE FAR * LPRECO_DICTCLOSE;

typedef BOOL (WINAPI RECO_DICTSTESTWORD)( RECOHDICT hMainDict, RECOHDICT hUserDict, UCHR *pWord, int iLen);
typedef RECO_DICTSTESTWORD FAR * LPRECO_DICTSTESTWORD;

typedef BOOL (WINAPI RECO_RECOSPELLCHECKWORD)( CUCHR * wrd, UCHR * ans, int buf_len, RECOHDICT h_main_dict, RECOHDICT h_user_dict, int flags );
typedef RECO_RECOSPELLCHECKWORD FAR * LPRECO_RECOSPELLCHECKWORD;

typedef BOOL (WINAPI RECO_DICTADDWORD)( RECOHDICT *phDict, TCHAR *pDictFileName, UCHR *pWord, int iLen );
typedef RECO_DICTADDWORD FAR * LPRECO_DICTADDWORD;

typedef BOOL (WINAPI RECO_DICTSAVE)( RECOHDICT hDict, LPCTSTR szFileName, BOOL bSaveAnyway );
typedef RECO_DICTSAVE FAR * LPRECO_DICTSAVE;

typedef int (WINAPI RECO_PZDICTCREATEDICT)( void * * pd );
typedef RECO_PZDICTCREATEDICT FAR * LPRECO_PZDICTCREATEDICT;

typedef int (WINAPI RECO_PZDICTFREEDICT)( void * * pd );
typedef RECO_PZDICTFREEDICT FAR * LPRECO_PZDICTFREEDICT;

typedef int (WINAPI RECO_PZDICTADDWORD)( CUCHR word, unsigned char attr, void * * pd );
typedef RECO_PZDICTADDWORD FAR * LPRECO_PZDICTADDWORD;

typedef BOOL (WINAPI RECO_LOOKININTERNETDICT)( LPCTSTR );
typedef RECO_LOOKININTERNETDICT FAR * LPRECO_LOOKININTERNETDICT;

typedef BOOL (WINAPI RECO_UPDATEINTERNETDICT)( void * * pd );
typedef RECO_UPDATEINTERNETDICT FAR * LPRECO_UPDATEINTERNETDICT;

typedef BOOL (WINAPI RECO_ADDUSERWORDS)( void * * pd, void * d );
typedef RECO_ADDUSERWORDS FAR * LPRECO_ADDUSERWORDS;

BOOL     DLLEXP DictSave( RECOHDICT hDict, LPCTSTR szFileName, BOOL bSaveAnyway );
RECOHDICT DLLEXP DictCreate( LPCTSTR szFileName, BOOL bMustExist );
BOOL     DLLEXP DictClose( RECOHDICT *phDict, LPCTSTR szFileName );
BOOL     DLLEXP DictAddWord( RECOHDICT *phDict, TCHAR *pDictFileName, UCHR *pWord, int iLen );
BOOL     DLLEXP DictsTestWord( RECOHDICT hMainDict, RECOHDICT hUserDict, UCHR *pWord, int iLen);
BOOL     DLLEXP LookInInternetDict( LPCTSTR szAlt );
int      DLLEXP UpdateInternetDict( RECOHDICT *phDict );
int      DLLEXP AddUserWords( RECOHDICT * phDict, RECOHDICT hUser );

#ifdef __cplusplus
}
#endif

#endif // RECODICT_H
