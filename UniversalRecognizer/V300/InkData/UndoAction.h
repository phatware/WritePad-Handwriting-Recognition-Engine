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

#if !defined(AFX_UNDOACTION_H__BE35AB16_262F_49C8_81DC_F6ED0DDE8905__INCLUDED_)
#define AFX_UNDOACTION_H__BE35AB16_262F_49C8_81DC_F6ED0DDE8905__INCLUDED_

#include "RecognizerApi.h"
#include "PHStroke.h"
#ifdef IMAGE_SUPPORT
#include "ImageObject.h"
#endif // IMAGE_SUPPORT
#ifdef TEXT_SUPPORT
#include "TextObject.h"
#endif // TEXT_SUPPORT

class CInkData;

#define UNDO_STROKES_DELETED    0x01
#define UNDO_STROKES_ADDED      0x02
#define UNDO_STROKES_MOVED      0x04
#define UNDO_STROKES_CHANGED    0x08

#define UNDO_TEXT_CHANGED		0x10
#define UNDO_TEXT_DELETED		0x20
#define UNDO_TEXT_ADDED			0x40
#define UNDO_TEXT_MOVED			0x80
#define UNDO_IMAGE_CHANGED		0x100
#define UNDO_IMAGE_MOVED		0x200
#define UNDO_IMAGE_ADDED		0x400
#define UNDO_IMAGE_DELETED		0x800

#define DEF_UNDO_LEVELS		10
#define MAX_UNDO_LEVELS     100

class CUndoData
{
public:
    CUndoData( int iType );
    virtual ~CUndoData();
    void AddStroke( PHStroke *pStroke );
    BOOL IsEmpty();
    void RemoveAll();
		
    int             m_iUndoType;
    int             m_iAdded;
    PHStrokeArr *   m_pStrokes;
    PHStrokeArr *   m_pStrokesOld;
    BOOL            m_bStrokesDetached;
	int				m_nObjectIndex;
#ifdef IMAGE_SUPPORT
	POINT			m_ptImagePos;
	int				m_iImageHeight, m_iImageWidth;
	BOOL			AddImageObject( int nIndex, CImageObject * pImageObject, BOOL bClone );
    PHImageObjArray  m_arrImageObjects;
#endif
#ifdef TEXT_SUPPORT
	POINT           m_ptTextPos;
    int             m_iTextHeight, m_iTextWidth;
	
	BOOL			AddTextObject( int nIndex, CTextObject * pTextObject, BOOL bClone );
    PHTextObjArray  m_arrTextObjects;
	// CTextObject *	m_pTextObject;
#endif // TEXT_SUPPORT
};

class CUndoAction  
{
public:
	CUndoAction( int nLevel = DEF_UNDO_LEVELS );
	virtual ~CUndoAction();
	
    BOOL        DoUndo( CInkData *pData, CUndoAction *pRedo );
	
    void        SetUndoLevel( int nLevel );
    int         GetUndoLevel() { return m_nUndoLevels; }
    BOOL        CanUndo() { return (m_nUndoItems > 0); }
	
    BOOL        AddItem( int iType );
    BOOL        AddStroke( PHStroke *pStroke );
    BOOL        AddDeleteAction();
    void        RemoveLastUndo();
    void        FreeAll();
    void        FreeUnused();
#ifdef IMAGE_SUPPORT
	BOOL		AddImageObject( int iType, CImageObject * pImageObject, int nIndex = -1, BOOL bClone = true );
	BOOL		AddItem( int iType, CImageObject * pImageObject, int nIndex = -1, BOOL bClone = true );
#endif
#ifdef TEXT_SUPPORT
	BOOL		UpdateLastType( int iType );
	BOOL		AddItem( int iType, CTextObject * pTextObject, int nIndex = -1, BOOL bClone = true );
	BOOL		AddTextObject( int iType, CTextObject * pTextObject, int nIndex = -1, BOOL bClone = true );
    void        FinishMoveTextUndo( CTextObject * pTextObject );
    BOOL        CanUndoForCurrentObject( CTextObject *pto );
#endif // TEXT_SUPPORT
#ifdef IMAGE_SUPPORT
#endif
	
protected:
    int         m_nUndoLevels;
    int         m_nUndoItems;
    CUndoData **m_pUndoData;
	
    // The array stores detached strokes when Undo is not 
    // linked to an active document. The pool is required
    // as multiple undo actions may refer to the same
    // stroke.

    BOOL        AllocBuffer( void );

};

#endif // !defined(AFX_UNDOACTION_H__BE35AB16_262F_49C8_81DC_F6ED0DDE8905__INCLUDED_)
