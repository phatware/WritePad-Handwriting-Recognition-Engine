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

#if !defined(AFX_SHAPESREC_H__AB5A99A0_415A_4F9C_89BC_16CF88BEF9C8__INCLUDED_)
#define AFX_SHAPESREC_H__AB5A99A0_415A_4F9C_89BC_16CF88BEF9C8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PHTemplArray.h"
#include "RecognizerApi.h"

///////////////////////////////////////////////////////////////////////////
// type definitions  

// this structure stores min/max information of entropy array
typedef struct tagMINMAX {
    float   fVal;
    float   nX;
    float   nY;
    int     nIndex;
    BOOL    bMaximum;
} MINMAX, * LPMINMAX;

typedef struct tagMYPOINT {
    POINTS   pt;
    POINTS   cell;
} MYPOINT, * LPMYPOINT;

typedef PHArray< LPMINMAX > ArrMinMax;
typedef PHArray< MYPOINT >  ArrPoints;
typedef PHArray< float >    ArrEntropy;

typedef UInt32		UINT;

///////////////////////////////////////////////////////////////////////////
// class CShapesRec  

class CShapesRec  
{
public:
	CShapesRec();
	virtual ~CShapesRec();

	SHAPETYPE RecognizeShape( LPPOINTS pInStroke, LPPOINTS * ppOutStroke, UINT *pnStrokeCnt, SHAPETYPE typein );
	SHAPETYPE RecognizeShape( CGPoint * pInStroke, LPPOINTS * ppOutStroke, UINT *pnStrokeCnt, SHAPETYPE typei );
    SHAPETYPE RecognizeShape( CGTracePoint * pInStroke, LPPOINTS * ppOutStroke, UINT *pnStrokeCnt, SHAPETYPE inType );

protected:
    BOOL	GetStrokeRect( BOOL bConencted );
	BOOL	IsConnectedShape( LPPOINTS pStroke, UINT & nStrokeCnt, UINT & nBegPoint );
	BOOL	FindCell( LPPOINTS pStroke, UINT & nIndex, UINT nCnt, LPMYPOINT pPoint );
	BOOL	AdjustToGrid();
	BOOL	IsStrateLine();
    POINTS	FindGravityCenter();
    BOOL    CopyStrokePoints( LPPOINTS pStroke, UINT nBegPoint, UINT nStrokeCnt );
    BOOL    AnalyzeMinMax( LPPOINTS ppts, int nPointCnt, ArrMinMax & minmaxarray );
    BOOL    MakeParallel( POINTS & pt1, POINTS & pt2, POINTS & pt3, POINTS & pt4 );
    BOOL    GenerateCirclePts( float radius, LPPOINTS * ppStroke, UINT *pnStrokeCnt );
	int		TraceToRec( int in_numpts, LPPOINTS in_pt, LPPOINTS out_pt ) const;
	int		FilterTrajectory( int innumpts, LPPOINTS instrkin, LPPOINTS * iostrkout ) const;
	BOOL	IsMonotonous( int indexFrom, int indexTo );

    SHAPETYPE   CalcConnectedShape( LPPOINTS * ppStroke, UINT * pnStrokeCnt, SHAPETYPE inType );
	SHAPETYPE	CalcStraightLine( LPPOINTS * ppStroke, UINT * pnStrokeCnt, SHAPETYPE inType );

private:
	ArrPoints	m_points;
    int         m_nLeft, m_nTop, m_nRight, m_nBottom;
    RECT        m_ShapeRect;
    POINTS      m_ptCenter;
    POINTS      m_ptGravCenter;
    int         m_nCellLeft, m_nCellTop, m_nCellRight, m_nCellBottom;
    float       m_nGridSize;
    LPPOINTS    m_pStroke;
    UINT        m_nStrokeCnt;
    int         m_nDirSpan;

#ifdef _EXPORT_CSV
    BOOL    GenerateFileName( LPTSTR pszFileName );
    BOOL    ExportCSV( LPCTSTR pszFileName, ArrEntropy & entarray );
#endif // _EXPORT_CSV
};


///////////////////////////////////////////////////////////////////////////
// inline functions
//
// binary square root for integer numbers; it is faster than 
// using the floating numbers
//

__inline ULONG _isqrt( ULONG val )
{
    ULONG   rem = 0;
    ULONG   root = 0;

    for ( int i = 0; i < 16; i++ )
    {
        root <<= 1;
        rem = ((rem << 2) + (val >> 30));
        val <<= 2;
        root++;
        if ( root <= rem )
        {
            rem -= root;
            root++;
        }
        else
        {
            root--;
        }
    }
    return (ULONG)(root >> 1);
}


#endif // !defined(AFX_SHAPESREC_H__AB5A99A0_415A_4F9C_89BC_16CF88BEF9C8__INCLUDED_)
