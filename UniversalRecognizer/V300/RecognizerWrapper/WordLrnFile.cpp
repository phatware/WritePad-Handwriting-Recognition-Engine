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
#include "WordLrnFile.h"

typedef struct {
    WCHAR   szWord[HW_MAXWORDLEN];
    USHORT  nWeight;
    int     nIndex;
    int     nProb;
    int     nLnk;
} ALTWORD;

typedef struct __file_header {
    DWORD	dwSize;
    DWORD	dwVersion;
    WORD    wLanguageID;
    DWORD   dwFlags;
    DWORD	dwWordCount;
    DWORD	dwReserved1;
    DWORD	dwReserved2;
} FILE_HEADER, * PFILE_HEADER;

#ifdef HW_RECINT_UNICODE
#define FILE_VERSION	4201
#else
#define FILE_VERSION	1002
#endif

/*
#define COEFF_USED          1.0
#define COEFF_WEIGHT        10.0
#define COEFF_LINKS         2.0
#define COEFF_DICTIONARY    1.0
*/

//////////////////////////////////////////////////////////////////////
// Class CWord methods
//////////////////////////////////////////////////////////////////////

BOOL CWordLrnFile::CWord::Save( FILE * pFile ) const
{
    if ( NULL == m_pWord )
    {
        // SetLastError( ERROR_BAD_ARGUMENTS );
        return false;
    }

    BYTE    sz = (BYTE)(sizeof( WCHAR ) * (_STRLEN( m_pWord ) + 1));
    UInt32  nSize = 2 * sizeof( USHORT ) + sizeof( BYTE ) + m_links.Save( NULL );
    nSize += (DWORD)sz;
    LPBYTE  pBuff = new BYTE[nSize+2];
    if ( NULL == pBuff )
    {
        // SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return false;
    }
    LPBYTE  ptr = pBuff;
    *ptr++ = sz;
    memcpy( ptr, m_pWord, sz );
    ptr += sz;
    memcpy( ptr, &m_nCount, sizeof( USHORT ) );
    ptr += sizeof( USHORT );

    USHORT  w = (m_nWeight & RW_WEIGHTMASK);
    if ( m_bDictionary )
        w |= RW_DICTIONARYWORD;
    memcpy( ptr, &w, sizeof( USHORT ) );
    ptr += sizeof( USHORT );
    m_links.Save( ptr );

    // write buffer
    if ( fwrite( pBuff, 1, nSize, pFile ) < nSize )
        return false;
    delete [] pBuff;
    return true;
}

int	CWordLrnFile::CWord::SaveToMemory( char * pData ) const
{
    if ( NULL == m_pWord )
    {
        // SetLastError( ERROR_BAD_ARGUMENTS );
        return 0;
    }
    
    BYTE    sz = (BYTE)(sizeof( WCHAR ) * (_STRLEN( m_pWord ) + 1));
    int  nSize = 2 * sizeof( USHORT ) + sizeof( BYTE ) + m_links.Save( NULL );
    nSize += (int)sz;
    
    if ( pData == NULL )
        return nSize;
    
    LPBYTE  ptr = (LPBYTE)pData;
    *ptr++ = sz;
    memcpy( ptr, m_pWord, sz );
    ptr += sz;
    memcpy( ptr, &m_nCount, sizeof( USHORT ) );
    ptr += sizeof( USHORT );
    
    USHORT  w = (m_nWeight & RW_WEIGHTMASK);
    if ( m_bDictionary )
        w |= RW_DICTIONARYWORD;
    memcpy( ptr, &w, sizeof( USHORT ) );
    ptr += sizeof( USHORT );
    m_links.Save( ptr );
    return nSize;
}

BOOL CWordLrnFile::CWord::Load( const LPBYTE pBuff, long &dwPos )
{
    Init();

    // get recognized word
    BYTE    nSize = 0;
    nSize = pBuff[dwPos];
    dwPos++;
    if ( nSize < 2 || nSize >= HW_MAXWORDLEN * sizeof( WCHAR ) )
    {
        // SetLastError( ERROR_BAD_FORMAT );
        return false;
    }
    m_pWord	= (LPWSTR)malloc( nSize+4 );
    if ( NULL == m_pWord )
    {
        // SetLastError( ERROR_NOT_ENOUGH_MEMORY );
        return false;
    }
    memcpy( m_pWord, &pBuff[dwPos], nSize );
    dwPos += nSize;
    memcpy( &m_nCount, &pBuff[dwPos], sizeof( USHORT ) );
    dwPos += sizeof( USHORT );
    memcpy( &m_nWeight, &pBuff[dwPos], sizeof( USHORT ) );
    dwPos += sizeof( USHORT );
    if ( 0 != (m_nWeight & RW_DICTIONARYWORD) )
        m_bDictionary = true;
    m_nWeight &= RW_WEIGHTMASK;

    // get word links
    dwPos += m_links.Load( &pBuff[dwPos] );
    return true;
}

//////////////////////////////////////////////////////////////////////
// Class CWordLrnFile methods
//////////////////////////////////////////////////////////////////////

CWordLrnFile::CWordLrnFile( WORD wLangID )
{
    m_wLanguageID = (0 == wLangID) ? 1 : wLangID;
    m_nCurrentSort = 0;
    m_nSortOrder = 0;
}

int CWordLrnFile::SaveToMemory( char ** ppData, WORD wLanguageID /* = 0*/ )
{
    *ppData = NULL;
    int nTotalLen = 0;
    CWord * pWM;
    for ( int dw = 0; dw < (int)m_words.GetSize(); dw++ )
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
    
    memcpy( pData, &fh, sizeof( FILE_HEADER ) );
    int pos = sizeof( FILE_HEADER );
    
    if ( fh.dwWordCount > 0 )
    {
        CWord * pWM;
        for ( int dw = 0; dw < (int)fh.dwWordCount; dw++ )
        {
            pWM = m_words[dw];
            if ( NULL != pWM )
            {
                pos += pWM->SaveToMemory( (char *)(pData + pos) );
            }
        }
    }
    
    *ppData = pData;	
    return nTotalLen;
}

BOOL CWordLrnFile::Save( const char * pszFileName, WORD wLanguageID /* = 0*/ )
{
    if ( NULL == pszFileName || 0 == *pszFileName )
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

    if ( wLanguageID != 0 )
        m_wLanguageID = wLanguageID;

    FILE_HEADER	fh = {0};
    fh.dwSize = sizeof( FILE_HEADER );
    fh.dwVersion = FILE_VERSION;
    fh.wLanguageID = m_wLanguageID;
    fh.dwWordCount = m_words.GetSize();

    BOOL	bResult = false;
    if ( fwrite( &fh, 1, sizeof( FILE_HEADER ), file ) < sizeof( FILE_HEADER ) )
    {
        fclose( file );
        return false;
    }

    if ( fh.dwWordCount > 0 )
    {
        CWord * pWM;
        for ( int dw = 0; dw < (int)fh.dwWordCount; dw++ )
        {
            if ( NULL != (pWM = m_words[dw]) )
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

BOOL CWordLrnFile::LoadFromMemory( const char * pData, WORD wLangID )
{
    if ( NULL == pData )
    {
        return false;
    }
    
    Free();
    
    FILE_HEADER	fh = {0};
    BOOL		bResult = false;
    memcpy( &fh, pData, sizeof( FILE_HEADER ) );	
    if ( fh.dwSize != sizeof( FILE_HEADER ) || fh.dwVersion != FILE_VERSION || fh.dwWordCount < 1 )
    {
        return false;
    }
    
    LPBYTE  pBuff = (LPBYTE)(pData + sizeof( FILE_HEADER ));
    m_wLanguageID = fh.wLanguageID;
    if ( fh.dwWordCount > 0 )
    {
        long  nPos = 0;
        CWord * pWM = NULL;
        for ( DWORD dw = 0; dw < fh.dwWordCount; dw++ )
        {
            pWM = new CWord;
            if ( NULL != pWM )
            {
                bResult = pWM->Load( pBuff, nPos );
                if ( ! bResult )
                {
                    delete pWM;
                    break;
                }
                m_words.Add( pWM );
            }
        }
    }
    delete [] pBuff;
    return bResult;
}

BOOL CWordLrnFile::Load( const char * pszFileName, WORD wLangID )
{
    if ( NULL == pszFileName || 0 == *pszFileName )
    {
        // SetLastError( ERROR_BAD_ARGUMENTS );
        return false;
    }
    
    FILE *	file = fopen( pszFileName, "r" );
    if ( file == NULL )
    {
        file = fopen( pszFileName, "w+" );
        if ( file == NULL )
            return false;
        Free();
        m_wLanguageID = wLangID;
        fclose( file );
        return true;
    }
    
    Free();
    
    // get dictionary file length
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

    LPBYTE  pBuff = new BYTE[nLen];
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
    if ( fh.dwWordCount > 0 )
    {
        long  nPos = 0;
        CWord * pWM = NULL;
        for ( DWORD dw = 0; dw < fh.dwWordCount; dw++ )
        {
            pWM = new CWord;
            if ( NULL != pWM )
            {
                bResult = pWM->Load( pBuff, nPos );
                if ( ! bResult )
                {
                    delete pWM;
                    break;
                }
                m_words.Add( pWM );
            }
            if ( nLen <= nPos )
                break;
        }
    }
    delete [] pBuff;
    return bResult;
}

BOOL CWordLrnFile::ReplaceWord( LPCWSTR pszWord1, USHORT nWeight1, LPCWSTR pszWord2, USHORT nWeight2 )
{
    USHORT  w1 = (nWeight1 & RW_WEIGHTMASK);
    USHORT  w2 = (nWeight2 & RW_WEIGHTMASK);
    int nIndex1 = GetWordIndex( pszWord1 );
    if ( nIndex1 < 0 )
        nIndex1 = AddWord( pszWord1, w1, 0 != (nWeight1&RW_DICTIONARYWORD) );
    int nIndex2 = GetWordIndex( pszWord2 );
    if ( nIndex2 < 0 )
        nIndex2 = AddWord( pszWord2, w2, 0 != (nWeight2&RW_DICTIONARYWORD) );

    if ( nIndex1 < 0 || nIndex2 < 0 )
        return false;

    CWord * pWM = m_words[nIndex1];
    int     nCnt = pWM->m_links.GetSize();
    DWORD   dw;

    for ( int i = 0; i < nCnt; i++ )
    {
        if ( (WORD)nIndex2 == LOWORD( pWM->m_links[i] ) )
        {
            WORD   w = HIWORD( pWM->m_links[i] );
            if ( w < MAX_WORD_COUNT )
                w++;
            pWM->m_links.SetAt( i, MAKELONG( (WORD)nIndex2, w ) );
            goto quit;
        }
    }

    dw = MAKELONG( (WORD)nIndex2, 1 );
    pWM->m_links.Add( dw );

quit:
    // update info: word 1
    if ( pWM->m_nCount > 1 )
        pWM->m_nCount--;
    if ( w1 > MIN_RECOGNITION_WEIGHT )
        pWM->m_nWeight = (w1 + pWM->m_nWeight)/2;

    // update info: word 2
    pWM = m_words[nIndex2];
    if ( w2 > MIN_RECOGNITION_WEIGHT )
        pWM->m_nWeight = (w2 + pWM->m_nWeight)/2;
    if ( pWM->m_nCount < MAX_WORD_COUNT )
        pWM->m_nCount++;
    return true;
}

int CWordLrnFile::AnalizeResult( LPCWSTR pWordList, USHORT * pWeights, LPWSTR pResult, USHORT * pnWeight )
{
    ALTWORD           w;
    PHArray<ALTWORD>  wordlist;
    int      n = 0, j = 0, i = 0;
    USHORT *          pw = pWeights;
    memset( &w, 0, sizeof( w) );
    BOOL			dict = false;
     
    // 1. Create array of alternatives
    for ( j = 0, i = 0; pWordList[i] != 0; i++ )
    {
        if ( pWordList[i] == PM_ALTSEP || pWordList[i] == PM_LISTSEP )
        {
            w.szWord[j] = 0;
            if ( j > 0 )
            {
                if ( NULL != pw )
                {
                    if ( *pw > 0 && *pw != PM_LISTSEP )
                        w.nWeight = *pw;
                }
                w.nIndex = GetWordIndex( w.szWord );
                w.nProb = (int)(w.nWeight & RW_WEIGHTMASK);
                if ( wordlist.GetSize() < 1 )
                {
                    dict = (w.nWeight & RW_DICTIONARYWORD) ? true : false;
                    if ( ! dict )
                        w.nProb -= 5;
                }
                w.nLnk = 0;
                wordlist.Add( w );
            }
            if ( pWordList[i] == PM_LISTSEP )
                break;
            if ( NULL != pw ) pw++;
            j = 0;
            w.nWeight = 0;
            w.szWord[0] = 0;
            w.nIndex = -1;
            w.nProb = 0;
        }
        else
        {
            w.szWord[j++] = pWordList[i];
        }
    }

    int nLen = wordlist.GetSize();
    if ( nLen < 2 )
        return 0;       // must be at least 2 words
    
    CWord * pword0 = NULL;
    CWord * pwordI = NULL;
    int     nIndexI = 0, nIndex0 = wordlist.GetAt(0).nIndex;

    int     sum = 0;
    for ( i = 0; i < nLen; i++ )
    {
        int idx = wordlist.GetAt(i).nIndex;
        if ( idx >= 0 )
        {
            pwordI = m_words[idx];
            sum += pwordI->m_nCount;
        }
        else
        {
            sum++;
        }
    }

    if ( sum > 0 )
    {
        for ( i = 0; i < nLen; i++ )
        {
            w = wordlist.GetAt(i);
            int idx = w.nIndex;
            int cnt = 0;
            if ( idx >= 0 )
            {
                pwordI = m_words[idx];
                cnt = pwordI->m_nCount;
            }
            if ( w.nProb < MIN_RECOGNITION_WEIGHT )
                w.nProb = MIN_RECOGNITION_WEIGHT + (AVE_RECOGNITION_WEIGHT * cnt)/sum;
            else
                w.nProb += (AVE_RECOGNITION_WEIGHT * cnt)/sum;
            wordlist.SetAt( i, w );
        }
    }

    if ( nIndex0 >= 0 )
    {
        pword0 = m_words[nIndex0];
        if ( pword0->m_links.GetSize() > 0 )
        {
            sum = 0;
            for ( j = 0; j < pword0->m_links.GetSize(); j++ )
            {
                nIndexI = (int)LOWORD( pword0->m_links[j] );
                sum += (int)HIWORD( pword0->m_links[j] );
                if ( nIndexI < m_words.GetSize() && NULL != (pwordI = m_words[nIndexI]) )
                {               
                    // I. search for a replace word in suggestions
                    for ( i = 1; i < nLen; i++ )
                    {
                        if ( *pwordI == (LPCWSTR)wordlist.GetAt(i).szWord )                            
                            break;  // word has been found
                    }
                    if ( i < nLen )
                    {
                        int lnk = (int)HIWORD( pword0->m_links[j] );
                        for ( n = 0; n < pwordI->m_links.GetSize(); n++ )
                        {
                            if ( nIndex0 == (int)LOWORD( pwordI->m_links[n] ) )
                            {
                                // back links found, substruct them
                                lnk -= (int)HIWORD( pwordI->m_links[n] );
                                sum += (int)HIWORD( pwordI->m_links[n] );
                                break;
                            }
                        } // for n
                        w = wordlist.GetAt(i);
                        w.nLnk = lnk;
                        wordlist.SetAt( i, w );
                    }
                }
            } // for j
            if ( sum > 0 )
            {
                for ( i = 0; i < nLen; i++ )
                {
                    w = wordlist.GetAt(i);
                    if ( w.nProb < MIN_RECOGNITION_WEIGHT )
                        w.nProb = MIN_RECOGNITION_WEIGHT + (AVE_RECOGNITION_WEIGHT * w.nLnk)/sum;
                    else
                        w.nProb += (AVE_RECOGNITION_WEIGHT * w.nLnk)/sum;
                    wordlist.SetAt( i, w );
                } // for i
            }
        }
    }
    
    if ( NULL == pnWeight )
    {
        // return resorted word list instead of the first result
        int		len = 0;
        for ( i = 0; i < nLen; i++ )
        {
            // rearrange words by priority
            int     nMax = wordlist.GetAt(i).nProb;
            for ( int j = i+1; j < nLen; j++ )
            {
                w = wordlist.GetAt(j);
                if ( w.nProb > nMax )
                {
                    nMax = w.nProb;
                    wordlist.SetAt( j, wordlist.GetAt(i) );
                    wordlist.SetAt( i, w );
                }
            }
            _STRCPY( &pResult[len], wordlist.GetAt(i).szWord );
            len += (int)(1 + _STRLEN( &pResult[len] ));
            pResult[len] = 0;
        }
        return len;
    }
    else
    {
        // find word with max prob
        int     nMax = wordlist.GetAt(0).nProb;
        int     nFirstWordIndex = 0;
        for ( i = 1; i < nLen; i++ )
        {
            w = wordlist.GetAt(i);
            if ( w.nProb > nMax )
            {
                nMax = w.nProb;
                nFirstWordIndex = i;
            }
        }
        if ( nFirstWordIndex > 0 && nFirstWordIndex < nLen )
        {
            *pnWeight = (wordlist.GetAt(nFirstWordIndex).nWeight & RW_WEIGHTMASK);
            _STRCPY( pResult, wordlist.GetAt(nFirstWordIndex).szWord );
            return (int)_STRLEN( pResult );
        }
    }
    return 0;
}
