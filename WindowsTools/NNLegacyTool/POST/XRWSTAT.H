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

#ifndef XRWSTAT_H

#define XRWSTAT_H

/*  The switch-on define.  */
#define XRW_STAT_ON          0

#if XRW_STAT_ON   //CHE
#include <stdio.h>
#endif  /*  XRW_STAT_ON  */

/*CHE: for "shifts" statistics rather than "xrs" should be "1": */
#define XRWSTAT_SHIFTS  1

/*  The correction type defines.  */
#define XRWSTAT_ZEROBADS     1
#define XRWSTAT_MAXGOODS     2
#define XRWSTAT_DECREASE     4
#define XRWSTAT_INCREASE     8
#define XRWSTAT_READVALUES  16
#define XRWSTAT_REDUNDANT    0x8000
#define XRWSTAT_CORR_ERROR   0x4000

/*  Define all the types needed for the statistics gathering.  */

typedef struct _XRW_STAT   /*  Header of all statistics info.    */
{
	p_UCHAR pDti;           /*  dti pointer.                      */
	_ULONG  totalCorrections;
	_ULONG  redundantCorrections;
	_ULONG  symOffset[256]; /*  symbol offsets within the block.  */
} XRW_STAT, _PTR p_XRW_STAT;

typedef struct _XRW_STAT_SYM   /*  Symbol header in the statistics area.  */
{
	_USHORT chCorrect;
	_USHORT chIncorrect;
	_USHORT num_vars;                        /* Number of variants of sym   */
	_USHORT chCorrections;
	_USHORT varCorrect[DTI_MAXVARSPERLET];
	_USHORT varIncorrect[DTI_MAXVARSPERLET];
	_USHORT varCorrections[DTI_MAXVARSPERLET];
	_UCHAR  var_lens[DTI_MAXVARSPERLET];     /* Array of var lens           */
} XRW_STAT_SYM, _PTR p_XRW_STAT_SYM;

typedef struct _XRW_STAT_XR   /* Statistics for the individual xr-element. */
{
	_USHORT good[XR_COUNT];
	_USHORT bad[XR_COUNT];
	_USHORT correctionType[XR_COUNT];
} XRW_STAT_XR, _PTR p_XRW_STAT_XR;

typedef struct _XRW_STAT_CORR_ARGS   /*  Arguments of the correction.  */
{
	_INT ch;
	_INT var;
	_INT xrNum;
	_INT otherXrNum;
	_INT chGoodCount;
	_INT chBadCount;
	_INT varGoodCount;
	_INT varBadCount;
	_INT goodCount;
	_INT badCount;
	_INT value;
	_INT whom;   //ayv
	_INT fun;    //ayv
} XRW_STAT_CORR_ARGS, _PTR p_XRW_STAT_CORR_ARGS;


/*  Declare the global statistics pointer.  */

#if XRW_STAT_ON
#ifndef XRW_STAT_C
extern
#endif  /*  XRW_STAT_C  */
p_XRW_STAT gpXrwStat
#ifdef XRW_STAT_C
    = _NULL
#endif  /*  XRW_STAT_C  */
      ;
#endif  /*  XRW_STAT_ON  */


/*  All staistics-related functions prototypes/defines.  */

#if XRW_STAT_ON

#include "postcalc.h"

/*  External calls prototypes.  */
_VOID  XrwStatAllocData(p_UCHAR p_dti);
_VOID  XrwStatDumpData(_VOID);
_VOID  XrwStatGatherWordData(P_POST_PARAMS pParams);

/*  Internal calls prototypes.  */
p_XRW_STAT_SYM XrwStatGetSymAddress(_INT ch);
p_XRW_STAT_XR XrwStatGetVarAddress(_INT ch, _INT var);
_BOOL   XrwStatMarkGoodCorrs(P_POST_PARAMS pParams);
_BOOL   XrwStatMarkBadCorrs(P_POST_PARAMS pParams);
_VOID   XrwStatFixDti(p_CHAR fileName, _INT mode);
_USHORT XrwStatSetCorrValue(p_XRW_STAT_CORR_ARGS pArgs, _INT mode);
_INT    XrwStatGetVarOfChar(_INT chIn, _INT nv, p_xrp_type _PTR pXvb);
_VOID   XrwStatReadStatistics(_VOID);
_BOOL   XrwStatSkipDelimiters(p_CHAR _PTR ppString);
_VOID   XrwStatSkipNumbers(p_CHAR _PTR ppString);
_VOID   XrwStatDumpCorrections(_VOID);
_VOID   XrwStatCorrectOnTheFly(p_XRW_STAT_CORR_ARGS pArgs,
                               p_XRW_STAT_SYM symStat, p_XRW_STAT_XR varXr,
                               P_POST_PARAMS pParams, _INT mode);
_VOID   XrwStatDumpIncorrectData(FILE *out, RWG_PPD_type _PTR NewPPDData,
                                 p_RWS_type pRWS, _INT size, _INT ppdIndex,
                                 RWG_PPD_type _PTR pCorrectPPD,
                                 p_RWS_type pCorrectRWS, _INT correctSize);
_VOID   XrwStatDumpCorrectTrace(P_POST_PARAMS pParams);
_VOID   XrwStatDumpIncorrectTrace(P_POST_PARAMS pParams);
_INT    XrwStatFindCorrectWeight(P_POST_PARAMS pParams, p_UCHAR word);
_VOID   XrwStatReadValues(FILE *in);

#else

/*  Kill external calls in case there's no need for statistics.  */
#define XrwStatAllocData(a)
#define XrwStatDumpData()
#define XrwStatGatherWordData(a)

#endif  /*  XRW_STAT_ON  */

#endif  /*  XRWSTAT_H  */
