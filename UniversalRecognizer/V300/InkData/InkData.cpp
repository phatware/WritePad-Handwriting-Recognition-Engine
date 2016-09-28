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

#include <string.h>
#include <limits.h>

#include "InkData.h"
#include "CompressedStroke.h"
#include "PHStream.h"
#include "InternalTypes.h"
#include "ImageObject.h"


static BYTE CalcDeltaBits( short deltaX, short deltaY );
static BOOL WriteCompressedStroke( CPHStream &phFile, BYTE *bCompressedPoints, SInt32 zOrder,
	BYTE *bDeltas, UInt16 nCompressedPoints, BYTE bStrokeFlags, 
	short startX, short startY, 
	BYTE bColorIndex, BYTE &bPrevColorIndex, 
	short bWidth, short &bPrevWidth, BOOL b3bit );

#define ABS(a) ( ((a) > 0) ? (a) : -(a) )

#define DIST_TOUCH		6

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInkData::CInkData()
{
    m_bRecordingUndo = false;
    m_bModified = false;
	m_bRecorgnizeShapes = false;
	m_pCurrentImageObject = NULL;
    m_bUndoEnabled = true;
	SetSize( 0, 1 ); //empty array, grow by 1
}

CInkData::~CInkData()
{
#ifdef TEXT_SUPPORT
	FreeText();
#endif // TEXT_SUPPORT
#ifdef IMAGE_SUPPORT
	FreeImages();
#endif // TEXT_SUPPORT
    FreeUndo();
	FreeStrokes();
}

PHStroke * CInkData::GetStroke( UInt32 uiStroke )
{
    if ( (int)uiStroke >= GetSize() || (int)uiStroke < 0 )
        return NULL;
    else
        return (*this)[(int)uiStroke];
}

int CInkData::AddNewStroke( float fWidth, COLORREF iColor, DWORD dwFlags /* = 0xFFFFFFFF */ )
{
    PHStroke *pStroke = new PHStroke( fWidth, iColor );
	if ( pStroke )
    {
        if ( dwFlags != 0xFFFFFFFF )
            pStroke->SetStrokeFlags( dwFlags );
        Add( pStroke );
		
        if ( m_bRecordingUndo && m_bUndoEnabled )
            m_Undo.AddDeleteAction();
        m_bModified = true;
    }
	return (GetSize() - 1);
}

int CInkData::AddPointsToStroke( UInt32 uStroke, const LPPOINT points, int iPointCnt )
{
	PHStroke *pStroke = GetStroke( uStroke );
    if ( NULL != pStroke )
    {
	    return pStroke->AddPoints( points, iPointCnt );
    }
    return 0;
}

int CInkData::AddPointsToStroke( UInt32 uStroke, const LPPOINTS points, int iPointCnt )
{
	PHStroke *pStroke = GetStroke( uStroke );
    if ( NULL != pStroke )
    {
	    return pStroke->AddPoints( points, iPointCnt );
    }
    return 0;
}

int CInkData::AddPointsToStroke( UInt32 uStroke, const CGTracePoint points[], int iPointCnt )
{
	PHStroke *pStroke = GetStroke( uStroke );
    if ( NULL != pStroke )
    {
	    return pStroke->AddPoints( points, iPointCnt );
    }
    return 0;
}

int CInkData::AddPointsToStrokeTrace( UInt32 uStroke, const CGTracePoint * points, int iPointCnt )
{
	PHStroke *pStroke = GetStroke( uStroke );
    if ( NULL != pStroke )
    {
	    return pStroke->AddPoints( points, iPointCnt );
    }
    return 0;
}

int CInkData::AddPointsToStroke( UInt32 uStroke, const CGPoint * points, int iPointCnt )
{
	PHStroke *pStroke = GetStroke( uStroke );
    if ( NULL != pStroke )
    {
	    return pStroke->AddPoints( points, iPointCnt );
    }
    return 0;
}

void CInkData::EnableUndo( BOOL enabled )
{
    m_bUndoEnabled = enabled;
    m_Undo.FreeAll();
    m_Redo.FreeAll();
}

int CInkData::RemoveStroke( int iStroke )
{
    PHStroke *pStroke = GetStroke(iStroke);
	if ( NULL != pStroke )
    {
        if ( m_bRecordingUndo && m_bUndoEnabled )
            m_Undo.AddStroke( pStroke );
        else
            delete pStroke;
        m_bModified = true;
        RemoveAt( iStroke );
    }
    return 1;
}

int CInkData::StrokesTotal( void ) 
{
    return GetSize();
}

BOOL CInkData::IsStrokeSelected( int uStroke )
{
	PHStroke *pStroke = GetStroke( uStroke );
    if ( NULL != pStroke )
        return pStroke->IsSelected();
    return false;
}

int CInkData::HasStrokes( BOOL bSelected )
{
    if ( bSelected )
    {
		int iSelected = 0;
        for ( register int i = StrokesTotal() - 1; i >= 0; i-- )
        {
            if ( IsStrokeSelected(i) )
				iSelected++;
        }
        return iSelected;
    }
    return StrokesTotal();
}

BOOL CInkData::ResizeStroke( UInt32 uStroke, float x0, float y0, float scalex, float scaley, BOOL bReset, LPRECT lpRect /* = NULL */ )
{
    PHStroke *pStroke = GetStroke(uStroke);
    if ( NULL == pStroke )
        return false;
	
    // get first point
    PHSubStroke *pList = pStroke->GetSubStroke();
    if ( NULL == pList )
        return false;
	
    if ( pList->GetSize() < 1 )
        return false;
	
	if ( m_bRecordingUndo && m_bUndoEnabled )
    {
		m_Undo.AddStroke( pStroke );
    }

    RECT    rect1 = {0}, rect2 = {0};
    // scale the ink and calc the size before and after
    CalcStrokeRect( pStroke, &rect1 );	
	pStroke->SetScaledPoints( x0, y0, scalex, scaley, false, &rect2 );
		
    if ( lpRect )
    {
        // calculate the area that needs to be repainted
        CalcStrokeRect( pStroke, &rect2 );
        if ( UnionRect( lpRect, &rect1, &rect2 ) )
        {
            float   nWidth = pStroke->GetWidth() + 1.0f;
            InflateRect( lpRect, 2.0f * nWidth, 2.0f * nWidth );
        }
    }
    m_bModified = true;
    return true;
}

BOOL CInkData::SaveScaledStroke( UInt32 uStroke )
{
    PHStroke *pStroke = GetStroke(uStroke);
    if ( NULL == pStroke )
        return false;
	return (pStroke->StoreScaledPoints() > 0);
}

BOOL CInkData::ResetScaledStroke( UInt32 uStroke )
{
    PHStroke *pStroke = GetStroke(uStroke);
    if ( NULL == pStroke )
        return false;
	return pStroke->RevertToOriginal();
}

inline float DistanceSQ( float x1, float y1, float x2, float y2, float x0, float y0, float proximity )
{
	float aa = (x2-x1);
	float bb = (y2-y1);
	
	if ( (aa*aa + bb*bb) < (proximity * proximity) )
	{
		float aa1 = (x1-x0);
		float bb1 = (y1-y0);
		float aa2 = (x2-x0);
		float bb2 = (y2-y0);
		float r1 = (aa1*aa1 + bb1*bb1);
		float r2 = (aa2*aa2 + bb2*bb2);
		float result = min( r1, r2 );
		return result;
	}
	else
	{
        float result = -1.0;
		float dd = (x2-x1)*(y1-y0)-(x1-x0)*(y2-y1);
		if ( dd == 0.0 )
			result = 0.0;
		else if ( aa == 0.0 && bb == 0.0 )
			result = (dd*dd);
        else
            result = (dd * dd)/(aa*aa + bb*bb);
        if ( result < proximity * proximity )
        {
            RECT    r;
            r.left = min( x1, x2 );
            r.right = max( x1, x2 );
            r.top = min( y1, y2 );
            r.bottom = max( y1, y2 );
            POINTS pt = { x0, y0 };
            if ( ! PtInRect( &r, pt ) )
                result = -1.0;
        }
		return result;		
	}
}

int CInkData::SelectStrokesInRect( const LPRECT rect )
{
	int nResult = 0;
	SelectAllStrokes( false );
	for ( register int i = StrokesTotal()-1; i >= 0; i-- )
	{
		if ( IsStrokeInRect( i, rect ) )
		{
			PHStroke *pStroke = GetStroke(i);
			pStroke->SetSelected( true );
			pStroke->StoreScaledPoints();
			nResult++;
		}
	}	
	return nResult;
}

#define MIN_DELTA	3

BOOL CInkData::IsStrokeInRect( int nStroke, const LPRECT rect )
{
	PHStroke *pStroke = GetStroke(nStroke);
	if ( pStroke != NULL )
	{
		PHSubStroke * pList = pStroke->GetSubStroke();
		int     iCnt = pList->GetSize();
		PHPoint pt1, pt2;
		POINTS	pt;
        
        for ( register int i = 0; i < iCnt; i++ )
		{
			pt2 = pList->GetAt(i);
			pt.x = pt2.X(); pt.y = pt2.Y();
			if ( PtInRect( rect, pt ) )
				return true;
			if ( i > 0 )
			{
				pt1 = pList->GetAt(i-1);
				float		delta = (pt1.X() + pt1.Y()) - (pt2.X() + pt2.Y());
				if ( delta < 0 )
					delta = -delta;
				if ( delta >= 2.0f * MIN_DELTA )
				{
					int		cnt = (int)(delta/MIN_DELTA);
					float	deltaX = pt2.X() - pt1.X();
					float	deltaY = pt2.Y() - pt1.Y();
					for ( int j = 1; j < cnt; j++ )
					{
						pt.x = pt1.X() + (deltaX * j)/cnt;
						pt.y = pt1.Y() + (deltaY * j)/cnt;
						if ( PtInRect( rect, pt ) )
						{
							return true;
						}
					}
				}
			}
		}		
	}
	return false;
}

int CInkData::IsPointNearStroke( POINTS point, float proximity )
{
    if ( proximity <= 0.0 )
        proximity = DIST_TOUCH;
	for ( register int i = StrokesTotal()-1; i >= 0; i-- )
	{
		PHStroke *pStroke = GetStroke(i);
		if ( NULL != pStroke )
		{
			PHSubStroke * pList = pStroke->GetSubStroke();
			float	ds; // aa1, bb1;
			int     iCnt = pList->GetSize();
			PHPoint pt1, pt2;
            pt1 = pList->GetAt(0);
			for ( register int n = 1; n < iCnt; n++ )
			{
                
                pt2 = pList->GetAt(n);
                ds = DistanceSQ( pt1.X(), pt1.Y(), pt2.X(), pt2.Y(), point.x, point.y, proximity );
                pt1 = pt2;
                /*/
				pt1 = pList->GetAt(n);
				aa1 = (pt1.X()-point.x);
				bb1 = (pt1.Y()-point.y);
				ds = (aa1*aa1 + bb1*bb1); */
				if ( ds >= 0.0 && ds < (proximity * proximity) )
					return i;
			}
		}
	}
	return -1;
}

typedef struct line {
    CGPoint p1;
    CGPoint p2;
} line;


inline BOOL check_lines(line *line1, line *line2, CGPoint *hitp)
{
    /* Introduction:
     * This code is based on the solution of these two input equations:
     *  Pa = P1 + ua (P2-P1)
     *  Pb = P3 + ub (P4-P3)
     *
     * Where line one is composed of points P1 and P2 and line two is composed
     *  of points P3 and P4.
     *
     * ua/b is the fractional value you can multiple the x and y legs of the
     *  triangle formed by each line to find a point on the line.
     *
     * The two equations can be expanded to their x/y components:
     *  Pa.x = p1.x + ua(p2.x - p1.x) 
     *  Pa.y = p1.y + ua(p2.y - p1.y) 
     *
     *  Pb.x = p3.x + ub(p4.x - p3.x)
     *  Pb.y = p3.y + ub(p4.y - p3.y)
     *
     * When Pa.x == Pb.x and Pa.y == Pb.y the lines intersect so you can come 
     *  up with two equations (one for x and one for y):
     *
     * p1.x + ua(p2.x - p1.x) = p3.x + ub(p4.x - p3.x)
     * p1.y + ua(p2.y - p1.y) = p3.y + ub(p4.y - p3.y)
     *
     * ua and ub can then be individually solved for.  This results in the
     *  equations used in the following code.
     */
    
    // Denominator for ua and ub are the same so store this calculation 
    float d =   ((line2->p2.y - line2->p1.y)*(line1->p2.x-line1->p1.x) - 
                (line2->p2.x - line2->p1.x)*(line1->p2.y-line1->p1.y));
    float n_a = ((line2->p2.x - line2->p1.x)*(line1->p1.y-line2->p1.y) - 
                (line2->p2.y - line2->p1.y)*(line1->p1.x-line2->p1.x));    
    float n_b = ((line1->p2.x - line1->p1.x)*(line1->p1.y - line2->p1.y) -
                (line1->p2.y - line1->p1.y)*(line1->p1.x - line2->p1.x));
    
    if ( d == 0.0 )
        return false;
    
    // Calculate the intermediate fractional point that the lines potentially intersect.
    float ua = n_a/d;
    float ub = n_b/d;
    
    if( ua >= 0.0 && ua <= 1.0 && ub >= 0.0 && ub <= 1.0 )
    {
        hitp->x = line1->p1.x + (ua * (line1->p2.x - line1->p1.x));
        hitp->y = line1->p1.y + (ua * (line1->p2.y - line1->p1.y));
        return true;
    }
    return false;
}

BOOL CInkData::CurveIntersectsStroke( int nStroke, const CGTracePoint * points, int nPointCount )
{
    if ( nStroke < 0 || nStroke >= StrokesTotal() || points == NULL || nPointCount < 3 )
        return false;
    PHStroke *pStroke = GetStroke(nStroke);
    if ( NULL != pStroke )
    {
        PHSubStroke * pList = pStroke->GetSubStroke();
        int     iCnt = pList->GetSize();
        line    line1, line2;
        CGPoint c_point;
		CGFloat nWidth = (CGFloat)((pStroke->GetWidth() + 1.0) / 2.0);
        
        if ( iCnt == 1 )
        {
            RECT    r;
            
            r.left = pList->GetAt(0).GetPoint().pt.x - nWidth;
            r.right = pList->GetAt(0).GetPoint().pt.x + nWidth;
            r.top = pList->GetAt(0).GetPoint().pt.y - nWidth;
            r.bottom = pList->GetAt(0).GetPoint().pt.y + nWidth;
            for ( register int i = 0; i < nPointCount; i++ )
            {
                POINT   p = { static_cast<int>(points[i].pt.x), static_cast<int>(points[i].pt.y) };
                if ( PtInRect( &r, p ) )
                {
                    return true;
                }
            }
            return false;
        }
        else if ( iCnt == 2 )
        {
            CGFloat xx = pList->GetAt(0).GetPoint().pt.x - pList->GetAt(1).GetPoint().pt.x;
            CGFloat yy = pList->GetAt(0).GetPoint().pt.y - pList->GetAt(1).GetPoint().pt.y;
            if ( xx*xx + yy*yy <= nWidth * nWidth * 4.0 )
            {
                RECT    r;
                
                r.left = pList->GetAt(0).GetPoint().pt.x - nWidth;
                r.right = pList->GetAt(0).GetPoint().pt.x + nWidth;
                r.top = pList->GetAt(0).GetPoint().pt.y - nWidth;
                r.bottom = pList->GetAt(0).GetPoint().pt.y + nWidth;
                for ( register int i = 0; i < nPointCount; i++ )
                {
                    POINT   p = { static_cast<int>(points[i].pt.x), static_cast<int>(points[i].pt.y) };
                    if ( PtInRect( &r, p ) )
                    {
                        return true;
                    }
                }
                return false;
            }
        }
        
        line2.p1 = points[0].pt;
        for ( int j = 1; j < nPointCount; j++ )
        {
            line2.p2 = points[j].pt;
            line1.p1 = pList->GetAt(0).GetPoint().pt;
            for ( register int n = 1; n < iCnt; n++ )
            {
                line1.p2 = pList->GetAt(n).GetPoint().pt;
                if ( check_lines( &line1, &line2, &c_point ) )
                {
                    if ( c_point.x >= min( line1.p1.x, line1.p2.x )-nWidth && c_point.x <= max( line1.p1.x, line1.p2.x )+nWidth &&
                         c_point.x >= min( line2.p1.x, line2.p2.x )-nWidth && c_point.x <= max( line2.p1.x, line2.p2.x )+nWidth &&
                         c_point.y >= min( line1.p1.y, line1.p2.y )-nWidth && c_point.y <= max( line1.p1.y, line1.p2.y )+nWidth &&
                         c_point.y >= min( line2.p1.y, line2.p2.y )-nWidth && c_point.y <= max( line2.p1.y, line2.p2.y )+nWidth)
                    {
                        return true;
                    }
                    else
                    {
                        c_point.x = c_point.y = 0.0;
                    }
                }
                line1.p1 = line1.p2;
            }
            line2.p1 = line2.p2;
        }
    }
    return false;
}

int CInkData::DeleteIntersectedStrokes( const CGTracePoint * points, int nPointCount )
{
    int nDeleted = 0;
    RECT    rect;
    
    if ( nPointCount < 3 || points == NULL )
        return nDeleted;
    
    rect.left = rect.right = points[0].pt.x;
    rect.top = rect.bottom = points[0].pt.y;
    
    for ( int i = 1; i < nPointCount; i++ )
    {
        if ( (points[i].pt.x - 1) < rect.left )
            rect.left = points[i].pt.x - 2;
        if ( (points[i].pt.x + 1) > rect.right )
            rect.right = points[i].pt.x + 2;
        if ( (points[i].pt.y - 1) < rect.top )
            rect.top = points[i].pt.y - 2;
        if ( (points[i].pt.y + 1) > rect.bottom )
            rect.bottom = points[i].pt.y + 2;
    }

    BOOL bUndoStarted = false;    
    for ( int i = StrokesTotal()-1; i >= 0; i-- )
    {
        if ( IsStrokeInRect( i, &rect ) )
        {
            if ( CurveIntersectsStroke( i, points, nPointCount ) )
            {
                if ( !bUndoStarted )
                {
                    RecordUndo(UNDO_STROKES_DELETED);
                    bUndoStarted = true;
                }
                RemoveStroke(i);
                nDeleted++;
            }
        }
    }
	if ( bUndoStarted )
        StopRecordingUndo();

    return nDeleted;
}

BOOL CInkData::MoveStroke( UInt32 uStroke, float xOff, float yOff, LPRECT lpRect /* = NULL */ )
{
    PHStroke *pStroke = GetStroke(uStroke);
    if ( NULL == pStroke )
        return false;
	
    PHSubStroke *pList = pStroke->GetSubStroke();
    if ( NULL == pList )
        return false;	
    int     iCnt = pList->GetSize();
    float   nWidth = pStroke->GetWidth()+1.0f;
    if ( iCnt < 1 )
        return false;
	
    if ( m_bRecordingUndo && m_bUndoEnabled )
    {
        m_Undo.AddStroke( pStroke );
    }

    // get first point
    PHPoint   pt = pList->GetAt(0);
    if ( NULL != lpRect )
    {
        lpRect->left = lpRect->right = pt.X();
        lpRect->bottom = lpRect->top = pt.Y();
        InflateRect( lpRect, nWidth, nWidth );
    }
	
	pStroke->ResetPosition();
	
    register int i = 0;
	
    for ( i = 0; i < iCnt; i++ )
	{
		pt = pList->GetAt(i);
		
        if ( NULL != lpRect )
        {
            // calc stroke boundries rect before offset
		    if ( (pt.X() - nWidth) < lpRect->left )
			    lpRect->left = pt.X() - nWidth;
		    if ( (pt.X() + nWidth) > lpRect->right ) 
			    lpRect->right = pt.X() + nWidth;
		    if ( (pt.Y() - nWidth) < lpRect->top )
			    lpRect->top = pt.Y() - nWidth;
		    if ( (pt.Y() + nWidth) > lpRect->bottom )
			    lpRect->bottom = pt.Y() + nWidth;
        }
		
        pt.offset( xOff, yOff );
        pList->SetAt( i, pt );
		
        if ( NULL != lpRect )
        {
            // calc stroke boundries rect after offset
		    if ( (pt.X() - nWidth) < lpRect->left )
			    lpRect->left = pt.X() - nWidth;
		    if ( (pt.X() + nWidth) > lpRect->right ) 
			    lpRect->right = pt.X() + nWidth;
		    if ( (pt.Y() - nWidth) < lpRect->top )
			    lpRect->top = pt.Y() - nWidth;
		    if ( (pt.Y() + nWidth) > lpRect->bottom )
			    lpRect->bottom = pt.Y() + nWidth;
        }
	}
	
    if ( pList != pStroke->GetSubStroke1() )
    {
        // move the original pixels, if different
        pList = pStroke->GetSubStroke1();
        iCnt = pList->GetSize();
        for ( i = 0; i < iCnt; i++ )
        {
		    pt = pList->GetAt(i);
            pt.offset( xOff, yOff );
            pList->SetAt( i, pt );
        }
    }
    m_bModified = true;
    return true;
}

UInt32 CInkData::ReadOneStroke( UInt32 uStroke, CGTracePoint* pPoints, float& fWidth, COLORREF& Color, int& uPntCnt )
{
    uPntCnt = 0;
    PHStroke *pStroke = GetStroke(uStroke);
    if ( NULL == pStroke )
        return -1;
	
    PHSubStroke *pList = pStroke->GetSubStroke();
    if ( NULL == pList )
        return -1;
	
    uPntCnt = pList->GetSize();
    if ( NULL != pPoints )
    {
	    for( register int i = 0; i < (int)uPntCnt; i++ )
	    {
            pPoints[i].pt = pList->GetAt(i).GetPoint().pt;
            pPoints[i].pressure = pList->GetAt(i).GetPoint().pressure;
	    }
    }
	
    Color = pStroke->GetColor(); 
    fWidth = pStroke->GetWidth();
    return uPntCnt;
}

int CInkData::ReadOneStroke( UInt32 uStroke, CGTracePoint * pPoints, int * pnPntCnt, float * pfWidth, COLORREF * pColor )
{
	int		nPoints = 0;
    PHStroke *pStroke = GetStroke(uStroke);
    if ( NULL == pStroke )
        return -1;
	
    PHSubStroke *pList = pStroke->GetSubStroke();
    if ( NULL == pList )
        return -1;
	
    nPoints = pList->GetSize();
    if ( NULL != pPoints )
    {
	    for( register int i = 0; i < nPoints; i++ )
	    {
            pPoints[i].pt = pList->GetAt(i).GetPoint().pt;
            pPoints[i].pressure = pList->GetAt(i).GetPoint().pressure;
	    }
    }
	
	if ( NULL != pColor )
		*pColor = pStroke->GetColor(); 
	if ( NULL != pfWidth )
		*pfWidth = pStroke->GetWidth();
	if ( NULL != pnPntCnt )
		*pnPntCnt = nPoints;
    return nPoints;
}

int CInkData::ReadOneStroke( UInt32 uStroke, CGPoint * pPoints, int * pnPntCnt, float * pfWidth, COLORREF * pColor )
{
	int		nPoints = 0;
    PHStroke *pStroke = GetStroke(uStroke);
    if ( NULL == pStroke )
        return -1;
	
    PHSubStroke *pList = pStroke->GetSubStroke();
    if ( NULL == pList )
        return -1;
	
    nPoints = pList->GetSize();
    if ( NULL != pPoints )
    {
	    for( register int i = 0; i < nPoints; i++ )
	    {
		    pPoints[i] = pList->GetAt(i).GetPoint().pt;
	    }
    }
	
	if ( NULL != pColor )
		*pColor = pStroke->GetColor(); 
	if ( NULL != pfWidth )
		*pfWidth = pStroke->GetWidth();
	if ( NULL != pnPntCnt )
		*pnPntCnt = nPoints;
    return nPoints;
}

UInt32 CInkData::GetStrokePointCnt( UInt32 uStroke )
{
    PHStroke *pStroke = GetStroke(uStroke);
    return (NULL == pStroke) ? 0 : pStroke->GetSubStroke()->GetSize();
}

BOOL CInkData::RemoveSelected( BOOL bDeleteAll /* =false */ )
{
    BOOL bUndoStarted = false;
	
    for ( register int i = StrokesTotal() - 1; i >= 0; i-- )
    {
        if ( bDeleteAll || IsStrokeSelected(i) )
        {
            if ( !bUndoStarted )
            {
                RecordUndo(UNDO_STROKES_DELETED);
                bUndoStarted = true;
            }
            RemoveStroke(i);
        }
    }
	
#ifdef IMAGE_SUPPORT
	if ( m_pCurrentImageObject != NULL )
	{
		int index = GetImageObjectIndex(m_pCurrentImageObject);
		m_pCurrentImageObject->m_iIndex = index;
		RecordUndo( UNDO_IMAGE_DELETED, m_pCurrentImageObject, index, true );
		DeleteImageObject(m_pCurrentImageObject);
		m_pCurrentImageObject = NULL;
		bUndoStarted = true;
	}
#endif
	
	if ( bUndoStarted )
        StopRecordingUndo();
    SetModified( true );
	
    return bUndoStarted;
}

void CInkData::SelectStroke( int nStroke, BOOL bSelect /* = true */ )
{
    PHStroke * pStr = GetStroke( nStroke );
    if ( pStr ) 
	{
        pStr->SetSelected( bSelect );
		pStr->StoreScaledPoints();
	}
}


BOOL CInkData::SelectAllStrokes( BOOL bSelect /* = true */ )
{
	BOOL    bResult = false;
    register PHStroke * pStr = NULL;
    for ( register int nStroke = 0; nStroke < StrokesTotal(); nStroke++ )
    {
        pStr = GetStroke( nStroke );
        if ( pStr ) 
        {
            // return true if state of at least one stroke changes
			if ( bSelect )
				pStr->StoreScaledPoints();
            if ( !((pStr->IsSelected() && bSelect) ||
				   (! pStr->IsSelected() && ! bSelect)) )
            {
                bResult = true;
                pStr->SetSelected( bSelect );
            }
        }
    }
    return bResult;
}

void CInkData::SetStrokesRecognizable( BOOL bSet /* = true */, BOOL bSelectedOnly /* = true */ )
{
    register PHStroke * pStr = NULL;
    for ( register int nStroke = 0; nStroke < StrokesTotal(); nStroke++ )
    {
        pStr = GetStroke( nStroke );
        if ( pStr ) 
        {
            if ( !bSelectedOnly || pStr->IsSelected() )
            {
                pStr->SetRecognizable( bSet );
                m_bModified = true;
            }
        }
    }
}

void CInkData::SetStrokeRecognizable( int nStroke, BOOL bSet /* = true */ )
{
    PHStroke * pStr = GetStroke( nStroke );
    if ( pStr ) 
        pStr->SetRecognizable( bSet );
}

BOOL CInkData::IsStrokeRecognizable( int nStroke )
{
    PHStroke * pStr = GetStroke( nStroke );
    if ( pStr ) 
        return pStr->IsRecognizable();
    return false;
}

void CInkData::SetSelColor( COLORREF Color )
{
    BOOL bUndoStarted = false;
    register PHStroke *pStroke;
	
    for ( register int i = StrokesTotal() - 1; i >= 0; i-- )
    {
        if ( IsStrokeSelected(i) )
        {
            pStroke = GetStroke( i );
            if ( pStroke ) 
            {
                if ( m_bUndoEnabled )
                {
                    if ( !bUndoStarted )
                    {
                        RecordUndo(UNDO_STROKES_CHANGED);
                        bUndoStarted = true;
                    }
                    m_Undo.AddStroke( pStroke );
                }
                pStroke->SetColor( Color );
            }
        }
    }
	
    if ( bUndoStarted )
    {
        StopRecordingUndo();
        m_bModified = true;
    }
}

void CInkData::ChangeZOrder( int iDepth, BOOL bFwd )
{
    BOOL bUndoStarted = false;
    int tot = StrokesTotal();
    int nMoved = 0;
    register PHStroke * pStroke = NULL;
	
#ifdef IMAGE_SUPPORT
    if ( m_pCurrentImageObject != NULL )
	{ 
		if(bFwd)
		{
			int max = INT_MIN;
			for(int i = 0; i < GetImageObjectCnt(); i++)
			{
				if(m_arrImages.GetAt(i)->m_attrib.iZOrder > max)
				{
					max = m_arrImages.GetAt(i)->m_attrib.iZOrder;
				}
			}
			if(max + 1 < StrokesTotal())
				max = StrokesTotal();
			m_pCurrentImageObject->m_attrib.iZOrder = max + 1;			
		}
		else
		{
			int min = INT_MAX;
			for(int i = 0; i < GetImageObjectCnt(); i++)
			{
				if(m_arrImages.GetAt(i)->m_attrib.iZOrder < min)
				{
					min = m_arrImages.GetAt(i)->m_attrib.iZOrder;
				}
			}
			if(min - 1 >= StrokesTotal())
				min = StrokesTotal();
			
			m_pCurrentImageObject->m_attrib.iZOrder = min - 1;
		}
		m_bModified = true;
		return;
	}
#endif // IMAGE_SUPPORT
	
	if ( bFwd ) // bring forward
    {
        if ( iDepth == 0 )  // to the end
        {
            for ( register int i = 0; i < tot; i++ )
            {
                pStroke = GetStroke(i);
                if ( pStroke->IsSelected() )
                {
                    RemoveAt(i);
                    InsertAt( nMoved, pStroke );
                    nMoved ++;
					
                    // RecordUndo(UNDO_STROKES_DELETED);
                    bUndoStarted = true;
                }
            }
#ifdef IMAGE_SUPPORT			
			//move all images behind this stroke
			for(int i = 0; i < GetImageObjectCnt(); i++)
			{
				if(m_arrImages.GetAt(i)->m_attrib.iZOrder >= tot)
				{
					m_arrImages.GetAt(i)->m_attrib.iZOrder = tot - 1;
				}
			}
#endif // IMAGE_SUPPORT
        }
    }
    else
    {
        if ( iDepth == 0 )  // to the end
        {
            for ( register int i = tot - 1; i >= 0; i-- )
            {
                pStroke = GetStroke(i);
                if ( pStroke->IsSelected() )
                {
                    RemoveAt(i);
                    InsertAt( tot-nMoved-1, pStroke );
                    nMoved ++;
					
                    // RecordUndo(UNDO_STROKES_DELETED);
                    bUndoStarted = true;
                }
            }
        }
    }
	
    if ( bUndoStarted )
    {
        StopRecordingUndo();
        m_bModified = true;
    }
}

void CInkData::SetSelWidth( float fWidth )
{
    BOOL bUndoStarted = false;
    register PHStroke *pStroke;
	
    for ( register int i = StrokesTotal() - 1; i >= 0; i-- )
    {
        if ( IsStrokeSelected(i) )
        {
            pStroke = GetStroke( i );
            if ( pStroke ) 
            {
                if ( m_bUndoEnabled )
                {
                    if ( !bUndoStarted )
                    {
                        RecordUndo(UNDO_STROKES_CHANGED);
                        bUndoStarted = true;
                    }
                    m_Undo.AddStroke( pStroke );
                }
                pStroke->SetWidth( fWidth );
            }
        }
    }
	
    if ( bUndoStarted )
    {
        StopRecordingUndo();
        m_bModified = true;
    }
}

void CInkData::SetStrokeColor( int nStroke, COLORREF Color )
{
    PHStroke * pStr = GetStroke( nStroke );
    if ( pStr ) 
    {
        if ( m_bRecordingUndo && m_bUndoEnabled )
            m_Undo.AddStroke( pStr );
        pStr->SetColor( Color );
    }
}

void CInkData::SetStrokeWidth( int nStroke, float fWidth )
{
    PHStroke * pStr = GetStroke( nStroke );
    if ( pStr ) 
    {
        if ( m_bRecordingUndo && m_bUndoEnabled )
            m_Undo.AddStroke( pStr );
        pStr->SetWidth( fWidth );
    }
}

void CInkData::SetStroke( int nStroke, float fWidth, COLORREF clr )
{
    PHStroke * pStr = GetStroke( nStroke );
    if ( pStr ) 
    {
        if ( m_bRecordingUndo && m_bUndoEnabled )
            m_Undo.AddStroke( pStr );
        pStr->SetWidth( fWidth );
        pStr->SetColor( clr );
    }
}

int CInkData::Write( DWORD dwFormat, FILE * pFile, void ** ppData, long * pcbSize )
{
    DWORD dwFmt = INK_DATA_FMT(dwFormat);
    if ( IS_MEM_STREAM(dwFormat) )
    {		
        switch( dwFmt )
        {
            case INK_RAW :
                *pcbSize = CopyRawInk( ppData );
                return (*pcbSize > 0 );
				
            case INK_PWCOMP :
				{
					CPHMemStream phStream;
					
					if ( WritePhatWareInk( phStream, 0 != (dwFormat & IGNORE_LAST_STROKE), 0 != (dwFormat & SAVE_PRESSURE) ) == TRFILE_OK )
					{
						long	dwSize = phStream.GetSize();
						LPBYTE	pMem = (LPBYTE)malloc( dwSize );
						if ( NULL != pMem )
						{
							LPBYTE  pTrace = pMem;
							phStream.SetPos( 0, SEEK_SET );
							phStream.Read( pTrace, dwSize );
							*pcbSize = dwSize;
							*ppData = (void *)pMem;
							return (*pcbSize > 0 );
						}
					}
				}
			    break;
				
            default:
                break;
        }
        return 0;
    }
    else
    {
        switch( dwFmt )
        {
            case INK_RAW :
                WriteRawInk( pFile, dwFormat );
			    break;
				
            case INK_PWCOMP :
                WritePhatWareInk( pFile, dwFormat );
			    break;
        }
    }
    return 1;
}

int CInkData::Read( DWORD dwFormat, FILE * pFile, void * pData, long cbSize, BOOL skipImages )
{
    DWORD dwFmt = INK_DATA_FMT(dwFormat);
    int ret = 1;
	
    FreeUndo();

    if ( IS_MEM_STREAM(dwFormat) )
    {
        switch( dwFmt )
        {
            case INK_RAW :
                PasteRawInk( (LPBYTE)pData, cbSize );
			    return 1;
				
            case INK_PWCOMP :
				{
					LPBYTE pBytes = (LPBYTE)pData;
					int ret = TRFILE_BADFILE;
					if ( pBytes )
					{
						CPHMemStream phStream( pBytes, cbSize );
						ret = ReadPhatWareInk( phStream, skipImages );
						return (ret == TRFILE_OK) ? 1 : 0;
					}
				}
                break;
				
            default:
                break;
        }
        return 0;
    }
    else
    {
        switch( dwFmt )
        {
            case INK_RAW :
                ReadRawInk( pFile, dwFormat );
                break;
				
            case INK_PWCOMP :
                ReadPhatWareInk( pFile,  dwFormat);
                break;
				
            default:
                return 0;
        }
    }
	
    return ret;
}

int CInkData::ReadRawInk( FILE * pFile, DWORD dwFlags )
{
    CPHFileStream phStream( pFile );
    return ReadRawFromStream( phStream ) ? TRFILE_OK : TRFILE_BADFILE;
}

int CInkData::WriteRawInk( FILE * pFile, DWORD dwFlags )
{
    CPHFileStream phStream(pFile);
    return WriteRawToStream(phStream);
}

int CInkData::WriteRawToStream( CPHStream &phStream, BOOL bSelectedOnly /*=false*/ )
{
    RawTraceHeader RawTraceHdr;
    RawStrokeHeader RawStrokeHdr;
	int nTotal = StrokesTotal();
	
    RawTraceHdr.uStrokeCnt = bSelectedOnly ? HasStrokes(true) : nTotal;
    RawTraceHdr.wFlags = 0;
	
    if ( !phStream.Write( &RawTraceHdr, sizeof(RawTraceHeader) ) )
        return TRFILE_BADFILE;
	
    if ( 0 == RawTraceHdr.uStrokeCnt )
        return TRFILE_OK;
	
    for ( register int i = 0; i < nTotal; i++ )
    {
		if ( bSelectedOnly && !IsStrokeSelected(i) )
			continue;
		
        UInt32 uCnt = GetStrokePointCnt( i );
        CGTracePoint *pPnt = new CGTracePoint[uCnt];
		
        if ( NULL == pPnt )
            return TRFILE_BADFILE;
		
        RawStrokeHdr.uPointCount = 0;
        ReadOneStroke( i, pPnt, RawStrokeHdr.fPenWidth, RawStrokeHdr.lColor, RawStrokeHdr.uPointCount );
		
        PHStroke *pStroke = GetStroke(i);
        if ( NULL != pStroke )
            RawStrokeHdr.dwFlags = pStroke->GetStrokeFlags();
		
        //send stroke header first
        if ( !phStream.Write( &RawStrokeHdr, sizeof( RawStrokeHeader ) ) )
        {
            delete [] pPnt;
            return TRFILE_BADFILE;
        }
		
        //send the raw points 
        if ( !phStream.Write( pPnt, RawStrokeHdr.uPointCount * sizeof( CGTracePoint ) ) )
        {
            delete [] pPnt;
            return TRFILE_BADFILE;
        }   
        delete [] pPnt;
    }
    return TRFILE_OK;
}

BOOL CInkData::PasteRawInk( const void * pMem, long cbSize, float x /* =0 */, float y /* =0 */ )
{
    if ( pMem != NULL )
    {
        int ret;
	    CPHMemStream phStream( (LPBYTE)pMem, cbSize );
        RecordUndo(UNDO_STROKES_ADDED);
        ret = ReadRawFromStream( phStream, x, y, true );
        StopRecordingUndo();
        if ( ret )
            m_bModified = true;
        return ret;
    }
	return false;
}

BOOL CInkData::ReadRawFromStream( CPHStream &phStream, float x /* =0 */, float y /* =0 */, BOOL select )
{
    // read strokes from memory
    RawTraceHeader RawTraceHdr = {0};
    RawStrokeHeader RawStrokeHdr = {0};
	
    if ( ! phStream.Read( (BYTE *)&RawTraceHdr, sizeof(RawTraceHdr) ) )
        return false;
	
    float xMin = 0.0f, yMin = 0.0f;
    int nFirstStroke = -1;
    RECT rc;
	
    for ( register UInt32 i = 0; i < RawTraceHdr.uStrokeCnt; i++ )
    {
        phStream.Read( &RawStrokeHdr, sizeof(RawStrokeHdr) );
		
        CGTracePoint * pPnt = new CGTracePoint[RawStrokeHdr.uPointCount];		
        if ( NULL != pPnt )
        {
            phStream.Read( pPnt, RawStrokeHdr.uPointCount * sizeof( CGTracePoint ) );
            int nStroke = AddNewStroke( RawStrokeHdr.fPenWidth, RawStrokeHdr.lColor, RawStrokeHdr.dwFlags );
            if ( nStroke >= 0 )
            {
                AddPointsToStroke( nStroke, pPnt, (int)RawStrokeHdr.uPointCount );
				
                PHStroke *pStroke = GetStroke(nStroke);
				pStroke->SetSelected( select );
                if ( NULL != pStroke )
                {
                    pStroke->SetStrokeFlags(RawStrokeHdr.dwFlags);
					
                    CalcStrokeRect( pStroke, &rc );
					
                    if ( nFirstStroke == - 1 )
                    {
                        nFirstStroke = nStroke;
                        xMin = rc.left;
                        yMin = rc.top;
                    }
                    else
                    {
                        xMin = min( xMin, rc.left );
                        yMin = min( yMin, rc.top );
                    }
				}
            }
            delete [] pPnt;
        }
    }
	
    // offset the inserted strokes
    if ( nFirstStroke >= 0 && (x != 0 || y != 0) && (xMin != 0 || yMin != 0) )
    {
        for ( register UInt32 i = (UInt32)nFirstStroke; i < (UInt32)GetSize(); i++ )
        {
            PHStroke *pStroke = GetStroke(i);
            PHSubStroke *pList = pStroke->GetSubStroke();
            if ( NULL == pList )
                continue;
			
            int iCnt = pList->GetSize();
            if ( iCnt <= 0 )
                continue;
			
            for( register int k = 0; k < iCnt; k++ )
            {
		        CGTracePoint pt = pList->GetAt(k).GetPoint();
                pt.pt.x = pt.pt.x - xMin + x;
                pt.pt.y = pt.pt.y - yMin + y;
                pList->SetAt( k, pt );
            }
        }
    }
	
    return true;
}

long CInkData::CopyRawInk( void ** ppMem )
{
    CPHMemStream phStream;
	
    if ( WriteRawToStream( phStream, true ) == TRFILE_OK )
    {
        long dwSize = phStream.GetSize();
        LPBYTE pMem = (LPBYTE)malloc( dwSize );
        if ( NULL != pMem )
        {
            phStream.SetPos( 0, SEEK_SET );
            phStream.Read( pMem, dwSize );
			*ppMem = (void *)pMem;
			return dwSize;
        }
    }
    return 0;
}

BOOL CInkData::CalcTraceRect( LPRECT lpRect, BOOL bSelected /* = false */ )
{
    if ( StrokesTotal() < 1 )
        return false;
	
    BOOL			bResult = false;
    CGPoint			pt;
    UInt32			nStrokeCnt;
    PHSubStroke *	pList = NULL;
    PHStroke *		pStr = NULL;
    register int	nStroke, i, iCnt;
    float           nWidth;
	
    lpRect->left = lpRect->top = 32000;
    lpRect->bottom = lpRect->right = -32000;
	
    for ( nStroke = StrokesTotal() - 1; nStroke >= 0; nStroke-- )
    {
        nStrokeCnt = GetStrokePointCnt( nStroke );
        if ( nStrokeCnt > 0 )
        {
            pStr = GetStroke( nStroke );
            if ( pStr == NULL || (bSelected && (! pStr->IsSelected())) )
                continue;   // skip this stroke, it is not selected
            pList = pStr->GetSubStroke();
            if ( NULL == pList )
                continue;
            nWidth = pStr->GetWidth() + 1.0f;
            iCnt = pList->GetSize();
	        for( i = 0; i < iCnt; i++ )
	        {
		        pt = pList->GetAt(i).GetPoint().pt;
		        if ( (pt.x - nWidth) < lpRect->left )
			        lpRect->left = pt.x - nWidth;
		        if ( (pt.x + nWidth) > lpRect->right ) 
			        lpRect->right = pt.x + nWidth;
		        if ( (pt.y - nWidth) < lpRect->top )
			        lpRect->top = pt.y - nWidth;
		        if ( (pt.y + nWidth) > lpRect->bottom )
			        lpRect->bottom = pt.y + nWidth;
                bResult = true;
	        }
        }
    }
    if ( ! bResult )
	{
        memset( lpRect, 0, sizeof( RECT ) );      // empty rect
	}
	return bResult;
}

BOOL CInkData::CalcStrokeRect( int nStroke, LPRECT lpRect, BOOL bAddWidth /* = true */ )
{
    if ( nStroke < 0 || nStroke >= StrokesTotal() )
        return false;
    return CalcStrokeRect( GetStroke( nStroke ), lpRect, bAddWidth, NULL );
}

BOOL CInkData::CalcStrokeRect( PHStroke *  pStr, LPRECT lpRect, BOOL bAddWidth /* = true */, CGFloat * pnGrav /* = NULL */ )
{
    if ( NULL == pStr )
        return false;
    PHSubStroke *pList = pStr->GetSubStroke();
    if ( NULL == pList )
        return false;
	
    BOOL    bResult = false;
    float   nWidth = bAddWidth ? (pStr->GetWidth() + 1.0f) : 1.0f;
    int     iCnt = pList->GetSize();
    float   yGrav = 0;
	int		ng = 1;
	
    if ( iCnt < 1 )
        return false;
    CGPoint   pt = pList->GetAt(0).GetPoint().pt;
		
    lpRect->left = lpRect->right = pt.x;
    lpRect->bottom = lpRect->top = pt.y;
	InflateRect( lpRect, nWidth, nWidth );
    yGrav = pt.y;
    if ( iCnt < 2 )
    {
        bResult = true;
    }
    else
    {
		float			yFltPrv, yi, yf, yPrv;
		register int	i, j, NumFilteredPoints;
		yPrv = yFltPrv = pt.y;
        for( i = 1; i < iCnt; i++ )
        {
		    pt = pList->GetAt(i).GetPoint().pt;
		    if ( (pt.x - nWidth) < lpRect->left )
			    lpRect->left = pt.x - nWidth;
		    if ( (pt.x + nWidth) > lpRect->right ) 
			    lpRect->right = pt.x + nWidth;
		    if ( (pt.y - nWidth) < lpRect->top )
			    lpRect->top = pt.y - nWidth;
		    if ( (pt.y + nWidth) > lpRect->bottom )
			    lpRect->bottom = pt.y + nWidth;
            bResult = true;
			if ( pnGrav )
			{
				// smoth curve to get better gravity center calculation
				yGrav += pt.y;
				ng++;
				if ( i == 1 )
					NumFilteredPoints=1;
				else
					NumFilteredPoints=N_SEGS;
				for ( j = 1; j <= NumFilteredPoints; j++ ) 
				{
					yi = (pt.y - yPrv) * j / N_SEGS + yPrv;
					yf = (yi - yFltPrv) / N_SEGS + yFltPrv;
					if ( yf != yFltPrv ) 
					{
						yFltPrv = yf;
						yGrav += yf;
						ng++;
					}
				}
				yPrv = pt.y;
			}
        }
    }
    if ( pnGrav )
        *pnGrav = yGrav/ng;
	return bResult;
}

CGPoint CInkData::GetLastPoint( int nStroke )
{
    CGPoint pt = { 0, 0 };
	
    if ( nStroke >= 0 && nStroke < StrokesTotal() )
    {
	    PHStroke *pStroke = GetStroke( nStroke );
        PHSubStroke *pList = pStroke->GetSubStroke();
        int iCnt;
		
        if ( NULL != pList && (iCnt = pList->GetSize()) > 0 )
            pt = pList->GetAt(iCnt-1).GetPoint().pt;
    }
    return pt;
}

CGPoint CInkData::GetFirstPoint( int nStroke )
{
    CGPoint pt = { 0, 0 };
	
    if ( nStroke >= 0 && nStroke < StrokesTotal() )
    {
	    PHStroke *pStroke = GetStroke( nStroke );
        PHSubStroke *pList = pStroke->GetSubStroke();
        int iCnt;
		
        if ( NULL != pList && (iCnt = pList->GetSize()) > 0 )
            pt = pList->GetAt(0).GetPoint().pt;
    }
    return pt;
}

void CInkData::FreeStrokes( void )
{
	PHStroke *pStroke;
	int count = GetSize();
	
	for ( int i = 0; i < count; i++ )
	{
		pStroke = GetStroke(i);
		if ( pStroke )
			delete pStroke;
	}
	
	RemoveAll();
}

TRACE_FILE_STATUS CInkData::WritePhatWareInk( FILE * pFile, DWORD dwFlags)
{
	CPHFileStream phFile(pFile);
	return WritePhatWareInk( phFile, 0 != (dwFlags & IGNORE_LAST_STROKE),  0 != (dwFlags & SAVE_PRESSURE) );
}

TRACE_FILE_STATUS CInkData::WritePhatWareInk( CPHStream &phFile, BOOL bIgnoreLast, BOOL savePressure )
{
	int     nStrokes = GetSize();
	int     nCompressedStrokes = 0;
	BYTE    bColorIndex;
	
#if !defined(TEXT_SUPPORT) && !defined(IMAGE_SUPPORT)
    if ( nStrokes <= 0 )
        return TRFILE_OK;
#endif // TEXT_SUPPORT
	
	COLORREF            colors[MAX_COLOR] = {0};
	int                 nColors = 0;
	PHCompTraceHeader   header;
	PHStroke *          pStroke;
	register int        i, k;
    TRACE_FILE_STATUS   nResult = TRFILE_BADFILE;
    int                 nNonEmptyStrokes = 0;
	PHSubStroke *       pPoints = NULL;
	
	// Count and collect the colors used
	for ( i = 0; i < nStrokes; i++ )
	{
		pStroke = GetStroke(i);
		if ( pStroke )
		{
		    pPoints = pStroke->GetSubStroke();
            if ( pPoints->GetSize() > 0 )
                nNonEmptyStrokes ++;
			
			bColorIndex = 0xff;
			for ( k = 0; k < nColors; k++ )
			{
				if ( colors[k] == pStroke->GetColor() )
				{
					bColorIndex = k;
					break;
				}
			}
			
			if ( bColorIndex == 0xff && nColors < MAX_COLOR )
			{
				colors[nColors] = pStroke->GetColor();
				nColors ++;
			}
		}
	}
	
	if ( nColors < 0 )
		nColors = 1;
	
	header.m_dwStamp = PWINK_FILE_STAMP;
	strncpy( header.m_ucVersion, (savePressure) ? TRACE_FORMAT_VERSION_V105 : TRACE_FORMAT_VERSION_V104, VER_FLD_LEN );
	header.m_ucVersion[VER_FLD_LEN] = 0;
	header.m_uStrokes = nNonEmptyStrokes; // write the uncompressed number, overwrite later if required
	header.m_Options = 0;	// turned off for text saving
#ifdef TEXT_SUPPORT
	header.m_Options |= FHF_TEXTOBJECTS;	// turned on for text saving
#endif // TEXT_SUPPORT
#ifdef IMAGE_SUPPORT
	header.m_Options |= FHF_IMAGEOBJECTS;	// turned on for image saving
	header.m_Options |= FHF_IMAGEROTATION;	// turned on for image rotation support
#endif // TEXT_SUPPORT
    header.m_Options |= FHF_INKZORDER;
    if ( savePressure )
        header.m_Options |= FHF_INKPRESSURE;
    header.m_Options |= FHF_FLOATWIDTH;
	
	header.m_nColors = nColors;
    header.m_ucFlags = 0;
	
	// Write Header
    if ( !phFile.Write( (BYTE *)&header, sizeof(header) ) )
        return TRFILE_BADFILE;
	
	// Write color table
    if ( !phFile.Write( (BYTE *)colors, sizeof(COLORREF)*nColors ) )
        return TRFILE_BADFILE;
	
	// Write strokes
	BYTE *      pCompressedPoints = NULL;
	BYTE *      pDeltas = NULL;
	int         nCompressedPoints = 0, iCnt;
	short       startX = 0, startY = 0;
	short       prevX = 0, prevY = 0;
	short       deltaX, deltaY;
	CGTracePoint  pt;
	BYTE        deltaBits;
	BYTE        bStrokeFlags = 0;
	BOOL        bFirst = true, bSaved = false;
	BYTE        bPrevColorIndex = -1;
	short       bPrevWidth = -1;
	int         iDeltaByte, iDeltaInByte, iDeltasSize;
	BOOL        b3bit = true;
    int         zOrder = -1;
    short       bWidth = 100;
	
	for ( i = 0; i < nStrokes; i++ )
	{
		pStroke = GetStroke(i);
		if ( !pStroke )
			continue;
		
		pPoints = pStroke->GetSubStroke();
		iCnt = pPoints->GetSize();
        if ( iCnt == 0 )
            continue;
		
		bWidth = (short)(pStroke->GetWidth() * 100.0f);
		bStrokeFlags = 0;
		bFirst = true;
		bSaved = false;
		b3bit = true;
		
        if ( !pStroke->IsRecognizable() )
            bStrokeFlags |= NOT_REC_FLAG;
		
		// Get the stroke color index
		bColorIndex = 0;
		for ( k = 0; k < nColors; k++ )
		{
			if ( colors[k] == pStroke->GetColor() )
			{
				bColorIndex = k;
				break;
			}
		}
        
        zOrder = pStroke->ZOrder();
		
        if ( pCompressedPoints != NULL )
            delete [] pCompressedPoints;
		pCompressedPoints = new BYTE[iCnt];
		if ( pCompressedPoints == NULL )
			goto err;
		
		iDeltasSize = iCnt/4 + 1;
        if ( pDeltas != NULL )
            delete [] pDeltas;
		pDeltas = new BYTE[iDeltasSize];
		if ( pDeltas == NULL )
            goto err;
		
		for ( k = 0; k < iCnt; k++ )
		{
			pt.pt = pPoints->GetAt(k).GetPoint().pt;
            pt.pressure = pPoints->GetAt(k).GetPoint().pressure;
            pt.pt.x *= 4.0;
            pt.pt.y *= 4.0;
			bSaved = false;
						
			if ( bFirst )
			{
				bFirst = false;
				memset( pDeltas, 0, iDeltasSize );
				nCompressedPoints = 0;
				prevX = (short)(int)pt.pt.x;
				prevY = (short)(int)pt.pt.y;
				
				if ( (bStrokeFlags & CONNECT_DELTA_FLAG) == 0 )
				{
					startX = (short)(int)pt.pt.x;
					startY = (short)(int)pt.pt.y;
				}
				b3bit = true;
			}
			else
			{
				deltaX = (short)((int)pt.pt.x - prevX);
				deltaY = (short)((int)pt.pt.y - prevY);
				
				if ( (deltaX*deltaX + deltaY*deltaY) <= TOLERANCE )
				{
					// skip this point
					continue;
				}
				else if ( (deltaX > MAX_DELTA_4BIT) || (deltaX < -MAX_DELTA_4BIT) ||
						 (deltaY > MAX_DELTA_4BIT) || (deltaY < -MAX_DELTA_4BIT) )
				{					
					// cant put it in the same stroke, have to split
					if ( !WriteCompressedStroke( phFile, pCompressedPoints, zOrder,
												pDeltas, nCompressedPoints, bStrokeFlags,
												startX, startY, bColorIndex, bPrevColorIndex, bWidth, bPrevWidth, b3bit ) )
					{
                        goto err;
					}
					if ( savePressure )
						WritePressures(phFile, pPoints, k - nCompressedPoints, k );
					// new stroke starting from this point
					nCompressedStrokes ++;
					
					if ( deltaX < 128 && deltaX > -128 && deltaY < 128 && deltaY > -128 )
					{
						// for the next sub-stroke store delta rather than starting point
						bStrokeFlags |= CONNECT_DELTA_FLAG;
						startX = deltaX;
						startY = deltaY;
					}
					else
						bStrokeFlags &= ~CONNECT_DELTA_FLAG; // have to clean this flag
					
					k --;
					bStrokeFlags |= CONNECT_TO_PREV_FLAG;
					bFirst = true;
					continue;
				}
				
				// Add delta type for this point
				prevX = (short)(int)pt.pt.x;
				prevY = (short)(int)pt.pt.y;
				deltaBits = CalcDeltaBits( deltaX, deltaY );
				if ( deltaX < 0 )
					deltaX = -deltaX;
				if ( deltaY < 0 )
					deltaY = -deltaY;
				
				if ( b3bit && (deltaX > MAX_DELTA_3BIT || deltaY > MAX_DELTA_3BIT) )
					b3bit = false;
				
				iDeltaByte = nCompressedPoints/4;
				iDeltaInByte = (nCompressedPoints%4)*2;
				pDeltas[iDeltaByte] |= deltaBits << iDeltaInByte;
				pCompressedPoints[nCompressedPoints] = deltaX | (deltaY<<4);
				nCompressedPoints ++;
				
				if ( nCompressedPoints >= MAX_STROKE_POINTS )
				{
					// have to split as cant have more than Ushort in a stroke
					if ( !WriteCompressedStroke( phFile, pCompressedPoints, zOrder,
												pDeltas, nCompressedPoints, bStrokeFlags, 
												startX, startY, bColorIndex, bPrevColorIndex, bWidth, bPrevWidth, b3bit ) )
					{
                        goto err;
					}
                    if ( savePressure )
						WritePressures(phFile, pPoints, k - nCompressedPoints, k );
					// new stroke from next point
					bSaved = true;
					nCompressedStrokes ++;
					bStrokeFlags |= CONNECT_TO_PREV_FLAG;
					bStrokeFlags &= ~CONNECT_DELTA_FLAG; // have to clean this flag
					bFirst = true;
					continue;
				}
			}
		}
		
		if ( !bSaved || (!bFirst && nCompressedPoints > 0) )
		{
			// write stroke
			if ( !WriteCompressedStroke( phFile, pCompressedPoints, zOrder,
										pDeltas, nCompressedPoints, bStrokeFlags, 
										startX, startY, bColorIndex, bPrevColorIndex, bWidth, bPrevWidth, b3bit ) )
			{
                goto err;
			}
			if ( savePressure )
				WritePressures(phFile, pPoints, k - nCompressedPoints, k );
            nCompressedStrokes++;
		}
	}
	
	
#ifdef TEXT_SUPPORT
	/* Write text edit objects */
	if ( ! WriteTextElements( phFile ) )
		goto err;
#endif // TEXT_SUPPORT
	
#ifdef IMAGE_SUPPORT
	/* Write images */
	if ( ! WriteImages( phFile ) )
		goto err;
#endif // TEXT_SUPPORT
	
	// Re-write number of strokes
	if ( nCompressedStrokes != nNonEmptyStrokes )
	{
		header.m_uStrokes = nCompressedStrokes;
		phFile.SetPos( 0L, SEEK_SET );
		if ( !phFile.Write( (BYTE *)&header, sizeof(header) ) )
			goto err;
	}
	
    nResult = TRFILE_OK;
	
err:
    if ( NULL != pCompressedPoints )
	    delete [] pCompressedPoints;
    if ( NULL != pDeltas )
	    delete [] pDeltas;
    return nResult;
}

#define MIN_PRESSURE_DELTA  3


void CInkData::WritePressures(CPHStream &phFile, PHSubStroke* pPoints, int nFrom, int nTo )
{
	int     k;
    int     len = nTo - nFrom;

	BOOL    constantPressure = true;
    BYTE    p = (BYTE)pPoints->GetAt(nFrom).GetPoint().pressure;
    if ( len > 2 )
    {
        for ( k = nFrom + 1; k <= nTo; k++ )
        {
            if ( abs( pPoints->GetAt(k).GetPoint().pressure - p ) > MIN_PRESSURE_DELTA )
            {
                constantPressure = false;
                break;
            }
        }
    }
	BYTE pressureHeader = PRESSURE_NORMAL_MASK;
	if( constantPressure )
	{
		pressureHeader = PRESSURE_CONST_MASK;
	}
    
    // first DWORD - header
	phFile.Write( &pressureHeader, sizeof(pressureHeader) );
    // second byte - original pressure

	if ( constantPressure )
    {
        phFile.Write( &p, sizeof(p) );
    }
    else
	{
        int     nPressureCnt = 0;
        WORD    curWord = 0;
        for ( k = nFrom + 1; k < nTo; k++ )
        {
            nPressureCnt++;
            if ( abs( pPoints->GetAt(k).GetPoint().pressure - p ) <= MIN_PRESSURE_DELTA )
            {
                if ( nPressureCnt < 0xFF )
                    continue;
            }
            //
            curWord = p | ((nPressureCnt&0xFF)<<8);
            phFile.Write( &curWord, sizeof(curWord) );

            p = pPoints->GetAt(k).GetPoint().pressure;
            nPressureCnt = 0;
        }
        // the end
        curWord = 0;
        phFile.Write( &curWord, sizeof(curWord) );
	}
}

static BOOL WriteCompressedStroke( CPHStream &phFile, BYTE * pCompressedPoints, SInt32 zOrder,
								  BYTE * pDeltas, UInt16 nCompressedPoints, BYTE bStrokeFlags, 
								  short startX, short startY, 
								  BYTE bColorIndex, BYTE &bPrevColorIndex, 
								  short bWidth, short &bPrevWidth, BOOL b3bit )
{
	BYTE b = 0;
	
	// we do not write color/width if stroke is part of previous one
	if ( (bStrokeFlags & CONNECT_TO_PREV_FLAG) == 0 )
	{
		if ( bColorIndex != bPrevColorIndex || bWidth != bPrevWidth )
		{
			bStrokeFlags &= ~COLORWIDTH_PREV_FLAG;
			bPrevWidth = bWidth;
			bPrevColorIndex = bColorIndex;
		}
		else
		{
			bStrokeFlags |= COLORWIDTH_PREV_FLAG;
		}
	}
	else
	{
		bStrokeFlags |= COLORWIDTH_PREV_FLAG;
	}
	
	bStrokeFlags &= ~(COMPR_4BIT_FLAG|COMPR_3BIT_FLAG);
	bStrokeFlags |= (b3bit ? COMPR_3BIT_FLAG : COMPR_4BIT_FLAG);
	
	if ( nCompressedPoints == 0 )
		bStrokeFlags |= NO_POINTS_FLAG;
	else if ( nCompressedPoints >= 256 )
		bStrokeFlags &= ~NUMPOINTS_1BYTE_FLAG;
	else
		bStrokeFlags |= NUMPOINTS_1BYTE_FLAG;
	
	// Write flags
    if ( !phFile.Write( &bStrokeFlags, sizeof(bStrokeFlags) ) )
        return false;
    if ( !phFile.Write( &zOrder, sizeof(SInt32) ) )
        return false;
	
	// Write color/width
	if ( (bStrokeFlags & COLORWIDTH_PREV_FLAG) == 0 )
	{
		// b = bWidth | (bColorIndex<<4);
		if ( !phFile.Write( &bWidth, sizeof(short) ) )
			return false;
		if ( !phFile.Write( &bColorIndex, sizeof(BYTE) ) )
			return false;
	}
	
	// Write number of points
	if ( nCompressedPoints >= 256 )
	{
		if ( !phFile.Write( (BYTE *)&nCompressedPoints, sizeof(UInt16) ) )
			return false;
	}
	else if ( nCompressedPoints > 0 )
	{
		b = (BYTE)nCompressedPoints;
		if ( !phFile.Write( &b, sizeof(b) ) )
			return false;
	}
	
	// Write start position
	if ( (bStrokeFlags & CONNECT_DELTA_FLAG) != 0 )
	{
		// one byte for each co-ordinate delta
		if ( !phFile.Write( (BYTE *)&startX, sizeof(BYTE) ) || !phFile.Write( (BYTE *)&startY, sizeof(BYTE) ) )
			return false;
	}
	else
	{
		if ( !phFile.Write( (BYTE *)&startX, sizeof(short) ) || !phFile.Write( (BYTE *)&startY, sizeof(short) ) )
			return false;
	}
	
	if ( nCompressedPoints == 0 )
		return true;
	
	// Write delta types
	if ( !b3bit )
	{
		if ( !phFile.Write( pDeltas, nCompressedPoints/4+1 ) )
			return false;
	}
	else
	{
		// mearge delta types back into data
		BYTE deltaX, deltaY, deltaBits;
		BYTE iDeltaByte, iDeltaInByte;
		
		for ( int i = 0; i < nCompressedPoints; i++ )
		{
			iDeltaByte = i/4;
			iDeltaInByte = (i%4)*2;
			deltaBits = (pDeltas[iDeltaByte] >> iDeltaInByte) & 0x03;
			deltaX = pCompressedPoints[i] & 0x0f;
			deltaY = (pCompressedPoints[i] >> 4) & 0x0f;
			if ( deltaBits & 0x01 )
				deltaX |= 0x08;
			if ( deltaBits & 0x02 )
				deltaY |= 0x08;
			pCompressedPoints[i] = deltaX | (deltaY<<4);
		}
	}
	
	// Write points
    if ( nCompressedPoints > 0 && !phFile.Write( pCompressedPoints, nCompressedPoints ) )
        return false;
	
	return true;
}

static BYTE CalcDeltaBits( short deltaX, short deltaY )
{
	BYTE b = 0;
	if ( deltaX < 0 )
		b |= 0x01;
	if ( deltaY < 0 )
		b |= 0x02;
	return b;
}

TRACE_FILE_STATUS CInkData::ReadPhatWareInk( FILE * pFile, DWORD dwFlags )
{
	if ( NULL != pFile )
    {
        CPHFileStream phStream( pFile );
        return ReadPhatWareInk( phStream );
    }
	return TRFILE_NAME_INVALID;
}

BOOL CInkData::ReadPressures(CPHStream &phFile, PHStroke * pStroke, int nPoints )
{
	int     k;
    BYTE    pressureHeader = 0;
    BYTE    p = DEFAULT_PRESSURE;
    int     size = pStroke->GetSubStroke1()->GetSize() - 1;
    
    if ( !phFile.Read( &pressureHeader, sizeof(pressureHeader) ) )
        return false;
    
    if ( pressureHeader == PRESSURE_CONST_MASK )
    {
        // same pressure everywhere
        if ( !phFile.Read( &p, sizeof(p) ) )
            return false;
        if ( nPoints < 1 )
        {
            if ( size >= 0 )
                pStroke->SetPressure( size, p );
        }
        else
        {
            for ( k = 0; k < nPoints; k++ )
            {
                if ( size-k >= 0 )
                    pStroke->SetPressure( size-k, p );
            }
        }
    }
    else
    {
        // variable pressure
        WORD    curWord = 0;
        if ( ! phFile.Read( &curWord, sizeof(curWord) ) )
            return false;
        int index = size - nPoints;
        while ( curWord != 0 )
        {
            p = curWord & 0xFF;
            int     nPressureCnt = (curWord >> 8) & 0xFF;
            for ( int i = 0; i < nPressureCnt; i++ )
            {
                if ( index <= size )
                {
                    pStroke->SetPressure( index, p );
                    index++;
                }
            }
            if ( ! phFile.Read( &curWord, sizeof(curWord) ) )
                return false;
        }
        // if there are still some pixels left, set last known pressure
        while ( index <= size )
        {
            pStroke->SetPressure( index, p );
            index++;
        }
    }
    return true;
}

TRACE_FILE_STATUS CInkData::ReadPhatWareInk( CPHStream &phStream, BOOL skipImages )
{
    // Clear the existing point array to get new trace
    FreeStrokes();
#ifdef TEXT_SUPPORT
	FreeText();
#endif // TEXT_SUPPORT
#ifdef IMAGE_SUPPORT
	FreeImages();
#endif // TEXT_SUPPORT
	
	PHCompTraceHeader header;
	COLORREF	colors[MAX_COLOR];
	short		lastX = 0, lastY = 0;
	
	// Read Header
    if ( !phStream.Read( &header, sizeof(header) ) )
        return TRFILE_BADFILE;
	if ( header.m_dwStamp != PWINK_FILE_STAMP )
		return TRFILE_BADFILE;
	
	// Read color table
    if ( !phStream.Read( colors, sizeof(COLORREF)*header.m_nColors ) )
        return TRFILE_BADFILE;
    
    // fix transperent colors
    for ( int k = 0; k < header.m_nColors; k++ )
    {
        if ( ((colors[k] >> 24) & 0xFF) == 0 )
            colors[k] |= (0xFF << 24);
    }

	BOOL	byteWidth = ((0!=strcmp(header.m_ucVersion, TRACE_FORMAT_VERSION_V100)) && (0!=strcmp(header.m_ucVersion, TRACE_FORMAT_VERSION_V101)));
	BOOL	supportPressure = (header.m_Options & FHF_INKPRESSURE) ? true : false;
	
	for ( int i = 0; i < header.m_uStrokes; i++ )
	{
		UInt16 nCompressedPoints = 0;
		if ( !ReadCompressedStroke( phStream, colors, header.m_nColors, lastX, lastY, nCompressedPoints, byteWidth, header.m_Options ) )
			return TRFILE_BADFILE;
		
		if( supportPressure)
        {
            PHStroke* pLastStroke = GetStroke(GetSize() - 1);
            if ( !ReadPressures( phStream, pLastStroke, nCompressedPoints ) )
                return TRFILE_BADFILE;
        }
	}
	
#ifdef TEXT_SUPPORT
	// Read Text Elements - only read if the m_Options is set
	if ( 0 != (FHF_TEXTOBJECTS & header.m_Options) )
	{
		if ( ! ReadTextElements( phStream ) )
			return TRFILE_BADFILE;
	}
#endif // TEXT_SUPPORT
#ifdef IMAGE_SUPPORT
 	if ( 0 != (FHF_IMAGEOBJECTS & header.m_Options) && (!skipImages) )
	{
		if ( ! ReadImages( phStream ) )
			return TRFILE_BADFILE;
	}
#endif // IMAGE_SUPPORT
	return TRFILE_OK;
}

BOOL CInkData::ReadCompressedStroke( CPHStream &phStream, COLORREF *colors, int nColors,
									short &lastX, short &lastY, UInt16& nCompressedPoints, BOOL byteForWidth, UCHAR options )
{
	BYTE        bStrokeFlags, b;
    BYTE        bColorIndex = 0;
    float       fWidth = 1.0;
	short       startX, startY;
	BYTE *      pCompressedPoints = NULL;
	BYTE *      pDeltas = NULL;		// 2 bits for deltas (used for 4-bit packing only)
	BOOL        b3bit = false;
	COLORREF    color = 0;
	PHStroke *  pStroke = NULL;
    SInt32      zorder = -1;
	
	// Read flags
    if ( !phStream.Read( &bStrokeFlags, sizeof(bStrokeFlags) ) )
		return false;
    
    if ( (options & FHF_INKZORDER) != 0 )
    {
        if ( !phStream.Read( &zorder, sizeof(SInt32) ) )
            return false;
    }
	
	b3bit = (bStrokeFlags & COMPR_3BIT_FLAG) != 0;
	// Read color/width
	if ( (bStrokeFlags & COLORWIDTH_PREV_FLAG) == 0 )
	{
		if ( byteForWidth )
		{
            if ( (options & FHF_FLOATWIDTH) != 0 )
            {
                short nWidth = 100;
                if ( !phStream.Read( &nWidth, sizeof(short) ) )
                    return false;
                fWidth = (float)nWidth/100.0f;
            }
            else
            {
                BYTE    bWidth = 1;
                if ( !phStream.Read( &bWidth, sizeof(BYTE) ) )
                    return false;
                fWidth = (float)bWidth;
            }
			if ( !phStream.Read( &bColorIndex, sizeof(BYTE) ) )
				return false;
		}
		else
		{
			if ( !phStream.Read( &b, sizeof(b) ) )
				return false;
		
			bColorIndex = (b & COLOR_MASK) >> 4;
			fWidth = (float)(b & WIDTH_MASK);
		}
		
		if ( bColorIndex >= nColors )
			bColorIndex = 0;
		color = colors[bColorIndex];
	}
	
	// Read number of points
	if ( bStrokeFlags & NO_POINTS_FLAG )
	{
		nCompressedPoints = 0;
	}
	else if ( bStrokeFlags & NUMPOINTS_1BYTE_FLAG )
	{
		if ( !phStream.Read( &b, sizeof(b) ) )
			return false;
		nCompressedPoints = (UInt16)b;
	}
	else
	{
		if ( !phStream.Read( &nCompressedPoints, sizeof(UInt16) ) )
			return false;
	}
	
	// Read start position
	if ( bStrokeFlags & CONNECT_DELTA_FLAG )
	{
		// 1 byte per start delta
		if ( !phStream.Read( &b, 1 ) )
			return false;
		
		startX = lastX + (short)((signed char)b);
		
		if ( !phStream.Read( &b, 1 ) )
			return false;
		
		startY = lastY + (short)((signed char)b);
	}
	else
	{
		if ( !phStream.Read( &startX, sizeof(short) ) || !phStream.Read( &startY, sizeof(short) ) )
			return false;
	}
	
	// Read delta types
	if ( !b3bit )
	{
		pDeltas = new BYTE[nCompressedPoints/4+1];
		if ( pDeltas == NULL )
			return false;
		
		if ( !phStream.Read( pDeltas, nCompressedPoints/4+1 ) )
		{
			delete [] pDeltas;
			return false;
		}
	}
	
	// Read points
	pCompressedPoints = new BYTE[nCompressedPoints+1];
	if ( pCompressedPoints == NULL )
	{
		if ( pDeltas )
			delete [] pDeltas;
        return false;
	}
	
    if ( nCompressedPoints > 0 && !phStream.Read( pCompressedPoints, nCompressedPoints ) )
	{
		if ( pDeltas )
			delete [] pDeltas;
		delete [] pCompressedPoints;
        return false;
	}
	
	// Uncompress
	int iStrokes = GetSize();
	if ( bStrokeFlags & CONNECT_TO_PREV_FLAG )
	{
		pStroke = GetStroke(iStrokes-1);
	}
	else
	{
		if ( (bStrokeFlags & COLORWIDTH_PREV_FLAG) != 0 )
		{
			pStroke = GetStroke(iStrokes-1);
			if ( pStroke )
			{
				fWidth = pStroke->GetWidth();
				color = pStroke->GetColor();
			}
		}
		
        pStroke = new PHStroke( fWidth, color );
		if ( pStroke )
        {
            if ( (bStrokeFlags & NOT_REC_FLAG) != 0 )
                pStroke->SetRecognizable( false );
			
			SetAtGrow( iStrokes, pStroke );
            pStroke->SetZOrder( (int)zorder );
        }
	}
	
	if ( pStroke )
	{
		int iDeltaByte, iDeltaInByte;
		BYTE deltaBits;
		short deltaX, deltaY;
		POINTS pt = { static_cast<float>(startX), static_cast<float>(startY) };
		
		lastX = (short)pt.x;
		lastY = (short)pt.y;
        {
            POINTS   ptt = { pt.x/4.0f, pt.y/4.0f };
		    pStroke->AddPoints( &ptt, 1 );
        }
		
		for ( int i = 0; i < nCompressedPoints; i++ )
		{
			deltaX = pCompressedPoints[i] & 0x0f;
			deltaY = (pCompressedPoints[i] >> 4) & 0x0f;
			
			if ( !b3bit )
			{
				iDeltaByte = i/4;
				iDeltaInByte = (i%4)*2;
				deltaBits = (pDeltas[iDeltaByte] >> iDeltaInByte) & 0x03;
				
				if ( deltaBits & 0x01 )
					deltaX = -deltaX;
				if ( deltaBits & 0x02 )
					deltaY = -deltaY;
			}
			else
			{
				if ( deltaX & 0x08 )
					deltaX = -(deltaX & 0x07);
				else
					deltaX = (deltaX & 0x07);
				
				if ( deltaY & 0x08 )
					deltaY = -(deltaY & 0x07);
				else
					deltaY = (deltaY & 0x07);
			}
			
			pt.x += deltaX;
			pt.y += deltaY;
			
			lastX = (short)pt.x;
			lastY = (short)pt.y;
            {
                POINTS   ptt = { pt.x/4.0f, pt.y/4.0f };
		        pStroke->AddPoints( &ptt, 1 );
            }
		}
	}
	
	delete [] pCompressedPoints;
	if ( pDeltas )
		delete [] pDeltas;
	return true;
}


void CInkData::SetUndoLevels( int nLevel )
{
    m_Undo.SetUndoLevel(nLevel);
    m_Redo.SetUndoLevel(nLevel);
}

BOOL CInkData::Undo()
{
    if ( ! m_bUndoEnabled )
        return false;
    if ( m_Undo.DoUndo( this, &m_Redo ) )
    {
        m_bModified = true;
        return true;
    }
    return false;
}

BOOL CInkData::Redo()
{
    if ( ! m_bUndoEnabled )
        return false;
    if ( m_Redo.DoUndo( this, &m_Undo ) )
    {
        m_bModified = true;
        return true;
    }
    return false;
}

BOOL CInkData::RecordUndo( int iType )
{
    if ( !m_bUndoEnabled )
    {
        m_bRecordingUndo = false;
    }
    else if ( iType == UNDO_TEXT_DELETED || iType == UNDO_TEXT_ADDED ||
        iType == UNDO_TEXT_CHANGED || iType == UNDO_TEXT_MOVED ||
		iType == UNDO_IMAGE_ADDED || iType == UNDO_IMAGE_DELETED ||
		iType == UNDO_IMAGE_MOVED)
    {
        m_bRecordingUndo = true;
    }
    else
    {
	    m_bRecordingUndo = m_Undo.AddItem( iType );
    }
    return m_bRecordingUndo;
}

void CInkData::StopRecordingUndo()
{
    m_bRecordingUndo = false;
    m_Undo.FreeUnused();
    m_Redo.FreeAll();
}

void CInkData::RemoveLastUndo()
{
    m_Undo.RemoveLastUndo();
}

void CInkData::FreeUndo()
{
    m_Undo.FreeAll();
    m_Redo.FreeAll();
}

// sort fuctions, used by qsort

int  CompareStrokesG( const void *arg1, const void *arg2 )
{
    // vertical sort by gravity 
    const LPSTRRECT pStr1 = (const LPSTRRECT)arg1;
    const LPSTRRECT pStr2 = (const LPSTRRECT)arg2;
	
    if ( pStr1->yGrav < pStr2->yGrav )
        return -1;
    else if ( pStr1->yGrav > pStr2->yGrav )
        return 1;
    return 0;
}

int  CompareStrokesL( const void *arg1, const void *arg2 )
{
    // line sort by line number
    const LPSTRRECT pStr1 = (const LPSTRRECT)arg1;
    const LPSTRRECT pStr2 = (const LPSTRRECT)arg2;
	
    if ( pStr1->line < pStr2->line )
        return -1;
    else if ( pStr1->line > pStr2->line )
        return 1;
    return 0;
}

int  CompareStrokesH( const void *arg1, const void *arg2 )
{
    // horizontal sort by position
    const LPSTRRECT pStr1 = (const LPSTRRECT)arg1;
    const LPSTRRECT pStr2 = (const LPSTRRECT)arg2;
	
	CGFloat cx1 = pStr1->rect.right - pStr1->rect.left;
	CGFloat cx2 = pStr2->rect.right - pStr2->rect.left;
    if ( ABS( pStr2->rect.left - pStr1->rect.left ) < MIN_V &&
		ABS( pStr2->rect.right - pStr1->rect.right) < MIN_V )
    {
        return 0;
    }
    else if ( pStr2->rect.left < pStr1->rect.left &&
			 pStr2->rect.right < pStr1->rect.right ) 
    {
        return 1;
    }
    else if ( pStr1->rect.left == pStr2->rect.left && 
			 pStr2->rect.right + cx2/8 < pStr1->rect.right - cx1/8 )
    {
        return 1;
    }
    return -1;
}

int	CInkData::GetImageObjectCnt() const
{
	return m_arrImages.GetSize();
}

LPSTRRECT CInkData::GetRecoStrokes( int nFirstStroke, int * pnStrokesCnt, BOOL bSort, BOOL bSelectedOnly )
{
    // sort strokes left-to-right, top to bottom single row
    int     nStrokeCnt = StrokesTotal();
	if ( nStrokeCnt < 1 )
		return NULL;
	if ( *pnStrokesCnt > 0 )
		nStrokeCnt = min( *pnStrokesCnt, nStrokeCnt );
	
	int		nMemSize = (nStrokeCnt + 1) * sizeof( STRRECT );
    LPSTRRECT   pStrokes = (LPSTRRECT)malloc( nMemSize );
	CGFloat		hMax = MIN_H;
    if ( NULL == pStrokes )
        return NULL;
	memset( (void *)pStrokes, 0, nMemSize );
	
    // 1. calc rectangles and vertial gravity centers for all recognizable strokes
	register int j, k, i, i0, line;
	CGFloat h, hAve = 0;
	for ( j = 0, i = nFirstStroke, k = 0; i < nStrokeCnt; i++ )
	{
        PHStroke * pStroke = GetStroke(i);
        if ( (NULL != pStroke && pStroke->IsRecognizable()) && ((!bSelectedOnly) || pStroke->IsSelected()) )
        {
			pStrokes[j].num = i;
			CalcStrokeRect( pStroke, &(pStrokes[j].rect), false, &(pStrokes[j].yGrav) );
			h = ABS(pStrokes[j].rect.bottom - pStrokes[j].rect.top);
			if ( h > MIN_H )
			{
				hAve += h;
				k++;
				hMax = max( h, hMax );
			}
            j++;
        }
	}
    nStrokeCnt = j;
	
	if ( ! bSort || (nStrokeCnt < 4 || k < 2) )
	{
		*pnStrokesCnt = nStrokeCnt;
		return pStrokes;
	}
	hAve /= (float)k;
	
    // 2. sort stroke array vertically (by gravity center)
    qsort( (void *)pStrokes, nStrokeCnt, sizeof( STRRECT ), CompareStrokesG );
	CGFloat gAve = 0, gMax = 0, gMin = 32000;
    for ( i = 1; i < nStrokeCnt; i++ )
	{
		h = pStrokes[i].yGrav - pStrokes[i-1].yGrav;
		h = (h < 0) ? (-h) : h;
		gMax = max( gMax, h );
		gMin = min( gMin, h );
		gAve += h;
	}
	gAve = gAve/nStrokeCnt;
	gAve = (gAve + ((gMax-gMin)/2))/2;
	
    // 3. Calculate line rect, but ignore small rects 
    CGFloat     cy0 = 0, dy = 1;
    int			nLineCnt = 0;
    CGFloat     grav;
    SIZE		sz, vz;
    PHArray <SIZE> arrSize;
	
	// skip narrow strokes in the beginning.
    for ( i = 0; cy0 <= hAve && i < nStrokeCnt; i++ )
		cy0 = (pStrokes[i].rect.bottom - pStrokes[i].rect.top);
    if ( i >= (nStrokeCnt-1) )
    {
        // all rectangles are too small -- nothing to sort
		*pnStrokesCnt = nStrokeCnt;
		return pStrokes;
    }
	
    line = 0;
    pStrokes[i].line = (1 + line);
    vz.cx = pStrokes[i].rect.top;
    vz.cy = pStrokes[i].rect.bottom;
    grav = pStrokes[i].yGrav;
	
	i++;
	int		nStrokesPerLine = 1;
    for ( ; i < nStrokeCnt; i++ )
    {
        sz.cx = (pStrokes[i].rect.right - pStrokes[i].rect.left);
		sz.cy = (pStrokes[i].rect.bottom - pStrokes[i].rect.top);
        if ( sz.cy > hAve/3 )		// ignore very narrow strokes
        {
			dy = pStrokes[i].yGrav - (grav/nStrokesPerLine);
			dy = (dy < 0) ? (-dy) : dy;
            if ( dy <= gAve )
            {
                // same line
                pStrokes[i].line = (1 + line);
                if ( pStrokes[i].rect.top < vz.cx )
                    vz.cx = pStrokes[i].rect.top;
                if ( pStrokes[i].rect.bottom > vz.cy )
                    vz.cy = pStrokes[i].rect.bottom;
				// adjust gravity center
                grav += pStrokes[i].yGrav;
				nStrokesPerLine++;
            }
            else 
            {
                // new line
                arrSize.Add( vz );
                line++;
				nStrokesPerLine = 1;
                pStrokes[i].line = (1 + line);
                vz.cx = pStrokes[i].rect.top;
                vz.cy = pStrokes[i].rect.bottom;
                grav = pStrokes[i].yGrav;
            }               
        }
    }
    arrSize.Add( vz );
    // line++;
	
    // 4. If there are any unassigned strokes, assign them to lines now
    for ( i = 0; i < nStrokeCnt; i++ )
    {
        if ( pStrokes[i].line < 1 )
        {
            for ( j = arrSize.GetSize()-1; j >= 0; j-- )
            {
                sz = arrSize.GetAt(j);
                if ( pStrokes[i].rect.top >= sz.cx )
                {
                    pStrokes[i].line = (j + 1);
                    break;
                }
            }
        }
        if ( pStrokes[i].line < 1 )
        {
            if ( pStrokes[i].rect.bottom > arrSize.GetAt( arrSize.GetSize()-1 ).cy )
                pStrokes[i].line = arrSize.GetSize();
            else if ( pStrokes[i].rect.top < arrSize.GetAt(0).cx )
                pStrokes[i].line = 1;
        }
    }
	
    // 5. sort strokes by line number
    qsort( (void *)pStrokes, nStrokeCnt, sizeof( STRRECT ), CompareStrokesL );
	
    // 6. sort strokes in each line left to right
    i0 = 0;
    line = pStrokes[i0].line;
    nLineCnt = 1;
    for ( i = 1; i < nStrokeCnt; i++ )
    {
        if ( pStrokes[i].line == line )
        {
            nLineCnt++;
        }
        else
        {
            // sort strokes inside the line
            qsort( (void *)&pStrokes[i0], nLineCnt, sizeof( STRRECT ), CompareStrokesH );
            line = pStrokes[i].line;
            nLineCnt = 1;
            i0 = i;
        }
    }
    if ( nLineCnt > 1 )
        qsort( (void *)&pStrokes[i0], nLineCnt, sizeof( STRRECT ), CompareStrokesH );
    *pnStrokesCnt = nStrokeCnt;
	return pStrokes;
}

#ifdef TEXT_SUPPORT

void CInkData::FreeText( void )
{
	m_arrTextObjects.Free();
}

CTextObject * CInkData::GetTextObject( int nIndex )
{
	if ( nIndex < 0 || nIndex >= GetTextObjectCnt() )
		return NULL;
	return m_arrTextObjects.GetAt( nIndex );
}

CTextObject * CInkData::EnumTextObjects( int & nPos )
{
	if ( nPos <= 0 )
		nPos = 0;
	if ( nPos >= GetTextObjectCnt() )
		return NULL;
	return m_arrTextObjects.GetAt( nPos++ );
}

CTextObject * CInkData::AddTextObject( int x, int y, int w, int h, DWORD alignment, LPLOGFONTW pFont, const UNCHAR * pszText, int nTextIndex )
{
	CTextObject *	pto = new CTextObject( x, y, w, h, pFont, pszText );
	if ( NULL != pto )
	{
		pto->m_attrib.dwJustification = alignment;
        int iIndex = m_arrTextObjects.GetSize();
		if ( m_arrTextObjects.GetSize() > nTextIndex && nTextIndex >= 0 )
		{
			CTextObject *	p = m_arrTextObjects.GetAt( nTextIndex );
			m_arrTextObjects.SetAt( nTextIndex, pto );
			if ( m_bRecordingUndo && m_bUndoEnabled )
			{
				// appending to the existing undo data
				m_Undo.AddTextObject( UNDO_TEXT_CHANGED, p, nTextIndex, true );
			}
			if ( p ) delete p;
		}
		else if ( m_arrTextObjects.Add( pto ) < 0 )
		{
			delete pto;
			pto = NULL;
		}
        else if ( m_bRecordingUndo && m_bUndoEnabled )
        {
            // appending to the existing undo data
            m_Undo.AddTextObject( UNDO_TEXT_ADDED, NULL, iIndex, false );
        }
	}
	return pto;
}

CTextObject * CInkData::ReplaceTextObject( int nIndex, CTextObject * pTextObject )
{
	CTextObject * pto = new CTextObject( pTextObject );
	
	if ( NULL == pto )
		return NULL;
	
	if ( nIndex < 0 || nIndex >= m_arrTextObjects.GetSize() )
	{
		m_arrTextObjects.Add( pto );
	}
	else
	{
		CTextObject * p = m_arrTextObjects.GetAt(nIndex);
		if ( p ) delete p;
		m_arrTextObjects.SetAt( nIndex, pto );
	}
	m_bModified = true;
	return pto;
}

CTextObject * CInkData::InsertTextObject( int nIndex, CTextObject * pTextObject )
{
	CTextObject * pto = new CTextObject( pTextObject->m_attrib.pt.x, 
										pTextObject->m_attrib.pt.y,
										pTextObject->m_attrib.iWidth,
										pTextObject->m_attrib.iHeight,
										&pTextObject->m_attrib.lf,
										pTextObject->GetText() );
	if ( NULL == pto )
		return NULL;
	memcpy( &pto->m_attrib, &pTextObject->m_attrib, sizeof( pTextObject->m_attrib ) );
	if ( nIndex < 0 || nIndex >= m_arrTextObjects.GetSize() )
	{
		m_arrTextObjects.Add( pto );
		m_bModified = true;
	}
	else
	{
		m_arrTextObjects.InsertAt( nIndex, pto );
		m_bModified = true;
	}
	return pto;
}

CTextObject * CInkData::GetTextObject( POINTS point )	
{
	CTextObject *	pto;
	int		count = m_arrTextObjects.GetSize();
	
	for ( int i = 0; i < count; i++ )
	{
		pto = m_arrTextObjects.GetAt(i);
		if ( pto &&
			(point.x > pto->m_attrib.pt.x && point.x < pto->m_attrib.pt.x + pto->m_attrib.iWidth &&
			 point.y > pto->m_attrib.pt.y && point.y < pto->m_attrib.pt.y + pto->m_attrib.iHeight) )
		{
            pto->m_iIndex = i;
			return pto;
		}
	}
	return NULL;
}

BOOL CInkData::DeleteTextObject( CTextObject * pto )
{
	int		count = m_arrTextObjects.GetSize();
	
	for ( int i = 0; i < count; i++ )
	{
		if ( pto && pto == m_arrTextObjects.GetAt(i) )
		{
			delete pto;
			m_arrTextObjects.RemoveAt(i);
			m_bModified = true;
			return true;
		}
	}
	return false;
}

int CInkData::GetTextObjectIndex( CTextObject * pto )
{
	int		count = m_arrTextObjects.GetSize();
	for ( int i = 0; i < count; i++ )
	{
		if ( pto == m_arrTextObjects.GetAt(i) )
			return i;
	}
	return (-1);
}

BOOL CInkData::DeleteTextObject( int nIndex )
{
	if ( nIndex >= 0 && nIndex < m_arrTextObjects.GetSize() )
	{
		CTextObject * pto = m_arrTextObjects.GetAt(nIndex);
		if ( pto )
			delete pto;
		m_arrTextObjects.RemoveAt(nIndex);
		m_bModified = true;
		return true;
	}
	return false;
}

BOOL CInkData::SetTextUserData( int nIndex, void * userData )
{
	if ( nIndex >= 0 && nIndex < m_arrTextObjects.GetSize() )
	{
		CTextObject * pto = m_arrTextObjects.GetAt(nIndex);
		if ( pto )
		{
			pto->SetUserData( userData );
			return true;
		}
	}
	return false;
}

void *	CInkData::GetTextUserData( int nIndex )
{
	void * userData = NULL;
	if ( nIndex >= 0 && nIndex < m_arrTextObjects.GetSize() )
	{
		CTextObject * pto = m_arrTextObjects.GetAt(nIndex);
		if ( pto )
		{
			userData = pto->GetUserData();
		}
	}
	return userData;
}

BOOL CInkData::SetTextFrame( int nTextIndex, int x, int y, int w, int h )
{
	if ( m_arrTextObjects.GetSize() > nTextIndex && nTextIndex >= 0 )
	{
		CTextObject *	p = m_arrTextObjects.GetAt( nTextIndex );
		p->m_attrib.pt.x = x;
		p->m_attrib.pt.y = y;
		p->m_attrib.iWidth = w;
		p->m_attrib.iHeight = h;
		return true;
	}
	return false;	
}

CTextObject *CInkData::RemoveTextObject( int nIndex )
{
    CTextObject * pto = NULL;
	if ( nIndex >= 0 && nIndex < m_arrTextObjects.GetSize() )
	{
		pto = m_arrTextObjects.GetAt(nIndex);
		if ( pto )
        {
		    m_arrTextObjects.RemoveAt(nIndex);
		    m_bModified = true;
        }
	}
	return pto;
}

BOOL CInkData::WriteTextElements(CPHStream &phFile)	
{
	int	 iCountSaveElements = m_arrTextObjects.GetSize();
	register int i;
	
	//1. Record number of text edit objects
	if ( !phFile.Write( (BYTE *)&iCountSaveElements, sizeof(iCountSaveElements) ) )
		return false;
	
	//2. Write all of the text editor objects
	CTextObject * pto = NULL;
	for ( i = 0; i < iCountSaveElements; i++)	
	{
		pto = m_arrTextObjects.GetAt(i);
		if ( NULL == pto )
			return false;
		if ( ! pto->Write( phFile ) )
			return false;
	}
	return true;
}

BOOL CInkData::ReadTextElements( CPHStream &phStream )	
{
	//1. read number of text objects
	int  	iCountLoadElements = 0;
	register int i;
	
	if ( !phStream.Read( &iCountLoadElements, sizeof(iCountLoadElements) ) )
		return false;
	
	// 2. Read text elements
	CTextObject * pto = NULL;
	for ( i = 0; i < iCountLoadElements; i++ )	
	{
		pto = AddTextObject( 0, 0, 0, 0 );
		if ( NULL == pto )
			return false;
		if ( ! pto->Read( phStream ) )
			return false;
	}
	return true;
}

BOOL CInkData::RecordUndo( int iType, CTextObject * pTextObject, int nIndex, BOOL bClone )
{
    if ( !m_bUndoEnabled )
    {
        m_bRecordingUndo = false;
        return m_bRecordingUndo;
    }
    if ( nIndex < 0 )
	{
		int		count = m_arrTextObjects.GetSize();
		for ( int i = 0; i < count; i++ )
		{
			if ( pTextObject == m_arrTextObjects.GetAt(i) )
			{
				nIndex = i;
				break;
			}
		}
	}
	m_bRecordingUndo = m_Undo.AddItem( iType, (iType==UNDO_TEXT_ADDED) ? NULL : pTextObject, nIndex, bClone );
    return m_bRecordingUndo;
}

void CInkData::StopRecordingUndo( int iType )
{
	if ( iType != 0 )
		m_Undo.UpdateLastType( iType );
    m_bRecordingUndo = false;
    m_Undo.FreeUnused();
    m_Redo.FreeAll();
}

void CInkData::FinishMoveTextUndo( CTextObject * pTextObject )
{
    m_Undo.FinishMoveTextUndo( pTextObject );
    m_bRecordingUndo = false;
    m_Redo.FreeAll();
}

BOOL CInkData::DeleteAllTextObjects( BOOL bRecordUndo /*= true*/ )
{
    int	count = m_arrTextObjects.GetSize();
    if ( count > 0 )
    {
        CTextObject * pto;
		
	    for ( int i = count-1; i >= 0; i-- )
	    {
            pto = m_arrTextObjects.GetAt(i);
		    if ( pto )
		    {
                if ( bRecordUndo && m_bUndoEnabled )
                {
                    if ( i == count-1 )
                        m_Undo.AddItem( UNDO_TEXT_DELETED, pto, i, false );
                    else
                        m_Undo.AddTextObject( UNDO_TEXT_DELETED, pto, i, false );
					
                    // do not delete - object just moved
                }
                else
                {
			        delete pto;
                }
            }
			m_arrTextObjects.RemoveAt(i);
	    }
        SetModified( true );
        return true;
    }
    return false;
}

BOOL CInkData::CanUndoForCurrentObject( CTextObject *pto )
{
    if ( ! m_bUndoEnabled )
        return false;
    return m_Undo.CanUndoForCurrentObject( pto );
}

#endif // TEXT_SUPPORT

#ifdef IMAGE_SUPPORT

BOOL CInkData::DeleteImageObject( CImageObject * pto )
{
	int		count = m_arrImages.GetSize();
	
	for ( int i = 0; i < count; i++ )
	{
		if ( pto && pto == m_arrImages.GetAt(i) )
		{
			delete pto;
			m_arrImages.RemoveAt(i);
			m_bModified = true;
			return true;
		}
	}
	return false;
}

CImageObject *CInkData::RemoveImageObject( int nIndex )
{
    CImageObject * pto = NULL;
	if ( nIndex >= 0 && nIndex < m_arrImages.GetSize() )
	{
		pto = m_arrImages.GetAt(nIndex);
		if ( pto )
        {
		    m_arrImages.RemoveAt(nIndex);
		    m_bModified = true;
        }
	}
	return pto;
}

CImageObject * CInkData::GetImageObject( POINTS point )	
{
	CImageObject *	pto;
	int		count = m_arrImages.GetSize();
	
	// go from topmost...
	for ( int i = count - 1; i >= 0; i-- )
	{
		pto = m_arrImages.GetAt(i);
		if ( pto &&
			(point.x > pto->m_attrib.pt.x && point.x < pto->m_attrib.pt.x + pto->m_attrib.iWidth &&
			 point.y > pto->m_attrib.pt.y && point.y < pto->m_attrib.pt.y + pto->m_attrib.iHeight) )
		{
            pto->m_iIndex = i;
			return pto;
		}
	}
	return NULL;
}

void CInkData::FreeImages( void )
{
	m_arrImages.Free();
}

CImageObject * CInkData::AddImageObject( int x, int y, int w, int h, const char * pszFilePath )
{
	CImageObject *	pio = new CImageObject( x, y, pszFilePath, w, h);
	if ( pszFilePath == NULL )
	{
		if ( m_arrImages.Add( pio ) < 0 )
		{
			delete pio;
			pio = NULL;
		}
		return pio;
	}
	if ( NULL != pio )
	{
		int iIndex = m_arrImages.GetSize();
        if ( m_bUndoEnabled )
            m_bRecordingUndo = m_Undo.AddItem( UNDO_IMAGE_ADDED, (CImageObject*)NULL, iIndex, false );
        else
            m_bRecordingUndo = false;
		if ( m_arrImages.Add( pio ) < 0 )
		{
			delete pio;
			pio = NULL;
		}
        else if ( m_bRecordingUndo && m_bUndoEnabled )
        {
            //appending to the existing undo data
			m_Undo.AddImageObject( UNDO_IMAGE_ADDED, NULL, iIndex, false );
        }
	}
	return pio;
}

BOOL CInkData::SetImageFrame( int nImageIndex, int x, int y, int w, int h )
{
	if ( m_arrImages.GetSize() > nImageIndex && nImageIndex >= 0 )
	{
		CImageObject *	p = m_arrImages.GetAt( nImageIndex );
		p->m_attrib.pt.x = x;
		p->m_attrib.pt.y = y;
		p->m_attrib.iWidth = w;
		p->m_attrib.iHeight = h;
		return true;
	}
	return false;
}

int CInkData::AddImageObject( int x, int y, int w, int h, UInt32 flags, int iZOrder, const void * pBytes, UInt32 cbSize, void * userData, int nImageIndex )
{
	CImageObject *	pio = new CImageObject( x, y, w, h, pBytes, cbSize, userData );
	int		res = -1;
	if ( NULL != pio )
	{
		pio->m_attrib.iZOrder = iZOrder;
		pio->m_attrib.dwRotation = (double)flags;
		if ( m_arrImages.GetSize() > nImageIndex && nImageIndex >= 0 )
		{
			CImageObject *	p = m_arrImages.GetAt( nImageIndex );
			if ( p ) delete p;
			m_arrImages.SetAt( nImageIndex, pio );
			res = nImageIndex;
		}
		else if ( (res = m_arrImages.Add( pio )) < 0 )
		{
			delete pio;
			pio = NULL;
		}
	}
	return res;
}

BOOL CInkData::SetImageUserData( int nImageIndex, void * userData )
{
	CImageObject * image = GetImageObject( nImageIndex );
	if ( image == NULL )
		return false;
	image->m_userData = userData;
	return true;
}


//v3Format does not support image rotation
BOOL CInkData::ReadImages(CPHStream &phFile )	
{
	m_arrImages.Free();
	register UInt32 i;
	UInt32 iCountLoadElements;	
	if ( !phFile.Read( (BYTE *)&iCountLoadElements, sizeof(iCountLoadElements) ) )
		return false;
	// 2. Read text elements
	CImageObject * pio = NULL;
	for ( i = 0; i < iCountLoadElements; i++ )	
	{
		pio = AddImageObject( 0, 0, 0, 0, NULL );
		if ( NULL == pio )
			return false;
		if ( ! pio->Read( phFile ) )
			return false;
		// int iIndex = m_arrImages.GetSize();
		// appending to the existing undo data
	}
	return true;
}

BOOL CInkData::WriteImages(CPHStream &phFile)	
{
	int	iCountSaveElements = m_arrImages.GetSize();
	register int i;
	
	//1. Record number of image objects
	if ( !phFile.Write( (BYTE *)&iCountSaveElements, sizeof(iCountSaveElements) ) )
		return false;
	
	//2. Write images
	CImageObject * pio = NULL;
	for ( i = 0; i < iCountSaveElements; i++)	
	{
		pio = m_arrImages.GetAt(i);
		if ( NULL == pio )
			return false;
		if ( ! pio->Write( phFile ) )
			return false;
	}
	return true;
}

CImageObject * CInkData::GetImageObject( int nIndex )
{
	if ( nIndex < 0 || nIndex >= GetImageObjectCnt() )
		return NULL;
	return m_arrImages.GetAt( nIndex );
}

BOOL CInkData::DeleteImageObject( int nIndex )
{
	if ( nIndex >= 0 && nIndex < m_arrImages.GetSize() )
	{
		CImageObject * pto = m_arrImages.GetAt(nIndex);
		if ( pto )
			delete pto;
		m_arrImages.RemoveAt(nIndex);
		m_bModified = true;
		return true;
	}
	return false;
}

BOOL CInkData::DeleteAllImageObjects()
{
	int cnt = 0;
	for ( int nIndex = 0; nIndex < m_arrImages.GetSize(); nIndex++ )
	{
		CImageObject * pto = m_arrImages.GetAt(nIndex);
		if ( pto )
		{
			delete pto;
			cnt++;
		}
	}
	if ( cnt > 0 )
	{
		m_arrImages.RemoveAll();
		m_bModified = true;
		return true;
	}
	return false;
}

CImageObject * CInkData::ReplaceImageObject( int nIndex, CImageObject * pImageObject )
{
	CImageObject * pto = new CImageObject( pImageObject );
	
	if ( NULL == pto )
		return NULL;
	
	if ( nIndex < 0 || nIndex >= m_arrImages.GetSize() )
	{
		m_arrImages.Add( pto );
	}
	else
	{
		CImageObject * p = m_arrImages.GetAt(nIndex);
		if ( p )
            delete p;
		m_arrImages.SetAt( nIndex, pto );
	}
	m_bModified = true;
	return pto;
}

CImageObject * CInkData::InsertImageObject( int nIndex, CImageObject * pImageObject )
{
	CImageObject * pto = new CImageObject( pImageObject );
	if ( NULL == pto )
		return NULL;
	memcpy( &pto->m_attrib, &pImageObject->m_attrib, sizeof( pImageObject->m_attrib ) );
	if ( nIndex < 0 || nIndex >= m_arrImages.GetSize() )
	{
		m_arrImages.Add( pto );
		m_bModified = true;
	}
	else
	{
		m_arrImages.InsertAt( nIndex, pto );
		m_bModified = true;
	}
	return pto;
}

int CInkData::GetImageObjectIndex( CImageObject * pto )
{
	int		count = m_arrImages.GetSize();
	for ( int i = 0; i < count; i++ )
	{
		if ( pto == m_arrImages.GetAt(i) )
			return i;
	}
	return (-1);
}

BOOL CInkData::RecordUndo( int iType, CImageObject * pImageObject, int nIndex, BOOL bClone )
{
    if ( ! m_bUndoEnabled )
    {
        m_bRecordingUndo = false;
        return m_bRecordingUndo;
    }
    if ( nIndex < 0 )
	{
		int		count = m_arrImages.GetSize();
		for ( int i = 0; i < count; i++ )
		{
			if ( pImageObject == m_arrImages.GetAt(i) )
			{
				nIndex = i;
				break;
			}
		}
	}
	m_bRecordingUndo = m_Undo.AddItem( iType, (iType==UNDO_IMAGE_ADDED) ? NULL : pImageObject, nIndex, bClone );
    return m_bRecordingUndo;
}

#endif // IMAGE_SUPPORT

PHStroke *CInkData::FindStroke( PHStroke *pSample )
{
    int		nCount = StrokesTotal();
    PHSubStroke *pPoints;
    PHSubStroke *pSamplePoints = pSample->GetSubStroke();
	UInt32 	nPoints;
	int nSamplePoints = pSamplePoints ? pSamplePoints->GetSize() : 0;
    int	i, iSample;
    CGPoint	pt1, pt2;
    register PHStroke * pStr = NULL;
    BOOL	bDiff;
    CGFloat	deltaX, deltaY;
	CGFloat	prevX = 0.0, prevY = 0.0;
	
    for ( register int nStroke = 0; nStroke < nCount; nStroke++ )
    {
        pStr = GetStroke( nStroke );
        if ( pStr )
        {
            pPoints = pStr->GetSubStroke();
            nPoints = pPoints ? pPoints->GetSize() : 0;
            if ( pPoints )
            {
                bDiff = false;
                iSample = 0;
	            for( i = 0; !bDiff && i < (int)nPoints && iSample < (int)nSamplePoints; iSample++, i++ )
	            {
                    pt1 = pPoints->GetAt(i).GetPoint().pt;
                    pt2 = pSamplePoints->GetAt(iSample).GetPoint().pt;
                    // the point could be skipped
                    if ( iSample > 0 && (pt1.x != pt2.x || pt1.y != pt2.y) )
                    {
				        deltaX = (pt2.x - prevX);
				        deltaY = (pt2.y - prevY);
                        if ( (deltaX*deltaX + deltaY*deltaY) <= TOLERANCE )
                        {
                            i --;
                            continue;
                        }
                    }
					
                    if ( iSample < nSamplePoints )
                    {
				        prevX = pt2.x;
				        prevY = pt2.y;
                    }
                    else
                        bDiff = true;
					
		            if ( pt1.x != pt2.x || pt1.y != pt2.y )
                        bDiff = true;
	            }
				
                if ( (!bDiff) && i == (int)nPoints && iSample == (int)nSamplePoints )
                    return pStr;
            }
        }
    }
    return NULL;
}

void CInkData::SortStrokes()
{
    // sort strokes left-to-right, single row
    int   nStrokeCnt = StrokesTotal();
	if ( nStrokeCnt < 1 )
		return;
    STRRECT *	pStrokes = new STRRECT[nStrokeCnt+1];
	STRRECT	sRect;
	register int j, i;
    if ( NULL != pStrokes )
	{
		register float	cxi, cxj;
		for ( i = 0; i < (int)nStrokeCnt; i++ )
		{
			pStrokes[i].num = i;
			CalcStrokeRect( i, &(pStrokes[i].rect) );
		}
		// sort strokes
		for ( i = 0; i < nStrokeCnt; i++ )
		{
			for ( j = i+1; j < nStrokeCnt; j++ )
			{
				cxi = pStrokes[i].rect.right - pStrokes[i].rect.left;
				cxj = pStrokes[j].rect.right - pStrokes[j].rect.left;
				if ( pStrokes[j].rect.left + cxj/8 < pStrokes[i].rect.left - cxi/8 &&
					pStrokes[j].rect.right < pStrokes[i].rect.right )
				{
					memcpy( &sRect, &pStrokes[i], sizeof( STRRECT ) );
					memcpy( &pStrokes[i], &pStrokes[j], sizeof( STRRECT ) );
					memcpy( &pStrokes[j], &sRect, sizeof( STRRECT ) );
					PHStroke * si = GetStroke( i );
					PHStroke * sj = GetStroke( j );
					SetAt( i, sj );
					SetAt( j, si );					
				}
				else if ( pStrokes[j].rect.left == pStrokes[i].rect.left )
				{
					if ( pStrokes[j].rect.right + cxj/8 < pStrokes[i].rect.right - cxi/8 )
					{
						memcpy( &sRect, &pStrokes[i], sizeof( STRRECT ) );
						memcpy( &pStrokes[i], &pStrokes[j], sizeof( STRRECT ) );
						memcpy( &pStrokes[j], &sRect, sizeof( STRRECT ) );
						PHStroke * si = GetStroke( i );
						PHStroke * sj = GetStroke( j );
						SetAt( i, sj );
						SetAt( j, si );					
					}
				}
			}
		}
	}
	delete [] pStrokes;
}

void CInkData::CopyData( CInkData * pFrom )
{	
	FreeStrokes();
	
	UInt32 nCnt = pFrom->StrokesTotal();
	for ( UInt32 i = 0; i < nCnt; i++ )
	{
		PHStroke * pStroke = new PHStroke( pFrom->GetStroke( i ) );
		Add( pStroke );
	}
	m_bModified = false;	
}

