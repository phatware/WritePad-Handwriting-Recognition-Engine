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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <iostream>
#include <math.h>

#pragma hdrstop

#include "mlp.h"
#include "main.h"

using namespace std;

/* **************************************************************************** */
/* **************************************************************************** */
/* *  Storage class member functions                                          * */
/* **************************************************************************** */
/* **************************************************************************** */

/* **************************************************************************** */
/* *        Storage class constructor                                         * */
/* **************************************************************************** */
samples_storage_class::samples_storage_class()
{
    num_samples  = 0;
    num_antisamp = 0;
    num_blocks   = 0;
    max_in_block = (MAIN_SIZE_BLOCK)/sizeof(sample_type);
    num_in_block = 0;
}

/* **************************************************************************** */
/* *        Storage class destructor                                          * */
/* **************************************************************************** */
samples_storage_class::~samples_storage_class()
{
    int i;
    
    for (i = 0; i < num_blocks; i ++)
    {
        free(blocks[i]);
    }
}

/* **************************************************************************** */
/* *        Add sample to storage                                             * */
/* **************************************************************************** */
int samples_storage_class::AddSample(p_sample_type sample)
{
    p_sample_type cur_block;
    
    if (num_samples >= num_blocks*max_in_block)
    {
        if ((blocks[num_blocks] = (p_sample_type)malloc(MAIN_SIZE_BLOCK)) == 0) throw 0x0004;
        memset(blocks[num_blocks], 0, MAIN_SIZE_BLOCK);
        num_in_block = 0;
        num_blocks ++;
    }
    
    cur_block = blocks[num_blocks-1];
    cur_block[num_in_block] = *sample;
    
    num_samples  ++;
    num_antisamp += sample->nsym - 1;
    num_in_block ++;
    
    return 0;
}

/* **************************************************************************** */
/* *        Get pointer to desired data element                               * */
/* **************************************************************************** */
int samples_storage_class::GetSample(int num, p_sample_type * pp_sample)
{
    p_sample_type cur_block;
    
    if (num > num_samples) throw 0x0006;
    
    cur_block  = blocks[num/max_in_block];
    *pp_sample = &cur_block[num%max_in_block];
    
    return 0;
}

/* **************************************************************************** */
/* *        Get flags   value                                                 * */
/* **************************************************************************** */
int samples_storage_class::GetSampleFlags(int num)
{
    p_sample_type p_sample;
    
    GetSample(num, &p_sample);
    
    return p_sample->flags;
}

/* **************************************************************************** */
/* *        Set flags   value                                                 * */
/* **************************************************************************** */
int samples_storage_class::SetSampleFlags(int num, uchar flags)
{
    p_sample_type p_sample;
    
    GetSample(num, &p_sample);
    
    p_sample->flags = flags;
    
    return p_sample->flags;
}

/* **************************************************************************** */
/* *        Set flags   value                                                 * */
/* **************************************************************************** */
int samples_storage_class::ResetSampleFlags(uchar fl)
{
    int   i, j;
    p_sample_type psamp;
    
    for (i = 0; i < num_blocks; i ++)
    {
        for (j = 0, psamp = blocks[i]; j < max_in_block; j ++, psamp ++)
        {
            psamp->flags = fl;
        }
    }
    
    return 0;
}

/* **************************************************************************** */
/* *           Load samples                                                   * */
/* **************************************************************************** */

int LoadSamples(char (*dat_name)[MAX_DATA_FILES][MAIN_FILENAME_LEN], samples_storage_class * samples, _INT langID )
{
    int    i, j, k, l, z, n, nf;
    int    state, len, weight, cw;
    FILE * dat_file;
    char   str[1024];
    int    m[MAIN_SIZE_SAMPLE];
    ulong  b[MAIN_SIZE_BMAP];
    int    w[128];
    uchar  s[128];
    uchar  v[128];
    uchar  sym, nvar, correct;
    p_UCHAR   allowed_sym = GetSnnSymbol( langID );
    sample_type sample;
    //p_sample_type samp;
    
    for (nf = 0; nf < MAX_DATA_FILES && (*dat_name)[nf][0] != 0; nf ++)
    {
        if ((dat_file=fopen((*dat_name)[nf],"rt")) == 0) goto err;
        
        cout << "Loading " << (*dat_name)[nf] << " data file..." << endl;
        
        fgets(str, 1024, dat_file);
        if (strncmp(str, SNN_DATA_VER, strlen(SNN_DATA_VER)) != 0)
        {
            cout << endl << "Wrong type of data file: " << (*dat_name)[nf] << endl;
            goto err;
        }
        
        for (n = 0, state = 0; !feof(dat_file); n ++)
        {
            str[0] = 0;
            fgets(str, 1024, dat_file);
            len = (int)strlen(str);
            
            if (state == 0 && str[0] == '*')
            {
                if (sscanf(str+2, "%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d  %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d   %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu %lu",
                           &m[0], &m[1],   &m[2],  &m[3],  &m[4],  &m[5],  &m[6],  &m[7],
                           &m[8], &m[9],  &m[10], &m[11], &m[12], &m[13], &m[14], &m[15],
                           &m[16],&m[17], &m[18], &m[19], &m[20], &m[21], &m[22], &m[23],
                           &m[24],&m[25], &m[26], &m[27], &m[28], &m[29], &m[30], &m[31],
                           &b[0], &b[1],   &b[2],  &b[3],  &b[4],  &b[5],  &b[6],  &b[7],
                           &b[8], &b[9],  &b[10], &b[11], &b[12], &b[13], &b[14], &b[15]
                           ) != MAIN_SIZE_SAMPLE + MAIN_SIZE_IBMAP)
                {cout << "Error 1 reading data file, line " << (n+1); goto err;}
                
                memset((void*)&sample, 0, sizeof(sample));
                memcpy(sample.bmap, b, sizeof(sample.bmap));
                
                //        j = (m[0]-1) * 64; if (j < 0) j = 0; if (j > 255) j = 255;
                //        sample.data[0] = (_UCHAR)j;
                //        sample.data[1] = (_UCHAR)(m[1] * 64);
                
                for (j = 0; j < MAIN_SIZE_SAMPLE; j ++) sample.data[j] = m[j];
                //      for (j = 2; j < MAIN_SIZE_SAMPLE; j ++)
                //       {
                //        m[j] += 128;
                //        if (m[j] < 0) m[j] = 0;
                //        if (m[j] > 255) m[j] = 255;
                //        sample.data[j] = (uchar)m[j];
                //       }
                
                state = 1;
                continue;
            }  // '*' record read
            
            if (state == 1) // Lets read letter/variants string
            {
                i = j = 0;
                //        for (i = 0, j = 0; i < 100 && i < len/6; i ++) ,_ now we have only ONE variant possible!!
                {
                    //          if (sscanf(str+i*6, "%c%c%c%d",  &sym, &nvar, &correct, &weight) == 4)
                    if (strlen(str) >= 4)
                    {
                        sym = str[0];
                        nvar = str[1];
                        correct = str[2];
                        sscanf(str+3, "%d", &weight);
                        
                        //          if (sym >= 128+16) sym = 128+15;
                        if (nvar > '9') nvar = (uchar)(nvar - 'a' + 10); else nvar -= '0'; if (nvar > 15) nvar = 15;
                        correct -= '0'; if (correct != 0)  correct = 1;
                        
                        if (correct || j == 0)
                        {
                            cw = weight;
                            sample.syms[0]  = (uchar)(sym);
                            sample.nvs[0]   = (uchar)(nvar << 4);
                        }
                        else
                        {
                            s[j] = sym;
                            w[j] = weight;
                            v[j] = nvar;
                            j ++;
                        }
                    }
                    else
                    {
                        {cout << "Error 2 reading data file, line " << (n+1); goto err;}
                    }
                }
                
                if (sample.syms[0] == 0)
                {cout << "Error 3 reading data file, line " << (n+1); goto err;}
                
                //      if (strchr(allowed_sym, sample.syms[0]))
                {
                    for (k = z = 1; k < MAIN_NUMSYMINSAMP; k ++)
                    {
                        l = weight = -100;    // Take best weighted syms and order them by weight
                        for (i = 0; i < j; i ++) if (w[i] > weight) {weight = w[i]; l = i;}
                        
                        if (l < 0) break; // NOthing left
                        //        if (weight < cw - 10) break; // What's left is too bad
                        //        if (!(isalpha(sample.syms[0]) && tolower(sample.syms[0]) == tolower(s[l])))
                        if ( strchr( (char *)allowed_sym, (int)s[l] ) )
                        {
                            sample.syms[z]   = s[l];
                            sample.nvs[z/2] |= (uchar)(v[l] << ((z%2) ? 0:4));
                            z ++;
                        }
                        
                        w[l] = -100;
                    }
                    
                    sample.flags = 0;
                    sample.nsym  = (uchar)z;
                    if (samples->AddSample(&sample)) goto err;
                }
                
                state = 0;
                continue;
            }
        }
        
        cout << samples;
    }
    
    
    return 0;
err:
    return 1;
}

/* **************************************************************************** */
/* *        Get inputs from sample                                            * */
/* **************************************************************************** */
int GetInputs(int num, uchar *syms, uchar * inps, samples_storage_class * samples)
{
    int k;
    p_sample_type psamp;
    
    samples->GetSample(num, &psamp);
    
    for (k = 0; k < MAIN_SIZE_SAMPLE; k ++)
        inps[k] = psamp->data[k];
    for (k = 0; k < MAIN_SIZE_BMAP; k ++)
        inps[k+MAIN_SIZE_SAMPLE] = (uchar)(((psamp->bmap[k/16] >> ((k%16)*2)) & 3) << 6);
    
    //  k = (psamp->data[14]-1) * 64; if (k < 0) k = 0; if (k > 255) k = 255;
    //  inps[14] = (uchar)k;
    //  inps[15] = (uchar)(psamp->data[15] * 64);
    
    for(k = 0; k < MAIN_NUMSYMINSAMP; k ++)
        syms[k] = psamp->syms[k];
    
    return psamp->flags;
}

/* **************************************************************************** */
/* *        Get inputs from sample                                            * */
/* **************************************************************************** */
int GetInputsAtRandom(int flags, uchar *what, uchar * inps, uchar * syms, int * ns, samples_storage_class * samples)
{
    int    k;
    int    r, n, rn, lim;
    int    num = 0, nsym;
    int    found;
    //  uchar  sym;
    uchar  sms[MAIN_NUMSYMINSAMP];
    p_sample_type psamp = _NULL;
    static int symlog[256][16][2];
    static int norm_sym = 0;
    
    *what = 0;
    
    if ((flags & 0x01) == 0) // Init random get
    {
        samples->ResetSampleFlags(0);
        norm_sym = *ns;
        //    d = norm_symnum/10; if (d < 1) d = 1;
        
        if (flags == 0)
        {
            samples->counter = 0;
            
            { // Debug testing
                int i, j;
                
                n = r = 0;
                for (i = 0; i < 256; i ++)
                    for (j = 0; j < 16; j ++) {n += symlog[i][j][0]; r += symlog[i][j][1];}
                
                symlog[0][0][0] = n;
                symlog[0][0][1] = r;
            }
            
            memset(symlog, 0, sizeof(symlog));
            
            // Statistical precut of sample buffer
            for (k = 0; k < samples->num_samples; k ++)
            {
                samples->GetSample(k, &psamp);
                if ( psamp != _NULL )
                {
                    for (nsym = 0; nsym < psamp->nsym; nsym ++)
                    {
                        r = (nsym == 0) ? 0:1;
                        n = (nsym % 2) ? psamp->nvs[nsym/2] & 0x0f : psamp->nvs[nsym/2] >> 4;
                        symlog[psamp->syms[nsym]][n][r] ++;
                    }
                }
            }
            
            lim = norm_sym*norm_sym;
            
            for (k = 32; k < 256; k ++)
            {
                for (n = 0; n < 16; n ++)
                {
                    r  = symlog[k][n][0];
                    if (r < lim) {symlog[k][n][0] = 10000; rn = r; if (rn < 10) rn = 10;}
                    else
                    {
                        rn = (int)(norm_sym * sqrt(r+2));
                        if (r <= rn) symlog[k][n][0] = 10000;
                        else symlog[k][n][0] = (10000 * rn) / r;
                    }
                    
                    r = symlog[k][n][1];
                    if (r < rn) symlog[k][n][1] = 10000;
                    else symlog[k][n][1] = (10000 * rn) / r;
                    
                    //          r  = symlog[k][n][1];
                    //          if (r == 0) symlog[k][n][1] = 10000;
                    //           {
                    //            rn = (int)(norm_sym * sqrt(r+2));
                    //            if (r <= rn) symlog[k][n][1] = 10000;
                    //             else symlog[k][n][1] = (10000 * rn) / r;
                    //           }
                    
                    
                    //          r  = symlog[k][n][0];
                    //          rn = (int)(norm_sym * sqrt(r+2));
                    //          if (r < rn)
                    //           { // Do not allow too many bad samples if there not enough good
                    //            if (r < rn/10) r = rn/10;
                    //            if (r < symlog[k][n][1]) symlog[k][n][1] = (int)((symlog[k][n][1] * rn)/r);
                    //            r = rn; // And leave all correct ones alive
                    //           }
                    //          symlog[k][n][0] = (10000 * rn) / r;
                    //          r = symlog[k][n][1]; if (r < rn) r = rn;
                    //          symlog[k][n][1] = (10000 * rn) / r;
                }
            }
            
            for (k = 0; k < samples->num_samples; k ++)
            {
                samples->GetSample(k, &psamp);
                if ( psamp == _NULL )
                    continue;
                for (nsym = 0; nsym < psamp->nsym; nsym ++)
                {
                    r = rand() % 10000;
                    n = (nsym % 2) ? psamp->nvs[nsym/2] & 0x0f : psamp->nvs[nsym/2] >> 4;
                    if (r > symlog[psamp->syms[nsym]][n][(nsym == 0) ? 0:1])
                        psamp->flags |= (uchar)(0x01 << nsym);
                }
            }
        } // End of mode == 0 if
        
        samples->counter = 0;
        memset(symlog, 0, sizeof(symlog));
    }
    
    if (flags == 1) // Let's get sample
    {
        if (samples->counter >= samples->num_samples) goto err;
        
        while (samples->counter < samples->num_samples) // Mark as 'used' too frequent letters
        {
            for (k = 0, found = 0; k < 100 && !found; k ++) // Try n times to get random
            {
                r   = rand() * rand();
                num = samples->counter + (r % (samples->num_samples - samples->counter));
                samples->GetSample(num, &psamp);
                if ( psamp == _NULL )
                    continue;
#if TAKE_BY_FIRST
                if ((psamp->flags & 0x01) == 0)
                {
                    found = 1;
                    break;
                }
#else
                for (n = 0; n < psamp->nsym; n ++)
                {
                    if ((psamp->flags & (0x01 << n)) == 0)
                    {
                        found = 1;
                        break;
                    }
                }
#endif
            }
            
            if (!found)  // Sample taken?
            {
                for (; !found && samples->counter < samples->num_samples; samples->counter ++)
                {
                    samples->GetSample(samples->counter, &psamp);
                    
#if TAKE_BY_FIRST
                    if ((psamp->flags & 0x01) == 0) {found = 1; break;}
#else
                    for (n = 0; n < psamp->nsym; n ++)
                    {
                        if ((psamp->flags & (0x01 << n)) == 0)
                        {
                            found = 1;
                            break;
                        }
                    }
#endif
                }
                
                num = samples->counter;
            }
            
            *ns = num;
            
            GetInputs(num, sms, inps, samples);
            
#if TAKE_BY_FIRST
            for (k = 0; k < psamp->nsym; k ++)
            {
                syms[k] = sms[k];
                r = (k % 2) ? psamp->nvs[k/2] & 0x0f : psamp->nvs[k/2] >> 4;
                symlog[psamp->syms[k]][r][1] ++; // Just for debug and control ...
            }
#else
            if ( ! (psamp->flags & 0x01) )
            {
                syms[0] = sms[0];
                r = psamp->nvs[0] >> 4;
                symlog[psamp->syms[0]][r][0] ++; // Just for debug and control ...
            }
            else syms[0] = ' '; // Mark as invalid
            
            for (k = 1, n = 1; k < psamp->nsym; k ++)
            {
                if (!(psamp->flags & (0x01 << k)))
                {
                    syms[n++] = sms[k];
                    
                    r = (k % 2) ? psamp->nvs[k/2] & 0x0f : psamp->nvs[k/2] >> 4;
                    symlog[psamp->syms[k]][r][1] ++; // Just for debug and control ...
                }
            }
#endif

            syms[n] = 0;
            psamp->flags = (uchar)(0xFF);
            break;
        }
        
        RockCoeffs(0, inps);
        //    for (k = 0; k < 8; k ++)  inps[k+MAIN_SIZE_SAMPLE] = ((sym & (0x01 << k)) == 0) ? 0.1 : 0.9;
        *what |= 0x03;
    }
    
    return 0;
err:
    return 1;
}

/* **************************************************************************** */
/* *        Get inputs from sample                                            * */
/* **************************************************************************** */
int GetInputsSequencial(int flags, uchar *what, uchar * inps, uchar * syms, int * ns, samples_storage_class * samples)
{
    //  int    i, k;
    //  uchar  sym;
    //  uchar  syms[MAIN_NUMSYMINSAMP];
    static int cur_samp = 0;
    
    *what = 0;
    
    if ((flags & 0x01) == 0) // Init random get
    {
        samples->ResetSampleFlags(0);
        samples->counter = 0;
        cur_samp = 0;
    }
    else // Let's get sample
    {
        if (samples->counter >= samples->num_samples) goto err;
        
        GetInputs(samples->counter, syms, inps, samples);
        
        //    sym = syms[cur_samp];
        //    for (k = 0; k < 8; k ++)  inps[k+MAIN_SIZE_SAMPLE] = ((sym & (0x01 << k)) == 0) ? 0.1 : 0.9;
        
        *what = 0x03;
        
        //    strncpy(syms, psamp->syms, MAIN_NUMSYMINSAMP);
        syms[MAIN_NUMSYMINSAMP] = 0;
        
        *ns = samples->counter++;
        
        //    cur_samp ++;
        //    for (k = 0; k < MAIN_NUMSYMINSAMP && syms[k]; k ++);
        //    if (cur_samp >= k) {cur_samp = 0; samples->counter ++;}
    }
    
    return 0;
err:
    return 1;
}

/* **************************************************************************** */
/* *        Change around the coeffs                                          * */
/* **************************************************************************** */

int RockCoeffs(int mode, uchar * inps)
{
    int   i;
    float r, f;
    float k1, k2;
    float x, y;
    
    
    if (mode == 0)
        //  if (mode == -1)
    {                           // Look lower before const change!!!
        f = ((float)(rand() % 10000) * 0.0001f * 0.2f) + 0.9f;
        r = ((float)(rand() % 10000) * 0.0001f * 0.2f) - 0.1f;
        
        
        k1 = 1 - r*r/2;
        k2 = r - r*r*r/6;
        
        for (i = 5; i < MAIN_SIZE_SAMPLE-1; i ++)
        {
            if (((i - 4) % 3) == 0) 
				continue; // Do not touch pressure
            
            r = ((i % 3) == 0) ? ((float)inps[i]-128.0f)*f + 128.0f : ((float)inps[i]-128.0f)*(2.1f-f) + 128.0f; // compress/expand
            if (r <= 255 && r >= 0)
				inps[i] = (_UCHAR)(r);
			else 
				inps[i] = (uchar)((r > 255) ? 255 : 0);
            
            if ((i % 3) == 0)   // Rotate
            {
                x = inps[i-1]-128.0f;
                y = inps[i+0]-128.0f;
                r = x*k1 - y*k2 + 128; 
				if (r < 0)
					r = 0;
				if (r > 255)
					r = 255;
                inps[i-1] = (uchar)r;
                r = x*k2 + y*k1 + 128;
				if (r < 0)
					r = 0; 
				if (r > 255)
					r = 255;
                inps[i+0] = (uchar)r;
            }
        }
    }
    
    return 0;
}

/* **************************************************************************** */
/* *         Dump cell to given stream                                        * */
/* **************************************************************************** */

ostream& operator << (ostream& os, samples_storage_class *stor)
{
    os << "DataStor report:"   << endl;
    os << "Num samples:      " << stor->num_samples  << endl;
    os << "Num antisamples:  " << stor->num_antisamp << endl;
    os << "Num blocks:       " << stor->num_blocks   << endl;
    os << "Memory allocated: " << stor->num_blocks * MAIN_SIZE_BLOCK << endl;
    os << endl;
    
    return os;
}


