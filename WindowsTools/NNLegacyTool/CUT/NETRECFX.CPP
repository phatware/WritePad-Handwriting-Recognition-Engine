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

/* ************************************************************************************* */
/* *    PhatWare WritePad handwriting recognition engine configurator                  * */
/* *    Copyright (c) 1997-2014 PhatWare(r) Corp. All rights reserved.                 * */
/* ************************************************************************************* */

/* ************************************************************************************* *
*
* File: netrecfx.cpp
*
* Unauthorized distribution of this code is prohibited.
* Contractor/manufacturer is PhatWare Corp.
* 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
*
* ************************************************************************************* */

#include "hwr_sys.h"
#include "netrecfx.h"

ROM_DATA_EXTERNAL _INT rom_ncells [];
ROM_DATA_EXTERNAL Fixed32 rom_matrix [];
ROM_DATA_EXTERNAL RCELL rom_cell [];
ROM_DATA_EXTERNAL _SHORT EXP_TABL[EXP_TABL_SIZE];

_INT Rget_answer(_INT *cur_coeff, Fixed32 *char_answer)
{
	int i, j, nchar, nc, ncells;
	int cur_radius, *pcoor;
	int size_matrix = rom_NUM_COEFF*rom_NUM_COEFF;
	Fixed32 *cur_matrix, *pmatr_elem;
	Fixed32 cell_answer, cur_dist, power;
	Fixed32 cur_vect_coor;
	Fixed32 cur_mcoeff[rom_NUM_COEFF];
	short *cur_mcenter;
	p_RCELL cur_cell;
	Fixed32 Fradius, thresh;
	Fixed32 wrk_char_answer;


	cur_matrix = (Fixed32 *) rom_matrix;
	cur_cell = (p_RCELL) rom_cell;
	for (nchar = 0; nchar < rom_NUM_CHAR; nchar++)
	{
		wrk_char_answer = ONE_FIX;

		pmatr_elem = cur_matrix;
		for (i = 0; i < rom_NUM_COEFF; i++)
		{
			cur_mcoeff[i] = 0;
			pcoor = cur_coeff;
			for (j = 0; j < rom_NUM_COEFF; j++)
			{
				cur_mcoeff[i] += ((*pmatr_elem)*(*pcoor));
				pmatr_elem++;
				pcoor++;
			}
		}

		ncells = rom_ncells[nchar];
		for (nc = 0; nc<ncells; nc++)
		{
			cur_mcenter = cur_cell->mcenter;
			cur_radius = cur_cell->radius;
			Fradius = LongToFix32(cur_radius);
			thresh = Fradius + Fradius;
			thresh += thresh;
			thresh += Fradius;
			cur_dist = 0;

			cur_vect_coor = cur_mcoeff[2] - LongToFix32((long) cur_mcenter[2]);
			cur_dist += FixMul32(cur_vect_coor, cur_vect_coor);
			if (cur_dist>thresh)
			{
				goto END_CELL;
			}

			cur_vect_coor = cur_mcoeff[8] - LongToFix32((long) cur_mcenter[8]);
			cur_dist += FixMul32(cur_vect_coor, cur_vect_coor);
			if (cur_dist > thresh)
			{
				goto END_CELL;
			}

			cur_vect_coor = cur_mcoeff[9] - LongToFix32((long) cur_mcenter[9]);
			cur_dist += FixMul32(cur_vect_coor, cur_vect_coor);
			if (cur_dist > thresh)
			{
				goto END_CELL;
			}

			cur_vect_coor = cur_mcoeff[10] - LongToFix32((long) cur_mcenter[10]);
			cur_dist += FixMul32(cur_vect_coor, cur_vect_coor);
			if (cur_dist > thresh)
			{
				goto END_CELL;
			}

			cur_vect_coor = cur_mcoeff[4] - LongToFix32((long) cur_mcenter[4]);
			cur_dist += FixMul32(cur_vect_coor, cur_vect_coor);
			if (cur_dist > thresh)
			{
				goto END_CELL;
			}

			cur_vect_coor = cur_mcoeff[6] - LongToFix32((long) cur_mcenter[6]);
			cur_dist += FixMul32(cur_vect_coor, cur_vect_coor);
			if (cur_dist > thresh)
			{
				goto END_CELL;
			}

			cur_vect_coor = cur_mcoeff[0] - LongToFix32((long) cur_mcenter[0]);
			cur_dist += FixMul32(cur_vect_coor, cur_vect_coor);
			if (cur_dist > thresh)
			{
				goto END_CELL;
			}

			cur_vect_coor = cur_mcoeff[1] - LongToFix32((long) cur_mcenter[1]);
			cur_dist += FixMul32(cur_vect_coor, cur_vect_coor);
			if (cur_dist > thresh)
			{
				goto END_CELL;
			}

			cur_vect_coor = cur_mcoeff[5] - LongToFix32((long) cur_mcenter[5]);
			cur_dist += FixMul32(cur_vect_coor, cur_vect_coor);
			if (cur_dist > thresh)
			{
				goto END_CELL;
			}

			cur_vect_coor = cur_mcoeff[7] - LongToFix32((long) cur_mcenter[7]);
			cur_dist += FixMul32(cur_vect_coor, cur_vect_coor);
			if (cur_dist > thresh)
			{
				goto END_CELL;
			}

			cur_vect_coor = cur_mcoeff[3] - LongToFix32((long) cur_mcenter[3]);
			cur_dist += FixMul32(cur_vect_coor, cur_vect_coor);

			if (cur_dist <= thresh)
			{
				power = (cur_radius > 0) ? -cur_dist / cur_radius : MINUSINF;
				cell_answer = EXP(power);
				wrk_char_answer = FixMul32(wrk_char_answer, ONE_FIX - cell_answer);
			}
END_CELL:
			cur_cell++;
		}
		char_answer[nchar] = ONE_FIX - wrk_char_answer;

		cur_matrix += size_matrix;
	}
	return(1);
}

Fixed32 EXP(Fixed32 x)
{
	_LONG i;
	Fixed32 q, r;
	Fixed32 retval;

	if (x <= LongToFix32(MIN_POWER))
	{
		return(0);
	}
	if (x >= 0)
	{
		return(ONE_FIX);
	}
	q = -x*SCALE;
	i = Fix32ToLong(q);
	r = q - LongToFix32(i);
	retval = FixMul32(ONE_FIX - r, (Fixed32) EXP_TABL[i]) +
	         FixMul32(r, (Fixed32) EXP_TABL[i + 1]);
	return(retval);
}


