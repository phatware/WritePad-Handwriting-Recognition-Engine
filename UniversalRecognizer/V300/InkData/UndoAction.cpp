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

#include "InkData.h"
#include "UndoAction.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CUndoAction::CUndoAction( int nLevel /* = 5 */ )
{
    if ( m_nUndoLevels < 1 )
        m_nUndoLevels = DEF_UNDO_LEVELS;
	else if ( m_nUndoLevels > MAX_UNDO_LEVELS )
		m_nUndoLevels = MAX_UNDO_LEVELS;
	else
        m_nUndoLevels = nLevel;
    m_nUndoItems = 0;
    m_pUndoData = NULL;
}

CUndoAction::~CUndoAction()
{
    if ( m_pUndoData != NULL )
    {
		FreeAll();
        free( m_pUndoData );
        m_pUndoData = NULL;
    }
}

void CUndoAction::FreeAll()
{
    if ( m_pUndoData != NULL )
    {
        for ( int i = 0; i < m_nUndoItems; i++ )
        {
            if ( NULL != m_pUndoData[i] )
                delete m_pUndoData[i];
        }
    }
    m_nUndoItems = 0;
}

void CUndoAction::RemoveLastUndo()
{
    if ( m_nUndoItems > 0 && m_pUndoData[m_nUndoItems-1] )
    {
        CUndoData *pUndo = m_pUndoData[m_nUndoItems-1];
        delete pUndo;
        m_nUndoItems --;
    }
}

BOOL CUndoAction::DoUndo( CInkData *pData, CUndoAction *pRedo )
{
    if ( m_nUndoItems > 0 && m_pUndoData[m_nUndoItems-1] )
    {
		PHStroke *pStroke = NULL;
        CUndoData *pUndo = m_pUndoData[m_nUndoItems-1];
        int iType = pUndo->m_iUndoType;
		
#ifdef TEXT_SUPPORT

		if ( (iType == UNDO_TEXT_DELETED || iType == UNDO_TEXT_CHANGED || iType == UNDO_TEXT_ADDED)&& 
            pUndo->m_arrTextObjects.GetSize() > 0 )
		{
            CTextObject *pto;
            BOOL bUndoCreated = false;
	        int count = pUndo->m_arrTextObjects.GetSize();
			
	        for ( int i = count-1; i >= 0; i-- )
	        {
		        pto = pUndo->m_arrTextObjects.GetAt(i);
				
                if ( iType == UNDO_TEXT_DELETED && pto )
                {
			        if ( pRedo )
                    {
                        if ( !bUndoCreated )
                        {
				            pRedo->AddItem( UNDO_TEXT_ADDED, (CTextObject*)NULL, pto->m_iIndex, false );
                            bUndoCreated = true;
                        }
                        else
                        {
				            pRedo->AddTextObject( UNDO_TEXT_ADDED, NULL, pto->m_iIndex, false );
                        }
                    }
			        pData->InsertTextObject( pto->m_iIndex, pto );
                }
                else if ( iType == UNDO_TEXT_CHANGED && pto )
                {
			        // either text, position or any other paramteres
			        if ( pRedo )
			        {
				        CTextObject * pObj = pData->GetTextObject( pto->m_iIndex );
                        if ( pObj )
				            pRedo->AddItem( UNDO_TEXT_CHANGED, pObj, pto->m_iIndex, true );
			        }
			        pData->ReplaceTextObject( pto->m_iIndex, pto );
                }
                else if ( iType == UNDO_TEXT_ADDED )
                {
                    if ( pUndo->m_nObjectIndex >= pData->GetTextObjectCnt() )
                        pUndo->m_nObjectIndex = pData->GetTextObjectCnt() - 1;
                    
			        if ( pRedo )
			        {
                        // not sure it will be ok if we ever want to add more than 
                        // one object in one action...
				        CTextObject * pObj = pData->RemoveTextObject( pUndo->m_nObjectIndex );
                        if ( pObj )
                        {
                            if ( !bUndoCreated )
                            {
				                pRedo->AddItem( UNDO_TEXT_DELETED, pObj, pUndo->m_nObjectIndex, false );
                                bUndoCreated = true;
                            }
                            else
                            {
                                pRedo->AddTextObject( UNDO_TEXT_DELETED, pObj, pUndo->m_nObjectIndex, false );
                            }
                        }
			        }
                    else
			            pData->DeleteTextObject( pUndo->m_nObjectIndex );
                }
            }
			delete pUndo;
			m_nUndoItems --;
			return true;
		}
        else if ( iType == UNDO_TEXT_MOVED )
        {
            CTextObject * pObj = pData->GetTextObject( pUndo->m_nObjectIndex );
            if ( pObj )
            {
                pObj->m_attrib.pt = pUndo->m_ptTextPos;
                pObj->m_attrib.iHeight = pUndo->m_iTextHeight;
                pObj->m_attrib.iWidth = pUndo->m_iTextWidth;
            }
			delete pUndo;
			m_nUndoItems --;
			return true;
        }
#endif // TEXT_SUPPORT
#ifdef IMAGE_SUPPORT
		pData->m_pCurrentImageObject = NULL;
		if ( (iType == UNDO_IMAGE_DELETED || iType == UNDO_IMAGE_ADDED || iType == UNDO_IMAGE_CHANGED)&& 
            pUndo->m_arrImageObjects.GetSize() > 0 )
		{
            CImageObject *pto;
            BOOL bUndoCreated = false;
	        int count = pUndo->m_arrImageObjects.GetSize();
			
	        for ( int i = count-1; i >= 0; i-- )
	        {
		        pto = pUndo->m_arrImageObjects.GetAt(i);
				
                if ( iType == UNDO_IMAGE_DELETED && pto )
                {
			        if ( pRedo )
                    {
                        if ( !bUndoCreated )
                        {
				            pRedo->AddItem( UNDO_IMAGE_ADDED, (CImageObject*)NULL, pto->m_iIndex, false );
                            bUndoCreated = true;
                        }
                        else
                        {
				            pRedo->AddImageObject( UNDO_IMAGE_ADDED, NULL, pto->m_iIndex, false );
                        }
                    }
			        pData->InsertImageObject( pto->m_iIndex, pto );
                }
                else if ( iType == UNDO_IMAGE_CHANGED && pto )
                {
			        // either position or any other paramteres
			        if ( pRedo )
			        {
				        CImageObject * pObj = pData->GetImageObject( pto->m_iIndex );
                        if ( pObj )
				            pRedo->AddItem( UNDO_IMAGE_CHANGED, pObj, pto->m_iIndex, true );
			        }
			        pData->ReplaceImageObject( pto->m_iIndex, pto );
                }
                else if ( iType == UNDO_IMAGE_ADDED )
                {
                    if ( pUndo->m_nObjectIndex >= pData->GetImageObjectCnt() )
                        pUndo->m_nObjectIndex = pData->GetImageObjectCnt() - 1;
                    
			        if ( pRedo )
			        {
                        // not sure it will be ok if we ever want to add more than 
                        // one object in one action...
				        CImageObject * pObj = pData->RemoveImageObject( pUndo->m_nObjectIndex );
                        if ( pObj )
                        {
                            if ( !bUndoCreated )
                            {
				                pRedo->AddItem( UNDO_IMAGE_DELETED, pObj, pUndo->m_nObjectIndex, false );
                                bUndoCreated = true;
                            }
                            else
                            {
                                pRedo->AddImageObject( UNDO_IMAGE_DELETED, pObj, pUndo->m_nObjectIndex, false );
                            }
                        }
			        }
                    else
					    pData->DeleteImageObject( pUndo->m_nObjectIndex );
                }
            }
			delete pUndo;
			m_nUndoItems --;
			pData->SetModified( true );
			return true;
		}
        else if ( iType == UNDO_IMAGE_MOVED )
        {
            CImageObject * pObj = pData->GetImageObject( pUndo->m_nObjectIndex );
            if ( pObj )
            {
                pObj->m_attrib.pt = pUndo->m_ptImagePos;
                pObj->m_attrib.iHeight = pUndo->m_iImageHeight;
                pObj->m_attrib.iWidth = pUndo->m_iImageWidth;
            }
			delete pUndo;
			m_nUndoItems --;
			return true;
        }
#endif // IMAGE_SUPPORT
		
        if ( pUndo->m_iUndoType == UNDO_STROKES_DELETED && NULL != pUndo->m_pStrokes )
        {
	        int count = pUndo->m_pStrokes->GetSize();
			
            if ( pRedo )
                pRedo->AddItem(UNDO_STROKES_ADDED);
			
	        for ( int i = count-1; i >= 0; i-- )
	        {
		        pStroke = (*pUndo->m_pStrokes)[i];
		        if ( NULL != pStroke )
                {
                    pStroke->SetSelected( false );
                    pData->Add( pStroke );
					
                    if ( pRedo )
                        pRedo->AddDeleteAction();
                }
	        }
            pUndo->RemoveAll();
			
            if ( pRedo )
                pRedo->FreeUnused();
        }
        else if ( pUndo->m_iUndoType == UNDO_STROKES_MOVED && NULL != pUndo->m_pStrokes && NULL != pUndo->m_pStrokesOld )
        {
	        int count1 = pUndo->m_pStrokes->GetSize();
            int count2 = pUndo->m_pStrokesOld->GetSize();
			
            if ( count1 == count2 )
            {
                if ( pRedo )
                    pRedo->AddItem( UNDO_STROKES_MOVED );
				
	            for ( int i = 0; i < count1; i++ )
	            {
		            pStroke = (*pUndo->m_pStrokes)[i];
		            if ( NULL != pStroke )
                    {
                        PHStroke * pStrOld = (*pUndo->m_pStrokesOld)[i];
                        if ( NULL != pStrOld )
                        {
                            pRedo->AddStroke( pStroke );
                            pStroke->RemoveAllPoints();
                            pStroke->AddPoints( pStrOld );
							pStroke->RevertToOriginal();
                            pUndo->m_bStrokesDetached = false;
                        }
                    }
                }
	        }
            pUndo->RemoveAll();
            if ( pRedo )
                pRedo->FreeUnused();
        }
        else if ( pUndo->m_iUndoType == UNDO_STROKES_CHANGED && NULL != pUndo->m_pStrokes && NULL != pUndo->m_pStrokesOld )
        {
	        int count1 = pUndo->m_pStrokes->GetSize();
            int count2 = pUndo->m_pStrokesOld->GetSize();
			
            if ( count1 == count2 )
            {
                if ( pRedo )
                    pRedo->AddItem( UNDO_STROKES_CHANGED );
				
	            for ( int i = 0; i < count1; i++ )
	            {
		            pStroke = (*pUndo->m_pStrokes)[i];
		            if ( NULL != pStroke )
                    {
                        PHStroke * pStrOld = (*pUndo->m_pStrokesOld)[i];
                        if ( NULL != pStrOld )
                        {
                            pRedo->AddStroke( pStroke );
                            pStroke->SetColor(pStrOld->GetColor());
                            pStroke->SetWidth(pStrOld->GetWidth());
                            pUndo->m_bStrokesDetached = false;
                        }
                    }
                }
	        }
            pUndo->RemoveAll();
            if ( pRedo )
                pRedo->FreeUnused();
        }
        else if ( pUndo->m_iUndoType == UNDO_STROKES_ADDED && pUndo->m_iAdded > 0 )
        {
            if ( pRedo )
                pRedo->AddItem(UNDO_STROKES_DELETED);
			
            for ( int i = pData->StrokesTotal() - 1; i >= 0 && pUndo->m_iAdded > 0; i-- )
            {
                pStroke = pData->GetStroke(i);
	            if ( NULL != pStroke )
                {
                    if ( pRedo )
                        pRedo->AddStroke( pStroke );
                    else
                        delete pStroke;
					
                    pData->RemoveAt(i);
                }
                pUndo->m_iAdded --;
            }
			
            if ( pRedo )
                pRedo->FreeUnused();
        }
        delete pUndo;
        m_nUndoItems --;
        return true;
    }
    return false;
}

void CUndoAction::SetUndoLevel( int nLevel )
{
    if ( m_nUndoLevels < 1 )
        m_nUndoLevels = DEF_UNDO_LEVELS;
	else if ( m_nUndoLevels > MAX_UNDO_LEVELS )
		m_nUndoLevels = MAX_UNDO_LEVELS;
	else
        m_nUndoLevels = nLevel;
	
    if ( m_nUndoLevels != nLevel )
    {
        if ( m_pUndoData )
        {
            CUndoData **pNewData = (CUndoData **)malloc( nLevel*sizeof(CUndoData *) );
            if ( pNewData )
            {
                if ( m_nUndoItems > nLevel )
                {
                    for ( int i = 0; i < m_nUndoItems - nLevel; i++ )
                        delete m_pUndoData[i];
                }
				
                if ( m_nUndoItems > 0 )
                {
                    memcpy( pNewData, 
						   m_pUndoData + ((m_nUndoItems > nLevel) ? (m_nUndoItems - nLevel) : 0), 
						   min(m_nUndoItems,nLevel) );
                }
                free( m_pUndoData );
                m_pUndoData = pNewData;
                m_nUndoLevels = nLevel;
            }
        }
    }
}

BOOL CUndoAction::AllocBuffer( void )
{
    if ( m_pUndoData == NULL )
    {
        m_pUndoData = (CUndoData **)malloc( m_nUndoLevels*sizeof(CUndoData *) );
        if ( m_pUndoData )
            memset( m_pUndoData, 0, m_nUndoLevels*sizeof(CUndoData *) );
        else
            return false;
    }
    return true;
}

BOOL CUndoAction::AddItem( int iType )
{
    if ( !AllocBuffer() )
        return false;
	
    // if buffer is full free some space
    if ( m_nUndoItems == m_nUndoLevels )
    {
        delete m_pUndoData[0];
        if ( m_nUndoLevels > 0 )
            memmove( m_pUndoData, m_pUndoData+1, (m_nUndoLevels-1)*sizeof(m_pUndoData[0]) );
		
        m_nUndoItems --;
    }
	
    m_pUndoData[m_nUndoItems] = new CUndoData(iType);
    if ( m_pUndoData[m_nUndoItems] )
    {
        m_nUndoItems ++;
        return true;
    }
    return false;
}

void CUndoAction::FreeUnused()
{
    if ( m_pUndoData && m_nUndoItems > 0 && 
        m_pUndoData[m_nUndoItems-1] && m_pUndoData[m_nUndoItems-1]->IsEmpty() )
    {
        m_nUndoItems --;
        delete m_pUndoData[m_nUndoItems];
        m_pUndoData[m_nUndoItems] = NULL;
    }
}

BOOL CUndoAction::AddStroke( PHStroke *pStroke )
{
    if ( m_pUndoData && m_nUndoItems > 0 && m_pUndoData[m_nUndoItems-1] )
    {
        m_pUndoData[m_nUndoItems-1]->AddStroke( pStroke );
        return true;
    }
	return false;
}

BOOL CUndoAction::AddDeleteAction()
{
    if ( m_pUndoData && m_nUndoItems > 0 && m_pUndoData[m_nUndoItems-1] )
    {
        m_pUndoData[m_nUndoItems-1]->m_iAdded ++;
        return true;
    }
	return false;
}

//////////////////////////////////////////////////////////////////////
// CUndoData class implementation
//////////////////////////////////////////////////////////////////////

CUndoData::CUndoData( int iType )
{
    m_bStrokesDetached = false;
    m_iUndoType = iType;
    m_iAdded = 0;
    m_pStrokes = NULL;
    m_pStrokesOld = NULL;
	m_nObjectIndex = -1;
#ifdef TEXT_SUPPORT
    m_ptTextPos.x = m_ptTextPos.y = 0;
    m_iTextHeight = m_iTextWidth = 10;
#endif // TEXT_SUPPORT
    if ( m_iUndoType == UNDO_STROKES_DELETED || 
        m_iUndoType == UNDO_STROKES_MOVED || 
        m_iUndoType == UNDO_STROKES_CHANGED )
    {
        m_pStrokes = new PHStrokeArr;
        if ( m_iUndoType == UNDO_STROKES_MOVED || m_iUndoType == UNDO_STROKES_CHANGED )
            m_pStrokesOld = new PHStrokeArr;
    }
#ifdef TEXT_SUPPORT
	if ( m_iUndoType == UNDO_TEXT_CHANGED ||  m_iUndoType == UNDO_TEXT_DELETED )
	{
	}
#endif // TEXT_SUPPORT
}

CUndoData::~CUndoData()
{
    // AO 04-jun-04: See below - change it so we do not allocate strokes for deleted
    RemoveAll();
    if ( NULL != m_pStrokesOld )
    {
        delete m_pStrokesOld;
        m_pStrokesOld = NULL;
    }
    if ( NULL != m_pStrokes )
    {
        delete m_pStrokes;
        m_pStrokes = NULL;
    }
	
#ifdef TEXT_SUPPORT
    m_arrTextObjects.Free();
#endif // TEXT_SUPPORT
}

void CUndoData::RemoveAll()
{
    PHStroke *pStroke;
    int  count;
		
    if ( NULL != m_pStrokes )
    {
        if ( m_bStrokesDetached )
        {
	        count = m_pStrokes->GetSize();
			
	        for ( int i = 0; i < count; i++ )
	        {
		        pStroke = (*m_pStrokes)[i];
		        if ( NULL != pStroke )
			        delete pStroke;
	        }
        }
        m_bStrokesDetached = false;
        m_pStrokes->RemoveAll();
    }
    
    if ( NULL != m_pStrokesOld )
    {
        count = m_pStrokesOld->GetSize();
        
        for ( int i = 0; i < count; i++ )
        {
            pStroke = (*m_pStrokesOld)[i];
            if ( NULL != pStroke )
                delete pStroke;
        }
        m_pStrokesOld->RemoveAll();
    }
}

void CUndoData::AddStroke( PHStroke * pStroke )
{
    if ( NULL != m_pStrokes )
        m_pStrokes->Add( pStroke );
	
    if ( NULL != m_pStrokesOld )
    {
        // AO 04-jun-04: I think this might work as well - no need to alloc extra stroke
        if ( m_iUndoType == UNDO_STROKES_DELETED )
        {
            m_pStrokesOld->Add( pStroke );
        }
        else
        {
            PHStroke * pStr = new PHStroke( pStroke->GetWidth(), pStroke->GetColor() );
            if ( NULL != pStr )
            {
                pStr->AddPoints( pStroke );        
                m_pStrokesOld->Add( pStr );
            }
            // m_bStrokesDetached = true;
        }
    }
}

BOOL CUndoData::IsEmpty()
{
#ifdef TEXT_SUPPORT
	if ( m_iUndoType == UNDO_TEXT_CHANGED || m_iUndoType == UNDO_TEXT_DELETED )
		return (m_arrTextObjects.GetSize() == 0);
	else if ( m_iUndoType == UNDO_TEXT_ADDED  )
		return (0 > m_nObjectIndex);
#endif // TEXT_SUPPORT
#ifdef IMAGE_SUPPORT
	if ( m_iUndoType == UNDO_IMAGE_CHANGED || m_iUndoType == UNDO_IMAGE_DELETED )
		return (m_arrImageObjects.GetSize() == 0);
	else if ( m_iUndoType == UNDO_IMAGE_ADDED  )
		return (0 > m_nObjectIndex);
#endif // TEXT_SUPPORT
    return ( m_iAdded == 0 && (m_pStrokes == NULL || m_pStrokes->GetSize() == 0));
}

#ifdef TEXT_SUPPORT

BOOL CUndoAction::UpdateLastType( int iType )
{
	if ( m_nUndoItems < 1 )
		return false;
	CUndoData * pUndo = m_pUndoData[m_nUndoItems-1];
	if ( pUndo == NULL )
		return false;
	if ( pUndo->m_iUndoType == UNDO_TEXT_ADDED && iType == UNDO_TEXT_DELETED )
	{
		delete pUndo;
		m_nUndoItems--;		// delete last undo action
		return true;
	}
	if ( 0 != (pUndo->m_iUndoType & (UNDO_TEXT_CHANGED|UNDO_TEXT_DELETED)) &&
		0 != (iType & (UNDO_TEXT_CHANGED|UNDO_TEXT_DELETED)) )
	{
		pUndo->m_iUndoType = iType;
		return true;
	}
	return false;
}

BOOL CUndoData::AddTextObject( int nIndex, CTextObject *pTextObject, BOOL bClone )
{
	m_nObjectIndex = nIndex;
	
    if ( pTextObject )
    {
        CTextObject *pto = bClone ? (new CTextObject( pTextObject )) : pTextObject;
        if ( pto )
        {
            pto->m_iIndex = nIndex;
            m_arrTextObjects.Add( pto );
	        return true;
        }
    }
    else
    {
        m_arrTextObjects.Add( pTextObject );
	    return true;
    }
	return false;
}

BOOL CUndoAction::AddItem( int iType, CTextObject * pTextObject, int nIndex /* = -1 */, BOOL bClone /* = true */ )
{
    if ( !AllocBuffer() )
        return false;
	
    // if buffer is full free some space
    if ( m_nUndoItems == m_nUndoLevels )
    {
        delete m_pUndoData[0];
        if ( m_nUndoLevels > 0 )
            memmove( m_pUndoData, m_pUndoData+1, (m_nUndoLevels-1)*sizeof(m_pUndoData[0]) );
        m_nUndoItems--;
    }
	
    CUndoData *pUndo = new CUndoData(iType);
    m_pUndoData[m_nUndoItems] = pUndo;
    if ( pUndo )
    {
        if ( iType == UNDO_TEXT_MOVED && pTextObject )
        {
            pUndo->m_nObjectIndex = nIndex;
            pUndo->m_ptTextPos = pTextObject->m_attrib.pt;
            pUndo->m_iTextHeight = pTextObject->m_attrib.iHeight;
            pUndo->m_iTextWidth = pTextObject->m_attrib.iWidth;
        }
        else
        {
		    pUndo->AddTextObject( nIndex, pTextObject, bClone );
        }
		
        m_nUndoItems++;
        return true;
    }
    return false;
}

/**
 AddTextObject
 The function adds a text object to the last undo action.
 */
BOOL CUndoAction::AddTextObject( int iType, CTextObject * pTextObject, 
								int nIndex /* = -1 */, BOOL bClone /* = true */ )
{
    if ( m_nUndoItems > 0 && m_pUndoData && m_pUndoData[m_nUndoItems-1] && 
        m_pUndoData[m_nUndoItems-1]->m_iUndoType == iType )
    {
        m_pUndoData[m_nUndoItems-1]->AddTextObject( nIndex, pTextObject, bClone );
        return true;
    }
    else
    {
        // add new undo action
        return AddItem( iType, pTextObject, nIndex, bClone );
    }
}

void CUndoAction::FinishMoveTextUndo( CTextObject * pTextObject )
{
    if ( m_nUndoItems > 0 && pTextObject )
    {
        CUndoData *pUndo = m_pUndoData[m_nUndoItems-1];
		
        if ( pUndo->m_iUndoType == UNDO_TEXT_MOVED &&
            pUndo->m_ptTextPos.x == pTextObject->m_attrib.pt.x &&
            pUndo->m_ptTextPos.y == pTextObject->m_attrib.pt.y &&
            pUndo->m_iTextHeight == pTextObject->m_attrib.iHeight &&
            pUndo->m_iTextWidth == pTextObject->m_attrib.iWidth )
        {
            m_nUndoItems --;
            delete pUndo;
        }
    }
}

BOOL CUndoAction::CanUndoForCurrentObject( CTextObject *pTextObject )
{
    if ( m_nUndoItems > 0 && pTextObject )
    {
        CUndoData *pUndo = m_pUndoData[m_nUndoItems-1];
		
        if ( pUndo->m_iUndoType == UNDO_TEXT_ADDED || 
            pUndo->m_iUndoType == UNDO_TEXT_CHANGED )
        {
            CTextObject *pto = pUndo->m_arrTextObjects.GetAt(0);
			
            return pto && (
						   pto->m_attrib.pt.x != pTextObject->m_attrib.pt.x ||
						   pto->m_attrib.pt.y != pTextObject->m_attrib.pt.y ||
						   pto->m_attrib.iHeight != pTextObject->m_attrib.iHeight ||
						   pto->m_attrib.iWidth != pTextObject->m_attrib.iWidth ||
						   memcmp( &pto->m_attrib, &pTextObject->m_attrib, sizeof( pTextObject->m_attrib ) ) != 0 );
        }
    }
    return false;
}

#endif // TEXT_SUPPORT

#ifdef IMAGE_SUPPORT

BOOL CUndoAction::AddItem( int iType, CImageObject * pImageObject, int nIndex /* = -1 */, BOOL bClone /* = true */ )
{
    if ( !AllocBuffer() )
        return false;
	
    // if buffer is full free some space
    if ( m_nUndoItems == m_nUndoLevels )
    {
        delete m_pUndoData[0];
        if ( m_nUndoLevels > 0 )
            memmove( m_pUndoData, m_pUndoData+1, (m_nUndoLevels-1)*sizeof(m_pUndoData[0]) );
        m_nUndoItems--;
    }
	
    CUndoData *pUndo = new CUndoData(iType);
    m_pUndoData[m_nUndoItems] = pUndo;
    if ( pUndo )
    {
        if ( iType == UNDO_IMAGE_MOVED && pImageObject )
        {
            pUndo->m_nObjectIndex = nIndex;
            pUndo->m_ptImagePos = pImageObject->m_attrib.pt;
            pUndo->m_iImageHeight = pImageObject->m_attrib.iHeight;
            pUndo->m_iImageWidth = pImageObject->m_attrib.iWidth;
        }
        else
        {
		    pUndo->AddImageObject( nIndex, pImageObject, bClone );
        }
		
        m_nUndoItems++;
        return true;
    }
    return false;
}

BOOL CUndoAction::AddImageObject( int iType, CImageObject * pImageObject, 
								 int nIndex /* = -1 */, BOOL bClone /* = true */ )
{
    if ( m_nUndoItems > 0 && m_pUndoData && m_pUndoData[m_nUndoItems-1] && 
        m_pUndoData[m_nUndoItems-1]->m_iUndoType == iType )
    {
        m_pUndoData[m_nUndoItems-1]->AddImageObject( nIndex, pImageObject, bClone );
        return true;
    }
    else
    {
        // add new undo action
        return AddItem( iType, pImageObject, nIndex, bClone );
    }
}

BOOL CUndoData::AddImageObject( int nIndex, CImageObject *pImageObject, BOOL bClone )
{
	m_nObjectIndex = nIndex;

    if ( pImageObject )
    {
        CImageObject *pto = bClone ? (new CImageObject( pImageObject )) : pImageObject;
        if ( pto )
        {
            pto->m_iIndex = nIndex;
            m_arrImageObjects.Add( pto );
	        return true;
        }
    }
    else
    {
        m_arrImageObjects.Add( pImageObject );
	    return true;
    }
	return false;
}

#endif
