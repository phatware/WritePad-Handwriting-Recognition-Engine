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

#include "ams_mg.h"
#include "dti.h"

#if DTI_LRN_SUPPORTFUNC

#include "ligstate.h"

#define LIG_NUMBITS             (sizeof((*((LIGStatesType*)0))[0]) * 8)

#define LIG_BITINDEX(let, gr)   ((((let) - LIG_FIRST_LETTER) * LIG_LET_NUM_GROUPS + (gr)) * LIG_NUM_BITS_PER_GROUP)

#define LIG_BITSSHIFT(bitindex) (LIG_NUMBITS - LIG_NUM_BITS_PER_GROUP - bitindex % LIG_NUMBITS)

/*
 * Sets state for a given letter and group.
 * Returns 0 if letter and group are in the allowed range, -1 otherwise.
 */
int LIGSetGroupState(
	LIGStatesType *ioGStates,
	int           inLetter,
	int           inGroup,
	E_LIG_STATE   inGroupState)
{
	int i;
	int shift;

	if (inLetter < LIG_FIRST_LETTER ||
	    inLetter > LIG_LAST_LETTER  ||
		inGroup < 0 ||
		inGroup > LIG_LET_NUM_GROUPS)
    {
		return -1;
	}

	i = LIG_BITINDEX(inLetter, inGroup);
	shift = LIG_BITSSHIFT(i);
	i /= LIG_NUMBITS;
	(*ioGStates)[i] &= ~(LIG_NUM_BIT_GROUP_MASK << shift);
	(*ioGStates)[i] |= (inGroupState << shift);

	return 0;
}

/*
 * Returns state for given letter and group.
 */
E_LIG_STATE LIGGetGroupState(
	const LIGStatesType *inGStates,
	int                 inLetter,
	int                 inGroup)
{
	int i;
	int shift;

	if (inLetter < LIG_FIRST_LETTER ||
	    inLetter > LIG_LAST_LETTER  ||
		inGroup < 0 ||
		inGroup > LIG_LET_NUM_GROUPS)
    {
		return LIG_STATE_UNDEF;
	}

    i = LIG_BITINDEX(inLetter, inGroup);
	shift = LIG_BITSSHIFT(i);
	i /= LIG_NUMBITS;
	return ((E_LIG_STATE)(((*inGStates)[i] >> shift) & LIG_NUM_BIT_GROUP_MASK));
}

#endif // DTI_LRN_SUPPORTFUNC
