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

#define           EPS0_SH            24         /*horisontal shelf     */
/*      radius         */
#define           DLT0               4          /* current depth  of   */
/* crossings search    */
#define           EPS_PY             35         /* max vertical and    */
/* horisontal sizes of */
#define           EPS_PX             30         /* 'points'            */
/*                     */
#define           EPS_ST             35         /* max stroke length   */
/*                     */
#define           EPSA_ST            6          /* max stroke          */
/*  inclination        */
#define           EPSR_ST            12         /* max integral        */
/* stroke curvature    */
#define           EPS_Y              8          /* environs marking    */
/*out of extremums     */
#define           EPS_X              10         /* preliminary marking */
/* out of shelves      */
#define           EPS_SM             3          /*environs of marking  */
/*out of extremums on ends*/
#define           EPSX_SH            40         /* min horizontal size */
/*       of shelves    */
#define           EPSY_SH            3          /* half max vertical   */
/* shelves` size       */
#define           EPSL_SH            25         /* max height of       */
/* shelves` pasting    */
#define           HORDA              10         /* normalization chord */
/*                     */
#define           EPS_F              32767      /* utmost distance     */
/* between points      */
/* common table        */
#define           A0                 12         /* max utmost value    */
/* of spatial environs */
/*                     */
#define           B0                 2       /* coefficient of exponent*/
/* growth              */
#define           LF0                32      /* ending point of limit`s*/
/* environs growth     */
#define           LZ0                8          /* restricted area of  */
/* crossings           */
#define           A1                 14         /* table for open      */
/* crossings           */
#define           B1                 25         /*                     */
/*                     */
#define           LF1                32         /*                     */
/*                     */
#define           LZ1                6          /*                     */
/*                     */
#define           A2                 10         /*  table for breaks   */
/*                     */
#define           B2                 35         /*                     */
/*                     */
#define           LF2                12         /*                     */
/*                     */
#define           LZ2                2          /*                     */
/*  tables for ends    */
#define           A3                 10         /*                     */
/*                     */
#define           B3                 35         /*                     */
/*                     */
#define           LF3                8          /*                     */
/*                     */
#define           LZ3                0          /*                     */
/*                     */
#define           NSR                260        /* max possible number */
/* of points in letter */
#define           J_STEP             1          /*                     */
/*                     */
#define           I_STEP             1          /*                     */
/*                     */
