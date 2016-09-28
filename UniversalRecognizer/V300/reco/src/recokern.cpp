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

#include "bastypes.h"
#include "hwr_sys.h"
#include "ams_mg.h"
#include "lowlevel.h"

#include "bit_mark.h"

// A bunch of constants are declared in Glob.h
// I removed the include of this from init_grf

#include "glob.h"

// From hwr_math.c
RECO_DATA _UCHAR SQRTa[256] =
{
    0,  17,  23,  28,  32,  36,  40,  43,  46,  48,  51,  54,
    56,  58,  60,  62,  64,  66,  68,  70,  72,  74,  76,  77,
    79,  80,  82,  84,  85,  87,  88,  90,  91,  92,  94,  95,
    96,  98,  99, 100, 102, 103, 104, 105, 107, 108, 109, 110,
    111, 112, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123,
    124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135,
    136, 137, 138, 139, 140, 141, 142, 143, 144, 144, 145, 146,
    147, 148, 149, 150, 151, 151, 152, 153, 154, 155, 156, 156,
    157, 158, 159, 160, 160, 161, 162, 163, 164, 164, 165, 166,
    167, 168, 168, 169, 170, 171, 171, 172, 173, 174, 174, 175,
    176, 176, 177, 178, 179, 179, 180, 181, 182, 182, 183, 184,
    184, 185, 186, 186, 187, 188, 188, 189, 190, 190, 191, 192,
    192, 193, 194, 194, 195, 196, 196, 197, 198, 198, 199, 200,
    200, 201, 202, 202, 203, 204, 204, 205, 205, 206, 207, 207,
    208, 208, 209, 210, 210, 211, 212, 212, 213, 213, 214, 215,
    215, 216, 216, 217, 218, 218, 219, 219, 220, 220, 221, 222,
    222, 223, 223, 224, 224, 225, 226, 226, 227, 227, 228, 228,
    229, 230, 230, 231, 231, 232, 232, 233, 233, 234, 235, 235,
    236, 236, 237, 237, 238, 238, 239, 239, 240, 240, 241, 242,
    242, 243, 243, 244, 244, 245, 245, 246, 246, 247, 247, 248,
    248, 249, 249, 250, 250, 250, 250, 250, 250, 250, 250, 250,
    250, 250, 250, 250
};

RECO_DATA _LONG SQRTb[267] =
{
    0,     1,     4,     9,    16,    25,    36,    49,    64,    81,
    100,   121,   144,   169,   196,   225,   256,   289,   324,   361,
    400,   441,   484,   529,   576,   625,   676,   729,   784,   841,
    900,   961,  1024,  1089,  1156,  1225,  1296,  1369,  1444,  1521,
    1600,  1681,  1764,  1849,  1936,  2025,  2116,  2209,  2304,  2401,
    2500,  2601,  2704,  2809,  2916,  3025,  3136,  3249,  3364,  3481,
    3600,  3721,  3844,  3969,  4096,  4225,  4356,  4489,  4624,  4761,
    4900,  5041,  5184,  5329,  5476,  5625,  5776,  5929,  6084,  6241,
    6400,  6561,  6724,  6889,  7056,  7225,  7396,  7569,  7744,  7921,
    8100,  8281,  8464,  8649,  8836,  9025,  9216,  9409,  9604,  9801,
    10000, 10201, 10404, 10609, 10816, 11025, 11236, 11449, 11664, 11881,
    12100, 12321, 12544, 12769, 12996, 13225, 13456, 13689, 13924, 14161,
    14400, 14641, 14884, 15129, 15376, 15625, 15876, 16129, 16384, 16641,
    16900, 17161, 17424, 17689, 17956, 18225, 18496, 18769, 19044, 19321,
    19600, 19881, 20164, 20449, 20736, 21025, 21316, 21609, 21904, 22201,
    22500, 22801, 23104, 23409, 23716, 24025, 24336, 24649, 24964, 25281,
    25600, 25921, 26244, 26569, 26896, 27225, 27556, 27889, 28224, 28561,
    28900, 29241, 29584, 29929, 30276, 30625, 30976, 31329, 31684, 32041,
    32400, 32761U, 33124U, 33489U, 33856U, 34225U, 34596U, 34969U, 35344U, 35721U,
    36100U, 36481U, 36864U, 37249U, 37636U, 38025U, 38416U, 38809U, 39204U, 39601U,
    40000U, 40401U, 40804U, 41209U, 41616U, 42025U, 42436U, 42849U, 43264U, 43681U,
    44100U, 44521U, 44944U, 45369U, 45796U, 46225U, 46656U, 47089U, 47524U, 47961U,
    48400U, 48841U, 49284U, 49729U, 50176U, 50625U, 51076U, 51529U, 51984U, 52441U,
    52900U, 53361U, 53824U, 54289U, 54756U, 55225U, 55696U, 56169U, 56644U, 57121U,
    57600U, 58081U, 58564U, 59049U, 59536U, 60025U, 60516U, 61009U, 61504U, 62001U,
    62500U, 63001U, 63504U, 64009U, 64516U, 65025U, 65535U
};

// from PICT.C

  #define    REL_HIGHT_QANT         11
  #define    AF                     ALEF

 /* First index is highest , second index is lowest .                      */

  RECO_DATA _SCHAR maxA_H_end[REL_HIGHT_QANT-1][REL_HIGHT_QANT-1] =

 /*   H \ L    _D4_  _D3_  _D2_  _D1_  _DM_  _UM_  _U1_  _U2_  _U3_  _U4_  */
            {
 /* _D4_ */   { 55 ,  -2 , -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D3_ */   { 55 ,  55 , -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D2_ */   { 55 ,  55 , 55 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D1_ */   { 55 ,  55 , 55 ,  45 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _DM_ */   { 45 ,  45 , 55 ,  45 ,  50 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _UM_ */   { 45 ,  45 , 45 ,  40 ,  50 ,  55 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _U1_ */   { 25 ,  45 , 45 ,  40 ,  45 ,  58 ,  60 ,  -2 ,  -2 ,  -2 } ,
 /* _U2_ */   { 25 ,  25 , 45 ,  45 ,  45 ,  55 ,  65 ,  68 ,  -2 ,  -2 } ,
 /* _U3_ */   { 22 ,  25 , 25 ,  55 ,  45 ,  45 ,  68 ,  73 ,  73 ,  -2 } ,
 /* _U4_ */   { 22 ,  22 , 25 ,  25 ,  45 ,  45 ,  45 ,  73 ,  75 ,  75 }
            } ;

 /**************************************************************************/

 /* First index is highest , second index is lowest .                      */


  RECO_DATA _SHORT maxCR_H_end[REL_HIGHT_QANT-1][REL_HIGHT_QANT-1] =

 /*   H \ L    _D4_  _D3_  _D2_  _D1_  _DM_  _UM_  _U1_  _U2_  _U3_  _U4_  */
            {
 /* _D4_ */    { 20 ,  -2 , -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D3_ */    { 20 ,  20 , -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D2_ */    { 20 ,  20 , 20 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D1_ */    { 18 ,  20 , 20 ,  20 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _DM_ */    { 15 ,  16 , 20 ,  20 ,  20 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _UM_ */    {  8 ,  10 , 12 ,  16 ,  18 ,  20 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _U1_ */    {  4 ,   5 ,  6 ,  10 ,  14 ,  18 ,  20 ,  -2 ,  -2 ,  -2 } ,
 /* _U2_ */    {-AF ,   2 ,  3 ,   5 ,   8 ,  14 ,  20 ,  20 ,  -2 ,  -2 } ,
 /* _U3_ */    {-AF , -AF ,  1 ,   3 ,   6 ,  12 ,  18 ,  20 ,  24 ,  -2 } ,
 /* _U4_ */    {-AF , -AF ,-AF , -AF ,   4 ,  10 ,  16 ,  20 ,  22 ,  24 }
            } ;

 /**************************************************************************/

 /* First index is highest , second index is lowest .                      */

 RECO_DATA  _SCHAR minL_H_end[REL_HIGHT_QANT-1][REL_HIGHT_QANT-1] =

 /*   H \ L    _D4_  _D3_  _D2_  _D1_  _DM_  _UM_  _U1_  _U2_  _U3_  _U4_  */
            {
 /* _D4_ */   { 28 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D3_ */   { 28 ,  28 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D2_ */   { 28 ,  28 ,  28 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D1_ */   { 28 ,  28 ,  28 ,  25 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _DM_ */   { 15 ,  15 ,  15 ,  15 ,  15 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _UM_ */   { 15 ,  15 ,  15 ,  15 ,  12 ,  12 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _U1_ */   { 15 ,  15 ,  15 ,  15 ,  15 ,  12 ,  12 ,  -2 ,  -2 ,  -2 } ,
 /* _U2_ */   { 25 ,  25 ,  15 ,  15 ,  15 ,  15 ,  15 ,  15 ,  -2 ,  -2 } ,
 /* _U3_ */   { 22 ,  25 ,  25 ,  15 ,  15 ,  15 ,  15 ,  25 ,  25 ,  -2 } ,
 /* _U4_ */   { 22 ,  22 ,  25 ,  25 ,  15 ,  15 ,  15 ,  25 ,  32 ,  32 }
            } ;

 /**************************************************************************/

 /* First index is highest , second index is lowest .                      */

 RECO_DATA _SCHAR maxX_H_end[REL_HIGHT_QANT-1][REL_HIGHT_QANT-1] =

 /*   H \ L    _D4_  _D3_  _D2_  _D1_  _DM_  _UM_  _U1_  _U2_  _U3_  _U4_  */
            {
 /* _D4_ */   { 35 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D3_ */   { 35 ,  35 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D2_ */   { 35 ,  35 ,  32 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D1_ */   { 32 ,  32 ,  32 ,  32 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _DM_ */   { 30 ,  30 ,  30 ,  30 ,  20 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _UM_ */   { 30 ,  30 ,  30 ,  20 ,  18 ,  18 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _U1_ */   { 30 ,  30 ,  30 ,  30 ,  20 ,  25 ,  25 ,  -2 ,  -2 ,  -2 } ,
 /* _U2_ */   { 30 ,  30 ,  30 ,  30 ,  30 ,  25 ,  30 ,  45 ,  -2 ,  -2 } ,
 /* _U3_ */   { 30 ,  30 ,  30 ,  30 ,  30 ,  30 ,  35 ,  65 ,  68 ,  -2 } ,
 /* _U4_ */   { 30 ,  30 ,  30 ,  30 ,  30 ,  30 ,  30 ,  68 ,  70 ,  90 }
            } ;

 /**************************************************************************/

 /* First index is highest , second index is lowest .                      */

 RECO_DATA _SCHAR maxY_H_end[REL_HIGHT_QANT-1][REL_HIGHT_QANT-1] =

 /*   H \ L    _D4_   _D3_ _D2_  _D1_  _DM_  _UM_  _U1_  _U2_  _U3_  _U4_  */
            {
 /* _D4_ */   { 30 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D3_ */   { 30 ,  30 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D2_ */   { 30 ,  30 ,  30 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _D1_ */   { 32 ,  32 ,  25 ,  25 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _DM_ */   { 35 ,  35 ,  22 ,  22 ,  22 ,  -2 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _UM_ */   { 35 ,  35 ,  35 ,  22 ,  22 ,  22 ,  -2 ,  -2 ,  -2 ,  -2 } ,
 /* _U1_ */   { 35 ,  35 ,  35 ,  35 ,  25 ,  28 ,  32 ,  -2 ,  -2 ,  -2 } ,
 /* _U2_ */   { 35 ,  35 ,  35 ,  35 ,  35 ,  32 ,  35 ,  43 ,  -2 ,  -2 } ,
 /* _U3_ */   { 35 ,  35 ,  35 ,  35 ,  35 ,  35 ,  40 ,  50 ,  60 ,  -2 } ,
 /* _U4_ */   { 35 ,  35 ,  35 ,  35 ,  35 ,  35 ,  35 ,  60 ,  85 , 100 }
            } ;

// from STROKA.C
RECO_DATA _SCHAR TG1[3][2][2]=
        {{{35,35},{35,35}},{{0,0},{0,0}},{{35,50},{50,50}}};
RECO_DATA _SCHAR TG2[3][2][2]=
        {{{35,45},{35,35}},{{45,45},{45,45}},{{35,50},{50,50}}};
RECO_DATA _SCHAR H1[3][2][2]=
        {{{34,48},{37,48}},{{32,32},{32,32}},{{47,50},{50,50}}};
RECO_DATA _SCHAR H2[3][2][2]=
        {{{48,48},{48,48}},{{32,32},{32,32}},{{47,50},{50,50}}};

RECO_DATA _SCHAR TG1_GER[3][2][2]=
        {{{35,35},{35,35}},{{45,55},{45,45}},{{26,50},{40,50}}};
RECO_DATA _SCHAR TG2_GER[3][2][2]=
        {{{35,45},{35,35}},{{36,55},{45,45}},{{26,55},{50,50}}};
RECO_DATA _SCHAR H1_GER[3][2][2]=
        {{{31,48},{37,48}},{{32,40},{30,32}},{{40,50},{50,50}}};
RECO_DATA _SCHAR H2_GER[3][2][2]=
        {{{48,48},{48,48}},{{32,50},{30,32}},{{40,50},{50,50}}};

RECO_DATA _SCHAR CS=80;

// from TXrWordDomain.c

RECO_DATA _UCHAR alpha_charset[][80] =
{
    // LANGUAGE_NONE
    {
        0,
    },
    // LANGUAGE_ENGLISH
    {
        "abcdefghijklmnopqrstuvwxyz\'"
    },
    // LANGUAGE_FRENCH
    {
        "abcdefghijklmnopqrstuvwxyz\'"
        "\xe0\xe2\xe8\xe9\xea\xeb\xee\xef\xf4\xf9\xfb\xe7"
        //"\x88\x8d\x8e\x95\x99\x9d\x91\x89\x8f\x90\x94\x9e\xcf\'"
        //"\x8A\x8C\x9A\x9F\xA7";
    },
    // LANGUAGE_GERMAN
    {
        "abcdefghijklmnopqrstuvwxyz\'"
        "\xe4\xf6\xfc\xdf"
    },
    // LANGUAGE_SPANISH
    {
        "abcdefghijklmnopqrstuvwxyz\'"
        "\xe0\xe1\xe7\xe8\xe9\xec\xed\xf1\xf2\xf3\xf9\xfa\xfc\xfd"
    },
    // LANGUAGE_ITALIAN
    {
        "abcdefghijklmnopqrstuvwxyz\'"
        "\xe0\xe8\xec\xf2\xf9\xe7"
        //"\x88\x8d\x8e\x95\x99\x9d\x91\x89\x8f\x90\x94\x9e\xcf\'"
        //"\x8A\x8C\x9A\x9F\xA7";
    },
    // LANGUAGE_SWEDISH
    {
        "abcdefghijklmnopqrstuvwxyz\'"
        "\xe5\xe4\xf6"
    },
    // LANGUAGE_NORWEGIAN
    {
        "abcdefghijklmnopqrstuvwxyz\'"
        "\xe6\xf8\xe5\xe8\xe9"
    },
    // LANGUAGE_DUTCH
    {
        "abcdefghijklmnopqrstuvwxyz\'"
        "\xe0\xe1\xe2\xe4\xe8\xe9\xea\xeb\xec\xed\xee\xef\xf2\xf3\xf4\xf6\xf9\xfa\xfb\xfc"
    },
    // LANGUAGE_DANISH
    {
        "abcdefghijklmnopqrstuvwxyz\'"
        "\xe6\xf8\xe5\xe8\xe9"
    },
    // LANGUAGE_PORTUGUESE
    {
        "abcdefghijklmnopqrstuvwxyz\'"
        "\xe0\xe1\xe2\xe3\xe7\xe9\xea\xed\xf2\xf3\xf4\xf5\xfa\xfc"
    },
    // LANGUAGE_PORTUGUESEB
    {
        "abcdefghijklmnopqrstuvwxyz\'"
        "\xe0\xe1\xe2\xe3\xe7\xe9\xea\xed\xf2\xf3\xf4\xf5\xfa\xfc"
    },

    // LANGUAGE_MEDICAL
    {
        "abcdefghijklmnopqrstuvwxyz\'"
    },

    // LANGUAGE_FINNISH
    {
        "abcdefghijklmnopqrstuvwxyz\'"
        "\xe5\xe4\xf6"
    },
    // LANGUAGE_INDONESIAN
    {
        "abcdefghijklmnopqrstuvwxyz\'"
    },
    // LANGUAGE_ENGLISHUK
    {
        "abcdefghijklmnopqrstuvwxyz\'"
    }
};

RECO_DATA _UCHAR lpunct_charset[][10] =
{
    // LANGUAGE_NONE
    {
        0,
    },
    // LANGUAGE_ENGLISH
    {
        "\'\"-([{<"
    },
    // LANGUAGE_FRENCH
    {
        "\'\"-([{<\xab"
    },
    // LANGUAGE_GERMAN
    {
        "'\"-([{<\xab"
    },
    // LANGUAGE_SPANISH
    {
        "\'\"-([{<"
    },
    // LANGUAGE_ITALIAN
    {
        "\'\"-([{<\xab"
    },
    // LANGUAGE_SWEDISH
    {
        "\'\"-([{<"
    },
    // LANGUAGE_NORWEGIAN
    {
        "\'\"-([{<"
    },
    // LANGUAGE_DUTCH
    {
        "\'\"-([{<"    
    },
    // LANGUAGE_DANISH
    {
        "\'\"-([{<"
    },
    // LANGUAGE_PORTUGUESE
    {
        "\'\"-([{<"    
    },
    // LANGUAGE_PORTUGUESEB
    {
        "\'\"-([{<"   
    },
    // LANGUAGE_MEDICAL
    {
        0,
    },
    // LANGUAGE_FINNISH
    {
        "\'\"-([{<"
    },
    // LANGUAGE_INDONESIAN
    {
        "\'\"-([{<"
    },
    // LANGUAGE_ENGLISHUK
    {
        "\'\"-([{<"
    },
};

RECO_DATA _UCHAR epunct_charset[][20] = {
    // LANGUAGE_NONE
    {
        0,
    },
    // LANGUAGE_ENGLISH
    {
        "\'\".,-!?)]}>;:_"
    },
    // LANGUAGE_FRENCH
    {
        "\'\".,-!?)]}>;:\xbb_"
    },
    // LANGUAGE_GERMAN
    {
        "'\".,-!?)]}>\xbb;:_"
    },
    // LANGUAGE_SPANISH
    {
        "\'\".,-!?)]}>;:_"
    },
    // LANGUAGE_ITALIAN
    {
        "\'\".,-!?)]}>;:\xbb_"
    },
    // LANGUAGE_SWEDISH
    {
        "\'\".,-!?)]}>;:_"
    },
    // LANGUAGE_NORWEGIAN
    {
        "\'\".,-!?)]}>;:_"
    },
    // LANGUAGE_DUTCH
    {
        "\'\".,-!?)]}>;:_"
    },
    // LANGUAGE_DANISH
    {
        "\'\".,-!?)]}>;:_"
    },
    // LANGUAGE_PORTUGUESE
    {
        "\'\".,-!?)]}>;:_"
    },
    // LANGUAGE_PORTUGUESEB
    {
        "\'\".,-!?)]}>;:_"
    },
    // LANGUAGE_MEDICAL
    {
        "\'\".,-!?)]}>;:_"
    },
    // LANGUAGE_FINNISH
    {
        "\'\".,-!?)]}>;:_"
    },
    // LANGUAGE_INDONESIAN
    {
        "\'\".,-!?)]}>;:_"
    },
    // LANGUAGE_ENGLISHUK
    {
        "\'\".,-!?)]}>;:_"
    },
};

RECO_DATA _UCHAR other_charset[][20] =
{
    // LANGUAGE_NONE
    {
        0,
    },
    // LANGUAGE_ENGLISH
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
    // LANGUAGE_FRENCH
    {
        "\\_@#$\xa3\xa5\xa9\xae"//plus should be "numero"
    },
    // LANGUAGE_GERMAN
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
    // LANGUAGE_SPANISH
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
    // LANGUAGE_ITALIAN
    {
        "\\_@#$\xa3\xa5\xa9\xae" //plus should be "numero"
    },
    // LANGUAGE_SWEDISH
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
    // LANGUAGE_NORWEGIAN
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
    // LANGUAGE_DUTCH
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
    // LANGUAGE_DANISH
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
    // LANGUAGE_PORTUGUESE
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
    // LANGUAGE_PORTUGUESEB
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
    // LANGUAGE_MEDICAL
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
    // LANGUAGE_FINNISH
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
    // LANGUAGE_INDONESIAN
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
    // LANGUAGE_ENGLISHUK
    {
        "\\_@#$\xa3\xa5\xa9\xae"
    },
};

RECO_DATA _UCHAR addr_charset[] = "@!\"#$%&*+-/=?^_;{|}~`";
RECO_DATA _UCHAR num_charset[]  = "0123456789";
RECO_DATA _UCHAR math_charset[] = "+-*/=()[]{}<>^~|%&.,";

RECO_DATA _UCHAR sp_vs_q_bd[15] = {80, 30, 50, 60, 70, 80, 110, 140, 170, 200, 210, 220, 230, 240, 250 },
                sp_vs_q_ts[15] = {64, 32, 48, 56, 64, 72,  80,  88,  96,  99, 100, 101, 102, 103, 104 };

// from coderf3.c
#if 0 //nobody seems to need this

RECO_DATA _LONG COS_12_TAB[129] =
{
    4096L,  4095L,  4094L,  4093L,  4091L,  4088L,  4084L,  4080L,
    4076L,  4071L,  4065L,  4058L,  4051L,  4043L,  4035L,  4026L,
    4017L,  4007L,  3996L,  3985L,  3973L,  3960L,  3947L,  3933L,
    3919L,  3904L,  3889L,  3873L,  3856L,  3839L,  3821L,  3803L,
    3784L,  3764L,  3744L,  3723L,  3702L,  3680L,  3658L,  3635L,
    3612L,  3588L,  3563L,  3538L,  3513L,  3487L,  3460L,  3433L,
    3405L,  3377L,  3348L,  3319L,  3289L,  3259L,  3229L,  3197L,
    3166L,  3134L,  3101L,  3068L,  3034L,  3000L,  2966L,  2931L,
    2896L,  2860L,  2824L,  2787L,  2750L,  2713L,  2675L,  2637L,
    2598L,  2559L,  2519L,  2480L,  2439L,  2399L,  2358L,  2317L,
    2275L,  2233L,  2191L,  2148L,  2105L,  2062L,  2018L,  1975L,
    1930L,  1886L,  1841L,  1796L,  1751L,  1705L,  1659L,  1613L,
    1567L,  1520L,  1474L,  1427L,  1379L,  1332L,  1284L,  1237L,
    1189L,  1140L,  1092L,  1043L,   995L,   946L,   897L,   848L,
    799L,   749L,   700L,   650L,   601L,   551L,   501L,   451L,
    401L,   351L,   301L,   251L,   200L,   150L,   100L,    50L,
    0L
};

#endif //0  

// from bit_mark.h

RECO_DATA    _UCHAR     BoxScale[MAX_BOX_SCALE] =
{
    0  , 10 , 20 , 30 , 40 , 50 , 60 , 70 , 80  , 90 ,
    100, 110, 120, 130, 140, 150, 160, 170, 180 , 190,
    200, 210, 220, 230
} ;


RECO_DATA    _UCHAR     NumExtScale[SCALE_EXT_DIMENSION] =
{
    0 , 1 , 2 , BIT_MARK_MAX
} ;


RECO_DATA    _CHAR      IntScale[SCALE_INT_DIMENSION] = { ISOLATE  , INTERSECTED };

 /*------------------------------------------------------------------------*/

RECO_DATA    BIT_MARK   Int_Um_Com_Marks[SCALE_INT_DIMENSION] = { {10}, {10} } ;

RECO_DATA    BIT_MARK   Int_Com_Um_Marks[SCALE_INT_DIMENSION] = { {10}, { 6} } ;

RECO_DATA    BIT_MARK   Pos_Cd_Com_Marks[SCALE_POS_DIMENSION] = { { 4}, {10}, {10} } ;

RECO_DATA    BIT_MARK   Pos_Com_Cd_Marks[SCALE_POS_DIMENSION] = { {10}, { 4}, { 4} } ;

                                                        /*  END_LAST  MID_LAST  COMMON */

 /*------------------------------------------------------------------------*/
//#if 0 // sketch is not in project !
// From sketch.cpp
RECO_DATA    BIT_MARK   CdElsMarks[SCALE_Y_ELS_DIMENSION][SCALE_X_ELS_DIMENSION] =  
{
    { { 0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {1}, {1}, {1}, {1}, {1}, {1}, {0}, {0} } ,
    { { 0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {2}, {2}, {3}, {3}, {3}, {3}, {2}, {1} } ,
    { { 1}, {1}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {1}, {3}, {4}, {4}, {4}, {4}, {4}, {3}, {2} } ,
    { { 3}, {3}, {2}, {2}, {2}, {2}, {2}, {2}, {2}, {3}, {3}, {3}, {5}, {6}, {6}, {6}, {5}, {4} } ,
    { { 4}, {4}, {4}, {3}, {3}, {3}, {3}, {3}, {3}, {5}, {5}, {6}, {7}, {7}, {7}, {6}, {5}, {5} } ,
    { { 5}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {6}, {7}, {7}, {7}, {7}, {7}, {6}, {5}, {5} } ,
    { { 5}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {6}, {7}, {7}, {7}, {7}, {7}, {6}, {5}, {5} } ,
    { { 5}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {6}, {7}, {7}, {7}, {7}, {7}, {6}, {5}, {5} } ,
    { { 5}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {6}, {7}, {7}, {7}, {7}, {7}, {6}, {5}, {5} } ,
    { { 5}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {6}, {7}, {7}, {7}, {7}, {7}, {6}, {5}, {5} } ,
    { { 4}, {4}, {4}, {5}, {5}, {5}, {5}, {5}, {5}, {6}, {7}, {7}, {7}, {7}, {7}, {6}, {5}, {5} } ,
    { { 3}, {3}, {3}, {4}, {5}, {5}, {5}, {5}, {5}, {6}, {7}, {7}, {7}, {7}, {7}, {6}, {5}, {5} } ,
    { { 1}, {2}, {2}, {3}, {3}, {4}, {5}, {5}, {5}, {6}, {7}, {7}, {7}, {7}, {7}, {6}, {5}, {5} } ,
    { { 0}, {1}, {1}, {2}, {3}, {4}, {4}, {5}, {5}, {6}, {7}, {7}, {7}, {7}, {7}, {6}, {5}, {5} } ,
    { { 0}, {0}, {0}, {1}, {2}, {2}, {3}, {4}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {5}, {5} }
} ;

RECO_DATA   BIT_MARK   Pos_Cd_Els_Marks[SCALE_POS_DIMENSION] = { { 4}, {10}, {10} };

RECO_DATA   BIT_MARK   Pos_Els_Cd_Marks[SCALE_POS_DIMENSION] = { {10}, { 4}, {10} };

                                                        /*  END_LAST  MID_LAST  COMMON   */


RECO_DATA    BIT_MARK  Int_Cd_Els_Marks[SCALE_INT_DIMENSION] = { {10}, {10} } ;

/*
RECO_DATA    BIT_MARK  Int_Els_Cd_Marks[SCALE_INT_DIMENSION] = { { 8}, {10} } ;
*/

RECO_DATA    BIT_MARK  Int_Els_Cd_Marks[SCALE_INT_DIMENSION] = { {10}, {10} } ;

                                                        /*  ISOLATE   INTERSECTED        */

 /*------------------------------------------------------------------------*/


RECO_DATA    BIT_MARK   CdDotMarks[SCALE_Y_DOT_DIMENSION][SCALE_X_DOT_DIMENSION] =
{
    { { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 1} } ,
    { { 0}, { 0}, { 1}, { 1}, { 1}, { 1}, { 1}, { 1}, { 1}, { 1}, { 1}, { 2}, { 2}, { 2}, { 3} } ,
    { { 2}, { 2}, { 2}, { 2}, { 2}, { 2}, { 2}, { 2}, { 2}, { 2}, { 2}, { 3}, { 3}, { 4}, { 5} } ,
    { { 3}, { 3}, { 3}, { 3}, { 3}, { 3}, { 3}, { 3}, { 3}, { 3}, { 3}, { 4}, { 5}, { 6}, { 7} } ,
    { { 4}, { 5}, { 6}, { 4}, { 4}, { 4}, { 4}, { 4}, { 4}, { 4}, { 4}, { 5}, { 7}, { 8}, { 9} } ,
    { { 5}, { 5}, { 5}, { 5}, { 5}, { 5}, { 5}, { 5}, { 5}, { 5}, { 5}, { 7}, { 9}, {10}, {10} } ,
    { { 6}, { 5}, { 5}, { 6}, { 6}, { 6}, { 6}, { 6}, { 6}, { 6}, { 6}, { 7}, { 9}, {10}, {10} } ,
    { { 8}, { 7}, { 7}, { 7}, { 7}, { 7}, { 7}, { 7}, { 6}, { 6}, { 6}, { 7}, { 9}, {10}, {10} } ,
    { {10}, { 8}, { 8}, { 9}, { 9}, { 9}, { 9}, { 8}, { 7}, { 7}, { 7}, { 7}, { 9}, {10}, {10} } ,
    { { 9}, {10}, {10}, {10}, {10}, {10}, {10}, { 9}, { 9}, { 8}, { 8}, { 9}, {10}, {10}, {10} } ,
    { { 8}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10} }
} ;

RECO_DATA  BIT_MARK   Int_Cd_Dot_Marks[SCALE_INT_DIMENSION] = { {10}, {10} } ;

RECO_DATA  BIT_MARK   Int_Dot_Cd_Marks[SCALE_INT_DIMENSION] = { {10}, { 7} } ;

                                                        /*  ISOLATE   INTERSECTED        */

 /*------------------------------------------------------------------------*/


RECO_DATA    BIT_MARK   CdStrMarks[SCALE_Y_STR_DIMENSION][SCALE_X_STR_DIMENSION] = 
{
    { { 2}, { 2}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} } ,
    { { 4}, { 3}, { 1}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, { 0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} } ,
    { { 6}, { 5}, { 3}, { 2}, { 1}, { 1}, { 2}, { 2}, { 1}, { 1}, { 1}, { 0}, { 0}, { 0}, { 0}, {0}, {0}, {0}, {0}, {0}, {0}, {0}, {0} } ,
    { { 8}, { 6}, { 5}, { 3}, { 3}, { 3}, { 3}, { 3}, { 3}, { 3}, { 3}, { 2}, { 1}, { 1}, { 1}, {1}, {0}, {0}, {0}, {0}, {0}, {0}, {0} } ,
    { { 9}, { 7}, { 6}, { 5}, { 5}, { 5}, { 5}, { 4}, { 4}, { 4}, { 5}, { 3}, { 3}, { 3}, { 3}, {2}, {1}, {0}, {0}, {0}, {0}, {0}, {0} } ,
    { { 9}, { 8}, { 8}, { 7}, { 7}, { 7}, { 6}, { 5}, { 5}, { 6}, { 7}, { 5}, { 5}, { 4}, { 5}, {4}, {2}, {0}, {0}, {0}, {0}, {0}, {0} } ,
    { { 9}, {10}, { 9}, { 8}, { 8}, { 9}, { 8}, { 7}, { 7}, { 8}, { 8}, { 7}, { 6}, { 5}, { 5}, {4}, {2}, {2}, {2}, {2}, {2}, {2}, {2} } ,
    { {10}, {10}, {10}, {10}, {10}, {10}, {10}, { 9}, { 9}, {10}, {10}, { 9}, { 8}, { 7}, { 6}, {5}, {3}, {4}, {3}, {3}, {2}, {2}, {2} } ,
    { {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, { 9}, { 8}, { 6}, {6}, {5}, {5}, {4}, {3}, {3}, {3}, {2} } ,
    { {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, { 9}, { 8}, {6}, {5}, {5}, {5}, {4}, {3}, {3}, {3} } ,
    { {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, { 9}, {8}, {7}, {6}, {5}, {4}, {4}, {3}, {3} } ,
    { {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {10}, {9}, {8}, {7}, {6}, {5}, {5}, {4}, {3} }
} ;


