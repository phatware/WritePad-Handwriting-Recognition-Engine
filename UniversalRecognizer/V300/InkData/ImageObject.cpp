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

#include "ImageObject.h"

CImageObject::CImageObject( int x, int y, int w, int h, const void * pBytes, UInt32 dwByteCount, void * userData )
{
	memset( &m_attrib, 0, sizeof( m_attrib ) );
	m_pImageData = NULL;
	m_attrib.iZOrder = 0;
	m_userData = userData;
	
	//iOrigWidth = 0;
	//iOrigHeight = 0;
	if ( pBytes && dwByteCount > 0 )
	{
		m_attrib.iWidth = w;
		m_attrib.iHeight = h;
		m_attrib.dwSize = dwByteCount;
		m_pImageData = (void *)pBytes;
		m_attrib.pt.x = m_attrib.pt4.x = x;
		m_attrib.pt2.x = m_attrib.pt3.x = x + m_attrib.iWidth;
		m_attrib.pt.y = m_attrib.pt2.y = y;
		m_attrib.pt3.y = m_attrib.pt4.y = y + m_attrib.iHeight;
		m_ptOrig[0] = m_attrib.pt;
		m_ptOrig[1] = m_attrib.pt2;
		m_ptOrig[2] = m_attrib.pt3;
		m_ptOrig[3] = m_attrib.pt4;
	}
	m_bModified = false;
}

CImageObject::CImageObject( int x, int y, const char * pszFilePath, int w, int h )
{
	memset( &m_attrib, 0, sizeof( m_attrib ) );
	m_attrib.iZOrder = 0;
	m_pImageData = NULL;
	m_userData = NULL;
	
	if ( NULL != pszFilePath )
	{
		FILE * pFile = fopen( pszFilePath, "r" );
		if ( NULL != pFile )
		{
			CPHFileStream phStream( pFile );
			//m_attrib.pt.x = x;
			//m_attrib.pt.y = y;
			m_attrib.iWidth = w;
			m_attrib.iHeight = h;
			m_attrib.dwSize = (int)phStream.GetSize();
			m_pImageData = malloc( m_attrib.dwSize );
			phStream.Read( m_pImageData, m_attrib.dwSize);
			phStream.Close();
			m_attrib.pt.x = m_attrib.pt4.x = x;
			m_attrib.pt2.x = m_attrib.pt3.x = x + m_attrib.iWidth;
			m_attrib.pt.y = m_attrib.pt2.y = y;
			m_attrib.pt3.y = m_attrib.pt4.y = y + m_attrib.iHeight;
			m_ptOrig[0] = m_attrib.pt;
			m_ptOrig[1] = m_attrib.pt2;
			m_ptOrig[2] = m_attrib.pt3;
			m_ptOrig[3] = m_attrib.pt4;
		}
	}
	m_bModified = false;
}

CImageObject::CImageObject( CImageObject *pCopyFrom )
{
	memcpy( &m_attrib, &pCopyFrom->m_attrib, sizeof(m_attrib) );
	if ( pCopyFrom->m_pImageData )
	{
		if ( NULL != m_pImageData )
			free( m_pImageData );
		m_pImageData = malloc( m_attrib.dwSize );
		m_userData = pCopyFrom->m_userData;
		memcpy( m_pImageData, pCopyFrom->m_pImageData, m_attrib.dwSize);
	}
	else
	{
		m_pImageData = NULL;
	}
	m_bModified = false;
}

CImageObject::~CImageObject(void)
{
	if ( NULL != m_pImageData )
	{
		free( m_pImageData );
		m_pImageData = NULL;
	}
}

bool CImageObject::Write( CPHStream &phFile )
{
	if ( NULL == m_pImageData || 0 == m_attrib.dwSize )
		return false;	// nothing to store
	if ( ! phFile.Write( (BYTE *)&m_attrib, sizeof (  m_attrib ) ) )
		return false;
	if ( ! phFile.Write( (BYTE *)m_pImageData, (sizeof ( BYTE ) * (m_attrib.dwSize)) ) )
		return false;
	return true;
}

bool CImageObject::Read( CPHStream &phFile )
{
	if ( ! phFile.Read( (BYTE *)&m_attrib, sizeof (  m_attrib ) ) )
		return false;
	if ( (int)m_attrib.dwSize >= phFile.GetSize() )
		return false;
	if ( m_attrib.dwSize == 0 )
		return false;
	if ( NULL != m_pImageData )
		free( m_pImageData );
	m_pImageData = malloc( m_attrib.dwSize );
	if ( ! phFile.Read( (BYTE *)m_pImageData, (sizeof ( BYTE ) * (m_attrib.dwSize)) ) )
		return false;
	return true;
}


RECT CImageObject::GetRect( void )
{
	RECT rc;
	rc.left = (float)m_attrib.pt.x;
	rc.top = (float)m_attrib.pt.y;
	rc.right = (float)m_attrib.pt3.x;
	rc.bottom = (float)m_attrib.pt3.y;
	return rc;
}

void PHImageObjArray::Free( void )
{
	CImageObject *	pto;
	int		count = GetSize();

	for ( int i = 0; i < count; i++ )
	{
		pto = GetAt(i);
		if ( pto )
			delete pto;
	}
	RemoveAll();
}
