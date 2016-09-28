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

#include <stdlib.h>
#include <string.h>

#include "InternalTypes.h"
#include "RecognizerApi.h"
#include "PHStream.h"

/***********************************************************************
	CPHFileStream - File Stream class implementation
 ***********************************************************************/
CPHFileStream::CPHFileStream()
  : m_pFile(NULL)
  , m_bCloseOnDelete(false)
{
}

CPHFileStream::CPHFileStream( FILE * pFile )
  : m_pFile(pFile)
  , m_bCloseOnDelete(false)
{
}

CPHFileStream::~CPHFileStream()
{
	Close();
}

void CPHFileStream::Close( void )
{
	if ( m_pFile != NULL && m_bCloseOnDelete )
		::fclose( m_pFile );

	m_bCloseOnDelete = false;
	m_pFile = NULL; // set handle to NULL even if we do not need to close the file
}

void CPHFileStream::SetHandle( FILE * pFile )
{
	Close();
	m_pFile = pFile;
}

BOOL CPHFileStream::Write( void *pBytes, long size )
{
	if ( m_pFile != NULL )
	{
		return (0!=::fwrite( pBytes, 1, size, m_pFile ));
	}
	return false;
}

BOOL CPHFileStream::Read( void *pBytes, long size )
{
	if ( m_pFile != NULL )
	{
		return (0!=::fread( pBytes, size, 1, m_pFile));
	}
	return false;
}

long CPHFileStream::SetPos( long iOffset, int iOrigin )
{
	if ( m_pFile )
		return ::fseek( m_pFile, iOffset, iOrigin );
	return 0;
}

long CPHFileStream::GetSize( void )
{
	if ( m_pFile )
	{
		// get dictionary file length
		long nSave = ::ftell( m_pFile );
		::fseek( m_pFile, 0, SEEK_END );
		long nLen = ::ftell( m_pFile );
		::fseek( m_pFile, nSave, SEEK_SET );
	
        return nLen;
	}
	return 0;
}

/***********************************************************************
	CPHMemStream - Memory Stream class implementation
 ***********************************************************************/

CPHMemStream::CPHMemStream()
{
    m_nCurPos = 0;
    m_nUsed = 0;
    m_bCloseOnDelete = true;
    m_bReadOnly = false;
	m_nAllocated = MEMSTR_ALLOC_GRAN;
	m_pData = (LPBYTE)malloc(m_nAllocated);
}

CPHMemStream::CPHMemStream( LPBYTE pBytes, long iSize )
{
    m_nCurPos =0;
    m_nUsed = iSize;
    m_nAllocated = iSize;
    m_bCloseOnDelete = false;
    m_bReadOnly = true;
    m_pData = pBytes;
}

CPHMemStream::~CPHMemStream()
{
	FreeData();
}

void CPHMemStream::FreeData()
{
	if ( m_pData && m_bCloseOnDelete )
		free( (void *)m_pData);

	m_pData = NULL;
	m_nAllocated = m_nUsed = m_nCurPos = 0;
	m_bReadOnly = true;
	m_bCloseOnDelete = false;
}

void CPHMemStream::SetData( LPBYTE pBytes, int size, BOOL bCopy )
{
	FreeData();

	if ( !bCopy )
	{
		m_pData = pBytes;
		m_nAllocated = m_nUsed = size;
		m_bReadOnly = true;
	}
	else
	{
		m_pData = (BYTE *)malloc( size );
		if ( m_pData )
		{
			memcpy( m_pData, pBytes, size );
			m_nAllocated = m_nUsed = size;
			m_bReadOnly = false;
            m_bCloseOnDelete = true;
		}
	}
}

BOOL CPHMemStream::Write( void *pBytes, long size )
{
	if ( m_pData == NULL || m_bReadOnly )
		return false;

	if ( m_nCurPos + size > m_nAllocated )
	{
		long iSizeNew = m_nCurPos + size + MEMSTR_ALLOC_GRAN;
		LPBYTE pNew = (LPBYTE)realloc( m_pData, iSizeNew );
		if ( pNew )
		{
			m_nAllocated = iSizeNew;
			m_pData = pNew;
		}
		else
		{
			return false;
		}
	}

	memcpy( m_pData + m_nCurPos, pBytes, size );
	m_nCurPos += size;
    m_nUsed = max( (int)m_nUsed, (int)m_nCurPos );

	return true;
}

BOOL CPHMemStream::Read( void *pBytes, long size )
{
	if ( m_pData == NULL || m_nCurPos + size > m_nUsed )
	{
		return false;
	}
	else
	{
		memcpy( pBytes, m_pData + m_nCurPos, size );
		m_nCurPos += size;
		return true;
	}
}

long CPHMemStream::SetPos( long iOffset, int iOrigin )
{
	if ( iOrigin == SEEK_SET )
		m_nCurPos = 0;
	else if ( iOrigin == SEEK_CUR )
	{
		if ( m_nCurPos + iOffset <= m_nUsed )
			m_nCurPos += iOffset;
	}
	else if ( iOrigin == SEEK_END )
	{
		if ( m_nUsed - iOffset >= 0 )
			m_nCurPos = m_nUsed - (int)iOffset;
	}
 
	return m_nCurPos;
}
