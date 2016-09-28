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

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "InkWrapper.h"
#include "InkData.h"
#include "ShapesRec.h"

#define STROKE_BUFFER_SIZE	1024

extern "C"
INK_DATA_PTR INK_InitData()
{
	CInkData * pData = new CInkData;
	return (INK_DATA_PTR)pData;
}

extern "C"
INK_DATA_PTR INK_CreateCopy( INK_DATA_PTR pData )
{
	CInkData * pNewData = NULL;
	if ( NULL != pData )
	{
		pNewData = new CInkData;
		if ( NULL != pNewData )
		{
			pNewData->CopyData( (CInkData *)pData );
		}
	}
	return pNewData;
}

extern "C"
void INK_SortInk( INK_DATA_PTR pData )
{
	if ( NULL == pData )
		return;
	CInkData * gpData = (CInkData *)pData;	
	gpData->SortStrokes() ;
}


extern "C"
void INK_SetUndoLevels( INK_DATA_PTR pData, int levels )
{
	if ( NULL == pData )
		return;
	CInkData * gpData = (CInkData *)pData;	
	gpData->SetUndoLevels( levels );
}	

extern "C"
void INK_Undo( INK_DATA_PTR pData )
{
	if ( NULL == pData )
		return;
	CInkData * gpData = (CInkData *)pData;	
	gpData->Undo() ;
}

extern "C"
void INK_Redo( INK_DATA_PTR pData )
{
	if ( NULL == pData )
		return;
	CInkData * gpData = (CInkData *)pData;	
	gpData->Redo() ;
}

extern "C"
BOOL INK_CanRedo( INK_DATA_PTR pData )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;	
	return gpData->CanRedo() ;
}

extern "C"
BOOL INK_CanUndo( INK_DATA_PTR pData )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;	
	return gpData->CanUndo() ;
}

extern "C"
BOOL INK_DeleteSelectedStrokes( INK_DATA_PTR pData, BOOL bAll )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;	
	return gpData->RemoveSelected( bAll );
}

extern "C"
BOOL INK_IsStrokeSelected( INK_DATA_PTR pData, int nStroke )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;	
	return gpData->IsStrokeSelected( nStroke );
}

extern "C"
BOOL INK_IsStrokeRecognizable( INK_DATA_PTR pData, int nStroke )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;	
	return gpData->IsStrokeRecognizable( nStroke );
}

extern "C"
BOOL INK_SelectAllStrokes( INK_DATA_PTR pData, BOOL bSelect )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;	
	return gpData->SelectAllStrokes( bSelect );
}

extern "C"
void INK_SetStrokesRecognizable( INK_DATA_PTR pData, BOOL bSet, BOOL bSelectedOnly )
{
	if ( NULL == pData )
		return;
	CInkData * gpData = (CInkData *)pData;	
	gpData->SetStrokesRecognizable( bSet, bSelectedOnly );
}

extern "C"
void INK_SetStrokeRecognizable( INK_DATA_PTR pData, int nStroke, BOOL bSet )
{
	if ( NULL == pData )
		return;
	CInkData * gpData = (CInkData *)pData;	
	gpData->SetStrokeRecognizable( bSet );
}

extern "C"
void INK_SelectStroke( INK_DATA_PTR pData, int nStroke, BOOL bSelect )
{
	if ( NULL == pData )
		return;
	CInkData * gpData = (CInkData *)pData;	
	gpData->SelectStroke( nStroke, bSelect );
}


extern "C"
void INK_FreeData( INK_DATA_PTR pData )
{	
	if ( NULL != pData )
	{
		CInkData * gpData = (CInkData *)pData;
        gpData->DeleteAllImageObjects();
        gpData->DeleteAllTextObjects( false );
        gpData->FreeUndo();
		delete gpData;
	}
}

extern "C"
void INK_EmptyUndoBuffer( INK_DATA_PTR pData )
{
	if ( NULL == pData )
		return;
	CInkData * gpData = (CInkData *)pData;	
	gpData->FreeUndo();
}	


extern "C"
void INK_Erase( INK_DATA_PTR pData )
{
	if ( NULL == pData )
		return;
	CInkData * gpData = (CInkData *)pData;	
	gpData->FreeUndo();
	gpData->FreeStrokes();	
}	

extern "C"
int INK_StrokeCount( INK_DATA_PTR pData, BOOL selectedOnly )
{
	if ( NULL == pData )
		return 0;
	CInkData * gpData = (CInkData *)pData;
	return gpData->HasStrokes( selectedOnly );
}	

extern "C"
void INK_EnableShapeRecognition( INK_DATA_PTR pData, BOOL bEnable )
{
	if ( NULL == pData )
		return;
	CInkData * gpData = (CInkData *)pData;
	gpData->m_bRecorgnizeShapes = bEnable;
}	

extern "C"
BOOL INK_IsShapeRecognitionEnabled( INK_DATA_PTR pData )
{
	if ( NULL == pData )
		return 0;
	CInkData * gpData = (CInkData *)pData;
	return gpData->m_bRecorgnizeShapes;
}	

extern "C"
int INK_AddStroke( INK_DATA_PTR pData, CGStroke pStroke, int nStrokeCnt, float fWidth, COLORREF color )
{
	if ( NULL == pData )
		return 0;
	CInkData * gpData = (CInkData *)pData;	
	
	gpData->RecordUndo( UNDO_STROKES_ADDED );
	int nStroke = gpData->AddNewStroke( (float)fWidth, color );
	gpData->StopRecordingUndo();
	if ( nStroke < 0 )
		return 0;
    
	if ( gpData->m_bRecorgnizeShapes && nStrokeCnt > 15 )
	{
		CShapesRec	shapes;
		LPPOINTS	pOutStroke = NULL;
		UINT         nNewStrokeCnt = nStrokeCnt;
		SHAPETYPE	result = shapes.RecognizeShape( pStroke, &pOutStroke, &nNewStrokeCnt, (SHAPETYPE)(SHAPE_ALL & (~SHAPE_SCRATCH)) );
		if ( result != SHAPE_UNKNOWN && pOutStroke != NULL )
		{
			int nRes = gpData->AddPointsToStroke( nStroke, pOutStroke, (int)nNewStrokeCnt );
			free( (void *)pOutStroke );
			return nRes;
		}
	}
    
	return gpData->AddPointsToStroke( nStroke, pStroke, nStrokeCnt );
}


extern "C"
SHAPETYPE INK_RecognizeShape( CGStroke pStroke, int nStrokeCnt, SHAPETYPE inType )
{
	if ( nStrokeCnt > 15 )
	{
		CShapesRec	shapes;
		LPPOINTS	pOutStroke = NULL;
		UINT		nNewStrokeCnt = nStrokeCnt;
		SHAPETYPE	result = shapes.RecognizeShape( pStroke, &pOutStroke, &nNewStrokeCnt, inType );
		if ( pOutStroke != NULL )
			free( (void *)pOutStroke );
		return result;
	}
	return SHAPE_UNKNOWN;
}

extern "C"
int  INK_GetStroke( INK_DATA_PTR pData, int nStroke, CGPoint ** ppoints, float * pnWidth, COLORREF * pColor )
{
	static int	nMemSize = STROKE_BUFFER_SIZE;
	if ( NULL == pData )
		return (-1);
	
	CInkData * gpData = (CInkData *)pData;
	int nStrokeCnt = gpData->StrokesTotal();
	if ( nStroke >= nStrokeCnt )
		return 0;
	
	CGPoint *	points = ppoints ? *ppoints : NULL;
	int		nStrokeLen = (int)gpData->GetStrokePointCnt( nStroke );
	if ( points == NULL )
	{
		nMemSize = nStrokeLen + STROKE_BUFFER_SIZE;
		points = (CGPoint *)malloc( nMemSize * sizeof( CGPoint ) );
	}
	else if ( nStrokeLen >= nMemSize )
	{
		nMemSize = STROKE_BUFFER_SIZE + nStrokeLen;
		points = (CGPoint *)realloc( (void *)points, nMemSize * sizeof( CGPoint ) );
	}
    if ( points == NULL )
        return (-1);
    
    float width = 1.0;
	if (  gpData->ReadOneStroke( nStroke, points, &nStrokeLen, &width, pColor ) > 0 )
	{
        if ( pnWidth != NULL )
            *pnWidth = width;
		if ( ppoints == NULL )
			free( (void *)points );
		else
			*ppoints = points;
	}
    else
    {
        if ( ppoints == NULL )
            free( (void *)points );
    }
	return nStrokeLen;
}


extern "C"
int  INK_GetStrokeP( INK_DATA_PTR pData, int nStroke, CGStroke* ppoints, float * pfWidth, COLORREF * pColor )
{
	static int	nMemSize = STROKE_BUFFER_SIZE;
	if ( NULL == pData )
		return (-1);
	
	CInkData * gpData = (CInkData *)pData;
	int nStrokeCnt = gpData->StrokesTotal();
	if ( nStroke >= nStrokeCnt )
		return 0;
	
	CGStroke	points = ppoints ? *ppoints : NULL;
	int		nStrokeLen = (int)gpData->GetStrokePointCnt( nStroke );
	if ( points == NULL )
	{
		nMemSize = nStrokeLen + STROKE_BUFFER_SIZE;
		points = (CGStroke)malloc( nMemSize * sizeof( CGTracePoint ) );
	}
	else if ( nStrokeLen >= nMemSize )
	{
		nMemSize = STROKE_BUFFER_SIZE + nStrokeLen;
		points = (CGStroke)realloc( (void *)points, nMemSize * sizeof( CGTracePoint ) );
	}
    float width = 1.0;
	if (  gpData->ReadOneStroke( nStroke, points, &nStrokeLen, &width, pColor ) > 0 )
	{
        if ( pfWidth != NULL )
            *pfWidth = width;
		if ( ppoints == NULL )
			free( (void *)points );
		else
			*ppoints = points;
	}
	return nStrokeLen;
}

extern "C"
void INK_EnableUndo( INK_DATA_PTR pData, BOOL enable )
{
    if ( NULL == pData )
        return;
    CInkData * gpData = (CInkData *)pData;
    gpData->EnableUndo( enable );
}

extern "C"
BOOL INK_ResizeStroke( INK_DATA_PTR pData, int nStroke, float x0, float y0, float scalex, float scaley, BOOL bReset, CGRect * pRect, BOOL recordUndo )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	RECT	rData = {0};
		
	if ( recordUndo )
		gpData->RecordUndo( UNDO_STROKES_MOVED );
	
	int nStrokesCnt = gpData->StrokesTotal();
	if ( nStroke < 0 || nStroke >= nStrokesCnt )
	{	
		for ( int i = 0; i < nStrokesCnt; i++ )
		{
			if ( (!gpData->IsStrokeSelected(i)) )
				continue;
			RECT	r;
			if (  gpData->ResizeStroke( i, x0, y0, scalex, scaley, bReset, &r ) )
			{
				if ( i == 0 )
				{
					rData = r;
				}
				else
				{
					rData.left = min( rData.left, r.left );
					rData.top = min( rData.top, r.top );
					rData.right = max( rData.right, r.right );
					rData.bottom = max( rData.bottom, r.bottom );
				}
			}
		}
	}
	else
	{
		gpData->ResizeStroke( nStroke, x0, y0, scalex, scaley, bReset, &rData );
	}

	if ( recordUndo )
		gpData->StopRecordingUndo();
	
	if ( rData.left == 0 && rData.right == 0 && rData.top == 0 && rData.bottom == 0 )
		return false;
	pRect->origin.x = rData.left;
	pRect->origin.y = rData.top;
	pRect->size.width = rData.right - rData.left;
	pRect->size.height = rData.bottom - rData.top;
	return true;
	
}

extern "C"
int INK_Serialize( INK_DATA_PTR pData, BOOL bWrite, FILE * pFile, void ** ppData, long * pcbSize, BOOL skipImages, BOOL savePressure )
{
	if ( NULL == pData )
		return 0;

	CInkData * gpData = (CInkData *)pData;
	if ( bWrite )
	{
		DWORD dwFormat = MAKE_WRITE_FMT( INK_PWCOMP, (pFile == NULL) ? true : false );
        if ( savePressure )
            dwFormat |= SAVE_PRESSURE;
		return  gpData->Write( dwFormat, pFile, ppData, pcbSize );
	}
	else
	{
		DWORD dwFormat = MAKE_READ_FMT( INK_PWCOMP, (pFile == NULL) ? true : false );
		return gpData->Read( dwFormat, pFile, *ppData, *pcbSize, skipImages );
	}
}

extern "C"
int INK_FindStrokeByPoint( INK_DATA_PTR pData, CGPoint thePoint, float proximity )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	POINTS pt;
	pt.x = thePoint.x;
	pt.y = thePoint.y;
	return gpData->IsPointNearStroke( pt, proximity );
}

extern "C"
int INK_DeleteIntersectedStrokes( INK_DATA_PTR pData, const CGStroke points, int nPointCount )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	return gpData->DeleteIntersectedStrokes( points, nPointCount );
}

extern "C"
BOOL INK_CurveIntersectsStroke( INK_DATA_PTR pData, int nStroke, const CGStroke points, int nPointCount )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	return gpData->CurveIntersectsStroke( nStroke, points, nPointCount );
}

extern "C"
BOOL INK_Paste( INK_DATA_PTR pData, const void * pRawData, long cbSize, CGPoint atPosition )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	return gpData->PasteRawInk( pRawData, cbSize, atPosition.x, atPosition.y );
}

extern "C"
BOOL INK_Copy( INK_DATA_PTR pData, void ** ppRawData, long * pcbSize )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	*pcbSize = gpData->CopyRawInk( ppRawData );
	return ( *pcbSize > 0 ) ? true : false;	
}

extern "C"
BOOL INK_MoveStroke( INK_DATA_PTR pData, int nStroke, float xOffset, float yOffset, CGRect * pRect, BOOL recordUndo )
{
	if ( NULL == pData )
		return false;
	CInkData *	gpData = (CInkData *)pData;
	RECT		r, rData = { 0, 0, 0, };
	BOOL		result = false;

	if ( recordUndo )
		gpData->RecordUndo( UNDO_STROKES_MOVED );
	
	if ( nStroke == -1 )
	{
		// move all selected strokes
		int		nCnt = 0;
		for ( register int i = 0; i < gpData->StrokesTotal(); i++ )
		{
			if ( gpData->IsStrokeSelected( i ) )
			{
				if ( nCnt == 0 )
				{
					if ( ! gpData->MoveStroke( i, xOffset, yOffset, &r  ) )
						break;
				}
				else
				{
					if ( ! gpData->MoveStroke( i, xOffset, yOffset, &rData  ) )
						break;
					r.left = min( rData.left, r.left );
					r.top = min( rData.top, r.top );
					r.right = max( rData.right, r.right );
					r.bottom = max( rData.bottom, r.bottom );
				}
				nCnt++;
			}
		}
		if ( nCnt > 0 )
		{
			pRect->origin.x = r.left;
			pRect->origin.y = r.top;
			pRect->size.width = r.right - r.left;
			pRect->size.height = r.bottom - r.top;				
			result = true;
		}
	}
	else
	{
		if ( gpData->MoveStroke( nStroke, xOffset, yOffset, &r ) )
		{
			pRect->origin.x = r.left;
			pRect->origin.y = r.top;
			pRect->size.width = r.right - r.left;
			pRect->size.height = r.bottom - r.top;
			result = true;
		}
	}
	if ( recordUndo )
		gpData->StopRecordingUndo();

	return result;
}

extern "C" 
void INK_ChangeSelZOrder( INK_DATA_PTR pData, int iDepth, BOOL bFwd )
{
	if ( NULL == pData )
		return;
	CInkData * gpData = (CInkData *)pData;
	gpData->ChangeZOrder( iDepth, bFwd );
}

extern "C"
int INK_GetStrokeZOrder( INK_DATA_PTR pData, int nStroke )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
    PHStroke * stroke = gpData->GetStroke( nStroke );
    if ( stroke != NULL )
        return stroke->ZOrder();
    return (-1);
}

extern "C"
BOOL INK_SetStrokeZOrder( INK_DATA_PTR pData, int nStroke, int iZOrder )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
    PHStroke * stroke = gpData->GetStroke( nStroke );
    if ( stroke != NULL )
    {
        stroke->SetZOrder( iZOrder );
        return true;
    }
    return false;   
}

extern "C"
BOOL INK_DeleteStroke( INK_DATA_PTR pData, int nStroke )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	gpData->RecordUndo( UNDO_STROKES_DELETED );
	if ( nStroke == -1 )
	{
		// delete last stroke
		int nCnt = gpData->StrokesTotal();
		if ( nCnt > 0 )
		{
			gpData->RemoveStroke( nCnt-1 );
		}
	}
	else
	{
		gpData->RemoveStroke( nStroke );
	}
	gpData->StopRecordingUndo();
	return true;
}

extern "C"
int INK_AddEmptyStroke( INK_DATA_PTR pData, float fWidth, COLORREF color )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;	
	int nStroke = gpData->AddNewStroke( fWidth, color );
	return nStroke;
}

extern "C"
int INK_AddPixelToStroke( INK_DATA_PTR pData, int nStroke, float x, float y, int p )
{
	if ( NULL == pData )
		return -1;
	CInkData * gpData = (CInkData *)pData;	
	int nStrokeCnt = gpData->StrokesTotal();
	if ( nStroke >= nStrokeCnt )
		return -1;
	if ( nStroke < 0 )
		nStroke = nStrokeCnt-1;	// add to last stroke
	CGTracePoint pt;
	pt.pt.x = x;
	pt.pt.y = y;
    pt.pressure = (p > 0) ? p : DEFAULT_PRESSURE;
	gpData->AddPointsToStroke( nStroke, &pt, 1 );
	return (int)gpData->GetStrokePointCnt( nStroke );
}

extern "C"
BOOL INK_GetStrokePoint( INK_DATA_PTR pData, int nStroke, int nPoint, float * pX, float * pY )
{
    int p = 0;
    return INK_GetStrokePointP( pData, nStroke, nPoint, pX, pY, &p );
}

extern "C"
BOOL INK_GetStrokePointP( INK_DATA_PTR pData, int nStroke, int nPoint, float * pX, float * pY, int * pP )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;	
	int nStrokeCnt = gpData->StrokesTotal();
	if ( nStroke >= nStrokeCnt )
		return false;
	if ( nStroke < 0 )
		nStroke = nStrokeCnt-1;	// add to last stroke
    int nStrokeLen = (int)gpData->GetStrokePointCnt( nStroke );
    if ( nStrokeLen < 1 || nPoint >= nStrokeLen )
        return false;
    if ( nPoint < 0 )
        nPoint = nStrokeLen-1;
    PHStroke * stroke = gpData->GetStroke( nStroke );
    if ( NULL == stroke )
        return false;
    PHSubStroke * subs = stroke->GetSubStroke1();
    if ( NULL == subs )
        return false;
    *pX = (float)subs->GetAt(nPoint).GetPoint().pt.x;
    *pY = (float)subs->GetAt(nPoint).GetPoint().pt.y;
    if ( pP != NULL )
        *pP = (int)subs->GetAt(nPoint).GetPoint().pressure;
    return true;
}


extern "C"
BOOL INK_GetStrokeRect( INK_DATA_PTR pData, int nStroke, CGRect * rect, BOOL bAddWidth )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	RECT	   rStroke = { 0, 0, 0, 0 };
	if ( gpData->StrokesTotal() < 1 )
        return false;
    if ( nStroke < 0 )
        nStroke = gpData->StrokesTotal()-1;
	if ( gpData->CalcStrokeRect( nStroke, &rStroke, bAddWidth ) )
	{
		rect->origin.x = rStroke.left;
		rect->origin.y = rStroke.top;
		rect->size.width = rStroke.right - rStroke.left;
		rect->size.height = rStroke.bottom - rStroke.top;
		return true;
	}
	return false;
}

extern "C"
BOOL INK_GetDataRect( INK_DATA_PTR pData, CGRect * rect, BOOL selectedOnly )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	RECT	   rStroke = { 0, 0, 0, 0 };
	RECT	   rData =  { 0, 0, 0, 0 };
	
	if ( gpData->StrokesTotal() < 1 )
		return false;
	for ( int i = 0; i < gpData->StrokesTotal(); i++ )
	{
		if ( selectedOnly && (!gpData->IsStrokeSelected(i)) )
			continue;
		if ( rData.left == 0 && rData.right == 0 && rData.top == 0 && rData.bottom == 0 )
		{
			if ( ! gpData->CalcStrokeRect( i, &rData, true ) )
				return false;
		}
		else
		{
			if ( ! gpData->CalcStrokeRect( i, &rStroke, true ) )
				break;
			rData.left = min( rData.left, rStroke.left );
			rData.top = min( rData.top, rStroke.top );
			rData.right = max( rData.right, rStroke.right );
			rData.bottom = max( rData.bottom, rStroke.bottom );
		}
	}
	if ( rData.left == 0 && rData.right == 0 && rData.top == 0 && rData.bottom == 0 )
		return false;
	rect->origin.x = rData.left;
	rect->origin.y = rData.top;
	rect->size.width = rData.right - rData.left;
	rect->size.height = rData.bottom - rData.top;
	return true;
}

extern "C"
int	INK_AddImage( INK_DATA_PTR pData, const ImageAttributes * pImage )
{
	if ( NULL == pData )
		return -1;
	CInkData * gpData = (CInkData *)pData;
	return gpData->AddImageObject( (int)pImage->imagerect.origin.x, 
								  (int)pImage->imagerect.origin.y, 
								  (int)pImage->imagerect.size.width, 
								  (int)pImage->imagerect.size.height, 
								  pImage->flags, 
								  pImage->iZOrder,
								  pImage->pData, 
								  pImage->nDataSize, 
								  pImage->userData );
}

extern "C"
int	INK_SetImage( INK_DATA_PTR pData, int nImageIndex, const ImageAttributes * pImage )
{
	if ( NULL == pData )
		return -1;
	CInkData * gpData = (CInkData *)pData;
	return gpData->AddImageObject( (int)pImage->imagerect.origin.x, 
								  (int)pImage->imagerect.origin.y, 
								  (int)pImage->imagerect.size.width, 
								  (int)pImage->imagerect.size.height, 
								  pImage->flags,
								  pImage->iZOrder,
								  pImage->pData, 
								  pImage->nDataSize, 
								  pImage->userData, 
								  nImageIndex );
}

extern "C"
BOOL INK_DeleteImage( INK_DATA_PTR pData, int nImageIndex )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	return gpData->DeleteImageObject( nImageIndex );
}

static void SetInputAttributes( ImageAttributes * pAttrib, const CImageObject * pImage )
{
	memset( pAttrib, 0, sizeof( ImageAttributes ) );
	pAttrib->imagerect.origin.x = (CGFloat)pImage->m_attrib.pt.x;
	pAttrib->imagerect.origin.y = (CGFloat)pImage->m_attrib.pt.y;
	pAttrib->imagerect.size.width = (CGFloat)pImage->m_attrib.iWidth;
	pAttrib->imagerect.size.height = (CGFloat)pImage->m_attrib.iHeight;
	pAttrib->iZOrder = pImage->m_attrib.iZOrder;
	pAttrib->nDataSize = pImage->m_attrib.dwSize;
	pAttrib->pData = pImage->m_pImageData;
	pAttrib->userData = pImage->m_userData;
	pAttrib->nIndex = pImage->m_iIndex;
	pAttrib->flags = (UInt32)pImage->m_attrib.dwRotation;
}	


extern "C"
BOOL INK_GetImage( INK_DATA_PTR pData, int nImageIndex, ImageAttributes * pAttrib )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	CImageObject * pImage = gpData->GetImageObject( nImageIndex );
	if ( pImage == NULL )
		return false;
	SetInputAttributes( pAttrib, pImage );
	return true;
}

extern "C"
int INK_GetImageFromPoint( INK_DATA_PTR pData, CGPoint point, ImageAttributes * pAttrib )
{
	if ( NULL == pData )
		return -1;
	CInkData * gpData = (CInkData *)pData;
	POINTS pts;
	pts.x = point.x; pts.y = point.y;
	CImageObject * pImage = gpData->GetImageObject( pts );
	if ( pImage == NULL )
		return -1;
	SetInputAttributes( pAttrib, pImage );
	return pImage->m_iIndex;
}

extern "C"
BOOL INK_SetImageFrame( INK_DATA_PTR pData, int nImageIndex, CGRect frame )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	return gpData->SetImageFrame( nImageIndex, (int)frame.origin.x, (int)frame.origin.y, (int)frame.size.width, (int)frame.size.height );
}

extern "C"
int INK_SelectStrokesInRect( INK_DATA_PTR pData, CGRect selRect )
{
	if ( NULL == pData )
		return -1;
	CInkData * gpData = (CInkData *)pData;
	RECT	r;
	r.left = selRect.origin.x;
	r.top = selRect.origin.y;
	r.right = r.left + selRect.size.width;
	r.bottom = r.top + selRect.size.height;
	return gpData->SelectStrokesInRect( &r );
}

extern "C"
BOOL INK_SetStrokeWidthAndColor( INK_DATA_PTR pData, int nStroke, COLORREF color, float fWidth )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	
	int nStrokeChanged = 0;
	if ( nStroke == -1 )
	{
		// 
		for ( register int i = 0; i < gpData->StrokesTotal(); i++ )
		{
			if ( gpData->IsStrokeSelected( i ) )
			{
				gpData->SetStroke( i, (float)fWidth, color );
				nStrokeChanged++;
			}
		}
	}
	else
	{
		gpData->SetStroke( nStroke, (float)fWidth, color );
		nStrokeChanged++;
	}
	return (nStrokeChanged > 0) ? true : false;
}

extern "C"
BOOL INK_DeleteAllImages( INK_DATA_PTR pData )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	return gpData->DeleteAllImageObjects();
}

extern "C"
int INK_CountImages( INK_DATA_PTR pData )
{
	if ( NULL == pData )
		return 0;
	CInkData * gpData = (CInkData *)pData;
	return gpData->GetImageObjectCnt();
}

extern "C"
BOOL INK_SetImageUserData( INK_DATA_PTR pData, int nImageIndex, void * userData )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	return gpData->SetImageUserData( nImageIndex, userData );
}

static void SetFontAttributes( LPLOGFONTW plf, const TextAttributes * pText )
{
	memset( plf, 0, sizeof( LOGFONTW ) );
	plf->lfHeight = pText->fontSize;
	ucscpy( plf->lfFaceName, pText->pFontName );
	if ( pText->fontAttributes & LF_FONT_BOLD )
		plf->lfWeight = 700;
	if ( pText->fontAttributes & LF_FONT_STRIKE )
		plf->lfStrikeOut = 1;
	if ( pText->fontAttributes & LF_FONT_ITALIC )
		plf->lfItalic = 1;
	if ( pText->fontAttributes & LF_FONT_UNDERSCORE )
		plf->lfUnderline = 1;
}

extern "C"
BOOL INK_AddText( INK_DATA_PTR pData, const TextAttributes * pText )
{
	if ( NULL == pData )
		return -1;
	CInkData * gpData = (CInkData *)pData;
	LOGFONTW	lf;
	SetFontAttributes( &lf, pText );
	CTextObject * pNewObject = gpData->AddTextObject( (int)pText->textrect.origin.x, (int)pText->textrect.origin.y, 
										  (int)pText->textrect.size.width, (int)pText->textrect.size.height, 
										  pText->alignment, &lf, pText->pUnicodeText );
	if ( pNewObject != NULL )
	{
		pNewObject->m_attrib.dwReserved = pText->iZOrder;
		pNewObject->m_attrib.crBack = pText->backColor;
		pNewObject->m_attrib.crText = pText->fontColor;
        pNewObject->m_attrib.dwFlags = pText->flags;
		return true;
	}
	return false;		
}

extern "C"
BOOL INK_SetText( INK_DATA_PTR pData, int nTextIndex, const TextAttributes * pText )
{
	if ( NULL == pData )
		return 0;
	CInkData *	gpData = (CInkData *)pData;
	LOGFONTW	lf;
	SetFontAttributes( &lf, pText );
	CTextObject * pObject = gpData->AddTextObject( (int)pText->textrect.origin.x, (int)pText->textrect.origin.y, 
													 (int)pText->textrect.size.width, (int)pText->textrect.size.height, 
													 pText->alignment, &lf, pText->pUnicodeText, nTextIndex );
	if ( pObject != NULL )
	{
		pObject->m_attrib.dwReserved = pText->iZOrder;
		pObject->m_attrib.crBack = pText->backColor;
		pObject->m_attrib.crText = pText->fontColor;
        pObject->m_attrib.dwFlags = pText->flags;
		return true;
	}
	return false;		
}

extern "C"
BOOL INK_SetTextUserData( INK_DATA_PTR pData, int nTextIndex, void * userData )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	return gpData->SetTextUserData( nTextIndex, userData );
}

extern "C"
BOOL INK_DeleteText( INK_DATA_PTR pData, int nTextIndex )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	return gpData->DeleteTextObject( nTextIndex );

}

static void SetTextAttributes( TextAttributes * pText, const CTextObject * pTextObject )
{
	memset( pText, 0, sizeof( TextAttributes ) );
	pText->textrect.origin.x = (CGFloat)pTextObject->m_attrib.pt.x;
	pText->textrect.origin.y = (CGFloat)pTextObject->m_attrib.pt.y;
	pText->textrect.size.width = (CGFloat)pTextObject->m_attrib.iWidth;
	pText->textrect.size.height = (CGFloat)pTextObject->m_attrib.iHeight;
	pText->userData = pTextObject->m_pUserData;
	pText->nIndex = pTextObject->m_iIndex;
	pText->pUnicodeText = pTextObject->GetText();
	pText->iZOrder = (int)pTextObject->m_attrib.dwReserved;
    
	pText->fontColor = pTextObject->m_attrib.crText;
    if ( ((pText->fontColor >> 24) & 0xFF) == 0 )
    {
        // fix transparent font color
        pText->fontColor |= (0xFF << 24);
    }
    
	pText->backColor = pTextObject->m_attrib.crBack;
	pText->fontSize = pTextObject->m_attrib.lf.lfHeight;
	pText->alignment = pTextObject->m_attrib.dwJustification;
    pText->flags = pTextObject->m_attrib.dwFlags;
	if ( NULL != pText->pUnicodeText )
		pText->nTextLength = ucslen( pText->pUnicodeText );
	if ( (int)pText->fontSize < 0 )
		pText->fontSize = -pText->fontSize;
	pText->pFontName = ucsdup( pTextObject->m_attrib.lf.lfFaceName );
	if ( pTextObject->m_attrib.lf.lfWeight > 400 )
		pText->fontAttributes |= LF_FONT_BOLD;
	if ( pTextObject->m_attrib.lf.lfStrikeOut > 0 )
		pText->fontAttributes |= LF_FONT_STRIKE;
	if ( pTextObject->m_attrib.lf.lfItalic > 0 )
		pText->fontAttributes |= LF_FONT_ITALIC;
	if ( pTextObject->m_attrib.lf.lfUnderline > 0 )
		pText->fontAttributes |= LF_FONT_UNDERSCORE;
}	


extern "C"
BOOL INK_GetText( INK_DATA_PTR pData, int nTextIndex, TextAttributes * pText )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	CTextObject * pTextObject = gpData->GetTextObject( nTextIndex );
	if ( pTextObject == NULL )
		return false;
	SetTextAttributes( pText, pTextObject );
	return true;	
}

extern "C"
int INK_GetTextFromPoint( INK_DATA_PTR pData, CGPoint point, TextAttributes * pText )
{
	if ( NULL == pData )
		return -1;
	CInkData * gpData = (CInkData *)pData;
	POINTS pts;
	pts.x = point.x; pts.y = point.y;
	CTextObject * pTextObject = gpData->GetTextObject( pts );
	if ( pTextObject == NULL )
		return -1;
	SetTextAttributes( pText, pTextObject );
	return pTextObject->m_iIndex;	
}

extern "C"
BOOL INK_DeleteAllTexts( INK_DATA_PTR pData, BOOL bRecordUndo )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	return gpData->DeleteAllTextObjects( bRecordUndo );
}

extern "C"
int	 INK_CountTexts( INK_DATA_PTR pData )
{
	if ( NULL == pData )
		return 0;
	CInkData * gpData = (CInkData *)pData;
	return gpData->GetTextObjectCnt();
}

extern "C"
BOOL INK_SetTextFrame( INK_DATA_PTR pData, int nTextIndex, CGRect frame )
{
	if ( NULL == pData )
		return false;
	CInkData * gpData = (CInkData *)pData;
	return gpData->SetTextFrame( nTextIndex, (int)frame.origin.x, (int)frame.origin.y, (int)frame.size.width, (int)frame.size.height );
}

