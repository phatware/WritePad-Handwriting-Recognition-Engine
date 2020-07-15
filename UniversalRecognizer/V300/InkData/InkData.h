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

#if !defined(AFX_INKDATA_H__2DA34DD0_148F_42A7_A46C_FB550985D5D4__INCLUDED_)
#define AFX_INKDATA_H__2DA34DD0_148F_42A7_A46C_FB550985D5D4__INCLUDED_

#pragma once

#include "RecognizerApi.h"

#include "PHStroke.h"
#include "PHStream.h"
#include "UndoAction.h"
#if IMAGE_SUPPORT
#include "ImageObject.h"
#endif // IMAGE_SUPPORT
#if TEXT_SUPPORT
#include "TextObject.h"
#endif // TEXT_SUPPORT

#define ST_NONE             0
#define ST_RIGHT2LEFT       1
#define ST_LEFT2RIGHT       2
#define ST_TOP2BOTTOM       3
#define ST_BOTTOM2TOP       4

#define RESIZE_LEFT         0x0001
#define RESIZE_RIGHT        0x0002
#define RESIZE_TOP          0x0004
#define RESIZE_BOTTOM       0x0008

#define N_SEGS				3

#define MIN_H   4
#define MIN_V   4

enum TRACE_FILE_STATUS
{
    TRFILE_OK = 0,
    TRFILE_BADFILE,
    TRFILE_NAME_INVALID
};

class CImagePos
{
public:
	int nPos;
	int iZOrder;
};

class CImageSort
{
public:
	CImageSort(int pos, int order)
	{
		nPos = pos;
		iZOrder = order;
	};
	int nPos;
	int iZOrder;
};


typedef struct __stroke_rect
{
    int	    line;
    CGFloat yGrav;
    UInt32  num;
	RECT	rect;
} STRRECT, * LPSTRRECT;

typedef struct __struct_size
{
	CGFloat	cx;
	CGFloat	cy;
} SIZE, * LPSIZE;

struct RawTraceHeader
{
    UInt32        uStrokeCnt; //how many strokes in the trace
    UInt16        wFlags;
};

typedef struct RawTraceHeader RawTraceHeader;


struct RawStrokeHeader
{
    float       fPenWidth;
    COLORREF    lColor;
    int         uPointCount;
    DWORD       dwFlags;
};

typedef struct RawStrokeHeader RawStrokeHeader;

class CInkData : public PHStrokeArr  
{
public:
	CInkData();
	virtual ~CInkData();

    //Trace handling
	int     AddNewStroke( float fWidth, COLORREF iColor, DWORD dwFlags = 0xFFFFFFFF );
    int     AddPointsToStroke( UInt32 iStroke, const CGTracePoint points[], int iPointCnt );
	int		AddPointsToStrokeTrace( UInt32 uStroke, const CGTracePoint * points, int iPointCnt );
    int     AddPointsToStroke( UInt32 iStroke, const CGPoint * points, int iPointCnt );
	int		AddPointsToStroke( UInt32 uStroke, const LPPOINTS points, int iPointCnt );
	int		AddPointsToStroke( UInt32 uStroke, const LPPOINT points, int iPointCnt );
    int     RemoveStroke( int iStroke );
    void    SetStrokeColor( int nStroke, COLORREF Color );
    void    SetStrokeWidth( int nStroke, float fWidth );
    int     StrokesTotal( void );
    UInt32  ReadOneStroke( UInt32 uStroke, CGTracePoint * pPoints, float& fWidth, COLORREF& Color, int& uPntCnt );
	int		ReadOneStroke( UInt32 uStroke, CGTracePoint * pPoints, int * pnPntCnt = NULL, float * pfWidth = NULL, COLORREF * pColor = NULL );
    int     ReadOneStroke( UInt32 uStroke, CGPoint * pPoints, int * nPntCnt = NULL, float * pfWidth = NULL, COLORREF * pColor = NULL );
    UInt32  GetStrokePointCnt( UInt32 uStroke );
    BOOL    CalcTraceRect( LPRECT pRect, BOOL bSelected = false );
    BOOL    CalcStrokeRect( int nStroke, LPRECT pRect, BOOL bAddWidth = true );
    BOOL    CalcStrokeRect( PHStroke *  pStr, LPRECT lpRect, BOOL bAddWidth = true, CGFloat *pnGrav = NULL );
	void	FreeStrokes( void );
    BOOL    SelectAllStrokes( BOOL bSelect = true );
    void    SetStrokeRecognizable( int nStroke, BOOL bSet = true );
    BOOL    IsStrokeRecognizable( int nStroke );
    void    SetStrokesRecognizable( BOOL bSet = true, BOOL bSelectedOnly = true );
	void	SelectStroke( int uStroke, BOOL bSelect = true );
    int     HasStrokes( BOOL bSelected );
    BOOL    IsStrokeSelected( int uStroke );
    long 	CopyRawInk( void ** ppMem );
    BOOL    PasteRawInk( const void * pMem, long cbSize, float x = 0.0f, float y = 0.0f );
    BOOL    GetModified() const { return m_bModified; }
    void    SetModified( BOOL bModified ) { m_bModified = bModified; }
    BOOL    MoveStroke( UInt32 uStroke, float xOff, float yOff, LPRECT lpRect = NULL );
    void    ChangeZOrder( int iDepth, BOOL bFwd );
    void    SetStroke( int nStroke, float fWidth, COLORREF clr );
	BOOL	ResetScaledStroke( UInt32 uStroke );
	BOOL	SaveScaledStroke( UInt32 uStroke );
	int		IsPointNearStroke( POINTS point, float proximity );
    BOOL    ResizeStroke( UInt32 uStroke, float x0, float y0, float scalex, float scaley, BOOL bReset, LPRECT lpRect = NULL );
    LPSTRRECT	GetRecoStrokes( int nFirstStroke, int * pnStrokesCnt, BOOL bSort, BOOL bSelectedOnly );
    PHStroke *	FindStroke( PHStroke *pSample );
    BOOL    CurveIntersectsStroke( int nStroke, const CGTracePoint * points, int nPointCount );
    int     DeleteIntersectedStrokes( const CGTracePoint * points, int nPointCount );
	
    //Serialization
    int     Read( DWORD dwFormat, FILE * pFile, void * pData, long cbSize, BOOL skipImages );
    int     Write( DWORD dwFormat, FILE * pFile, void ** ppData, long * pcbSize );
	
    int     ReadRawInk(  FILE * pFile, DWORD dwFlags );
    int     WriteRawInk( FILE * pFile, DWORD dwFlags );
	
	TRACE_FILE_STATUS WritePhatWareInk( FILE * pFile, DWORD dwFlags);
	TRACE_FILE_STATUS ReadPhatWareInk( FILE * pFile, DWORD dwFlags );
	
	PHStroke *	GetStroke( UInt32 uiStroke );
	void		SortStrokes();
	void		CopyData( CInkData * pFrom );
	
    // Undo/Redo
    void    SetUndoLevels( int nLevels );
    int     GetUndoLevels() { return m_Undo.GetUndoLevel(); }
    BOOL    CanUndo() { return m_Undo.CanUndo(); }
    BOOL    CanRedo() { return m_Redo.CanUndo(); }
    BOOL    Undo();
    BOOL    Redo();
    void    FreeUndo();
	
    BOOL    RecordUndo( int iType );
    void    StopRecordingUndo();
    void    RemoveLastUndo();
	
	int		SelectStrokesInRect( const LPRECT rect );
	BOOL	IsStrokeInRect( int nStroke, const LPRECT rect );
	
    BOOL    RemoveSelected( BOOL bDeleteAll = false );
	
    void    SetSelColor( COLORREF Color );
    void    SetSelWidth( float fWidth );
    void    EnableUndo( BOOL enabled );
	
    CGPoint GetFirstPoint( int nStroke );
    CGPoint GetLastPoint( int nStroke );
		
#if IMAGE_SUPPORT
	CImageObject *	AddImageObject( int x, int y, int w, int h, const char * pszFilePath );
	int				AddImageObject( int x, int y, int w, int h, UInt32 flags, int iZOrder, const void * pBytes, UInt32 cbSize, void * userData, int nImageIndex = -1 );
	CImageObject *	EnumImageObjects( int & nPos );
	int				GetImageObjectCnt() const;
	void			FreeImages( void );
	BOOL			SetImageUserData( int nImageIndex, void * imageRef );
	BOOL			DeleteAllImageObjects();
	CImageObject *	GetImageObject( POINTS point );
 	CImageObject *	GetImageObject( int nIndex );
	CImageObject *  RemoveImageObject( int nIndex );
	BOOL			SetImageFrame( int nImageIndex, int x, int y, int w, int h );
	BOOL			DeleteImageObject( CImageObject * pto );
	BOOL			DeleteImageObject( int nIndex );
	CImageObject *	InsertImageObject( int nIndex, CImageObject * pImageObject );
	CImageObject *	ReplaceImageObject( int nIndex, CImageObject * pImageObject );
	CImageObject *	m_pCurrentImageObject;
	BOOL			RecordUndo( int iType, CImageObject* pImageObject, int nIndex = -1, BOOL bClone = true );	
	int				GetImageObjectIndex( CImageObject * pto );
#endif
	
#if TEXT_SUPPORT
	int				GetTextObjectCnt() const { return m_arrTextObjects.GetSize(); }
	void			StopRecordingUndo( int iType );
	void			FreeText( void );
	BOOL			RecordUndo( int iType, CTextObject * pTextObject, int nIndex = -1, BOOL bClone = true );
	BOOL			DeleteTextObject( CTextObject * pto );
	BOOL			DeleteTextObject( int nIndex );
    BOOL            DeleteAllTextObjects( BOOL bRecordUndo = false );
	BOOL			SetTextUserData( int nTextIndex, void * userData );
	void *			GetTextUserData( int nTextIndex );
    CTextObject *   RemoveTextObject( int nIndex );
	int				GetTextObjectIndex( CTextObject * pto );
	CTextObject *	InsertTextObject( int nIndex, CTextObject * pTextObject );
	CTextObject *	ReplaceTextObject( int nIndex, CTextObject * pTextObject );
	CTextObject *	GetTextObject( int nIndex );
	CTextObject *	GetTextObject( POINTS point );
	CTextObject *	EnumTextObjects( int & nPos );
	CTextObject *	AddTextObject( int x, int y, int w, int h, DWORD alignment = 0, LPLOGFONTW pFont = NULL, const UNCHAR * pszText = NULL, int nIndex = -1 );
	BOOL			SetTextFrame( int nTextIndex, int x, int y, int w, int h );
    void            FinishMoveTextUndo( CTextObject * pTextObject );
    BOOL            CanUndoForCurrentObject( CTextObject *pto );
#endif // TEXT_SUPPORT
	
	BOOL	m_bRecorgnizeShapes;
	
protected:
	BOOL ReadCompressedStroke( CPHStream &phStream, COLORREF *colors, 
							  int nColors, short &lastX, short &lastY, UInt16& nCompressedPoints, BOOL byteForWidth, UCHAR options );
	int	 WriteDataToStream( CPHStream &phStream, BOOL bSelectedOnly =false );
    BOOL ReadRawFromStream( CPHStream &phStream, float x = 0, float y = 0, BOOL select = false );
    int  WriteRawToStream( CPHStream &phStream, BOOL bSelectedOnly = false );
	void WritePressures(CPHStream &phFile, PHSubStroke* pPoints, int iFrom, int iTo );
    BOOL ReadPressures(CPHStream &phFile, PHStroke * pStroke, int nPoints );
    
	TRACE_FILE_STATUS WritePhatWareInk( CPHStream &phFile, BOOL bIgnoreLast, BOOL bSavePressure );
    TRACE_FILE_STATUS ReadPhatWareInk( CPHStream &phStream, BOOL skipImages = false );
	
#if TEXT_SUPPORT
	BOOL	WriteTextElements( CPHStream &phFile );
	BOOL	ReadTextElements( CPHStream &phStream );
#endif // TEXT_SUPPORT
	
#if IMAGE_SUPPORT
	BOOL	WriteImages( CPHStream &phFile );
	BOOL	ReadImages( CPHStream &phStream );
#endif // IMAGE_SUPPORT
	
private:
    BOOL			m_bModified;
    BOOL			m_bRecordingUndo;
	
#if TEXT_SUPPORT
	// text support
	PHTextObjArray	m_arrTextObjects;
#endif // TEXT_SUPPORT
	
    // Undo/Redo
    CUndoAction		m_Undo;
    CUndoAction		m_Redo;
    BOOL            m_bUndoEnabled;
	
public:
	
#if IMAGE_SUPPORT
	PHImageObjArray	m_arrImages;
#endif // IMAGE_SUPPORT
	
};

//////////////////////////////////////////////////////////////////////////////////////////////////
// Inline functions

inline void InflateRect( LPRECT pr, float x, float y )
{
	pr->left -= x;
	pr->right += x;
	pr->top -= y;
	pr->bottom += y;
}

inline BOOL UnionRect( LPRECT rOut, const LPRECT rIn1, const LPRECT rIn2 )
{
	rOut->left = min( rIn1->left, rIn2->left );
	rOut->top = min( rIn1->top, rIn2->top );
	rOut->right = max( rIn1->right, rIn2->right );
	rOut->bottom = max( rIn1->bottom, rIn2->bottom );
	return true;
}

inline BOOL PtInRect( const LPRECT rect, const POINTS pt )
{
	if ( (rect->left <= pt.x && rect->right >= pt.x) && (rect->top <= pt.y && rect->bottom >= pt.y) )
		return true;
	return false;
}

inline BOOL PtInRect( const LPRECT rect, const POINT pt )
{
	if ( (rect->left <= pt.x && rect->right >= pt.x) && (rect->top <= pt.y && rect->bottom >= pt.y) )
		return true;
	return false;
}

#endif // !defined(AFX_INKDATA_H__2DA34DD0_148F_42A7_A46C_FB550985D5D4__INCLUDED_)
