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

#ifndef     CDCMAIN_INCLUDED
#define     CDCMAIN_INCLUDED

#include    "bastypes.h"
#include    "hwr_sys.h"
#include    "cdctp.h"

// Pen events Status Constant --
#define     PE_PENUP         (-1)
#define     PE_PENDOWN       (-2)
#define     PE_END           (-3)
#define     PE_ERROR         (-4)

//  Encoder / Decoder Mode Constants
#define     MODE_TABLET_J1     1
#define     MODE_SCREEN_J1     2

/***************************************************************/
/*   Encoder Functions Prototype                               */
/***************************************************************/

p_CDC _FPREFIX  EncoderOpen      ( _SHORT  Mode);
_VOID _FPREFIX  EncoderClose     (p_CDC pCDC);
_BOOL _FPREFIX  EncoderAddPoints (p_CDC pCDC, _SHORT PenState, _USHORT nPoints, p_VOID pPoints);
p_VOID _FPREFIX  EncoderGetData   (p_CDC pCDC, p_LONG pDataSize);

/***************************************************************/
/*   Decoder Functions Prototype                               */
/***************************************************************/

p_DCC  _FPREFIX  DecoderOpen      ( _SHORT Mode, p_VOID pData );
_VOID  _FPREFIX  DecoderClose     ( p_DCC  pDCC  );
_SHORT _FPREFIX  DecoderGetPoints ( p_DCC  pDCC, _USHORT nPoints, p_VOID pPoints);

#endif
