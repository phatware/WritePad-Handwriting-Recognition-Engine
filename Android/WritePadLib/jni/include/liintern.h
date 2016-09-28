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

#ifndef __LIINTERN_H
#define __LIINTERN_H

typedef unsigned char	US8_t;
typedef unsigned short	US16_t;

#define	kLIHeaderSize			4	/* sizeof(long)						*/

#define	M_GetNumLetters(letptr) \
( \
	  ((long)(((US8_t*)(letptr))[0]) << 24) \
	+ ((long)(((US8_t*)(letptr))[1]) << 16) \
	+ ((long)(((US8_t*)(letptr))[2]) << 8)  \
	+ ((long)(((US8_t*)(letptr))[3]) << 0)  \
)

#define	M_SetNumLetters(letptr, nlet) \
( \
	(((US8_t*)(letptr))[0]) = (US8_t)((nlet) >> 24), \
	(((US8_t*)(letptr))[1]) = (US8_t)((nlet) >> 16), \
	(((US8_t*)(letptr))[2]) = (US8_t)((nlet) >> 8),  \
	(((US8_t*)(letptr))[3]) = (US8_t)((nlet) >> 0)   \
)

#define	kLILetterFieldSize		1	/* sizeof(let->Letter)				*/
#define	kLINumVarFieldSize		1	/* sizeof(let->numberOfVariants)	*/
#define	kLIOffsetFieldSize		2	/* sizeof(let->LetImageOffset)		*/

#define	kLILetterFieldOffset \
	0												/* offsetof(let->Letter)			*/

#define	kLINumVarFieldOffset \
	(kLILetterFieldOffset + kLILetterFieldSize)		/* offsetof(let->numberOfVariants)	*/

#define	kLIOffseFieldOffset	\
	(kLINumVarFieldOffset + kLINumVarFieldSize)		/* offsetof(let->LetImageOffset)	*/

#define	M_CalcLetHSize(nvar)		\
  ( (nvar) * kLIOffsetFieldSize  +	\
     kLINumVarFieldSize +			\
     kLILetterFieldSize				\
  )

#define	M_GetLetHLetField(letptr) \
	(((US8_t*)(letptr))[kLILetterFieldOffset])

#define	M_GetLetHNVarField(letptr) \
	(((US8_t*)(letptr))[kLINumVarFieldOffset])

#define	M_GetLetHOffsetField(letptr, var) \
( \
	  ((US16_t)((US8_t*)(letptr))[kLIOffseFieldOffset + (var) * kLIOffsetFieldSize + 0] << 8) \
	+ ((US16_t)((US8_t*)(letptr))[kLIOffseFieldOffset + (var) * kLIOffsetFieldSize + 1]) \
)

#define	M_SetLetHLetField(letptr, let) \
	(((US8_t*)(letptr))[kLILetterFieldOffset] = (US8_t)(let))

#define	M_SetLetHNVarField(letptr, nvar) \
	(((US8_t*)(letptr))[kLINumVarFieldOffset] = (US8_t)(nvar))

#define	M_SetLetHOffsetField(letptr, var, offset) \
( \
	((US8_t*)(letptr))[kLIOffseFieldOffset + (var) * kLIOffsetFieldSize + 0] \
		= (US8_t)((offset) >> 8), \
	((US8_t*)(letptr))[kLIOffseFieldOffset + (var) * kLIOffsetFieldSize + 1] \
		= (US8_t)(offset) \
)

#define	kLVGroupNumFieldSize		1		/* sizeof(letV->groupLetNumber) */
#define	kLVStrkNumFieldSize			1		/* sizeof(letV->numberOfSrokes) */
#define kLVStrokeOffsetFieldSize	2		/* sizeof(letH->strokeOffset)   */

#define	kLVGroupNumFieldOffset \
	0													/* offsetof(letV->groupLetNumber)	*/

#define	kLVNumStrkFieldOffset \
	(kLVGroupNumFieldOffset + kLVGroupNumFieldSize)		/* offsetof(letV->numberOfSrokes)	*/

#define	kLVStrkOffseFieldOffset	\
	(kLVNumStrkFieldOffset + kLVStrkNumFieldSize)		/* offsetof(letV->strokeOffset)		*/

#define	M_CalcLetVSize(nstrk)				\
  ( (nstrk) * kLVStrokeOffsetFieldSize  +	\
     kLVStrkNumFieldSize +					\
     kLVGroupNumFieldSize					\
  )

#define	M_GetLVGroupNumField(letvptr)	\
	 (((US8_t*)(letvptr))[kLVGroupNumFieldOffset])

#define	M_GetLVStrkNumField(letvptr) \
	(((US8_t*)(letvptr))[kLVNumStrkFieldOffset])

#define	M_GetLVStrokeOffsetField(letvptr, strkind) \
( \
	  ((US16_t)((US8_t*)(letvptr))[kLVStrkOffseFieldOffset + (strkind) * kLVStrokeOffsetFieldSize + 0] << 8) \
	+ ((US16_t)((US8_t*)(letvptr))[kLVStrkOffseFieldOffset + (strkind) * kLVStrokeOffsetFieldSize + 1]) \
)

#define	M_SetLVGroupNumField(letvptr, groupnum) \
	(((US8_t*)(letvptr))[kLVGroupNumFieldOffset] = (US8_t)(groupnum))

#define	M_SetLVStrkNumField(letvptr, nstrk) \
	(((US8_t*)(letvptr))[kLVNumStrkFieldOffset] = (US8_t)(nstrk))

#define	M_SetLVStrokeOffsetField(letvptr, strkind, offset) \
( \
	((US8_t*)(letvptr))[kLVStrkOffseFieldOffset + (strkind) * kLVStrokeOffsetFieldSize + 0] \
		= (US8_t)((offset) >> 8), \
	((US8_t*)(letvptr))[kLVStrkOffseFieldOffset + (strkind) * kLVStrokeOffsetFieldSize + 1] \
		= (US8_t)(offset) \
)

#define	kLSNumPointsFieldSize		1		/* sizeof(letStrk->numberOfPoints)	*/
#define	kLSPointFieldSize			2		/* sizeof(letStrk->spoints)			*/

#define	kLSNumPointsFieldOffset	\
	0														/* offsetof(letStrk->numberOfPoints)	*/

#define	kLSPointFieldOffset	\
	((kLSNumPointsFieldOffset + kLSNumPointsFieldSize))		/* offsetof(letStrk->spoints)			*/

#define	M_CalcLetSSize(npts)				\
  ( (npts) * kLSPointFieldSize  +			\
     kLSNumPointsFieldSize					\
  )

#define	M_GetLSNumPointsField(strkptr)	\
	 (((US8_t*)(strkptr))[kLSNumPointsFieldOffset])

#define	M_GetLSStrkPtXField(strkptr, ptindex) \
	(((US8_t*)(strkptr))[kLSPointFieldOffset + ptindex * kLSPointFieldSize])

#define	M_GetLSStrkPtYField(strkptr, ptindex) \
	(((US8_t*)(strkptr))[kLSPointFieldOffset + ptindex * kLSPointFieldSize + 1])

#define	M_GetLSStrkPtField(strkptr, ptindex, x, y) \
( \
	(x) = M_GetLSStrkPtXField(strkptr, ptindex), \
	(y) = M_GetLSStrkPtYField(strkptr, ptindex) \
)

#define	M_SetLSNumPointsField(strkptr, npts)	\
	 ((US8_t*)(strkptr))[kLSNumPointsFieldOffset] = (US8_t)(npts);

#define	M_SetLSStrkPtField(strkptr, ptindex, x, y) \
( \
	((US8_t*)(strkptr))[kLSPointFieldOffset + ptindex * kLSPointFieldSize] = (US8_t)(x), \
	((US8_t*)(strkptr))[kLSPointFieldOffset + ptindex * kLSPointFieldSize + 1] = (US8_t)(y) \
)

#endif /* __LIINTERN_H */
