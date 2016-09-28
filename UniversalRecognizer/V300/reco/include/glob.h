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

#ifndef __include_glob_h__
#define __include_glob_h__


#include  "hwr_sys.h"
#include  "const.h"
#include  "def.h"

/************************** Global variables **************************/

RECO_DATA _SHORT  sqrtab[LENTH_S] =
{
    0,1,1,1,2,2,2,2,2,3,3,3,3,3,3,3,
    4,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,
    6,6,6,6,6,6,6,6,6,6,6,6,6,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,
    11,11,11,11,11,11,11
} ;

RECO_DATA _ULONG bit[32] =
{
    0x1,0x2,0x4,0x8,0x10,0x20,0x40,0x80,0x100,0x200,
    0x400,0x800,0x1000,0x2000,0x4000,0x8000,0x10000L,
    0x20000L,0x40000L,0x80000L,0x100000L,0x200000L,0x400000L,
    0x800000L,0x1000000L,0x2000000L,0x4000000L,0x8000000L,
    0x10000000L,0x20000000L,0x40000000L,0x80000000L
};

RECO_DATA _SHORT  quad[LENTH_Q]=
{
    0,    1,    4,    9,   16,   25,   36,   49,
    64,   81,  100,  121,  144,  169,  196,  225,
    256,  289,  324,  361,  400,  441,  484,  529,
    576,  625,  676,  729,  784,  841,  900,  961,
    1024, 1089, 1156, 1225, 1296, 1369, 1444, 1521,
    1600, 1681, 1764, 1849, 1936, 2025, 2116, 2209,
    2304, 2401, 2500, 2601, 2704, 2809, 2916, 3025,
    3136, 3249, 3364, 3481, 3600, 3721, 3844, 3969
} ;

RECO_DATA _SHORT  eps0[LENTH_E]=
{
    0,    0,    0,    0,    0,    0,    0,    0,
    0,    1,    3,    5,    8,   11,   15,   20,
    25,   30,   36,   42,   48,   55,   62,   69,
    77,   85,   92,  101,  109,  117,  126,  135,
    135,  135,  135,  135,  135,  135,  135,  135,
    135,  135,  135,  135,  135,  135,  135,  135,
    135,  135,  135,  135,  135,  135,  135,  135,
    135,  135,  135,  135,  135,  135,  135,  135
} ;

RECO_DATA _SHORT  eps1[LENTH_E]=
{
    0,    0,    0,    0,    0,    0,    9,   30,
    54,   78,  100,  118,  134,  146,  157,  165,
    172,  177,  181,  184,  187,  189,  190,  192,
    193,  193,  194,  194,  195,  195,  195,  195,
    195,  195,  195,  195,  195,  195,  195,  195,
    195,  195,  195,  195,  195,  195,  195,  195,
    195,  195,  195,  195,  195,  195,  195,  195,
    195,  195,  195,  195,  195,  195,  195,  195
} ;

RECO_DATA _SHORT  eps2[LENTH_E] =
{
    0,    1,    9,   26,   44,   59,   71,   80,
    87,   92,   95,   98,   98,   98,   98,   98,
    98,   98,   98,   98,   98,   98,   98,   98,
    98,   98,   98,   98,   98,   98,   98,   98,
    98,   98,   98,   98,   98,   98,   98,   98,
    98,   98,   98,   98,   98,   98,   98,   98,
    98,   98,   98,   98,   98,   98,   98,   98,
    98,   98,   98,   98,   98,   98,   98,   98
} ;

RECO_DATA _SHORT  eps3[LENTH_E]=
{
    9,   27,   46,   61,   74,   84,   91,   96,
    96,   96,   96,   96,   96,   96,   96,   96,
    96,   96,   96,   96,   96,   96,   96,   96,
    96,   96,   96,   96,   96,   96,   96,   96,
    96,   96,   96,   96,   96,   96,   96,   96,
    96,   96,   96,   96,   96,   96,   96,   96,
    96,   96,   96,   96,   96,   96,   96,   96,
    96,   96,   96,   96,   96,   96,   96,   96
} ;

RECO_DATA _SHORT  nbcut0 = 10;      /* Utmost radius of intersection   */
/* search                          */
RECO_DATA _SHORT  nbcut1 = 95;       /* A square of utmost radius of    */
/* intersection search             */
RECO_DATA _SHORT  nbcut2 = 2;

/*******************  Low level constants *****************************/

RECO_DATA CONSTS   const1 =
{
    (_SHORT)HORDA  ,         /* Normalization chord               */
    (_SHORT)DLT0   ,         /*                                   */
    (_SHORT)NSR    ,         /*                                   */
    (_SHORT)J_STEP ,         /* Initial steps of search  for      */
    (_SHORT)I_STEP ,         /* intersectons                      */
    (_SHORT)EPS_Y ,          /* Vicinity of pointing out extremums*/
    (_SHORT)EPS_X,           /* and preliminary search for        */
    /* shelves                           */
    (_SHORT)EPS_PY ,         /* Maximum horizontal and vertical   */
    (_SHORT)EPS_PX ,         /* size of "points" .                */
    (_SHORT)EPS_ST ,         /* Maximum length of a stroke.       */
    
    /* (_SHORT)EPSA_ST , */        /* Maximum slope of a stroke.        */
    /* (_SHORT)EPSR_ST , */        /* Stroke maximum integral curvature */
    /* (_SHORT)EPSX_SH , */        /* Minimum horizontal shelves        */
    /*                   */        /* size .                            */
    /* (_SHORT)EPSY_SH , */        /* Half maximum vertical shelf       */
    /*                   */        /* size                              */
    /* (_SHORT)EPSL_SH , */        /* Maximum height of shelves gluing  */
    /* (_SHORT)EPS_SM ,  */        /* Vicinity  of ending extremums     */
    /* marks                             */
    (_SHORT)A0 ,             /*                                   */
    (_SHORT)B0 ,             /*                                   */
    (_SHORT)LF0 ,            /*                                   */
    (_SHORT)LZ0 ,            /*                                   */
    (_SHORT)A1 ,             /*                                   */
    (_SHORT)B1 ,             /*                                   */
    (_SHORT)LF1 ,            /*                                   */
    (_SHORT)LZ1 ,            /*                                   */
    (_SHORT)A2 ,             /*                                   */
    (_SHORT)B2 ,             /*                                   */
    (_SHORT)LF2 ,            /*                                   */
    (_SHORT)LZ2 ,            /*                                   */
    (_SHORT)A3 ,             /*                                   */
    (_SHORT)B3 ,             /*                                   */
    (_SHORT)LF3 ,            /*                                   */
    (_SHORT)LZ3              /*                                   */
    /* (_SHORT)EPS_F  */
} ;

/***************************************************************************/

#endif // __include_glob_h__
