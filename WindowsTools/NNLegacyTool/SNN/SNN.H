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

//#define PEGASUS

#define MLP_LEARN_MODE          0
#define MLP_EMULATE_INT         0
#define MLP_BYTE_NET            0

#ifdef SNN_FAT_NET_ON
#define MLP_FAT_NET            1
#else
#define MLP_FAT_NET            0
#endif

#ifdef PEGASUS
#ifndef MLP_PRELOAD_MODE
#define MLP_PRELOAD_MODE      1
#endif
#define MLP_UPSCALE            1
#define MLP_INT_MODE           1
#else
#ifndef MLP_PRELOAD_MODE
#define MLP_PRELOAD_MODE      1
#endif
#define MLP_UPSCALE            1
#define MLP_INT_MODE           1
#endif

#if MLP_FAT_NET
#define NET_TYPE_ID ".fat"
#else
#define NET_TYPE_ID ".slim"
#endif

// ------------------- Defines -----------------------------------------------

#ifdef FOR_FRENCH
#define MLP_ID_STR "MLP.Network.V.4.12.f" NET_TYPE_ID
#define MLP_NET_NUMOUTPUTS    (92+24)
#define MLP_NET_SYMCO "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"   \
                       "0123456789@#$%&!?*/+=-(){}[];:~\\<>^|"                  \
                       "£•  " /*"£•©Æ" until next net, which will do better on these*/ \
                       "‡‚ËÈÍÎÓÔÙ˘˚Á"                                           \
                       "¿¬»… ÀŒœ‘Ÿ€«"
#elif defined (FOR_GERMAN)
#define MLP_ID_STR "MLP.Network.V.4.12.g" NET_TYPE_ID
#define MLP_NET_NUMOUTPUTS    (92+8)
#define MLP_NET_SYMCO "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"   \
                       "0123456789@#$%&!?*/+=-(){}[];:~\\<>^|"                  \
                       "£•  " /*"£•©Æ" until next net, which will do better on these*/ \
                       "‰ˆ¸ﬂ"                                           \
                       "ƒ÷‹\""
#elif defined (FOR_INTERNATIONAL)
#define MLP_ID_STR "MLP.Network.V.4.12.i" NET_TYPE_ID
#define MLP_NET_NUMOUTPUTS    (92+60)
#define MLP_NET_SYMCO "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"   \
                       "0123456789@#$%&!?*/+=-(){}[];:~\\<>^|"                  \
                       "£•  " /*"£•©Æ" until next net, which will do better on these*/ \
                       "‡·‚„‰ÂÁËÈÍÎÏÌÓÔÒÚÛÙıˆ¯˘˙˚¸˝ˇöú"                         \
                       "¿¡¬√ƒ≈«»… ÀÃÕŒœ—“”‘’÷ÿŸ⁄€‹›üäå"
#else
#define MLP_ID_STR "MLP.Network.V.4.12.e" NET_TYPE_ID
#define MLP_NET_NUMOUTPUTS    92
#define MLP_NET_SYMCO "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"   \
                       "0123456789@#$%&!?*/+=-(){}[];:~\\<>^|"                  \
                       "£•  " /*"£•©Æ" until next net, which will do better on these*/
#endif

#define MLP_COEF_SHARE        96
#define MLP_BMP_SHARE         32
#define MLP_NUM_CFF           32
#define MLP_NUM_BMP         (16*16)

#define MLP_NET_NUMLAYERS      4
#define MLP_NET_NUMINPUTS    288

#define MLP_CELL_MAXINPUTS    32
#define MLP_LAYER_MAXCELLS   (MLP_NET_NUMOUTPUTS*MLP_CELL_MAXINPUTS)

#if MLP_FAT_NET
#define MLP_PREOUT_STEP       4
#else
#define MLP_PREOUT_STEP       2
#endif

#define MLP_NET_0L_NUMCELLS  (MLP_NET_NUMINPUTS)
#define MLP_NET_1L_NUMCELLS  (MLP_COEF_SHARE+MLP_BMP_SHARE)
#define MLP_NET_2L_NUMCELLS  (MLP_NET_NUMOUTPUTS*MLP_PREOUT_STEP + (MLP_CELL_MAXINPUTS-MLP_PREOUT_STEP))
#define MLP_NET_3L_NUMCELLS  (MLP_NET_NUMOUTPUTS)
#define MLP_NET_4L_NUMCELLS    0
#define MLP_NET_NUMCELLS     (MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS+MLP_NET_3L_NUMCELLS+MLP_NET_4L_NUMCELLS)
#define MLP_NET_NUMSIGNALS   (MLP_NET_0L_NUMCELLS+MLP_NET_1L_NUMCELLS+MLP_NET_2L_NUMCELLS+MLP_NET_3L_NUMCELLS+MLP_NET_4L_NUMCELLS)

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
	fint_s    signals[MLP_NET_NUMSIGNALS];
} mlp_data_type, * p_mlp_data_type;

// ------------------- Prototypes --------------------------------------------

_INT  InitSnnData(p_UCHAR name, p_mlp_data_type mlpd);
_INT  GetSnnResults(p_UCHAR pCoeff, p_UCHAR answs, p_mlp_data_type mlpd);

int   LoadNetData(p_mlp_data_type mlpd, p_CHAR net_name);
int   SaveNetData(p_mlp_data_type mlpd, p_CHAR net_name, float e);
int   DumpNetData(p_mlp_data_type mlpd, p_CHAR dmp_name, float e);

#endif // SNN_HEADER_INCLUDED
