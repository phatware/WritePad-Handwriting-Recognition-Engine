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

\

#ifndef __WordLearnFile_h__
#define __WordLearnFile_h__

#define MAX_WORD_COUNT      64000

#include <stdio.h>
#include "recotypes.h"
#include "PHTemplArray.h"
#include "hwr_sys.h"


class CWordLrnFile
{
public:
    class CWord
    {
    public:
        CWord( LPCWSTR pWord, USHORT nWeight, BOOL bDictionary, USHORT nCount = 1 )
        {
            m_pWord = _STRDUP( pWord );
            m_nWeight = (nWeight < MIN_RECOGNITION_WEIGHT) ? AVE_RECOGNITION_WEIGHT : nWeight;
            m_nCount = nCount;
            m_bDictionary = bDictionary;
            m_dwFlags = 0;
        }

        CWord()
        {
            m_pWord = NULL;
            m_nWeight = AVE_RECOGNITION_WEIGHT;
            m_nCount = 1;
            m_bDictionary = false;
            m_dwFlags = 0;
        }

        ~CWord()
        {
            Init();
        }

        BOOL	Load( const LPBYTE pBuff, long &dwPos );
        BOOL	Save( FILE * pFile ) const;
        int		SaveToMemory( char * pData ) const;		

        CWord operator=( CWord &wm )
        {
            Init();
            m_pWord = _STRDUP( wm.m_pWord );
            m_nWeight = wm.m_nWeight;
            m_nCount = wm.m_nCount;
            return *this;
        }

        BOOL CompareWords( LPCWSTR pszStr1, LPCWSTR pszStr2, BOOL bIgnoreCase = true )
        {
            LPCWSTR    p1 = pszStr1;
            LPCWSTR    p2 = pszStr2;
            while ( 0!= *p1 && 0!= *p2 )
            {
                if ( bIgnoreCase )
                {
                    if ( tolower( *p1 ) != tolower( *p2 ) )
                        return false;
                }
                else
                {
                    if ( *p1 != *p2 )
                        return false;
                }
                p1++;
                p2++;
            }
            return (*p1 == *p2);
        }

        BOOL operator==( LPCWSTR pWord )
        {
            return CompareWords( m_pWord, pWord );
        }

        LPWSTR	m_pWord;
        DWORD	m_dwFlags;
        USHORT	m_nWeight;
        USHORT  m_nCount;
        BOOL    m_bDictionary;

        typedef PHArray <DWORD>	WORDLINKS;
        WORDLINKS   m_links;


        void	Init()
        {
            if ( NULL != m_pWord )
                free( (void *)m_pWord );
            m_pWord = NULL;
            m_nWeight = 0;
        }
    };

    typedef PHArray <CWord *>	WORDMAP;

public:
    CWordLrnFile( WORD wLangID );
    virtual ~CWordLrnFile()
    {
        Free();
    }


    BOOL	Load( const char * pszFileName, WORD wLangID );
    BOOL	LoadFromMemory( const char * pData, WORD wLangID );
    BOOL	Save( const char * pszFileName, WORD wLanguageID  = 0 );
    int		SaveToMemory( char ** ppData, WORD wLanguageID = 0 );
    WORD    GetLanguageID() const { return m_wLanguageID; }
    BOOL    ReplaceWord( LPCWSTR pWord1, USHORT nWeight1, LPCWSTR pWord2, USHORT nWeight2 );
    int     AnalizeResult( LPCWSTR pWordList, USHORT * pWeights, LPWSTR pResult, USHORT * pnWeight );

    CWord *	GetWord( LPCWSTR pWord ) const
    {
        CWord * pWM;
        int     nCnt = m_words.GetSize();
        for ( int i = 0; i < nCnt; i++ )
        {
            pWM = m_words[i];
            if ( pWM && (*pWM == pWord) )
                return pWM;
        }
        return NULL;
    }

    CWord *	FindWord( CWord * p ) const
    {
        CWord * pWM;
        int     nCnt = m_words.GetSize();
        for ( int i = 0; i < nCnt; i++ )
        {
            pWM = m_words[i];
            if ( pWM == p )
                return pWM;
        }
        return NULL;
    }

    int GetWordIndex( LPCWSTR pWord ) const
    {
        CWord * pWM;
        int     nCnt = m_words.GetSize();
        for ( int i = 0; i < nCnt; i++ )
        {
            pWM = m_words[i];
            if ( pWM && (*pWM == pWord) )
                return i;
        }
        return -1;
    }

#ifdef HW_RECINT_UNICODE
    
    int	AddWord( const char * pWord, USHORT nWeight, BOOL bDictionary )
    {
        UCHR * unicode = new UCHR[HWRStrLen( pWord )+2];
        StrToUNICODE( unicode, pWord, HWRStrLen( pWord )+1 );
        int result = AddWord( (LPCWSTR)unicode, nWeight, bDictionary );
        delete [] unicode;
        return result;
    }
    
#endif // HW_RECINT_UNICODE
    
    int	AddWord( LPCWSTR pWord, USHORT nWeight, BOOL bDictionary )
    {
        CWord * pWM;
        int nIndex = GetWordIndex( pWord );
        if ( nIndex >= 0 )
        {
            pWM = m_words[nIndex];
            if ( nWeight > MIN_RECOGNITION_WEIGHT )
                pWM->m_nWeight = (nWeight + pWM->m_nWeight)/2;
            if ( pWM->m_nCount < MAX_WORD_COUNT )
                pWM->m_nCount++;
            // if ( ! pWM->m_bDictionary )
            pWM->m_bDictionary = bDictionary;
            return nIndex;
        }
        if ( m_words.GetSize() > MAX_WORD_COUNT )
            return -1;
        pWM = new CWord( pWord, nWeight, bDictionary );
        if ( NULL == pWM )
            return -1;
        return AddWord( pWM );
    }

    BOOL RemoveWord( LPCWSTR pWord )
    {
        CWord * pWM;
        for ( int i = 0; i < m_words.GetSize(); i++ )
        {
            pWM = m_words[i];
            if ( pWM && (*pWM == pWord) )
            {
                delete pWM;
                m_words.RemoveAt(i);
                return true;
            }
        }
        return false;
    }

    void		RemoveAll() { Free(); }
    int			GetWordCnt() const { return m_words.GetSize(); }

protected:
    WORDMAP		m_words;
    WORD        m_wLanguageID;
    WORD        m_nCurrentSort;
    WORD        m_nSortOrder;
    void		Free()
    {
        for ( int i = 0; i < m_words.GetSize(); i++ )
        {
            if ( NULL != m_words[i] )
                delete ( m_words[i] );
        }
        m_words.RemoveAll();
    }
    int AddWord( CWord * pWM )
    {
        return m_words.Add( pWM );
    }

};

#endif 
