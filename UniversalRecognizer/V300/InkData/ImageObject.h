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

#include "RecognizerApi.h"
#include "PHStream.h"
#include "PHTemplArray.h"
#include "PHStream.h"

#define EDIT_LIMIT		16000

#define X_BORDER_EDC	SCALEX(4)
#define Y_BORDER_EDC	SCALEY(4)
#define DEFAULT_WIDTH	SCALEY(120)
#define DEFAULT_HEIGHT	SCALEY(48)

/////////////////////////////////////////////////////////////////////////////////
// CTextObject class declaration

class CImageObject
{
public:
	CImageObject( int x, int y, int w, int h, const void * pData, UInt32 dwByteCount, void * userData );
	CImageObject( int x, int y, const char * szFilePath, int w = 0, int h = 0);
    CImageObject( CImageObject *pCopyFrom );
	virtual ~CImageObject(void);

	RECT	GetRect( void );
	bool	GetModified() const { return m_bModified; }
	void	SetModified( bool bMod = true ) { m_bModified = bMod; }
	bool	Write( CPHStream &phFile);
	bool	Read( CPHStream &phFile );

	typedef struct _Image_Attrib
	{
		POINT		pt;					//Upper left coordinate of image
		POINT		pt2;				//Upper right coordinate of image
		POINT		pt3;				//Bottom right coordinate of image
		POINT		pt4;				//Bottom left coordinate of image
		int         iHeight;
		int         iWidth;
		UInt32		dwSize;
		int			iZOrder;		//Z-Order relative to strokes
									//Z-order of 0 means that image is displayed before stroke 0
									//default value is 0
		double		dwRotation;		//Image rotation in degrees
	} SImageAttrib;
	SImageAttrib	m_attrib;

	POINT		m_ptOrig[4];
	void *		m_pImageData;
	
	void *		m_userData;

protected:
	bool		m_bModified;

public:
	int         m_iIndex; // currently used only for Undo
};

// typedef PHArray<CTextObject *> PHTextObjArray;

class PHImageObjArray : public PHArray<CImageObject *>
{
public:
    ~PHImageObjArray() { Free(); }
    void Free( void );
};

