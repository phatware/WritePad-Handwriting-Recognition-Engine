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

#include "PHStroke.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define N_SEGS		3

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#if defined(DEBUG) || defined(_DEBUG)
int PHStroke::m_siObjects = 0;
#endif

PHStroke::PHStroke()
{
    m_nColor = 0;
    m_fWidth = 1.0;
    m_ZOrder = -1;
    m_SubStrList.RemoveAll(); 
    m_ScaledPoints.RemoveAll(); 
    m_SubStrCnt = 0;
    m_dwFlags = FSTROKE_RECOGNIZE;      // recognize all strokes by default
#if defined(DEBUG) || defined(_DEBUG)
    m_siObjects += 1;
#endif
    InitScale();
}

PHStroke::PHStroke(float fWidth, COLORREF iColor )
{
    m_fWidth = fWidth;
    m_nColor = iColor; 
    m_SubStrCnt = 0;
    m_ZOrder = -1;
    m_dwFlags = FSTROKE_RECOGNIZE;      // recognize all strokes by default
    m_SubStrList.RemoveAll(); 
    m_ScaledPoints.RemoveAll(); 
#if defined(DEBUG) || defined(_DEBUG)
    m_siObjects += 1;
#endif
    InitScale();
}

PHStroke::PHStroke(const PHStroke * stroke )
{
	PHStroke();
	if ( stroke != NULL )
	{
		m_SubStrList.Copy( stroke->m_SubStrList );
		m_ScaledPoints.Copy( stroke->m_ScaledPoints );
		m_nColor = stroke->GetColor();
		m_fWidth = stroke->GetWidth();
		m_dwFlags = stroke->m_dwFlags;
		m_SubStrCnt = stroke->m_SubStrCnt;
		m_xScale = stroke->m_xScale;
		m_yScale = stroke->m_yScale;
		m_x0 = stroke->m_x0;
		m_y0 = stroke->m_y0;
        m_ZOrder = stroke->m_ZOrder;
	}
}

PHStroke::~PHStroke()
{
    m_SubStrList.RemoveAll(); 
    m_ScaledPoints.RemoveAll();
}

//append the stroke the    
int PHStroke::AddPoints( const PHPoint *pPnt, UInt32 uCnt )
{
	int iFirst = m_SubStrList.GetSize();
    for( register int i = iFirst; i < iFirst + (int)uCnt; i++ )
    {
		m_SubStrList.InsertAt( i, pPnt[i-iFirst], 1 );
    }
	return m_SubStrList.GetSize();
}

int PHStroke::AddPoints( const CGTracePoint * points, UInt32 uCnt )
{
	int    iFirst = m_SubStrList.GetSize();
    for ( register int i = iFirst; i < iFirst + (int)uCnt; i++ )
    {
		m_SubStrList.InsertAt( i, PHPoint( points[i-iFirst].pt, points[i-iFirst].pressure ), 1 );
    }
	return m_SubStrList.GetSize();
}

int PHStroke::AddPoints( const POINTS *pPnt, UInt32 uCnt, int pressure )
{
	int iFirst = m_SubStrList.GetSize();
    for ( register int i = iFirst; i < iFirst + (int)uCnt; i++ )
	{
		m_SubStrList.InsertAt( i, PHPoint(pPnt[i-iFirst], pressure), 1 );
	}
	return m_SubStrList.GetSize();
}

int PHStroke::AddPoints( const POINT *pPnt, UInt32 uCnt, int pressure )
{
	int iFirst = m_SubStrList.GetSize();
    for ( register int i = iFirst; i < iFirst + (int)uCnt; i++ )
	{
		m_SubStrList.InsertAt( i, PHPoint(pPnt[i-iFirst], pressure), 1 );
	}
	return m_SubStrList.GetSize();
}

int PHStroke::AddPoints( const CGPoint *pPnt, UInt32 uCnt, int pressure )
{
	int iFirst = m_SubStrList.GetSize();
    for ( register int i = iFirst; i < iFirst + (int)uCnt; i++ )
	{
		m_SubStrList.InsertAt( i, PHPoint(pPnt[i-iFirst], pressure), 1 );
	}
	return m_SubStrList.GetSize();
}

int PHStroke::AddPoints( PHStroke * pStroke )
{
    PHSubStroke *pList = pStroke->GetSubStroke();
    if ( NULL == pList )
        return 0;
    int    uCnt = pList->GetSize();
	int    iFirst = m_SubStrList.GetSize();
    for ( register int i = iFirst; i < iFirst + uCnt; i++ )
	{
		m_SubStrList.InsertAt( i, pList->GetAt(i), 1 );
	}
	return m_SubStrList.GetSize();
}

int PHStroke::RemovePoints( const PHPoint Pnt[], UInt32 iFirst, UInt32 uCnt )
{
	for( register int i = (int)iFirst; i < (int)(iFirst + uCnt); i++ )
    {
		m_SubStrList.RemoveAt( i );
    }
	return 1;
}

BOOL PHStroke::RevertToOriginal()
{
	if ( m_ScaledPoints.GetSize() < 1 )
		return false;
	
	InitScale();
	m_ScaledPoints.RemoveAll();
	return true;
}

int PHStroke::StoreScaledPoints()
{
	if ( m_ScaledPoints.GetSize() < 1 )
		return 0;
	
	m_SubStrList.RemoveAll();
    
	register int	i;
	PHPoint			pt;
	int				iLast = m_ScaledPoints.GetSize();
	
	for ( i = 0; i < iLast; i++ )
    {
        pt = m_ScaledPoints.GetAt( i );
		m_SubStrList.Add( pt );
	}
	RevertToOriginal();
	return i;
}

void PHStroke::SetPressure( int iIndex, int pressure )
{
    if ( iIndex < 0 || m_SubStrList.GetSize() < iIndex )
        return;
	PHPoint pt = m_SubStrList.GetAt( iIndex );
	pt.SetPenPressure(pressure);    
	m_SubStrList.SetAt(iIndex, pt);
}

int PHStroke::GetPressure( int iIndex )
{
    if ( iIndex < 0 || m_SubStrList.GetSize() < iIndex )
        return DEFAULT_PRESSURE;
	PHPoint pt = m_SubStrList.GetAt( iIndex );
	return pt.P();       
}

int PHStroke::SetScaledPoints( float x0, float y0, float dscalex, float dscaley, bool bReset, LPRECT lpRect )
{
    if ( m_SubStrList.GetSize() < 1 )
        return 0;       // no points
	
	if ( dscalex != 0.0 || dscaley != 0.0 || bReset )
	{
		int        iLast = m_SubStrList.GetSize();
		
		// scale cannot be less than 1%
		if ( m_xScale + dscalex < 0.01f )
			m_xScale = 0.01f;
		else
			m_xScale += dscalex;
		if ( m_yScale + dscaley < 0.01f )
			m_yScale = 0.01f;
		else
			m_yScale += dscaley;
				
		if ( m_xScale == 1.0 && m_yScale == 1.0 )
		{
			RevertToOriginal();
			return iLast;
		}
		
		register int i;
		register float  x, y;
		PHPoint       pt;
		
		m_ScaledPoints.RemoveAll();
				
		if ( 0 == m_y0 )
			m_y0 = y0;
		if ( 0 == m_x0 )
			m_x0 = x0;
		
		for ( i = 0; i < iLast; i++ )
		{
			pt = m_SubStrList.GetAt( i );
			pt.GetXY( x, y );
			
			// move, if needed
			if ( m_x0 != x0 || m_y0 != y0 )
			{
				x += (x0-m_x0);
				y += (y0-m_y0);
				m_SubStrList.SetAt( i, PHPoint( x, y, pt.P() ) );
			}
			
			// then scale
			x = x0 + ((x-x0) * m_xScale);
			y = y0 + ((y-y0) * m_yScale);
			m_ScaledPoints.Add( PHPoint( x, y, pt.P() ) );

			if ( NULL != lpRect )
			{
				if ( x < lpRect->left )
					lpRect->left = x;
				if ( x > lpRect->right ) 
					lpRect->right = x;
				if ( y < lpRect->top )
					lpRect->top = y;
				if ( y > lpRect->bottom )
					lpRect->bottom = y;
			}
		}
		m_y0 = y0;
		m_x0 = x0;
	}
	return m_ScaledPoints.GetSize();
}

PHStroke * PHStroke::Clone( void )
{
    PHStroke *pNew = new PHStroke();
	
    pNew->m_nColor = m_nColor;
    pNew->m_fWidth = m_fWidth;
    pNew->m_SubStrCnt = m_SubStrCnt;
    pNew->m_dwFlags = m_dwFlags;
    pNew->m_xScale = m_xScale;
    pNew->m_yScale = m_yScale;
	pNew->m_x0 = m_x0;
	pNew->m_y0 = m_y0;
	pNew->m_ScaledPoints.Copy( m_ScaledPoints );
	pNew->m_SubStrList.Copy( m_SubStrList );
    pNew->m_ZOrder = m_ZOrder;
	
    return pNew;
}
