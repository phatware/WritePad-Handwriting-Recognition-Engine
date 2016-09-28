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

#include "hwr_sys.h"
#include "ams_mg.h"
#include "div_let.h"
#include "lowlevel.h"
#include "xr_names.h"
#include "reco.h"
#include "langid.h"

/*************************************************************************/
/* This function gets information about beginning and ending each letter */
/* in the answer (I mean numbers of points in corresponding trajectory). */
/* Input: xrdata and rec_words for this variant of answer.               */
/* Output: pointer on structure with all nessesary information           */
/* Return code: SUCCESS - everything is OK, after using data it's        */
/* nessesary to free memory in output structure                          */
/*              UNSUCCESS - memory problems, function can't work         */
/*************************************************************************/

static _BOOL X_IsLikeXTST(xrd_el_type _PTR  pxrd, _INT lang )
{
	if (lang == LANGUAGE_GERMAN || lang == LANGUAGE_NORWEGIAN
		|| lang == LANGUAGE_DANISH || lang == LANGUAGE_SWEDISH
		|| lang == LANGUAGE_DUTCH || lang == LANGUAGE_FINNISH)
    {
		return ((pxrd)->xr.type==X_ST || (pxrd)->xr.type==X_XT
                || (pxrd)->xr.type==X_XT_ST || (pxrd)->xr.type==X_UMLAUT);
	}
    else if (lang == LANGUAGE_FRENCH)
    {
		return ((pxrd)->xr.type==X_ST || (pxrd)->xr.type==X_XT
                || (pxrd)->xr.type==X_XT_ST || (pxrd)->xr.type==X_CEDILLA
                || (pxrd)->xr.type==X_UMLAUT);
	}
    else if (lang == LANGUAGE_ITALIAN)
    {
		return ((pxrd)->xr.type==X_ST || (pxrd)->xr.type==X_XT
                || (pxrd)->xr.type==X_XT_ST);
	}
    else if (lang == LANGUAGE_SPANISH)
    {
		return ((pxrd)->xr.type==X_ST || (pxrd)->xr.type==X_XT
                || (pxrd)->xr.type==X_XT_ST || (pxrd)->xr.type==X_UMLAUT);
	}
    
    else if (lang == LANGUAGE_PORTUGUESE || lang == LANGUAGE_PORTUGUESEB)
    {
		return ((pxrd)->xr.type==X_ST || (pxrd)->xr.type==X_XT
                || (pxrd)->xr.type==X_XT_ST || (pxrd)->xr.type==X_CEDILLA
                || (pxrd)->xr.type==X_UMLAUT);
	}
    else
    {
		return ((pxrd)->xr.type==X_ST || (pxrd)->xr.type==X_XT
                || (pxrd)->xr.type==X_XT_ST);
	}
}

/*  Auxiliary function: */

#ifdef _OS_LINUX
static  _VOID  UpdateBegEnd ( pPart_of_letter pCurPart, xrd_el_type _PTR  xrd );
#else // _OS_LINUX

static  _VOID  UpdateBegEnd ( pPart_of_letter pCurPart, xrdata_type _PTR  xrd );
#endif // _OS_LINUX

static  _VOID  UpdateBegEnd ( pPart_of_letter pCurPart, xrd_el_type _PTR  xrd )
{
    if  ( xrd->begpoint < pCurPart->ibeg )
        pCurPart->ibeg = xrd->begpoint;
    if  ( xrd->endpoint > pCurPart->iend )
        pCurPart->iend = xrd->endpoint;
}

#if !defined _EMBEDDED_DEVICE || defined PSR_DLL

_SHORT connect_trajectory_and_answers(xrd_el_type _PTR xrdata, rec_w_type _PTR rec_word,
                                      pOsokin_output pOutputData)
{ _SHORT i,j,ibeg_xr,iend_xr,num_parts,all_parts;
    pPart_of_letter pParts;
    //  _WORD memory=w_lim*MAX_PARTS_IN_LETTER*sizeof(Part_of_letter);
    
    if  ( rec_word[0].linp[0] == 0 )  /* i.e. no letters info */
        return  UNSUCCESS;
    
    //  if((pOutputData->pParts_of_letters=(pPart_of_letter)HWRMemoryAlloc(memory))==_NULL)
    //    goto err;
    pParts=pOutputData->Parts_of_letters;
    HWRMemSet((p_VOID)pParts,0,sizeof(pOutputData->Parts_of_letters));
    HWRMemSet((p_VOID)pOutputData->num_parts_in_letter,0,w_lim);
    
    /* i means number of letter in word */
    /* all_parts - number of all parts of all letters in word */
    for(iend_xr=0,all_parts=0,i=0;rec_word->word[i];i++)
    {
        /* num_parts - number of parts of given letter in word */
        /* ibeg_xr, iend_xr - number of 1-st and last xr_element in letter */
        ibeg_xr=iend_xr+1;
        iend_xr=ibeg_xr+rec_word->linp[i]-1;
        if(all_parts>=(w_lim-1)*MAX_PARTS_IN_LETTER)
            goto err;
        if(connect_trajectory_and_letter(xrdata,ibeg_xr,iend_xr, &num_parts,pParts, lang ) !=SUCCESS)
            goto err;
        pOutputData->num_parts_in_letter[i]=(_UCHAR)num_parts;
        pParts+=num_parts;
        all_parts+=num_parts;
    }
    
    /*Check result*/
    all_parts=0;
    for(i=0;rec_word->word[i]!=0;i++)
    {
        for(j=0;j < pOutputData->num_parts_in_letter[i];j++)
        {
            if(pOutputData->Parts_of_letters[all_parts].ibeg >
               pOutputData->Parts_of_letters[all_parts].iend)
                goto err;
            all_parts++;
        }
    }
    
    return SUCCESS;
    
err:
    return UNSUCCESS;
    
}

#endif // 0

/*************************************************************************/
/* This function gets information about the beginning and ending of the   */
/* single letter (I mean numbers of points in corresponding trajectory). */
/* Input: xrdata, index of xr_beg and xr_end for this letter             */
/* Output: pointer on structure with parts of letter and number of parts */
/* Return code: SUCCESS - everything is OK,                              */
/*              UNSUCCESS - memory problems, function can't work         */
/*************************************************************************/

_SHORT connect_trajectory_and_letter(xrd_el_type _PTR xrdata, _SHORT ibeg_xr, _SHORT iend_xr,
                                     p_SHORT n_parts,pPart_of_letter pParts, _INT lang )
{
    _SHORT j,num_parts;
    _BOOL bNewStroke;
    pPart_of_letter pCurPart = _NULL;
    xrd_el_type _PTR  xrd_j;
    
    for(num_parts=0,bNewStroke=_TRUE,j=ibeg_xr;j<=iend_xr;j++)
    {
        xrd_j = &xrdata[j];
        /* if break in letter - new stroke begins */
        if(X_IsBreak(xrd_j) || xrd_j->xr.type==X_ZN)
        {
            if(!bNewStroke)
                bNewStroke=_TRUE;
        }
        /* if XT or ST element - write this element as another
         part, but see coments CHE below */
        else if( X_IsLikeXTST( xrd_j, lang ) )
        {
            /* Check whether this XR is the doubled one: */
            if  ( j > ibeg_xr )
            {
                _INT  jPrev;
                for  ( jPrev=ibeg_xr;  jPrev<j;  jPrev++ )
                {
                    if  (   xrdata[jPrev].xr.type == xrd_j->xr.type
                         && xrdata[jPrev].begpoint == xrd_j->begpoint
                         && xrdata[jPrev].endpoint == xrd_j->endpoint )
                    {
                        break;
                    }
                }
                if  ( jPrev < j )  //i.e. this XR is the doubled one
                    continue;
            }
            
            if(!bNewStroke)
            {
                if ( X_IsLikeXTST(xrd_j - 1, lang ) )
                    bNewStroke = _TRUE;
            }
            pCurPart = &pParts[num_parts++];
            if(num_parts>MAX_PARTS_IN_LETTER)
                goto err;
            pCurPart->ibeg = xrd_j->begpoint;
            pCurPart->iend = xrd_j->endpoint;
            if  ( !bNewStroke )
            {
                /* Don't do new part if the _ST_(_XT_) appears
                 within codes sequence for one and the same stroke. */
                Part_of_letter  partTmp = *pCurPart;
                *pCurPart = pParts[num_parts-2];
                pParts[num_parts-2] = partTmp;
            }
        }
        /* if other xr_element and new stroke begins - write new
         part, if it's the old one - update beg and end */
        else
        {
            if(bNewStroke)
            {
                pCurPart = &pParts[num_parts++];
                if(num_parts>MAX_PARTS_IN_LETTER)
                    goto err;
                pCurPart->ibeg = xrd_j->begpoint;
                pCurPart->iend = xrd_j->endpoint;
                bNewStroke=_FALSE;
            }
            else
            {
                UpdateBegEnd( pCurPart, xrd_j );
            }
        }
    }
    
    *n_parts=num_parts;
    
    return SUCCESS;
    
err:
    return UNSUCCESS;
    
}
