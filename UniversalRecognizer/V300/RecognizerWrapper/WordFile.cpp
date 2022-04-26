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

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "InternalTypes.h"
#include "RecognizerWrapper.h"
#include "WordFile.h"

typedef struct __file_header {
	DWORD	dwSize;
	DWORD	dwVersion;
    WORD    wLanguageID;
    DWORD   dwFlags;
    DWORD   dwSortOrder;
	DWORD	dwWordCount;
	DWORD	dwReserved1;
	DWORD	dwReserved2;
} FILE_HEADER, * PFILE_HEADER;

#ifdef HW_RECINT_UNICODE
#define FILE_VERSION	1007
#else
#define FILE_VERSION	1002
#endif

//////////////////////////////////////////////////////////////////////
// Class CWordMap methods
//////////////////////////////////////////////////////////////////////

BOOL CWordMap::Save( FILE * pFile ) const
{
	if ( NULL == m_pWordFind || NULL == m_pWordReplace )
	{
		// SetLastError( ERROR_BAD_ARGUMENTS );
		return false;
	}

    size_t nSize = 2 * sizeof( WORD ) + sizeof( DWORD ) + sizeof( USHORT );
    nSize += (sizeof( WCHAR ) * (_STRLEN( m_pWordFind ) + _STRLEN( m_pWordReplace ) + 2));
    LPBYTE  pBuff = new BYTE[nSize+2];
    if ( NULL == pBuff )
    {
		// SetLastError( ERROR_NOT_ENOUGH_MEMORY );
		return false;
    }
    WORD    sz = 0;
    LPBYTE  ptr = pBuff;
	sz = (WORD)(sizeof( WCHAR ) * (_STRLEN( m_pWordFind ) + 1));
    memcpy( ptr, &sz, sizeof( WORD ) );
    ptr += sizeof( WORD );
    memcpy( ptr, m_pWordFind, sz );
    ptr += sz;
    sz = (WORD)(sizeof( WCHAR ) * (_STRLEN( m_pWordReplace ) + 1));
    memcpy( ptr, &sz, sizeof( WORD ) );
    ptr += sizeof( WORD );
    memcpy( ptr, m_pWordReplace, sz );
    ptr += sz;
    memcpy( ptr, &m_dwFlags, sizeof( DWORD ) );
    ptr += sizeof( DWORD );
    memcpy( ptr, &m_nWeight, sizeof( USHORT ) );

    // write buffer
	if ( nSize > fwrite( pBuff, 1, nSize, pFile ) )
		return false;
    delete [] pBuff;
    return true;
}

size_t CWordMap::SaveToMemory( char * pData ) const
{
	if ( NULL == m_pWordFind || NULL == m_pWordReplace )
	{
		// SetLastError( ERROR_BAD_ARGUMENTS );
		return 0;
	}
	
    size_t nSize = 2 * sizeof( WORD ) + sizeof( DWORD ) + sizeof( USHORT );
	nSize += (sizeof( WCHAR ) * (_STRLEN( m_pWordFind ) + _STRLEN( m_pWordReplace ) + 2));

    if ( pData == NULL )
		return nSize;
	
	WORD    sz = 0;
    LPBYTE  ptr = (LPBYTE)pData;
	sz = (WORD)(sizeof( WCHAR ) * (_STRLEN( m_pWordFind ) + 1));
    memcpy( ptr, &sz, sizeof( WORD ) );
    ptr += sizeof( WORD );
    memcpy( ptr, m_pWordFind, sz );
    ptr += sz;
    sz = (WORD)(sizeof( WCHAR ) * (_STRLEN( m_pWordReplace ) + 1));
    memcpy( ptr, &sz, sizeof( WORD ) );
    ptr += sizeof( WORD );
    memcpy( ptr, m_pWordReplace, sz );
    ptr += sz;
    memcpy( ptr, &m_dwFlags, sizeof( DWORD ) );
    ptr += sizeof( DWORD );
    memcpy( ptr, &m_nWeight, sizeof( USHORT ) );
	
    return nSize;
}

BOOL CWordMap::Export( FILE * pFile ) const
{
    char *	pStrOut = new char[MAX_STRING_BUFFER];
	if ( NULL == pStrOut )
		return false;
    LPCWSTR pw = m_pWordFind;
    unsigned char *  pt = (unsigned char *)pStrOut;
    *pt++ = '\"';
    while( *pw )
    {
        if ( *pw == '\"' )
            *pt++ = '\"';
		*pt++ = (unsigned char)*pw++;
    }
    *pt++ = '\"';
    *pt++ = ',';
    *pt++ = '\"';
    pw = m_pWordReplace;
    while( *pw )
    {
        if ( *pw == '\"' )
            *pt++ = '\"';
        *pt++ = (unsigned char)*pw++;
    }
    *pt++ = '\"';
	*pt = 0;

    sprintf( (char *)pt, ",%d,%d,%d\r\n", (m_dwFlags & WCF_IGNORECASE) ? 1 : 0,
        (m_dwFlags & WCF_ALWAYS) ? 1 : 0, (m_dwFlags & WCF_DISABLED) ? 1 : 0 );

    _WORD     nSize = HWRStrLen( (_CSTR)pStrOut );
	if ( nSize > fwrite( pStrOut, 1, nSize, pFile ) )
		return false;
	delete [] pStrOut;
    return true;
}

BOOL CWordMap::Import( const char * pBuff )
{
    int     i = 0, nCol = 0;
    BOOL    bQuote = false;
    char    pStrIn[2 * HW_MAXWORDLEN];

    Init();

    for ( const char *  p = pBuff; *p != 0; p++ )
    {
        if ( *p == '\"' )
        {
            if ( ! bQuote )
                bQuote = true;
            else
            {
                if ( *(p+1) == '\"' )
                {
                    pStrIn[i++] = '\"';
                    p++;
                }
                else
                {
                    bQuote = false;
                }
            }
        }
        else if ( *p == ',' && ! bQuote )
        {
            if ( i > 0 )
            {
                if ( i >= HW_MAXWORDLEN )
                    i = HW_MAXWORDLEN-1;
                pStrIn[i] = 0;

                switch( nCol )
                {
                    case 0 :
                        m_pWordFind = _STRDUP( pStrIn );
                        break;

                    case 1 :
                        m_pWordReplace = _STRDUP( pStrIn );
                        break;

                    case 2 :
                        if ( pStrIn[0] == '1' )
                            m_dwFlags |= WCF_IGNORECASE;
                        else
                            m_dwFlags &= ~WCF_IGNORECASE;
                        break;

                    case 3 :
                        if ( pStrIn[0] == '1' )
                            m_dwFlags |= WCF_ALWAYS;
                        else
                            m_dwFlags &= ~WCF_ALWAYS;
                        break;

                    case 4 :
                        if ( pStrIn[0] == '1' )
                            m_dwFlags |= WCF_DISABLED;
                        else
                            m_dwFlags &= ~WCF_DISABLED;
                        break;

                    default :
                        return false;
                }
                nCol++;
            }
            i = 0;
        }
        else
        {
            pStrIn[i++] = *p;
        }
    }

    if ( NULL == m_pWordFind || NULL == m_pWordReplace )
        return false;
    return true;
}

BOOL CWordMap::Load( const LPBYTE pBuff, long &dwPos )
{
	Init();

	// get recognized word
	WORD    nSize = 0;
    memcpy( &nSize, &pBuff[dwPos], sizeof( WORD ) );
    dwPos += sizeof( WORD );
	if ( nSize < 2 || nSize > HW_MAXWORDLEN * sizeof( WCHAR ) )
	{
		// SetLastError( ERROR_BAD_FORMAT );
		return false;
	}
	m_pWordFind	= (LPWSTR)malloc( nSize+4 );
	if ( NULL == m_pWordFind )
	{
		// SetLastError( ERROR_NOT_ENOUGH_MEMORY );
		return false;
	}
    memcpy( m_pWordFind, &pBuff[dwPos], nSize );
    dwPos += nSize; 

    memcpy( &nSize, &pBuff[dwPos], sizeof( WORD ) );
    dwPos += sizeof( WORD );
	if ( nSize < 2 || nSize > HW_MAXWORDLEN * sizeof( WCHAR ) )
	{
		// SetLastError( ERROR_BAD_FORMAT );
		return false;
    }
	m_pWordReplace	= (LPWSTR)malloc( nSize+4 );
	if ( NULL == m_pWordReplace )
	{
		// SetLastError( ERROR_NOT_ENOUGH_MEMORY );
		return false;
    }
    memcpy( m_pWordReplace, &pBuff[dwPos], nSize );
    dwPos += nSize;

    memcpy( &m_dwFlags, &pBuff[dwPos], sizeof( DWORD ) );
    dwPos += sizeof( DWORD );
    memcpy( &m_nWeight, &pBuff[dwPos], sizeof( USHORT ) );
    dwPos += sizeof( USHORT );
	return true;
}

//////////////////////////////////////////////////////////////////////
// Class CWordList methods
//////////////////////////////////////////////////////////////////////

CWordList::CWordList()
{
    m_nCurrentSort = 0;
    m_nSortOrder = 0;
    m_wLanguageID = 0;
}

size_t CWordList::SaveToMemory( char ** ppData, WORD wLanguageID /* = 0*/ )
{
	*ppData = NULL;
	size_t nTotalLen = 0;
	CWordMap * pWM;
	for ( int dw = 0; dw < m_words.GetSize(); dw++ )
	{
		pWM = m_words[dw];
		if ( NULL != pWM )
		{
			nTotalLen += pWM->SaveToMemory( NULL );
		}
	}

	if ( nTotalLen < 1 )
		return 0;
	
	nTotalLen += sizeof( FILE_HEADER );
	char * pData = (char *)malloc( nTotalLen + 2 );
	if ( NULL == pData )
		return 0;

    if ( wLanguageID != 0 )
        m_wLanguageID = wLanguageID;
	
	FILE_HEADER	fh = {0};
	fh.dwSize = sizeof( FILE_HEADER );
	fh.dwVersion = FILE_VERSION;
    fh.wLanguageID = m_wLanguageID;
	fh.dwWordCount = m_words.GetSize();
    fh.dwSortOrder = MAKELONG( m_nCurrentSort, m_nSortOrder );

	memcpy( pData, &fh, sizeof( FILE_HEADER ) );
	size_t pos = sizeof( FILE_HEADER );
	
	if ( fh.dwWordCount > 0 )
	{
        CWordMap * pWM;
		for ( DWORD dw = 0; dw < fh.dwWordCount; dw++ )
		{
			pWM = m_words[(int)dw];
			if ( NULL != pWM )
			{
				pos += pWM->SaveToMemory( (char *)(pData + pos) );
			}
		}
	}
	
	*ppData = pData;	
	return nTotalLen;
}


BOOL CWordList::Save( const char * pszFileName, WORD wLanguageID /* = 0*/ )
{
	FILE *	file = fopen( pszFileName, "w+" );
	if ( file == NULL )
		return false;
	
    if ( wLanguageID != 0 )
        m_wLanguageID = wLanguageID;

	FILE_HEADER	fh = {0};
	fh.dwSize = sizeof( FILE_HEADER );
	fh.dwVersion = FILE_VERSION;
    fh.wLanguageID = m_wLanguageID;
	fh.dwWordCount = m_words.GetSize();
    fh.dwSortOrder = MAKELONG( m_nCurrentSort, m_nSortOrder );

	BOOL	bResult = false;
	if ( fwrite( &fh, 1, sizeof( FILE_HEADER ), file ) < sizeof( FILE_HEADER ) )
	{
		fclose( file );
		return false;
	}

	if ( fh.dwWordCount > 0 )
	{
        CWordMap * pWM;
		for ( int dw = 0; dw < (int)fh.dwWordCount; dw++ )
		{
			pWM = m_words[dw];
			if ( NULL != pWM )
			{
				bResult = pWM->Save( file );
				if ( ! bResult )
					break;
			}
		}
	}
	fclose( file );
	return bResult;
}

BOOL CWordList::LoadFromMemory( const char * pszInBuffer )
{
	FILE_HEADER	 fh;
	memcpy( &fh, pszInBuffer, sizeof( FILE_HEADER ) );
	BOOL		bResult = false;
	if ( fh.dwSize != sizeof( FILE_HEADER ) || fh.dwVersion != FILE_VERSION || fh.dwWordCount < 1 )
	{
		// invalid or empty data
		return false;
	}
	
	Free();
	
	// get dictionary file length
    LPBYTE  pBuff = (LPBYTE)(pszInBuffer + sizeof( FILE_HEADER ));
    if ( NULL == pBuff )
        return false;

    m_wLanguageID = fh.wLanguageID;
    m_nCurrentSort = LOWORD( fh.dwSortOrder );
    m_nSortOrder = HIWORD( fh.dwSortOrder );
	if ( fh.dwWordCount > 0 )
	{
        long   dwPos = 0;
        CWordMap * pWM = NULL;
		for ( DWORD dw = 0; dw < fh.dwWordCount; dw++ )
		{
			pWM = new CWordMap;
			if ( NULL != pWM )
			{
				bResult = pWM->Load( pBuff, dwPos );
				if ( ! bResult )
				{
					delete pWM;
					break;
				}
				m_words.Add( pWM );
			}
		}
	}
    if ( ! bResult )
        Free();
	return bResult;
}

BOOL CWordList::Load( const char * pszFileName )
{
	FILE *	file = fopen( pszFileName, "r" );
	if ( file == NULL )
		return false;

	fseek( file, 0, SEEK_END );
	long nLen = ftell( file );
	fseek( file, 0, SEEK_SET );
	
	if ( nLen <= (long)sizeof( FILE_HEADER ) )
	{
		// the file is empty
		fclose(file);
		return true;
	}	
	
	FILE_HEADER	fh = {0};
	BOOL		bResult = false;
	if ( fread( &fh, 1, sizeof( FILE_HEADER ), file ) < 1 )
	{
		fclose( file );
		return false;
	}
	if ( fh.dwSize != sizeof( FILE_HEADER ) || fh.dwVersion != FILE_VERSION )
	{
		fclose( file );
		// SetLastError( ERROR_BAD_FORMAT );
		return false;
	}

	Free();

	// get dictionary file length
    LPBYTE  pBuff = new BYTE[nLen+2];
    if ( NULL == pBuff )
    {
        fclose( file );
        // SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return false;
    }       
    nLen -= sizeof( FILE_HEADER );
	if ( (int)fread( pBuff, 1, nLen, file ) < nLen )
	{
        delete [] pBuff;
		fclose( file );
		return false;
	}
	fclose( file );

    m_wLanguageID = fh.wLanguageID;
    m_nCurrentSort = LOWORD( fh.dwSortOrder );
    m_nSortOrder = HIWORD( fh.dwSortOrder );
	if ( fh.dwWordCount > 0 )
	{
        long   dwPos = 0;
        CWordMap * pWM = NULL;
		for ( DWORD dw = 0; dw < fh.dwWordCount; dw++ )
		{
			pWM = new CWordMap;
			if ( NULL != pWM )
			{
				bResult = pWM->Load( pBuff, dwPos );
				if ( ! bResult )
				{
					delete pWM;
					break;
				}
				m_words.Add( pWM );
			}
            if ( nLen <= dwPos )
                break;
		}
	}
    if ( ! bResult )
        Free();
	delete [] pBuff;
	return bResult;
}

BOOL CWordList::Export( const char * pszFileName )
{
	if ( NULL == pszFileName || 0 == pszFileName[0] )
	{
		// SetLastError( ERROR_BAD_ARGUMENTS );
		return false;
	}
    if ( m_words.GetSize() < 1 )
    {
		// SetLastError( ERROR_NO_DATA );
        return false;
    }
	FILE *	file = fopen( pszFileName, "w+" );
	if ( file == NULL )
		return false;
	
    CWordMap *  pWM = NULL;
    BOOL        bResult = false;        
	for ( int dw = 0; dw < m_words.GetSize(); dw++ )
	{
		pWM = m_words[dw];
		if ( NULL != pWM )
		{
			bResult = pWM->Export( file );
			if ( ! bResult )
				break;
		}
	}
	fclose( file );
	return bResult;
}

BOOL CWordList::Import( const char * pszFileName )
{
	if ( NULL == pszFileName || 0 == pszFileName[0] )
	{
		// SetLastError( ERROR_BAD_ARGUMENTS );
		return false;
	}
	
	FILE *	file = fopen( pszFileName, "r" );
	if ( file == NULL )
		return true;
	
	fseek( file, 0, SEEK_END );
	long nLen = ftell( file );
	fseek( file, 0, SEEK_SET );

    LPBYTE  pBuff = new BYTE[nLen+2];

    if ( NULL == pBuff )
    {
        fclose( file );
        // SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return false;
    }       
	if ( fread( pBuff, 1, nLen, file ) < 1 )
	{
        delete [] pBuff;
		fclose( file );
		return false;
	}
	fclose( file );
	
    Free();
	
    char    szTmp[MAX_STRING_BUFFER];
    int     i = 0;
    CWordMap *  pWM = NULL;
    BOOL    bResult = true;

    for ( long dw = 0; dw < nLen; dw++ )
    {
        if ( pBuff[dw] == '\r' )
        {
            // no action
        }
        else if ( pBuff[dw] == '\n' )
        {
            if ( i > 1 )
            {
                szTmp[i++] = ',';   // add a comma at the end
                szTmp[i] = 0;
			    pWM = new CWordMap;
			    if ( NULL != pWM )
			    {
				    bResult = pWM->Import( szTmp );
				    if ( ! bResult )
				    {
					    delete pWM;
					    break;  // corrupted file
				    }
				    m_words.Add( pWM );
			    }
            }
            i = 0;
        }
        else
        {
            if ( i >= MAX_STRING_BUFFER-1 )
            {
                bResult = false;
                break;      // corrupted file
            }
            szTmp[i++] = pBuff[dw];
        }
    }

    if ( ! bResult )
        Free();

    delete [] pBuff;
    return bResult;
}

int	CWordList::EnumWordList( RECO_ONGOTWORDLIST callback, void * pParam )
{
	if ( m_words.GetSize() > 0 && NULL != callback )
	{
        CWordMap * pWM;
		for ( int dw = 0; dw < m_words.GetSize(); dw++ )
		{
			pWM = m_words[dw];
			(*callback)( pWM->m_pWordFind, pWM->m_pWordReplace, pWM->m_dwFlags, pParam );
		}
	}
	return m_words.GetSize();
}
