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

#ifndef _REC_MAIN_H
#define _REC_MAIN_H

#include "dti.h"

#define MAX_LINELEN 255

#define VRIRCX(p) ((p)->prc)
#define VRITRACE(p) ((p)->pTrace)
#define VRIXRDATA(p) ((p)->pxrdata)
#define VRIRECWORD(p) ((p)->prwords)
#define VRIMAINBUF(p) ((p)->pmbuf)
#define VRIALIAS(p) ((p)->pavars)
#define VRISYGRAPH(p) ((p)->prwg)

#define RIRCX(p) ((rc_type _PTR)VRIRCX(p))
#define RITRACE(p) ((PS_point_type _PTR)VRITRACE(p))
#define RIXRDATA(p) ((xrdata_type _PTR)VRIXRDATA(p))
#define RIRECWORD(p) ((rec_w_type _PTR)VRIRECWORD(p))
#define RIMAINBUF(p) ((DES_DATA_TYPE _PTR)VRIMAINBUF(p))
#define RIALIAS(p) ((ALS_NODE_TYPE _PTR)VRIALIAS(p))
#define RISYGRAPH(p) ((RWG_type _PTR)VRISYGRAPH(p))

#define  MAX_BUF_POINTS    32000        /* Size of main points storage buf */
#define  _PASS_BORDER      1
/*
This structure is borrowed from penwin.h (16 bit penwindows)
to allow us use boxedit
                      cx_box
           x_origin <........>
  y_origin o--------+----A---+ A
           |        |    :   | :
           |    A   |    :cy_base
           |    :cy_mid  :   | :cy_box
           | [__V_] | [__V_] | :
           |A      A|        | :
           +:------:+--------+ V
            :      :
            .........cx_base
*/
typedef struct __BOXGUIDE
{
	int x_origin;
	int y_origin;
	int cx_box;
	int cy_box;
	int cx_base;
	int cy_base;
	int c_horzbox;
	int c_vertbox;
	int cy_mid;
} _BOXGUIDE, _PTR p_BOXGUIDE;

#endif /* _REC_MAIN_H */

/* ************************************************************************** */
/* *  END OF HEADER                                                         * */
/* ************************************************************************** */

