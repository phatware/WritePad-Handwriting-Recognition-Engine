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

#include "snn.h"
#define USE_ASM   0

#if !MLP_PRELOAD_MODE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#endif

#include "mlp.h"

#if USE_ASM
#if defined( _WIN32_WCE ) && (defined( _SH3_ ) || defined ( _MIPS_ ))
#ifdef __cplusplus
extern "C" {
void __asm(const char *, ...);
}
#else
extern void __asm(const char *,...);
#endif
#endif
#endif  //USE_ASM

#if MLP_UNROLL_CYCLES && HWR_SYSTEM == HWR_EPOC32
// ARM WARNING!!! The following macro definitions rely on the fact that
// pointers 'signals' and 'weights' are aligned to 4 byte boundary each.

#define DECLARE_SIGNAL_ITERATION_VARS(insignals, inweights)   \
   register signed long  *ppi     = (signed long*)(insignals); \
   register signed long  *weights = (signed long*)(inweights); \
   register signed long  pp, ww

#define CELL_SIGNAL_ITERATION(i, v)                           \
   pp = ppi[(i)];                                              \
   ww = weights[(i)];                                          \
   (v) += (pp >> 16) * (ww >> 16);                             \
   (v) += ((pp << 16) >> 16) * ((ww << 16) >> 16)

#endif /* HWR_EPOC32 */

/* **************************************************************************** */
/* *         Calc cell output                                                 * */
/* **************************************************************************** */
fint_s CountCellSignal(_INT nc, p_mlp_data_type mlpd)
{
	_INT    i;
	flong   v;
	p_mlp_net_type net = (p_mlp_net_type) mlpd->net;
	p_mlp_cell_type cell = &(net->cells[nc]);
#ifndef DECLARE_SIGNAL_ITERATION_VARS
	fint_s  *ppi = &(mlpd->signals[cell->inp_ind]);
	fint_c  *weights = cell->weights;
#else
	DECLARE_SIGNAL_ITERATION_VARS(&(mlpd->signals[cell->inp_ind]), cell->weights);
#endif

	v = FINT_C_OF(cell->bias) x_UPCO_S;

#if MLP_UNROLL_CYCLES

#if MLP_CELL_MAXINPUTS != 32
#error "Can't unroll this net configuration!"
#endif

	v += FINT_S_OF(ppi[0])  * FINT_C_OF(weights[0]) + FINT_S_OF(ppi[1])  * FINT_C_OF(weights[1]) +
	     FINT_S_OF(ppi[2])  * FINT_C_OF(weights[2]) + FINT_S_OF(ppi[3])  * FINT_C_OF(weights[3]) +
	     FINT_S_OF(ppi[4])  * FINT_C_OF(weights[4]) + FINT_S_OF(ppi[5])  * FINT_C_OF(weights[5]) +
	     FINT_S_OF(ppi[6])  * FINT_C_OF(weights[6]) + FINT_S_OF(ppi[7])  * FINT_C_OF(weights[7]) +
	     FINT_S_OF(ppi[8])  * FINT_C_OF(weights[8]) + FINT_S_OF(ppi[9])  * FINT_C_OF(weights[9]) +
	     FINT_S_OF(ppi[10]) * FINT_C_OF(weights[10]) + FINT_S_OF(ppi[11]) * FINT_C_OF(weights[11]) +
	     FINT_S_OF(ppi[12]) * FINT_C_OF(weights[12]) + FINT_S_OF(ppi[13]) * FINT_C_OF(weights[13]) +
	     FINT_S_OF(ppi[14]) * FINT_C_OF(weights[14]) + FINT_S_OF(ppi[15]) * FINT_C_OF(weights[15]);

	v += FINT_S_OF(ppi[16]) * FINT_C_OF(weights[16]) + FINT_S_OF(ppi[17]) * FINT_C_OF(weights[17]) +
	     FINT_S_OF(ppi[18]) * FINT_C_OF(weights[18]) + FINT_S_OF(ppi[19]) * FINT_C_OF(weights[19]) +
	     FINT_S_OF(ppi[20]) * FINT_C_OF(weights[20]) + FINT_S_OF(ppi[21]) * FINT_C_OF(weights[21]) +
	     FINT_S_OF(ppi[22]) * FINT_C_OF(weights[22]) + FINT_S_OF(ppi[23]) * FINT_C_OF(weights[23]) +
	     FINT_S_OF(ppi[24]) * FINT_C_OF(weights[24]) + FINT_S_OF(ppi[25]) * FINT_C_OF(weights[25]) +
	     FINT_S_OF(ppi[26]) * FINT_C_OF(weights[26]) + FINT_S_OF(ppi[27]) * FINT_C_OF(weights[27]) +
	     FINT_S_OF(ppi[28]) * FINT_C_OF(weights[28]) + FINT_S_OF(ppi[29]) * FINT_C_OF(weights[29]) +
	     FINT_S_OF(ppi[30]) * FINT_C_OF(weights[30]) + FINT_S_OF(ppi[31]) * FINT_C_OF(weights[31]);

#else
	for (i = 0; i < MLP_CELL_MAXINPUTS; i += 4, weights += 4, ppi += 4)
	{
		v += FINT_S_OF(ppi[0]) * FINT_C_OF(weights[0]) +
		     FINT_S_OF(ppi[1]) * FINT_C_OF(weights[1]) +
		     FINT_S_OF(ppi[2]) * FINT_C_OF(weights[2]) +
		     FINT_S_OF(ppi[3]) * FINT_C_OF(weights[3]);
	}
#endif

	i = ((v x_DNCO_C) * (MLP_EXPTABL_SIZE / MLP_EXPTABL_MAX)) x_DNCO_S;

	if (i >= 0)
	{
		if (i >= MLP_EXPTABL_SIZE)
		{
			return net->exp_tabl[MLP_EXPTABL_SIZE - 1];
		}
		else
		{
			return net->exp_tabl[i];
		}
	}
	else
	{
		//    i = -i;
		if (i <= -MLP_EXPTABL_SIZE)
		{
			return (fint_s) (MLP_MAX_INT_S - net->exp_tabl[MLP_EXPTABL_SIZE - 1]);
		}
		else
		{
			return (fint_s) (MLP_MAX_INT_S - net->exp_tabl[-i]);
		}
	}
}
