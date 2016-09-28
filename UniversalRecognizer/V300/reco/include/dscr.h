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

#ifndef  DISCR_INCLUDED
#define  DISCR_INCLUDED

#include "bastypes.h"
#include "param.h"

// ******************************************************************* //

_BOOL _FPREFIX Trace2DToDct( _WORD nTrace, p_POINT pTrace,
                             _WORD  Order, p_POINT pCoeffs,
                             _WORD  nItr ,  _WORD  nFiltrItr,
                            p_LONG  pLam , p_LONG  pErr, _BOOL  fCutTail );

_BOOL _FPREFIX DctToCurve2D( _WORD Order , p_POINT pCfs,
                             _WORD Resam , p_POINT pCrv );


// ******************************************************************* //


_BOOL _FPREFIX Trace3DToDct( _WORD nTrace, p_3DPOINT pTrace,
                             _WORD  Order, p_3DPOINT pCoeffs,
                             _WORD  nItr , _WORD nFiltrItr,
                            p_LONG  pLam , p_LONG pErr, _BOOL fCutTail );

_BOOL _FPREFIX DctToCurve3D( _WORD Order , p_3DPOINT pCfs,
                             _WORD Resam , p_3DPOINT pCrv );
                                                          
#endif // DISCR_INCLUDED
