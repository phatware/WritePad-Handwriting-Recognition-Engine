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

RECO_DATA_EXTERNAL _UCHAR SQRTa[];
RECO_DATA_EXTERNAL _LONG SQRTb[];

/**************************************************************************
 *                                                                         *
 *    Square root.                                                         *
 *                                                                         *
 **************************************************************************/

_INT  HWRMathILSqrt (_LONG x)
{
    _INT  iShift;
    
    _SHORT  sq;
    if (x<0)
        return 0;
    
    for (iShift=0; x>0x07FFFL; iShift++)
        x >>= 2;
    
    sq = ((_SHORT)HWRMathISqrt ((_INT)x)) << iShift;
    
    if (sq >= 0)
        return sq;
    else
        return 0x7fff;
}

_INT  HWRMathISqrt (_INT x)
{
    unsigned  sq;
    
    if (x<0)
        return 0;
    
    if ( x < 256 )
        return ( (SQRTa [x] + 7) >> 4 );
    
    sq = SQRTa[(unsigned char) (x>>8)] ;
    
    
    if ( (_WORD)x > (_WORD)SQRTb[sq+=3] )
        sq+=3 ;
    if ( (_WORD)x < (_WORD)SQRTb[--sq] )
    {
        if ( (_WORD)x < (_WORD)SQRTb[--sq] )
        {
            if ( (_WORD)x < (_WORD)SQRTb[--sq] )
                sq-- ;
        }
    }
    
    if ((int) (x - SQRTb[sq]) > (int) (SQRTb[sq+1] - x))
        return sq+1;
    else
        return sq;
}

/**************************************************************************
 *    HWRAbs.                                                              *
 **************************************************************************/

_INT  HWRAbs (_INT iArg)
{
    return (iArg>0 ? iArg : -iArg);
}

/**************************************************************************
 *    HWRLAbs.                                                              *
 **************************************************************************/

_LONG  HWRLAbs (_LONG lArg)
{
    return (lArg>0 ? lArg : -lArg);
}


#if 0

#include <math.h>

_WORD  HWRMathSystemSqrt (_DOUBLE dArg, p_DOUBLE pdRes)
{
    if (dArg < 0)
        return _NULL;
    *pdRes = sqrt(dArg);
    return _TRUE;
}

_WORD  HWRMathSystemLSqrt (_LONG lArg, p_DOUBLE pdRes)
{
    if (lArg < 0)
        return _NULL;
    *pdRes = sqrt((_DOUBLE)lArg);
    return _TRUE;
}


/**************************************************************************
 *                                                                         *
 *    Exponent.                                                            *
 *                                                                         *
 **************************************************************************/

_WORD  HWRMathSystemExp (_DOUBLE dArg, p_DOUBLE pdRes)
{
    *pdRes = exp(dArg);
    return _TRUE;
}


/**************************************************************************
 *                                                                         *
 *    sin.                                                                 *
 *                                                                         *
 **************************************************************************/

_WORD  HWRMathSystemSin (_DOUBLE dArg, p_DOUBLE pdRes)
{
    *pdRes = sin (dArg);
    return _TRUE;
}


/**************************************************************************
 *                                                                         *
 *    cos.                                                                 *
 *                                                                         *
 **************************************************************************/

_WORD  HWRMathSystemCos(_DOUBLE dArg, p_DOUBLE pdRes)
{
    *pdRes = cos (dArg);
    return _TRUE;
}


/**************************************************************************
 *                                                                         *
 *    Arctg(Arg1/Arg2).                                                    *
 *                                                                         *
 **************************************************************************/

_WORD  HWRMathSystemAtan2 (_DOUBLE dArg1, _DOUBLE dArg2, p_DOUBLE pdRes)
{
    *pdRes = atan2 (dArg1, dArg2);
    return _TRUE;
}

/**************************************************************************
 *                                                                         *
 *    floor.                                                               *
 *                                                                         *
 **************************************************************************/

_WORD  HWRMathSystemFloor(_DOUBLE dArg, p_DOUBLE pdRes)
{
    *pdRes = floor(dArg);
    return _TRUE;
}

#endif
