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

#ifdef LSTRIP

#include "hwr_sys.h"
#include "ams_mg.h"
#include "lowlevel.h"
#include "calcmacr.h"
#include "def.h"
#include "low_dbg.h"
#include "fix32.h"
#include "xr_names.h"

#define MAX_NUM_EXTR 50
#define NORM   1
#define HOR_STR  2
#define PNT  3

#define ONE_HALF_FIX (DoubleToFix32(0.5))

#define MIN_HEIGHT 32                 //These three parameters
#define PEN_FOR_SLOPE_COEFF 2         // may be tuned
#define MAX_TANGENS  ONE_HALF_FIX

#define EXTR_DEPTH   16  // 8 for German; 12 for others


typedef struct
{
    _SHORT  x;
    _SHORT  y;
}  EXTR, _PTR p_EXTR ;

typedef  _UCHAR cb_type[CB_NUM_VERTEX*2];

Fixed32  alpha_up,alpha_dn,beta_up,beta_dn;


/* ************************************************************************* */
/* *               Fill RC field describing curved border                  * */
/* ************************************************************************* */
_SHORT FillRCNB(p_rc_type rc)
{
    _SHORT   i;
    _SHORT   xorg, yorg, xsize, ysize;
    _SHORT   xi;
    _SHORT   du, dd;
    cb_type _PTR cb;
    
    xorg  = rc->trace_rect.left;
    yorg  = rc->trace_rect.top;
    xsize = (_SHORT)DX_RECT(rc->trace_rect);
    if  ( xsize == 0 )
        xsize = 1;
    ysize = (_SHORT)DY_RECT(rc->trace_rect);
    if  ( ysize == 0 )
        ysize = 1;
    
    cb    = &(rc->curv_bord);
    
    for (i=0; i < CB_NUM_VERTEX; i ++)
    {
        xi=(_SHORT)(xorg+((_LONG)(xsize)*(_LONG)i)/(_LONG)(CB_NUM_VERTEX-1));
        //  du       = bord_u[xi]-yorg; if (du < 1) du = 1; if (du > ysize) du = ysize;
        //  dd       = bord_d[xi]-yorg; if (dd < 1) dd = 1; if (dd > ysize) dd = ysize;
        du=(_SHORT)(Fix32ToLong(alpha_up*xi+beta_up)-yorg);
        if (du < 1) du = 1; if (du > ysize) du = ysize;
        dd=(_SHORT)(Fix32ToLong(alpha_dn*xi+beta_dn)-yorg);
        if (dd < 1) dd = 1; if (dd > ysize) dd = ysize;
        (*cb)[2*i] = (_UCHAR)(((_LONG)du * 255l)/(_LONG)ysize);
        (*cb)[2*i+1] = (_UCHAR)(((_LONG)dd * 255l)/(_LONG)ysize);
    }
    
    return SUCCESS;
}
/***************************************************************************/

_BOOL straight_stroke(_SHORT ibeg, _SHORT iend, p_SHORT x, p_SHORT y,_SHORT C)
{
    _SHORT iMostFar=(_SHORT)iMostFarFromChord(x,y,ibeg,iend);
    _SHORT dxNormal=(_SHORT)(y[ibeg]-y[iend]);
    _SHORT dyNormal=(_SHORT)(x[iend]-x[ibeg]);
    _LONG ChordLenSq=dxNormal*(_LONG)dxNormal+dyNormal*(_LONG)dyNormal;
    _LONG MaxScal=(x[iMostFar]-x[ibeg])*(_LONG)dxNormal+
    (y[iMostFar]-y[ibeg])*(_LONG)dyNormal;
    TO_ABS_VALUE(MaxScal);
    if (C*MaxScal > ChordLenSq || ChordLenSq==0)
        return(_FALSE);
    else
        return(_TRUE);
}

/***************************************************************************/
/***************************************************************************/

_BOOL hor_stroke(p_SPECL cur, p_SHORT x, p_SHORT y, _SHORT n_str)

{
    _SHORT ibeg_str,iend_str,dx,dy,n_ext=0,imid,ibeg1,iend1,dx1,dy1,
    dx_beg,dy_beg,dx_end,dy_end,dx_beg1end,dy_beg1end,
    dx_begend1,dy_begend1;
    
    iend_str=cur->iend;
    cur=cur->prev;
    while (cur->mark!=BEG) { n_ext++; cur=cur->prev; }
    if (n_str>1 && n_ext>3) return(_FALSE);
    if (n_str==1 && n_ext>5) return(_FALSE);
    ibeg_str=cur->ibeg;
    dx=(_SHORT)HWRAbs(x[iend_str]-x[ibeg_str]);
    dy=(_SHORT)HWRAbs(y[iend_str]-y[ibeg_str]);
    if ( straight_stroke(ibeg_str,iend_str,x,y,5)==_TRUE && 18L*dy < 10L*dx
        //       ||  straight_stroke(ibeg_str,iend_str,x,y,4)==_TRUE && 30L*dy < 10L*dx
        )
        return(_TRUE);
    //    if (n_str==1)
    //    {
    imid=(_SHORT)ONE_HALF(ibeg_str+iend_str);
    ibeg1=(_SHORT)iMostFarFromChord(x,y,(_SHORT)ibeg_str,(_SHORT)imid);
    iend1=(_SHORT)iMostFarFromChord(x,y,(_SHORT)imid,(_SHORT)iend_str);
    dx1=(_SHORT)HWRAbs(x[iend1]-x[ibeg1]);
    dy1=(_SHORT)HWRAbs(y[iend1]-y[ibeg1]);
    dx_beg=(_SHORT)HWRAbs(x[ibeg_str]-x[ibeg1]);
    dy_beg=(_SHORT)HWRAbs(y[ibeg_str]-y[ibeg1]);
    dx_end=(_SHORT)HWRAbs(x[iend_str]-x[iend1]);
    dy_end=(_SHORT)HWRAbs(y[iend_str]-y[iend1]);
    dx_beg1end=(_SHORT)HWRAbs(x[ibeg1]-x[iend_str]);
    dy_beg1end=(_SHORT)HWRAbs(y[ibeg1]-y[iend_str]);
    dx_begend1=(_SHORT)HWRAbs(x[ibeg_str]-x[iend1]);
    dy_begend1=(_SHORT)HWRAbs(y[ibeg_str]-y[iend1]);
    
    if (straight_stroke(ibeg1,iend1,x,y,5/*5*/)==_TRUE && 20L*dy1 < 10L*dx1 &&
        dx_beg<ONE_FOURTH(dx1) && dy_beg<ONE_FOURTH(dx1) &&
        dx_end<ONE_FOURTH(dx1) && dy_end<ONE_FOURTH(dx1)
        ||
        straight_stroke(ibeg1,iend_str,x,y,5/*5*/)==_TRUE &&
        20L*dy_beg1end < 10L*dx_beg1end &&
        dx_beg<ONE_FOURTH(dx_beg1end) &&
        dy_beg<ONE_FOURTH(dx_beg1end)
        ||
        straight_stroke(ibeg_str,iend1,x,y,5/*5*/)==_TRUE &&
        20L*dy_begend1 < 10L*dx_begend1 &&
        dx_end<ONE_FOURTH(dx_begend1) &&
        dy_end<ONE_FOURTH(dx_begend1)
        )
        return(_TRUE);
    //    }
    return(_FALSE);
    
}


/***************************************************************************
 
 _BOOL pnt(_RECT box,_INT med_ampl)
 {
 if (box.right-box.left<ONE_THIRD(med_ampl) &&
 box.bottom-box.top<ONE_THIRD(med_ampl)
 )
 return(_TRUE);
 else
 return(_FALSE);
 }
 
 _SHORT classify_strokes(p_rc_type rc,p_SHORT x,p_SHORT y,p_SPECL specl)//,_INT med_ampl
 {
 p_SPECL cur=specl, wrk;
 _SHORT  ibeg_str=0, iend_str, n_str=0,
 thresh=rc->stroka.extr_depth;
 _RECT stroke_box;
 
 while (cur!=_NULL)
 {
 if (cur->mark == BEG)
 {
 ibeg_str=cur->ibeg;
 cur=cur->next;
 continue;
 }
 
 if (cur->mark==MINW || cur->mark==MAXW)
 {
 cur->attr=NORM;
 cur->code=0;
 }
 
 if (cur->mark==END)
 {
 n_str++;
 cur->attr=0;
 iend_str=cur->iend;
 GetTraceBox ( x,y,(_SHORT)ibeg_str,(_SHORT)iend_str,&stroke_box);
 if (is_umlyut(cur,stroke_box,ibeg_str,iend_str,x,y,med_ampl)==_TRUE)
 {
 cur->attr=UML;
 wrk=cur;
 while (wrk->mark!=BEG)
 {
 if (wrk->mark==MINW || wrk->mark==MAXW)
 wrk->attr=UML;
 wrk=wrk->prev;
 }
 }
 */
/*
if (hor_stroke(cur,x,y,n_str)==_TRUE)
{
    cur->attr=HOR_STR;
    wrk=cur;
    while (wrk->mark!=BEG)
    {
        if (wrk->mark==MINW || wrk->mark==MAXW)
            wrk->attr=HOR_STR;
            wrk=wrk->prev;
            }
    
    cur->ibeg=ibeg_str;
}
 */
/*
 if (is_i_point(D,cur,stroke_box,med_ampl)==_TRUE)
 {
 cur->attr=PNT;
 wrk=cur;
 while (wrk->mark!=BEG)
 {
 if (wrk->mark==MINW || wrk->mark==MAXW)
 wrk->attr=PNT;
 wrk=wrk->prev;
 }
 }
 */
/*
if (cur->prev->mark==BEG)
{
    cur->attr = (stroke_box.right-stroke_box.left<thresh) ? PNT : HOR_STR;
    cur->ibeg=ibeg_str;
}
}
if (cur->next==_NULL) break;
cur=cur->next;
}
*/
/*
 epunct=0;
 if (n_str>1)  epunct=end_punct(D,cur,med_ampl);
 if (epunct!=0)
 {
 wrk=cur;
 wrk->attr=PUNC;
 while (wrk->mark!=BEG)
 {
 if (wrk->mark==MINW || wrk->mark==MAXW)
 {
 wrk->attr=PUNC;
 }
 wrk=wrk->prev;
 }
 if (n_str>2 && epunct==2)
 {
 wrk=wrk->prev;
 wrk->attr=PUNC;
 while (wrk->mark!=BEG)
 {
 if (wrk->mark==MINW || wrk->mark==MAXW)
 {
 wrk->attr=PUNC;
 }
 wrk=wrk->prev;
 }
 }
 }
 lpunct=0;
 if (n_str-epunct>1) lpunct=lead_punct(D);
 if (lpunct!=0)
 {
 wrk=D->specl;
 while (wrk->mark!=END)
 {
 if (wrk->mark==MINW || wrk->mark==MAXW)
 {
 wrk->attr=PUNC;
 }
 wrk=wrk->next;
 }
 wrk->attr=PUNC;
 if (n_str-epunct>2 && lpunct==2)
 {
 wrk=wrk->next;
 while (wrk->mark!=END)
 {
 if (wrk->mark==MINW || wrk->mark==MAXW)
 {
 wrk->attr=PUNC;
 }
 wrk=wrk->next;
 }
 wrk->attr=PUNC;
 }
 }
 */

/***************************************************************************/

/***************************************************************************/

_SHORT extract_all_extr(p_SPECL specl, p_SHORT x, p_SHORT y,
                        _UCHAR TYPE, p_EXTR line_extr, p_SHORT pn_line_extr)
{
    p_SPECL cur=specl;
    _SHORT i=0,i_extr;
    
    while (cur != _NULL)
    {
        if (cur->mark == TYPE && cur->attr==NORM)
        {
            i_extr=cur->ipoint0;
            if (TYPE!=MAXW ||
                y[i_extr]>(_SHORT)Fix32ToLong(alpha_up*x[i_extr]+beta_up))
            {
                if (i>=MAX_NUM_EXTR)
                {
                    return(UNSUCCESS);
                }
                line_extr[i].x=x[i_extr];
                line_extr[i].y=y[i_extr];
                i++;
            }
        }
        if (cur->next==_NULL) *pn_line_extr=i;
        cur=cur->next;
    }
    return(SUCCESS);
}

/***************************************************************************/
_BOOL prev_is_too_close(p_SHORT x,p_SHORT y,p_SPECL cur,p_SPECL prv)
{
    
    _SHORT  i1=cur->ipoint0, i2=prv->ipoint0;
    _SHORT x_mid=(_SHORT)((x[i1]+x[i2])/2);
    _SHORT height=(_SHORT)(Fix32ToLong(alpha_dn*x_mid+beta_dn)-
                           Fix32ToLong(alpha_up*x_mid+beta_up));
    
    return ((HWRAbs(y[i1]-y[i2])<height/2) ? _TRUE : _FALSE);
}

/***************************************************************************/

_VOID extract_pseudo_xr(p_SPECL specl,p_SHORT x,p_SHORT y)
{
    p_SPECL cur=specl, prv_extr=_NULL;
    
    
    while (cur != _NULL)
    {
        if (cur->mark==END && (cur->attr==HOR_STR || cur->attr==PNT))
            cur->attr=PSEUDO_XR_DEF;
        
        else if ((cur->mark == MAXW || cur->mark == MINW) && cur->attr==NORM &&
                 (prv_extr==_NULL || !prev_is_too_close(x,y,cur,prv_extr))
                 )
            cur->attr = (cur->mark == MAXW) ? (_SHORT)PSEUDO_XR_MAX : (_SHORT)PSEUDO_XR_MIN;
        
        else   cur->attr=0;
        
        if ((cur->mark == MAXW || cur->mark == MINW) && cur->attr==NORM)
            prv_extr=cur;
        
        cur=cur->next;
    }
    
    return;
}

/***************************************************************************/

void calc_sums(p_EXTR extr,_SHORT n_extr,_RECT box,
               p_LONG psum_x,p_LONG psum_x_sq,p_LONG psum_xy,p_LONG psum_y)
{
    _SHORT i;
    _LONG x,y;
    _LONG sum_x=0,sum_x_sq=0,sum_xy=0,sum_y=0;
    
    for (i=0; i<n_extr; i++)
    {
        x=extr[i].x-box.left; //-mid_x;
        y=extr[i].y-box.top; //-mid_y;
        sum_x+=x;
        sum_x_sq+=x*x;
        sum_xy+=x*y;
        sum_y+=y;
    }
    
    *psum_x=sum_x;
    *psum_x_sq=sum_x_sq;
    *psum_xy=sum_xy;
    *psum_y=sum_y;
    return;
}

/***************************************************************************/
/***************************************************************************/

_SHORT StrokaN(p_rc_type rc, p_SPECL specl)
{
    _SHORT n_str,n_min,n_max,retval=UNSUCCESS;
    p_VOID mem_segm;
    p_EXTR line_min, line_max;
    _LONG sum_x_min,sum_x_sq_min,sum_xy_min,sum_y_min,
    sum_x_max,sum_x_sq_max,sum_xy_max,sum_y_max,
    slope_coeff_min,slope_coeff_max;
    _RECT  box=rc->trace_rect;
    _SHORT mid_x=(_SHORT)ONE_HALF(box.left+box.right),
    /*mid_y,*/mid_d_bord,mid_u_bord,mid_height;
    _SHORT left_x=box.left, right_x=box.right,
    left_d_bord, right_d_bord, left_u_bord, right_u_bord;
    _SHORT shift,delta_x;
    _UCHAR bad_bord=0;
    _LONG znam;
    
    p_SHORT x,y;
    
    
    mem_segm = HWRMemoryAlloc(2L*MAX_NUM_EXTR*sizeof(EXTR) + 2L*sizeof(_SHORT)*rc->ii);
    if (mem_segm==_NULL) goto EXIT_FREE;
    line_max=(p_EXTR)mem_segm;
    line_min=line_max+MAX_NUM_EXTR;
    HWRMemSet(line_max,0,2*sizeof(EXTR)*MAX_NUM_EXTR);
    x=(p_SHORT)(line_min+MAX_NUM_EXTR);
    y=x+rc->ii;
    
    trace_to_xy(x,y,rc->ii,rc->trace);
    
    n_str=classify_strokes(rc,x,y,specl);
    
    if (extract_all_extr(specl,x,y,MINW,line_min,&n_min)==UNSUCCESS)
        goto EXIT_FREE;
    if (rc->stroka.pos_sure_in>=75 && rc->stroka.size_sure_in>=75
        && n_min<MAX_NUM_EXTR-1
        //     && n_min<=3
        //     || D->rc->stroka.pos_sure_in==100 && D->rc->stroka.size_sure_in==100 && n_min<MAX_NUM_EXTR-1
        )
    {
        line_min[n_min].y=(_SHORT)(rc->stroka.dn_pos_in-rc->stroka.size_in);
        line_min[n_min].x=left_x;
        n_min++;
        
        line_min[n_min].y=(_SHORT)(rc->stroka.dn_pos_in-rc->stroka.size_in);
        line_min[n_min].x=right_x;
        n_min++;
        
        if (rc->stroka.pos_sure_in==100 && rc->stroka.size_sure_in==100 && n_min<MAX_NUM_EXTR-1)
        {
            line_min[n_min] = line_min[n_min-2]; n_min ++;
            line_min[n_min] = line_min[n_min-2]; n_min ++;
        }
        
    }
    
    if (n_min>0)
    {
        calc_sums(line_min,n_min,rc->trace_rect,&sum_x_min,&sum_x_sq_min,&sum_xy_min,&sum_y_min);
        slope_coeff_min=PEN_FOR_SLOPE_COEFF*sum_x_min/n_min; slope_coeff_min*=slope_coeff_min;
        znam=n_min*sum_x_sq_min-sum_x_min*sum_x_min+n_min*slope_coeff_min;
        if (znam==0) znam=1;
        alpha_up = LongToFix32(n_min*sum_xy_min-sum_x_min*sum_y_min)/znam;
        
        beta_up =  (LongToFix32(sum_y_min)-alpha_up*sum_x_min)/n_min;
        beta_up += (LongToFix32(box.top)-alpha_up*left_x); //Inverse shift
    }
    else
    {
        alpha_up=0;
        beta_up=LongToFix32(box.top);
    }
    
    
    if (extract_all_extr(specl,x,y,MAXW,line_max,&n_max)==UNSUCCESS)
        goto EXIT_FREE;
    
    if (rc->stroka.pos_sure_in>=75
        && n_max<MAX_NUM_EXTR-1
        //      && n_max<=3
        //      || D->rc->stroka.pos_sure_in==100 && n_max<MAX_NUM_EXTR-1
        )
    {
        line_max[n_max].y=rc->stroka.dn_pos_in;
        line_max[n_max].x=left_x;
        n_max++;
        line_max[n_max].y=rc->stroka.dn_pos_in;
        line_max[n_max].x=right_x;
        n_max++;
        
        if (rc->stroka.pos_sure_in==100 && n_max<MAX_NUM_EXTR-1)
        {
            line_max[n_max] = line_max[n_max-2]; n_max ++;
            line_max[n_max] = line_max[n_max-2]; n_max ++;
        }
    }
    
    if (n_max>0)
    {
        calc_sums(line_max,n_max,rc->trace_rect,&sum_x_max,&sum_x_sq_max,&sum_xy_max,&sum_y_max);
        slope_coeff_max=PEN_FOR_SLOPE_COEFF*sum_x_max/n_max; slope_coeff_max*=slope_coeff_max;
        znam=n_max*sum_x_sq_max-sum_x_max*sum_x_max+n_max*slope_coeff_max;
        if (znam==0) znam=1;
        alpha_dn =  LongToFix32(n_max*sum_xy_max-sum_x_max*sum_y_max)/znam;
        
        beta_dn =  ( LongToFix32(sum_y_max)-alpha_dn*sum_x_max)/n_max;
        beta_dn += ( LongToFix32(box.top)-alpha_dn*left_x); //Inverse shift
    }
    else
    {
        alpha_dn=0;
        beta_dn=LongToFix32(box.bottom);
    }
    
    left_d_bord=(_SHORT)Fix32ToLong(alpha_dn*left_x+beta_dn);
    right_d_bord=(_SHORT)Fix32ToLong(alpha_dn*right_x+beta_dn);
    left_u_bord=(_SHORT)Fix32ToLong(alpha_up*left_x+beta_up);
    right_u_bord=(_SHORT)Fix32ToLong(alpha_up*right_x+beta_up);
    
    
    if (left_d_bord<left_u_bord+MIN_HEIGHT)
    {
        bad_bord=1;
        shift=(_SHORT)((left_u_bord+MIN_HEIGHT-left_d_bord)/2+1);
        //     if (2*shift<MIN_HEIGHT) shift=MIN_HEIGHT/2;
        left_u_bord-=shift;
        left_d_bord+=shift;
    }
    if (right_d_bord<right_u_bord+MIN_HEIGHT)
    {
        bad_bord=1;
        shift=(_SHORT)((right_u_bord+MIN_HEIGHT-right_d_bord)/2+1);
        //     if (2*shift<MIN_HEIGHT) shift=MIN_HEIGHT/2;
        right_u_bord-=shift;
        right_d_bord+=shift;
    }
    
    if (bad_bord==1)
    {
        delta_x=(_SHORT)HWRMax((right_x-left_x),1);
        alpha_up=LongToFix32((_LONG)(right_u_bord-left_u_bord))/delta_x;
        alpha_dn=LongToFix32((_LONG)(right_d_bord-left_d_bord))/delta_x;
        beta_up=
        LongToFix32((_LONG)left_u_bord*right_x-(_LONG)right_u_bord*left_x)/delta_x;
        beta_dn=
        LongToFix32((_LONG)left_d_bord*right_x-(_LONG)right_d_bord*left_x)/delta_x;
    }
    
    mid_d_bord=(_SHORT)Fix32ToLong(alpha_dn*mid_x+beta_dn);
    mid_u_bord=(_SHORT)Fix32ToLong(alpha_up*mid_x+beta_up);
    
    if (alpha_up>MAX_TANGENS) {bad_bord=2; alpha_up=MAX_TANGENS;}
    if (alpha_up<-MAX_TANGENS) {bad_bord=2; alpha_up=-MAX_TANGENS;}
    if (alpha_dn>MAX_TANGENS) {bad_bord=2; alpha_dn=MAX_TANGENS;}
    if (alpha_dn<-MAX_TANGENS) {bad_bord=2; alpha_dn=-MAX_TANGENS;}
    if (bad_bord==2)
    {
        beta_up=LongToFix32((_LONG)mid_u_bord)-alpha_up*mid_x;
        beta_dn=LongToFix32((_LONG)mid_d_bord)-alpha_dn*mid_x;
        
        left_d_bord=(_SHORT)Fix32ToLong(alpha_dn*left_x+beta_dn);
        right_d_bord=(_SHORT)Fix32ToLong(alpha_dn*right_x+beta_dn);
        left_u_bord=(_SHORT)Fix32ToLong(alpha_up*left_x+beta_up);
        right_u_bord=(_SHORT)Fix32ToLong(alpha_up*right_x+beta_up);
    }
    
    extract_pseudo_xr(specl,x,y);
    
    /*
     TRANSFORM:
     
     for (i=0; i<D->ii; i++)
     {
     bord_d=Fix32ToLong(alpha_dn*D->x[i]+beta_dn);
     bord_u=Fix32ToLong(alpha_up*D->x[i]+beta_up);
     
     if (D->y[i] == BREAK) continue;
     D->y[i]=(_SHORT)(STR_UP+
     (_LONG)(STR_DOWN-STR_UP)*
     (D->y[i]-bord_u)/
     HWRMax(1,bord_d-bord_u));
     }
     
     for (i=0; i<D->ii; i++)
     if (D->y[i] != BREAK)
     D->x[i]=(_SHORT)((STR_DOWN-STR_UP)+
     (_LONG)(STR_DOWN-STR_UP)*
     (D->x[i]-D->box.left)/
     HWRMax(1,mid_height));
     */
    
    mid_x=(_SHORT)ONE_HALF(rc->trace_rect.left+rc->trace_rect.right);
    mid_d_bord=(_SHORT)Fix32ToLong(alpha_dn*mid_x+beta_dn);
    mid_u_bord=(_SHORT)Fix32ToLong(alpha_up*mid_x+beta_up);
    mid_height=(_SHORT)(mid_d_bord-mid_u_bord);
    
    rc->stroka.size_out =(_SHORT)mid_height;
    rc->stroka.dn_pos_out =(_SHORT)mid_d_bord;
    
    
    if (bad_bord==0 && n_min>=5 && n_max>=5
        && rc->stroka.pos_sure_in<100
        )
    {
        rc->stroka.size_sure_out=75;
        rc->stroka.pos_sure_out=75;
    }
    else
    {
        rc->stroka.size_sure_out=0;
        rc->stroka.pos_sure_out=0;
    }
    
    if  ( FillRCNB(rc) == UNSUCCESS )
        goto  EXIT_FREE;
    
    retval = SUCCESS;
    
EXIT_FREE:
    
    if (mem_segm!=_NULL) HWRMemoryFree(mem_segm);
    
    return(retval);
    
}



_UCHAR AddElem(_SHORT ibeg,_SHORT ipnt,_SHORT iend,_UCHAR type,p_SPECL specl,p_SHORT pnelem)
{
    _SHORT nelem=*pnelem;
    if (nelem>=SPECVAL) return(EMPTY);
    p_SPECL cur=specl+nelem;
    if (nelem>0) { cur->prev=cur-1; cur->prev->next=cur; }
    else  cur->prev=NULL;
    cur->next=_NULL;
    cur->ibeg=ibeg;
    cur->ipoint0=ipnt;
    cur->iend=iend;
    cur->mark=type;
    cur->attr=NORM;  /**/
    
    *pnelem=(_SHORT)(nelem+1);
    return(type);
}


_VOID FindExtr(p_PS_point_type trace, _SHORT N, _SHORT Eps, p_SPECL specl)
{
    _UCHAR LastElemAdd;
    _SHORT nelem=0;
    _SHORT i,j,val,CurMax,CurMin,iCurMax,iCurMin,iCurBeg;
    
    LastElemAdd=EMPTY;
    
    for (i=0; i<N; i++)
    {
        val=trace[i].y;
        
        if (val==BREAK)
        {
            if (LastElemAdd==MINW)
            {
                for (j=iCurMax; j>=0; j--)
                    if (CurMax-trace[j].y>Eps) break;
                LastElemAdd=AddElem(j,iCurMax,i-1,MAXW,specl,&nelem);
            }
            else if (LastElemAdd==MAXW)
            {
                for (j=iCurMin; j>=0; j--)
                    if (trace[j].y-CurMin>Eps) break;
                LastElemAdd=AddElem(j,iCurMin,i-1,MINW,specl,&nelem);
            }
            
            if (i>0 && trace[i-1].y!=BREAK)
                LastElemAdd=AddElem(i-1,i-1,i-1,END,specl,&nelem);
            
            continue;
        }
        else if (i==0 || trace[i-1].y==BREAK)
        {
            LastElemAdd=AddElem(i,i,i,BEG,specl,&nelem);
            iCurBeg=i;
            CurMax=val;  CurMin=val;
            iCurMax=i;    iCurMin=i;
            continue;
        }
        
        switch (LastElemAdd)
        {
            case BEG :
                if(val<CurMin) {CurMin = val; iCurMin=i;}
                if(val>CurMax) {CurMax = val; iCurMax=i;}
                break;
            case MINW :
                if(val<CurMin) {CurMin = val; iCurMin=i;}
                if(val>CurMax) {CurMax = val; iCurMax=i; CurMin=val; iCurMin=i;}
                break;
            case MAXW :
                if(val<CurMin) {CurMin = val; iCurMin=i; CurMax = val; iCurMax=i;}
                if(val>CurMax) {CurMax = val; iCurMax=i;}
                break;
        }
        
        if (CurMax-CurMin>Eps)
        {
            switch (LastElemAdd)
            {
                case BEG :
                    LastElemAdd = (iCurMax>iCurMin) ?
                    AddElem(iCurBeg,iCurMin,i,MINW,specl,&nelem) :
                    AddElem(iCurBeg,iCurMax,i,MAXW,specl,&nelem);
                    break;
                case MINW :
                    for (j=iCurMax; j>=0; j--)
                        if (CurMax-trace[j].y>Eps) break;
                    LastElemAdd = AddElem(j,iCurMax,i,MAXW,specl,&nelem);
                    break;
                case MAXW :
                    for (j=iCurMin; j>=0; j--)
                        if (trace[j].y-CurMin>Eps) break;
                    LastElemAdd = AddElem(j,iCurMin,i,MINW,specl,&nelem);
                    break;
            }
            CurMin=val; CurMax=val;
            iCurMin=i;  iCurMax=i;
        }
    }
    
    return;
    
}



_VOID insert_pseudo_xr(xrd_el_type _PTR xrd_num_elem, xrinp_type _PTR pxr,
                       p_SPECL cur, PS_point_type _PTR trace)
{
    _RECT box;
    
    HWRMemCpy(&(xrd_num_elem->xr),pxr,sizeof(xrinp_type));
    xrd_num_elem->hotpoint=cur->ipoint0;
    xrd_num_elem->begpoint=cur->ibeg;
    xrd_num_elem->endpoint=cur->iend;
    GetBoxFromTrace( trace,
                    xrd_num_elem->begpoint, xrd_num_elem->endpoint,
                    &box );
    xrd_num_elem->box_left=box.left;
    xrd_num_elem->box_up=box.top;
    xrd_num_elem->box_right=box.right;
    xrd_num_elem->box_down=box.bottom;
    xrd_num_elem->location=0;
    return;
}

/************************************************************************** */

_SHORT form_pseudo_xr_data(p_SPECL specl, p_PS_point_type trace, xrdata_type _PTR xrdata)
{
    p_SPECL  cur;
    xrd_el_type _PTR xrd = &(*xrdata->xrd)[0];
    xrinp_type  pseudo_xr, FF_xr;
    _SHORT num_elem;
    
    
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
    
    insert_pseudo_xr(&xrd[0],&FF_xr,specl,trace);
    num_elem=1;
    
    for (cur=specl; cur!=NULL; cur=cur->next)
    {
        if (cur->attr==PSEUDO_XR_MIN || cur->attr==PSEUDO_XR_MAX || cur->attr==PSEUDO_XR_DEF)
        {
            pseudo_xr.type=cur->attr;
            insert_pseudo_xr(&xrd[num_elem],&pseudo_xr,cur,trace);
            num_elem++;
        }
        if (cur->mark==END)
        {
            insert_pseudo_xr(&xrd[num_elem],&FF_xr,cur,trace);
            num_elem++;
        }
        
    }
    xrdata->len=num_elem;
    HWRMemSet(&xrd[num_elem],0,sizeof(xrd[0]));
    return (SUCCESS);
}



/* *************************************************************** */


_SHORT low_level (PS_point_type _PTR trace, xrdata_type _PTR xrdata, rc_type _PTR rc)
{
    p_SPECL specl=NULL;
    
    xrdata->len = 0;
    
    if (rc->ii < 3)
    {
        return UNSUCCESS;
    }
    
    if ((specl=(p_SPECL)HWRMemoryAlloc((_ULONG)SPECVAL*sizeof(SPECL)))==_NULL)
    {
        return UNSUCCESS;
    }
    
    GetBoxFromTrace(trace,0,rc->ii-1,&(rc->trace_rect));
    
    rc->stroka.extr_depth=(_SHORT)HWRMax(2,(_LONG)EXTR_DEPTH*DY_RECT(rc->trace_rect)/(STR_DOWN-LIN_UP));
    
    FindExtr(trace,rc->ii,rc->stroka.extr_depth,specl);
    
    
    StrokaN(rc,specl);
    
    form_pseudo_xr_data(specl, trace, xrdata);
    
    if (specl!=_NULL) HWRMemoryFree(specl);
    
    return SUCCESS;
    
}

#endif //#ifdef LSTRIP




