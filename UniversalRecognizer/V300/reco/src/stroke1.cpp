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

#include "ams_mg.h"
#include "stroke1.h"
#include "langid.h"
#include "reco.h"
#ifndef NULL
#define NULL _NULL
#endif

static _BOOL non_super(p_EXTR line_min, _INT i,p_SHORT x,p_SHORT y,p_SHORT bord_d, _INT lang );
static _VOID glitch_to_super_min(p_EXTR line_min, _INT n_line_min,p_SHORT bord_d,
                                 _INT max_height,p_SHORT x,p_SHORT y,_BOOL gl_up_left, _INT lang );
static _VOID all_susp_extr(p_EXTR extr,_INT n_extr,_INT n_allextr,_UCHAR type,p_SHORT y,
                           _INT base_ampl,_INT base_ampl_sure,_INT max_ampl,
                           p_SHORT bord_d,_INT ft_height, _INT lang );


#ifndef LSTRIP

#ifndef __JUST_FIRST_PART

/***************************************************************************/

_VOID glitch_to_super_min(p_EXTR line_min, _INT n_line_min,p_SHORT bord_d,
                          _INT max_height,p_SHORT x,p_SHORT y,_BOOL gl_up_left, _INT lang )
{
    _INT i;
    _LONG  P=55;//50;   //55;    //60;  //50
    
    
    for (i=0; i<n_line_min; i++)
    {
        if (line_min[i].susp==GLITCH_UP)
        {
            if ((line_min[i].pspecl->attr==NORM || line_min[i].pspecl->attr==I_MIN )&&
                (gl_up_left==_TRUE ||
                 (non_super(line_min,i,x,y,bord_d, lang)==_FALSE &&
                  100L*(bord_d[line_min[i].i]-line_min[i].y) >= P*max_height) ) )
            {
                line_min[i].susp = SUPER_SCRIPT ;
            }
            else
            {
                line_min[i].susp = 0;
            }
        }
        if (line_min[i].susp==DBL_GLITCH_UP && i<n_line_min-1 )
        {
            if ((line_min[i].pspecl->attr==NORM || line_min[i].pspecl->attr==I_MIN )&&
                (line_min[i+1].pspecl->attr==NORM || line_min[i+1].pspecl->attr==I_MIN )&&
                (gl_up_left==_TRUE ||
                 (non_super(line_min,i,x,y,bord_d, lang)==_FALSE &&
                  non_super(line_min,i+1,x,y,bord_d, lang)==_FALSE &&
                  100L*(bord_d[line_min[i].i]-line_min[i].y) >= P*max_height &&
                  100L*(bord_d[line_min[i+1].i]-line_min[i+1].y) >= P*max_height) ) )
            {
                line_min[i].susp = line_min[i+1].susp = SUPER_SCRIPT ;
            }
            else
            {
                line_min[i].susp = line_min[i+1].susp = 0;
            }
            i++;
        }
        if (line_min[i].susp==TRP_GLITCH_UP && i<n_line_min-2 )
        {
            if ((line_min[i].pspecl->attr==NORM || line_min[i].pspecl->attr==I_MIN )&&
                (line_min[i+1].pspecl->attr==NORM || line_min[i+1].pspecl->attr==I_MIN )&&
                (line_min[i+2].pspecl->attr==NORM || line_min[i+2].pspecl->attr==I_MIN )&&
                (gl_up_left==_TRUE ||
                 (non_super(line_min,i,x,y,bord_d, lang)==_FALSE &&
                  non_super(line_min,i+1,x,y,bord_d, lang)==_FALSE &&
                  non_super(line_min,i+2,x,y,bord_d, lang)==_FALSE &&
                  100L*(bord_d[line_min[i].i]-line_min[i].y) >= P*max_height &&
                  100L*(bord_d[line_min[i+1].i]-line_min[i+1].y) >= P*max_height &&
                  100L*(bord_d[line_min[i+2].i]-line_min[i+2].y) >= P*max_height) ) )
            {
                line_min[i].susp = line_min[i+1].susp = line_min[i+2].susp = SUPER_SCRIPT ;
            }
            else
            {
                line_min[i].susp = line_min[i+1].susp = line_min[i+2].susp = 0;
            }
            i+=2;
        }
    }
    return;
}

/***************************************************************************/

static _BOOL non_super(p_EXTR line_min, _INT i,p_SHORT x,p_SHORT y,p_SHORT bord_d, _INT lang )
{
    p_SPECL cur=line_min[i].pspecl, nxt=cur->next, prv=cur->prev;
    
    if (lang == LANGUAGE_GERMAN)
        return(_FALSE);
    if (cur->code==RET_ON_LINE)
        return(_TRUE);
    if ( i > 3 && prv->mark==MAXW && nxt->mark==MAXW &&
        x[cur->iend]-x[cur->ibeg] > THREE(HWRMax(y[cur->ibeg],y[cur->iend])-y[cur->ipoint0]) &&
        y[nxt->ipoint0]>ONE_THIRD(line_min[i].y)+TWO_THIRD(bord_d[line_min[i].i]) &&
        y[prv->ipoint0]>ONE_THIRD(line_min[i].y)+TWO_THIRD(bord_d[line_min[i].i]) )
    {
        return(_TRUE);
    }
    return(_FALSE);
}

/***************************************************************************/

_BOOL sub_max_to_line(low_type _PTR D,p_EXTR line_max,p_INT pn_line_max, p_SHORT bord_d,_INT max_dist)
{
    p_SPECL cur = D->specl;
    p_SHORT y = D->buffers[1].ptr, i_back = D->buffers[2].ptr;
    _INT    dist;
    _BOOL   ins = _FALSE;
    _LONG   P = 35; //40;
    
    while (cur!=_NULL)
    {
        if (cur->mark==MAXW && cur->code==SUB_SCRIPT)
        {
            dist=y[cur->ipoint0]-bord_d[i_back[cur->ipoint0]];
            if (100L*dist<P*max_dist)
            {
                ins=insert_line_extr(D,cur,line_max,pn_line_max);
                cur->code=RET_ON_LINE;
            }
        }
        cur=cur->next;
    }
    return(ins);
}

/***************************************************************************/

_VOID super_min_to_line(p_EXTR line_min, _INT n_line_min, p_SHORT bord_d,
                        _INT max_line_height,_INT mid_line_height, p_INT pn_super)
{
    _INT i;
    for (i=0; i<n_line_min; i++)
    {
        if (line_min[i].susp==SUPER_SCRIPT &&
            bord_d[line_min[i].i] - line_min[i].y <= THREE_FOURTH(max_line_height)+ONE_FOURTH(mid_line_height) )
        {
            line_min[i].susp=RET_ON_LINE;
            (*pn_super)--;
        }
    }
    return;
}

/***************************************************************************/

_VOID spec_neibour_extr(p_EXTR extr,_INT n_extr,_UCHAR type,_INT LINE_POS)
{
    _INT i, OUTSIDE_LINE = ON_LINE;
    if (type ==MAXW)
        OUTSIDE_LINE = SUB_SCRIPT;
    else if (type==MINW)
        OUTSIDE_LINE = SUPER_SCRIPT;
    
    for (i=1; i<n_extr; i++)
    {
        if (extr[i].susp==OUTSIDE_LINE && extr[i-1].susp==INSIDE_LINE)
        {
            if (LINE_POS==STR_TOO_NARROW)
                ret_to_line(extr,n_extr,i,i);
            else
                ret_to_line(extr,n_extr,i,i-1);
        }
        if (extr[i].susp==INSIDE_LINE && extr[i-1].susp==OUTSIDE_LINE)
        {
            if (LINE_POS==TOO_WIDE)
                ret_to_line(extr,n_extr,i,i);
            else
                ret_to_line(extr,n_extr,i,i-1);
        }
    }
    return;
}

/***************************************************************************/

_INT neibour_susp_extr(p_EXTR extr,_INT n_extr,_UCHAR type,p_SHORT bord_d,
                       _INT ft_height)
{
    _INT		i, n_unsusp=0, mid_height=0, cur_height =0, prv_height=0;
    _INT		OUTSIDE_LINE = ON_LINE;
    _LONG       sum_height=0L;
    EXTR		cur_extr,prv_extr;
    
    if (type==MAXW)
        OUTSIDE_LINE = SUB_SCRIPT;
    if (type==MINW)
        OUTSIDE_LINE = SUPER_SCRIPT;
    for (i=0; i<n_extr; i++)
        if (extr[i].susp!=OUTSIDE_LINE && extr[i].susp!=INSIDE_LINE) n_unsusp++;
    if (n_unsusp>1)
    {
        for (i=0; i<n_extr; i++)
        {
            if (extr[i].susp!=OUTSIDE_LINE && extr[i].susp!=INSIDE_LINE)
            {
                if (type==MAXW) cur_height=extr[i].y;
                if (type==MINW) cur_height=bord_d[extr[i].i]-extr[i].y;
                sum_height+=(_LONG)cur_height;
            }
        }
        mid_height = (_INT)(sum_height/n_unsusp);
    }
    if (n_unsusp<=1) return(UNSUCCESS);
    
    for (i=1; i<n_extr; i++)
    {
        if ((extr[i].susp==OUTSIDE_LINE && extr[i-1].susp==INSIDE_LINE) ||
            (extr[i].susp==INSIDE_LINE && extr[i-1].susp==OUTSIDE_LINE) )
        {
            cur_extr=extr[i];
            prv_extr=extr[i-1];
            /*
             if (mid_height==0 && n_extr>2)
             {
             for (j=0; j<n_extr; j++)
             {
             if (j==i-1 || j==i) continue;
             if (type==MAXW) cur_height=extr[j].y;
             if (type==MINW) cur_height=bord_d[extr[j].i]-extr[j].y;
             sum_height+=(_LONG)cur_height;
             }
             mid_height = sum_height/(n_extr-2);
             }
             */
            if (type==MAXW)
            {
                cur_height=cur_extr.y;
                prv_height=prv_extr.y;
            }
            if (type==MINW)
            {
                cur_height=bord_d[cur_extr.i]-cur_extr.y;
                prv_height=bord_d[prv_extr.i]-prv_extr.y;
                if (ft_height>0 && cur_extr.susp==OUTSIDE_LINE && cur_height>=ft_height)
                {
                    ret_to_line(extr,n_extr,i,i-1);
                    continue;
                }
                if (ft_height>0 && prv_extr.susp==OUTSIDE_LINE && prv_height>=ft_height)
                {
                    ret_to_line(extr,n_extr,i,i);
                    continue;
                }
            }
            if (HWRAbs(cur_height-mid_height) < HWRAbs(prv_height-mid_height) )
                ret_to_line(extr,n_extr,i,i);
            else
                ret_to_line(extr,n_extr,i,i-1);
        }
    }
    return(SUCCESS);
}

/***************************************************************************/

_VOID all_susp_extr(p_EXTR extr,_INT n_extr,_INT n_allextr,_UCHAR type,p_SHORT y,
                    _INT base_ampl,_INT base_ampl_sure,_INT max_ampl,p_SHORT bord_d,
                    _INT ft_height, _INT lang)
{
    _INT	i,ind_wrk,ind_ret,OUTSIDE_LINE= ON_LINE,ampl_l=0,ampl,ret_ampl=0;
    p_SPECL wrk=_NULL;
    _LONG	CONST_NARROW;
    
    if (lang == LANGUAGE_GERMAN)
    {
        CONST_NARROW = 10;
    }
    else
    {
        CONST_NARROW = 8;
    }
    if (type==MINW)
    {
        OUTSIDE_LINE=SUPER_SCRIPT;
    }//sign=-1; opp_type=MAXW;}
    if (type==MAXW)
    {
        OUTSIDE_LINE=SUB_SCRIPT;
    }//sign=1; opp_type=MINW;}
    for (i=1,ampl=0; i<n_extr; i++)
    {
        if ((extr[i].susp==OUTSIDE_LINE && extr[i-1].susp==INSIDE_LINE) ||
            (extr[i].susp==INSIDE_LINE && extr[i-1].susp==OUTSIDE_LINE) )
        {
            if (extr[i].susp==INSIDE_LINE)
            {
                ind_wrk=i;
                ind_ret=i-1;
            }
            else
            {
                ind_wrk=i-1;
                ind_ret=i;
            }
            wrk=extr[ind_wrk].pspecl;
            if ( type==MAXW && wrk->prev->mark==BEG )
            {
                ret_ampl=calc_ampl(extr[ind_ret],y,type);
                if (ret_ampl<THREE_HALF(base_ampl))
                {
                    ret_to_line(extr,n_extr,i,ind_ret);
                    continue;
                }
                
            }
            if (type==MAXW)
            {
                if (ampl!=0)
                    ampl_l=ampl;
                else
                    ampl_l=calc_ampl(extr[i-1],y,type);
                ampl=calc_ampl(extr[i],y,type);
            }
            if (type==MINW)
            {
                ampl_l=bord_d[extr[i-1].i]-extr[i-1].y;
                ampl=bord_d[extr[i].i]-extr[i].y;
                if (ft_height>0 && extr[i].susp==OUTSIDE_LINE && ampl>=ft_height)
                {
                    ret_to_line(extr,n_extr,i,i-1);
                    continue;
                }
                if (ft_height>0 && extr[i-1].susp==OUTSIDE_LINE && ampl_l>=ft_height)
                {
                    ret_to_line(extr,n_extr,i,i);
                    continue;
                }
            }
            if (CONST_NARROW*ampl_l<=max_ampl)
            {
                ret_to_line(extr,n_extr,i,i);
                continue;
            }
            if (CONST_NARROW*ampl<=max_ampl)
            {
                ret_to_line(extr,n_extr,i,i-1);
                continue;
            }
            if (lang != LANGUAGE_GERMAN && (HWRAbs(ampl-base_ampl) < HWRAbs(ampl_l-base_ampl)))
            {
                ret_to_line(extr,n_extr,i,i);
            }
            else if ((base_ampl*base_ampl<ampl*ampl_l && ampl<ampl_l) ||
                     (base_ampl*base_ampl>=ampl*ampl_l && ampl>=ampl_l) )
            {
                ret_to_line(extr,n_extr,i,i);
            }
            else
            {
                ret_to_line(extr,n_extr,i,i-1);
            }
        }
        else ampl=0;
    }
    return;
}

/**************************************************************************/

_INT calc_ampl(EXTR extr, p_SHORT y, _UCHAR type)
{
    _INT    ampl, sign = (type==MINW)? -1 : 1, opp_type = (type==MINW) ? MAXW : MINW;
    p_SPECL prv = extr.pspecl->prev;
    p_SPECL nxt = extr.pspecl->next;
    _INT prv_ampl = (prv->mark==opp_type && (prv->attr==NORM /*|| prv->attr==NON_SUPER*/ || prv->attr==I_MIN)) ? sign*(extr.y-y[prv->ipoint0]) : 0;
    _INT nxt_ampl = (nxt->mark==opp_type && (nxt->attr==NORM /*|| nxt->attr==NON_SUPER*/ || nxt->attr==I_MIN)) ? sign*(extr.y-y[nxt->ipoint0]) : 0;
    
    if (prv_ampl!=0 && nxt_ampl!=0)
    {
        if (nxt->next->mark!=END && prv->prev->mark!=BEG)
            ampl=MEAN_OF(prv_ampl,nxt_ampl);
        else
            ampl=HWRMax(prv_ampl,nxt_ampl);
    }
    else
    {
        ampl=HWRMax(prv_ampl,nxt_ampl);
    }
    if (ampl==0)
    {
        prv_ampl = (prv->mark==opp_type) ? sign*(extr.y-y[prv->ipoint0]) : 0;
        nxt_ampl = (nxt->mark==opp_type) ? sign*(extr.y-y[nxt->ipoint0]) : 0;
        ampl = THREE_FOURTH(HWRMax(prv_ampl,nxt_ampl));
    }
    return(ampl);
}

/***************************************************************************/

_VOID ret_to_line(p_EXTR extr,_INT n_extr,_INT i,_INT ind)
{
    if (ind==i)
    {
        extr[i].susp=(_SHORT)(-extr[i].susp);
        if (i+1<n_extr && extr[i+1].susp==-extr[i].susp)
        {
            extr[i+1].susp=extr[i].susp;
            if (i+2<n_extr && extr[i+2].susp==-extr[i].susp)
                extr[i+2].susp=extr[i].susp;
        }
    }
    if (ind==i-1)
    {
        extr[i-1].susp=(_SHORT)(-extr[i-1].susp);
        if (i-2>=0 && extr[i-2].susp==-extr[i-1].susp)
        {
            extr[i-2].susp=extr[i-1].susp;
            if (i-3>=0 && extr[i-3].susp==-extr[i-1].susp)
                extr[i-3].susp=extr[i-1].susp;
        }
    }
    return;
}

/***************************************************************************/

_BOOL delete_line_extr(p_EXTR line_extr, p_INT pn_extr, _INT MARK)
{
    _INT i,j,n_extr=*pn_extr;
    for (i=0; i<n_extr; i++)
    {
        if (line_extr[i].susp==MARK)
        {
            for (j=i; j<n_extr; j++)
                line_extr[j]=line_extr[j+1];
            n_extr--;
            i--;
        }
    }
    *pn_extr=n_extr;
    return(_TRUE);
}

/***************************************************************************/

_BOOL insert_line_extr(low_type _PTR D, p_SPECL cur, p_EXTR extr,
                       p_INT pn_extr)
{
    p_SHORT x=D->buffers[0].ptr, y=D->buffers[1].ptr, i_back=D->buffers[2].ptr;
    _INT    i=0, j, i_extr, n_extr=*pn_extr, shift;
    p_SPECL wrk;
    
    wrk=cur->next;
    while (wrk->mark!=END)
        wrk=wrk->next;
    shift=wrk->attr;
    
    while (i<n_extr && extr[i].x < x[cur->ipoint0])
        i++;
    for (j=n_extr-1; j>=i; j--)
        extr[j+1]=extr[j];
    i_extr=cur->ipoint0;
    extr[i].x=x[i_extr];
    extr[i].y=y[i_extr];
    extr[i].i=i_back[i_extr];
    extr[i].susp=RET_ON_LINE;
    extr[i].shift=(_SHORT)shift;
    extr[i].pspecl=cur;
    (*pn_extr)++;
    return(_TRUE);
}

/***************************************************************************/

_INT calc_average(p_SHORT array, _INT N)
{
    _INT i, aver;
    _LONG sum=0;
    if (N<=0) return(UNSUCCESS);
    for (i=0; i<N; i++)
        sum+=array[i];
    aver=(_INT)(sum/N);
    return(aver);
}

/***************************************************************************/

_INT mid_other(p_EXTR extr,_INT n_extr,_INT i)
{
    _INT    j;
    _LONG   sum_other=0L;
    
    if (n_extr<=2)
        return (UNSUCCESS);
    for (j=0; j<n_extr; j++)
    {
        if (j==i-1 || j==i)
            continue;
        sum_other+=extr[j].y;
    }
    return (_INT)((sum_other/(n_extr-2)));
}

/***************************************************************************/

_SHORT point_of_smooth_bord(_INT cur_point,_INT n_extr,p_EXTR extr, low_type _PTR D,_INT should)
{
    _INT    jl, jr, j, k, i, max_ind, mid_y, dx;
    _INT    x_left  = D->x[cur_point] - should, x_right = D->x[cur_point] + should;
    _INT    wextr_x[NUM_EXTR_IN_SHOULD], wextr_y[NUM_EXTR_IN_SHOULD];
    _LONG   dy, integ;
    
    j=0;
    while (j<n_extr && extr[j].x <= x_left)
        j++;
    jl=j-1;
    while (j<n_extr && extr[j].x < x_right)
        j++;
    jr=j;
    max_ind=jr-jl;
    
    if (max_ind+1 > NUM_EXTR_IN_SHOULD)
    {
        max_ind=NUM_EXTR_IN_SHOULD-1;
    }
    
    wextr_x[0]=HWRMax(D->box.left, x_left);
    
    if (jl==-1)
        wextr_y[0]=extr[0].y;
    if (jl==n_extr-1)
        wextr_y[0]=extr[n_extr-1].y;
    if (jl!=-1 && jl!=n_extr-1)
    {
        dy=(_LONG)(extr[jl+1].y-extr[jl].y);
        dx=HWRMax(1,(extr[jl+1].x-extr[jl].x));
        wextr_y[0] =
        extr[jl].y + (_SHORT) (dy * (x_left-extr[jl].x) / dx);
    }
    
    for (k=1; k<max_ind; k++)
    {
        wextr_x[k]=extr[jl+k].x;
        wextr_y[k]=extr[jl+k].y;
    }
    
    wextr_x[max_ind]=HWRMin(D->box.right,x_right);
    
    if (jr==0)
        wextr_y[max_ind]=extr[0].y;
    if (jr==n_extr)
        wextr_y[max_ind]=extr[n_extr-1].y;
    if (jr!=0 && jr!=n_extr)
    {
        dy=(_LONG)(extr[jr].y-extr[jr-1].y);
        dx=HWRMax(1,(extr[jr].x-extr[jr-1].x));
        wextr_y[max_ind] =
        extr[jr-1].y + (_SHORT) (dy * (x_right-extr[jr-1].x) / dx);
    }
    
    
    for (i=0; i<max_ind; i++)
    {
        k=i;
        while (k+1<=max_ind && wextr_x[k+1]==wextr_x[i])
            k++;
        if (k>i)
        {
            for (j=0, mid_y=0; j<=k-i; j++)
                mid_y+=wextr_y[i+j];
            mid_y/=k-i+1;
            wextr_y[i]=wextr_y[k]=mid_y;
            i=k;
        }
    }
    
    integ=0;
    
    if (wextr_x[max_ind]-wextr_x[0] == 0)
    {
        integ=wextr_y[0];
    }
    else
    {
        for (k=1; k<=max_ind; k++)
            integ += (_LONG)(wextr_y[k]+wextr_y[k-1])*
            (_LONG)(wextr_x[k]-wextr_x[k-1]);
        integ = ONE_HALF(integ);
        
    }
    return( (_SHORT)( integ/HWRMax(1,(wextr_x[max_ind]-wextr_x[0])) ) );
    
}

/***************************************************************************/

_VOID smooth_d_bord(p_EXTR line_max,_INT n_line_max,low_type _PTR D,
                    _INT should,p_SHORT bord_d)
{
    _INT i;
    //   _INT prev_mid =(D->yu_beg+D->yu_end+D->yd_beg+D->yd_end)/4;
    if (n_line_max>0)
    {
        for (i=0; i<D->ii; i++)
        {
            if (D->y[i] == BREAK)
            {
                bord_d[i]=0;
                continue;
            }
            bord_d[i]=point_of_smooth_bord(i,n_line_max,line_max,D,should);
        }
    }
    else
    {
        for (i=0; i<D->ii; i++)
        {
            if (D->y[i] == BREAK)
            {
                bord_d[i]=0;
                continue;
            }
            /*
             if (prev_mid!=0 && D->box.top<prev_mid && D->box.bottom>prev_mid)
             bord_d[i]=D->yd_beg;
             else
             */
#ifdef USE_WORDSPLIT_PARMS
            if (/*D->rc->stroka.size_sure_in >=50 && */D->rc->stroka.pos_sure_in >=50)
                bord_d[i]=D->rc->stroka.dn_pos_in;
            else
#endif /* USE_WORDSPLIT_PARMS */
                bord_d[i]=D->box.bottom;
        }
    }
    return;
}

/***************************************************************************/

_VOID smooth_u_bord(p_EXTR line_min,_INT n_line_min,low_type _PTR D,
                    _INT should,p_SHORT bord_u,p_SHORT bord_d)
{
    _INT i, min_bord_d;
    //   _INT prev_mid =(D->yu_beg+D->yu_end+D->yd_beg+D->yd_end)/4;
    if (n_line_min>1)
    {
        for (i=0; i<D->ii; i++)
        {
            if (D->y[i] == BREAK)
            {
                bord_u[i]=0;
                continue;
            }
            bord_u[i] = point_of_smooth_bord(i,n_line_min,line_min,D,should);
        }
    }
    if (n_line_min==1)
    {
        for (i=0; i<D->ii; i++)
        {
            if (D->y[i] == BREAK)
                bord_u[i]=0;
            else
                bord_u[i]=(_SHORT)(bord_d[i]-(bord_d[line_min[0].i]-line_min[0].y));
        }
    }
    if (n_line_min==0)
    {
        if (D->rc->lmod_border_used==LMOD_BORDER_TEXT)
        {
            for (i=0,min_bord_d=ALEF; i<D->ii; i++)
                if (D->y[i] != BREAK && bord_d[i] < min_bord_d)
                    min_bord_d = bord_d[i];
            
            for (i=0; i<D->ii; i++)
            {
                if (D->y[i] == BREAK)
                {
                    bord_u[i]=0;
                    continue;
                }
                /*
                 if (prev_mid!=0 && D->box.top<prev_mid && D->box.bottom>prev_mid)
                 bord_u[i]=D->yu_beg;
                 else
                 */
#ifdef USE_WORDSPLIT_PARMS
                if (D->rc->stroka.size_sure_in >=50 && D->rc->stroka.pos_sure_in>=50)
                    bord_u[i]=
                    (_SHORT)(D->rc->stroka.dn_pos_in - D->rc->stroka.size_in);
                else
#endif /* USE_WORDSPLIT_PARMS */
                    bord_u[i]=(_SHORT)(TWO_THIRD(D->box.top)+ONE_THIRD(min_bord_d));
            }
        }
        else
        {
            for (i=0; i<D->ii; i++)
            {
                if (D->y[i] == BREAK)
                    bord_u[i]=0;
                else
                    bord_u[i]=D->box.top;
            }
        }
    }
    return;
}

/***************************************************************************/

_INT sign(_INT a, _INT b)
{
    _INT ret=0;
    
    if (a>b)
        ret=1;
    if (a<b)
        ret=-1;
    return(ret);
}

/***************************************************************************/

_INT fill_i_point(p_SHORT i_point, low_type _PTR D)
{
    /*   The "i_point"                                        */
    /*  array must always be filled with legal values for     */
    /*  Pashintsev's function "FillRCNB" at the very end of   */
    /*  transform (see comments at the call to it).           */
    _INT i,j,k,n_x=0,last_x=-1;
    /* i_point[i] is number of one of those */
    /* points of trajectory, which have i-th */
    for (i=0; i<D->ii; i++)          /* from left value of x-coordinate;   */
    {
        if (D->y[i]==BREAK)
            continue;
        if (D->x[i]>last_x)
        {
            i_point[n_x++]=(_SHORT)i;
            last_x=D->x[i];
            continue;
        }
        for (j=n_x-1; j>=0; j--)
        {
            if (D->x[i]==D->x[i_point[j]])
                break;
            if (D->x[i]>D->x[i_point[j]])
            {
                for (k=n_x++; k>j+1; k--)
                    i_point[k]=i_point[k-1];
                i_point[j+1]=(_SHORT)i;
                break;
            }
        }
        if (D->x[i]<D->x[i_point[0]])
        {
            for (k=n_x++; k>0; k--)
                i_point[k]=i_point[k-1];
            i_point[0] = (_SHORT)i;
        }
    }
    return(n_x);
}
/* ************************************************************************* */
_BOOL num_bord_correction(p_EXTR extr,p_INT pn_extr,_INT n_allextr,
                          _UCHAR type,_INT med_ampl,p_SHORT bord_d,p_SHORT y, _INT lang )
{
    register _INT	i, j;
    _UCHAR	OUTSIDE_LINE=ON_LINE, opp_type=EMPTY;
    _BOOL	del=_FALSE;
    p_SPECL cur;
    
    if (type==MAXW)
    {
        OUTSIDE_LINE=SUB_SCRIPT;
        opp_type=MINW;
    }
    if (type==MINW)
    {
        OUTSIDE_LINE=SUPER_SCRIPT;
        opp_type=MAXW;
    }
    for (i=0; i<*pn_extr; i++)
        if (extr[i].pspecl->attr==SLASH)
            extr[i].susp=OUTSIDE_LINE;
        else
            extr[i].susp=0;
    delete_line_extr(extr,pn_extr,OUTSIDE_LINE);
    
    find_gaps_in_line(extr,*pn_extr,n_allextr,med_ampl,type,0,ALEF,bord_d,y,
                      _TRUE,_FALSE, lang );
    find_glitches_in_line(extr,*pn_extr,med_ampl,type,0,ALEF,bord_d,0,0,1,
                          _TRUE,_FALSE, lang );
    
    for (i=0; i<*pn_extr; i++)
    {
        if (((type==MAXW && extr[i].susp==GLITCH_DOWN) ||
             (type==MINW && extr[i].susp==GLITCH_UP))
             && extr[i].pspecl->attr==STRT )
        {
            extr[i].susp=OUTSIDE_LINE;
            cur=extr[i].pspecl;
            if (cur->prev->mark==opp_type)
                cur->prev->attr=SLASH;
            if (cur->next->mark==opp_type)
                cur->next->attr=SLASH;
        }
        if (((type==MAXW && extr[i].susp==DBL_GLITCH_DOWN) ||
             (type==MINW && extr[i].susp==DBL_GLITCH_UP)) && i<*pn_extr-1 )
        {
            if (extr[i].pspecl->attr==STRT && extr[i+1].pspecl->attr==STRT)
            {
                for (j=i; j<=i+1;j++)
                {
                    extr[j].susp=OUTSIDE_LINE;
                    cur=extr[j].pspecl;
                    if (cur->prev->mark==opp_type)
                        cur->prev->attr=SLASH;
                    if (cur->next->mark==opp_type)
                        cur->next->attr=SLASH;
                }
            }
            i++;
        }
    }
    del=delete_line_extr(extr,pn_extr,OUTSIDE_LINE);
    
    for (i=0; i<*pn_extr; i++)
        extr[i].susp=0;
    find_gaps_in_line(extr,*pn_extr,n_allextr,med_ampl,type,0,ALEF,bord_d,y,
                      _FALSE,_FALSE, lang );
    find_glitches_in_line(extr,*pn_extr,med_ampl,type,0,ALEF,bord_d,0,0,1,
                          _FALSE,_FALSE, lang );
    for (i=0; i<*pn_extr; i++)
    {
        if ((type==MAXW && (extr[i].susp==GLITCH_UP)) ||
            (type==MINW && (extr[i].susp==GLITCH_DOWN)) )
        {
            extr[i].susp=INSIDE_LINE;
        }
    }
    delete_line_extr(extr,pn_extr,INSIDE_LINE);
    return(del);
}


/* ************************************************************************* */

_BOOL bord_correction(low_type _PTR D,p_EXTR extr,p_INT pn_extr,_INT n_allextr,
                      _UCHAR type,_INT step,_INT med_ampl,_INT mid_ampl,
                      _INT max_ampl,_INT x_left,_INT x_right,_INT LINE_POS,_UCHAR pass,
                      p_SHORT bord_d,_INT max_height,_INT ft_height,
                      _BOOL gl_up_left,_BOOL gl_down_left  )
{
    _BOOL   del=_FALSE;
    p_SHORT x = D->buffers[0].ptr, y = D->buffers[1].ptr;
    _INT    i, n_out,n_inside,max_line_height,mid_line_height,base_ampl;
    _INT    n_extr=*pn_extr,SHIFT=0;
    _INT    thresh=D->rc->stroka.extr_depth;
    _INT    base_ampl_sure=D->rc->stroka.size_sure_in;
    _UCHAR  MARK=0;
    
    if (type==MAXW)
    {
        MARK=SUB_SCRIPT;
        SHIFT=1;
    }
    if (type==MINW)
    {
        MARK=SUPER_SCRIPT;
        SHIFT=2;
    }
    
    for (i=0; i<n_extr; i++)
        extr[i].susp=0;
    if (n_extr<2)
        return(_FALSE);
    
    find_gaps_in_line(extr,n_extr,n_allextr,med_ampl,type,x_left,x_right,
                      bord_d,y,_FALSE,_FALSE, D->rc->lang );
    find_glitches_in_line(extr,n_extr,med_ampl,type,x_left,x_right,
                          bord_d,x,y,SHIFT,_FALSE,_FALSE, D->rc->lang );
    if (type==MAXW)
        glitch_to_sub_max(D,extr,n_extr,mid_ampl,gl_down_left);
    if (type==MINW)
    {
        glitch_to_super_min(extr,n_extr,bord_d,max_height,x,y,gl_up_left, D->rc->lang );
        for (i=0,n_out=0,max_line_height=0,mid_line_height=0; i<n_extr; i++)
        {
            if (extr[i].susp==MARK)
                n_out++;
            else
            {
                max_line_height=
                HWRMax(max_line_height,bord_d[extr[i].i]-extr[i].y);
                mid_line_height+=bord_d[extr[i].i]-extr[i].y;
            }
        }
        if (n_out<n_extr)
            mid_line_height/=(n_extr-n_out);
        if (n_out>0)
        {
            super_min_to_line(extr,n_extr,bord_d,max_line_height,mid_line_height,
                              &n_out);
        }
    }
    glitch_to_inside(extr,n_extr,type,y,mid_ampl,x_left,x_right, D->rc->lang );
    if (pass==1 && LINE_POS!=0)
        spec_neibour_extr(extr,n_extr,type,LINE_POS);
    else
        if (neibour_susp_extr(extr,n_extr,type,bord_d,ft_height-thresh)==UNSUCCESS)
        {
            //         if (n_allextr<=4)
            base_ampl=step;
            //         else           base_ampl=med_ampl;
            all_susp_extr(extr,n_extr,n_allextr,type,y,base_ampl,base_ampl_sure,
                          max_ampl,bord_d,ft_height-thresh, D->rc->lang );
        }
    for (i=0,n_out=0; i<n_extr; i++)
    {
        if (extr[i].susp==MARK)
            n_out++;
    }
    if (n_out>0 /*&& (type==MINW || n_out<=ONE_HALF(n_extr))*/)
    {
        for (i=0; i<n_extr; i++)
        {
            if (extr[i].susp==MARK)
                extr[i].pspecl->code=MARK;
            else
                extr[i].susp=0;
        }
        del=delete_line_extr(extr, pn_extr, MARK);
        n_extr=*pn_extr;
        
        find_gaps_in_line(extr,n_extr,n_allextr,med_ampl,type,x_left,x_right,
                          bord_d,y,_FALSE,_FALSE, D->rc->lang );
        find_glitches_in_line(extr,n_extr,med_ampl,type,x_left,x_right,
                              bord_d,x,y,SHIFT,_FALSE,_FALSE, D->rc->lang );
        if (type==MAXW)
            glitch_to_sub_max(D,extr,n_extr,mid_ampl,gl_down_left);
        if (type==MINW)
        {
            glitch_to_super_min(extr,n_extr,bord_d,max_height,x,y,gl_up_left, D->rc->lang);
            for (i=0,n_out=0,max_line_height=0,mid_line_height=0; i<n_extr; i++)
            {
                if (extr[i].susp==MARK)
                    n_out++;
                else
                {
                    max_line_height=
                    HWRMax(max_line_height,bord_d[extr[i].i]-extr[i].y);
                    mid_line_height+=bord_d[extr[i].i]-extr[i].y;
                }
            }
            if (n_out<n_extr)
                mid_line_height/=(n_extr-n_out);
            if (n_out>0)
            {
                super_min_to_line(extr,n_extr,bord_d,max_line_height, mid_line_height,&n_out);
            }
        }
        glitch_to_inside(extr,n_extr,type,y,mid_ampl,x_left,x_right, D->rc->lang );
        if (pass==1 && LINE_POS!=0)
            spec_neibour_extr(extr,n_extr,type,LINE_POS);
        else
            if (neibour_susp_extr(extr,n_extr,type,bord_d,ft_height-thresh)==UNSUCCESS)
            {
                //            if (n_allextr<=4)
                base_ampl=step;
                //            else           base_ampl=med_ampl;
                all_susp_extr(extr,n_extr,n_allextr,type,y,base_ampl,base_ampl_sure,
                              max_ampl,bord_d,ft_height-thresh, D->rc->lang );
            }
    }
    for (i=0,n_inside=0; i<n_extr; i++)
    {
        if (extr[i].susp==INSIDE_LINE)
            n_inside++;
    }
    
    if ( n_inside > 0 /*&& n_inside<=ONE_HALF(n_extr)*/ )
    {
        for ( i=0; i < n_extr; i++ )
        {
            if (extr[i].susp==INSIDE_LINE)
                extr[i].pspecl->code=INSIDE_LINE;
            else
                extr[i].susp=0;
        }
        del=delete_line_extr(extr, pn_extr, INSIDE_LINE);
        n_extr = *pn_extr;
    }
    
    return(del);
}

/***************************************************************************/

_VOID del_tail_min(p_EXTR line_min,p_INT pn_line_min,p_SHORT y,
                   p_SHORT bord_d,_UCHAR pass)
{
    p_SPECL cur,nxt,prv;
    _INT    i,n_line_min=*pn_line_min;
    _LONG   Q = 2,
    
    P = (pass==0) ? 5 : 10;
    for (i=0; i<n_line_min; i++)
    {
        cur=line_min[i].pspecl;
        nxt=cur->next;
        prv=cur->prev;
        
        if ((prv->mark==BEG || (prv->mark==MAXW && P*(y[prv->ipoint0]-line_min[i].y)<
                                Q*(bord_d[line_min[i].i]-line_min[i].y)))
            &&
            (nxt->mark==END || (nxt->mark==MAXW && P*(y[nxt->ipoint0]-line_min[i].y)<
                                Q*(bord_d[line_min[i].i]-line_min[i].y))))
        {
            line_min[i].susp=TAIL_MIN;
            cur->code=TAIL_MIN;
        }
    }
    delete_line_extr(line_min,pn_line_min,TAIL_MIN);
    return;
}

/***************************************************************************/

_INT line_pos_mist(low_type _PTR D,_INT med_u_bord,_INT med_d_bord,
                   _INT med_height,_INT n_line_min,_INT n_line_max,
                   p_EXTR line_max,p_INT pUP_LINE_POS,p_INT pDOWN_LINE_POS,
                   p_SHORT bord_u,p_SHORT bord_d,_UCHAR pass )
{
    _INT    lang = D->rc->lang;
    _UCHAR	prv_min_inside=0, prv_max_inside=0;
    _INT	all_min_inside=0,all_max_inside=0,n_sub=0,n_inmin=0,n_inmax=0,n_super=0;
    _INT    min_y=0,max_y=0,n_str=0,n_allmin=0,n_allmax=0;
    p_SHORT x=D->buffers[0].ptr, y=D->buffers[1].ptr, i_back=D->buffers[2].ptr;
    _INT	thresh=D->rc->stroka.extr_depth, dn_pos=D->rc->stroka.dn_pos_in;
    _INT    dn_pos_sure=D->rc->stroka.pos_sure_in;
    _INT     size=D->rc->stroka.size_in, size_sure=D->rc->stroka.size_sure_in;
    _INT	ibeg_str=0,iend_str=0, i_stroke=0, cur_y,cur_i, min_to_ret, max_to_ret;
    _INT	shtraf=0;
    _INT	i, max_height;
    _RECT	stroke_box;
    _LONG	P_MIN=4, Q_MIN=5;
    _LONG   P_MAX=3, Q_MAX=4,  CONST_NARROW=8, CONST_INSIDE=2;
    
    p_SPECL	cur=D->specl, wrk;
    _BOOL	apostroph = _FALSE;
    
    if (lang == LANGUAGE_GERMAN)
    {
        P_MAX=4;
        Q_MAX=5;
        CONST_NARROW=10;
        CONST_INSIDE=3;
    }
    for (i=0, max_height=0; i<D->ii; i++)
    {
        if (bord_d[i]-D->y[i] > max_height)
            max_height = bord_d[i]-D->y[i];
        if (D->y[i]-bord_u[i] > max_height)
            max_height = D->y[i]-bord_u[i];
    }
    
    if (CONST_NARROW*med_height<=max_height)
        shtraf+=5;
    
    while (cur!=_NULL)
    {
        if (cur->mark==BEG)
        {
            i_stroke++;
            ibeg_str=cur->ibeg;
            all_min_inside=all_max_inside=0;
            min_y=ALEF;
            max_y=0;
            n_str++;
            prv_min_inside=0;
            prv_max_inside=0;
        }
        if (cur->mark==MINW)
        {
            n_allmin++;
            cur_y=y[cur->ipoint0];
            cur_i=i_back[cur->ipoint0];
            if (cur->attr==NORM || cur->attr==I_MIN)
                min_y=HWRMin(min_y,cur_y);
            if (cur->code==RET_ON_LINE) cur->code=ON_LINE;
            if (cur->code==NOT_ON_LINE && cur_y > bord_u[cur_i])
                cur->code=INSIDE_LINE;
            if (cur->code==NOT_ON_LINE && cur_y < bord_u[cur_i])
                cur->code=SUPER_SCRIPT;
            if (cur->code==SUPER_SCRIPT || cur->code==INSIDE_LINE ||
                cur->attr==T_MIN)
            {
                if (cur_y < bord_u[cur_i])
                    n_super++;
                if (cur_y > bord_u[cur_i])
                    n_inmin++;
            }
            if (cur->code==INSIDE_LINE)
            {
                if (cur_y<bord_d[cur_i])
                {
                    if (prv_min_inside>0 &&
                        extrs_open(D,cur,MINW,2)==_TRUE)
                    {
                        cur->code=RET_ON_LINE;
                        wrk=cur->prev;
                        while (wrk->mark!=BEG &&(wrk->mark!=MINW || wrk->code==0))
                            wrk=wrk->prev;
                        wrk->code=RET_ON_LINE;
                        shtraf+=1;
                        cur=cur->next;
                        continue;
                    }
                    prv_min_inside++;
                }
                if (all_min_inside>=0)
                {
                    all_min_inside++;
                }
            }
            if (cur->code==ON_LINE || cur->code==SUPER_SCRIPT ||
                cur->code==RET_ON_LINE || (cur->code==0 && cur_y<=med_u_bord) )
            {
                prv_min_inside=0;
                all_min_inside=-1;
            }
            
            if (D->rc->low_mode & LMOD_SMALL_CAPS && cur->code==SUPER_SCRIPT && i_stroke >2 )
            {
                wrk=cur;
                while (wrk->mark!=END)
                    wrk=wrk->next;
                iend_str=wrk->iend;
                GetTraceBox(x,y,(_SHORT)ibeg_str,(_SHORT)iend_str,&stroke_box);
                if (stroke_box.right<D->box.right-thresh)
                {
                    shtraf+=1;
                    cur->code=RET_ON_LINE;
                }
            }
            
            if (cur->code==ON_LINE || cur->code==RET_ON_LINE)
            {
                if (cur_y > bord_d[cur_i])
                {
                    cur->code=NOT_ON_LINE;
                    shtraf+=4;
                    cur=cur->next;
                    continue;
                }
                /*
                 if ( CONST_NARROW*(bord_d[cur_i]-cur_y) < max_height )
                 {
                 cur->code=NOT_ON_LINE;
                 shtraf+=1;
                 cur=cur->next;
                 continue;
                 }
                 */
                if ( CONST_NARROW*(bord_d[cur_i]-cur_y) < max_height ||
                    (CONST_INSIDE*(bord_d[cur_i]-cur_y) < med_height && cur->attr!=I_MIN) )
                {
                    _INT dyl = 0;
                    _INT dyr = 0;
                    
                    wrk=cur->prev;
                    while(wrk!=NULL)
                    {
                        if (wrk->mark==MINW && wrk->code==ON_LINE)
                        {
                            dyl=cur_y-y[wrk->ipoint0];
                            break;
                        }
                        wrk=wrk->prev;
                    }
                    
                    wrk=cur->next;
                    while(wrk!=NULL)
                    {
                        if (wrk->mark==MINW && wrk->code==ON_LINE)
                        {
                            dyr=cur_y-y[wrk->ipoint0];
                            break;
                        }
                        wrk=wrk->next;
                    }
                    
                    if (100L*dyl>=10L*med_height || 100L*dyr>=10L*med_height)
                    {
                        cur->code=NOT_ON_LINE;
                        shtraf+=1;
                        cur=cur->next;
                        continue;
                    }
                }
            }
        }
        
        if (cur->mark==MAXW)
        {
            n_allmax++;
            cur_y=y[cur->ipoint0];
            cur_i=i_back[cur->ipoint0];
            if (cur->attr==NORM)
                max_y=HWRMax(max_y,cur_y);
            if (cur->code==RET_ON_LINE)
                cur->code=ON_LINE;
            if (cur->code==NOT_ON_LINE && cur_y < bord_d[cur_i])
                cur->code=INSIDE_LINE;
            if (cur->code==NOT_ON_LINE && cur_y > bord_d[cur_i])
                cur->code=SUB_SCRIPT;
            if (cur->code==SUB_SCRIPT || cur->code==INSIDE_LINE)
            {
                if (cur_y > bord_d[cur_i])
                    n_sub++;
                if (cur_y < bord_d[cur_i])
                    n_inmax++;
            }
            if (cur->code==INSIDE_LINE)
            {
                if (cur_y>bord_u[cur_i])
                {
                    if (prv_max_inside>1 &&
                        extrs_open(D,cur,MAXW,2)==_TRUE)
                    {
                        cur->code=RET_ON_LINE;
                        wrk=cur->prev;
                        while (wrk->mark!=BEG &&
                               (wrk->mark!=MAXW || wrk->code==0)
                               )   wrk=wrk->prev;
                        wrk->code=RET_ON_LINE;
                        shtraf++;
                        if (prv_max_inside==2)  shtraf++;
                        cur=cur->next;
                        continue;
                    }
                    prv_max_inside++;
                }
                
                if (all_max_inside>=0)
                    all_max_inside++;
            }
            if (cur->code==ON_LINE || cur->code==SUB_SCRIPT ||
                cur->code==RET_ON_LINE || (cur->code==0 && cur_y>=med_d_bord) )
            {
                prv_max_inside=0;
                all_max_inside=-1;
            }
            
            if (cur->code==ON_LINE || cur->code==RET_ON_LINE)
            {
                if (cur_y < bord_u[cur_i])
                {
                    cur->code=NOT_ON_LINE;
                    shtraf+=4;
                    cur=cur->next;
                    continue;
                }
                /*
                 if ( CONST_NARROW*(cur_y-bord_u[cur_i]) < max_height )
                 {
                 cur->code=NOT_ON_LINE;
                 shtraf+=1;
                 cur=cur->next;
                 continue;
                 }
                 */
                if ( CONST_NARROW*(cur_y-bord_u[cur_i]) < max_height ||
                    CONST_INSIDE*(cur_y-bord_u[cur_i]) < med_height )
                {
                    _INT dyl = 0;
                    _INT dyr = 0;
                    
                    wrk=cur->prev;
                    while(wrk!=NULL)
                    {
                        if (wrk->mark==MAXW && wrk->code==ON_LINE)
                        {
                            dyl=y[wrk->ipoint0]-cur_y;
                            break;
                        }
                        wrk=wrk->prev;
                    }
                    
                    wrk=cur->next;
                    while(wrk!=NULL)
                    {
                        if (wrk->mark==MAXW && wrk->code==ON_LINE)
                        {
                            dyr=y[wrk->ipoint0]-cur_y;
                            break;
                        }
                        wrk=wrk->next;
                    }
                    
                    if (100L*dyl>=10L*med_height || 100L*dyr>=10L*med_height)
                    {
                        cur->code=NOT_ON_LINE;
                        shtraf+=1;
                        cur=cur->next;
                        continue;
                    }
                }
            }
        }
        
        if (cur->mark==END && cur->prev->mark!=BEG && (all_min_inside>0 || all_max_inside>0) )
            
        {
            //         shtraf+=1;
            iend_str=cur->iend;
            GetTraceBox(x,y,(_SHORT)ibeg_str,(_SHORT)iend_str,&stroke_box);
            if (stroke_box.right>D->box.right-thresh &&
                stroke_box.bottom-stroke_box.top<ONE_HALF(med_height) &&
                stroke_box.right-stroke_box.left<
                THREE_HALF(stroke_box.bottom-stroke_box.top) )
            {
                cur=cur->next;
                continue;
            }
            
            if (straight_stroke(ibeg_str,iend_str,x,y,5)==_TRUE
                || curve_com_or_brkt(D,cur,ibeg_str,iend_str,5, LMOD_BORDER_TEXT)!=0 )
            {
                if (all_max_inside>0 && stroke_box.right-stroke_box.left<=
                    stroke_box.bottom-stroke_box.top && cur->prev->mark==MAXW )
                {
                    cur_y=y[cur->prev->ipoint0];
                    cur_i=i_back[cur->prev->ipoint0];
                    if (bord_d[cur_i]-cur_y>ONE_FOURTH(med_height))
                        apostroph=_TRUE;
                }
                cur=cur->next;
                continue;
            }
            
            wrk=cur->prev;
            min_to_ret=max_to_ret=0;
            while (wrk->mark!=BEG)
            {
                if (all_min_inside>0 && wrk->mark==MINW &&
                    y[wrk->ipoint0]==min_y &&
                    (all_min_inside>2 ||
                     (pass>0 && max_y-min_y>ONE_FOURTH(med_d_bord-med_u_bord)) ||
                     min_y<P_MIN*med_d_bord/Q_MIN + med_u_bord/Q_MIN ) )
                {
                    wrk->code=RET_ON_LINE;
                    min_to_ret+=1;
                }
                if (all_max_inside>0 && wrk->mark==MAXW && y[wrk->ipoint0]==max_y &&
                    (all_max_inside>2 ||
                     (pass>0 && max_y-min_y>ONE_FOURTH(med_d_bord-med_u_bord)) ||
                     (max_y>P_MAX*med_u_bord/Q_MAX + med_d_bord/Q_MAX &&
                      min_y>med_u_bord) ) )
                {
                    wrk->code=RET_ON_LINE;
                    max_to_ret+=1;
                }
                wrk=wrk->prev;
            }
            
            shtraf+=min_to_ret+max_to_ret;//+1;
        }
        
        cur=cur->next;
    }
    
    if (n_super>5)
    {
        if ((lang == LANGUAGE_GERMAN &&n_line_min<5) || n_super>n_line_min+1)
        {
            *pUP_LINE_POS=STR_TOO_NARROW;
            shtraf+=n_super-n_line_min-1;
        }
    }
    if (D->box.right-D->box.left > D->box.bottom-D->box.top || n_str>1)
    {
        if (n_inmax>n_line_max && n_sub<=n_line_max+1)
        {
            *pDOWN_LINE_POS=TOO_WIDE;
            shtraf+=n_inmax-n_line_max;
        }
        if (n_inmin>n_line_min && n_super<=n_line_min+1)
        {
            *pUP_LINE_POS=TOO_WIDE;
            shtraf+=n_inmin-n_line_min;
        }
        if (n_sub>n_line_max+1 && n_inmax<=n_line_max)
        {
            *pDOWN_LINE_POS=STR_TOO_NARROW;
            shtraf+=n_sub-n_line_max-1;
        }
        if (n_allmax>=5 && n_allmin>=5 && n_inmax>=2 && D->box.right-line_max[n_line_max-1].x > ONE_HALF(DX_RECT(D->box)) )
        {
            shtraf+=2;
        }
    }
    
    if ( med_d_bord<dn_pos-size &&
        ((dn_pos_sure>=90 && size_sure>=90 && n_line_max<=3) ||
         (dn_pos_sure>=75 && size_sure>=75 && n_line_max<=2) ||
         (dn_pos_sure>=55 && size_sure>=55 && n_line_max<=1) ) )
    {
        *pDOWN_LINE_POS=STR_TOO_NARROW;
        shtraf+=3;
    }
    
    if ( med_height<TWO_FIFTH(size) && *pDOWN_LINE_POS!=STR_TOO_NARROW &&
        ((dn_pos_sure>=90 && size_sure>=90 && n_line_min<=3) ||
         (dn_pos_sure>=75 && size_sure>=75 && n_line_min<=2) ||
         (dn_pos_sure>=55 && size_sure>=55 && n_line_min<=1) ) )
    {
        *pUP_LINE_POS=STR_TOO_NARROW;
        shtraf+=3;
    }
    
    if ( med_d_bord>dn_pos+size &&
        ((dn_pos_sure>=90 && size_sure>=90 && n_line_max<=3) ||
         (dn_pos_sure>=75 && size_sure>=75 && n_line_max<=2) ||
         (dn_pos_sure>=55 && size_sure>=55 && n_line_max<=1) ) )
    {
        *pDOWN_LINE_POS=TOO_WIDE;
        shtraf+=3;
    }
    
    if ( med_height>size+THREE_HALF(size) &&
        ((dn_pos_sure>=90 && size_sure>=90 && n_line_max<=3) ||
         (dn_pos_sure>=75 && size_sure>=75 && n_line_max<=2) ||
         (dn_pos_sure>=55 && size_sure>=55 && n_line_max<=1) ) )
    {
        *pUP_LINE_POS=TOO_WIDE;
        shtraf+=3;
    }
    
    if ( med_u_bord > dn_pos &&
        ((dn_pos_sure>=90 && (n_line_max<=3 || n_line_min<=3)) ||
         (dn_pos_sure>=75 && (n_line_max<=2 || n_line_min<=2)) ||
         (dn_pos_sure>=55 && (n_line_max<=1 || n_line_min<=1)) ) )
    {
        *pUP_LINE_POS=STR_TOO_NARROW;
        shtraf+=5;
    }
    
    if (apostroph==_TRUE && n_line_min<6 && n_line_max<6 && n_super==0 && n_sub==0)
        shtraf+=100;
    
    //this if for the A_amstrem charcter
    if (lang == LANGUAGE_SWEDISH || lang == LANGUAGE_FINNISH || lang == LANGUAGE_NORWEGIAN || lang == LANGUAGE_DANISH)
    {
        if (n_line_min == 1 && n_line_max == 1 && n_sub > 0 && n_super == 0 &&
            CONST_NARROW * med_height <= max_height * 3)
            shtraf+=100;
    }
    
    return(shtraf);
}
/***************************************************************************/

_BOOL extrs_open(low_type _PTR D,p_SPECL cur,_UCHAR type,_INT num)
{
    p_SPECL wrk, prv;
    _INT    i, ibeg, iend, thresh = D->rc->stroka.extr_depth;
    _INT    cur_x, cur_y, prv_x, prv_y, sign;
    p_SHORT x = D->buffers[0].ptr, y = D->buffers[1].ptr;
    
    wrk=cur->next;
    while (wrk->mark!=END && wrk->mark!=type)
        wrk=wrk->next;
    if (wrk->mark!=END)
        wrk=wrk->next;
    iend=wrk->iend;
    wrk=cur->prev;
    prv=cur;
    if (num>1)
    {
        while (wrk->mark!=BEG && (wrk->mark!=type || wrk->code==0))
            wrk=wrk->prev;
        prv=wrk;
        wrk=prv->prev;
    }
    while (wrk->mark!=BEG && wrk->mark!=type)
        wrk=wrk->prev;
    if (wrk->mark!=BEG)
        wrk=wrk->prev;
    ibeg=wrk->ibeg;
    
    cur_y=y[cur->ipoint0];
    cur_x=x[cur->ipoint0];
    prv_y=y[prv->ipoint0];
    prv_x=x[prv->ipoint0];
    sign = (type==MINW) ? 1 : -1;
    
    for (i=ibeg; i<iend; i++)
    {
        if (((i<cur->ibeg || i>cur->iend) &&
             sign*(cur_y-y[i])>0 && HWRAbs(cur_x-x[i])<ONE_HALF(thresh))
            ||
            ((i<prv->ibeg || i>prv->iend) &&
             sign*(prv_y-y[i])>0 && HWRAbs(prv_x-x[i])<ONE_HALF(thresh)) )
        {
            return(_FALSE);
        }
    }
    
    return(_TRUE);
}
/***************************************************************************/
_INT calc_med_heights(low_type _PTR D,p_EXTR line_min,p_EXTR line_max,
                      p_SHORT bord_u,p_SHORT bord_d,p_SHORT i_point,
                      _INT n_line_min,_INT n_line_max,_INT n_x,
                      p_INT pmed_height,p_INT pmed_u_bord,p_INT pmed_d_bord)
{
    _INT    i, j, xl, xr, il, ir, n_height;
    _INT    ipnt, x_lim;
    //   _INT old_med_height;
    p_SHORT height = _NULL;
    p_SHORT x = D->x;
    
    if (n_line_min<2 || n_line_max<2)
    {
        il=0;
        ir=n_x-1;
        n_height=n_x;
    }
    else
    {
        //      xl=HWRMax(line_min[0].x,line_max[0].x);
        //      xr=HWRMin(line_min[n_line_min-1].x,line_max[n_line_max-1].x);
        xl = HWRMin(line_min[0].x,line_max[0].x);
        xr = HWRMax(line_min[n_line_min-1].x,line_max[n_line_max-1].x);
        i=0;
        while (x[i_point[i]]<xl)
            i++;
        il=i;
        while (x[i_point[i]]<xr)
            i++;
        ir=i;
        n_height=ir-il+1;
    }
    
    xl = x[i_point[il]];
    xr = x[i_point[ir]];
    
    height=(p_SHORT)HWRMemoryAlloc(sizeof(_SHORT)*HWRMax(n_height,50));
    if (height==_NULL)
        return(UNSUCCESS);
    /*
     for (j=0; j<n_height; j++)
     height[j] =(_SHORT)(bord_d[i_point[il+j]]-bord_u[i_point[il+j]]);
     old_med_height=calc_mediana(height,n_height);
     */
    
    if (n_height<50)
    {
        for (j=0; j<n_height; j++)
            height[j] =(_SHORT)(bord_d[i_point[il+j]]-bord_u[i_point[il+j]]);
    }
    else
    {
        for (j=0,ipnt=il; j<50; j++)
        {
            x_lim=xl+(_INT)((_LONG)(xr-xl)*(_LONG)j/50L);
            for (i=ipnt; i<=ir; i++)
            {
                if (x[i_point[i]]>=x_lim)
                    break;
            }
            if (i>il && x[i_point[i]]-x_lim > x_lim - x[i_point[i-1]])
                ipnt=i-1;
            else
                ipnt=i;
            height[j] = (_SHORT)(bord_d[i_point[ipnt]]-bord_u[i_point[ipnt]]);
        }
    }
    *pmed_height=calc_mediana(height,HWRMin(50,n_height));     //HWRMin(50,n_height)
    
    //   if (old_med_height < (_INT)((_LONG)(*pmed_height)*9L/10L))
    //      SetTesterMark;
    
    if (n_height<50)
    {
        for (i=il,j=0; i<=ir; i++)
        {
            height[j++] = bord_d[i_point[i]];
        }
    }
    else
    {
        for (j=0,ipnt=il; j<50; j++)
        {
            x_lim=xl+(_INT)((_LONG)(xr-xl)*(_LONG)j/50L);
            for (i=ipnt; i<=ir; i++)
            {
                if (x[i_point[i]]>=x_lim)
                    break;
            }
            if (i>il && x[i_point[i]]-x_lim > x_lim - x[i_point[i-1]])
                ipnt=i-1;
            else
                ipnt=i;
            height[j] =  bord_d[i_point[ipnt]];
        }
    }
    
    *pmed_d_bord=calc_mediana(height,HWRMin(50,n_height));         //HWRMin(50,n_height)
    
    if (n_height<50)
    {
        for (i=il,j=0; i<=ir; i++)
            height[j++] = bord_u[i_point[i]];
    }
    else
    {
        for (j=0,ipnt=il; j<50; j++)
        {
            x_lim=xl+(_INT)((_LONG)(xr-xl)*(_LONG)j/50L);
            for (i=ipnt; i<=ir; i++)
                if (x[i_point[i]]>=x_lim)   break;
            if (i>il && x[i_point[i]]-x_lim > x_lim - x[i_point[i-1]])
                ipnt=i-1;
            else
                ipnt=i;
            height[j] =  bord_u[i_point[ipnt]];
        }
    }
    *pmed_u_bord = calc_mediana(height,HWRMin(50,n_height));         //HWRMin(50,n_height)
    
    if (height!=_NULL)
        HWRMemoryFree(height);
    return(SUCCESS);
}

/***************************************************************************/
_BOOL correct_narrow_segments(p_EXTR extr,p_INT pn_extr,p_SHORT bord_opp,
                              _UCHAR type,_INT med_height,_INT max_height, _INT lang )
{
    _BOOL	del=_FALSE;
    _INT	n_extr=*pn_extr;
    _INT	i,sign=0, n_inside, n_narrow;
    _LONG   CONST_NARROW=8, CONST_INSIDE=2;
    
    if (lang == LANGUAGE_GERMAN)
    {
        CONST_NARROW=10;
        CONST_INSIDE=3;
    }
    if (type==MINW)
        sign=1;
    if (type==MAXW)
        sign=-1;
    
    for (i=0,n_inside=0,n_narrow=0; i<n_extr; i++)
    {
        extr[i].susp=0;
        if (sign*(bord_opp[extr[i].i] - extr[i].y) < 0)
        {
            extr[i].susp=NARROW_LINE;
            n_narrow++;
            continue;
        }
        if ( sign*CONST_NARROW*(bord_opp[extr[i].i] - extr[i].y) < max_height )
        {
            extr[i].susp=NARROW_LINE;
            n_narrow++;
            continue;
        }
        if ( sign*CONST_INSIDE*(bord_opp[extr[i].i] - extr[i].y) < med_height && extr[i].pspecl->attr!=I_MIN)
        {
            _INT dyl = (i>0) ? sign*(extr[i].y-extr[i-1].y) : 0;
            _INT dyr = (i+1<n_extr) ? -sign*(extr[i+1].y-extr[i].y) : 0;
            
            if (100L*dyl>=10L*med_height || 100L*dyr>=10L*med_height)
            {
                extr[i].susp=INSIDE_LINE;
                n_inside++;
            }
        }
    }
    
    if (n_narrow>0)
    {
        for (i=0; i<n_extr; i++)
        {
            if (extr[i].susp==NARROW_LINE)
                (extr[i].pspecl)->code=INSIDE_LINE;
        }
        delete_line_extr(extr,pn_extr,NARROW_LINE);
        del=_TRUE;
    }
    if (n_inside>0 /*&& n_inside<=ONE_HALF(n_extr)*/)
    {
        for (i=0; i<n_extr; i++)
        {
            if (extr[i].susp==INSIDE_LINE)
                (extr[i].pspecl)->code=INSIDE_LINE;
        }
        delete_line_extr(extr,pn_extr,INSIDE_LINE);
        del=_TRUE;
    }
    
    return(del);
}

/***************************************************************************/

_BOOL correct_narrow_ends(p_EXTR extr,p_INT pn_extr,p_EXTR extr_opp,
                          _INT n_extr_opp,_INT height,_UCHAR type)
{
    _INT i, j, n_ins=0;
    if (type==BEG)
    {
        while (n_ins<n_extr_opp && extr_opp[n_ins].x < extr[0].x)
            n_ins++;
        for (j=n_ins-1; j>=0; j--)
        {
            for (i=(*pn_extr); i>0; i--)
                extr[i]=extr[i-1];
            
            extr[0].x=extr_opp[j].x;
            extr[0].y=(_SHORT)(extr_opp[j].y + height);
            extr[0].i=extr_opp[j].i;
            extr[0].susp=RET_ON_LINE;
            extr[0].pspecl=_NULL;
            (*pn_extr)++;
        }
    }
    if (type==END)
    {
        _INT n_extr=*pn_extr;
        while (n_ins<n_extr_opp && extr_opp[n_extr_opp-1-n_ins].x > extr[n_extr-1].x)
            n_ins++;
        for (j=n_extr_opp-n_ins; j<n_extr_opp; j++)
        {
            extr[n_extr].x=extr_opp[j].x;
            extr[n_extr].y=(_SHORT)(extr_opp[j].y + height);
            extr[n_extr].i=extr_opp[j].i;
            extr[n_extr].susp=RET_ON_LINE;
            extr[n_extr].pspecl=_NULL;
            n_extr++;
        }
        *pn_extr=n_extr;
    }
    return(_TRUE);
}
/***************************************************************************/

_BOOL numbers_in_text(low_type _PTR D, p_SHORT bord_u, p_SHORT bord_d)
{
    p_SPECL cur,wrk,pend;
    _BOOL	sub_in_stroke, super_in_stroke, all_sub, all_super, covered_max;
    _INT	n_str = 0, n_norm_str=0, n_pnt=0, x1line, x2line, x1sub, x2sub;
    _INT    ibeg_str = 0, iend_str=0, ibeg_neib_str, iend_neib_str;
    _INT    iLeft, iRight, height, dy_cur, dy_max = 0, dy_min = ALEF;
    p_SHORT x = D->buffers[0].ptr, y = D->buffers[1].ptr, i_back = D->buffers[2].ptr;
    _RECT	stroke_box, neib_stroke_box;
    
    classify_num_strokes(D,_NULL);
    all_sub=all_super=_TRUE;
    cur=D->specl;
    
    while (cur!=_NULL)
    {
        if (cur->mark==BEG)
            ibeg_str=cur->ibeg;
        
        if (cur->mark==END)
        {
            n_str++;
            iend_str=cur->iend;
            if (cur->attr==PNT)
                n_pnt++;
            //      if (cur->attr==HOR)
            //        n_hor++;
            if (cur->attr==OPER)
            {
                GetTraceBox ( x,y,(_SHORT)ibeg_str,(_SHORT)iend_str,&stroke_box);   /* if it is "t"*/
                if (stroke_box.top==D->box.top)
                    return(_FALSE);
            }
            if (cur->attr==L_BRKT || cur->attr==R_BRKT)
            {
                GetTraceBox ( x,y,(_SHORT)ibeg_str,(_SHORT)iend_str,&stroke_box);
                if (5L*stroke_box.bottom<4L*D->box.bottom + 1L*D->box.top)
                    return(_FALSE);
            }
            if (cur->attr==NORM || cur->attr==STRT || cur->attr==L_BRKT)
            {
                n_norm_str++;
                GetTraceBox ( x,y,(_SHORT)ibeg_str,(_SHORT)iend_str,&stroke_box);
                dy_cur=stroke_box.bottom-stroke_box.top;
                
                if (10L*(stroke_box.right-stroke_box.left)> 14L*(stroke_box.bottom-stroke_box.top) )
                    return(_FALSE);
                /*
                 if (10L*(stroke_box.right-stroke_box.left)>
                 20L*step_of_wr
                 )
                 return(_FALSE);
                 */
                iLeft=ixMin((_SHORT)ibeg_str,(_SHORT)iend_str,x,y);
                iRight=ixMax((_SHORT)ibeg_str,(_SHORT)iend_str,x,y);
                iLeft=i_back[iLeft];
                iRight=i_back[iRight];
                height=HWRMax(bord_d[iLeft]-bord_u[iLeft], bord_d[iRight]-bord_u[iRight]);
                
                if (10L*(stroke_box.bottom-stroke_box.top)< 11L*height )
                    return(_FALSE);
                sub_in_stroke=super_in_stroke=_FALSE;
                wrk=cur->prev;
                while (wrk->mark!=BEG)
                {
                    if (wrk->mark==MAXW && wrk->code==SUB_SCRIPT)
                        sub_in_stroke=_TRUE;
                    if (wrk->mark==MINW &&
                        (wrk->code==SUPER_SCRIPT ||
                         ((wrk->attr==T_MIN || wrk->code==TAIL_MIN ||
                           wrk->code==RET_ON_LINE) &&
                          y[wrk->iend]<bord_u[i_back[wrk->iend]]) ))
                    {
                        super_in_stroke=_TRUE;
                    }
                    wrk=wrk->prev;
                }
                if ( (all_sub ==_TRUE && sub_in_stroke==_FALSE) ||  (all_super==_TRUE && super_in_stroke==_FALSE) )
                {
                    //isn't it a snowman
                    wrk=cur->next;
                    if (wrk!=_NULL)
                    {
                        ibeg_neib_str=wrk->ibeg;
                        while (wrk->mark!=END)  wrk=wrk->next;
                        iend_neib_str=wrk->iend;
                        GetTraceBox ( x,y,(_SHORT)ibeg_neib_str,(_SHORT)iend_neib_str,&neib_stroke_box);
                        if (neib_stroke_box.top > stroke_box.bottom && neib_stroke_box.left > stroke_box.right)
                        {
                            sub_in_stroke=_TRUE;
                            dy_cur=neib_stroke_box.bottom-stroke_box.top;
                        }
                        
                        if (neib_stroke_box.bottom < stroke_box.top && neib_stroke_box.left > stroke_box.right )
                        {
                            super_in_stroke=_TRUE;
                            dy_cur=stroke_box.bottom-neib_stroke_box.top;
                        }
                    }
                    wrk=cur->prev;
                    if (wrk!=_NULL)
                    {
                        iend_neib_str=wrk->iend;
                        while (wrk->mark!=BEG)  wrk=wrk->prev;
                        ibeg_neib_str=wrk->ibeg;
                        GetTraceBox ( x,y,(_SHORT)ibeg_neib_str,(_SHORT)iend_neib_str,&neib_stroke_box);
                        if (neib_stroke_box.top > stroke_box.bottom && neib_stroke_box.right > stroke_box.left )
                        {
                            sub_in_stroke=_TRUE;
                            dy_cur=neib_stroke_box.bottom-stroke_box.top;
                        }
                        if (neib_stroke_box.bottom < stroke_box.top && neib_stroke_box.right > stroke_box.left )
                        {
                            super_in_stroke=_TRUE;
                            dy_cur=stroke_box.bottom-neib_stroke_box.top;
                        }
                    }
                }
                if (sub_in_stroke==_FALSE)
                    all_sub=_FALSE;
                if (super_in_stroke==_FALSE)
                    all_super=_FALSE;
                
                //        if (n_norm_str>1 && all_sub==_FALSE && all_super==_FALSE)
                //          return(_FALSE);
                dy_max = HWRMax(dy_max,dy_cur);
                dy_min = HWRMin(dy_min,dy_cur);
            }
        }
        if (cur->next==_NULL)
            break;
        cur=cur->next;
    }
    
    if (n_norm_str>1 && all_sub==_FALSE && all_super==_FALSE)
        return(_FALSE);
    
    if (10L*dy_max>15L*dy_min)
        return(_FALSE);
    
    if (n_norm_str==0 || (n_norm_str==1 && n_str>3))
        return(_FALSE);
    
    if (n_str==2 && n_pnt==1)                   /* if it is "i"*/
        return(_FALSE);
    
    if (n_norm_str==1)
    {                                          // of or 5 ,
        if (all_sub==_FALSE) return(_FALSE);
        while (cur->attr!=NORM && cur->attr!=STRT)
            cur=cur->prev;
        if (cur->attr==STRT)
            return(_FALSE);
        pend=cur;
        while (cur->mark!=BEG)
        {
            if (cur->mark==MAXW && cur->code==ON_LINE)
            {
                x1line=x[cur->ibeg];
                x2line=x[cur->iend];
                covered_max=_FALSE;
                for (wrk=pend; wrk->mark!=BEG; wrk=wrk->prev)
                {
                    if (wrk->mark==MAXW && wrk->code==SUB_SCRIPT)
                    {
                        x1sub=x[wrk->ibeg];
                        x2sub=x[wrk->iend];
                        if (HWRMax(x1line,x2line) >= HWRMin(x1sub,x2sub) &&
                            HWRMax(x1sub,x2sub) >= HWRMin(x1line,x2line)
                            )
                        {
                            covered_max=_TRUE;
                            break;
                        }
                    }
                }
                if (covered_max==_FALSE)
                    return(_FALSE);
            }
            cur=cur->prev;
        }
    }
    
    return(_TRUE);
}


/* ************************************************************************* */
/* *               Fill RC field describing curved border                  * */
/* ************************************************************************* */

_INT FillRCNB(p_SHORT ip, _INT nip, low_type _PTR D, p_SHORT bord_u, p_SHORT bord_d)
{
    _INT   i, j;
    _INT   xorg, yorg, xsize, ysize;
    _INT   cur_lim, jprev;
    _INT   xi;
    _INT   du, dd;
    cb_type _PTR cb;
    
    xorg  = D->box.left;
    yorg  = D->box.top;
    xsize = DX_RECT(D->box);
    if  ( xsize == 0 )
        xsize = 1;
    ysize = DY_RECT(D->box);
    if  ( ysize == 0 )
        ysize = 1;
    
    cb  = &(D->rc->curv_bord);
    
    //  if  ( D->bSpecBorder )  /*CHE: was: ip == _NULL */
    //    return  FillRCSpec( cb, D ); /* Special treatment for SpecBorder */
    
    if  ( ip == _NULL  ||  nip <= 0 )
    {
        return  UNSUCCESS;
    }
    
    xi = ip[0];
    du = bord_u[xi]-yorg;
    if (du < 1)
        du = 1;
    if (du > ysize)
        du = ysize;
    dd = bord_d[xi]-yorg;
    if (dd < 1)
        dd = 1;
    if (dd > ysize)
        dd = ysize;
    (*cb)[0] = (_UCHAR)(((_LONG)du * 255l)/(_LONG)ysize);
    (*cb)[1] = (_UCHAR)(((_LONG)dd * 255l)/(_LONG)ysize);
    
    for (i = 1, jprev = 0; i < CB_NUM_VERTEX; i ++)
    {
        cur_lim  = (_INT)(((_LONG)(xsize)*(_LONG)i)/(_LONG)(CB_NUM_VERTEX-1));
        cur_lim += xorg;
        
        for (j = jprev; j < nip; j ++)
        {
            xi = ip[j];
            
            if (D->p_trace[xi].y == BREAK)
                continue;
            if (D->p_trace[xi].x < cur_lim)
                continue;
            if (bord_u[xi] == 0)
                continue;      // Not defined here
            
            du = bord_u[xi]-yorg; if (du < 1) du = 1; if (du > ysize) du = ysize;
            dd = bord_d[xi]-yorg; if (dd < 1) dd = 1; if (dd > ysize) dd = ysize;
            (*cb)[2*i]   = (_UCHAR)(((_LONG)du * 255l)/(_LONG)ysize);
            (*cb)[2*i+1] = (_UCHAR)(((_LONG)dd * 255l)/(_LONG)ysize);
            
            jprev = j;
            break;
        }
    }
    
    return SUCCESS;
}
/***************************************************************************/

#if 0

_INT  FillRCSpec( cb_type _PTR cb, low_type _PTR D )
{
    _INT                i;
    _INT              yorg, ysize, du, dd, ystepu, ystepd;
    
    
    if  ( !D->bSpecBorder )
    {
        return  UNSUCCESS;
    }
    
    yorg  = D->box.top;
    ysize = DY_RECT(D->box);  if (ysize <= 0) ysize = 1;
    
    //  ystepu   = ((D->yu_end - D->yu_beg)*10)/(CB_NUM_VERTEX-1);
    //  ystepd   = ((D->yu_end - D->yu_beg)*10)/(CB_NUM_VERTEX-1);
    
    for (i = 0; i < CB_NUM_VERTEX; i ++)
    {
        du = D->rc->stroka.dn_pos_out+D->rc->stroka.size_out -yorg;
        if (du < 1)
            du = 1;
        if (du > ysize)
            du = ysize;
        dd = D->rc->stroka.dn_pos_out-yorg;
        if (dd < 1)
            dd = 1;
        if (dd > ysize)
            dd = ysize;
        (*cb)[i*2]   = (_UCHAR)(((_LONG)du * 255l)/(_LONG)ysize);
        (*cb)[i*2+1] = (_UCHAR)(((_LONG)dd * 255l)/(_LONG)ysize);
    }
    
    return SUCCESS;
}

#endif /* if 0 */

/* ************************************************************************* */

_BOOL is_defis(low_type _PTR D,_INT n_str)
{
    p_SPECL cur;
    
    if (n_str>2 || n_str<=0)
        return(_FALSE);
    cur = D->specl;
    while(cur!=_NULL && cur->mark!=END) cur=cur->next;
    if (cur!=_NULL && cur->prev->attr!=HOR_STR)
        return(_FALSE);
    if (n_str==1)
        return(_TRUE);
    cur = cur->next;
    while (cur!=_NULL && cur->mark!=END)  cur=cur->next;
    if (cur!=_NULL && cur->prev->attr==HOR_STR)
        return(_TRUE);
    return(_FALSE);
    
}

/* ************************************************************************* */

_VOID SpecBord(low_type _PTR D,p_SHORT bord_d,p_SHORT bord_u,
               p_INT pmed_d_bord,p_INT pmed_u_bord,p_INT pmed_height,
               p_INT pn_x,_BOOL defis,p_EXTR line_max,_INT n_line_max)
{
    _INT	i,bord_d_pos=0,bord_u_pos=0,box_height;//prev_mid;
    p_SHORT i_point=D->buffers[3].ptr;
    
#ifdef USE_WORDSPLIT_PARMS
    if (D->rc->stroka.size_sure_in>=50 && D->rc->stroka.pos_sure_in>=50)
    {
        bord_d_pos=D->rc->stroka.dn_pos_in;
        bord_u_pos=D->rc->stroka.dn_pos_in - D->rc->stroka.size_in;
    }
    if (D->rc->stroka.size_sure_in>=50 && D->rc->stroka.pos_sure_in<50)
    {
        if (D->box.bottom-D->box.top < D->rc->stroka.size_in)
        {
            bord_d_pos=ONE_HALF(D->box.bottom)+ ONE_HALF(D->box.top) + ONE_HALF(D->rc->stroka.size_in);
            bord_u_pos=ONE_HALF(D->box.bottom)+ ONE_HALF(D->box.top) - ONE_HALF(D->rc->stroka.size_in);
        }
        if (D->box.bottom-D->box.top >= D->rc->stroka.size_in)
        {
            if (n_line_max>1)
            {
                for (i=0; i<n_line_max; i++)
                    bord_d[i]=line_max[i].y;
                bord_d_pos=calc_mediana(bord_d,n_line_max);
            }
            else
                bord_d_pos=D->box.bottom;
            bord_u_pos=bord_d_pos-D->rc->stroka.size_in;
        }
    }
    if (D->rc->stroka.size_sure_in<50)
#endif /* USE_WORDSPLIT_PARMS */
    {
        if (defis==0)
        {
            if (n_line_max>1)
            {
                for (i=0; i<n_line_max; i++)
                    bord_d[i]=line_max[i].y;
                bord_d_pos=calc_mediana(bord_d,n_line_max);
            }
            else
            {
                bord_d_pos = D->box.bottom;
            }
            bord_u_pos = HWRMax(D->box.top, bord_d_pos-TWO_THIRD(D->box.bottom-D->box.top));
            //             bord_u_pos=TWO_THIRD(D->box.top)+ONE_THIRD(D->box.bottom);
        }
        else
        {
            box_height = HWRMax(D->box.bottom-D->box.top,MIN_STR_HEIGHT);
            bord_d_pos = D->box.bottom+box_height;
            bord_u_pos = D->box.top-box_height;
        }
    }
    for (i=0; i<D->ii; i++)
    {
        bord_d[i] = (D->y[i] != BREAK) ?  (_SHORT)bord_d_pos : (_SHORT)0;
        bord_u[i] = (D->y[i] != BREAK) ?  (_SHORT)bord_u_pos : (_SHORT)0;
    }
    
    *pmed_u_bord=bord_u_pos;
    *pmed_d_bord=bord_d_pos;
    *pmed_height=bord_d_pos-bord_u_pos;
    *pn_x=fill_i_point(i_point,D);
}
#endif  /* __JUST_FIRST_PART */

#endif // #ifndef LSTRIP

