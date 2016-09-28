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

#ifndef LK_CODE_INCLUDED
#define LK_CODE_INCLUDED

#include "ams_mg.h"
#include "xr_names.h"
#include "accents.h"

/***********************************************************************/
/*                    IMPORTANT CONSTANTS                              */
/***********************************************************************/

#define ICOS        -79L          /* % limit of cros. angle for stick  */
#define OCOS         -1L          /* 65                  for circle    */
#define GCOS         70L          /* angle of  gammas` curve  opening  */
#ifdef FORMULA
#define DUGCOS       35L
#define O_LITTLE     50        /* % coeff. for defin. of small circles */
#else
#define DUGCOS       50L
#define O_LITTLE     50        /* % coeff. for defin. of small circles */
#endif /*FORMULA*/
#define Z_LITTLE     25        /* % coeff. for defin. of small breaks  */
#define LEN_CROSS    75        /* % coeff. of min crossing`s length    */

#define  DHEIGHT_BIG_DARCS  6  /* Height difference of the ends of the */
                               /* vertical arc for it to be consi-     */
                               /* dered as big one (usually bracket).  */
#define  DHEIGHT_SMALL_DARCS 3   /* Height difference of the ends of the */
                               /* vertical arc for it to be consi-     */
                               /* dered as small one (usually arc at   */
                               /* "p"                                  */

/***********************************************************************/
/*                CODES OF MAIN CONSTANTS                              */
/***********************************************************************/
#define  _NO_CODE  0x00                   /* no code                   */
#define  _ZZ_      0x01                   /* break                     */
#define  _UU_      0x02                   /* arc. up                   */
#define  _IU_      0x03                   /* I up   - max black        */
#define  _GU_      0x04                   /* gamma up                  */
#define  _O_       0x05                   /* O element                 */
#define  _GD_      0x06                   /* gamma down                */
#define  _ID_      0x07                   /* I-down    - min black     */
#define  _UD_      0x08                   /* arc down                  */
#define  _UUL_     0x09                   /*arc up with min to the left*/
#define  _UUR_     0x0a                  /*arc up with min to the right*/
#define  _UDL_     0x0b                 /*arc down with min to the left*/
#define  _UDR_     0x0c                /*arc down with min to the right*/
#define  _XT_      0x0d                   /* stroke                    */
#define  _ANl      0x0e                   /* direction changing        */
#define  _DF_      0x0f                   /* shelf                     */
#define  _ST_      0x10                   /* point                     */
#define  _ANr      0x11                   /* direction changing        */
#define _ZZZ_      0x12                   /* break                     */
#define _Z_        0x13                   /* next elem - to the left   */
#define _FF_       0x14                   /* large break               */
#define _DUR_      0x15   /* Double move upper with right end          */
#define _CUR_      0x16   /* Circle upper with right end               */
#define _CUL_      0x17   /* Circle upper with left end                */
#define _DUL_      0x18   /* Double move down with left end            */
#define _DDR_      0x19   /* Double move down  with right end          */
#define _CDR_      0x1a   /* Circle down  with right end               */
#define _CDL_      0x1b   /* Circle down  with left end                */
#define _DDL_      0x1c   /* Double move down with left end            */
#define _GUs_      0x1d   /* Small gamma up                            */
#define _GDs_      0x1e   /* Small gamma down                          */
#define _Gl_       0x1f   /* Gamma left                                */
#define _Gr_       0x20   /* Gamma right                               */
#define _UUC_      0x21   /* circle arc up                             */
#define _UDC_      0x22   /* circle arc down                           */

  /*CHE*/
#define _TS_       0x23   /* S-like arc                                */
#define _TZ_       0x24   /* Z-like arc                                */
#define _BR_       0x25   /* Right D-arc                               */
#define _BL_       0x26   /* Left D-arc                                */

#define _BSS_      0x27   /* Special SHELF                             */

#define _AN_UR     0x28   /* Angle-like upper-right corner.            */
#define _AN_UL     0x29   /* Angle-like upper-left corner.             */

#ifdef  FORMULA

#error  Recode these elems for FORMULA project!!!!
#define  _ERU_     0x14                   /* End right-up              */
#define  _ELU_     0x15                   /* End left-up               */
#define  _ERD_     0x16                   /* End right-down            */
#define  _ELD_     0x17                   /* End left-down             */

#define  _SRU_     0x18                   /* Start from right-up       */
#define  _SLU_     0x19                   /* Start from left-up        */
#define  _SRD_     0x20                   /* Start from right-down     */
#define  _SLD_     0x21                   /* Start from left-down      */

#endif /*FORMULA*/

/***********************************************************************/
/*       POSITION OF ELEMENTS AND THEIR SIGNS                          */
/***********************************************************************/

#define _f_    ((_UCHAR)0x10)     /* arc moves clockwise               */
#define _b_    ((_UCHAR)0x20)     /* arc moves counter clockwise       */


#define _fUS1_  (_f_|_US1_)           /* h    clockwise                    */
#define _fUS2_  (_f_|_US2_)
#define _fUE1_  (_f_|_UE1_)           /* e    clockwise                    */
#define _fUE2_  (_f_|_UE2_)
#define _fUI1_  (_f_|_UI1_)           /* i    clockwise                    */
#define _fUI2_  (_f_|_UI2_)
#define _fMD_   (_f_|_MD_)            /* g    clockwise                    */
#define _fDI1_  (_f_|_DI1_)
#define _fDI2_  (_f_|_DI2_)           /* h    clockwise                    */
#define _fDE1_  (_f_|_DE1_)
#define _fDE2_  (_f_|_DE2_)           /* t    clockwise                    */
#define _fDS1_  (_f_|_DS1_)
#define _fDS2_  (_f_|_DS2_)           /*                                   */

#define _bUS1_  (_b_|_US1_)           /* h    counter clockwise            */
#define _bUS2_  (_b_|_US2_)
#define _bUE1_  (_b_|_UE1_)           /* e    counter clockwise            */
#define _bUE2_  (_b_|_UE2_)
#define _bUI1_  (_b_|_UI1_)           /* i    counter clockwise            */
#define _bUI2_  (_b_|_UI2_)
#define _bMD_   (_b_|_MD_)            /* g    counter clockwise            */
#define _bDI1_  (_b_|_DI1_)
#define _bDI2_  (_b_|_DI2_)           /* h    counter clockwise            */
#define _bDE1_  (_b_|_DE1_)
#define _bDE2_  (_b_|_DE2_)           /* t    counter clockwise            */
#define _bDS1_  (_b_|_DS1_)
#define _bDS2_  (_b_|_DS2_)           /*      counter clockwise            */

#define _OVR_      ((_UCHAR)0x40)     /* veto on wordcutting after element */
#define _fb_       ((_UCHAR)0x30)   /* code for marking out round direction*/

/***********************************************************************/
/*         Code for storing in byte specl->other                       */
/***********************************************************************/

/* directions of angles and information concerning use of them -
           files ANGLE.CPP, LK_BEGIN.CPP, CROSS_G.CPP, LK_NEXT.CPP */

#define _ANGLE_UP_           0x10 /* angle up                          */
#define _ANGLE_DOWN_         0x20 /* angle down                        */
#define _ANGLE_LEFT_         0x40 /* angle to the left                 */
#define _ANGLE_RIGHT_        0x80 /* angle to the right                */

#define _ANGLE_DIRECT_       0xf0 /* code for marking angle direction  */

#define NOT_RESTORED         0x01 /* for not restored angles   */
#define WAS_DELETED_BY_CROSS 0x02 /* deleted by CROSS at first */
#define WAS_DELETED_BY_STICK 0x04 /* deleted by STICK at first */
#define INNER_ANGLE          0x08 /* angle inside crossing like B */

/* mark field other for elements inside and outside the circle -
 module CROSS_G.C, LK_NEXT.C */
#define INSIDE_CIRCLE         0x40
#define OUTSIDE_CIRCLE        0x80

/* Mark SHELF as belongs to small 'h' - module PICT.C */
#define LETTER_H_STEP        0x01
#define ZERO_PENALTY         0x02
#define REGULAR_PENALTY      0x04
#define LARGE_PENALTY        0x08

/* mark field other for CROSS elements - module CIRCLE.CPP */
#define CIRCLE_FIRST          0x63
#define CIRCLE_NEXT           0x64

/* Mark O as converted from GU - module CROSS_G.CPP, LK_NEXT.CPP */
#define WAS_CONVERTED_FROM_GU    0x01
/* Mark O as too narrow - module CROSS_G.CPP, LK_NEXT.CPP */
#define TOO_NARROW               0x02
/* Mark Gsmall as converted from Gamma - module CROSS_G.CPP, LK_NEXT.CPP */
#define WAS_CONVERTED_FROM_GAMMA 0x04
/* mark field other for Gamma-elements,
 which appears in the narrow places - module CROSS_G.CPP, LK_NEXT.CPP */
#define NO_ARC                   0x08

/* Mark IU or ID, if they were STICK or CROSS - module CROSS_G.CPP, LK_NEXT.CPP */
#define WAS_STICK_OR_CROSS    0x01
/* Mark IU or ID, if they were in STICK, but it was too little
 and was thrown away - module CROSS_G.CPP, LK_NEXT.CPP */
#define WAS_INSIDE_STICK      0x02
/* Mark IU, ID or any arc with tail as "non-deleted" - module LK_NEXT.CPP */
#define DONT_DELETE           0x04
/* Mark IU, ID as near artificial break - module LK_BEGIN.CPP, BREAKS.CPP */
#define MIN_MAX_CUTTED        0x08
/* Mark IU, ID as glued with another one - module LK_NEXT.CPP */
#define WAS_GLUED             0x10
/* Mark IU or ID, if there are in redirected stick - module BREAKS.CPP */
#define WAS_REDIRECTED        0x20

/* Mark STROKEs and DOTs as:
   - processed or not-processed - module BREAKS.CPP
   STROKE as:
   - cutted - modules PICT.CPP,BREAKS.CPP,CONVERT.CPP
   - with crossing - module BREAKS.CPP,CONVERT.CPP
   - fake - module BREAKS.C
   - reanimated - modules SKETCH.CPP,BREAKS.CPP,CONVERT.CPP (LANG_FRENCH)
   DOT as:
   - artificial made to convert it to UMLAUT - module BREAKS.CPP (LANG_GERMAN and LANG_FRENCH)
   - apostrof - module el_aps.CPP
   - clitics - modules BREAKS.CPP,CONVERT.CPP
   - cedilla - modules SKETCH.CPP,BREAKS.CPP,CONVERT.CPP (LANG_FRENCH)
   - cedilla in the end - modules SKETCH.CPP,BREAKS.CPP,CONVERT.CPP (LANG_FRENCH)
   - artificial made to convert it to ANGSTREM - module BREAKS.CPP (LANG_SWED)
*/
#define NOT_PROCESSED         0x00
#define PROCESSED             0x01

#define CUTTED                0x02
#define WITH_CROSSING         0x04
#define FAKE                  0x08
#define REANIMATED            0x10
#define RIGHT_KREST           0x20

#define ST_UMLAUT             0x02

#define ST_APOSTR             0x04
#define ST_QUOTE              0x08

#define ST_CEDILLA            0x10
#define CEDILLA_END           0x20

#define ST_ANGSTREM           0x40

/* Mark Breaks as:
 - FF_PUNCT to treat them as punctuation separators - module BREAKS.CPP
 - NO_PENALTY to treat them as apostrof separators - modules EL_APS.CPP,BREAKS.CPP
 - SPECIAL_ZZ to distinguish them from the original (with crossing) ZZ - module BREAKS.CPP, CONVERT.CPP
 - FF_CUTTED (artificially made) - module LK_BEGIN.CPP, BREAKS.CPP, CONVERT.CPP
 - Z_UP_DOWN (2-nd stroke higher 1-st) to distinguish them from the original (with crossing) Z - module BREAKS.CPP, CONVERT.CPP
 */
#define FF_PUNCT             0x01
#define NO_PENALTY           0x02
#define SPECIAL_ZZ           0x04
#define FF_CUTTED            0x08
#define Z_UP_DOWN            0x10

/***********************************************************************/
/* Function's prototypes                                               */
/***********************************************************************/

_VOID Restore_AN(p_low_type low_data,p_SPECL pCross, _UCHAR RestoreMask,_SHORT NumCr);

#endif  /*  LK_CODE_INCLUDED  */
