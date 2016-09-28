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

#ifndef FLOATS_INCLUDED
#define FLOATS_INCLUDED

#include "bastypes.h"

#if HWR_SYSTEM == HWR_WINDOWS || HWR_SYSTEM == HWR_DOS || HWR_SYSTEM == HWR_ANSI || HWR_SYSTEM == HWR_EPOC32 /* substitute floating   */
/* point operations      */
#define  HWRfl_add(a,b,c)   ((*(c))=((a)+(b)))   /*                       */
#define  HWRfl_sub(a,b,c)   ((*(c))=((a)-(b)))   /*                       */
#define  HWRfl_mul(a,b,c)   ((*(c))=((a)*(b)))   /*                       */
#define  HWRfl_div(a,b,c)   ((*(c))=((a)/(b)))   /*                       */
#define  HWRfl_assign(a,b)  ((*(a))=(b))         /*                       */
#define  HWRfl_gt(a,b)      ((a)>(b))            /*                       */
#define  HWRfl_ge(a,b)      ((a)>=(b))           /*                       */
#define  HWRfl_lt(a,b)      ((a)<(b))            /*                       */
#define  HWRfl_le(a,b)      ((a)<=(b))           /*                       */
#define  HWRfl_eq(a,b)      ((a)==(b))           /*                       */
#define  HWRfl_ne(a,b)      ((a)!=(b))           /*                       */
#define  HWRitof(a,b)       ((*(b))=((_DOUBLE)(a)))/*                     */
#define  HWRltof(a,b)       ((*(b))=((_DOUBLE)(a)))/*                     */
#define  HWRftoi(a)         ((_SHORT)(a))        /*                       */
#define  HWRftol(a)         ((_LONG)(a))         /*                       */
/*                       */
#else                                             /*                       */
/*                       */
_VOID  HWRfl_add(_DOUBLE add1,_DOUBLE add2,p_DOUBLE res);/*               */
/*                       */
_VOID  HWRfl_sub(_DOUBLE sub1,_DOUBLE sub2,p_DOUBLE res);/*               */
/*                       */
_VOID  HWRfl_mul(_DOUBLE mul1,_DOUBLE mul2,p_DOUBLE res);/*               */
/*                       */
_VOID  HWRfl_div(_DOUBLE div1,_DOUBLE div2,p_DOUBLE res);/*               */
/*                       */
_VOID  HWRfl_assign(p_DOUBLE res,_DOUBLE ass);   /*                       */
/*                       */
_BOOL  HWRfl_gt(_DOUBLE op1,_DOUBLE op2);        /*                       */
/*                       */
_BOOL  HWRfl_ge(_DOUBLE op1,_DOUBLE op2);        /*                       */
/*                       */
_BOOL  HWRfl_lt(_DOUBLE op1,_DOUBLE op2);        /*                       */
/*                       */
_BOOL  HWRfl_le(_DOUBLE op1,_DOUBLE op2);        /*                       */
/*                       */
_BOOL  HWRfl_eq(_DOUBLE op1,_DOUBLE op2);        /*                       */
/*                       */
_BOOL  HWRfl_ne(_DOUBLE op1,_DOUBLE op2);        /*                       */
/*                       */
_VOID  HWRitof(_SHORT op1,p_DOUBLE res);         /*                       */
/*                       */
_VOID  HWRltof(_LONG op1,p_DOUBLE res);          /*                       */
/*                       */
_SHORT HWRftoi(_DOUBLE op1);                     /*                       */
/*                       */
_LONG  HWRftol(_DOUBLE op1);                     /*                       */
/*                       */
#endif                                   /* floating point operations      */

#endif  /*  FLOATS_INCLUDED  */
