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
#include <math.h>

#include "ShapesRec.h"
#include "InkData.h"

#define MIN_GRIDSIZE		5
#define MAX_GRIDSIZE        15
#define MIN_STROKE_LEN      5
#define DIRECTION_SPAN      2       // or 3?

#define CELL_SIZE           10
#define N_SEGS				3
 
#define FOUR_ANGLE          4
#define TRI_ANGLE           3
#define TWO_ANGLE           2


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CShapesRec::CShapesRec()
{
    m_nLeft = m_nTop = m_nRight = m_nBottom = 0;
    m_nCellLeft = m_nCellTop = m_nCellRight = m_nCellBottom = 0;
    m_ptCenter.x = m_ptCenter.y = 0.0f;
    m_ptGravCenter.x = m_ptGravCenter.y = 0.0f;
    m_ShapeRect.left = m_ShapeRect.right = m_ShapeRect.top = m_ShapeRect.bottom = 0.0f;
    m_nGridSize = MIN_GRIDSIZE;
    m_pStroke = NULL;
    m_nStrokeCnt = 0;
    m_nDirSpan = DIRECTION_SPAN;
}

CShapesRec::~CShapesRec()
{
    if ( m_pStroke != NULL )
        delete [] m_pStroke;
}

////////////////////////////////////////////////////////////////////////////
//  int TraceToRec( int in_numpts, LPPOINT in_pt, LPPOINT out_pt ) 
////////////////////////////////////////////////////////////////////////////

int CShapesRec::TraceToRec( int in_numpts, LPPOINTS in_pt, LPPOINTS out_pt ) const
{
	float		xFltPrv, yFltPrv, xPrv, yPrv;
	int         m_nPointsRec = 1, NumFilteredPoints;
	float		xToRec, yToRec;
	register float x,y;

	register int i, j;

	x = in_pt[0].x;
	y = in_pt[0].y;

	xPrv = xFltPrv = out_pt[0].x = (x*2.0f);
	yPrv = yFltPrv = out_pt[0].y = (y*2.0f);

	for( i = 1; i < in_numpts; i++ )
	{
		x = in_pt[i].x;
		y = in_pt[i].y;
		xToRec = x*2.0f;
		yToRec = y*2.0f;
		if ( i == 1 )
			NumFilteredPoints=1;
		else
			NumFilteredPoints=N_SEGS;
		for ( j = 1; j <= NumFilteredPoints; j++ ) 
		{
			float xi = (xToRec - xPrv) * j / N_SEGS + xPrv;
			float yi = (yToRec - yPrv) * j / N_SEGS + yPrv;
			float xf = (xi - xFltPrv) / N_SEGS + xFltPrv;
			float yf = (yi - yFltPrv) / N_SEGS + yFltPrv;
			if ( xf != xFltPrv || yf != yFltPrv ) 
			{
				xFltPrv = out_pt[m_nPointsRec].x = xf; 
				yFltPrv = out_pt[m_nPointsRec++].y = yf; 
			}
		}
 		xPrv = xToRec;
		yPrv = yToRec;
	}
	x = in_pt[in_numpts-1].x;
	y = in_pt[in_numpts-1].y;
    out_pt[m_nPointsRec].x  = (x*2.0f);
	out_pt[m_nPointsRec++].y= (y*2.0f);

	for ( i = 0; i < m_nPointsRec; i++ )
	{
		out_pt[i].x /= 2;
		out_pt[i].y /= 2;
	}
	return m_nPointsRec;
}	/* end of TraceToRec */

int CShapesRec::FilterTrajectory( int innumpts, LPPOINTS instrkin, LPPOINTS * iostrkout ) const
{
	int		outbuflen = (innumpts * (N_SEGS + 2));
	int		retnumpts = 0;
	*iostrkout = new POINTS[outbuflen];
	if ( NULL == *iostrkout )
		return 0;
	retnumpts = TraceToRec(	innumpts, instrkin, *iostrkout );
	return retnumpts;
}

SHAPETYPE CShapesRec::RecognizeShape( CGTracePoint * pInStroke, LPPOINTS * ppOutStroke, UINT *pnStrokeCnt, SHAPETYPE inType )
{
	if ( NULL == pInStroke || ppOutStroke == NULL || pnStrokeCnt == NULL || *pnStrokeCnt < MIN_STROKE_LEN )
        return SHAPE_UNKNOWN;
	
	UINT	nStrokeCnt = *pnStrokeCnt;
	LPPOINTS	pTempStr = (LPPOINTS)malloc( sizeof( POINTS ) * (nStrokeCnt+1) );
	register int i;
	for ( i = 0; i < (int)nStrokeCnt; i++ )
	{
		pTempStr[i].x = pInStroke[i].pt.x;
		pTempStr[i].y = pInStroke[i].pt.y;
	}
	SHAPETYPE result = RecognizeShape( pTempStr, ppOutStroke, pnStrokeCnt, inType );
	free( (void *)pTempStr );
	return result;
}


SHAPETYPE CShapesRec::RecognizeShape( CGPoint * pInStroke, LPPOINTS * ppOutStroke, UINT *pnStrokeCnt, SHAPETYPE inType )
{
	if ( NULL == pInStroke || ppOutStroke == NULL || pnStrokeCnt == NULL || *pnStrokeCnt < MIN_STROKE_LEN )
        return SHAPE_UNKNOWN;
	
	UINT	nStrokeCnt = *pnStrokeCnt;
	LPPOINTS	pTempStr = (LPPOINTS)malloc( sizeof( POINTS ) * (nStrokeCnt+1) );
	register int i;
	for ( i = 0; i < (int)nStrokeCnt; i++ )
	{
		pTempStr[i].x = pInStroke[i].x;
		pTempStr[i].y = pInStroke[i].y;
	}
	SHAPETYPE result = RecognizeShape( pTempStr, ppOutStroke, pnStrokeCnt, inType );
	free( (void *)pTempStr );
	return result;
}

SHAPETYPE CShapesRec::RecognizeShape( LPPOINTS pInStroke, LPPOINTS * ppOutStroke, UINT *pnStrokeCnt, SHAPETYPE inType )
{
	if ( NULL == pInStroke || ppOutStroke == NULL || pnStrokeCnt == NULL || *pnStrokeCnt < MIN_STROKE_LEN )
        return SHAPE_UNKNOWN;
	UINT        nStrokeCnt = *pnStrokeCnt;
	LPPOINTS    pStroke = pInStroke;

    // check if the shape is connected
    UINT    nBegPoint = 0;
	BOOL	bConnected = IsConnectedShape( pStroke, nStrokeCnt, nBegPoint );

    // add missing pixels before adjusting to grid
    if ( ! CopyStrokePoints( pStroke, nBegPoint, nStrokeCnt ) )
        return SHAPE_UNKNOWN;       // unable to allocate memory

    // calulate stroke boundries
	if ( ! GetStrokeRect( bConnected ) )
		return SHAPE_UNKNOWN;		// unable to obtain a valid rectangle

	// adjust the stroke to grid
	if ( ! AdjustToGrid() )
		return SHAPE_UNKNOWN;		// unable to ajust to grid

    // delete unused stroke array
    if ( m_pStroke != NULL )
        delete [] m_pStroke;
    m_pStroke = NULL;
    m_nStrokeCnt = 0;

    SHAPETYPE nShapeType = SHAPE_UNKNOWN;
	
	if ( 0 != (inType & SHAPE_SCRATCH) )
	{
		nShapeType = CalcConnectedShape( ppOutStroke, pnStrokeCnt, SHAPE_SCRATCH );
		if ( nShapeType != SHAPE_UNKNOWN )
			return nShapeType;
	}
    if ( bConnected )
    {
        // recognize the connected shape: circle, rectangle, ellipse, or triangle 
        nShapeType = CalcConnectedShape( ppOutStroke, pnStrokeCnt, (SHAPETYPE)(inType & ~SHAPE_SCRATCH) );
    }
    else
    {
        // recognize not conencted shape (straight line or broken line)
        nShapeType = CalcStraightLine( ppOutStroke, pnStrokeCnt, (SHAPETYPE)(inType & ~SHAPE_SCRATCH) );
    }
	return nShapeType;
}

////////////////////////////////////////////////////////////////////////////
// BOOL CopyStrokePoints( LPPOINT pStroke, UINT nStrokeCnt )
//
// This function adds pixels between each 2 pixels in the stroke, if the 
// distance between these 2 pixels is bigger than grid size (m_nGridSize)
// this is done, so there will be no empty grid cells within the stroke.
// If there are empty grid cells within the stroke, the gravity center will
// not be calculated correctly which will increase the possibility of an error.
// during shape recognition.
//
////////////////////////////////////////////////////////////////////////////

BOOL CShapesRec::CopyStrokePoints( const LPPOINTS pStroke, UINT nBegPoint, UINT nStrokeCnt )
{
    if ( m_pStroke != NULL )
        delete [] m_pStroke;
	m_pStroke = NULL;

	// filter the stroke
	m_nStrokeCnt = FilterTrajectory( (int)(nStrokeCnt-nBegPoint), &pStroke[nBegPoint], &m_pStroke );
	if ( NULL == m_pStroke || m_nStrokeCnt < MIN_STROKE_LEN )
	{
		if ( NULL != m_pStroke )
			delete [] m_pStroke;
		m_pStroke = NULL;
		return false;
	}
	return true;
}

////////////////////////////////////////////////////////////////////////////
// BOOL GetStrokeRect( LPPOINT pStroke, UINT nStrokeCnt )
//
// this function calculates the size of the rectangle that contains a stroke
//
////////////////////////////////////////////////////////////////////////////

BOOL CShapesRec::GetStrokeRect( BOOL bConnected )
{
	POINTS			pt = m_pStroke[0];

	m_ShapeRect.left = m_ShapeRect.right = pt.x; 
	m_ShapeRect.top = m_ShapeRect.bottom = pt.y;
    m_nLeft = m_nTop = m_nRight = m_nBottom = 0;
	
	for ( int i = 1; i < (int)m_nStrokeCnt; i++ )
	{
		pt = m_pStroke[i];
		if ( pt.x < m_ShapeRect.left )
        {
			m_ShapeRect.left = pt.x;
            m_nLeft = i;        // index of the most left pixel
        }
		if ( pt.x > m_ShapeRect.right ) 
        {
			m_ShapeRect.right = pt.x;
            m_nRight = i;       // index of the most right pixel
        }
		if ( pt.y < m_ShapeRect.top )
        {
			m_ShapeRect.top = pt.y;
            m_nTop = i;         // index of the most top pixel
        }
		if ( pt.y > m_ShapeRect.bottom )
        {
			m_ShapeRect.bottom = pt.y;
            m_nBottom = i;      // index of the most bottom pixel
        }
	}

    // calculate center of the rectangle
    float width = fabsf( m_ShapeRect.right - m_ShapeRect.left );
    float height = fabsf( m_ShapeRect.bottom - m_ShapeRect.top );
    if ( width < MIN_STROKE_LEN && height < MIN_STROKE_LEN )
        return false;   // the object is too small

    // calculate best grid size
    float     gr1 = width/10.0f;
    float     gr2 = height/10.0f;

    if ( bConnected )
        m_nGridSize = (gr1 + gr2 + 2.0f)/4.0f;
    else
        m_nGridSize = max( gr1, gr2 )/2.0f;

    if ( m_nGridSize <= MIN_GRIDSIZE )
    {
        m_nDirSpan = 1;     // the default is 2
        m_nGridSize = MIN_GRIDSIZE;
    }
    else if ( m_nGridSize >= MAX_GRIDSIZE )
        m_nDirSpan = 3;
    else
        m_nDirSpan = DIRECTION_SPAN;

    m_ptCenter.x = m_ShapeRect.left + width/2.0f;
    m_ptCenter.y = m_ShapeRect.top + height/2.0f;

    InflateRect( &m_ShapeRect, (m_nGridSize)/2.0f, (m_nGridSize)/2.0f );

    // MYTRACE3( "**********>>>> Grid Size=%d, Center (x=%d, y=%d)\n", m_nGridSize, m_ptCenter.x, m_ptCenter.y );
	return true;
}

////////////////////////////////////////////////////////////////////////////
// BOOL IsConnectedShape( LPPOINT pStroke, UINT & nStrokeCnt )
//
// this function returns true if the last point of the stroke is within 
// the defined limit (MAX_DISTANCE) This fuction may resuce number of 
// pixels in the stroke.
//
////////////////////////////////////////////////////////////////////////////

BOOL CShapesRec::IsConnectedShape( LPPOINTS pStroke, UINT & nStrokeCnt, UINT & nBeg )
{
	int 	nLastPoint = (int)nStrokeCnt - 1;
    int     nMaxPts = min( MIN_STROKE_LEN * 2, nLastPoint/4 );
	if ( nLastPoint > nMaxPts )
	{
		POINTS	pt1 = pStroke[0];
		POINTS	pt2 = pStroke[nLastPoint];

		float   mdelta = fabsf((pt1.x + pt2.y) - (pt2.x + pt1.y)); 
		UINT	nEndPoint = nStrokeCnt;
        UINT    nBegPoint = nBeg;
        // nMaxPts <<= DIV;

        // calcualte minimum distance between first and last pixels,
        // if nessesary, exclude some pixels at the beginning and the end of
        // stroke.
		for ( register int j = 0; j < min( nLastPoint, nMaxPts ); j++ )
        {
		    pt1 = pStroke[j];
            for ( register int i = nLastPoint - 1; i >= max( 0, (nLastPoint - nMaxPts) ); i-- )
		    {
			    pt2 = pStroke[i];
			    // |(x1+y2)-(x2+y1)| = |x1-x2| + |y2-y1|
			    float	delta = fabsf((pt1.x + pt2.y) - (pt2.x + pt1.y));
			    if ( delta < mdelta ) 
			    {
				    mdelta = delta;
				    nEndPoint = i;
                    nBegPoint = j;
			    }
		    }
        }

		if ( mdelta < (float)nMaxPts && (int)nEndPoint > nMaxPts && nBegPoint < nEndPoint - nMaxPts )
		{
			// the stroke is connected; set the first and last pixels
            float dsx = 0.0f; // pStroke[nBegPoint].x - pStroke[nEndPoint].x;
            float dsy = 0.0f; // pStroke[nBegPoint].y - pStroke[nEndPoint].y;
            float x1 = pStroke[nBegPoint].x;
            float y1 = pStroke[nBegPoint].y;
            float x2 = x1;
            float y2 = y1;
		    for ( register UINT i = nBegPoint+1; i < nEndPoint; i++ )
            {
                dsx += (pStroke[i].x - pStroke[i-1].x);
                dsy += (pStroke[i].y - pStroke[i-1].y);
                if ( pStroke[i].x < x1 )
                    x1 = pStroke[i].x;
                if ( pStroke[i].x > x2 )
                    x2 = pStroke[i].x;
                if ( pStroke[i].y < y1 )
                    y1 = pStroke[i].y;
                if ( pStroke[i].y > y2 )
                    y2 = pStroke[i].y;
            }
            if ( fabsf( dsx ) <= fabsf( (2.0f*(x2 - x1))/3.0f + 1.0f ) && fabsf( dsy ) <= fabsf( 2.0f*((y2 - y1))/3.0f + 1.0f ) )
            {
			    nStrokeCnt = nEndPoint;
                nBeg = nBegPoint;
			    return true;
            }
		}
	}
	return false;
}

////////////////////////////////////////////////////////////////////////////
// BOOL FindCell( LPPOINT pStroke, UINT & nIndex, UINT nCnt, LPPOINT pPt )
//
// this function finds a grid cell for each pixel in the stroke
//
////////////////////////////////////////////////////////////////////////////

BOOL CShapesRec::FindCell( LPPOINTS pStroke, UINT & nIndex, UINT nCnt, LPMYPOINT pPoint )
{
	float		nWidth = (m_ShapeRect.right - m_ShapeRect.left)/m_nGridSize;
	float		nHeight = (m_ShapeRect.bottom - m_ShapeRect.top)/m_nGridSize;
	RECT	r;
	POINTS	pt = pStroke[nIndex++];

    if ( nWidth <= 0.0f || nWidth > 32000.0f || nHeight <= 0.0f || nHeight > 32000.0f )
        return false;

	r.left = m_ShapeRect.left;
	r.right = r.left + m_nGridSize;
	for ( float x = 0; x < nWidth; x++ )
	{
		r.top = m_ShapeRect.top;
		r.bottom = r.top + m_nGridSize;
		for ( float y = 0; y < nHeight; y++ )
		{
			if ( PtInRect( &r, pt ) )
			{
                // cell found
                // real coordinates
				pPoint->pt.x = r.left + m_nGridSize/2;
				pPoint->pt.y = r.top + m_nGridSize/2;
                // cell index (x,y)
                pPoint->cell.x = x * CELL_SIZE;
                pPoint->cell.y = y * CELL_SIZE;
				while ( nIndex < nCnt && PtInRect( &r, pt ) )
				{
					// skip all pixels that belong to the same cell
					pt = pStroke[nIndex++];
				} 
				return true;
			}
			r.top += m_nGridSize;
			r.bottom += m_nGridSize;
		}
		r.left += m_nGridSize;
		r.right += m_nGridSize;
	}
	return true;	
}

////////////////////////////////////////////////////////////////////////////
// BOOL AdjustToGrid()
//
// this function generates array of cells with the grid size resolution
// it creates the m_points array of pixels. Each pixel in this array is 
// located in the middle of each cell
//
////////////////////////////////////////////////////////////////////////////

BOOL CShapesRec::AdjustToGrid()
{
	if ( m_nStrokeCnt < MIN_STROKE_LEN || NULL == m_pStroke )
		return false;

	m_points.RemoveAll();
	MYPOINT	point = { {0, 0 }, { 0, 0 } };
    RECT    rAdjusted;
    int     nCellIndex = 0;
    rAdjusted.left = rAdjusted.right = m_ptCenter.x;
	rAdjusted.top = rAdjusted.bottom = m_ptCenter.y;
	
    m_nCellLeft = m_nCellRight = 
    m_nCellTop = m_nCellBottom = -1;
    m_points.RemoveAll();
	for ( UINT nIndex = 0; nIndex < m_nStrokeCnt; )
	{
		if ( ! FindCell( m_pStroke, nIndex, m_nStrokeCnt, &point ) )
			return false;		// error: can't find cell
        BOOL    bAlreadyExist = false;
        for ( int j = m_points.GetSize()-1; j >= max( 0, m_points.GetSize()-4); j-- )
        {
            if ( point.cell.x == m_points.GetAt(j).cell.x && point.cell.y == m_points.GetAt(j).cell.y )
            {
                bAlreadyExist = true;
                break;
            }
        }
        if ( ! bAlreadyExist )
        {
		    nCellIndex = m_points.Add( point );
		    if ( point.pt.x < rAdjusted.left )
            {
			    rAdjusted.left = point.pt.x;
                m_nCellLeft = nCellIndex;        // index of the most left cell
            }
		    if ( point.pt.x > rAdjusted.right ) 
            {
			    rAdjusted.right = point.pt.x;
                m_nCellRight = nCellIndex;       // index of the most right cell
            }
		    if ( point.pt.y < rAdjusted.top )
            {
			    rAdjusted.top = point.pt.y;
                m_nCellTop = nCellIndex;         // index of the most top cell
            }
		    if ( point.pt.y > rAdjusted.bottom )
            {
			    rAdjusted.bottom = point.pt.y;
                m_nCellBottom = nCellIndex;      // index of the most bottom cell
            }
        }
	}
    m_ptGravCenter = FindGravityCenter();
    // MYTRACE2( "Gravity Center x=%d, y=%d\n", m_ptGravCenter.x, m_ptGravCenter.y );
	return true;
}

////////////////////////////////////////////////////////////////////////////
// BOOL FindGravityCenter()
//
// this function finds the gravity center of the m_points array
//
////////////////////////////////////////////////////////////////////////////

POINTS CShapesRec::FindGravityCenter()
{
    POINTS   pt = {0.0f, 0.0f};
    float    sumx = 0.0f, sumy = 0.0f;
    int     N = m_points.GetSize();
    for ( int i = 0; i < N; i++ )
    {
        sumx += m_points.GetAt(i).pt.x;
        sumy += m_points.GetAt(i).pt.y;
    }
    pt.x = sumx / N;
    pt.y = sumy / N;
    return pt;
}

////////////////////////////////////////////////////////////////////////////
// BOOL CalcStraightLine( LPPOINT * ppOutStroke, UINT *pnStrokeCnt )
//
// this function attempts to recognize the connected shape. If the shape is
// recognized it returns the shape ID and a new stroke (point array)
//
////////////////////////////////////////////////////////////////////////////

SHAPETYPE CShapesRec::CalcStraightLine( LPPOINTS * ppOutStroke, UINT *pnStrokeCnt, SHAPETYPE inType )
{
    SHAPETYPE   nResult = SHAPE_UNKNOWN;
	int			N = m_points.GetSize();
	// first and last point
	POINTS		pt1 = m_points.GetAt(0).pt;
	POINTS		pt2 = m_points.GetAt(N-1).pt;
	float		dx = pt2.x - pt1.x;
	float		dy = pt2.y - pt1.y;
	BOOL		bError = false;
    BOOL        bArrow = false;
    int         nDirCnt = 0;
    float       sddx = 0, sddy = 0;

    // is it arrow?
    if ( dx && dy )
    {
        int     NN = 0;
        float   d1 = 0.0f;

        for ( int i = 1; i < N; i++ )
        {
		    float    ddx = (m_points.GetAt(i).pt.x - pt1.x);
            float    ddy = (m_points.GetAt(i).pt.y - pt1.y);
            float    diff = sqrtf( ddx * ddx + ddy * ddy );
            if ( diff >= d1 )
            {
                d1 = diff;
                NN = i;
            }
        }
        if ( NN < N-3 && NN > N/2 )
        {
            N = NN + 1;
            pt2 = m_points.GetAt(N-1).pt;
	        dx = pt2.x - pt1.x;
	        dy = pt2.y - pt1.y;
            bArrow = true;
        }
    }

	int     xSign = (dx > 0.0f) ? 1 : ((dx == 0.0f) ? 0 : -1); 
    int     ySign = (dy > 0.0f) ? 1 : ((dy == 0.0f) ? 0 : -1);
	for ( int i = 1; i < N; i++ )
	{
		float ddx = (m_points.GetAt(i).pt.x - m_points.GetAt(i-1).pt.x);
        float ddy = (m_points.GetAt(i).pt.y - m_points.GetAt(i-1).pt.y);
        if ( ddx != 0 )
        {
            int s = (ddx < 0.0f) ? -1 : 1;
            if ( s != xSign )
                nDirCnt++;
        }
        if ( ddy != 0 )
        {
            int s = (ddy < 0.0f) ? -1 : 1;
            if ( s != ySign )
                nDirCnt++;
        }
        sddx += fabsf( ddx );
        sddy += fabsf( ddy );
    }

    if ( nDirCnt > N/10 )
        return nResult;
    if ( sddx > fabsf( dx ) + 2.0f * m_nGridSize )
        return nResult;
    if ( sddy > fabsf( dy ) + 2.0f * m_nGridSize )
        return nResult;

	if ( fabsf( dx ) <= m_nGridSize )
	{
		// make it a vertical line
		pt1.x = pt2.x = (pt1.x + pt2.x)/2.0f;
        dx = 0.0f;
        // vertical line, calculate distance from min.x to x1
		for ( int i = 0; i < N; i++ )
		{
			float    ddx = fabsf( pt1.x - m_points.GetAt(i).pt.x );
			if ( ddx > (m_nGridSize * 2.0f) )
			{
				// error: this is not a straight line
				bError = true;
				break;
			}
		}
	}
	else if ( fabsf( dy ) <= m_nGridSize )
	{
		// make it a horizontal line
		pt1.y = pt2.y = (pt1.y + pt2.y)/2.0f;
        dy = 0.0f;
        // horizontal line, calculate distance from min.y to y1
		for ( int i = 0; i < N; i++ )
		{
			float    ddy = fabsf( pt1.y - m_points.GetAt(i).pt.y );
			if ( ddy > (m_nGridSize * 2.0f) )
			{
				// error: this is not a straight line
				bError = true;
				break;
			}
		}
	}
    else
    {
        // calcualate distance from a pixel to the line
        float    b = pt1.y - (dy * pt1.x) / dx;
        float    dir = 1.0f + sqrtf( (dy * dy)/(dx * dx) );

		for ( int i = 0; i < N; i++ )
		{
			POINTS	pt =  m_points.GetAt(i).pt;
			float   div = (dy * pt.x)/dx + (b - pt.y);
			float   ddy = fabsf( div/dir );
			if ( ddy > (m_nGridSize * 3.0f) )
			{ 
				bError = true;
				break;
			}
		}
    }
	if ( ! bError )
	{
		int		nStrokeLen = 2;
		LPPOINTS	pResult = new POINTS[5];
		if ( NULL == pResult )
			return nResult;
        pResult[0].x = pt1.x;
        pResult[0].y = pt1.y;
        pResult[1].x = pt2.x;
        pResult[1].y = pt2.y;
		if ( 0 != (inType & SHAPE_LINE) )
			nResult = SHAPE_LINE;
        if ( bArrow && 0 != (inType & SHAPE_ARROW) )
        {
            dx = pt2.x - pt1.x;
            dy = pt2.y - pt1.y;
            float x1, y1, x2, y2, x3, y3;
            if ( fabsf( dx ) > fabsf( dy ) )
            {
                if ( dx > 0 )
                {
                    for ( x1 = pt2.x; true; x1-- )
                    {
                        y1 = pt2.y + ((x1 - pt2.x) * dy) / dx;
                        float dx1 = pt2.x - x1;
                        float dy2 = pt2.y - y1;
                        if ( 12.0f <= sqrtf( dx1 * dx1 + dy2 * dy2 ) )
                        {
                            break;
                        }
                    }
                }
                else
                {
                    for ( x1 = pt2.x; true; x1++ )
                    {
                        y1 = pt2.y + ((x1 - pt2.x) * dy) / dx;
                        float dx1 = pt2.x - x1;
                        float dy2 = pt2.y - y1;
                        if ( 12.0f <= sqrtf( dx1 * dx1 + dy2 * dy2 ) )
                        {
                            break;
                        }
                    }
                }
                if ( dy == 0.0f )
                {
                    x2 = x3 = x1;
                    y2 = y1 - 4.0f;
                    y3 = y1 + 4.0f;
                }
                else
                {
                    for ( x2 = x1+1; true; x2++ )
                    {
                        y2 = y1 - ((x2 - x1) * dx) / dy;
                        float dx1 = pt2.x - x2;
                        float dy2 = pt2.y - y2;
                        if ( 15.0f <= sqrtf( dx1 * dx1 + dy2 * dy2 ) )
                        {
                            break;
                        }
                    }
                    if ( fabsf( y2 - y1 ) > 4.0f )
                    {
                        if ( y2 > y1 )
                            y2 = y1 + 4.0f;
                        else
                            y2 = y1 - 4.0f;
                        x2 = x1 - ((y2 - y1) * dy)/dx;
                    }
                    y3 = y1 - (y2 - y1);
                    if ( x2 == x1 )
                        x3 = x1 + ((dy>dx) ? 1.0f : -1.0f);
                    else
                        x3 = x1 - (x2 - x1);

                }
            }
            else
            {
                if ( dy > 0 )
                {
                    for ( y1 = pt2.y; true; y1-- )
                    {
                        x1 = pt2.x + ((y1 - pt2.y) * dx) / dy;
                        float dx1 = pt2.x - x1;
                        float dy2 = pt2.y - y1;
                        if ( 12.0f <= sqrtf( dx1 * dx1 + dy2 * dy2 ) )
                        {
                            break;
                        }
                    }
                }
                else
                {
                    for ( y1 = pt2.y; true; y1++ )
                    {
                        x1 = pt2.x + ((y1 - pt2.y) * dx) / dy;
                        float dx1 = pt2.x - x1;
                        float dy2 = pt2.y - y1;
                        if ( 12.0f <= sqrtf( dx1 * dx1 + dy2 * dy2 ) )
                        {
                            break;
                        }
                    }
                }
                if ( dx == 0 )
                {
                    y2 = y3 = y1;
                    x2 = x1 - 4.0f;
                    x3 = x1 + 4.0f;
                }
                else
                {
                    for ( y2 = y1+1; true; y2++ )
                    {
                        x2 = x1 - ((y2 - y1) * dy) / dx;
                        float dx1 = pt2.x - x2;
                        float dy2 = pt2.y - y2;
                        if ( 15.0f <= sqrtf( dx1 * dx1 + dy2 * dy2 ) )
                        {
                            break;
                        }
                    }
                    if ( fabsf( x2 - x1 ) > 4.0f )
                    {
                        if ( x2 > x1 )
                            x2 = x1 + 4.0f;
                        else
                            x2 = x1 - 4.0f;
                        y2 = y1 - ((x2 - x1) * dx)/dy;
                    }
                    x3 = x1 - (x2 - x1);
                    y3 = y1 - (y2 - y1);
                }
            }
            
            pResult[2].x = x2;
            pResult[2].y = y2;
            pResult[3].x = x3;
            pResult[3].y = y3;
            pResult[4].x = pt2.x;
            pResult[4].y = pt2.y;
			nStrokeLen = 5;        
		    nResult = SHAPE_ARROW;
        }
		*ppOutStroke = pResult;
		*pnStrokeCnt = nStrokeLen;
	}
	return nResult;
}

////////////////////////////////////////////////////////////////////////////
//
// BOOL CalcConnectedShape( LPPOINT * ppOutStroke, UINT *pnStrokeCnt )
//
// this function attempts to recognize the connected shape. If the shape is
// recognized it returns the shape ID and a new stroke (point array)
//
////////////////////////////////////////////////////////////////////////////

BOOL CShapesRec::IsMonotonous( int indexFrom, int indexTo )
{
	BOOL	bResult = false;
	// first and last point
	if ( (indexTo - indexFrom) < 2 )
		return false;
	
	int		nOffset = (indexTo - indexFrom)/10;
	POINTS	pt1 = m_points.GetAt(indexFrom+nOffset).pt;
	POINTS	pt2 = m_points.GetAt(indexTo-nOffset).pt;
	float	dx = pt2.x - pt1.x;
	float	dy = pt2.y - pt1.y;
	int     xSign = (dx > 0.0f) ? 1 : ((dx == 0.0f) ? 0 : -1); 
    int     ySign = (dy > 0.0f) ? 1 : ((dy == 0.0f) ? 0 : -1);
	
	if ( fabsf( dy ) > fabsf( dx/3.0f ) )
		return false;
	
	int sy = 0;
	for ( int i = indexFrom+1; i < indexTo; i++ )
	{
		float ddx = (m_points.GetAt(i).pt.x - m_points.GetAt(i-1).pt.x);
        float ddy = (m_points.GetAt(i).pt.y - m_points.GetAt(i-1).pt.y);
        if ( ddx != 0 )
        {
            int s = (ddx < 0.0f) ? -1 : 1;
            if ( s != xSign )
                return bResult;
        }
		
        if ( ddy != 0 )
        {
            int s = (ddy < 0.0f) ? -1 : 1;
            if ( s != ySign )
			{
				if ( (sy++) > 2 )
					return bResult;
			}
			else
				sy = 0;
        }
    }
	return true;
}


SHAPETYPE CShapesRec::CalcConnectedShape( LPPOINTS * ppOutStroke, UINT *pnStrokeCnt, SHAPETYPE inType )
{
    int         i, N = m_points.GetSize();
    float       sum = 0;
    ArrMinMax   minmaxarray;
    ArrEntropy  entarray;
    SHAPETYPE   nResult = SHAPE_UNKNOWN;

    for ( i = 0; i < N; i++ )
    {
        // calculate the distance from the gravity center to
        // ecah cell in the array, add this info to the entropy array
        float   dx = (m_points.GetAt(i).pt.x - m_ptGravCenter.x);
        float   dy = (m_points.GetAt(i).pt.y - m_ptGravCenter.y);
        float   h2 = dx * dx + dy * dy;
        float   sq = sqrtf( h2 );
        sum += sq;
        entarray.Add( sq );
    }

    // find min/max info of the enropy array
    int     nArrSize = entarray.GetSize();
    float   gc1 = sum / nArrSize;
    float   nMinMax = entarray.GetAt(0);
    int     nDirection = 0, nDirectionSpan = 0;
    int     nLimit = nArrSize;
    int     nIndex = 0;
    int     nMaxCnt = 0, nMinCnt = 0;

    for ( i = 1; i < nLimit; i++ )
    {
        float fn = entarray.GetAt(i);
        if ( 0 == nDirection )
        {
            // first pixel after min (or max) was found,
            // set the direction (1) up (-1) down
            if ( fn < nMinMax )
                nDirection = -1;
            else if ( fn > nMinMax )
                nDirection = 1;
            nMinMax = fn;
            nDirectionSpan = 0;
        }
        else if ( -1 == nDirection )
        {
            // negative direction (going down)
            if ( fn <= nMinMax )
            {
                nMinMax = fn;
                nIndex = i;
                nDirectionSpan = 0;
            }
            else
            {
                // if direction changes for 1-2 pixels, we should ignore it, not a minimum yet
                nDirectionSpan++;
                if ( nDirectionSpan > m_nDirSpan )
                {
                    // found minimum
                    LPMINMAX    pMM = new MINMAX;
                    if ( NULL != pMM )
                    {
                        pMM->fVal = nMinMax;
                        pMM->nX = m_points.GetAt(nIndex).pt.x;
                        pMM->nY = m_points.GetAt(nIndex).pt.y;
                        pMM->bMaximum = false;
                        pMM->nIndex = nIndex;
                        minmaxarray.Add( pMM );
                        nMinCnt++;
						if ( nMinCnt > FOUR_ANGLE && nMaxCnt > FOUR_ANGLE )
							break;
                        // MYTRACE3( "Minimum found: x=%d, y=%d, index=%d\n", pMM->nX, pMM->nY, pMM->nIndex );
                    }
                    nDirectionSpan = 0;
                    nDirection = 1;
                    nMinMax = fn;
                    nIndex = i;
                }
            }
        }
        else if ( 1 == nDirection )
        {
            // positive direction (going up)
            if ( fn >= nMinMax )
            {
                nMinMax = fn;
                nIndex = i;
                nDirectionSpan = 0;
            }
            else
            {
                // if direction changes for 1-2 pixels, we should ignore it, not a maximum yet
                nDirectionSpan++;
                if ( nDirectionSpan > m_nDirSpan )
                {
                    // found maximum
                    LPMINMAX    pMM = new MINMAX;
                    if ( NULL != pMM )
                    {
                        pMM->fVal = nMinMax;
                        pMM->nX = m_points.GetAt(nIndex).pt.x;
                        pMM->nY = m_points.GetAt(nIndex).pt.y;
                        pMM->bMaximum = true;
                        pMM->nIndex = nIndex;
                        minmaxarray.Add( pMM );
                        // MYTRACE3( "Maximum found: x=%d, y=%d, index=%d\n", pMM->nX, pMM->nY, pMM->nIndex );
                        nMaxCnt++;
						if ( nMinCnt > FOUR_ANGLE && nMaxCnt > FOUR_ANGLE )
							break;
                    }
                    nDirectionSpan = 0;
                    nDirection = -1;
                    nMinMax = fn;
                    nIndex = i;
                }
            }
        }
        if ( i == nArrSize - 1 && nLimit == nArrSize && minmaxarray.GetSize() > 0 )
        {
            // restart from the beginning, this time until the first minmax is found
            // this is needed because shape does not have to be connected at one of
            // its corners
            LPMINMAX pMM = minmaxarray.GetAt(0);
            if ( NULL != pMM )
            {
                nLimit = pMM->nIndex;
                i = 0;
            }
        }
    }

    // check minimax - delete minmax's that are too close
    nLimit = minmaxarray.GetSize(); 
    for ( i = 1; i < nLimit; i++ )
    {
        LPMINMAX pMM0 = minmaxarray.GetAt(i-1);
        LPMINMAX pMM1 = minmaxarray.GetAt(i);
        LPMINMAX pMM2 = minmaxarray.GetAt( (i >= nLimit - 1) ? 0 : i + 1 );
        if ( pMM0 != NULL && pMM1 != NULL && pMM2 != NULL )
        {
            if ( abs( pMM0->nIndex - pMM1->nIndex ) <= m_nDirSpan || 
                        abs( pMM2->nIndex - pMM1->nIndex ) <= m_nDirSpan )
            {
                // skip the minimax
                delete pMM1;
                minmaxarray.RemoveAt(i);
                nLimit--;
            }
        }
    }

#ifdef _EXPORT_CSV
    {{
        // store CSV file
        WCHAR   szFileName[MAX_PATH];
        if ( GenerateFileName( szFileName ) )
            ExportCSV( szFileName, entarray );
    }}
#endif // _EXPORT_CSV
	
	if ( nMaxCnt >= FOUR_ANGLE && nMinCnt >= FOUR_ANGLE && 0 != (inType & SHAPE_SCRATCH) )
	{
		// found maximum

		if ( (m_ShapeRect.right - m_ShapeRect.left) > (3 * (m_ShapeRect.bottom - m_ShapeRect.top))/2 )
		{
			MINMAX    mm0;
			mm0.fVal = entarray.GetAt(0);
			mm0.nX = m_points.GetAt(0).pt.x;
			mm0.nY = m_points.GetAt(0).pt.y;
			mm0.bMaximum = true;
			mm0.nIndex = 0;
			LPMINMAX pMM0 = &mm0;
			
			nLimit = minmaxarray.GetSize();

			for ( i = 0; i < nLimit; i++ )
			{	
				LPMINMAX pMM1 = minmaxarray.GetAt(i);
				if ( pMM1->bMaximum )
				{
					//if ( bMax )
					//	break;
					if ( ! IsMonotonous( pMM0->nIndex+1, pMM1->nIndex-1 ) )
						break;
					pMM0 = pMM1;
				}
			}
			if ( i >= nLimit-1 )
				nResult = SHAPE_SCRATCH;
		}
	}
    if ( SHAPE_UNKNOWN == nResult && nMaxCnt == FOUR_ANGLE && nMinCnt == FOUR_ANGLE && 0 != (inType & SHAPE_RECTANGLE) )
    {
        // rectangle?
        const int   nPointCnt = FOUR_ANGLE;
        POINTS      pts[FOUR_ANGLE];
        
        if ( AnalyzeMinMax( pts, nPointCnt, minmaxarray ) )
        {
            // make opposite lines parallel
            if ( MakeParallel( pts[0], pts[1], pts[2], pts[3] ) &&
                    MakeParallel( pts[1], pts[2], pts[3], pts[0] ) )
            {
                // replace the stroke 
				LPPOINTS	pResult = new POINTS[nPointCnt + 1];
                if ( NULL != pResult )
                {
                    for ( i = 0; i <= nPointCnt; i++ )
                    {
                        int index = ((i>=nPointCnt) ? 0 : i);
                        pResult[i].x = pts[index].x;
                        pResult[i].y = pts[index].y;
                    }
					(*ppOutStroke) = pResult;
                    (*pnStrokeCnt) = nPointCnt+1;
                    nResult = SHAPE_RECTANGLE;
                }
            }
        }
    }

    if ( SHAPE_UNKNOWN == nResult && 0 != (inType & SHAPE_CIRCLE) )
    {
        // is circle?
        float   negsum = 0.0f, possum = 0.0f;
        int     nspan = 0, pspan = 0;
        for ( i = 0; i < entarray.GetSize(); i++ )
        {
            float dx = entarray.GetAt(i) - gc1;
            if ( dx < 0.0f )
            {
                negsum -= dx;
                nspan++;
            }
            else if ( dx > 0.0f )
            {
                possum += dx;
                pspan++;
            }
        }

        float rad = gc1;
        if ( nspan > 0 && pspan > 0 )
        {
            float     rad1 = gc1 - negsum/nspan;
            float     rad2 = gc1 + possum/pspan;
            rad = (rad1 + rad2)/2.0f;
            // MYTRACE( L"Center line coord: gc1=%d, rad1=%d, rad2=%d\n", gc1, rad1, rad2 );
        }

        // find circle trashld
        if ( fabsf( gc1 - rad ) <= m_nGridSize )
        {
            // draw the circle
            int nFarPixels = 0;
            int nMaxPixels = entarray.GetSize();
            for ( i = 0; i < nMaxPixels; i++ )
            {
                float    de = fabsf( entarray.GetAt(i) - rad );
                if ( de > m_nGridSize * 2 )
                    nFarPixels++;
            }
            // allow up to 5% of pixels to be outside the defined boundries
            if ( nFarPixels <= (nMaxPixels + 10) / 20 && GenerateCirclePts( rad, ppOutStroke, pnStrokeCnt ) )
            {
                nResult = SHAPE_CIRCLE;
            }
        }
    }
    if ( SHAPE_UNKNOWN == nResult && nMaxCnt == TRI_ANGLE && nMinCnt == TRI_ANGLE && 0 != (inType & SHAPE_TRIANGLE) )
    {
        // triangle?
        // analyze the points
        const int   nPointCnt = TRI_ANGLE;
        POINTS      pts[TRI_ANGLE];
        if ( AnalyzeMinMax( pts, nPointCnt, minmaxarray ) )
        {
            // replace the stroke 
			LPPOINTS	pResult = (LPPOINTS)malloc( sizeof( POINTS ) * (nPointCnt+4) );
            if ( NULL != pResult )
            {
				for ( i = 0; i <= nPointCnt; i++ )
				{
					int index = ((i>=nPointCnt) ? 0 : i);
					pResult[i].x = (pts[index].x);
					pResult[i].y = (pts[index].y);
				}
				(*ppOutStroke) = pResult;
				(*pnStrokeCnt) = nPointCnt+1;
                nResult = SHAPE_TRIANGLE;
            }
        }
    }

    // free memory
    for ( i = 0; i < minmaxarray.GetSize(); i++ )
    {
        LPMINMAX pMM = minmaxarray.GetAt(i);
        if ( NULL != pMM )
            delete pMM;
    }
    return nResult;
}


BOOL CShapesRec::GenerateCirclePts( float radius, LPPOINTS * ppOutStroke, UINT *pnStrokeCnt )
{
    // using the mid-point algorithm
    float   xc = m_ptGravCenter.x;
    float   yc = m_ptGravCenter.y;
    float   x, rad2 = radius * radius;
    int     i = 0;
	LPPOINTS	pCircle = (LPPOINTS)malloc( sizeof( POINTS ) * (int)(10* radius) );
	if ( NULL == pCircle )
		return false;
    for ( x = -radius; x <= radius; x += 0.5 )
    {
        float    y = sqrtf( rad2 - x * x );
        pCircle[i].x = ((xc + x));
        pCircle[i].y = ((yc + y));
        i++;
    }
    for ( x = radius; x >= -radius; x -= 0.5 )
    {
        float    y = sqrtf( rad2 - x * x );
        pCircle[i].x = ((xc + x));
        pCircle[i].y = ((yc - y));
        i++;
    }
    pCircle[i++] = pCircle[0];
	*ppOutStroke = pCircle;
	*pnStrokeCnt = i;
    return true;
}


////////////////////////////////////////////////////////////////////////////
// AnalyzeMinMax( LPPOINT ppts, int nPointCnt, ArrMinMax & minmaxarray )
//
// this function analyzes the min/max info of the entropy array and finds
// corners of the object.
//
////////////////////////////////////////////////////////////////////////////

BOOL CShapesRec::AnalyzeMinMax( LPPOINTS ppts, int nPointCnt, ArrMinMax & minmaxarray )
{
    // analyze the points
    int     i, k, nMinMaxSize = minmaxarray.GetSize();
    BOOL    bError = false;

    for ( i = 0, k = 0; i < nMinMaxSize; i++ )
    {
        LPMINMAX pMM = minmaxarray.GetAt(i);
        if ( NULL != pMM )
        {
            if ( pMM->bMaximum && k < nPointCnt )
            {
                ppts[k].x = pMM->nX;
                ppts[k].y = pMM->nY;
                k++;
            }
        }
    }
    for ( i = 1; i <= nPointCnt; i++ )
    {
        int      index = ((i==nPointCnt) ? 0 : i);
        float    x2 = ppts[index].x;
        float    y2 = ppts[index].y;
        float    x1 = ppts[i-1].x;
        float    y1 = ppts[i-1].y;
        float    dx = x2 - x1;
        float    dy = y2 - y1;
        if ( (fabsf( dx ) + fabsf( dy )) < (m_nGridSize * 3.0f) )
        {
            bError = true;
            break;
        }

        bError = true;
        for ( k = 0; k < nMinMaxSize; k++ )
        {
            LPMINMAX pMM = minmaxarray.GetAt(k);
            if ( NULL != pMM )
            {
                if ( ! pMM->bMaximum )
                {
                    // this is minimum, check if it is the right one
                    if ( x1 < x2 )
                    {
                        if ( pMM->nX < x1 - m_nGridSize || pMM->nX > x2 + m_nGridSize )
                            continue;
                    }
                    else
                    {
                        if ( pMM->nX < x2 - m_nGridSize || pMM->nX > x1 + m_nGridSize )
                            continue;
                    }
                    if ( y1 < y2 )
                    {
                        if ( pMM->nY < y1 - m_nGridSize || pMM->nY > y2 + m_nGridSize )
                            continue;
                    }
                    else
                    {
                        if ( pMM->nY < y2 - m_nGridSize || pMM->nY > y1 + m_nGridSize )
                            continue;
                    }
                    // calculate the distance from the min point to the line
                    if ( 0 == dx )
                    {
                        // vertical line, calculate distance from min.x to x1
                        float    ddx = fabsf( x1 - pMM->nX );

                        // MYTRACE1( "ddx=%d\n", ddx );

                        if ( ddx < (m_nGridSize * 3.0f) )
                        {
                            bError = false;
                            break;
                        }
                    }
                    else
                    {
                        // calcualate distance from a pixel to the line
                        float    b = y1 - (dy * x1) / dx;
                        float    div = ((dy * pMM->nX)/dx + b - pMM->nY);
                        float    dir = 1.0f + sqrtf( (dy * dy)/(dx * dx) );
                        float    ddy = fabsf( div/dir );

                        // MYTRACE1( "ddy=%d\n", ddy );

                        if ( ddy < (m_nGridSize * 3.0f) )
                        {
                            bError = false;
                            break;
                        }
                    }
                }
            }
        }
        if ( bError )
            break;
    }
    return !bError;
}

////////////////////////////////////////////////////////////////////////////
// MakeParallel( POINTS pt1, POINTS pt2, POINTS pt3, POINTS pt4 )
//
// this function makes to opposite lines parallel
//
////////////////////////////////////////////////////////////////////////////

BOOL CShapesRec::MakeParallel( POINTS & pt1, POINTS & pt2, POINTS & pt3, POINTS & pt4 )
{
    float    dx1 = pt2.x - pt1.x;
    float    dy1 = pt2.y - pt1.y;
    float    dx2 = pt3.x - pt4.x;
    float    dy2 = pt3.y - pt4.y;

    if ( fabsf( dx1 ) <= (m_nGridSize * 3.0f) || fabsf( dx2 ) <= (m_nGridSize * 3.0f) )
    {
        // rectangle object: vertical line
        pt3.x = pt4.x = (pt3.x + pt4.x)/2;
        pt1.x = pt2.x = (pt1.x + pt2.x)/2;
        return true;
    }
    else if ( fabsf( dy1 ) <= (m_nGridSize * 3.0f) || fabsf( dy2 ) <= (m_nGridSize * 3.0f) )
    {
        // rectangle object: horizontal line
        pt3.y = pt4.y = (pt3.y + pt4.y)/2;
        pt1.y = pt2.y = (pt1.y + pt2.y)/2;
        return true;
    }
    else
    {
		dx1 = pt3.x - pt1.x;
        dy1 = pt3.y - pt1.y;
        dx2 = pt4.x - pt2.x;
        dy2 = pt4.y - pt2.y;

        if ( fabsf( dx1 ) <= (m_nGridSize * 4.0f) && fabsf( dy2 ) <= (m_nGridSize * 4.0f) ) // && dx1 != 0 )
        {
            // diamond object (vertical)
            pt1.x = pt3.x = (pt1.x + pt3.x)/2.0f;
            pt2.y = pt4.y = (pt2.y + pt4.y)/2.0f;
            if ( pt1.y < pt3.y )
                pt3.y = (pt2.y + pt4.y) - pt1.y;
            else 
                pt1.y = (pt2.y + pt4.y) - pt3.y;
            if ( pt2.x < pt4.x )
                pt4.x = (pt1.x + pt3.x) - pt2.x;
            else 
                pt2.x = (pt1.x + pt3.x) - pt4.x;
            return true;
        }
        else if ( fabsf( dx2 ) <= (m_nGridSize * 4.0f) && fabsf( dy1 ) <= (m_nGridSize * 4.0f) ) // && dx2 != 0 )
        {
            // diamond object (horizontal)
            pt2.x = pt4.x = (pt2.x + pt4.x)/2.0f;
            pt1.y = pt3.y = (pt1.y + pt3.y)/2.0f;
            if ( pt2.y < pt4.y )
                pt4.y = (pt1.y + pt3.y) - pt2.y;
            else 
                pt2.y = (pt1.y + pt3.y) - pt4.y;
            if ( pt1.x < pt3.x )
                pt3.x = (pt2.x + pt4.x) - pt1.x;
            else 
                pt1.x = (pt2.x + pt4.x) - pt3.x;
            return true;
        }
    }
    
    // TODO: make lines parallel
    
    return false;
}

#ifdef _EXPORT_CSV

const WCHAR szDataPath[] = L"\\data\\";
const WCHAR szFilePref[] = L"str";
const WCHAR szFileSuff[] = L".csv";

BOOL CShapesRec::GenerateFileName( LPTSTR pszFileName )
{
    // generate unique file name
    for ( int i = 0; i < 100; i++ ) // to avoid infinate loop
    {
        *pszFileName = 0;
        lstrcpy( pszFileName, szDataPath );
        lstrcat( pszFileName, szFilePref );
        wsprintf( &pszFileName[lstrlen(pszFileName)-1], L"%x", GetTickCount() );
        lstrcat( pszFileName, szFileSuff );
        if ( 0xFFFFFFFF == GetFileAttributes( pszFileName ) )
            return true;    // file does not exsist
    }
    return false;
}

BOOL CShapesRec::ExportCSV( LPCTSTR pszFileName, ArrEntropy & entarray )
{
    if ( m_points.GetSize() < 2 )
        return false;

    DWORD   dwLen, dwWritten = 0;
    WCHAR   szLine[MAX_PATH];
    HANDLE  hFile = CreateFile( pszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 
                               FILE_ATTRIBUTE_NORMAL, NULL);
    if ( INVALID_HANDLE_VALUE == hFile )
        return false;    
    for ( int i = 0; i < m_points.GetSize(); i++ )
    {
        POINT   pt = m_points.GetAt(i).pt;
        wsprintf( szLine, L"{%d,%d},\n", pt.x, pt.y );
        dwLen = sizeof( WCHAR ) * lstrlen( szLine );
        if ( ! WriteFile( hFile, szLine, dwLen, &dwWritten, NULL ) )
            break;  // error
    }

    wsprintf( szLine, L"\n\n==== Entropy ====\n\n" );
    dwLen = sizeof( WCHAR ) * lstrlen( szLine );
    WriteFile( hFile, szLine, dwLen, &dwWritten, NULL );

    for ( i = 0; i < entarray.GetSize(); i++ )
    {
        long en = entarray.GetAt(i);
        wsprintf( szLine, L"{%d,%d},\n", i, en );
        dwLen = sizeof( WCHAR ) * lstrlen( szLine );
        if ( ! WriteFile( hFile, szLine, dwLen, &dwWritten, NULL ) )
            break;  // error
    }

    CloseHandle( hFile );
    return true;
}

#endif // _EXPORT_CSV

