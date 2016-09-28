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


#include "hwr_sys.h"
#include "ams_mg.h"
#include "lowlevel.h"
#include "lk_code.h"
#include "def.h"
#include "langid.h"
#include "reco.h"
#include "calcmacr.h"

#define  DX_DOT_TO_STROKE             30
#define  DY_DOT_TO_STROKE             5
#define  DX_STROKE_TO_DOT             30
#define  DELTA_ANGLE_FROM_MIDPOINT    6
#define  DELTA_ANGLE_FROM_BEG         2
#define  NEW_ANGLE_ZONE_ENLARGE       5
#define  DX_BOTH_ARCS                 10
#define  MIN_SPECL_ELEMS_TO_SORT      3
#define  NUM_POINTS_TO_WRONG_CROSS    5

static _SHORT init_proc_XT_ST_CROSS( p_low_type low_data );
static _SHORT process_ZZ( p_low_type low_data );
static _SHORT process_AN( p_low_type low_data );
static _SHORT process_curves( p_low_type low_data );

/*********************************************************************/
/*****  the counting of line dimension                            ****/
/*********************************************************************/

_VOID DefLineThresholds( p_low_type pLowData )
{
    /* _SHORT lin_up,lin_down; */
    _SHORT z_little;
    _SHORT delta_STR_UP_LINE_UP, delta_up, one_ninth_baseline, two_ninth_baseline, delta_LINE_DN_STR_DN, delta_dn;

    /* ydist = pLowData->ymaxmax-pLowData->yminmin;*//* the word range */
    if ( pLowData->box.top < LIN_UP )
        pLowData->hght.y_US1_ = ONE_HALF(pLowData->box.top+LIN_UP);
    else
        pLowData->hght.y_US1_ = LIN_UP;
    delta_STR_UP_LINE_UP = STR_UP - pLowData->hght.y_US1_;
    pLowData->hght.y_US2_ = STR_UP - TWO_THIRD(delta_STR_UP_LINE_UP);
    pLowData->hght.y_UE1_ = STR_UP - ONE_THIRD(delta_STR_UP_LINE_UP);
    delta_up = ONE_NTH(delta_STR_UP_LINE_UP,6);
    pLowData->hght.y_UE2_ = STR_UP - delta_up;
    one_ninth_baseline = ONE_NTH(DY_STR,9);
    two_ninth_baseline = TWO(one_ninth_baseline);
    pLowData->hght.y_UI1_ = STR_UP + two_ninth_baseline;
    pLowData->hght.y_UI2_ = pLowData->hght.y_UI1_ + two_ninth_baseline;
    pLowData->hght.y_MD_ = pLowData->hght.y_UI2_ + one_ninth_baseline;

    pLowData->hght.y_DS2_ = ALEF;
    if ( pLowData->box.bottom > LIN_DOWN )
        pLowData->hght.y_DS1_ = ONE_HALF(pLowData->box.bottom+LIN_DOWN);
    else
        pLowData->hght.y_DS1_ = LIN_DOWN;
    delta_LINE_DN_STR_DN = pLowData->hght.y_DS1_ - STR_DOWN;
    pLowData->hght.y_DE2_ = STR_DOWN + TWO_THIRD(delta_LINE_DN_STR_DN);
    pLowData->hght.y_DE1_ = STR_DOWN + ONE_THIRD(delta_LINE_DN_STR_DN);
    delta_dn = ONE_NTH(delta_LINE_DN_STR_DN,6);
    pLowData->hght.y_DI2_ = STR_DOWN + delta_dn;
    pLowData->hght.y_DI1_ = pLowData->hght.y_MD_ + two_ninth_baseline;

    /* Sizes of small circle and break: */
    pLowData->o_little = (DY_STR * O_LITTLE) / 100;
    if ( pLowData->rc->lmod_border_used == LMOD_BORDER_NUMBER )
        pLowData->o_little = TWO_THIRD(pLowData->o_little);

    z_little = (DY_STR * Z_LITTLE) / 100;
    pLowData->z_little = z_little * z_little;

    if ( pLowData->rc->lmod_border_used == LMOD_BORDER_NUMBER )
        pLowData->z_little = ONE_HALF(pLowData->z_little);

} /*DefLineThresholds*/
/*********************************************************************/

_SHORT lk_begin( p_low_type low_data )
{
    p_SPECL specl = low_data->specl; /*  The list of special points on */
    /* the trajectory                 */

    DefLineThresholds( low_data );

    if ( Sort_specl( specl, low_data->len_specl ) != SUCCESS )
        return UNSUCCESS;

    if ( init_proc_XT_ST_CROSS( low_data ) != SUCCESS )
        return UNSUCCESS;
    if ( process_ZZ( low_data ) != SUCCESS )
        return UNSUCCESS;
    process_AN( low_data );
    process_curves( low_data );

    /* calculate step of writing using Zenia's function */
    low_data->StepSure = DefineWritingStep( low_data, &low_data->width_letter, _TRUE );

    return SUCCESS;
} /***** end of lk_begin *****/

/****************************************************************************/
/***** turn the strokes without crossings into points                   *****/
/***** transfer the crossings on the endings                            *****/
/****************************************************************************/
static _SHORT init_proc_XT_ST_CROSS( p_low_type low_data )
{
    p_SPECL specl = low_data->specl; /*  The list of special points on */
    /* the trajectory                 */
    p_SHORT x = low_data->x, /* x,y - co-ordinates             */
    y = low_data->y;
    p_SPECL cur, /* the index of the current elements  */
    prv, /*           of the previous elements */
    nxt, /*           of the next elements     */
    wcur, /*           of the working elements  */
    wrk; /*           of the working elements  */

    cur = specl;
    while ( cur != _NULL )
    {
        prv = cur->prev;
        nxt = cur->next;
        switch ( (_SHORT) cur->mark )
        {
            case DOT :
                /* if point is low and has a small y and a big x-strength,
                 then it will be a stroke  */
                if ( (y[MID_POINT(cur)] >= low_data->hght.y_UI2_) && (HWRAbs( x[cur->iend] - x[cur->ibeg] ) >= DX_DOT_TO_STROKE)
                        && (HWRAbs( y[cur->iend] - y[cur->ibeg] ) <= DY_DOT_TO_STROKE) )
                {
                    cur->mark = STROKE;
                    cur->ipoint0 = cur->ipoint1 = UNDEF;
                }
                break;
            case STROKE :
                /* if it`s stroke, then search for all HATCHs
                 which he has and transfer them to the stroke */
                wrk = cur;
                wrk = FindMarkRight( wrk, BEG );
                while ( wrk != _NULL )
                {
                    if ( (wrk->mark == HATCH) && FirstBelongsTo2nd( wrk->next, cur ) )
                    {
                        wcur = (wrk->next)->next;
                        MoveCrossing2ndAfter1st( cur, wrk );
                        nxt = wrk;
                        wrk = wcur;
                    }
                    else
                        wrk = wrk->next;
                }
                nxt = cur->next;
                /*if there are no HATCH after the stroke and it is short and
                 it is not high, then it will be a point*/
                if ( nxt != _NULL && (low_data->rc->rec_mode != RECM_FORMULA) && (nxt->mark != HATCH) && (y[MID_POINT(cur)] < STR_UP)
                        && (HWRAbs( x[cur->iend] - x[cur->ibeg] ) <= DX_STROKE_TO_DOT) )
                    cur->mark = DOT;
                break;
            case END :
                /* sort the crossings and the angle from the end */
                if ( prv->mark == CROSS || prv->mark == STICK || prv->mark == ANGLE || prv->mark == SHELF )
                {
                    wrk = prv->prev;
                    while ( wrk != _NULL && (wrk->mark == ANGLE || IsAnyCrossing( wrk ) || wrk->mark == SHELF) )
                        wrk = wrk->prev;
                    if ( wrk == _NULL )
                    {
                        return UNSUCCESS;
                    }
                    Move2ndAfter1st( prv, wrk );
                }
                break;
            case CROSS :
                if ( cur->other == CIRCLE_FIRST )
                {
                    wrk = cur;
                    wrk = FindMarkLeft( wrk, BEG );
                    while ( wrk->mark != END )
                    {
                        if ( wrk != cur && wrk != nxt && (wrk->mark == CROSS || wrk->mark == STICK) )
                        {
                            if ( FirstBelongsTo2nd( cur, wrk ) && FirstBelongsTo2nd( nxt, wrk->next ) )
                            {
                                DelCrossingFromSPECLList( cur );
                                break;
                            }
                            wrk = wrk->next;
                        }
                        wrk = wrk->next;
                    }
                }
                cur = cur->next;
                break;
        }
        cur = cur->next;
    }

    return SUCCESS;
} /***** end of init_proc_XT_ST_CROSS *****/

/****************************************************************************/
/*****                   processing of breaks                            ****/
/****************************************************************************/

static _SHORT process_ZZ( p_low_type low_data )
{
    p_SPECL specl = low_data->specl; /*  The list of special points on */
    /* the trajectory                 */
    p_SHORT x = low_data->x, /* x,y - co-ordinates             */
    y = low_data->y;
    p_SPECL cur, /* the index of the current elements  */
    prv, /*           of the previous elements */
    nxt; /*           of the next elements     */
    _LONG r; /* break`s dimension                  */
    _BOOL bIs_ST_XT = _FALSE;

    cur = specl;
    while ( cur != _NULL )
    {
        /* search for the beginnings of breaks */
        if ( cur->mark == BEG )
        {
            nxt = cur->next;
            SWT: switch ( (_SHORT) nxt->mark )
            {
                case MINW :
                    cur->code = _IU_;
                    break;
                case MAXW :
                    cur->code = _ID_;
                    break;
                case SHELF :
                    if ( cur->other & LETTER_H_STEP )
                        cur->code = _BSS_;
                    else
                        cur->code = _DF_;
                    break;
                case STROKE :
                    cur->code = _XT_;
                    break;
                case DOT :
                    cur->code = _ST_;
                    break;
                default :
                    cur->code = _NO_CODE;
                    while ( nxt != _NULL && (nxt->mark == ANGLE || IsAnyCrossing( nxt )) )
                        nxt = nxt->next;
                    Attach2ndTo1st( cur, nxt );
                    if ( nxt == _NULL || nxt->mark == END )
                    {
                        return UNSUCCESS;
                    }
                    goto SWT;
            }
            /* delete element after BEG and replace iend */
            cur->iend = nxt->iend;
            cur->other = nxt->other;
            cur->ipoint0 = nxt->ipoint0;
            if ( nxt->mark == MINW || nxt->mark == MAXW )
                cur->attr = HeightInLine( y[nxt->ipoint0], low_data );
            else
                cur->attr = MidPointHeight( cur, low_data );
            DelFromSPECLList( nxt );
            prv = cur->prev;

            /* New Andrey's breaks */
            if ( x[cur->ibeg - 1] != 0 )
            {
                cur->other |= MIN_MAX_CUTTED;
                if ( Is_IU_or_ID( prv ) )
                    prv->other |= MIN_MAX_CUTTED;
            }

            bIs_ST_XT = _FALSE;
            while ( prv != _NULL && (IsXTorST( prv ) || prv->mark == HATCH) ) /*CHE*/
            {
                prv = prv->prev;
                bIs_ST_XT = _TRUE;
            }

            /* if there is END before BEG */
            if ( cur != specl && prv != _NULL && prv->mark == END )
            {
                p_SPECL curNotST = cur; /*CHE*/

                while ( curNotST != _NULL && (IsXTorST( curNotST ) || curNotST->mark == HATCH || curNotST->mark == END) )
                {
                    curNotST = curNotST->next;
                    bIs_ST_XT = _TRUE;
                }
                if ( curNotST == _NULL || curNotST->mark != BEG )
                {
                    curNotST = cur;
                    if ( !IsXTorST( cur ) )
                        bIs_ST_XT = _FALSE;
                }
                /* count the distance between them */
                r = DistanceSquare( prv->iend, curNotST->ibeg, x, y );
                /* if it`s big, put here break element */
                if ((x[prv->iend + 1] != 0 || r > low_data->z_little
                    || (low_data->rc->low_mode & LMOD_SEPARATE_LET)
                    || (low_data->rc->low_mode & LMOD_SMALL_CAPS)
                    || bIs_ST_XT
                    || (Is_IU_or_ID( prv ) && Is_IU_or_ID( cur ) 
                        && prv->code == cur->code)))
                {
                    /* Here assumed: "(prv-1)" was deleted from list!!! */
                    (prv - 1)->mark = DROP;
                    if ( x[prv->iend + 1] != 0 )
                    {
                        (prv - 1)->code = _FF_;
                        (prv - 1)->other = FF_CUTTED;
                    }
                    else
                        (prv - 1)->code = _ZZZ_; /* its code _ZZZ_      */
                    (prv - 1)->attr = _MD_; /* its height - middle */
                    (prv - 1)->ibeg = prv->iend; /* its temporary       */
                    (prv - 1)->iend = curNotST->ibeg; /*    co-ordinates     */
                    Insert2ndAfter1st( prv, (prv - 1) ); /* put it between      */
                    /*     END and BEG:    */
                }
                else
                {
                    /* if the distance is too small */
                    if ( Is_IU_or_ID( prv ) && Is_IU_or_ID( cur ) )
                    {
                        _INT iBreak = cur->ibeg - 1;
                        nxt = cur->next;
                        while ( nxt != _NULL && nxt->mark != END )
                        //                      cur->iend>=nxt->ibeg-NUM_POINTS_TO_WRONG_CROSS)
                        {
                            if ( nxt->mark == CROSS || nxt->mark == STICK )
                            {
                                if ( nxt->ibeg > iBreak && (nxt->next)->iend < iBreak )
                                    DelCrossingFromSPECLList( nxt );
                                nxt = nxt->next;
                            }
                            nxt = nxt->next;
                        }
                        /* if codes aren't the same, delete END and BEG */
                        if ( prv->code != cur->code && !bIs_ST_XT ) /*CHE: temporarily */
                            DelThisAndNextFromSPECLList( prv );
                        /* otherwise delete break */
                        else if ( !bIs_ST_XT )
                        {
                            x[iBreak] = x[cur->ibeg];
                            y[iBreak] = y[cur->ibeg];
                            cur->ibeg = prv->ibeg;
                            if ( cur->code == _IU_ )
                                cur->mark = MINW;
                            else
                                cur->mark = MAXW;
                            DelFromSPECLList( prv );
                        }
                    }
                }
            }
        }
        else
        /* search for the break ends */
        if ( cur->mark == END )
        {
            prv = cur->prev;
            switch ( (_SHORT) prv->mark )
            {
                case MINW :
                    cur->code = _IU_;
                    break;
                case MAXW :
                    cur->code = _ID_;
                    break;
                case SHELF :
                    if ( cur->other & LETTER_H_STEP )
                        cur->code = _BSS_;
                    else
                        cur->code = _DF_;
                    break;
                case STROKE :
                    cur->code = _XT_;
                    break;
                case DOT :
                    cur->code = _ST_;
                    break;
                default :
                    cur->code = _NO_CODE;
            }

            if ( cur->code == _NO_CODE )
                DelFromSPECLList( cur );
            else
            {
                /* delete element before END and replace ibeg */
                cur->ibeg = prv->ibeg;
                cur->ipoint0 = prv->ipoint0;
                if ( prv->mark == MINW || prv->mark == MAXW )
                    ASSIGN_HEIGHT(cur,HeightInLine(y[prv->ipoint0],low_data));
                else
                    ASSIGN_HEIGHT(cur,MidPointHeight(cur,low_data));
                DelFromSPECLList( prv );
                prv = prv->prev;
            }
        }
        cur = cur->next;
    }

    return SUCCESS;
} /***** end of process_ZZ *****/

/****************************************************************************/
/*****         processing angles                                         ****/
/****************************************************************************/

static _SHORT process_AN( p_low_type low_data )
{
    _INT lang = low_data->rc->lang;
    p_SPECL specl = low_data->specl; /*  The list of special points on */
    /* the trajectory                 */
    p_SHORT x = low_data->x, /* x,y - co-ordinates             */
    y = low_data->y;
    p_SPECL cur, /* the index of the current elements  */
    prv, /*           of the previous elements */
    nxt, /*           of the next elements     */
    wrk, /*           of the working elements  */
    pCrossPrv = _NULL, pCrossNxt = _NULL;
    _SHORT nxt_beg, prv_end;
    _SHORT iVertex, iMidWrk;
    _SHORT dlt;
    _BOOL bCrossPrv, bCrossNxt, bIsPrv, bIsNxt;
    _SHORT NxtBegCr = 0, PrvEndCr = 0;

    for ( cur = specl; cur != _NULL; cur = cur->next )
    {
        if ( cur->mark != ANGLE )
            continue;
        /* mark angle as "new-born" - not restored yet */
        cur->other |= NOT_RESTORED;
        nxt = cur->next;
        prv = cur->prev;
        bCrossPrv = _FALSE;
        bCrossNxt = _FALSE;
        while ( prv->mark == CROSS && prv->other != 0 )
            prv = prv->prev;
        if ( IsAnyCrossing( prv ) )
        {
            prv = prv->prev;
            pCrossPrv = prv;
            bCrossPrv = _TRUE;
            while ( IsAnyCrossing( prv ) )
                prv = prv->prev;
        }
        if ( IsAnyCrossing( nxt ) )
        {
            pCrossNxt = nxt;
            bCrossNxt = _TRUE;
            while ( IsAnyCrossing( nxt ) )
                nxt = nxt->next;
        }
        if ( bCrossNxt )
            NxtBegCr = cur->attr ? pCrossNxt->ibeg - NEW_ANGLE_ZONE_ENLARGE : pCrossNxt->ibeg;
        if ( bCrossPrv )
            PrvEndCr = cur->attr ? pCrossPrv->iend + NEW_ANGLE_ZONE_ENLARGE : pCrossPrv->iend;
        nxt_beg = cur->attr ? nxt->ibeg - NEW_ANGLE_ZONE_ENLARGE : nxt->ibeg;
        prv_end = cur->attr ? prv->iend + NEW_ANGLE_ZONE_ENLARGE : prv->iend;

        /* if isolated angle */
        if ( ((bCrossNxt && cur->iend < NxtBegCr) || (!bCrossNxt && cur->iend < nxt_beg)) && ((bCrossPrv && cur->ibeg > PrvEndCr) || (!bCrossPrv
                && cur->ibeg > prv_end)) )
        {
            cur->other &= (~NOT_RESTORED);
            if ( (cur->other & _ANGLE_DIRECT_) == _ANGLE_RIGHT_ )
                cur->code = _ANr;
            else
                cur->code = _ANl;
            cur->attr = HeightInLine( y[MID_POINT(cur)], low_data );
            continue;
        }
        /* otherwise           */
        if ( cur->ibeg <= prv_end || cur->iend >= nxt_beg )
        {
            bIsPrv = bIsNxt = _FALSE;
            if ( cur->ibeg <= prv_end )
                bIsPrv = _TRUE;
            if ( cur->iend >= nxt_beg )
                bIsNxt = _TRUE;
            if ( lang == LANGUAGE_GERMAN || lang == LANGUAGE_FRENCH
                || lang == LANGUAGE_ITALIAN || lang == LANGUAGE_PORTUGUESE)
            {
                if((bIsPrv && (prv->other & LETTER_H_STEP))|| (bIsNxt && (nxt->other & LETTER_H_STEP))) 
                {
                    cur->other &= (~NOT_RESTORED);
                    if((cur->other & _ANGLE_DIRECT_)==_ANGLE_RIGHT_)
                        cur->code=_ANr;
                    else
                        cur->code=_ANl;
                    cur->attr=HeightInLine(y[MID_POINT(cur)],low_data);
                    /* Adjust sequence */
                    if(cur->code==_ANl && MID_POINT(prv)>cur->ipoint0)
                    {
                        DelFromSPECLList(prv);
                        Insert2ndAfter1st(cur,prv);
                    }
                    continue;
                }
            }
            if ( NO_ATTR_ASSIGNED(cur) || (bIsNxt && !bIsPrv && nxt->mark == END && nxt->code == _IU_ && x[nxt->iend] > x[nxt->ibeg])
                    || (!bIsPrv && bIsNxt && (nxt->other & LETTER_H_STEP)) || (!bIsNxt && bIsPrv && (prv->other & LETTER_H_STEP))
            )
            {
                iVertex = cur->ipoint0;
                if ( bIsPrv && !bIsNxt )
                {
                    iMidWrk = MID_POINT(prv);
                    if ( prv->mark == BEG && iMidWrk > iVertex )
                    {
                        DelFromSPECLList( cur );
                        continue;
                    }
                }
                if ( !bIsPrv && bIsNxt )
                {
                    iMidWrk = MID_POINT(nxt);
                    if ( nxt->mark == END && iMidWrk < iVertex )
                    {
                        DelFromSPECLList( cur );
                        continue;
                    }
                }
                if ( bIsPrv && bIsNxt && (prv->mark == BEG || nxt->mark == END) )
                {
                    DelFromSPECLList( cur );
                    continue;
                }
                if ( bIsPrv )
                    //             if(!bIsNxt)
                    wrk = prv;
                //             else
                //              wrk=(prv->iend-prv->ibeg>nxt->iend-nxt->ibeg ? prv : nxt);
                else
                    wrk = nxt;
                iMidWrk = MID_POINT(wrk);
                if ( wrk->mark == BEG || wrk->mark == END )
                    dlt = DELTA_ANGLE_FROM_BEG;
                else
                {
                    dlt = DELTA_ANGLE_FROM_MIDPOINT;
                    if ( wrk->mark == MINW )
                    {
                        if ( iVertex > iMidWrk )
                            iMidWrk = MEAN_OF(iVertex,wrk->ibeg);
                        else
                            iMidWrk = MEAN_OF(iVertex,wrk->iend);
                    }
                }
                if ( (((wrk->mark == MINW || wrk->mark == MAXW || wrk->mark == SHELF) && x[wrk->iend] > x[wrk->ibeg]) || wrk->mark == BEG
                        || wrk->mark == END) && HWRAbs( iMidWrk - iVertex ) > dlt )
                {
                    cur->other &= (~NOT_RESTORED);
                    if ( x[iVertex] < x[iMidWrk] )
                        cur->code = _ANl;
                    else
                        cur->code = _ANr;
                    /* Angle height         */
                    cur->attr = HeightInLine( y[iVertex], low_data );
                    /* Adjust sequence */
                    if ( wrk == prv && wrk->mark != BEG && iMidWrk > iVertex )
                    {
                        DelFromSPECLList( wrk );
                        Insert2ndAfter1st( cur, wrk );
                    }
                }
                else
                {
                    DelFromSPECLList( cur );
                    //               cur->other=1;
                }
            }
            else /* new angle */
            {
                DelFromSPECLList( cur );
                //            cur->other=1;
            }
        }
        /* deleted by crossings */
        else
        {
            _BOOL bIsStick = (bCrossNxt && pCrossNxt->mark == STICK) || (bCrossPrv && pCrossPrv->mark == STICK);
            DelFromSPECLList( cur );
            if ( bIsStick )
                cur->other |= WAS_DELETED_BY_STICK;
            else
                cur->other |= WAS_DELETED_BY_CROSS;
        }
    }

    return SUCCESS;
} /***** end of process_AN *****/

/****************************************************************************/
/*****         processing curves                                         ****/
/****************************************************************************/

static _SHORT process_curves( p_low_type low_data )
{
    p_SPECL specl = low_data->specl; /*  The list of special points on */
    /* the trajectory                 */
    p_SHORT x = low_data->x, y = low_data->y; /* x,y - co-ordinates                 */
    p_SPECL cur; /* the index of the current elements  */
    /*CHE: not used var.:   p_SPECL prv; */
    _SHORT iExtr; /* counters                           */
    _LONG coss;

    cur = specl->next;
    while ( cur != _NULL )
    {
        /* prv=cur->prev; */
        switch ( (_SHORT) cur->mark )
        {
            case MINW :
                /*arc position */
                cur->attr = HeightInLine( y[cur->ipoint0], low_data );
                /* find direction of arc */
                if ( x[cur->ibeg] < x[cur->iend] )
                    SET_CLOCKWISE(cur);
                else
                    SET_COUNTERCLOCKWISE(cur);
                /* search for extremum points */
                iExtr = extremum( cur->mark, cur->ibeg, cur->iend, y );
                /* cos of arc curve */
                coss = cos_vect( iExtr, cur->ibeg, iExtr, cur->iend, x, y );
                /* narrow curve will be stick */
                if ( coss >= DUGCOS )
                    cur->code = _IU_;
                else
                    cur->code = _UU_;
                break;
                
            case MAXW :
                /* arc position */
                cur->attr = HeightInLine( y[cur->ipoint0], low_data );
                /* find direction of arc */
                if ( x[cur->ibeg] > x[cur->iend] )
                    SET_CLOCKWISE(cur);
                else
                    SET_COUNTERCLOCKWISE(cur);
                /* search for extremum point */
                iExtr = extremum( cur->mark, cur->ibeg, cur->iend, y );
                /* cos of arc curve */
                coss = cos_vect( iExtr, cur->ibeg, iExtr, cur->iend, x, y );
                /* narrow curve will be stick */
                if ( coss >= DUGCOS )
                    cur->code = _ID_;
                else
                    cur->code = _UD_;

                break;
                
            case SHELF :
                if ( cur->other & LETTER_H_STEP )
                {
                    cur->code = _BSS_;
                    cur->attr = _MD_;
                    cur->ipoint0 = MID_POINT(cur); /* here it was real beg of BSS */
                }
                else
                    cur->code = _DF_;
                cur->attr = HeightInLine( y[MID_POINT(cur)], low_data ); /* AYV */
                break;
        }
        cur = cur->next;
    }
    return SUCCESS;
} /***** end of process_curves *****/

/****************************************************************************/
/*****     find extremums on arcs                                       *****/
/****************************************************************************/

_SHORT extremum( _UCHAR mark, _SHORT beg, _SHORT end, p_SHORT y )
{
    _SHORT i;
    _SHORT iExtr = beg;
    _SHORT yExtr = y[iExtr];

    switch ( (_SHORT) mark )
    {
        case MINW :
            for ( i = beg; i <= end; i++ )
                if ( y[i] < yExtr )
                {
                    yExtr = y[i];
                    iExtr = i;
                }
            break;
        case MAXW :
            for ( i = beg; i <= end; i++ )
                if ( y[i] > yExtr )
                {
                    yExtr = y[i];
                    iExtr = i;
                }
            break;
        default :
            return (0);
    }

    /* Find the middle point of the extr.plato: */

    for ( i = iExtr; i <= end; i++ )
        if ( y[i] != yExtr )
            break;
    return MEAN_OF(iExtr,i);

} /***** end of extremum *****/
/****************************************************************************/

/****************************************************************************/
/*****           the sorting elements by the time                        ****/
/****************************************************************************/

#define IsAnyExtremum(pEl)  (   REF(pEl)->mark==MINW     \
                             || REF(pEl)->mark==MINN     \
                             || REF(pEl)->mark==MAXW     \
                             || REF(pEl)->mark==MAXN     \
                             || REF(pEl)->mark==_MINX    \
                             || REF(pEl)->mark==_MAXX    \
                             || REF(pEl)->mark==MINXY    \
                             || REF(pEl)->mark==MAXXY    \
                             || REF(pEl)->mark==MINYX    \
                             || REF(pEl)->mark==MAXYX    \
                            )

_SHORT Sort_specl( p_SPECL specl, _SHORT len_specl )
{
    p_SPECL cur, /* index on the current element     */
    prv, /*        on the previous element   */
    nxt, /*        on the next element       */
    wrk = _NULL, /*        on the working element    */
    wcur; /*        on the working element    */
    _SHORT i; /* counters                         */
    _SHORT dnummin;
    _BOOL bCrossingCur;
    _LONG MaxIter, NumIter;

    if ( len_specl < MIN_SPECL_ELEMS_TO_SORT )
        return SUCCESS;

    /* Bubble-sort depending on "ibeg": */
    MaxIter = len_specl * len_specl;
    NumIter = 0;
    for ( i = 1; i <= len_specl; i++ )
    {
        cur = specl->next;
        while ( cur->next != _NULL )
        {
            nxt = cur->next;
            bCrossingCur = IsAnyCrossing( cur );
            if ( bCrossingCur )
            {
                if ( nxt->next == _NULL )
                    break;
                nxt = nxt->next;
            }
            if ( cur->ibeg < nxt->ibeg )
            {
                if ( bCrossingCur )
                    cur = cur->next;
                cur = cur->next;
            }
            else
            {
                if ( bCrossingCur )
                    DelCrossingFromSPECLList( cur );
                else
                    DelFromSPECLList( cur );
                if ( IsAnyCrossing( nxt ) )
                    nxt = nxt->next;
                if ( bCrossingCur )
                    InsertCrossing2ndAfter1st( nxt, cur );
                else
                    Insert2ndAfter1st( nxt, cur );
                NumIter++;
                if ( NumIter > MaxIter )
                {
                    return UNSUCCESS;
                }
            }
        }
    }

    cur = specl->next;
    NumIter = 0;
    while ( cur != _NULL )
    {
        prv = cur->prev;
        nxt = cur->next;
        if ( cur->mark == BEG )
        {
            if ( IsAnyCrossing( prv ) )
                prv = prv->prev;
            while ( prv != _NULL && prv->ibeg == cur->ibeg )
            {
                prv = prv->prev;
                if ( IsAnyCrossing( prv ) )
                    prv = prv->prev;
            }
            if ( prv == _NULL )
            {
                return UNSUCCESS;
            }
            if ( IsAnyCrossing( prv ) )
                prv = prv->next;
            if ( prv != cur->prev )
            {
                Move2ndAfter1st( prv, cur );
                NumIter++;
                if ( NumIter > MaxIter )
                {
                    return UNSUCCESS;
                }
            }
            dnummin = ALEF;
            wcur = cur->next;
            if ( wcur == _NULL )
                break;
            while ( wcur != _NULL && wcur->ibeg == cur->ibeg )
            {
                if ( (IsAnyExtremum(wcur) || wcur->mark == STROKE || wcur->mark == DOT) && dnummin > (wcur->iend - wcur->ibeg) )
                {
                    dnummin = wcur->iend - wcur->ibeg;
                    wrk = wcur;
                }
                if ( IsAnyCrossing( wcur ) )
                    wcur = wcur->next;
                wcur = wcur->next;
            }
            if ( wrk == _NULL )
            {
                return UNSUCCESS;
            }
            if ( wrk != cur->next )
            {
                Move2ndAfter1st( cur, wrk );
                NumIter++;
                if ( NumIter > MaxIter )
                {
                    return UNSUCCESS;
                }
            }
        }
        if ( cur->mark == END )
        {
            nxt = cur->next;
            if ( nxt == _NULL )
                break;
            while ( nxt != _NULL && nxt->iend == cur->iend )
            {
                if ( IsAnyCrossing( nxt ) )
                    nxt = nxt->next;
                wrk = nxt;
                nxt = nxt->next;
            }
            if ( nxt == _NULL || nxt != cur->next )
            {
                Move2ndAfter1st( wrk, cur );
                NumIter++;
                if ( NumIter > MaxIter )
                {
                    return UNSUCCESS;
                }
                if ( nxt != _NULL )
                    Attach2ndTo1st( cur, nxt );
                cur = (cur->prev)->prev;
            }
        }
        cur = cur->next;
    }

    /* printing of the sorted succession of elements */
    return SUCCESS;
} /***** end of Sort_specl *****/

/****************************************************************************/
/*****  Delete wrong elements from the SPECL                             ****/
/****************************************************************************/

#define   MIN_SPECL_LEN_TO_CLEAR  4

_SHORT Clear_specl( p_SPECL specl, _SHORT len_specl )
{
    p_SPECL cur, /* index of the current element     */
    nxt; /*       of the next element        */
    _BOOL bIsBEG, bIsEND, bIsBEG_END;

    if ( len_specl < MIN_SPECL_LEN_TO_CLEAR )
    {
        return UNSUCCESS;
    }

    cur = specl->next;
    while ( cur != _NULL )
    {
        nxt = cur->next;
        if ( nxt == _NULL )
            break;
        if ( cur->mark == BEG && nxt->mark == END )
        {
            DelThisAndNextFromSPECLList( cur );
            cur = cur->next;
        }
        cur = cur->next;
    }

    cur = specl->next;
    bIsBEG = bIsEND = bIsBEG_END = _FALSE;
    while ( cur != _NULL )
    {
        if ( cur->mark == BEG )
        {
            if ( !bIsBEG ) /* BEG with END */
            {
                bIsBEG = _TRUE;
                bIsBEG_END = _FALSE;
            }
            else /* BEG without END */
            {
                bIsBEG_END = _FALSE;
                break;
            }
        }
        if ( cur->mark == END )
        {
            if ( bIsBEG ) /* END with BEG */
                bIsEND = _TRUE;
            else /* END without BEG */
            {
                bIsBEG_END = _FALSE;
                break;
            }
        }
        if ( bIsBEG && bIsEND )
        {
            bIsBEG_END = _TRUE;
            bIsBEG = bIsEND = _FALSE;
        }
        cur = cur->next;
    }

    if ( !bIsBEG_END )
    {
        return UNSUCCESS;
    }

    return SUCCESS;
} /***** end of Clear_specl *****/

/****************************************************************************/
/*****  Get last index in SPECL                                          ****/
/****************************************************************************/

_SHORT get_last_in_specl( p_low_type low_data )
{
    p_SPECL     specl = low_data->specl;
    _SHORT      len_specl = low_data->len_specl;
    _SHORT      i;
    p_SPECL     cur = specl;
    
    while ( cur->next != _NULL )
        cur = cur->next;
    for ( i = 0; i < len_specl; i++ )
    {
        if ( (p_SPECL) &specl[i] == cur )
        {
            low_data->LastSpeclIndex = i;
            return SUCCESS;
        }
    }
    return UNSUCCESS;
} /***** end of get_last_in_specl *****/

#endif //#ifndef LSTRIP

