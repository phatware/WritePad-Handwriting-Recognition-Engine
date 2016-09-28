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

#ifndef LSTRIP

#include "ams_mg.h"
#include "hwr_sys.h"
#include "lowlevel.h"
#include "lk_code.h"
#include "def.h"
#include "calcmacr.h"

#define CR_TO_BE_STICK                5
#define CR_TO_BE_STICK_IF_UNKNOWN    20
#define TINY_CR                      10
#define SMALL_CR                     15
#define MED_CR                       20
#define LARGE_CR                     30
#define MIN_DL_DR                    10
#define CR_TO_BE_SMALL_Z_S           25

static _LINK_TYPE CalculateStickOrArc( p_SAD pSAD );
static _LINK_TYPE CalculateLinkLikeSZ( p_SAD pSAD, _INT dy );
static _LINK_TYPE CalculateLinkWithoutSDS( p_low_type low_data, p_SPECL pXr, p_SPECL nxt );
static _LINK_TYPE GetCurveLink( _SHORT cr, _BOOL bDirCW );
static _LINK_TYPE GetMovementLink( _UCHAR code );
static _SHORT RecountSlantInSDS( _SHORT slant, _SHORT slope );

/****************************************************************************/
/***** This program determines link between this and next xr             ****/
/****************************************************************************/

_SHORT GetLinkBetweenThisAndNextXr( p_low_type low_data, p_SPECL pXr, xrd_el_type _PTR xrd_elem )
{
    p_SPECL nxt = pXr->next;
    _LINK_TYPE Link;

    while ( nxt != _NULL && IsXTorST( nxt ) )
        nxt = nxt->next;
    /* no link for "break" elements */
    if ( IsAnyBreak( pXr ) || NULL_or_ZZ_this( nxt ) || IsXTorST( pXr ) || 
        (IsAnyAngle( pXr ) && CrossInTime( pXr, nxt ) && !IsAnyMovement( nxt ) && nxt->code != _DF_) )
    {
        Link = LINK_LINE; // LINK_UNKNOWN;
        goto ret;
    }

    if ( IsAnyMovement( pXr ) )
    {
        Link = GetMovementLink( pXr->code );
        goto ret;
    }

    if ( pXr->code == _DF_ )
    {
        Link = LINK_LINE;
        goto ret;
    }

    /* skip angles, crossing in time with current elements, to determine
     link between two "strong" elements */
    while ( nxt != _NULL && IsAnyAngle( nxt ) && CrossInTime( pXr, nxt ) )
        nxt = nxt->next;
    if ( nxt == _NULL )
    {
        Link = LINK_LINE; // LINK_UNKNOWN;
        goto ret;
    }

    /* for elements, following by "shelfs", determine link based on "shelf" */
    if ( nxt->code == _DF_ )
    {
        Link = LINK_LINE;
        goto ret;
    }

    if ( !IsAnyAngle( pXr ) )
    {
        p_SPECL pAfterAn = nxt;
        /* try to find "movement" element after angles */
        while ( pAfterAn != _NULL && IsAnyAngle( pAfterAn ) )
            pAfterAn = pAfterAn->next;
        /* for elements, following by special "movement" elements
         determine link based on this element */
        if ( pAfterAn != _NULL && (Link = GetMovementLink( pAfterAn->code )) != LINK_UNKNOWN )
            goto ret;
    }
    else if ( IsAnyMovement( nxt ) )
        nxt = nxt->next;

    Link = CalculateLinkWithoutSDS( low_data, pXr, nxt );

    ret:
    XASSIGN_XLINK(xrd_elem,(_UCHAR)Link)
    return ((_SHORT) Link);

} /* end of GetLinkBetweenThisAndNextXr */

/****************************************************************************/
/***** This program calculates special "movement" links                  ****/
/****************************************************************************/
_LINK_TYPE GetMovementLink( _UCHAR code )
{
    _LINK_TYPE Link = LINK_UNKNOWN;

    switch ( code )
    {
        case _TZ_ :
            Link = LINK_HZ_LIKE;
            break;
        case _TS_ :
            Link = LINK_HS_LIKE;
            break;
        case _BR_ :
            Link = LINK_HCR_CW;
            break;
        case _BL_ :
            Link = LINK_HCR_CCW;
            break;
    }

    return Link;

} /* end of GetMovementLink */

/****************************************************************************/
/***** This program calculates curvature of link                         ****/
/****************************************************************************/
_LINK_TYPE GetCurveLink( _SHORT cr, _BOOL bDirCW )
{
    _LINK_TYPE Link;

    if ( cr < TINY_CR )
        if ( bDirCW )
            Link = LINK_TCR_CW;
        else
            Link = LINK_TCR_CCW;
    else if ( cr < SMALL_CR )
        if ( bDirCW )
            Link = LINK_SCR_CW;
        else
            Link = LINK_SCR_CCW;
    else if ( cr < MED_CR )
        if ( bDirCW )
            Link = LINK_MCR_CW;
        else
            Link = LINK_MCR_CCW;
    else if ( cr < LARGE_CR )
        if ( bDirCW )
            Link = LINK_LCR_CW;
        else
            Link = LINK_LCR_CCW;
    else if ( bDirCW )
        Link = LINK_HCR_CW;
    else
        Link = LINK_HCR_CCW;

    return Link;

} /* end of GetCurveLink */

/****************************************************************************/
/***** This program calculates link, supposed to be Stick or Arc         ****/
/****************************************************************************/

static _LINK_TYPE CalculateStickOrArc( p_SAD pSAD )
{
    _LINK_TYPE Link = LINK_UNKNOWN;
    _SHORT dL = pSAD->dL, dR = pSAD->dR;

    /* to avoid small s-like and z-like links */
    if ( dL > FOUR(dR) && dR < MIN_DL_DR )
        dR = pSAD->dR = 0;
    else if ( dR > FOUR(dL) && dL < MIN_DL_DR )
        dL = pSAD->dL = 0;

    /* if curvature is small, it will be stick */
    if ( pSAD->cr < CR_TO_BE_STICK || (pSAD->cr < CR_TO_BE_STICK_IF_UNKNOWN && dL != 0 && dR != 0) )
        Link = LINK_LINE;
    else /* some kind of arc */
    {
        /* arcs LEFT, RIGHT and so on */
        if ( pSAD->dL == 0 || pSAD->dR == 0 )
        {
            if ( pSAD->dL != 0 )
                Link = GetCurveLink( pSAD->cr, _TRUE );
            else
                /* dR!=0 */
                Link = GetCurveLink( pSAD->cr, _FALSE );
        }
    }

    return (Link);

} /* end of CalculateStickOrArc */

/****************************************************************************/
/***** This program calculates link, supposed to be S or Z-like arc      ****/
/****************************************************************************/
static _LINK_TYPE CalculateLinkLikeSZ( p_SAD pSAD, _INT dy )
{
    _LINK_TYPE  Link;
    _SHORT      cr = pSAD->cr;

    if ( dy < 0 )
    {
        if ( pSAD->iLmax > pSAD->iRmax )
        {
            if ( cr <= CR_TO_BE_SMALL_Z_S )
                Link = LINK_S_LIKE;
            else
                Link = LINK_HS_LIKE;
        }
        else if ( cr <= CR_TO_BE_SMALL_Z_S )
            Link = LINK_Z_LIKE;
        else
            Link = LINK_HZ_LIKE;
    }
    else if ( pSAD->iLmax > pSAD->iRmax )
    {
        if ( cr <= CR_TO_BE_SMALL_Z_S )
            Link = LINK_S_LIKE;
        else
            Link = LINK_HS_LIKE;
    }
    else if ( cr <= CR_TO_BE_SMALL_Z_S )
        Link = LINK_Z_LIKE;
    else
        Link = LINK_HZ_LIKE;

    return (Link);

} /* end of CalculateLinkLikeSZ */

/****************************************************************************/
/***** This program calculates link, based on xr-elements only           ****/
/****************************************************************************/

#define ELEMENT_WITH_DIRECTION(pEl) ((pEl)->code==_GU_  || (pEl)->code==_GD_  || \
                                     (pEl)->code==_UU_  || (pEl)->code==_UD_  || \
                                     (pEl)->code==_UUC_ || (pEl)->code==_UDC_ || \
                                     (Is_IU_or_ID(pEl) && (pEl)->mark==CROSS) || \
                                     IsAnyArcWithTail(pEl)                       \
                                    )
#define MIN_POINTS_TO_BE_LINE 8

static _LINK_TYPE CalculateLinkWithoutSDS( p_low_type low_data, p_SPECL pXr, p_SPECL nxt )
{
    _LINK_TYPE Link;
    p_SHORT x = low_data->x, y = low_data->y;
    _SDS NewSDS;
    p_SDS pSDS = &NewSDS;
    p_SAD pSAD = &(pSDS->des);
    _SHORT xd_AND, yd_AND;
    _INT dy = 0;

    if ( (ELEMENT_WITH_DIRECTION(pXr)) && (ELEMENT_WITH_DIRECTION(nxt)) &&
    		(CIRCLE_DIR(pXr) == CIRCLE_DIR(nxt) || Is_IU_or_ID( pXr )
            || Is_IU_or_ID( nxt )) )
    {
        pSDS->ibeg = pXr->ipoint0; //MID_POINT(pXr);
        pSDS->iend = nxt->ipoint0; //MID_POINT(nxt);
    }
    else
    {
#if 0
        if(pXr->mark==STICK)
        pSDS->ibeg = pXr->ipoint0;
        else
        if(pXr->ipoint0!=UNDEF && pXr->ipoint0!=0)
        pSDS->ibeg = MEAN_OF(pXr->ipoint0,pXr->iend);
        else
        pSDS->ibeg = ONE_THIRD( pXr->ibeg + TWO(pXr->iend) ); //CHE  // MID_POINT(pXr);
        if(nxt->mark==STICK)
        pSDS->iend = nxt->ipoint0;
        else
        if(nxt->ipoint0!=UNDEF && nxt->ipoint0!=0)
        pSDS->iend = MEAN_OF(nxt->ibeg,nxt->ipoint0);
        else
        pSDS->iend = ONE_THIRD( TWO(nxt->ibeg) + nxt->iend ); // MID_POINT(nxt);
#endif
        // forget all smart stuff
        if ( pXr->ipoint0 != UNDEF && pXr->ipoint0 != 0 && pXr->code != _BSS_ )
            pSDS->ibeg = pXr->ipoint0;
        else
            pSDS->ibeg = (_SHORT) MID_POINT(pXr);
        if ( nxt->ipoint0 != UNDEF && nxt->ipoint0 != 0 && nxt->code != _BSS_ )
            pSDS->iend = nxt->ipoint0;
        else
            pSDS->iend = (_SHORT) MID_POINT(nxt);
    }

    if ( pSDS->iend - pSDS->ibeg <= MIN_POINTS_TO_BE_LINE )
        Link = LINK_LINE;
    else
    {

        if ( pSDS->ibeg >= 0 && pSDS->ibeg < low_data->ii && pSDS->iend >= 0 && pSDS->iend < low_data->ii )
        {
            dy = y[pSDS->iend] - y[pSDS->ibeg];
            iMostFarDoubleSide( x, y, pSDS, &xd_AND, &yd_AND, _FALSE );
            // pSAD->a=RecountSlantInSDS(pSAD->a,low_data->slope);
            if ( pSAD->dR != 0 && pSAD->dL != 0 && pSDS->mark == SDS_ISOLATE )
            {
                if ( pSAD->iLmax <= pSDS->ibeg + 1 || pSAD->iLmax >= pSDS->iend - 1 )
                    pSAD->dL = 0;
                else if ( pSAD->iRmax <= pSDS->ibeg + 1 || pSAD->iRmax >= pSDS->iend - 1 )
                    pSAD->dR = 0;
                else
                {
                    _LONG dLCos = cos_vect( pSAD->iLmax - 2, pSAD->iLmax + 2, pSDS->ibeg, pSDS->iend, x, y );
                    _LONG dRCos = cos_vect( pSAD->iRmax - 2, pSAD->iRmax + 2, pSDS->ibeg, pSDS->iend, x, y );

                    if ( dLCos > dRCos )
                        pSAD->dR = 0;
                    else
                        pSAD->dL = 0;
                }
            }

            Link = CalculateStickOrArc( pSAD );
        }
        else
            Link = LINK_UNKNOWN; // MS Fix

        if ( Link == LINK_UNKNOWN )
            Link = CalculateLinkLikeSZ( pSAD, dy );
    }

    return (Link);

} /* end of CalculateLinkWithoutSDS */

/****************************************************************************/
/***** This program recounts ibeg and iend of SDS according to back index ***/
/****************************************************************************/

_SHORT RecountBegEndInSDS( p_low_type low_data )
{
    _SHORT  i, NumPoints = low_data->ii;
    p_SHORT y = low_data->y, IndBack = low_data->buffers[2].ptr;
    p_SDS   pSDS = low_data->p_cSDS->pSDS;
    _SHORT  lSDS = low_data->p_cSDS->lenSDS;

    if ( pSDS == _NULL )
    {
        return UNSUCCESS;
    }
    for ( i = 0; i < lSDS; i++ )
    {
        pSDS[i].ibeg = NewIndex( IndBack, y, pSDS[i].ibeg, NumPoints, _FIRST );
        pSDS[i].iend = NewIndex( IndBack, y, pSDS[i].iend, NumPoints, _LAST );
        pSDS[i].des.iLmax = NewIndex( IndBack, y, pSDS[i].des.iLmax, NumPoints, _MEAD );
        pSDS[i].des.iRmax = NewIndex( IndBack, y, pSDS[i].des.iRmax, NumPoints, _MEAD );
        pSDS[i].des.imax = NewIndex( IndBack, y, pSDS[i].des.imax, NumPoints, _MEAD );
        pSDS[i].des.a = RecountSlantInSDS( pSDS[i].des.a, low_data->slope );
    }

    return SUCCESS;

} /* end of RecountBegEndInSDS */
/****************************************************************************/
/*** This program recounts slant of the SDS-element according to the slope **/
/****************************************************************************/
static _SHORT RecountSlantInSDS( _SHORT slant, _SHORT slope )
{
    _INT sl, dx;

    if ( slope == 0 )
        sl = slant;
    else
    {
        if ( slant == ALEF )
            sl = 100 * 100 / slope;
        else if ( (dx = 100 - SlopeShiftDx( -slant, slope )) == 0 )
            sl = ALEF;
        else
            sl = (_SHORT) ((_LONG) slant * 100 / dx);
    }

    return ((_SHORT) sl);

} /* end of RecountSlantInSDS */

#endif //#ifndef LSTRIP

