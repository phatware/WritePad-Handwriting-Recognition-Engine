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

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#if !MLP_PRELOAD_MODE
#include <string.h>
#endif

#include "mlp.h"

#include "reco.h"
#include "langid.h"
#include "langutil.h"

/* **************************************************************************** */
/* *         Calculate Net result                                             * */
/* **************************************************************************** */

_INT CountNetResult(p_UCHAR inps, p_UCHAR outs, p_mlp_data_type mlpd, _INT lang )
{
    mlp_size_info info;
    GetSnnSizeInfo(lang, &info);
    _INT   i ,k;
    
    // ----------------- Fill in Inputs --------------------------------------------
    
    k = info.net_cells_1+info.net_cells_2+info.net_cells_3+info.net_cells_4;
    for (i = 0; i < MLP_NET_NUMINPUTS; i ++, k ++)
    {
        mlpd->signals[k] = (fint_s)(((flong)inps[i] x_UPCO_S)/256); // Normalize inputs
    }
    // ----------------- Count net   cells -----------------------------------------
    
    for (i = 0; i < info.net_cells; i ++)
    {
        mlpd->signals[i] = CountCellSignal(i, mlpd);
    }
    // ----------------- Get results -----------------------------------------------
    
    k = info.net_cells_1+info.net_cells_2;
    for (i = 0; i < info.output; i ++, k ++)
    {
        outs[i] = (_UCHAR)(((flong)mlpd->signals[k] * 256) x_DNCO_S);
    }
    
    return 0;
}


#if !MLP_PRELOAD_MODE

/* **************************************************************************** */
/* *         Load net from   given stream                                     * */
/* **************************************************************************** */

_INT LoadNet(FILE * file, p_mlp_data_type mlpd, _INT langID )
{
    _INT   i, j;
    _INT   num_l, num_i, num_o, num_c, d;
    _CHAR  str[64];
    float  f;
    mlp_cell_type * cell;
    p_mlp_net_type net = (p_mlp_net_type)mlpd->net;
    
    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);
    net->num_outputs = info.output;
    
    const int MLP_NET_NUMCELLS = info.net_cells;
    const int MLP_NET_NUMOUTPUTS = info.output;
    
    if (net == 0)
        goto err;
    
    if ( fscanf(file, "%s %d %d %d %d", str, &num_l, &num_i, &num_o, &num_c) != 5 )
        throw 0x0008;

    
    
    //  if (strcmp(str, MLP_ID_STR) || num_l != MLP_NET_NUMLAYERS ||
    //      num_i != MLP_NET_NUMINPUTS || num_o != MLP_NET_NUMOUTPUTS  throw 0x0008;
    
    if (strcmp( str, (_STR)GetSnnID( langID )) || num_l != MLP_NET_NUMLAYERS ||
        num_i != MLP_NET_NUMINPUTS || num_o != MLP_NET_NUMOUTPUTS ||
        num_c != MLP_NET_NUMCELLS)
    {
        throw 0x0008;
    }
    
    strcpy( (_STR)net->id_str, (_STR)GetSnnID( langID ) );
    net->num_layers  = MLP_NET_NUMLAYERS;
    net->num_inputs  = MLP_NET_NUMINPUTS;
    net->num_outputs = MLP_NET_NUMOUTPUTS;
    
    //for (i = 0; i < num_o; i ++)
    // {
    //  fscanf(file, "%s", str);
    // }
    
    for (i = 0; i < MLP_EXPTABL_SIZE; i ++)
    {
        fscanf(file, "%f", &f);
        net->exp_tabl[i] = (fint_s)(f * (1 x_UPCO_S));
    }
    
    cell = &(net->cells[0]);
    for (i = 0; i < MLP_NET_NUMCELLS; i ++, cell ++)
    {
        //fscanf(file, "%d", &num_i); // Temp for compatibility
        //    fscanf(file, "%f", &f);
        fscanf(file, "%d", &d);
        cell->inp_ind = (_USHORT)d;
        fscanf(file, "%f", &f);
        cell->bias = (fint_c)(FINT_C_LD(f * (1 x_UPCO_C)));
        
        for (j = 0; j < MLP_CELL_MAXINPUTS; j ++)
        {
            if (fscanf(file, "%f", &f) != 1) throw 0x0008;
            cell->weights[j] = (fint_c)(FINT_C_LD(f * (1 x_UPCO_C)));
        }
    }
    
    return 0;
err:
    return 1;
}
#endif //!PRELOAD

//#if !MLP_PRELOAD_MODE

#if MLP_LEARN_MODE

/* **************************************************************************** */
/* *        Initialize shifts and weights of the given net                    * */
/* **************************************************************************** */

_INT InitNet( _INT type, p_mlp_data_type mlpd, _INT langID )
{
    _INT i;
    _INT inp;
    p_mlp_net_type net = (p_mlp_net_type)mlpd->net;
    
    strcpy((_STR) net->id_str, (_STR)GetSnnID( langID ) );
    net->num_layers  = MLP_NET_NUMLAYERS;
    net->num_inputs  = MLP_NET_NUMINPUTS;

    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);
    net->num_outputs = info.output;

    const int MLP_NET_1L_NUMCELLS = info.net_cells_1;
    const int MLP_NET_2L_NUMCELLS = info.net_cells_2;
    const int MLP_NET_3L_NUMCELLS = info.net_cells_3;
    const int MLP_NET_4L_NUMCELLS = info.net_cells_4;

    // ----------------- Create first hidden layer ---------------------------------
    
    for (i = 0; i < MLP_COEF_SHARE; i+=1)
    {
        net->cells[i].inp_ind = (_USHORT)(MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS+MLP_NET_3L_NUMCELLS+MLP_NET_4L_NUMCELLS);
        //    net->cells[i+1].inp_ind = (_USHORT)(MLP_CELL_MAXINPUTS+MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS+MLP_NET_3L_NUMCELLS+MLP_NET_4L_NUMCELLS);
        //    net->cells[i].out_ind = (_USHORT)i;
    }
    
    for (i = MLP_COEF_SHARE, inp = MLP_NUM_CFF; i < MLP_NET_1L_NUMCELLS; i ++, inp += MLP_CELL_MAXINPUTS)
    {
        if (inp >= MLP_NUM_CFF + MLP_NUM_BMP)
            inp = MLP_NUM_CFF;
        net->cells[i].inp_ind = (_USHORT)(inp+(MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS+MLP_NET_3L_NUMCELLS+MLP_NET_4L_NUMCELLS));
        //    net->cells[i].out_ind = (_USHORT)i;
    }
    
    // ----------------- Create second hidden layer --------------------------------
    
    for (i = 0; i < MLP_NET_2L_NUMCELLS; i ++)
    {
        inp = (i*MLP_CELL_MAXINPUTS) % MLP_NET_1L_NUMCELLS;
        net->cells[i+MLP_NET_1L_NUMCELLS].inp_ind = (_USHORT)inp;
        //    net->cells[i+MLP_NET_1L_NUMCELLS].out_ind = (_USHORT)(MLP_NET_1L_NUMCELLS+i);
    }
    
    // ----------------- Create output layer ---------------------------------------
    
    for (i = 0; i < MLP_NET_3L_NUMCELLS; i ++)
    {
        inp = i*MLP_PREOUT_STEP;
        net->cells[i+MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS].inp_ind = (_USHORT)(MLP_NET_1L_NUMCELLS+inp);
        //    net->cells[i+MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS].out_ind = (_USHORT)(MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS+i);
    }
    
    // ----------------- Fill Exponents table -------------------------------------
    
    FillExpTable(1, mlpd);
    
    return 0;
}

/* **************************************************************************** */
/* *         Set initial weights                                              * */
/* **************************************************************************** */

_INT InitNetWeights(_INT type, float ic, p_mlp_data_type mlpd, _INT langID)
{
    _INT i;
    
    /* TODO: 
    if (type >= 0)
        randomize();
    */
    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);

    for (i = 0; i < info.net_cells; i++)
    {
        InitCellWeights(type, ic, i, mlpd);
    }
    
    return 0;
}

/* **************************************************************************** */
/* *         Set initial weights to the sell                                  * */
/* **************************************************************************** */

_INT InitCellWeights(_INT type, float ic, _INT ncell, p_mlp_data_type mlpd)
{
    _INT i;
    float  w;
    p_mlp_net_type  net = (p_mlp_net_type)mlpd->net;
    p_mlp_cell_type cell = &(net->cells[ncell]);
    
    if (type == 1)
        ic = 0.1f;
    
    if (type == 0 || (type == 1 && cell->num_changes == 0))
    {
        cell->err = 0;
        
        w = (float) ((float) (rand() % 10000)*0.0001*ic);
        w += ic/8.0f;
        if ((rand() % 100) > 50)
            w *= -1.0f;
        cell->bias = w x_UPCO_C;
        cell->sbias = 0;
        cell->psbias = 0;
        cell->num_sws = 0;
        cell->num_psws= 0;
        
        for (i = 0; i < MLP_CELL_MAXINPUTS; i ++)
        {
            w = (float) ((float) (rand() % 10000)*0.0001*ic);
            w += ic/8.0f;
            if ((rand() % 100) > 50)
                w *= -1.0f;
            
            cell->weights[i] = w x_UPCO_C;
            cell->sws[i] = 0;
            cell->psws[i] = 0;
        }
    }
    
    if (type == 1)  // Init links on zero change cells
    {
        p_mlp_cell_type pcell = &(net->cells[cell->inp_ind]);
        
        for (i = 0; i < MLP_CELL_MAXINPUTS; i ++, pcell ++)
        {
            if (cell->inp_ind < ncell) // Do not init input layer
            {
                if (pcell->num_changes == 0)
                {
                    w = (float) ((float) (rand() % 10000)*0.0001*ic);
                    w += ic/8.0f;
                    if ((rand() % 100) > 50)
                        w *= -1.0f;
                    cell->weights[i] = w x_UPCO_C;
                }
            }
        }
    }
    
    
    if (type == 1 && cell->num_changes > 0)
    {
         // Let's decrease volume of weights to allow new to grow
        cell->bias /= 2;
        for (i = 0; i < MLP_CELL_MAXINPUTS; i ++)
            cell->weights[i] /= 2;
    }
    
    return 0;
}

/* **************************************************************************** */
/* *         Create exp formula table                                         * */
/* **************************************************************************** */

_INT FillExpTable(flong ic, p_mlp_data_type mlpd)
{
    _INT i;
    float v;
    p_mlp_net_type net = (p_mlp_net_type)mlpd->net;
    
    for (i = 0; i < MLP_EXPTABL_SIZE; i ++)
    {
        v = -(float)(((float) (i) +0.6) / (MLP_EXPTABL_SIZE / MLP_EXPTABL_MAX));
        net->exp_tabl[i] = (float)((1.0 / (1.0 + exp(ic*v))) * MLP_MAX_INT_S);
    }
    
    return 0;
}
//#endif // !PRELOAD_MODE


/* **************************************************************************** */
/* *         Calculate Net errors                                             * */
/* **************************************************************************** */

_INT CountNetError(float * desired_outputs, float zc, p_mlp_data_type mlpd, _INT langID )
{
    _INT   i;
    float  e, outp;
    fint_s *outs;
    p_mlp_cell_type cells;
    p_mlp_net_type net = (p_mlp_net_type)mlpd->net;
    
    // ----------------- Calculate output layer error ----------------------------

    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);

    const int MLP_NET_1L_NUMCELLS = info.net_cells_1;
    const int MLP_NET_2L_NUMCELLS = info.net_cells_2;
    const int MLP_NET_3L_NUMCELLS = info.net_cells_3;
    const int MLP_NET_NUMOUTPUTS = info.output;
    
    outs  = &mlpd->signals[MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS];
    cells = &net->cells[MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS];
    for (i = 0; i < MLP_NET_NUMOUTPUTS; i ++)
    {
        outp = outs[i];
        e = (float)(((desired_outputs[i] x_UPCO_S) - outp) x_DNCO_S);
        
        if (desired_outputs[i] < 0.5 && desired_outputs[i] > 0.01)
            e /= (desired_outputs[i]*100);
        
        e = MLP_ERR(e);
        
        e = outp * e * (MLP_MAX_INT_S - outp);
        if (desired_outputs[i] < 0.5)
            e *= zc;
        cells[i].err = (float)(e x_DNDNCO_S);
    }
    
    // -------------- Calculate hidden layers errors -----------------------------
    
    cells = &net->cells[0];
    for (i = 0; i < MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS; i ++, cells ++)
        cells->err = 0;
    
    CalcHiddenLayerError(MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS, MLP_NET_3L_NUMCELLS, mlpd);
    CalcHiddenLayerError(MLP_NET_1L_NUMCELLS, MLP_NET_2L_NUMCELLS, mlpd);
    
    return 0;
}

/* **************************************************************************** */
/* *       Calculate hidden layer error                                       * */
/* **************************************************************************** */

_INT CalcHiddenLayerError(_INT layer_st, _INT layer_len, p_mlp_data_type mlpd)
{
    _INT   i, n, layer_end, num;
    fint_s        * ou;
    mlp_cell_type * cell;
    mlp_cell_type * cp;
    mlp_cell_type * cpp;
    float * pf;
    p_mlp_net_type net = (p_mlp_net_type)mlpd->net;
    
    // ---------------- Summ up errors for previous layer ------------------------
    
    layer_end = layer_st + layer_len;
    for (n = layer_st; n < layer_end; n ++)
    {
        float e;
        
        cell = &(net->cells[n]);
        pf   = &(cell->weights[0]);
        cpp  = &(net->cells[cell->inp_ind]);
        e    = (float)(cell->err x_DNCO_C);
        for (i = 0; i < MLP_CELL_MAXINPUTS; i += 4, cpp += 4, pf += 4)
        {
            float a,b;
            
            a = pf[0] * e; // Hint compiler -- there's no dependency!
            b = pf[1] * e;
            cpp[0].err += a;
            cpp[1].err += b;
            a = pf[2] * e;
            b = pf[3] * e;
            cpp[2].err += a;
            cpp[3].err += b;
        }
    }
    
    // -------------------- Convert calculated summs to real error ---------------
    
    n   = net->cells[layer_st].inp_ind;
    num = net->cells[layer_end-1].inp_ind - n + MLP_CELL_MAXINPUTS;
    cp  = &(net->cells[n]);
    ou  = &(mlpd->signals[n]);
    for (n = 0; n < num; n += 4, cp += 4, ou += 4)
    {
        float a,b,c;
        
        c = ou[0];
        a = c * (MLP_MAX_INT_S - c) * cp[0].err;
        c = ou[1];
        b = c * (MLP_MAX_INT_S - c) * cp[1].err;
        cp[0].err = (float)(a x_DNDNCO_S);
        cp[1].err = (float)(b x_DNDNCO_S);
        
        c = ou[2];
        a = c * (MLP_MAX_INT_S - c) * cp[2].err;
        c = ou[3];
        b = c * (MLP_MAX_INT_S - c) * cp[3].err;
        cp[2].err = (float)(a x_DNDNCO_S);
        cp[3].err = (float)(b x_DNDNCO_S);
    }
    
    return 0;
}

/* **************************************************************************** */
/* *         Modify weight deltas                                             * */
/* **************************************************************************** */

_INT ModifyNetDeltas(_INT flags, p_mlp_data_type mlpd, _INT langID )
{
    _INT  n;
    float e;
    float *ppi;
    float a, b;
    mlp_cell_type * cell;
    p_mlp_net_type net = (p_mlp_net_type)mlpd->net;
    
    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);
    const int MLP_NET_NUMCELLS = info.net_cells;

    cell = &(net->cells[0]);
    for (n = 0; n < MLP_NET_NUMCELLS; n ++, cell ++)
    {
        cell->sbias += cell->err;
        
        ppi  = mlpd->signals + cell->inp_ind;
        e    = (float)(cell->err x_DNCO_S);
        
#if MLP_UNROLL_CYCLES
        a = ppi[0] * e;
        b = ppi[1] * e;
        cell->sws[0] += a;
        a = ppi[2] * e;
        cell->sws[1] += b;
        b = ppi[3] * e;
        cell->sws[2] += a;
        cell->sws[3] += b;
        
        a = ppi[4] * e;
        b = ppi[5] * e;
        cell->sws[4] += a;
        a = ppi[6] * e;
        cell->sws[5] += b;
        b = ppi[7] * e;
        cell->sws[6] += a;
        cell->sws[7] += b;
        
        a = ppi[8] * e;
        b = ppi[9] * e;
        cell->sws[8] += a;
        a = ppi[10] * e;
        cell->sws[9] += b;
        b = ppi[11] * e;
        cell->sws[10] += a;
        cell->sws[11] += b;
        
        a = ppi[12] * e;
        b = ppi[13] * e;
        cell->sws[12] += a;
        a = ppi[14] * e;
        cell->sws[13] += b;
        b = ppi[15] * e;
        cell->sws[14] += a;
        cell->sws[15] += b;
        
        a = ppi[16] * e;
        b = ppi[17] * e;
        cell->sws[16] += a;
        a = ppi[18] * e;
        cell->sws[17] += b;
        b = ppi[19] * e;
        cell->sws[18] += a;
        cell->sws[19] += b;
#else
        {
            _INT i;
            float *swsp = cell->sws;;
            for (i = 0; i < MLP_CELL_MAXINPUTS; i += 4, swsp += 4, ppi += 4)
            {
                a = ppi[0] * e;  // To Explain to compiler that there is no dependence on data
                b = ppi[1] * e;
                swsp[0] += a;
                a = ppi[2] * e;
                swsp[1] += b;
                b = ppi[3] * e;
                swsp[2] += a;
                swsp[3] += b;
            }
        }
#endif
        
        cell->num_sws ++;
    }
    
    return 0;
}

/* **************************************************************************** */
/* *         Adjust net weights according to propagated error                 * */
/* **************************************************************************** */

_INT AdjustNetWeights(_INT flags, float * lcs, float ic, p_mlp_data_type mlpd, _INT langID )
{
    _INT  i, n;
    float d, lc, llc;
    p_mlp_cell_type cell;
    p_mlp_net_type net = (p_mlp_net_type)mlpd->net;
    
    mlp_size_info info;
    GetSnnSizeInfo( langID, &info );

    const int MLP_NET_1L_NUMCELLS = info.net_cells_1;
    const int MLP_NET_2L_NUMCELLS = info.net_cells_2;
    const int MLP_NET_NUMCELLS = info.net_cells;


    if (flags & 0x01) // If mozhno, apply deltas to weights
    {
        cell = net->cells;
        for (n = 0, llc = lcs[1]; n < MLP_NET_NUMCELLS; n ++, cell ++)
        {
            if (n == MLP_NET_1L_NUMCELLS) 
                llc = lcs[2];
            if (n == MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS) 
                llc = lcs[3];
            
            if (cell->num_changes == 0 && cell->prev_val > 0) // Try to recover stuck cells
            {
                if ((cell->bias > 0 && cell->sbias < 0) || (cell->bias < 0 && cell->sbias > 0))
                    lc = llc * 4;
                else
                    lc = llc / 4;
            } else 
                lc = llc;
            
            d = cell->sbias*lc + ic*cell->psbias; cell->bias += d; cell->psbias = d; cell->sbias  = 0;
            
            for (i = 0; i < MLP_CELL_MAXINPUTS; i ++)
            {
                d = cell->sws[i]*lc + ic*cell->psws[i];
                cell->weights[i] += d;
                cell->psws[i] = d;
                cell->sws[i] = 0;
            }
            
            cell->num_psws = cell->num_sws; cell->num_sws = 0; 
        }
    }
    
    return 0;
}

/* **************************************************************************** */
/* *         Shake  net weights at random                                     * */
/* **************************************************************************** */

_INT ShakeNetWeights(_INT flags, float lc, p_mlp_data_type mlpd, _INT langID )
{
    _INT  i, n;
    float w;
    mlp_cell_type * cell;
    p_mlp_net_type net = (p_mlp_net_type)mlpd->net;

    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);

    const int MLP_NET_NUMCELLS = info.net_cells;

    
    cell = &(net->cells[0]);
    for (n = 0; n < MLP_NET_NUMCELLS; n ++, cell ++)
    {
        if (flags == 0 && lc > 0)
        {
            w = (float)((float) (rand() % 10000)*0.0001);
            if ((rand() % 100) > 50)
                w *= -1.0f;
            
            cell->bias  *= 1.0f + w*lc;
            
            w = (float) ((float) (rand() % 10000)*0.0001);
            if ((rand() % 100) > 50)
                w *= -1.0f;
            
            cell->bias  += w*lc x_UPCO_C;
            
            for (i = 0; i < MLP_CELL_MAXINPUTS; i ++)
            {
                w = (float) ((float) (rand() % 10000)*0.0001);
                if ((rand() % 100) > 50)
                    w *= -1.0f;
                
                cell->weights[i] *= 1 + w*lc;
                
                w = (float) ((float) (rand() % 10000)*0.0001);
                if ((rand() % 100) > 50)
                    w *= -1.0f;
                
                cell->weights[i] += w*lc x_UPCO_C;
            }
        }
        
        if (cell->bias >  MLP_MAX_INT_C)
            cell->bias = MLP_MAX_INT_C;
        if (cell->bias < -MLP_MAX_INT_C)
            cell->bias = -MLP_MAX_INT_C;
        
        for (i = 0; i < MLP_CELL_MAXINPUTS; i ++)
        {
            if (cell->weights[i] >  MLP_MAX_INT_C)
                cell->weights[i] = MLP_MAX_INT_C;
            if (cell->weights[i] < -MLP_MAX_INT_C)
                cell->weights[i] = -MLP_MAX_INT_C;
        }
    }
    
    return 0;
}

/* **************************************************************************** */
/* *         Count some stats                                                 * */
/* **************************************************************************** */

_INT CountNetStats(_INT mode, p_mlp_data_type mlpd, _INT langID )
{
    _INT i;
    p_mlp_net_type net = (p_mlp_net_type)mlpd->net;

    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);

    const int MLP_NET_1L_NUMCELLS = info.net_cells_1;
    const int MLP_NET_2L_NUMCELLS = info.net_cells_2;
    const int MLP_NET_3L_NUMCELLS = info.net_cells_3;
    
    CountLayerStats(mode, 1, 0, MLP_NET_1L_NUMCELLS, mlpd);
    CountLayerStats(mode, 2, MLP_NET_1L_NUMCELLS, MLP_NET_2L_NUMCELLS, mlpd);
    CountLayerStats(mode, 3, MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS, MLP_NET_3L_NUMCELLS, mlpd);
    
    if (mode == 3)
    {
        for (i = 0; i < MLP_NET_NUMLAYERS; i ++)
        {
            net->layers[i].num_sum = 0; net->layers[i].sum_delt = 0;
        }
    }
    
    return 0;
}

/* **************************************************************************** */
/* *       Gather some stats                                                  * */
/* **************************************************************************** */

_INT CountLayerStats(_INT mode, _INT layer_num, _INT layer_st, _INT layer_len, p_mlp_data_type mlpd)
{
    _INT  n;
    float ret;
    float output;
    mlp_cell_type * cell;
    p_mlp_net_type net = (p_mlp_net_type)mlpd->net;
    
    cell = &(net->cells[layer_st]);
    
    switch (mode)
    {
        case 0:
        {
            for (n = 0; n < layer_len; n ++, cell ++)
            {
                cell->num_changes = 0;
                cell->prev_val = 0;
            }
            break;
        }
            
        case 1:
        {
            for (n = 0; n < layer_len; n ++, cell ++)
            {
                output = mlpd->signals[n+layer_st];
                if (cell->prev_val > 0.0)
                {
                    if ((output > 0.2 && output < 0.8) || MLP_ABS(cell->prev_val - output) > 0.05)
                        cell->num_changes ++;
                }
                cell->prev_val = output;
            }
            
            break;
        }
            
        case 2:
        {
            for (n = 0, ret = 0.0f; n < layer_len; n ++, cell ++)
            {
                for (_INT i = 0; i < MLP_CELL_MAXINPUTS; i ++)
                    ret += MLP_ABS(cell->psws[i]);
                ret = ret / (float)MLP_CELL_MAXINPUTS;
                
                net->layers[layer_num].sum_delt += ret;
                net->layers[layer_num].num_sum  += cell->num_psws;
            }
            break;
        }
    }
    
    return 0;
}

/* **************************************************************************** */
/* *         Save net to the given file                                       * */
/* **************************************************************************** */

_INT SaveNet(FILE * file, p_mlp_data_type mlpd, _INT langID )
{
    _INT i, j;
    mlp_cell_type * cell;
    p_mlp_net_type net = (p_mlp_net_type)mlpd->net;

    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);

    const int MLP_NET_NUMCELLS = info.net_cells;

    fprintf(file, "%s %d %d %d %d ", net->id_str, net->num_layers, net->num_inputs, net->num_outputs, MLP_NET_NUMCELLS);
    
    for (i = 0; i < MLP_EXPTABL_SIZE; i ++)
        fprintf(file, "%f ", (float)net->exp_tabl[i] x_DNCO_S);
    
    cell = &(net->cells[0]);
    for (i = 0; i < MLP_NET_NUMCELLS; i ++, cell ++)
    {
        fprintf(file, "%d %f ", (_INT)cell->inp_ind, (float)cell->bias x_DNCO_C);
        for (j = 0; j < MLP_CELL_MAXINPUTS; j ++)
            fprintf(file, "%f ", (float)cell->weights[j] x_DNCO_C);
    }
    
    return 0;
}

/* **************************************************************************** */
/* *         Print net to a given file                                        * */
/* **************************************************************************** */

_INT DumpNet(FILE * file, p_mlp_data_type mlpd, _INT langID )
{
    _INT i, j, n = 0;
    mlp_cell_type * cell;
    p_mlp_net_type net = (p_mlp_net_type)mlpd->net;

    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);

    const int MLP_NET_NUMCELLS = info.net_cells;
    const int MLP_NET_1L_NUMCELLS = info.net_cells_1;
    const int MLP_NET_2L_NUMCELLS = info.net_cells_2;

    
    fprintf(file, "ID: %s\n", net->id_str);
    fprintf(file, "Num Layers:  %d\n", net->num_layers);
    fprintf(file, "Num Inputs:  %d\n", net->num_inputs);
    fprintf(file, "Num Outputs: %d\n", net->num_outputs);
    fprintf(file, "Assigned symbols: %s\n", GetSnnSymbol( langID ) );
    fprintf(file, "\n");
    
    cell = &(net->cells[0]);
    for (i = 0; i < MLP_NET_NUMCELLS; i ++, cell ++)
    {
        if (i == 0)
        {
            fprintf(file, "Layer 1. \n\n");
            n = 0;
        }
        if (i == MLP_NET_1L_NUMCELLS)
        {
            fprintf(file, "Layer 2. \n\n");
            n = MLP_NET_1L_NUMCELLS;
        }
        if (i == MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS)
        {
            fprintf(file, "Layer 3. \n\n");
            n = MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS;
        }
        
        fprintf(file, "Cell %d\n", i-n);
        fprintf(file, "Inp index   %d\n", (int)cell->inp_ind);
        fprintf(file, "Bias:       %f\n", (float)cell->bias);
        fprintf(file, "Output:     %f\n", (float)mlpd->signals[i]);
        fprintf(file, "Error:      %f\n", (float)cell->err);
        fprintf(file, "Num Steps:  %f\n", (float)cell->num_sws);
        fprintf(file, "Num Changes %f\n", (float)cell->num_changes);
        fprintf(file, "Sbias:      %f\n", (float)cell->sbias);
        fprintf(file, "PSbias:     %f\n", (float)cell->psbias);
        fprintf(file, "Weights: \n");
        for (j = 0; j < MLP_CELL_MAXINPUTS; j ++)
        {
            fprintf(file, "%f ", (float)cell->weights[j]);
            if (j%8 == 7) fprintf(file, "\n");
        }
        fprintf(file, "\n");
        fprintf(file, "SWS: \n");
        for (j = 0; j < MLP_CELL_MAXINPUTS; j ++)
        {
            fprintf(file, "%f ", (float)cell->sws[j]);
            if (j%8 == 7) fprintf(file, "\n");
        }
        fprintf(file, "\n");
        fprintf(file, "PSWS: \n");
        for (j = 0; j < MLP_CELL_MAXINPUTS; j ++)
        {
            fprintf(file, "%f ", (float)cell->psws[j]);
            if (j%8 == 7) fprintf(file, "\n");
        }
        fprintf(file, "\n");
    }
    
    return 0;
}
#endif // LEARN_MODE

/* *************************************************************************** */
/* *        Alloc or attach to mlp net                                       * */
/* *************************************************************************** */

_INT  InitSnnData( p_UCHAR name, p_mlp_data_type mlpd, _INT lang )
{
#if MLP_PRELOAD_MODE
    mlpd->net = (p_VOID) (GetNetData(lang));
    return 0;
#else
    if (name == _NULL || *name == 0)
    {
        mlpd->net = (p_VOID)( GetNetData( lang ) );
    }
    else
    {
        static int net_init = 0;
        static mlp_data_type gmlpd;
        
        if (net_init == 0)
        {
            net_init =  LoadNetData( &gmlpd, (_STR)name, lang ) ? 2 : 1;
        }
        
        if (net_init != 1)
            goto err;
        mlpd->net = gmlpd.net;
    }
    return 0;
err:
    return 1;
#endif // MLP_PRELOAD_MODE
}

/* *************************************************************************** */
/* *        Head NN routine called from WS.C                                 * */
/* *************************************************************************** */

_INT GetSnnResults(p_UCHAR pCoeff, p_UCHAR answs, p_mlp_data_type mlpd, _INT lang )
{
    mlp_size_info info;
    _INT i, j;
    _UCHAR outputs[MLP_NET_MAX_NUMOUTPUTS];
    
    GetSnnSizeInfo(lang, &info);
    RECO_DATA p_UCHAR symco = GetSnnSymbol(lang);
    mlp_net_type  * net = (p_mlp_net_type)mlpd->net;
    
    if (net == _NULL) goto err;
    
    CountNetResult(pCoeff, outputs, mlpd, lang );
    
    for (i = 0; i < info.output; i ++)
    {
        if ((j = outputs[i]) == 0) j = 1;
        answs[symco[i]] = (_UCHAR)j;
    }
    
    return 0;
err:
    return 1;
}

#if !MLP_PRELOAD_MODE
/* **************************************************************************** */
/* *         Load the net                                                     * */
/* **************************************************************************** */
_INT LoadNetData(p_mlp_data_type mlpd, _CHAR * net_name, _INT lang )
{
    FILE * file;
    
    //  if ((mlpd->net = HWRMemoryAlloc(sizeof(mlp_net_type))) == 0) goto err;
    mlpd->net = malloc( sizeof( mlp_net_type ) );
    if (mlpd->net == 0)
        goto err;
    
#if MLP_LEARN_MODE
    InitNet( 0, mlpd, lang );
#endif
    
    if (net_name) // Request to load net
    {
        if ((file = fopen(net_name, "rt")) == 0)
        {
            printf("Can't open net file: %s\n", net_name);
            goto err;
        }
        
        if ( LoadNet( file, mlpd, lang ) )
        {
            printf("Error loading net: %s\n", net_name);
        }
        
        fclose(file);
    }
    
    return 0;
err:
    return 1;
}

/* **************************************************************************** */
/* *         Save the net                                                     * */
/* **************************************************************************** */

_INT SaveNetData( p_mlp_data_type mlpd, _CHAR * net_name, float e, _INT langID )
{
    FILE * file;
    
    if ((file = fopen(net_name, "wt")) == 0)
    {
        printf("Can't create file for net: %s\n", net_name);
        goto err;
    }
    
#if MLP_LEARN_MODE
    SaveNet( file, mlpd, langID );
#endif
    
    fprintf(file, "Mean Sq Err per sample: %f\n", e);
    fprintf(file, "Created by %s", GetSnnID( langID ) );
    
    fclose(file);
    
    return 0;
err:
    return 1;
}

/* **************************************************************************** */
/* *         Dump the net                                                     * */
/* **************************************************************************** */

_INT DumpNetData(p_mlp_data_type mlpd, _CHAR * dmp_name, float e, _INT lang )
{
    FILE * file;
    
    if ((file = fopen(dmp_name, "wt")) == 0)
    {
        printf("Can't create file for net: %s\n", dmp_name);
        goto err;
    }
    
#if MLP_LEARN_MODE
    DumpNet(file, mlpd, lang );
#endif
    
    fprintf(file, "Mean Sq Err per sample: %f\n", e);
    fprintf(file, "Created by %s", GetSnnID( lang ));
    
    fclose(file);
    
    return 0;
err:
    return 1;
}

#endif // #if !MLP_PRELOAD_MODE

const p_UCHAR GetSnnID( _INT lang)
{
    static const p_UCHAR id_frn = (p_UCHAR)MLP_ID_STR_FRN;
    static const p_UCHAR id_itl = (p_UCHAR)MLP_ID_STR_ITL;
    static const p_UCHAR id_ger = (p_UCHAR)MLP_ID_STR_GER;
    static const p_UCHAR id_fin = (p_UCHAR)MLP_ID_STR_FIN;
    static const p_UCHAR id_int = (p_UCHAR)MLP_ID_STR_INT;
    static const p_UCHAR id_eng = (p_UCHAR)MLP_ID_STR_ENG;
    static const p_UCHAR id_ind = (p_UCHAR)MLP_ID_STR_IND;
    
    switch(lang)
    {
        case LANGUAGE_FRENCH :
        case LANGUAGE_PORTUGUESE :
            return &id_frn[0];
        
        case LANGUAGE_ITALIAN :
            return &id_itl[0];
        
        case LANGUAGE_GERMAN :
            return &id_ger[0];
        
        case LANGUAGE_FINNISH :
            return &id_fin[0];
            
        case LANGUAGE_NORWEGIAN :
        case LANGUAGE_DANISH :
        case LANGUAGE_SPANISH :
        case LANGUAGE_DUTCH :
        case LANGUAGE_PORTUGUESEB:
        case LANGUAGE_SWEDISH :
            return &id_int[0];
            
        case LANGUAGE_INDONESIAN :
            return &id_ind[0];
            
        default:
            return &id_eng[0];
    }
}

void GetSnnSizeInfo(const _INT lang, p_mlp_size_info info)
{
    switch(lang)
    {
        case LANGUAGE_FRENCH:
        case LANGUAGE_PORTUGUESE:
            info->output = MLP_NET_NUMOUTPUTS_FRN;
            break;
        
        case LANGUAGE_ITALIAN:
            info->output = MLP_NET_NUMOUTPUTS_ITL;
            break;
        
        case LANGUAGE_GERMAN:
            info->output = MLP_NET_NUMOUTPUTS_GER;
            break;
        
        case LANGUAGE_NORWEGIAN:
        case LANGUAGE_DANISH:
            info->output = MLP_NET_NUMOUTPUTS_DAN;
            break;
        
        case LANGUAGE_SPANISH :
        case LANGUAGE_DUTCH :
        case LANGUAGE_PORTUGUESEB :
        case LANGUAGE_SWEDISH :
            info->output = MLP_NET_NUMOUTPUTS_INT;
            break;
        
        case LANGUAGE_FINNISH:
            info->output = MLP_NET_NUMOUTPUTS_FIN;
            break;

        case LANGUAGE_INDONESIAN :
            info->output = MLP_NET_NUMOUTPUTS_IND;
            break;

        default:
            info->output = MLP_NET_NUMOUTPUTS_ENG;
            break;
    }
    info->layer_maxcells = info->output * MLP_CELL_MAXINPUTS;
    info->net_cells_0 = MLP_NET_NUMINPUTS;
    info->net_cells_1 = (MLP_COEF_SHARE+MLP_BMP_SHARE);
    info->net_cells_2 = (info->output*MLP_PREOUT_STEP + (MLP_CELL_MAXINPUTS-MLP_PREOUT_STEP));
    info->net_cells_3 = info->output;
    info->net_cells_4 = 0;
    info->net_cells = (info->net_cells_1+info->net_cells_2 + info->net_cells_3 + info->net_cells_4);
    info->net_signal = (info->net_cells_0 + info->net_cells_1 + info->net_cells_2 + info->net_cells_3 + info->net_cells_4);
}

const p_UCHAR GetSnnSymbol(_INT lang)
{
    static const p_UCHAR symbol_frn = (p_UCHAR)MLP_NET_SYMCO_FRN;
    static const p_UCHAR symbol_itl = (p_UCHAR)MLP_NET_SYMCO_ITL;
    static const p_UCHAR symbol_ger = (p_UCHAR)MLP_NET_SYMCO_GER;
    static const p_UCHAR symbol_fin = (p_UCHAR)MLP_NET_SYMCO_FIN;
    static const p_UCHAR symbol_int = (p_UCHAR)MLP_NET_SYMCO_INT;
    static const p_UCHAR symbol_eng = (p_UCHAR)MLP_NET_SYMCO_ENG;
    static const p_UCHAR symbol_dan = (p_UCHAR)MLP_NET_SYMCO_DAN;
    static const p_UCHAR symbol_ind = (p_UCHAR)MLP_NET_SYMCO_IND;
    
    switch(lang)
    {
        case LANGUAGE_FRENCH :
        case LANGUAGE_PORTUGUESE :
            return &symbol_frn[0];
            
        case LANGUAGE_ITALIAN :
            return &symbol_itl[0];
            
        case LANGUAGE_GERMAN :
            return &symbol_ger[0];
            
        case LANGUAGE_FINNISH :
            return &symbol_fin[0];
            
        case LANGUAGE_NORWEGIAN :
        case LANGUAGE_DANISH :
            return &symbol_dan[0];
            
        case LANGUAGE_SPANISH :
        case LANGUAGE_DUTCH :
        case LANGUAGE_PORTUGUESEB :
        case LANGUAGE_SWEDISH :
            return &symbol_int[0];
            
        case LANGUAGE_INDONESIAN :
            return &symbol_ind[0];

        default :
            return &symbol_eng[0];
    }
}

