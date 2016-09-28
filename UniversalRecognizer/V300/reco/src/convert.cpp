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
#include "calcmacr.h"
#include "accents.h"

#include "dti.h"
#include "def.h"

#include "xr_attr.h"

#include "reco.h"
#include "langid.h"
#include "langutil.h"

#define  SHOW_DBG_SPECL  0

#define  SOME_PENALTY         ((_UCHAR)5)
#define  MAXPENALTY           ((_UCHAR)18)

static _SHORT check_xrdata(xrd_el_type _PTR xrd, p_low_type low_data);
static _SHORT PutZintoXrd(p_low_type low_data,
                   xrd_el_type _PTR xrd_i_p1,
                   xrd_el_type _PTR xrd_i_m1,
                   xrd_el_type _PTR xrd_i,
                   _UCHAR lp,_SHORT i,p_SHORT len);

static _VOID  AssignInputPenaltyAndStrict(p_SPECL cur, xrd_el_type _PTR xrd_elem, _INT lang );
static _SHORT MarkXrAsLastInLetter(xrd_el_type _PTR xrd,p_low_type low_data,p_SPECL elem);

/* ************************************************************************ */
/**              Converter to Xr format                                   * */
/* ************************************************************************ */

/************************************************************************** */
/*       Receiving of input word and customizing it                         */
/************************************************************************** */

/************************************************************************** */
/*    Extracting codes from SPECL array and print picture                   */
/************************************************************************** */

#ifndef LSTRIP

_SHORT exchange(low_type _PTR low_data, xrdata_type _PTR xrdata)
{
    p_SPECL            spec  = low_data->specl;
    PS_point_type _PTR trace = low_data->p_trace;
    _SHORT i,num_elem;
    xrd_el_type _PTR xrd = &(*xrdata->xrd)[0];
    xrd_el_type _PTR xrd_num_elem;
    _UCHAR xr_code = '\0', fb, height;
    p_SPECL cur;
    _RECT box;
    _UCHAR lp = SOME_PENALTY;
    _SHORT _PTR xp=low_data->x,
    _PTR yp=low_data->y,
    _PTR pos=low_data->buffers[2].ptr;
    int lang = low_data->rc->lang;
    
    /* TEMPORARY FOR EXPERIMENTS */
    if (lang == LANGUAGE_GERMAN || lang == LANGUAGE_FRENCH || lang == LANGUAGE_ITALIAN || lang == LANGUAGE_PORTUGUESE)
    {
        cur=(p_SPECL)&spec[0];
        while( cur != _NULL )
        {
            if(cur->code==_BSS_)
                cur->attr=_MD_;
            cur=cur->next;
        }
    }
    
    num_elem = 0;
    xrd_num_elem = &xrd[num_elem];
    xrd_num_elem->xr.type     = X_FF;
    xrd_num_elem->xr.attrib   = 0;
    xrd_num_elem->xr.penalty  = SOME_PENALTY;
    XASSIGN_HEIGHT( xrd_num_elem, _MD_ );
    XASSIGN_XLINK( xrd_num_elem,            (_UCHAR)LINK_LINE );  //DX_UNDEF_VALUE );
    MarkXrAsLastInLetter(xrd_num_elem,low_data,spec);
    /* Some calculations for proper ibeg-iend pos. in 1st _FF_: */
    xrd_num_elem->hotpoint=xrd_num_elem->begpoint  = 1; /* Write location of elem */
    for  ( cur=spec->next;  cur!=_NULL; cur=cur->next )
    {
        if  ( !IsXTorST(cur) || cur->next==_NULL || IsAnyBreak(cur->next ) )
            break;
    }
    
    if  ( cur == _NULL )
        xrd_num_elem->endpoint  = 1;
    else
    {
        xrd_num_elem->endpoint  = cur->ibeg;
        while ( xrd_num_elem->endpoint>0  &&  yp[xrd_num_elem->endpoint-1] != BREAK )
            xrd_num_elem->endpoint--;
    }
    
    xrd_num_elem->box_left  = HWRMin( xp[1], xp[xrd_num_elem->endpoint]);
    xrd_num_elem->box_up    = HWRMin( yp[1], yp[xrd_num_elem->endpoint]);
    xrd_num_elem->box_right = HWRMax( xp[1], xp[xrd_num_elem->endpoint]);
    xrd_num_elem->box_down  = HWRMax( yp[1], yp[xrd_num_elem->endpoint]);
    
    num_elem ++;
    xrd_num_elem ++;
    
    cur=spec->next;
    
    if(cur==_NULL) xr_code=X_NOCODE;
    else if(cur->code==_ZZ_ || cur->code==_ZZZ_ || cur->code==_Z_)
        cur=cur->next;
    
    while( cur != _NULL ) /* Cycle for extracting of xr's */
    {
        xr_code=cur->code;
        height =cur->attr & _umd_;
        fb     =cur->attr & _fb_;
        
        xrd_num_elem->hotpoint=0;  //just in case...
        
        switch ((_SHORT)xr_code)
        {
            case _IU_ :
                switch ((_SHORT)cur->mark)
                {
                    case BEG: xr_code=X_IU_BEG;
                        break;
                        
                    case END: xr_code=X_IU_END;
                        break;
                        
                    default:
                        if(fb==_f_)
                        {
                            if(cur->mark==STICK || (cur->mark==CROSS && COUNTERCLOCKWISE(cur+1)) )
                                xr_code=X_IU_STK;
                            else
                                xr_code=X_IU_F;
                        }
                        else
                            xr_code=X_IU_B;
                        break;
                }
                xrd_num_elem->hotpoint=pos[cur->ipoint0];
                break;
            case _ID_ :
                switch ((_SHORT)cur->mark)
                {
                    case BEG: xr_code=X_ID_BEG;
                        break;
                        
                    case END: xr_code=X_ID_END;
                        break;
                        
                    default:
                        if(fb==_b_)
                        {
                            if(cur->mark==STICK )
                                xr_code=X_ID_STK;
                            else
                                xr_code=X_ID_F;
                        }
                        else
                            xr_code=X_ID_B;
                        break;
                }
                xrd_num_elem->hotpoint=pos[cur->ipoint0];
                break;
                
            case _DF_ : xr_code=X_DF;
                xrd_num_elem->hotpoint=0;
                break;
                
            case _BSS_:
                if (lang == LANGUAGE_GERMAN || lang == LANGUAGE_FRENCH
                    || lang == LANGUAGE_ITALIAN || lang == LANGUAGE_PORTUGUESE)
                {
                    xr_code=X_BSS;
                    xrd_num_elem->hotpoint=0;
                    break;
                }
                
            case _ST_ :
                if(IsUmlautSupport(lang) && cur->other & ST_UMLAUT)
                    xr_code=X_UMLAUT;
                else if (IsCedillaSupport(lang) && cur->other & ST_CEDILLA )
                    xr_code = X_CEDILLA;
                else
                    xr_code=X_ST;
                xrd_num_elem->hotpoint = 0;
                break;
                
            case _XT_ :
                if(cur->other & WITH_CROSSING)
                    xr_code=X_XT;
                else
                    xr_code=X_XT_ST;
                xrd_num_elem->hotpoint=0;
                break;
                
            case _ANl :
                xr_code=X_AL;
                xrd_num_elem->hotpoint=pos[cur->ipoint0];
                break;
                
            case _ANr :
                xr_code=X_AR;
                xrd_num_elem->hotpoint=pos[cur->ipoint0];
                break;
                
            case _GU_ :
                xr_code=X_BGU;
                xrd_num_elem->hotpoint=pos[cur->ipoint0];
                break;
                
            case _GD_ :
                xr_code=X_BGD;
                xrd_num_elem->hotpoint=pos[cur->ipoint0];
                break;
                
            case _GUs_:
                xr_code=X_SGU;
                xrd_num_elem->hotpoint=cur->ipoint0!=UNDEF ? pos[cur->ipoint0] : 0;
                break;
                
            case _GDs_:
                xr_code=X_SGD;
                xrd_num_elem->hotpoint=cur->ipoint0!=UNDEF ? pos[cur->ipoint0] : 0;
                break;
                
            case _Gl_ :
                xr_code=X_GL;
                xrd_num_elem->hotpoint=0;
                break;
            case _Gr_ :
                
                xr_code=X_GR;
                xrd_num_elem->hotpoint=0;
                break;
                
            case _UU_ :
                if(fb==_f_)
                    xr_code=X_UU_F;
                else
                    xr_code=X_UU_B;
                xrd_num_elem->hotpoint=pos[cur->ipoint0];
                break;
                
            case _UD_ :
                if(fb==_f_)
                    xr_code=X_UD_B;
                else
                    xr_code=X_UD_F;
                xrd_num_elem->hotpoint=pos[cur->ipoint0];
                break;
                
            case _UUC_:
                if(fb==_f_)
                    xr_code=X_UUC_F;
                else
                    xr_code=X_UUC_B;
                xrd_num_elem->hotpoint=cur->ipoint0!=UNDEF ? pos[cur->ipoint0] : 0;
                break;
                
            case _UDC_:
                if(fb==_f_)
                    xr_code=X_UDC_B;
                else
                    xr_code=X_UDC_F;
                xrd_num_elem->hotpoint=cur->ipoint0!=UNDEF ? pos[cur->ipoint0] : 0;
                break;
                
            case _UUR_:
                if(fb==_f_)
                    xr_code=X_UUR_F;
                else
                    xr_code=X_UUR_B;
                xrd_num_elem->hotpoint=pos[cur->ipoint0];
                break;
                
            case _UUL_:
                if(fb==_f_)
                    xr_code=X_UUL_F;
                else
                    xr_code=X_UUL_B;
                xrd_num_elem->hotpoint=pos[cur->ipoint0];
                break;
                
            case _UDR_:
                if(fb==_f_)
                    xr_code=X_UDR_B;
                else
                    xr_code=X_UDR_F;
                xrd_num_elem->hotpoint=pos[cur->ipoint0];
                break;
                
            case _UDL_:
                if(fb==_f_)
                    xr_code=X_UDL_B;
                else
                    xr_code=X_UDL_F;
                xrd_num_elem->hotpoint=pos[cur->ipoint0];
                break;
                
            case _DUR_:
                xr_code=X_DU_R;
                xrd_num_elem->hotpoint=cur->ipoint1!=UNDEF ? pos[cur->ipoint1] : 0;
                break;
                
            case _CUR_:
                xr_code=X_CU_R;
                xrd_num_elem->hotpoint=cur->mark!=CROSS && cur->ipoint1!=UNDEF ? pos[cur->ipoint1] : 0;
                break;
                
            case _CUL_:
                xr_code=X_CU_L;
                xrd_num_elem->hotpoint=cur->mark!=CROSS && cur->ipoint1!=UNDEF ? pos[cur->ipoint1] : 0;
                break;
                
            case _DUL_:
                xr_code=X_DU_L;
                xrd_num_elem->hotpoint=cur->ipoint1!=UNDEF ? pos[cur->ipoint1] : 0;
                break;
                
            case _DDR_:
                xr_code=X_DD_R;
                xrd_num_elem->hotpoint=cur->ipoint1!=UNDEF ? pos[cur->ipoint1] : 0;
                break;
                
            case _CDR_:
                xr_code=X_CD_R;
                xrd_num_elem->hotpoint=cur->mark!=CROSS && cur->ipoint1!=UNDEF ? pos[cur->ipoint1] : 0;
                break;
                
            case _CDL_:
                xr_code=X_CD_L;
                xrd_num_elem->hotpoint=cur->mark!=CROSS && cur->ipoint1!=UNDEF ? pos[cur->ipoint1] : 0;
                break;
                
            case _DDL_:
                xr_code=X_DD_L;
                xrd_num_elem->hotpoint=cur->ipoint1!=UNDEF ? pos[cur->ipoint1] : 0;
                break;
                
            case _Z_:
                if(cur->next!=_NULL)
                    xr_code=X_Z;
                else
                    xr_code=X_FF;
                xrd_num_elem->hotpoint=0;
                break;
                
            case _ZZ_ :
                if(cur->next!=_NULL)
                    xr_code=X_ZZ;
                else
                    xr_code=X_FF;
                xrd_num_elem->hotpoint=0;
                break;
                
            case _ZZZ_:
                if(cur->next!=_NULL)
                    xr_code=X_ZZZ;
                else
                    xr_code=X_FF;
                xrd_num_elem->hotpoint=0;
                break;
                
            case _FF_:
                xr_code=X_FF;
                xrd_num_elem->hotpoint=0;
                break;
                
            case _TS_:
                xr_code=X_TS;
                xrd_num_elem->hotpoint=0;
                break;
                
            case _TZ_:
                xr_code=X_TZ;
                xrd_num_elem->hotpoint=0;
                break;
                
            case _BR_:
                xr_code=X_BR;
                xrd_num_elem->hotpoint=0;
                break;
                
            case _BL_:
                xr_code=X_BL;
                xrd_num_elem->hotpoint=0;
                break;
                
            case _AN_UR:
                xr_code = X_AN_UR;
                //xrd_num_elem->hotpoint=pos[MID_POINT(cur)];
                xrd_num_elem->hotpoint=0;
                break;
            case _AN_UL:
                xr_code = X_AN_UL;
                //xrd_num_elem->hotpoint=pos[MID_POINT(cur)];
                xrd_num_elem->hotpoint=0;
                break;
                
            default:
                goto NXT;
        }
        
        xrd_num_elem->xr.type    = xr_code;
        xrd_num_elem->xr.height  = height;
        xrd_num_elem->xr.attrib  = 0;
        //xrd_num_elem->xr.penalty     = SOME_PENALTY;
        
        /* all experiments for input penalties and stricts */
        AssignInputPenaltyAndStrict( cur, xrd_num_elem, lang );
        
        //    if(cur->code==_ZZ_ && (cur->other & SPECIAL_ZZ))
        //     xrd_num_elem->xr.attrib |= X_SPECIAL_ZZ;
        if(cur->code==_XT_ && (cur->other & RIGHT_KREST))
            xrd_num_elem->xr.attrib |= X_RIGHT_KREST;
        xrd_num_elem->begpoint = cur->ibeg;      /* Write location of elem */
        xrd_num_elem->endpoint = cur->iend;
        
        // GIT
        GetLinkBetweenThisAndNextXr(low_data,cur,xrd_num_elem);
        MarkXrAsLastInLetter(xrd_num_elem,low_data,cur);
        
        if  ( xrd_num_elem->hotpoint == 0  &&  !IsAnyBreak( cur ) )
        {
            _INT iMid = MEAN_OF( cur->ibeg, cur->iend );
            xrd_num_elem->hotpoint = pos[iMid]; //!!!!
        }
        
        num_elem ++;
        xrd_num_elem ++;
        if (num_elem > XRINP_SIZE - 3) break;
        
NXT:
        cur = cur->next;
    }                                               /* End of cycle */
    
    if(xr_code != X_FF)
    {
        xrd_num_elem->xr.type     = X_FF;
        XASSIGN_HEIGHT( xrd_num_elem, _MD_ );
        XASSIGN_XLINK( xrd_num_elem,          (_UCHAR)LINK_LINE );  //DX_UNDEF_VALUE );
        xrd_num_elem->xr.orient =    (_UCHAR)LINK_LINE;
        xrd_num_elem->xr.penalty      = lp;
        xrd_num_elem->xr.attrib      = 0;
        MarkXrAsLastInLetter(xrd_num_elem,low_data,spec);
        i = (xrd_num_elem-1)->endpoint;
        xrd_num_elem->begpoint  = i; /* Write location of elem */
        xrd_num_elem->endpoint  = i;
        xrd_num_elem->hotpoint  = pos[i];
        xrd_num_elem->box_left  = xp[i];
        xrd_num_elem->box_up    = yp[i];
        xrd_num_elem->box_right = xp[i];
        xrd_num_elem->box_down  = yp[i];
        num_elem ++;
        xrd_num_elem ++;
    }
    else
    {
        if ( num_elem > 2 )
        {
            i = (xrd_num_elem-2)->endpoint;
            (xrd_num_elem-1)->begpoint  = i; /* Write location of elem */
            (xrd_num_elem-1)->endpoint  = i;
            (xrd_num_elem-1)->hotpoint  = pos[i];
        }
    }
    
    HWRMemSet((p_VOID)xrd_num_elem, 0, sizeof(xrd_el_type));
    
    
    /************************************************************************** */
    /*       Tails near ZZ and BCK                                              */
    /************************************************************************** */
    
    for (i = 0; i < XRINP_SIZE && xrd[i].xr.type != 0; i ++)
    {
        if(X_IsBreak(&xrd[i]))
        {
            if(i > 0)
            {
                xrd[i-1].xr.attrib |= TAIL_FLAG;
                if  ( i > 1 ) //skip all right-brought _XTs:
                {
                    _INT  iRealTail;
                    for  ( iRealTail=i-1;
                          iRealTail>0
                          && (xrd[iRealTail].xr.type == X_XT || xrd[iRealTail].xr.type == X_XT_ST)
                          && (xrd[iRealTail].xr.attrib & X_RIGHT_KREST);
                          iRealTail-- )
                        ;
                    if  ( iRealTail>0  &&  iRealTail < i-1 )
                        xrd[iRealTail].xr.attrib |= TAIL_FLAG;
                }
            }
            xrd[i+1].xr.attrib |= TAIL_FLAG;
            /* Double-tailing: */
            if (   (   xrd[i+1].xr.type == X_IU_BEG
                    || xrd[i+1].xr.type == X_UUL_F
                    || xrd[i+1].xr.type == X_UUR_B
                    || xrd[i+1].xr.type == X_GL
                    || xrd[i+1].xr.type == X_SGU
                    )
                && (   xrd[i+2].xr.type == X_UD_F
                    || (   xrd[i+2].xr.type == X_AL
                        && xrd[i+3].xr.type == X_UD_F
                        )
                    )
                )
            {
                _INT  iNext   = (xrd[i+2].xr.type==X_AL)? (i+3):(i+2);
                _INT  dHeight =   (_INT)(xrd[iNext].xr.height)
                - (_INT)(xrd[i+1].xr.height);
                if  ( dHeight>=0 && (i==0 || dHeight<=3) ) /* 1st tail may be high */
                    xrd[iNext].xr.attrib |= TAIL_FLAG;
            }
        }
        
    }
    
    /************************************************************************** */
    /*       Transform x and y in tablet coordinates                            */
    /************************************************************************** */
    
    for (i = 0; i < XRINP_SIZE && xrd[i].xr.type != 0; i ++)
    {
        xrd_num_elem = &xrd[i];
        xrd_num_elem->begpoint  = pos[xrd_num_elem->begpoint]; /* Write location of elem */
        xrd_num_elem->endpoint  = pos[xrd_num_elem->endpoint];
        /*CHE: for one-point xrs to be extended by one point: */
        if  ( !X_IsBreak(xrd_num_elem) && xrd_num_elem->begpoint == xrd_num_elem->endpoint )
        {
            if  ( trace[xrd_num_elem->begpoint-1].y == BREAK )
            {
                if  ( trace[xrd_num_elem->begpoint+1].y != BREAK )
                    xrd_num_elem->endpoint++;
            }
            else  if  ( trace[xrd_num_elem->begpoint+1].y == BREAK )
                xrd_num_elem->begpoint--;
            else
            {
                switch( xrd_num_elem->xr.type )
                {
                    case  X_IU_BEG:
                    case  X_ID_BEG:
                    case  X_UUL_F:
                    case  X_UUR_B:
                    case  X_UDL_F:
                    case  X_UDR_B:
                        xrd_num_elem->endpoint++;
                        break;
                        
                    case  X_IU_END:
                    case  X_ID_END:
                    case  X_UUL_B:
                    case  X_UUR_F:
                    case  X_UDL_B:
                    case  X_UDR_F:
                        xrd_num_elem->begpoint--;
                        break;
                        
                    case  X_IU_STK:
                    case  X_ID_STK:
                        if  ( X_IsBreak(xrd_num_elem-1) )
                            xrd_num_elem->endpoint++;
                        break;
                        
                }
            }
        }
        GetBoxFromTrace( trace,
                        xrd_num_elem->begpoint, xrd_num_elem->endpoint,
                        &box );
        xrd_num_elem->box_left  = box.left;
        xrd_num_elem->box_up    = box.top;
        xrd_num_elem->box_right = box.right;
        xrd_num_elem->box_down  = box.bottom;
    }
    check_xrdata(xrd,low_data);
    
    for (num_elem=0; num_elem<XRINP_SIZE && xrd[num_elem].xr.type!=0; num_elem++);
    xrdata->len = num_elem;
    
    FillXrFeatures(xrdata, low_data);
    
    return SUCCESS;
}

#endif //LSTRIP



/************************************************************************** */
/*  This function recounts input penalties and strict attributes            */
/************************************************************************** */

#define PENALTY_FOR_FAKE_STROKE                 ((_UCHAR)2)
#define ADD_PENALTY_FOR_CUTTED_STROKE           ((_UCHAR)6)
#define ADD_PENALTY_FOR_STROKE_WITH_1_CROSSING  ((_UCHAR)6)
#define ADD_PENALTY_FOR_GAMMAS_US_DS            ((_UCHAR)6)

#define DHEIGHT_FOR_LITTLE_PENALTY       ((_UCHAR)6)
#define DHEIGHT_FOR_BIG_PENALTY          ((_UCHAR)12)
#define ADD_PENALTY_FOR_LITTLE_DHEIGHT   ((_UCHAR)4)
#define ADD_PENALTY_FOR_BIG_DHEIGHT      ((_UCHAR)8)
#define ADD_PENALTY_FOR_ANGLE            ((_UCHAR)1)
#define ADD_PENALTY_FOR_LARGE_H_MOVE     ((_UCHAR)8)

RECO_DATA_EXTERNAL  _UCHAR  penlDefX[XR_COUNT];
RECO_DATA_EXTERNAL  _UCHAR  penlDefX_German[XR_COUNT];
RECO_DATA_EXTERNAL  _UCHAR  penlDefH[XH_COUNT];

static _VOID  AssignInputPenaltyAndStrict(p_SPECL cur, xrd_el_type _PTR xrd_elem, _INT lang )
{
    _UCHAR  height = (xrd_elem->xr.height);
    
    
    /*  For angle-like elements ("side extrs") don't do    */
    /* any processing except taking the value from "other" */
    /* field:                                              */
    
    if  ( cur->code==_AN_UR  ||  cur->code==_AN_UL )
    {
        xrd_elem->xr.penalty = cur->other;
        return;
    }
    
    if  (   (cur->code==_ANl  ||  cur->code==_ANr)
         && cur->ipoint1 != UNDEF
         )  {
        xrd_elem->xr.penalty = (_UCHAR)cur->ipoint1;
        return;
    }
    /*  Same thing for S- and Z- arcs: */
    
    if  ( cur->code==_TS_  ||  cur->code==_TZ_ )
    {
        xrd_elem->xr.penalty = cur->other;
        return;
    }
    
    /*  Special treatment for some peculiar cases: */
    
    if(   (cur->code==_FF_ || cur->code==_Z_)
       && ((cur->other & FF_CUTTED) || (cur->other & NO_PENALTY)) )
    {
        xrd_elem->xr.penalty = 0;
        return;
    }
    
    if(cur->code==_XT_ && (cur->other & FAKE))
    {
        xrd_elem->xr.penalty = PENALTY_FOR_FAKE_STROKE;
        return;
    }
    if((Is_IU_or_ID(cur) || IsAnyArcWithTail(cur)) && (cur->other & MIN_MAX_CUTTED))
    {
        xrd_elem->xr.penalty = 0;
        return;
    }
    
    /*  Regular treatment: */
    if  ( xrd_elem->xr.type<XR_COUNT  &&  height<XH_COUNT )
    {
        if (lang == LANGUAGE_GERMAN)
            xrd_elem->xr.penalty = penlDefX_German[xrd_elem->xr.type];
        else
            xrd_elem->xr.penalty = penlDefX[xrd_elem->xr.type];
        
        if  (cur->code != _ST_ && !IsAnyAngle( cur )
             && (lang != LANGUAGE_GERMAN || xrd_elem->xr.type != X_XT_ST))
            xrd_elem->xr.penalty += penlDefH[height];
    }
    else
    {
        xrd_elem->xr.penalty = 0;
    }
    
    
    if(cur->code==_XT_)
    {
        if(cur->other & CUTTED)
        {
            xrd_elem->xr.penalty += ADD_PENALTY_FOR_CUTTED_STROKE;
        }
        /* if it was the only crossing */
        else if((cur->other & WITH_CROSSING) &&
                cur->ipoint0!=0              &&
                cur->ipoint1==0 )
        {
            xrd_elem->xr.penalty += ADD_PENALTY_FOR_STROKE_WITH_1_CROSSING;
        }
    }
    
    else if((cur->code==_GU_ && HEIGHT_OF(cur)<=_US2_) ||
            (cur->code==_GD_ && HEIGHT_OF(cur)>=_DS1_) )
    {
        xrd_elem->xr.penalty += ADD_PENALTY_FOR_GAMMAS_US_DS;
    }
    
    if(lang == LANGUAGE_GERMAN && cur->code==_BSS_)
    {
        if(cur->other & ZERO_PENALTY)
            xrd_elem->xr.penalty = 0;
        else if(cur->other & LARGE_PENALTY)
            xrd_elem->xr.penalty += ADD_PENALTY_FOR_LARGE_H_MOVE;
    }
    
    /*  Now recount penalty basing on Lossev's idea about */
    /* non-covered parts of trj:                          */
    
    if  (   !IsAnyBreak(cur) && !IsXTorST(cur) && cur->prev->code != _NO_CODE )
    {
        p_SPECL  pPrev, pNext;
        
        for  ( pPrev=cur->prev; pPrev!=_NULL  &&  pPrev->code!=_NO_CODE; pPrev=pPrev->prev )
        {
            if  ( !IsXTorST(pPrev) )
                break;
        }
        for  ( pNext=cur->next; pNext!=_NULL; pNext=pNext->next )
        {
            if  ( !IsXTorST(pNext) )
                break;
        }
        
        if  (   pPrev != _NULL
             && pNext != _NULL
             && pPrev->code != _NO_CODE
             && ( !IsAnyBreak(pPrev)  ||  !IsAnyBreak(pNext) ) )
        {
            _SHORT  dHgtSum    = 0;
            _UCHAR  addPenalty = 0;
            
            if  ( !IsAnyBreak(pPrev) )
                dHgtSum += HWRAbs(HEIGHT_OF(cur) - HEIGHT_OF(pPrev));
            if  ( !IsAnyBreak(pNext) )
                dHgtSum += HWRAbs(HEIGHT_OF(cur) - HEIGHT_OF(pNext));
            
            if  ( dHgtSum >= DHEIGHT_FOR_BIG_PENALTY )
                addPenalty += ADD_PENALTY_FOR_BIG_DHEIGHT;
            else  if  ( dHgtSum >= DHEIGHT_FOR_LITTLE_PENALTY )
                addPenalty += ADD_PENALTY_FOR_LITTLE_DHEIGHT;
            
            if  ( IsAnyAngle(cur) )
                addPenalty = HWRMin( addPenalty, ADD_PENALTY_FOR_ANGLE );
            
            xrd_elem->xr.penalty += addPenalty;
            
            if  ( xrd_elem->xr.penalty > MAXPENALTY )
                xrd_elem->xr.penalty = MAXPENALTY;
        }
    } /*40*/
    
} /* end of AssignInputPenaltyAndStrict */

/************************************************************************** */
/*       Check sequence in xrdata                                           */
/************************************************************************** */
static _SHORT  check_xrdata(xrd_el_type _PTR xrd, p_low_type low_data)
{
    _SHORT i,len,dxo,dxne1,dyo,duo;
    _UCHAR lp = SOME_PENALTY;
    xrd_el_type _PTR  xrd_i;     /*CHE*/
    xrd_el_type _PTR  xrd_i_m1;
    xrd_el_type _PTR  xrd_i_p1;
    PS_point_type _PTR trace = low_data->p_trace;
    
    
    for (len = 0; len < XRINP_SIZE && xrd[len].xr.type != 0; len ++);
    
    if (len >= XRINP_SIZE-2)
        goto err;
    
    for (i = 4; i < len && len < XRINP_SIZE; i ++)
    {
        
        xrd_i = &xrd[i];       /*CHE*/
        xrd_i_m1 = xrd_i - 1;
        xrd_i_p1 = xrd_i + 1;
        
        /* ------ Put split after 'O_b' ------ */
        if(xrd[i-2].xr.type == X_UDC_F &&                                  /* O_b */
           (xrd_i->xr.type == X_IU_F || xrd_i->xr.type == X_UU_F) &&    /* IU || UU */
           !X_IsBreak(xrd_i_p1))
        {
            _RECT box;
            GetBoxFromTrace( trace, xrd[i-3].begpoint,xrd_i_m1->endpoint, &box );
            //       size_cross(xrd[i-3].begpoint,xrd_i_m1->endpoint,xp,yp,&box);
            if(box.left   ==ALEF ||
               box.right  ==ELEM ||
               box.top    ==ALEF ||
               box.bottom ==ELEM
               )
                continue;
            dxo  = DX_RECT(box);                           /* Dx for 'o'   */
            dyo  = DY_RECT(box);                           /* Dy for 'o'   */
            duo  = box.top;                                /* Up for 'o'   */
            dxne1= xrd_i->box_right - box.right;           /* Dx to next el*/
        }
        else
            continue;
        if(dxne1 > dxo/4                         &&
           xrd_i->box_up > (_SHORT)(duo - dyo/3) &&
           xrd_i->box_up < (_SHORT)(duo + dyo/3))
        {
            if(PutZintoXrd(low_data,xrd_i_p1,xrd_i_m1,xrd_i,lp,i,&len)!=SUCCESS)
                break;
        }
    }
    
#if (0)
    /* Temporary: insert pseudobreak before X_DU_R */
    for (i = 3; i < len && len < XRINP_SIZE; i ++)
    {
        xrd_i    = &xrd[i];
        xrd_i_m1 = xrd_i - 1;
        xrd_i_p1 = xrd_i + 1;
        if(xrd_i->xr.type != X_DU_R)
            continue;
        if(PutZintoXrd(low_data,xrd_i_p1,xrd_i_m1,xrd_i,lp,i,&len)!=SUCCESS)
            break;
        i++;
    }
#endif
    
    /* Insert pseudobreak after X_DF */
    for (i = 3; i < len-3 && len < XRINP_SIZE; i ++)
    {
        xrd_i    = &xrd[i];
        xrd_i_p1 = xrd_i + 1;
        if(xrd_i->xr.type     != X_DF     ||
           X_IsBreak(xrd_i_p1)            ||
           xrd_i_p1->xr.type  == X_UUR_F  ||
           xrd_i_p1->xr.type  == X_UDR_F  ||
           xrd_i_p1->xr.type  == X_IU_END ||
           xrd_i_p1->xr.type  == X_ID_END )
        {
            continue;
        }
        if(((xrd_i+1)->xr.type == X_UU_F || (xrd_i+1)->xr.type == X_UD_F) &&
           ((xrd_i+2)->xr.type == X_IU_END || (xrd_i+2)->xr.type == X_ID_END) )
        {
            continue;
        }
        if(PutZintoXrd(low_data,xrd_i_p1+1,xrd_i,xrd_i_p1,lp,i,&len)!=SUCCESS)
        {
            break;
        }
        i++;
    }
    
    return 0;
err:
    return 1;
}

/************************************************************************/
/*   Put pseudobreak into xrdata                                        */
/************************************************************************/
static _SHORT PutZintoXrd(p_low_type low_data,
                   xrd_el_type _PTR xrd_i_p1,
                   xrd_el_type _PTR xrd_i_m1,
                   xrd_el_type _PTR xrd_i,
                   _UCHAR lp,_SHORT i,p_SHORT len)
{
    _SHORT ibeg=xrd_i_m1->endpoint,iend=xrd_i->begpoint;
    _RECT box;
    PS_point_type _PTR trace=low_data->p_trace;
    
    if(ibeg>iend)
        SWAP_SHORTS(ibeg,iend);
    HWRMemCpy((p_VOID)xrd_i_p1, (p_VOID)xrd_i, sizeof(xrd_el_type)*(*len-i+1));
    xrd_i->xr.type = XR_NOBR_LINK;
    XASSIGN_HEIGHT( xrd_i, _MD_ );
    XASSIGN_XLINK( xrd_i, (_UCHAR)LINK_LINE );   //DX_UNDEF_VALUE );
    xrd_i->xr.orient = (_UCHAR)LINK_LINE;
    xrd_i->xr.penalty= lp;
    xrd_i->xr.attrib = 0;
    MarkXrAsLastInLetter(xrd_i,low_data,low_data->specl);
    xrd_i->hotpoint=xrd_i->begpoint  = ibeg;
    xrd_i->endpoint  = iend;
    GetBoxFromTrace(trace,ibeg,iend,&box);
    xrd_i->box_left  = box.left;
    xrd_i->box_up    = box.top;
    xrd_i->box_right = box.right;
    xrd_i->box_down  = box.bottom;
    
    (*len)++;
    if(*len >= XRINP_SIZE-1)
        return UNSUCCESS;
    
    return SUCCESS;
    
} /* end of PutZintoXrd */

/************************************************************************/
/* This function marks xr's as possible last in letter for use in XRLW  */
/************************************************************************/

#define MARK_XR_AS_LAST_IN_LETTER(xrd) { (xrd)->xr.attrib |= END_LETTER_FLAG; }

static _SHORT MarkXrAsLastInLetter(xrd_el_type _PTR xrd,p_low_type low_data,p_SPECL elem)
{
    _INT lang = low_data->rc->lang;
    p_SPECL specl=low_data->specl,
    nxt=elem->next,
    prv=elem->prev;
    _UCHAR  code=elem->code,mark=elem->mark,NxtCode;
    
    /* BREAK or special case - mark as end of letter (EOL) */
    if(elem==specl || IsAnyBreak(elem))
    {
        MARK_XR_AS_LAST_IN_LETTER(xrd)
        return SUCCESS;
    }
    
    /* don't allow EOL after BREAK */
    if(NULL_or_ZZ_after(elem))
        return SUCCESS;
    
    NxtCode=nxt->code;
    //CHE:
    if ( code==_XT_ && (elem->other & RIGHT_KREST) && NxtCode==_XT_ && (nxt->other & RIGHT_KREST) )
    {
        MARK_XR_AS_LAST_IN_LETTER(xrd)
        return SUCCESS;
    }
    
    /* don't allow EOL after BREAK, except elem: ST, ST_XT and after _Z_ */
    if ( prv==specl || IsAnyBreak(prv) )
    {
        if( ((code==_ST_ || code==_XT_ /*&& (elem->other & WITH_CROSSING)==0*/ ) && nxt!=_NULL)
           || (prv->code==_Z_ && (   (!NULL_or_ZZ_after(nxt) && (Is_IU_or_ID(elem) || IsAnyArcWithTail(elem)))
                      || (NULL_or_ZZ_after(nxt)  && (code==_IU_ || code==_UUL_)) )) )
            MARK_XR_AS_LAST_IN_LETTER(xrd)
        return SUCCESS;
    }
    
    if ( (lang == LANGUAGE_FRENCH || lang == LANGUAGE_ITALIAN || lang == LANGUAGE_PORTUGUESE)
       && code==_UD_ && COUNTERCLOCKWISE(elem) && NULL_or_ZZ_after(nxt))
    {
        MARK_XR_AS_LAST_IN_LETTER(xrd)
        return SUCCESS;
    }
    
    /* don't allow EOL before elem following by BREAK, except spec. cases above */
    if(NULL_or_ZZ_after(nxt))
        return SUCCESS;
    
    _BOOL is_lang_condition = _FALSE;
    if (lang == LANGUAGE_FRENCH || lang == LANGUAGE_ITALIAN
        || lang == LANGUAGE_GERMAN || lang == LANGUAGE_PORTUGUESE)
    {
        if (code ==_ANl && mark == 0)
            is_lang_condition = _TRUE;
    }
    /* xr's, which are potentially EOL */
    if(   code==_GD_  || code==_GDs_ || code==_Gl_
       || code==_DDL_ || code==_CDL_ || code==_CUL_ || code==_DUL_
       || (code==_UD_  && COUNTERCLOCKWISE(elem))
       || code==_UDC_ || mark==SHELF || code==_AN_UL
       || code==_ST_  || code==_XT_
       || code==_ID_  || is_lang_condition )
    {
        /* check small pair among them to prevent ending of letter on them */
        _BOOL IsSmallPair=_FALSE;
        if(code==_GD_ || (code==_UDC_ && CLOCKWISE(elem)))
        {
            p_SPECL p1st=nxt,p2nd=nxt->next;
            if(p1st->code==_UU_ || p1st->code==_UUC_ || p1st->code==_IU_)
            {
                while(!NULL_or_ZZ_this(p2nd) && !IsStrongElem(p2nd))
                    p2nd=p2nd->next;
                if(p2nd!=_NULL && NULL_or_ZZ_after(p2nd) &&
                   (p2nd->code==_ID_ || p2nd->code==_UDL_ || p2nd->code==_UDR_
                    || p2nd->code==_DDL_ || p2nd->code==_DDR_ ) )
                {
                    p_SHORT x=low_data->x, y=low_data->y;
                    _RECT   BoxOfPair;
                    GetTraceBox(x,y,p1st->ibeg,p2nd->iend,&BoxOfPair);
                    if(XMID_RECT(BoxOfPair)>TWO(YMID_RECT(BoxOfPair)))
                        IsSmallPair=_TRUE;
                }
            }
        }
        if(!IsSmallPair) MARK_XR_AS_LAST_IN_LETTER(xrd)
            return SUCCESS;
    }
    
    /* xr's, before which is potentially EOL */
    if(   NxtCode==_DUR_ || NxtCode==_CUR_
       || NxtCode==_GU_  || NxtCode==_GUs_ || NxtCode==_Gr_
       || NxtCode==_DF_  || NxtCode==_AN_UR || IsXTorST(nxt)
       || ((lang == LANGUAGE_FRENCH || lang == LANGUAGE_ITALIAN
            || lang == LANGUAGE_PORTUGUESE)
           && NxtCode==_BSS_) )
    {
        MARK_XR_AS_LAST_IN_LETTER(xrd)
        return SUCCESS;
    }
    if(   (prv->code==_UDC_       && COUNTERCLOCKWISE(prv))
       || (nxt->next->code==_UDC_ && COUNTERCLOCKWISE(nxt->next))
       || prv->code==_GU_
       || ((lang == LANGUAGE_FRENCH || lang == LANGUAGE_ITALIAN
            || lang == LANGUAGE_PORTUGUESE)
           && code==_UU_ && NxtCode==_UDC_ && COUNTERCLOCKWISE(nxt)) )
    {
        MARK_XR_AS_LAST_IN_LETTER(xrd)
        return SUCCESS;
    }
#if 0
    /* mark ID's far from BREAKs as EOL */
    if(code==_ID_ && elem->mark!=CROSS && COUNTERCLOCKWISE(elem))
    {
        p_SPECL wrk=prv->prev;
        _INT i=0;
        while(wrk!=specl && !IsAnyBreak(wrk) && i<2)
        {
            wrk=wrk->prev;
            i++;
        }
        if(i==2)
        {
            MARK_XR_AS_LAST_IN_LETTER(xrd)
            return SUCCESS;
        }
    }
#endif /* if 0 */
    
    /* check special combination with angles to put EOL */
    if((code==_UU_ || (code==_IU_ && (mark==MINW || mark==STICK))) && CLOCKWISE(elem))
    {
        if(prv->code==_ANl) MARK_XR_AS_LAST_IN_LETTER(xrd-1)
        if(nxt->code==_ANr) MARK_XR_AS_LAST_IN_LETTER(xrd)
        return SUCCESS;
    }
    if((code==_UD_ || (code==_ID_ && (mark==MINW || mark==STICK))) && COUNTERCLOCKWISE(elem))
    {
        if(prv->code==_ANl) MARK_XR_AS_LAST_IN_LETTER(xrd-1)
        if(nxt->code==_ANr) MARK_XR_AS_LAST_IN_LETTER(xrd)
        return SUCCESS;
    }
    return SUCCESS;
    
}

#endif //#ifndef LSTRIP


