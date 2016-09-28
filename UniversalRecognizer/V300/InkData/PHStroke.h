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

#include "PHTemplArray.h"
#include "RecognizerApi.h"
#include "recotypes.h"

#define FSTROKE_RECOGNIZE	0x0001
#define FSTROKE_SELECT		0x0002

/*
#define FMulDiv(a,b,c)      ( ((b)==(c)) ? (a) : ( ((double)(a)*(double)(b))/ (double)(c) ) )
#define MulDiv(a,b,c)       ( ((b)==(c)) ? (a) : ( (int)( ((double)(a)*(double)(b))/ (double)(c) ) ) )
#define CeilMulDiv(a,b,c)   ( ((b)==(c)) ? (a) : ( (int)ceil( (double)(a)*(double)(b)/(double)(c) )) )
#define AveMulDiv(a,b,c)    ( ((b)==(c)) ? (a) : (_calc_av( FMulDiv( a, b, c ) )) )
*/

class PHPoint
{
public:	
	//standard oparatotions
	PHPoint() { ::memset( &m_Point, 0, sizeof(CGTracePoint)); m_Point.pressure = DEFAULT_PRESSURE; }
    PHPoint( float iX, float iY, int pressure ) { m_Point.pt.x = iX; m_Point.pt.y = iY; m_Point.pressure = pressure; } 
	PHPoint( const POINTS * initPt, int pressure ) { m_Point.pt.x = initPt->x; m_Point.pt.y = initPt->y; m_Point.pressure = pressure; }
	PHPoint( const POINT * initPt, int pressure ) { m_Point.pt.x = (CGFloat)initPt->x; m_Point.pt.y = (CGFloat)initPt->y; m_Point.pressure = pressure; }
	PHPoint( const POINTS& initPt, int pressure ) { m_Point.pt.x = initPt.x; m_Point.pt.y = initPt.y; m_Point.pressure = pressure; }
	PHPoint( const POINT& initPt, int pressure ) { m_Point.pt.x = (CGFloat)initPt.x; m_Point.pt.y = (CGFloat)initPt.y; m_Point.pressure = pressure; }
	PHPoint( const CGPoint& initPt, int pressure ) { m_Point.pt.x = initPt.x; m_Point.pt.y = initPt.y; m_Point.pressure = pressure; }
	PHPoint( const PHPoint& initPt ) { m_Point = initPt.m_Point; }
	PHPoint( const CGTracePoint& initPt ) { ::memcpy( &m_Point, &initPt, sizeof( CGTracePoint ) ); }
	
    PHPoint operator= (const PHPoint pnt) 
    { 
        m_Point = pnt.m_Point;
        m_Point.pressure = pnt.m_Point.pressure;
		return m_Point;
    }
	
    PHPoint operator= (const POINTS * pPnt) 
    { 
        m_Point.pt.x = pPnt->x;  
        m_Point.pt.y = pPnt->y;  
        m_Point.pressure = DEFAULT_PRESSURE;
        return m_Point;
    }

    PHPoint operator= (const CGPoint * pPnt) 
    { 
        m_Point.pt.x = pPnt->x;  
        m_Point.pt.y = pPnt->y;  
        m_Point.pressure = DEFAULT_PRESSURE;
        return m_Point;
    }
	
    PHPoint operator= (const POINTS& pnt) 
    { 
        m_Point.pt.x = pnt.x;  
        m_Point.pt.y = pnt.y;  
        m_Point.pressure = DEFAULT_PRESSURE;
        return m_Point;
    }
		
    PHPoint operator+= (const float nOffset ) 
    { 
        m_Point.pt.x += nOffset;  
        m_Point.pt.y += nOffset;  
        return m_Point;
    }
	
	void offset( const float nOffsetX, const float nOffsetY )
	{
        m_Point.pt.x += nOffsetX;  
        m_Point.pt.y += nOffsetY;  
	}
	
	float	X() const { return m_Point.pt.x; }
	float	Y() const { return m_Point.pt.y; }
	int		P() const { return m_Point.pressure; }
    
	void	SetPenPressure( int pressure ) { m_Point.pressure = pressure; }
	
    BOOL operator== (const POINTS pnt) 
    { return ( ((pnt.x == m_Point.pt.x) && (pnt.y == m_Point.pt.y)) ? true : false ); }
    
    BOOL operator!= (const POINTS pnt) 
    { return ( ((pnt.x != m_Point.pt.x) || (pnt.y != m_Point.pt.y)) ? true : false ); }
    
    const CGTracePoint& GetPoint() const { return m_Point; }
    void  GetXY( float& iX,float& iY) const { iX = m_Point.pt.x; iY = m_Point.pt.y; }
	
private:
	CGTracePoint   m_Point;
};

typedef PHArray<PHPoint>  PHSubStroke;

class PHStroke
{
public:
	PHStroke();
	PHStroke( float fWidth, COLORREF iColor );
	PHStroke( const PHStroke * stroke );
	virtual ~PHStroke();
    
    void        SetColor( COLORREF iColor) { m_nColor = iColor; }
    COLORREF    GetColor() const { return m_nColor; }
	void        SetWidth( float fWidth) {m_fWidth = fWidth; }
    void        SetZOrder( int zOrder ) { m_ZOrder = zOrder; }
    float       GetWidth() const { return m_fWidth; }
    bool        IsSelected() const { return ((m_dwFlags & FSTROKE_SELECT) != 0); }
    void        SetSelected( BOOL b ) { if (b) m_dwFlags |= FSTROKE_SELECT; else m_dwFlags &= ~FSTROKE_SELECT; }
    bool        IsRecognizable() const { return ((m_dwFlags & FSTROKE_RECOGNIZE) != 0); }
    void        SetRecognizable( BOOL b ) { if (b) m_dwFlags |= FSTROKE_RECOGNIZE; else m_dwFlags &= ~FSTROKE_RECOGNIZE; }
    DWORD       GetStrokeFlags() const { return m_dwFlags; }
    void        SetStrokeFlags( DWORD dwFlags ) { m_dwFlags = dwFlags; }
	
	int         AddPoints( const PHPoint * pPnt, UInt32 uCnt );   
	int			AddPoints( const CGTracePoint * points, UInt32 uCnt );
	int			AddPoints( const POINT *pPnt, UInt32 uCnt, int pressure = DEFAULT_PRESSURE ); 
	int         AddPoints( const POINTS * pPnt, UInt32 uCnt, int pressure = DEFAULT_PRESSURE );   
	int         AddPoints( const CGPoint * pPnt, UInt32 uCnt, int pressure = DEFAULT_PRESSURE );   
    int         AddPoints( PHStroke * pStroke );
	
	void		SetPressure(int iIndex, int pressure );
	int			GetPressure(int iIndex);
	
	int         RemovePoints( const PHPoint Pnt[], UInt32 uFirstInd, UInt32 uCnt );   
    void        RemoveAllPoints() { m_SubStrList.RemoveAll(); m_ScaledPoints.RemoveAll(); }
	BOOL		RevertToOriginal();
	int			StoreScaledPoints();
	
    PHStroke *  Clone( void );
	
    PHSubStroke* GetSubStroke() 
    { 
        if ( m_ScaledPoints.GetSize() > 0 )
            return &m_ScaledPoints;
        else
            return &m_SubStrList; 
    }     
	PHSubStroke* GetSubStroke1() 
    { 
        return &m_SubStrList; 
    }     
    int         SetScaledPoints( float x0, float y0, float scalex, float scaley, bool bReset, LPRECT lpRect );
	
	void		GetScaleInfo( float * px0, float * py0, float * pscalex, float * pscaley )
	{
		*px0 = m_x0;
		*py0 = m_y0;
		*pscalex = m_xScale;
		*pscaley = m_yScale;
	}
	
	void	ResetPosition()
	{
		m_x0 = 0;
		m_y0 = 0;
	}
	
    void        ResetScale()
    {
		InitScale();
        m_ScaledPoints.RemoveAll();
    }
	
    BOOL        IsScaledX() const
    {
        return (m_xScale != 1.0);
    }
    BOOL        IsScaledY() const
    {
        return (m_yScale != 1.0);
    }
    
    int         ZOrder() const
    {
        return m_ZOrder;
    }
	
    void        InitScale()
    {
        m_xScale = 1.0;
        m_yScale = 1.0;
		m_x0 = 0.0;
		m_y0 = 0.0;
    }	
	
private:
    COLORREF    m_nColor;
    float       m_fWidth;
    UInt32      m_SubStrCnt;
    PHSubStroke m_SubStrList;
    PHSubStroke m_ScaledPoints;
    DWORD       m_dwFlags;
    float       m_xScale;
    float       m_yScale;
	float		m_x0;
	float		m_y0;
    int         m_ZOrder;
	
#if defined(DEBUG) || defined(_DEBUG)
	static int  m_siObjects;
#endif
};

typedef PHArray<PHStroke *> PHStrokeArr;

