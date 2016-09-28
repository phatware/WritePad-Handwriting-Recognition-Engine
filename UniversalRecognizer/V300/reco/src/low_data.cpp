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

#ifndef LSTRIP   //for vertigon2

#include "hwr_sys.h"
#include "ams_mg.h"
#include "def.h"
#include "lowlevel.h"
#include "calcmacr.h"
#include "sketch.h"
#include "arcs.h"

#include "reco.h"
#include "langid.h"
#include "langutil.h"

#define      KOEFF_BASE            10
#define      MIN_FILTER_KOEFF      ((_SHORT)2)
#define      EPS_Y_BORDER_LIMIT    ((_SHORT)2)

RECO_DATA_EXTERNAL CONSTS const1;


_VOID FillLowDataTrace( low_type _PTR pLowData, PS_point_type _PTR trace )
{
    _SHORT nPoints;
    
    nPoints = pLowData->ii = pLowData->rc->ii;
    trace_to_xy( pLowData->x, pLowData->y, nPoints, trace );
    if ( nPoints > 1 )
    {
        if ( pLowData->y[nPoints - 1] != BREAK )
            pLowData->y[nPoints - 1] = BREAK;
    }
    
    pLowData->p_trace = trace;
} /* end of FillLowDataTrace */

/*******************************************************************/

_VOID GetLowDataRect( low_type _PTR pLowData )
{
    size_cross (0,
                pLowData->ii-1,
                pLowData->x,
                pLowData->y,
                &pLowData->box);
} /* end of GetLowDataRect */

/*******************************************************************/

_BOOL PrepareLowData( low_type _PTR pLowData, PS_point_type _PTR trace, rc_type _PTR rc, p_SHORT _PTR pbuffer )
{
    _SHORT len_buf, num_buffers;
    _SHORT len_xy;
    
    // empty low level work structure
    HWRMemSet( (p_VOID) pLowData, 0, sizeof(low_type) );
    pLowData->rc = rc;
    
    // allocate memory for specl
    if ( !AllocSpecl( &pLowData->specl, SPECVAL ) )
    {
        goto ERR;
    }
    pLowData->nMaxLenSpecl = SPECVAL;
    
    /*  Define "len_xy" (it must be equal to "rc->ii" plus */
    /* probable addition in "ErrorProv"(for formulas) or   */
    /* other functions (as in Andrey's "FantomSt"):        */
    
    len_xy = MaxPointsGrown(
#ifdef  FORMULA
                            trace,
#endif  /*FORMULA*/
                            rc->ii );
    
    if ( len_xy > LOWBUF )
    {
        return _FALSE;
    }
    
    len_buf = LOWBUF;
    num_buffers = NUM_BUF; /* CHE: was "+2" - now lives in "len_xy" */
    
    // set required sizes of the data
    pLowData->nLenXYBuf = len_xy;
    pLowData->rmGrBord = N_GR_BORD;
    pLowData->rmAbsnum = N_ABSNUM;
    pLowData->iBegBlankGroups = ALEF;
    
    // allocate memory for data
    if ( LowAlloc( pbuffer, num_buffers, len_buf, pLowData ) != SUCCESS )
        goto ERR;
    
    return _TRUE;
    
ERR:
    low_dealloc( pbuffer );
    DeallocSpecl( &pLowData->specl );
    
    return _FALSE;
    
} /* end of PrepareLowData */

/*******************************************************************/
/*******************************************************************/
/*****  main program for low level processing                   ****/
/*******************************************************************/
/*******************************************************************/

_SHORT low_level( PS_point_type _PTR trace, xrdata_type _PTR xrdata, rc_type _PTR rc )
{
    low_type    low_data;
    _SHORT      retval = UNSUCCESS;
    p_SHORT     buffer = _NULL;
    _SDS_CONTROL cSDS;
    
    // trace;
    
    // scaling the trajectory depends on baseline
    if ( rc->ii < 3 )
    {
        // if input trace too short
        return UNSUCCESS;
    }
    
    // empty output structure
    xrdata->len = 0;
    
    // allocate low data buffers and initialize them,
    // copy trace to x and y arrays
    if ( !PrepareLowData( &low_data, trace, rc, &buffer ) )
        goto err;
    
    low_data.slope = low_data.rc->slope;
    
    // set work pointers for x and y arrays
    SetXYToInitial( &low_data );
    // copy trace to x and y arrays
    FillLowDataTrace( &low_data, trace );
    
    // calculate trace bounding rectangle
    GetLowDataRect( &low_data );
    
    if ( BaselineAndScale( &low_data ) != SUCCESS )
        goto err;
    if ( rc->low_mode & LMOD_BORDER_ONLY )
        goto FILL_RC;
    
#ifndef _WSBDLL // -- to allow removing of redundant code
    low_data.p_cSDS = &cSDS;
    if ( CreateSDS( &low_data, N_SDS ) == _FALSE )
        goto err;
    
    if ( AnalyzeLowData( &low_data, trace ) != SUCCESS )
        goto err;
    
    // at least fill output xrdata structure array from specl data
    if ( exchange( &low_data, xrdata ) != SUCCESS )
        goto err;
    
#endif // _WSBDLL
FILL_RC:
    
    retval = SUCCESS;
    
err:
#ifndef _WSBDLL
    DestroySDS( &low_data );
#endif
    
    low_dealloc( &buffer );
    DeallocSpecl( &low_data.specl );
    
    return retval;
    
} /*low_level*/

/* *************************************************************** */
/* * Performs baseline measurement and scales trace respectively * */
/* * This function assume that the LowData already initialized   * */
/* * and x, y arrays already filled with trace data              * */
/* *************************************************************** */
_INT BaselineAndScale( low_type _PTR pLowData )
{
    _INT lang = pLowData->rc->lang;
    _SHORT koeff;
    
    pLowData->rc->lmod_border_used = LMOD_NO_BORDER_DECISION;
    
    // calculate ratio between trace amplitude and fixed baseline
    koeff = (_SHORT) ((_LONG) DY_RECT(pLowData->box) * KOEFF_BASE / (STR_DOWN - LIN_UP));
    if ( koeff < MIN_FILTER_KOEFF )
        koeff = MIN_FILTER_KOEFF;
    
    // remove some garbage from x and y arrays
    Errorprov( pLowData );
    
    // normalizing of the trace from x and y array to working buffers
    if ( Filt( pLowData, koeff * const1.horda / KOEFF_BASE, NOABSENCE ) != SUCCESS )
        goto err;
    
    // level of the extremums determination
    pLowData->rc->stroka.extr_depth = const1.eps_y;
    
    // special baseline could be either passed from above
    // (in that case there'll be LMOD_BOX_EDIT)
    // or calculated for symbols: "+-=".
    // if exists than no casual baseline will produced
    if ( pLowData->rc->low_mode & LMOD_BOX_EDIT )
    {
        pLowData->rc->stroka.size_sure_in = 100;
        pLowData->rc->stroka.pos_sure_in = 100;
    }
    
    if ( !(pLowData->rc->low_mode & LMOD_BOX_EDIT) )
        // casual baseline wil be calculated
    {
        _SHORT epsy_Tmp;
        
        epsy_Tmp = (_SHORT) (const1.eps_y * koeff / KOEFF_BASE);
        /*CHE: for more rough Extrs for border*/
        if (lang != LANGUAGE_GERMAN)
            epsy_Tmp = THREE_HALF( epsy_Tmp );
        if ( epsy_Tmp < EPS_Y_BORDER_LIMIT )
            epsy_Tmp = EPS_Y_BORDER_LIMIT;
        if ( epsy_Tmp == const1.eps_y )
            epsy_Tmp = const1.eps_y - 1;
        
        if ( InitGroupsBorder( pLowData, NOINIT ) != SUCCESS )
            goto err;
        
        // specl initialization
        InitSpecl( pLowData, SPECVAL );
        
        // fill up specl with extremums
        if ( Extr( pLowData, epsy_Tmp, UNDEF, UNDEF, UNDEF, NREDUCTION_BORDER, Y_DIR ) != SUCCESS )
            goto err;
        
        pLowData->rc->stroka.extr_depth = epsy_Tmp;
    }
    
    // set x and y arrays work pointers to initial
    SetXYToInitial( pLowData );
    
    pLowData->ii = pLowData->rc->ii;
    
    // calculate baseline if required and scaling trace
    if ( transfrmN( pLowData ) != SUCCESS )
        goto err;
    return SUCCESS;
    
err:
    return UNSUCCESS;
}

/**********************************************************************/
/*            measure_slope(x,y,specl)                                */
/* definition of letters` middle inclination                          */
/* inclination is beeing measured in tg angle *100                    */
/*                                                                    */
/* while vertical disposition = 0                                     */
/* while right inclination slope > 0                                  */
/**********************************************************************/

_SHORT measure_slope( low_type _PTR low_data )
{
    p_SHORT x = low_data->x; /* co-ordinates of word`s points */
    p_SHORT y = low_data->y;
    p_SPECL specl = low_data->specl; /* the list of special points     */
    p_SPECL aspecl; /* current element       */
    p_SPECL nspecl; /*  next element         */
    _SHORT dx, dy;
    _SHORT slope; /* letters`inclination*/
    _SHORT sum_dx, sum_dy;
    _SHORT coef; /* coefficient of admittance of big inclinatons */
    _SHORT sum_dy_reset; /* the summ of deleted sticks */
    
    /**********************************************************************/
    
    coef = 2; /* at first delete inclinaton     */
rep:
    sum_dx = 0; /* the summ of segments` lengths on  X*/
    sum_dy = 0; /* the summ of segments` length on   Y*/
    sum_dy_reset = 0; /* the summ of deleted sticks*/
    aspecl = (SPECL near *) &specl[0]; /* address of list max-min    */
    if ( (aspecl = aspecl->next) == _NULL )
        return 0; /*to the 1st unempty elem*/
    nspecl = aspecl->next; /* the second element    */
    /* measuring of inclination must be done only in pairs:               */
    /* upper-down element (MINW - MAXW) without break                       */
    /* in the direction from the endof upper to the beginning of down     */
    
    while ( nspecl != _NULL ) /* go on the list         */
    {
        if ( (aspecl->mark == MINW || aspecl->mark == MINN) && (nspecl->mark == MAXW || nspecl->mark == MAXN) )
        { /* the pair of elem. suits*/
            dx = x[aspecl->iend] - x[nspecl->ibeg];
            dy = -y[aspecl->iend] + y[nspecl->ibeg]; /* "-" for sign slope     */
            /* select segments directed verticaly and down                        */
            if ( dy > 0 && coef * HWRAbs( dx ) < dy )
            {
                sum_dy += dy; /* common number of points */
                sum_dx += dx;
            }
            else
            {
                sum_dy_reset += dy;
            }
        }
        
#ifdef FORMULA
        if ((aspecl->mark == MAXW || aspecl->mark == MAXN) &&
            (nspecl->mark == MINW || nspecl->mark == MINN))
        { /* pair of elem. suits    */
            dx=-x[aspecl->iend]+x[nspecl->ibeg];
            dy=y[aspecl->iend]-y[nspecl->ibeg]; /* "-" for sign  slope    */
            /* select segments directed verticaly and down                        */
            if (dy > 0 && coef*HWRAbs(dx) < dy)
            {
                sum_dy+=(dy/COEF_UP_DOWN); /* common number of points */
                sum_dx+=(dx/COEF_UP_DOWN);
            }
            else
            {
                sum_dy_reset+=dy;
            }
        }
        
#endif     /* FORMULA  */
        aspecl = nspecl; /* passage to the next elem */
        nspecl = nspecl->next;
    }
    if ( coef > 0 && 4 * sum_dy_reset > 3 * sum_dy )/* a lot of refusals -    */
    /* - big inclination */
    {
        coef -= 1; /* weaken limits on inclination */
        goto rep;
        /* revise the account      */
    }
    if ( sum_dy == 0 ) /* there are no necessary elements    */
    {
        slope = 0;
    }
    else
    {
        slope = (_SHORT) ((long) 100 * sum_dx / sum_dy);
    }
    
    return slope;
}

/* *************************************************************** */

#ifndef _WSBDLL

_INT AnalyzeLowData( low_type _PTR pLowData, PS_point_type _PTR trace )
{
    _INT retval = UNSUCCESS;
    int lang = pLowData->rc->lang;
    
#ifdef  D_ARCS
    ARC_CONTROL ArcControl;
    p_ARC_CONTROL pArcControl = &ArcControl;
#endif
    
    _UM_MARKS_CONTROL UmMarksControl;
    UmMarksControl.pUmMarks=_NULL;
    pLowData->pUmMarksControl = &UmMarksControl;
    
    // calculate bounding box of the scaled trace
    GetLowDataRect( pLowData );
    
    // remove some garbage from x and y arrays
    Errorprov( pLowData );
    
    // normalizing
    if ( PreFilt( const1.horda, pLowData ) != SUCCESS )
        goto err;
    
    if ( InitGroupsBorder( pLowData, INIT ) != SUCCESS )
        goto err;
    
    DefLineThresholds( pLowData );
    InitSpecl( pLowData, SPECVAL );
    
    Extr( pLowData, const1.eps_y, const1.eps_x, const1.eps_x, ONE_HALF(const1.eps_y), NREDUCTION_BORDER, X_DIR | Y_DIR
         | XY_DIR );
    
    if ( lang != LANGUAGE_ENGLISH && lang != LANGUAGE_ENGLISHUK && lang != LANGUAGE_GERMAN && lang != LANGUAGE_INDONESIAN )
    {
        CreateUmlData ( pLowData->pUmMarksControl , UMSPC );
        Sketch( pLowData );
    }
    OperateSpeclArray( pLowData );
    if ( Sort_specl( pLowData->specl, pLowData->len_specl ) != SUCCESS )
        goto err;
    
    if ( InitGroupsBorder( pLowData, INIT ) != SUCCESS )
        goto err;
    
#ifdef D_ARCS
    if ( Prepare_Arcs_Data( pArcControl ) )
        goto err;
    
    if ( Arcs( pLowData, pArcControl ) == ! SUCCESS )
        goto err;
#endif   /* D_ARCS     */
    
    if ( Pict( pLowData ) != SUCCESS )
        goto err;
    
    Surgeon( pLowData );
    
    // normalizing of the trace from x and y array to working buffers
    //
    if ( Filt( pLowData, const1.horda, ABSENCE ) != SUCCESS )
        goto err;
    
    if ( InitGroupsBorder( pLowData, INIT ) != SUCCESS )
        goto err;
    
    /*  Fill "xBuf", "yBuf" with the original trajectory: */
    
    trace_to_xy( pLowData->xBuf, pLowData->yBuf, pLowData->rc->ii, trace );
    
    // ???? at last good extremum for common use
    if ( Extr( pLowData, const1.eps_y, UNDEF, UNDEF, UNDEF, NREDUCTION, Y_DIR ) != SUCCESS )
        goto err;
    /* measure_slope prohibits changing SPECL - only MIN and MAX required */
    // do not move call from here
    if ( (pLowData->rc->low_mode & LMOD_BOX_EDIT) || (pLowData->rc->rec_mode == RECM_FORMULA) )
        pLowData->slope = 0;
    else
        pLowData->slope = measure_slope( pLowData );
    
    if ( lang != LANGUAGE_ENGLISH && lang != LANGUAGE_ENGLISHUK && lang != LANGUAGE_GERMAN && lang != LANGUAGE_INDONESIAN )
        UmMarksControl.termSpecl1 = pLowData->len_specl;
    
    //  if (Circle(pLowData) != SUCCESS)
    //           goto err;
    if ( angl( pLowData ) != SUCCESS )
        goto err;
    
    if ( !FindSideExtr( pLowData ) )
        goto err;
    
    if (lang != LANGUAGE_ENGLISH && lang != LANGUAGE_ENGLISHUK && lang != LANGUAGE_GERMAN && lang != LANGUAGE_INDONESIAN )
    {
        UmMarksControl.termSpecl = pLowData->len_specl;
    }
    
    if ( Cross( pLowData ) != SUCCESS )
        goto err;
    
#ifdef D_ARCS
    ArcRetrace(pLowData, pArcControl);
    Dealloc_Arcs_Data (pArcControl);
#endif /* D_ARCS */
    
    if (lang != LANGUAGE_ENGLISH && lang != LANGUAGE_ENGLISHUK && lang != LANGUAGE_GERMAN && lang != LANGUAGE_INDONESIAN )
    {
        UmPostcrossModify( pLowData );
        if (IsCedillaSupport(lang))
            UmResultMark( pLowData );
        DestroyUmlData( pLowData->pUmMarksControl );
        DotPostcrossModify( pLowData );
    }
    
    // remove
    if ( Clear_specl( pLowData->specl, pLowData->len_specl ) != SUCCESS )
        goto err;
    /* preprosessing of elements*/
    if ( lk_begin( pLowData ) != SUCCESS )
        goto err;
    
    /* analyze crossings */
    lk_cross( pLowData );
    
    /* analize arcs */
    lk_duga( pLowData );
    
    Adjust_I_U( pLowData ); /*CHE*/
    
    /* analyzing of break element */
    if ( xt_st_zz( pLowData ) != SUCCESS )
        goto err;
    
    if ( RestoreColons( pLowData ) != SUCCESS )
        goto err;
    if ( !PostFindSideExtr( pLowData ) ) /*CHE*/
        goto err;
    retval = SUCCESS;
    
err:
    if (lang != LANGUAGE_ENGLISH && lang != LANGUAGE_ENGLISHUK && lang != LANGUAGE_GERMAN && lang != LANGUAGE_INDONESIAN )
    {
        DestroyUmlData( pLowData->pUmMarksControl );
    }
    
    return retval;
}

#endif // _WSBDLL
#ifdef LSTRIP

/************************************************************************** */

insert_pseudo_xr(xrd_el_type _PTR xrd_num_elem, xrinp_type _PTR pxr,
                 p_SPECL cur, PS_point_type _PTR trace, _SHORT _PTR pos)
{
    _RECT box;
    
    HWRMemCpy(&(xrd_num_elem->xr),pxr,sizeof(xrinp_type));
    xrd_num_elem->hotpoint=pos[cur->ipoint0];
    xrd_num_elem->begpoint=pos[cur->ibeg];
    xrd_num_elem->endpoint=pos[cur->iend];
    GetBoxFromTrace( trace,
                    xrd_num_elem->begpoint, xrd_num_elem->endpoint,
                    &box );
    xrd_num_elem->box_left=box.left;
    xrd_num_elem->box_up=box.top;
    xrd_num_elem->box_right=box.right;
    xrd_num_elem->box_down=box.bottom;
    xrd_num_elem->location=0;
}

/************************************************************************** */

_SHORT form_pseudo_xr_data(low_type _PTR low_data, xrdata_type _PTR xrdata)
{
    p_SPECL spec = low_data->specl, cur;
    PS_point_type _PTR trace = low_data->p_trace;
    _SHORT _PTR pos=low_data->buffers[2].ptr;
    xrd_el_type _PTR xrd = &(*xrdata->xrd)[0];
    xrd_el_type _PTR xrd_num_elem;
    xrinp_type pseudo_xr, FF_xr;
    _INT num_elem;
    
    pseudo_xr.type=0;
    pseudo_xr.attrib=0;
    pseudo_xr.penalty=0;
    pseudo_xr.height=0;
    pseudo_xr.shift=0;
    pseudo_xr.orient=0;
    pseudo_xr.depth=0;
    pseudo_xr.emp=0;
    
    FF_xr.type=X_FF;
    FF_xr.attrib=END_LETTER_FLAG;
    FF_xr.penalty=0;
    FF_xr.height=0;
    FF_xr.shift=0;
    FF_xr.orient=0;
    FF_xr.depth=0;
    FF_xr.emp=0;
    
    insert_pseudo_xr(&xrd[0],&FF_xr,cur,trace,pos);
    num_elem=1;
    
    for (cur=spec; cur!=NULL; cur=cur->next)
    {
        if (cur->attr==PSEUDO_XR_MIN || cur->attr==PSEUDO_XR_MAX || cur->attr==PSEUDO_XR_DEF)
        {
            pseudo_xr.type=cur->attr;
            insert_pseudo_xr(&xrd[num_elem],&pseudo_xr,cur,trace,pos);
            num_elem++;
        }
        if (cur->mark==END)
        {
            insert_pseudo_xr(&xrd[num_elem],&FF_xr,cur,trace,pos);
            num_elem++;
        }
        
    }
    xrdata->len=num_elem;
    HWRMemSet(&xrd[num_elem],0,sizeof(xrd[0]));
    return (SUCCESS);
}

#endif  //#ifdef LSTRIP

#endif //#ifndef LSTRIP
