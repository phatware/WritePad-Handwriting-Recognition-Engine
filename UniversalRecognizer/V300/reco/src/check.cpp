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


#include  "hwr_sys.h"
#include  "ams_mg.h"
#include  "lowlevel.h"
#include  "calcmacr.h"
#include  "def.h"

#ifdef  FORMULA
#include "frm_con.h"
#endif /*FORMULA*/

/*----------------------------------------------------------------------*/

extern   const CONSTS  const1 ;

/*----------------------------------------------------------------------*/

typedef  struct
{
    _SHORT  iBeg    ;
    _SHORT  iEnd    ;
    _SHORT  eps     ;
    _SHORT  wX      ;
    _SHORT  wY      ;
    _SHORT  wXY     ;
    _UCHAR  MaxName ;
    _UCHAR  MinName ;
}
_ENVIRONS  ;

typedef  _ENVIRONS _PTR   p_ENV ;

/*----------------------------------------------------------------------*/

static _SHORT   BigExtr( low_type _PTR low_data, _SHORT  begin, _SHORT end, _SHORT extr_axis , _SHORT  eps_fy ) ;
static _SHORT   DirectExtr( low_type _PTR  pLowData, p_ENV pEnvExtr , SPECL _PTR  pTmpSpecl , _SHORT  k ) ;
static p_SPECL  LastElemAnyKindFor  ( p_SPECL pSpecl , _UCHAR kind_of_mark  ) ;
static p_SPECL  FirstElemAnyKindFor ( p_SPECL pSpecl , _UCHAR kind_of_mark  ) ;

/**************************************************************************/

_SHORT  InitSpecl( low_type _PTR  pLowData  , _SHORT n )
{
    p_SPECL  pSpecl = pLowData->specl ;
    _SHORT   flag_init ;
    
    flag_init = SUCCESS ;
    
    HWRMemSet( (p_VOID)pSpecl , 0 , sizeof(SPECL)*n ) ;
    HWRMemSet( (p_VOID)pLowData->pAbsnum, 0, sizeof(_SHORT)*(pLowData->rmAbsnum)  ) ;
    
    pLowData->len_specl      = 1;
    pLowData->lenabs         = 0;
    pSpecl->prev             = _NULL;
    pSpecl->next             = pSpecl + 1;
    pSpecl->mark             = EMPTY;
    pSpecl->ipoint0          = UNDEF;
    pSpecl->ipoint1          = UNDEF;
    pLowData->LastSpeclIndex = 0;
    
    return( flag_init);
}

/**************************************************************************/

_SHORT  InitSpeclElement( SPECL _PTR pSpecl )
{
    _SHORT  flag_Init ;
    
    flag_Init = SUCCESS ;
    
    if  ( pSpecl != _NULL )
    {
        HWRMemSet( (p_VOID)pSpecl , 0 , sizeof(SPECL) ) ;
        pSpecl->prev = _NULL;
        pSpecl->next = _NULL;
        pSpecl->mark = EMPTY;
        pSpecl->ipoint0 = UNDEF;
        pSpecl->ipoint1 = UNDEF;
    }
    else
    {
        flag_Init = UNSUCCESS ;
    }
    return( flag_Init) ;
}

/**************************************************************************/

#define      LIM_RED_EPS_Y      1
#define      MIN_RED_EPS_Y      2

_SHORT  Extr( low_type _PTR pLowData, _SHORT eps_fy, _SHORT eps_fx ,
             _SHORT eps_fxy, _SHORT eps_fyx,
             _SHORT nMaxReduct, _SHORT extr_axis )
{
    p_POINTS_GROUP  pGroupsBorder = pLowData->pGroupsBorder ;
    _INT            lenGrBord = pLowData->lenGrBord;
    p_SHORT          pAbsnum = pLowData->pAbsnum;
    _INT            lenAbs = pLowData->lenabs;
    p_SPECL          pSpecl = pLowData->specl;
    _INT            old_lenSpecl  = pLowData->len_specl;
    p_SPECL          tmpSpecl;
    _INT            il , im;
    _INT            stAbsnum;
    _INT            iBeg, iEnd;
    
    _BOOL           fl0;
    
    if   ( lenGrBord <= 0 )
    {
        il = 0 ;
        goto   ERR ;
    }
    
    for  ( il = 0 , stAbsnum = 0  ;  il < lenGrBord  ;  il++ )
    {
        iBeg = ( pGroupsBorder + il )->iBeg ;
        iEnd = ( pGroupsBorder + il )->iEnd ;
        fl0  = _TRUE ;
        
        for  ( im = stAbsnum;  im < lenAbs;  im++ )
        {
            tmpSpecl = pSpecl + *(pAbsnum + im)  ;
            
            if  (  ( iBeg == tmpSpecl->ibeg )  && ( iEnd == tmpSpecl->iend ) )
            {
                stAbsnum = im ;
                fl0 = _FALSE  ;
                break ;
            }
        }
        
        if  ( fl0 == _FALSE )
            continue ;
        
        if  ( Mark( pLowData , BEG , 0, 0, 0, iBeg, iBeg, iBeg, iBeg  ) == UNSUCCESS )
            goto  ERR  ;
        
        if  ( extr_axis & Y_DIR )
        {
            _INT     epsred;
            _INT     nReduct = 0;
            
            p_SHORT  pLenSpecl    = &(pLowData->len_specl) ;
            _INT    old_lenSpecl = *pLenSpecl ;
            
            while  ( ( old_lenSpecl == *pLenSpecl ) && ( nReduct <= nMaxReduct ) )
            {
                nReduct++ ;
                epsred = eps_fy / nReduct ;
                
                if  ( epsred <= LIM_RED_EPS_Y )
                {
                    epsred  = MIN_RED_EPS_Y  ;
                    nReduct = nMaxReduct + 1 ;
                }
                
                if  ( BigExtr( pLowData, iBeg, iEnd, Y_DIR, epsred )
                     == UNSUCCESS  )
                    goto ERR ;
            }
        }
        
        if  ( extr_axis & X_DIR )
        {
            if  ( BigExtr( pLowData, iBeg, iEnd,  X_DIR, eps_fx )   == UNSUCCESS )
                goto  ERR  ;
        }
        
        if  ( extr_axis & XY_DIR )
        {
            if  ( BigExtr( pLowData, iBeg, iEnd, XY_DIR  , eps_fxy     ) == UNSUCCESS )
                goto  ERR ;
        }
        
        if  ( extr_axis & ( XY_DIR | YX_DIR ) )
        {
            if  ( BigExtr( pLowData, iBeg,  iEnd, YX_DIR  , eps_fyx     ) == UNSUCCESS )
                goto  ERR ;
        }
        
        if  ( Mark( pLowData, END,  0, 0 , 0, iEnd, iEnd ,  iEnd , iEnd  ) == UNSUCCESS )
            goto  ERR ;
    }
    
    //   QUIT:
    return   SUCCESS   ;
    
ERR :
    
    pLowData->iBegBlankGroups = (_SHORT)il;
    pLowData->len_specl = (_SHORT)old_lenSpecl;

    return   UNSUCCESS ;
    
}

/**************************************************************************/

#define    WX0          2                    /*  Weights corresponding   */
/*  by two directions of YX */
#define    WX1          1                    /*  scanning .              */

/*     This "define" in the same time the meaning of the next "define"    */

#define   Z(i)         ( wx * x[i]  + wy * y[i] ) / wxy


/*------------------------------------------------------------------------*/
/*     That "define" is the way of calculation of the previous "define"   */
/*     when wx and wy are either 0 or +-1 :                               */

#define   V(i)         (  (  ((wx==0)? 0:((wx>0)? x[(i)]:(-x[(i)]))) + ((wy==0)? 0:((wy>0)? y[(i)]:(-y[(i)]))) ) / wxy )


/*------------------------------------------------------------------------*/


static _SHORT BigExtr( low_type _PTR pLowData, _SHORT iBeg, _SHORT iEnd, _SHORT extr_axis, _SHORT eps )
{
    p_SPECL    pSpecl = pLowData->specl;
    p_SPECL    maxTmp, minTmp, pLastElement ;
    SPECL      tmpSpecl;
    
    _SHORT     flagExtr = SUCCESS;
    _INT       k;
    
    _ENVIRONS  envExtr;
    p_ENV      pEnvExtr = &envExtr ;
    
    
    envExtr.iBeg = iBeg ;
    envExtr.iEnd = iEnd ;
    envExtr.eps  = eps  ;
    
    
    if  ( extr_axis == Y_DIR )
    {
        envExtr.MaxName = MAXW  ;
        envExtr.wX =  0 ;
        envExtr.MinName = MINW  ;
        envExtr.wY =  1 ;
    }
    else  if  ( extr_axis == X_DIR )
    {
        envExtr.MaxName = _MAXX ;
        envExtr.wX =  1 ;
        envExtr.MinName = _MINX ;
        envExtr.wY =  0 ;
    }
    else  if  ( extr_axis == XY_DIR )
    {
        envExtr.MaxName = MAXXY ;
        envExtr.wX =  1 ;
        envExtr.MinName = MINXY ;
        envExtr.wY =  1 ;
    }
    
    else  if  ( extr_axis != YX_DIR )
    {
        flagExtr = UNSUCCESS ;
        goto  QUIT ;
    }
    
    /*---------------------------------------------------------------------*/
    /*                   Search for big extremums                          */
    /*---------------------------------------------------------------------*/
    
    if  ( extr_axis != YX_DIR )
    {
        p_SHORT      x = pLowData->x;
        p_SHORT      y = pLowData->y;
        _INT         eps = pEnvExtr->eps;
        _INT         wx = pEnvExtr->wX;
        _INT         wy = pEnvExtr->wY;
        _INT         wxy   ;
        _UCHAR       MaxName   = pEnvExtr->MaxName ;
        _UCHAR       MinName   = pEnvExtr->MinName ;
        
        SPECL _PTR   pTmpSpecl = &tmpSpecl;
        _UCHAR       prevMark  = EMPTY;
        
        _INT         extV  , maxV , minV;
        _INT         iOpen , iClose;
        _INT         jMax  , jMin;
        _INT         j     ;
        
        
        envExtr.wXY = HWRAbs( envExtr.wX ) + HWRAbs( envExtr.wY ) ;
        
        if  ( envExtr.wXY == 0 )
        {
            flagExtr = UNSUCCESS ;
            goto  QUIT ;
        }
        
        InitSpeclElement( pTmpSpecl ) ;
        wxy      = pEnvExtr->wXY      ;
        prevMark = ( pLowData->specl + pLowData->len_specl - 1 )->mark ;
        
        for  ( k = iBeg  ;  k <= iEnd  ;  k++ )
        {
            /*---------------------------------------------------------------------*/
            /*                 Preliminary search of extremums                     */
            /*---------------------------------------------------------------------*/
            
            if  ( (  ( V(k) >= V(k+1) )  &&  ( V(k) >= V(k-1) )  )
                 ||  (  ( V(k) <= V(k+1) )  &&  ( V(k) <= V(k-1) )  )
                 ||  ( k == iBeg )  ||   ( k == iEnd ) )
            {
                extV = V(k) ;
                
                j = k ;
                while  ( (HWRAbs(extV - V(j)) < eps) && (j >= iBeg) )
                    j-- ;
                iOpen = j+1 ;
                
                j = k ;
                while  ( (HWRAbs(extV - V(j)) < eps) && (j <= iEnd) )
                    j++ ;
                iClose  = j-1 ;
            }
            else
            {
                continue  ;
            }
            
            /*---------------------------------------------------------------------*/
            /*                         Search for maximums                         */
            /*---------------------------------------------------------------------*/
            
            if  ( ( ( ( iOpen != iBeg )
                        &&  ( V(iOpen-1)    < extV )
                        &&  ( ( V(iClose+1) < extV ) || (iClose == iEnd) )  )
                     ||
                     ( ( iClose != iEnd)
                      &&  ( V(iClose+1)   < extV )
                      &&  ( ( V(iOpen-1)  < extV ) || (iOpen  == iBeg) ) ) )
                 && ( prevMark != MaxName )  )
            {
                maxV = extV ;        jMax = k ;
                for  ( j = iOpen  ;  j <= iClose  ;  j++ )
                {
                    if  ( V(j) > maxV )
                    {
                        jMax = j ;
                        maxV = V(j) ;
                    }
                }
                
                for  ( j = jMax ;
                      V(j) == maxV  &&  j <= iEnd  ;
                      j++    ) ;
                jMax = MEAN_OF( jMax , (j-1) ) ;
                
                if  ( jMax != k )
                {
                    j = jMax ;
                    while  ( ( maxV-V(j) < eps ) && (j >= iBeg) )
                        j-- ;
                    iOpen  = j+1 ;
                    
                    j = jMax ;
                    while  ( ( maxV-V(j) < eps ) && (j <= iEnd) )
                        j++ ;
                    iClose = j-1 ;
                }
                
                InitSpeclElement( pTmpSpecl ) ;
                pTmpSpecl->ibeg    = (_SHORT)iOpen   ;
                pTmpSpecl->iend    = (_SHORT)iClose  ;
                pTmpSpecl->ipoint0 = (_SHORT)jMax    ;
                pTmpSpecl->ipoint1 = UNDEF   ;
                pTmpSpecl->mark    = MaxName ;
                prevMark           = MaxName ;
            }
            
            /*---------------------------------------------------------------------*/
            /*                         Search for minimums                         */
            /*---------------------------------------------------------------------*/
            
            else  if ( (  ( ( iOpen != iBeg )
                        &&  ( V(iOpen-1)    > extV )
                        &&  ( ( V(iClose+1) > extV ) || (iClose == iEnd)) )
                     ||
                     ( ( iClose != iEnd)
                      &&  ( V(iClose+1)   > extV )
                      &&  ( ( V(iOpen-1)  > extV ) || (iOpen  == iBeg)) ) )
                 &&
                 ( prevMark != MinName )  )
            {
                minV = extV ;        jMin = k ;
                for  ( j = iOpen  ;  j <= iClose  ;  j++ )
                {
                    if  ( V(j) < minV )
                    { jMin = j ;   minV = V(j) ;  }
                }
                
                for  ( j = jMin  ;
                      V(j) == minV  &&  j <= iEnd  ;
                      j++     ) ;
                jMin = MEAN_OF( jMin , (j-1) ) ;
                
                if  ( jMin != k )
                {
                    j = jMin ;
                    while  ( ( V(j)-minV < eps ) && (j <= iEnd) ) j++ ;
                    iClose = j-1 ;
                    
                    j = jMin ;
                    while  ( ( V(j)-minV < eps ) && (j >= iBeg) ) j-- ;
                    iOpen = j+1 ;
                }
                
                InitSpeclElement( pTmpSpecl ) ;
                pTmpSpecl->ibeg    = (_SHORT)iOpen   ;
                pTmpSpecl->iend    = (_SHORT)iClose  ;
                pTmpSpecl->ipoint0 = (_SHORT)jMin    ;
                pTmpSpecl->ipoint1 = UNDEF   ;
                pTmpSpecl->mark    = MinName ;
                prevMark           = MinName ;
            }
            
            
            if  ( tmpSpecl.mark != EMPTY )
            {
                if  ( MarkSpecl( pLowData, &tmpSpecl )  == UNSUCCESS )
                {
                    flagExtr = UNSUCCESS  ;
                    goto  QUIT ;
                }
                
                k = tmpSpecl.iend             ;
                InitSpeclElement( pTmpSpecl ) ;
            }
        }
    }
    else
    {
        envExtr.MaxName = MAXYX ;
        envExtr.MinName = MINYX ;
        envExtr.wY = -1 ;
        
        for ( k = iBeg  ;  k <= iEnd  ;  k++ )
        {
            envExtr.wX =  WX1 ;
            envExtr.wXY = HWRAbs( envExtr.wX ) + HWRAbs( envExtr.wY );
            
            DirectExtr( pLowData , pEnvExtr , &tmpSpecl , k ) ;
            
            if  ( tmpSpecl.mark == EMPTY )
            {
                envExtr.wX  = WX0 ;
                envExtr.wXY = HWRAbs( envExtr.wX ) +
                HWRAbs( envExtr.wY ) ;
                DirectExtr( pLowData, pEnvExtr, &tmpSpecl, k  ) ;
            }
            
            if  ( ( tmpSpecl.mark != EMPTY )  && ( tmpSpecl.iend >= k  ) )
            {
                if  ( MarkSpecl( pLowData, &tmpSpecl )
                     == UNSUCCESS )
                { flagExtr = UNSUCCESS ;   goto  QUIT ; }
                
                if  ( tmpSpecl.iend >= k )
                    k = tmpSpecl.iend  ;
            }
        }
    }
    
    /*---------------------------------------------------------------------*/
    /*                  Increasing extremums borders                       */
    /*---------------------------------------------------------------------*/
    
    pLastElement = pSpecl + pLowData->len_specl - 1 ;
    
    maxTmp       = LastElemAnyKindFor( pLastElement , envExtr.MaxName ) ;
    minTmp       = LastElemAnyKindFor( pLastElement , envExtr.MinName ) ;
    
    if  ( ( maxTmp != _NULL  &&  maxTmp->iend < iEnd )  &&
         ( minTmp != _NULL  &&  minTmp->iend < iEnd ) )
    {
        if  ( maxTmp->iend > minTmp->iend )
        {
            maxTmp->iend = (_SHORT)iEnd  ;
        }
        
        else
        {
            minTmp->iend = (_SHORT)iEnd  ;
        }
    }
    
    
    maxTmp = FirstElemAnyKindFor( pLastElement  , envExtr.MaxName  ) ;
    minTmp = FirstElemAnyKindFor( pLastElement  , envExtr.MinName  ) ;
    
    if  ( ( maxTmp != _NULL  &&  maxTmp->ibeg > iBeg )  &&
         ( minTmp != _NULL  &&  minTmp->ibeg > iBeg ) )
    {
        if  ( maxTmp->ibeg < minTmp->ibeg )
        {
            maxTmp->ibeg = (_SHORT)iBeg ;
        }
        else
        {
            minTmp->ibeg = (_SHORT)iBeg  ;
        }
    }
    
QUIT:
    return( flagExtr ) ;
}

/**************************************************************************/

static _SHORT  DirectExtr( low_type _PTR  pLowData, p_ENV pEnvExtr, SPECL _PTR  pTmpSpecl, _SHORT  k )
{
    p_SHORT  x           = pLowData->x;
    p_SHORT  y           = pLowData->y;
    
    _INT     iBeg        = pEnvExtr->iBeg;
    _INT     iEnd        = pEnvExtr->iEnd;
    _INT     eps         = pEnvExtr->eps;
    _INT     wx          = pEnvExtr->wX;
    _INT     wy          = pEnvExtr->wY;
    _INT     wxy         = pEnvExtr->wXY;
    _UCHAR   MaxName     = pEnvExtr->MaxName;
    _UCHAR   MinName     = pEnvExtr->MinName;
    
    _UCHAR   prevMark;
    _SHORT   flagDirExtr = SUCCESS;
    
    _INT     extZ  , maxZ   , minZ;
    _INT     iOpen , iClose ;
    _INT     jMax  , jMin   ;
    _INT     j     ;
    
    
    prevMark    = ( pLowData->specl + pLowData->len_specl - 1 )->mark ;
    InitSpeclElement( pTmpSpecl ) ;
    
    /*---------------------------------------------------------------------*/
    /*                 Preliminary search of extremums                     */
    /*---------------------------------------------------------------------*/
    
    if  ((  ( Z(k) >= Z(k+1) )  &&  ( Z(k) >= Z(k-1) )  )
         ||  ( ( Z(k) <= Z(k+1) )  &&  ( Z(k) <= Z(k-1) )  ) ||  ( k == iBeg )  ||   ( k == iEnd ) )
    {
        extZ = Z(k) ;
        
        j = k ;
        while  ( (HWRAbs(extZ - Z(j)) < eps) && (j >= iBeg) )  j-- ;
        iOpen = j+1 ;
        
        j = k ;
        while  ( (HWRAbs(extZ - Z(j)) < eps) && (j <= iEnd) )  j++ ;
        iClose  = j-1 ;
    }
    else
    {
        goto  QUIT  ;
    }
    
    /*---------------------------------------------------------------------*/
    /*                         Search for maximums                         */
    /*---------------------------------------------------------------------*/
    
    if  ( ( ( ( iOpen != iBeg ) &&  ( Z(iOpen-1)    < extZ )
                &&  ( ( Z(iClose+1) < extZ ) || (iClose == iEnd) ) ) ||
             (( iClose != iEnd)
              &&  ( Z(iClose+1) < extZ ) && ( ( Z(iOpen-1)  < extZ ) || (iOpen  == iBeg) ) ) )
         && ( prevMark != MaxName )  )
    {
        maxZ = extZ ;     jMax = k ;
        for  ( j = iOpen  ;  j <= iClose  ;  j++ )
        {
            if  ( Z(j) > maxZ )
            { jMax = j ;   maxZ = Z(j) ;  }
        }
        
        for  ( j = jMax  ;  Z(j) == maxZ  &&  j <= iEnd  ;  j++ ) ;
        jMax = MEAN_OF( jMax , (j-1) ) ;
        
        if  ( jMax != k )
        {
            j = jMax ;
            while  ( ( maxZ-Z(j) < eps ) && (j >= iBeg) )   j-- ;
            iOpen  = j+1 ;
            
            j = jMax ;
            while  ( ( maxZ-Z(j) < eps ) && (j <= iEnd) )   j++ ;
            iClose = j-1 ;
        }
        
        pTmpSpecl->ibeg    = (_SHORT)iOpen   ;
        pTmpSpecl->iend    = (_SHORT)iClose  ;
        pTmpSpecl->ipoint0 = (_SHORT)jMax    ;
        pTmpSpecl->ipoint1 = UNDEF   ;
        pTmpSpecl->mark    = MaxName ;
    }
    
    /*---------------------------------------------------------------------*/
    /*                         Search for minimums                         */
    /*---------------------------------------------------------------------*/
    
    else if  ( ( ( ( iOpen != iBeg ) &&  ( Z(iOpen-1) > extZ )
                    &&  ( ( Z(iClose+1) > extZ ) || (iClose == iEnd) ) ) ||
                 ( ( iClose != iEnd) &&  ( Z(iClose+1) > extZ )
                  &&  ( ( Z(iOpen-1)  > extZ ) || (iOpen  == iBeg) ) ) )
             && ( prevMark != MinName )  )
    {
        minZ = extZ ;
        jMin = k ;
        for  ( j = iOpen  ;  j <= iClose  ;  j++ )
        {
            if  ( Z(j) < minZ )
            {
                jMin = j ;
                minZ = Z(j) ;
            }
        }
        
        for  ( j = jMin  ;  Z(j) == minZ  &&  j <= iEnd; j++ )
            ;
        jMin = MEAN_OF( jMin , (j-1) ) ;
        
        if  ( jMin != k )
        {
            j = jMin ;
            while  ( ( Z(j)-minZ < eps ) && (j <= iEnd) )
                j++ ;
            iClose = j-1 ;
            
            j = jMin ;
            while  ( ( Z(j)-minZ < eps ) && (j >= iBeg) )
                j-- ;
            iOpen = j+1 ;
        }
        
        pTmpSpecl->ibeg    = (_SHORT)iOpen   ;
        pTmpSpecl->iend    = (_SHORT)iClose  ;
        pTmpSpecl->ipoint0 = (_SHORT)jMin    ;
        pTmpSpecl->ipoint1 = UNDEF   ;
        pTmpSpecl->mark    = MinName ;
    }

QUIT:
    return( flagDirExtr ) ;
}

/**************************************************************************/


static p_SPECL LastElemAnyKindFor ( p_SPECL pSpecl , _UCHAR kind_of_mark )
{
    for ( ; pSpecl!=_NULL ; pSpecl=pSpecl->prev )
    {
        if  ( pSpecl->mark == BEG )
        {
            pSpecl = _NULL;
            break;
        }
        if  ( pSpecl->mark == kind_of_mark )
            break;
    }
    return  ( pSpecl ) ;
}


/**************************************************************************/

static p_SPECL FirstElemAnyKindFor ( p_SPECL pSpecl , _UCHAR kind_of_mark )
{
    p_SPECL  pTmp , pFirst ;
    
    pTmp = pSpecl ;     pFirst = _NULL ;
    while  ( pTmp->mark != BEG )
    {
        if  ( pTmp->mark == kind_of_mark )
        {
            pFirst = pTmp ;
        }
        
        pTmp = pTmp->prev ;
    }
    
    return ( pFirst ) ;
}

/**************************************************************************/


_SHORT  Mark( low_type _PTR  pLowData,  _UCHAR mark, _UCHAR code, _UCHAR  attr, _UCHAR other,
             _SHORT begin , _SHORT end , _SHORT ipoint0, _SHORT ipoint1 )
{
    _SHORT _PTR   pLspecl   = &(pLowData->len_specl) ;
    _SHORT _PTR   pLenAbs   = &(pLowData->lenabs);
    p_SPECL        pSpecl    = pLowData->specl;
    _INT          iMarked   = *pLspecl ;
    p_SPECL        pMrkSpecl = pSpecl + iMarked ;
    SPECL        tmpSpecl  ;
    
    
    tmpSpecl.mark    = mark    ;
    tmpSpecl.code    = code    ;
    tmpSpecl.attr    = attr    ;
    tmpSpecl.other   = other   ;
    tmpSpecl.ibeg    = begin   ;
    tmpSpecl.iend    = end     ;
    tmpSpecl.ipoint0 = ipoint0 ;
    tmpSpecl.ipoint1 = ipoint1 ;
    
    if  ( NoteSpecl( pLowData , &tmpSpecl, ( SPECL _PTR ) pSpecl , pLspecl, SPECVAL ) == _FALSE  )
        goto  RET_UNSUCC ;
    
    pMrkSpecl->prev = pSpecl + pLowData->LastSpeclIndex  ;
    pMrkSpecl->next = (p_SPECL) _NULL ;
    ( pSpecl+pLowData->LastSpeclIndex )->next = pMrkSpecl ;
    pLowData->LastSpeclIndex = (_SHORT)iMarked ;
    
    switch( mark )
    {
        case  DOT    :
        case  STROKE :
        case  SHELF  :
            if  ( (*pLenAbs) < ( pLowData->rmAbsnum - 1 ) )
            {
                *( pLowData->pAbsnum + *pLenAbs ) = (_SHORT)iMarked ;
                (*pLenAbs)++ ;
            }
            else
            {
                goto  RET_UNSUCC;
            }
            
        default :
            break ;
    }
    
    return  SUCCESS ;
    
RET_UNSUCC:
    return  UNSUCCESS ;
    
}

 /**************************************************************************/

_SHORT  MarkSpecl( low_type _PTR pLowData, SPECL _PTR  p_tmpSpecl )
{
    _SHORT _PTR  pLspecl   = &(pLowData->len_specl) ;
    _SHORT _PTR  pLenAbs   = &(pLowData->lenabs);
    _SHORT       iMarked   = *pLspecl;
    p_SPECL       pSpecl    = pLowData->specl;
    p_SPECL       pMrkSpecl = pSpecl + iMarked;
    
    
    if  ( NoteSpecl( pLowData , p_tmpSpecl , ( SPECL _PTR ) pSpecl , pLspecl, SPECVAL ) == _FALSE  )
        goto  RET_UNSUCC;
    
    pMrkSpecl->prev = pSpecl + pLowData->LastSpeclIndex ;
    pMrkSpecl->next = (p_SPECL) _NULL;
    (pSpecl+pLowData->LastSpeclIndex)->next = pMrkSpecl ;
    pLowData->LastSpeclIndex = (_SHORT)iMarked;
    
    switch( p_tmpSpecl->mark )
    {
        case  DOT    :
        case  STROKE :
        case  SHELF  :
            
            if  ( (*pLenAbs) < ( pLowData->rmAbsnum - 1 ) )
            {
                *( pLowData->pAbsnum + *pLenAbs ) = (_SHORT)iMarked ;
                (*pLenAbs)++ ;
            }
            else
            {
                goto  RET_UNSUCC;
            }
            
        default :
            break ;
    }
    
    return  SUCCESS ;
    
RET_UNSUCC:
    return  UNSUCCESS ;
}

/**************************************************************************/


_BOOL  NoteSpecl( low_type _PTR  pLowData,   SPECL _PTR  pTmpSpecl,
                 SPECL _PTR  pSpecl,  _SHORT _PTR  pLspecl, _SHORT limSpecl  )
{
    p_SHORT        ind_Back  = pLowData->buffers[2].ptr;
    SPECL  _PTR    pNew = ( SPECL _PTR ) (pSpecl + *pLspecl );
    _UCHAR         mark      = pTmpSpecl->mark;
    _INT           iPoint0   = pTmpSpecl->ipoint0;
    _INT           iPoint1   = pTmpSpecl->ipoint1;
    _BOOL          flagNote  = _TRUE ;
    
    if   ( *pLspecl < limSpecl-1 )
    {
        pNew->mark  = mark  ;
        pNew->code  = pTmpSpecl->code  ;
        pNew->attr  = pTmpSpecl->attr  ;
        pNew->other = pTmpSpecl->other ;
        
        if  ( ( mark == SHELF  )  || ( mark == DOT)  || ( mark == STROKE ))
        {
            pNew->ibeg    = ind_Back[pTmpSpecl->ibeg] ;
            pNew->iend    = ind_Back[pTmpSpecl->iend] ;
            
            if  ( iPoint0 != UNDEF )
                pNew->ipoint0 = ind_Back[iPoint0] ;
            else
                pNew->ipoint0 = iPoint0  ;
            
            if  ( iPoint1 != UNDEF )
                pNew->ipoint1 = ind_Back[iPoint1] ;
            else
                pNew->ipoint1 = iPoint1  ;
        }
        else
        {
            pNew->ibeg    = pTmpSpecl->ibeg ;
            pNew->iend    = pTmpSpecl->iend ;
            pNew->ipoint0 = iPoint0  ;
            pNew->ipoint1 = iPoint1  ;
        }
        
        (*pLspecl)++ ;
    }
    else
    {
        flagNote = _FALSE;
    }
    
    return( flagNote ) ;
}


#endif  //#ifndef LSTRIP
