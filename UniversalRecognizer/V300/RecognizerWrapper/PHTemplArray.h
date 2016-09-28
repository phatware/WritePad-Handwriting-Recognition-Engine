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

#pragma once

#include <string.h>
#include <stdlib.h>

#include "InternalTypes.h"

template <class T>
class PHArray  
	{
	public:
		PHArray()
		{
			m_pData = NULL;
			m_nSize = m_nMaxSize = m_nGrowBy = 0;
		}
		
		virtual ~PHArray()
		{
			if ( NULL != m_pData )
				delete[] m_pData;
			m_pData = NULL;
		}
		
		
		// Attributes
		int     GetSize() const;
		int     GetUpperBound() const;
		void    SetSize( int nNewSize, int nGrowBy = -1 );
		
#if defined(WIN32_PLATFORM_PSPC)
		void    GetAsBlob( CEBLOB * pBlob ) const;
		BOOL    AddFromBlob( const CEBLOB * pBlob );
#endif
		
		int     AddUnique( T dwWord );
		int     Find( T dwWord ) const;
		
		// Operations
		// Clean up
		void    FreeExtra();
		void    RemoveAll();
		
		// memory methods
		int     Load( const LPBYTE pSrc );
		int     Save( LPBYTE pDst ) const;
		
		// Accessing elements
		T       GetAt(int nIndex) const;
		void    SetAt(int nIndex, T newElement);
		
		// Direct Access to the element data (may return NULL)
		const T * GetData() const;
		T *       GetData();
		
		// Potentially growing the array
		void    SetAtGrow( int nIndex, T newElement );
		
		int     Add( T newElement );
		
		int     Append(const PHArray& src);
		void    Copy(const PHArray& src);
		
		// overloaded operator helpers
		T   operator[](int nIndex) const;
		PHArray& operator=( PHArray & arr );
		
		// Operations that move elements around
		void    InsertAt(int nIndex, T newElement, int nCount = 1);
		
		void    RemoveAt(int nIndex, int nCount = 1);
		
		// Implementation
	protected:
		T *     m_pData;   // the actual array of data
		int     m_nSize;     // # of elements (upperBound - 1)
		int     m_nMaxSize;  // max allocated
		int     m_nGrowBy;   // grow amount
	};

////////////////////////////////////////////////////////////////////////////
template <class T>
__inline int PHArray<T>::GetSize() const
{ 
    return m_nSize; 
}

template <class T>
__inline int PHArray<T>::GetUpperBound() const
{ 
    return m_nSize - 1; 
}

template <class T>
__inline void PHArray<T>::RemoveAll()
{ 
    SetSize(0); 
}

template <class T>
__inline T PHArray<T>::GetAt(int nIndex) const
{ 
	return m_pData[nIndex]; 
}
template <class T>
__inline void PHArray<T>::SetAt(int nIndex, T newElement)
{ 
	m_pData[nIndex] = newElement; 
}

template <class T>
__inline const T * PHArray<T>::GetData() const
{ 
    return (const T *)m_pData; 
}

template <class T>
__inline T * PHArray<T>::GetData()
{ 
    return m_pData; 
}

template <class T>
__inline int PHArray<T>::Add(T newElement)
{ 
    int nIndex = m_nSize;
	SetAtGrow(nIndex, newElement);
	return nIndex; 
}

template <class T>
__inline T PHArray<T>::operator[](int nIndex) const
{ 
    return GetAt(nIndex); 
}

template <class T>
__inline PHArray<T>& PHArray<T>::operator=(PHArray<T> & arr )
{ 
    SetSize(0); 
    for ( int i = 0;  i < arr.GetSize(); i++ )
        Add( arr.GetAt(i) );
    return *this;
}

template <class T>
__inline int PHArray<T>::AddUnique( T dwWord )
{
    int nIndex = Find( dwWord );
	if ( nIndex >= 0 )
        return -2;
    return Add( dwWord );
}

template <class T>
__inline void PHArray<T>::SetSize(int nNewSize, int nGrowBy)
{
	if (nGrowBy != -1)
		m_nGrowBy = nGrowBy;  // set new size
	
	if (nNewSize == 0)
	{
		// shrink to nothing
        if ( NULL != m_pData )
		    delete[] m_pData;
		m_pData = NULL;
		m_nSize = m_nMaxSize = 0;
	}
	else if (m_pData == NULL)
	{
		// create one with exact size
		m_pData = new T[nNewSize];
		memset( m_pData, 0, nNewSize * sizeof(T));  // zero fill
		m_nSize = m_nMaxSize = nNewSize;
	}
	else if ( nNewSize <= m_nMaxSize )
	{
		// it fits
		if ( nNewSize > m_nSize )
		{
			// initialize the new elements
			memset(&m_pData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(T));
		}
		m_nSize = nNewSize;
	}
	else
	{
		// otherwise, grow array
		int nGrowBy = m_nGrowBy;
		if (nGrowBy == 0)
		{
			// heuristically determine growth when nGrowBy == 0
			//  (this avoids heap fragmentation in many situations)
			nGrowBy = 4;
			if ( m_nSize / 8 > nGrowBy )
			{
				nGrowBy = m_nSize / 8;
				if ( nGrowBy > 1024 )
					nGrowBy = 1024;
			}
			// nGrowBy = min( 1024.0, max(4, m_nSize / 8) );
		}
		int nNewMax;
		if (nNewSize < m_nMaxSize + nGrowBy)
			nNewMax = m_nMaxSize + nGrowBy;  // granularity
		else
			nNewMax = nNewSize;  // no slush
		
		T * pNewData = new T[nNewMax];
		
		// copy new data from old
		memcpy(pNewData, m_pData, m_nSize * sizeof(T));
		
		// construct remaining elements
		memset( &pNewData[m_nSize], 0, (nNewSize-m_nSize) * sizeof(T) );
		
		// get rid of old stuff (note: no destructors called)
        if ( NULL != m_pData )
		    delete[] m_pData;
		m_pData = pNewData;
		m_nSize = nNewSize;
		m_nMaxSize = nNewMax;
	}
}

template <class T>
__inline void PHArray<T>::FreeExtra()
{
	if (m_nSize != m_nMaxSize)
	{
		// shrink to desired size
		T * pNewData = NULL;
		if ( m_nSize != 0 )
		{
			pNewData = new T[m_nSize];
			// copy new data from old
			memcpy( pNewData, m_pData, m_nSize * sizeof(T) );
		}
		
		// get rid of old stuff (note: no destructors called) 
        if ( NULL != m_pData )
		    delete[] m_pData;
		m_pData = pNewData;
		m_nMaxSize = m_nSize;
	}
}

template <class T>
__inline void PHArray<T>::SetAtGrow( int nIndex, T newElement )
{
	if (nIndex >= m_nSize)
		SetSize(nIndex + 1);
	m_pData[nIndex] = newElement;
}

template <class T>
__inline void PHArray<T>::InsertAt(int nIndex, T newElement, int nCount)
{
	
	if (nIndex >= m_nSize)
	{
		// adding after the end of the array
		SetSize(nIndex + nCount);  // grow so nIndex is valid
	}
	else
	{
		// inserting in the middle of the array
		int nOldSize = m_nSize;
		SetSize( m_nSize + nCount );  // grow it to new size
		// shift old data up to fill gap
		memmove( &m_pData[nIndex + nCount], &m_pData[nIndex], (nOldSize - nIndex) * sizeof(T) );
		
		// re-init slots we copied from
		memset( &m_pData[nIndex], 0, nCount * sizeof(T) );
	}
	
	// insert new value in the gap
	// copy elements into the empty space
	while (nCount--)
		m_pData[nIndex++] = newElement;
}

template <class T>
__inline void PHArray<T>::RemoveAt(int nIndex, int nCount)
{
	// just remove a range
	int nMoveCount = m_nSize - (nIndex + nCount);
	
	if ( nMoveCount )
    {
		memmove( &m_pData[nIndex], &m_pData[nIndex + nCount],
				nMoveCount * sizeof(T));
    }
	m_nSize -= nCount;
}

template <class T>
__inline int PHArray<T>::Find( T elT ) const
{
    for ( int i = 0; i < GetSize(); i++ )
    {
        if ( elT == GetAt(i) )
            return i;
    }
    return -1;
}

template <class T>
__inline int PHArray<T>::Append(const PHArray& src)
{
	int nOldSize = m_nSize;
	SetSize( m_nSize + src.m_nSize );
	memcpy(m_pData + nOldSize, src.m_pData, src.m_nSize * sizeof( T ) );
	return nOldSize;
}

template <class T>
__inline void PHArray<T>::Copy(const PHArray& src)
{
	SetSize(src.m_nSize);
	memcpy(m_pData, src.m_pData, src.m_nSize * sizeof( T ));
}

template <class T>
__inline int PHArray<T>::Load(const LPBYTE pSrc )
{
    LPBYTE p = pSrc;
    memcpy( &m_nSize, p, sizeof( UInt32 ) );
    p += sizeof( UInt32 );
	SetSize( m_nSize );
	memcpy( m_pData, p, m_nSize * sizeof( T ));
    p += (m_nSize * sizeof( T ));
    return (int)(p - pSrc);
}

template <class T>
__inline int PHArray<T>::Save( LPBYTE pDst ) const
{
    if ( NULL == pDst )
        return (sizeof( UInt32 ) + m_nSize * sizeof( T ));
	
    LPBYTE p = pDst;
    memcpy( p, &m_nSize, sizeof( UInt32 ) );
    p += sizeof( UInt32 );
    if ( m_nSize > 0 )
    {
	    memcpy( p, m_pData, m_nSize * sizeof( T ) );
        p += (m_nSize * sizeof( T ));
    }
    return (int)(p - pDst);
}
