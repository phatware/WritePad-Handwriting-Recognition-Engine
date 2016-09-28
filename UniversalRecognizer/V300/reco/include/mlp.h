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

#ifndef MLP_HEADER_INCLUDED
#define MLP_HEADER_INCLUDED

#include "snn.h"

#if !MLP_PRELOAD_MODE || MLP_LEARN_MODE
#include <stdio.h>
#endif

// -------------------- Defines ------------------------------------------------

#define MLP_EXPTABL_MAX        8
#define MLP_EXPTABL_SIZE      512

// -------------------- Defines ------------------------------------------------

#define MLP_UNROLL_CYCLES      1
#define MLP_ROLLING_LEARN      0

#define MLP_F_CELL_CMASK      0x0001
#define MLP_F_CELL_PMASK      0x0002

// -----------------------------------------------------------------------------

#define  MLP_ABS(x) (((x) > 0) ? (x):(-(x)))
#define  MLP_ERR(e) ((e)*(e)*(e))
//#define  MLP_ERR(e) (((e) > 0) ? (e)*(e) : -((e)*(e)))

#if MLP_UPSCALE

#if MLP_BYTE_NET

#define MLP_MAX_INT_C 127
#define MLP_MAX_INT_S 255

#if MLP_INT_MODE
#define x_UPCO_C    << 3
#define x_DNCO_C    >> 3
#define x_UPCO_S    << 8
#define x_DNCO_S    >> 8
#define x_DNDNCO_S  >> 16
#else
#define x_UPCO_C    *(8)
#define x_DNCO_C    *(0.125)
#define x_UPCO_S    *(256)
#define x_DNCO_S    *(0.00390625)
#define x_DNDNCO_S  *(1.52587890625E-05)
#endif

#else // SHORT mode

#define MLP_MAX_INT_C 32767
#define MLP_MAX_INT_S 4095

#if MLP_INT_MODE
#define x_UPCO_C    << 9
#define x_DNCO_C    >> 9
#define x_UPCO_S    << 12
#define x_DNCO_S    >> 12
#define x_DNDNCO_S  >> 24
#else
#define x_UPCO_C    *(512)
#define x_DNCO_C    *(0.001953125)
#define x_UPCO_S    *(4096)
#define x_DNCO_S    *(0.000244140625)
#define x_DNDNCO_S  *(5.96046447753906E-08)
#endif

#endif // MLP_BYTE_NET

#else

// #define MLP_MAX_INT_C 63.9
// #define MLP_MAX_INT_C 15.875
#define MLP_MAX_INT_C 127
#define MLP_MAX_INT_S 1.0

#define x_UPCO_C
#define x_DNCO_C
#define x_UPCO_S
#define x_DNCO_S
#define x_DNDNCO_S

#endif

#if MLP_EMULATE_INT && !(MLP_INT_MODE)
#define FINT_C_OF(x) ((fint_c)(x))
#define FINT_S_OF(x) ((fint_s)(x))
#else
#define FINT_C_OF(x) ((flong)(x))
#define FINT_S_OF(x) ((flong)(x))
#endif

#if MLP_INT_MODE
#define FINT_C_LD(x) (((x) > -MLP_MAX_INT_C && (x) < MLP_MAX_INT_C) ? (fint_c)(x) : (((x) > 0) ? MLP_MAX_INT_C : -MLP_MAX_INT_C))
#else
#define FINT_C_LD(x) (x)
#endif

// -------------------- Structures and classes ---------------------------------

typedef struct
{
	_USHORT inp_ind;
	fint_c  bias;
	fint_c  weights[MLP_CELL_MAXINPUTS];

#if MLP_LEARN_MODE // learning
	float   err;

	float   sbias;
	float   psbias;
	float   sws[MLP_CELL_MAXINPUTS];
	float   psws[MLP_CELL_MAXINPUTS];
	_INT    num_sws;
	_INT    num_psws;

	_UCHAR  flags;
	//  _USHORT out_ind;
	_INT    num_changes;
	float   prev_val;
#endif
} mlp_cell_type, * p_mlp_cell_type;

typedef struct
{
	flong   sum_delt;
	_INT    num_sum;
} mlp_layer_type, * p_mlp_layer_type;

typedef struct
{
	_UCHAR        id_str[32];
	_INT          num_layers;
	_INT          num_inputs;
	_INT          num_outputs;
	fint_s        exp_tabl[MLP_EXPTABL_SIZE];
	mlp_cell_type cells[MLP_NET_MAX_NUMCELLS];
	mlp_layer_type layers[MLP_NET_NUMLAYERS];
} mlp_net_type, * p_mlp_net_type;

// ---------------- Functions --------------------------------------------------

_INT CountNetResult(p_UCHAR inps, p_UCHAR outs, p_mlp_data_type mlpd, _INT lang );
fint_s CountCellSignal(_INT nc, p_mlp_data_type mlpd);

RECO_DATA p_mlp_net_type GetNetData(_INT lang);

#if !MLP_PRELOAD_MODE

_INT LoadNet(FILE * file, p_mlp_data_type mlpd, _INT langID );
_INT InitNet(_INT type, p_mlp_data_type mlpd, _INT langID );

#endif

#if MLP_LEARN_MODE

_INT FillExpTable(flong ic, p_mlp_data_type mlpd);
_INT InitNetWeights(_INT type, float ic, p_mlp_data_type mlpd, _INT langID);
_INT CountNetError(float * desired_outputs, float zc, p_mlp_data_type mlpd, _INT langID);
_INT ModifyNetDeltas(_INT flags, p_mlp_data_type mlpd, _INT langID);
_INT AdjustNetWeights(_INT flags, float * lcs, float ic, p_mlp_data_type mlpd, _INT langID);
_INT ShakeNetWeights(_INT flags, float lc, p_mlp_data_type mlpd, _INT langID);
_INT CountNetStats(_INT mode, p_mlp_data_type mlpd, _INT langID);
_INT SaveNet(FILE *file, p_mlp_data_type mlpd, _INT langID );
_INT DumpNet(FILE *file, p_mlp_data_type mlpd, _INT langID );

_INT CalcHiddenLayerError(_INT layer_st, _INT layer_len, p_mlp_data_type mlpd);
_INT CountLayerStats(_INT mode, _INT layer_num, _INT layer_st, _INT layer_len, p_mlp_data_type mlpd);
_INT InitCellWeights(_INT type, float ic, _INT ncell, p_mlp_data_type mlpd);

#endif // MLP_LEARN_MODE

#endif // MLP_HEADER_INCLUDED
