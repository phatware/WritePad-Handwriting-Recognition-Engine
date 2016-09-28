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

#ifndef SNN_HEADER_INCLUDED
#define SNN_HEADER_INCLUDED

#include "ams_mg.h"

// ------------------- Defines -----------------------------------------------

//#define _EMBEDDED_DEVICE

// #define MLP_LEARN_MODE         0
#define MLP_EMULATE_INT        0
#define MLP_BYTE_NET           0

#ifdef SNN_FAT_NET_ON
#define MLP_FAT_NET            1
#else
#define MLP_FAT_NET            0
#endif

#ifdef _EMBEDDED_DEVICE
#ifndef MLP_PRELOAD_MODE
#define MLP_PRELOAD_MODE       1
#endif
#define MLP_UPSCALE            1
#define MLP_INT_MODE           1
#else
#ifndef MLP_PRELOAD_MODE
#define MLP_PRELOAD_MODE       1
#endif
#define MLP_UPSCALE            1
#define MLP_INT_MODE           0
#endif

#if MLP_FAT_NET
#define NET_TYPE_ID "_full"
#else
#define NET_TYPE_ID "_comp"
#endif

// ------------------- Defines -----------------------------------------------
#define MLP_NET_MAX_NUMOUTPUTS (92+60)

#define MLP_ID_STR_FRN "HWNet_FRN_9.0" NET_TYPE_ID
#define MLP_NET_NUMOUTPUTS_FRN    (92+24)
#define MLP_NET_SYMCO_FRN "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ" \
                   "0123456789 #$%&!?*/+=-(){}[];:~\\<>^|" \
                   "\xa3\xa5  " \
                   "\xe0\xe2\xe8\xe9\xea\xeb\xee\xef\xf4\xf9\xfb\xe7" \
                   "\xc0\xc2\xc8\xc9\xca\xcb\xce\xcf\xd4\xd9\xdb\xc7"
#define MLP_ID_STR_ITL "HWNet_ITL_9.0" NET_TYPE_ID
#define MLP_NET_NUMOUTPUTS_ITL    (92+24)
#define MLP_NET_SYMCO_ITL "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"   \
                       "0123456789@#$%&!?*/+=-(){}[];:~\\<>^|"                  \
                   "\xa3\xa5  " \
                   "\xe0\xe2\xe8\xe9\xea\xeb\xee\xef\xf4\xf9\xfb\xe7" \
                   "\xc0\xc2\xc8\xc9\xca\xcb\xce\xcf\xd4\xd9\xdb\xc7"
#define MLP_ID_STR_GER "HWNet_GER_9.0" NET_TYPE_ID
#define MLP_NET_NUMOUTPUTS_GER    (92+8)
#define MLP_NET_SYMCO_GER "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"   \
                       "0123456789@#$%&!?*/+=-(){}[];:~\\<>^|"                  \
                   "\xa3\xa5  " \
                   "\xe4\xf6\xfc\xdf" \
                   "\xc4\xd6\xdc\""
#define MLP_ID_STR_FIN "HWNet_FIN_9" NET_TYPE_ID
#define MLP_NET_NUMOUTPUTS_FIN    (92+60)
#define MLP_NET_SYMCO_FIN "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"   \
                   "0123456789@#$%&!?*/+=-(){}[];:~\\<>^|"                  \
                   "\xa3\xa5  " \
                   "\xe0\xe1\xe2\xe3\x20\x20\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf1\xf2\xf3\xf4\xf5\xf6\xf8\xf9\xfa\xfb\xfc\xfd\xff" \
                   "\xc0\xc1\xc2\xc3\x20\x20\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd1\xd2\xd3\xd4\xd5\xd6\xd8\xd9\xda\xdb\xdc\xdd"
#define MLP_ID_STR_INT "HWNet_DAN_9" NET_TYPE_ID
#define MLP_NET_NUMOUTPUTS_DAN    (92+60)
#define MLP_NET_SYMCO_DAN "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"   \
                   "0123456789@#$%&!?*/+=-(){}[];:~\\<>^|"                  \
                   "\xa3\xa5  " \
                   "\xe0\xe1\xe2\xe3\xe4\x20\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf1\xf2\xf3\xf4\xf5\xf6\xf8\xf9\xfa\xfb\xfc\xfd\xff" \
                   "\xc0\xc1\xc2\xc3\xc4\x20\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd1\xd2\xd3\xd4\xd5\xd6\xd8\xd9\xda\xdb\xdc\xdd"

#define MLP_NET_NUMOUTPUTS_INT    (92+60)
#define MLP_NET_SYMCO_INT "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"   \
                       "0123456789@#$%&!?*/+=-(){}[];:~\\<>^|"                  \
                   "\xa3\xa5  " \
                   "\xe0\xe1\xe2\xe3\xe4\xe5\xe7\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf1\xf2\xf3\xf4\xf5\xf6\xf8\xf9\xfa\xfb\xfc\xfd\xff" \
                   "\xc0\xc1\xc2\xc3\xc4\xc5\xc7\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf\xd1\xd2\xd3\xd4\xd5\xd6\xd8\xd9\xda\xdb\xdc\xdd"

#define MLP_ID_STR_ENG          "HWNet_ENG_9.0" NET_TYPE_ID
#define MLP_NET_NUMOUTPUTS_ENG  92
#define MLP_NET_SYMCO_ENG       "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"   \
                                "0123456789@#$%&!?*/+=-(){}[];:~\\<>^|"                  \
                                "\xa3\xa5  "

#define MLP_ID_STR_IND          "HWNet_IND_9.0" NET_TYPE_ID
#define MLP_NET_NUMOUTPUTS_IND  92
#define MLP_NET_SYMCO_IND       "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"   \
                                "0123456789@#$%&!?*/+=-(){}[];:~\\<>^|"                  \
                                "\xa3\xa5  "

#define MLP_COEF_SHARE          96
#define MLP_BMP_SHARE           32
#define MLP_NUM_CFF             32
#define MLP_NUM_BMP             (16*16)

#define MLP_NET_NUMLAYERS       4
#define MLP_NET_NUMINPUTS       288

#define MLP_CELL_MAXINPUTS      32

#if MLP_FAT_NET
#define MLP_PREOUT_STEP         4
#else
#define MLP_PREOUT_STEP         2
#endif

#define MLP_NET_0L_NUMCELLS      (MLP_NET_NUMINPUTS)
#define MLP_NET_1L_MAX_NUMCELLS  (MLP_COEF_SHARE+MLP_BMP_SHARE)
#define MLP_NET_2L_MAX_NUMCELLS  (MLP_NET_MAX_NUMOUTPUTS*MLP_PREOUT_STEP + (MLP_CELL_MAXINPUTS-MLP_PREOUT_STEP))
#define MLP_NET_3L_MAX_NUMCELLS  (MLP_NET_MAX_NUMOUTPUTS)
#define MLP_NET_4L_MAX_NUMCELLS  0
#define MLP_NET_MAX_NUMCELLS     (MLP_NET_1L_MAX_NUMCELLS+MLP_NET_2L_MAX_NUMCELLS+MLP_NET_3L_MAX_NUMCELLS+MLP_NET_4L_MAX_NUMCELLS)
#define MLP_NET_MAX_NUMSIGNALS   (MLP_NET_0L_NUMCELLS+MLP_NET_1L_MAX_NUMCELLS+MLP_NET_2L_MAX_NUMCELLS+MLP_NET_3L_MAX_NUMCELLS+MLP_NET_4L_MAX_NUMCELLS)

typedef struct
{
    int output; // MLP_NET_NUMOUTPUTS
    int layer_maxcells; // MLP_LAYER_MAXCELLS
    int net_cells_0;
    int net_cells_1;
    int net_cells_2;
    int net_cells_3;
    int net_cells_4;
    int net_cells;
    int net_signal;
} mlp_size_info, * p_mlp_size_info;

//#define MLP_NET_SYMCO_LONG "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789@#$%&*{}"

// ------------------- Structures --------------------------------------------

#if MLP_INT_MODE
#if MLP_BYTE_NET
typedef _SCHAR         fint_c;
typedef _UCHAR         fint_s;
#else
typedef _SHORT         fint_c;
typedef _SHORT         fint_s;
#endif
typedef _LONG          flong;
typedef fint_c *       p_fint_c;
typedef fint_s *       p_fint_s;
#else
typedef float          fint_c;
typedef float          fint_s;
typedef float          flong;
typedef fint_c *       p_fint_c;
typedef fint_s *       p_fint_s;
#endif

typedef struct
{
    p_VOID    net;
    fint_s    signals[MLP_NET_MAX_NUMSIGNALS];
} mlp_data_type, * p_mlp_data_type;

// ------------------- Prototypes --------------------------------------------

_INT  InitSnnData(p_UCHAR name, p_mlp_data_type mlpd, _INT lang );
_INT  GetSnnResults(p_UCHAR pCoeff, p_UCHAR answs, p_mlp_data_type mlpd, _INT lang );

int   LoadNetData(p_mlp_data_type mlpd, p_CHAR net_name, _INT lang);
int   SaveNetData(p_mlp_data_type mlpd, p_CHAR net_name, float e, _INT lang);
int   DumpNetData(p_mlp_data_type mlpd, p_CHAR dmp_name, float e, _INT lang);

const p_UCHAR GetSnnID(const _INT lang);
void GetSnnSizeInfo(const _INT lang, p_mlp_size_info info);
const p_UCHAR GetSnnSymbol(_INT lang);

#endif // SNN_HEADER_INCLUDED


