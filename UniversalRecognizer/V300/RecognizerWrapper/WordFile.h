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

#ifndef __WordCorrFile_h__
#define __WordCorrFile_h__

#include <stdio.h>
#include "recotypes.h"
#include "PHTemplArray.h"
#include "hwr_sys.h"

class CWordMap 
{
public:
	CWordMap( LPCWSTR pWordFrom, LPCWSTR pWordTo, DWORD dwFlags, USHORT nWeight )
	{
		m_pWordFind = _STRDUP( pWordFrom );
		m_pWordReplace = _STRDUP( pWordTo );
		m_dwFlags = (unsigned int)dwFlags;
		m_nWeight = nWeight;
	}

	CWordMap()
	{
		m_pWordFind = NULL;
		m_pWordReplace = NULL;
		m_dwFlags = WCF_IGNORECASE | WCF_ALWAYS;
		m_nWeight = 0;
	}

	~CWordMap()
	{
		Init();
	}

	size_t 	SaveToMemory( char * ppData ) const;
	BOOL	Load( const LPBYTE pBuff, long &dwPos );
	BOOL	Save( FILE * pFile ) const;
    BOOL    Export( FILE * pFile ) const;
    BOOL    Import( const char * pBuff );

	void Copy( CWordMap &wm )
	{
		Init();
		m_pWordFind = _STRDUP( wm.m_pWordFind );
		m_pWordReplace = _STRDUP( wm.m_pWordReplace );
		m_dwFlags = wm.m_dwFlags;
		m_nWeight = wm.m_nWeight;
	}
	
	CWordMap operator=( CWordMap &wm )
	{
		Copy( wm );
		return *this;
	}

    BOOL CompareWords( LPCWSTR pszStr1, LPCWSTR pszStr2, BOOL bIgnoreCase )
    {
        register LPCWSTR    p1 = pszStr1;
        register LPCWSTR    p2 = pszStr2;
        while ( 0!= *p1 && 0!= *p2 )
        {
            if ( bIgnoreCase )
            {
                if ( tolower( (int)*p1 ) != tolower( (int)*p2 ) )
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
        return CompareWords( (LPCWSTR)m_pWordFind, (LPCWSTR)pWord, (BOOL)(0 != (m_dwFlags & WCF_IGNORECASE)) );
	}

	LPWSTR	m_pWordFind;
	LPWSTR  m_pWordReplace;
	unsigned int     m_dwFlags;
	USHORT	m_nWeight;

	void	Init()
	{
		if ( NULL != m_pWordFind )
			free( (void *)m_pWordFind );
		if ( NULL != m_pWordReplace )
			free( (void *)m_pWordReplace );
		m_pWordFind = NULL;
		m_pWordReplace = NULL;
		m_dwFlags = WCF_IGNORECASE | WCF_ALWAYS;
		m_nWeight = 0;
	}
};

typedef PHArray <CWordMap *>	WORDMAP;

class CWordList  
{
public:
	CWordList();
	virtual ~CWordList()
	{
		Free();
	}


	BOOL	Load( const char * pszFileName );
	BOOL	Save( const char * pszFileName, WORD wLanguageID  = 0 );
	size_t 	SaveToMemory( char ** ppData, WORD wLanguageID  = 0 );
	BOOL	LoadFromMemory( const char * pszInBuffer );
    BOOL    Export( const char * pszFileName );
    BOOL    Import( const char * pszFileName );
    WORD    GetLanguageID() const { return m_wLanguageID; }
	int		EnumWordList( RECO_ONGOTWORDLIST callback, void * pParam );

	CWordMap *	GetMap( LPCWSTR pWord ) const
	{
        CWordMap * pWM;
		for ( register int i = 0; i < m_words.GetSize(); i++ )
		{
			pWM = m_words[i];
			if ( pWM && (*pWM == pWord) )
				return pWM;
		}
		return NULL;
	}

	CWordMap *	FindMap( CWordMap * p ) const
	{
        CWordMap * pWM;
		for ( register int i = 0; i < m_words.GetSize(); i++ )
		{
			pWM = m_words[i];
			if ( pWM == p )
				return pWM;
		}
		return NULL;
	}

	LPCWSTR GetReplaceWord( LPCWSTR pWord, DWORD * pdwFlags = NULL ) const
	{
		CWordMap * p = GetMap( pWord );
		if ( NULL != p ) 
		{
			if ( NULL != pdwFlags )
				*pdwFlags = p->m_dwFlags;
			return p->m_pWordReplace;
		}
		return NULL;
	}
    
#ifdef HW_RECINT_UNICODE
    
	CWordMap *	AddWord( const char * pWordFrom, const char * pWordTo )
    {
        LPWSTR from = new UCHR[HWRStrLen(pWordFrom)+2];
        LPWSTR to = new UCHR[HWRStrLen(pWordTo)+2];
        
        StrToUNICODE( from, pWordFrom, HWRStrLen(pWordFrom)+1);
        StrToUNICODE( to, pWordTo, HWRStrLen(pWordTo)+1);
        
        CWordMap *	result = AddWord( from, to);
        
        delete [] from;
        delete [] to;
        return result;
    }
    
#endif // HW_RECINT_UNICODE

	CWordMap *	AddWord( LPCWSTR pWordFrom, LPCWSTR pWordTo, BOOL bReplace = false, DWORD dwFlags = WCF_IGNORECASE | WCF_ALWAYS, USHORT nWeight = 0 )
	{
		CWordMap * pWM = GetMap( pWordFrom );
		if ( NULL != pWM )
		{
            if ( bReplace )
            {
				CWordMap	wm( pWordFrom, pWordTo, dwFlags, nWeight );
			    (*pWM) = wm;
			    return pWM;
            }
            return NULL;
		}
		pWM = new CWordMap( pWordFrom, pWordTo, dwFlags, nWeight );
		if ( NULL == pWM )
			return NULL;
		return AddWord( pWM );
	}

	CWordMap *	AddWord( CWordMap *	pWM )
	{
		m_words.Add( pWM );
		return pWM;
	}

	BOOL RemoveWord( LPCWSTR pWord )
	{
        CWordMap * pWM;
		for ( register int i = 0; i < m_words.GetSize(); i++ )
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

	void RemoveAll()
	{
		Free();
	}

	int GetWordCnt() const { return m_words.GetSize(); }

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

    friend class CWordDlg;
};

#endif // __WordCorrFile_h__
