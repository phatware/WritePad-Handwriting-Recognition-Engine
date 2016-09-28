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

#ifndef __LISTATE_H
#define __LISTATE_H

typedef enum __E_LIG_STATE {
	LIG_STATE_UNDEF  = 0,
	LIG_STATE_OFTEN  = 1,
	LIG_STATE_RARELY = 2,
	LIG_STATE_NEVER  = 3
}E_LIG_STATE;


#define LIG_FIRST_LETTER 0x20
#define LIG_LAST_LETTER  0xFF
#define LIG_NUM_LETTERS  (LIG_LAST_LETTER - LIG_FIRST_LETTER + 1)
#if LIG_NUM_LETTERS <= 0
	#error
#endif
#define LIG_LET_NUM_GROUPS     8
#define LIG_NUM_BITS_PER_GROUP 2
#define LIG_NUM_BIT_GROUP_MASK 0x3

#define LIG_STATES_SIZE \
	(LIG_NUM_LETTERS * LIG_LET_NUM_GROUPS * LIG_NUM_BITS_PER_GROUP / 8)

typedef unsigned char LIGStatesType[LIG_STATES_SIZE];


/*
 * Sets state for a given letter and group.
 * Returns 0 if letter and group are in the allowed range, -1 otherwise.
 */
int  LIGSetGroupState(LIGStatesType *ioGStates,
                             int           inLetter,
					         int           inGroup,
                             E_LIG_STATE   inGroupState);

/*
 * Returns state for a given letter and group.
 */
E_LIG_STATE LIGGetGroupState(const LIGStatesType *inGStates,
                             int                 inLetter,
					         int                 inGroup);

#endif /* __LISTATE_H */
