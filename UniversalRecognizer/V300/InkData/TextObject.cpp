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

#include "TextObject.h"

CTextObject::CTextObject( int x, int y, int w, int h, LPLOGFONTW pFont, LPCUSTR pszText )
{
	memset( &m_attrib, 0, sizeof( m_attrib ) );
	m_attrib.pt.x = x;
	m_attrib.pt.y = y;
	m_attrib.iHeight = h;
	m_attrib.iWidth = w;
	m_attrib.crBack = -1;
	m_attrib.crText = 0;
	m_attrib.crBorder = 0;
	m_attrib.dwBorderStyle = 0;
	m_attrib.nBorderWidth = 1;
	if ( pFont != NULL )
		memcpy( &m_attrib.lf, pFont, sizeof( LOGFONTW ) );
	m_pszText = NULL;
	m_nTextLen = 0;
	m_bModified = false;
    m_iIndex = -1;
	m_pUserData = NULL;

	SetText( pszText );
}

CTextObject::CTextObject( CTextObject *pCopyFrom )
{
    memcpy( &m_attrib, &pCopyFrom->m_attrib, sizeof(m_attrib) );

    if ( pCopyFrom->m_pszText )
    {
        m_pszText = ucsdup( pCopyFrom->m_pszText );
        m_nTextLen = ucslen( m_pszText );
    }
    else
    {
	    m_pszText = NULL;
	    m_nTextLen = 0;
    }
	m_bModified = false;
}

CTextObject::~CTextObject(void)
{
	if ( NULL != m_pszText )
		free( (void *)m_pszText );
	m_pszText = NULL;
}

void CTextObject::SetUserData( void * userData )
{
	m_pUserData = userData;
}

bool CTextObject::Write( CPHStream &phFile )
{
	if ( NULL == m_pszText )
		return false;	// nothing to store
	if ( ! phFile.Write( (BYTE *)&m_attrib, sizeof (  m_attrib ) ) )
		return false;
	if ( ! phFile.Write( (BYTE *)&m_nTextLen, sizeof ( int ) ) )
		return false;
	if ( ! phFile.Write( (BYTE *)m_pszText, (int)(sizeof ( UNCHAR ) * (m_nTextLen+1)) ) )
		return false;
	return true;
}

bool CTextObject::Read( CPHStream &phFile )
{
	if ( ! phFile.Read( (BYTE *)&m_attrib, sizeof (  m_attrib ) ) )
		return false;
	if ( ! phFile.Read( (BYTE *)&m_nTextLen, sizeof ( int ) ) )
		return false;
	if ( m_nTextLen < 1 || m_nTextLen > EDIT_LIMIT )
		return false;
	LPUSTR	pszw = (LPUSTR)malloc( sizeof( UNCHAR ) * (m_nTextLen+2) );
	if ( NULL == pszw )
		return false;
	if ( ! phFile.Read( (BYTE *)pszw, (int)(sizeof ( UNCHAR ) * (m_nTextLen+1)) ) )
		return false;
	if ( m_pszText )
		free( (void *)m_pszText );
	m_pszText = pszw;
	return true;
}

RECT CTextObject::GetRect( void )
{
	RECT rc;
	rc.left = (float)m_attrib.pt.x;
	rc.top = (float)m_attrib.pt.y;
	rc.right = rc.left + m_attrib.iWidth;
	rc.bottom = rc.top + m_attrib.iHeight;
	return rc;
}


void PHTextObjArray::Free( void )
{
	CTextObject *	pto;
	int		count = GetSize();

	for ( int i = 0; i < count; i++ )
	{
		pto = GetAt(i);
		if ( pto )
			delete pto;
	}
	RemoveAll();
}

