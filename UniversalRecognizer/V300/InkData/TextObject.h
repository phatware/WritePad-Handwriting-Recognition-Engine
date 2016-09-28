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

#include "PHStream.h"
#include "PHTemplArray.h"

#define EDIT_LIMIT		16000

#define X_BORDER_EDC	SCALEX(4)
#define Y_BORDER_EDC	SCALEY(4)
#define DEFAULT_WIDTH	SCALEY(120)
#define DEFAULT_HEIGHT	SCALEY(48)


#ifndef LF_FACESIZE

#define LF_FACESIZE         32

typedef struct tagLOGFONTW
{
	int       lfHeight;
	int       lfWidth;
	int       lfEscapement;
	int       lfOrientation;
	int       lfWeight;
	BYTE      lfItalic;
	BYTE      lfUnderline;
	BYTE      lfStrikeOut;
	BYTE      lfCharSet;
	BYTE      lfOutPrecision;
	BYTE      lfClipPrecision;
	BYTE      lfQuality;
	BYTE      lfPitchAndFamily;
	UNCHAR    lfFaceName[LF_FACESIZE];
} LOGFONTW, * LPLOGFONTW;

#endif 

inline int ucslen( LPCUSTR pText )
{
	int result = 0;
	while ( *pText++ )
		result++;
	return result;
}

inline void ucscpy( LPUSTR pOut, LPCUSTR pIn )
{
	while ( *pIn )
		*pOut++ = *pIn++;
	*pOut = 0;
}

inline LPUSTR ucsdup( LPCUSTR pText )
{
	long len = ucslen( pText );
	LPUSTR pResult = (LPUSTR)malloc( (2 + len) * sizeof( UNCHAR ) ); 
	memcpy( pResult, pText, (len + 1) * sizeof( UNCHAR ) );
	return pResult;
}


/////////////////////////////////////////////////////////////////////////////////
// CTextObject class declaration

class CTextObject
{
public:
	CTextObject( int x, int y, int w, int h, LPLOGFONTW hFont = NULL, LPCUSTR pszText = NULL );
    CTextObject( CTextObject *pCopyFrom );
	virtual ~CTextObject(void);

	void	SetText( LPCUSTR pText )
	{
		if ( pText == m_pszText )
			return;
		if ( NULL != pText && NULL != m_pszText )
			return;
		if ( m_pszText )
			free( (void *)m_pszText );
		m_pszText = NULL;
		m_nTextLen = 0;
		if ( NULL != pText && 0 != *pText )
		{
			m_pszText = ucsdup( pText );
			if ( m_pszText )
				m_nTextLen = ucslen( m_pszText );
		}
		m_bModified = true;
	}

	LPCUSTR	GetText() const { return (LPCUSTR)m_pszText; }
	long	GetTextLen() const { return m_nTextLen; }
	RECT	GetRect( void );
	bool	GetModified() const { return m_bModified; }
	void	SetModified( bool bMod = true ) { m_bModified = bMod; }
	bool	Write( CPHStream &phFile);
	bool	Read( CPHStream &phFile );
	void	SetUserData( void * userData );
	void *	GetUserData() const { return m_pUserData; }

public:
	typedef struct _Text_Attrib
	{
		POINT		pt;				//Upper left coordinate of window
		int         iHeight;
		int         iWidth;
		COLORREF	crBack;			//Color to draw background
		COLORREF	crText;			//Color to draw text
		COLORREF	crBorder;		//Color to draw border
		DWORD		dwBorderStyle;
		int 		nBorderWidth;
		DWORD		dwJustification;
		LOGFONTW	lf;
		DWORD		dwFlags;
		DWORD		dwReserved;
	} STextAttrib;
	STextAttrib	m_attrib;
	void *		m_pUserData;

protected:
	LPCUSTR		m_pszText;
	int 		m_nTextLen;
	bool		m_bModified;

public:
    int         m_iIndex; // currently used only for Undo
};

// typedef PHArray<CTextObject *> PHTextObjArray;

class PHTextObjArray : public PHArray<CTextObject *>
{
public:
    ~PHTextObjArray() { Free(); }
    void Free( void );
};
