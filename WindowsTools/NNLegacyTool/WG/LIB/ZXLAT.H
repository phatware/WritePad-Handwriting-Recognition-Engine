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

#ifndef _XLAT_INCLUDE_
#define _XLAT_INCLUDE_

/* Definitions for REC (DTE) international characters */
#ifndef  REC_BritishPound

#define REC_BritishPound    ((unsigned char)0x7f)
#define REC_A_umlaut        ((unsigned char)0x80)
#define REC_a_umlaut        ((unsigned char)0x81)
#define REC_O_umlaut        ((unsigned char)0x82)
#define REC_o_umlaut        ((unsigned char)0x83)
#define REC_U_umlaut        ((unsigned char)0x84)
#define REC_u_umlaut        ((unsigned char)0x85)
#define REC_ESZET           ((unsigned char)0x86)

#define REC_A_grave         ((unsigned char)0x87)
#define REC_a_grave         ((unsigned char)0x88)
#define REC_A_circumflex    ((unsigned char)0x89)
#define REC_a_circumflex    ((unsigned char)0x8a)
#define REC_C_cedilla       ((unsigned char)0x8b)
#define REC_c_cedilla       ((unsigned char)0x8c)
#define REC_E_grave         ((unsigned char)0x8d)
#define REC_e_grave         ((unsigned char)0x8e)
#define REC_E_acute         ((unsigned char)0x8f)
#define REC_e_acute         ((unsigned char)0x90)
#define REC_E_circumflex    ((unsigned char)0x91)
#define REC_e_circumflex    ((unsigned char)0x92)
#define REC_I_circumflex    ((unsigned char)0x93)
#define REC_i_circumflex    ((unsigned char)0x94)
#define REC_I_umlaut        ((unsigned char)0x95)
#define REC_i_umlaut        ((unsigned char)0x96)
#define REC_O_circumflex    ((unsigned char)0x97)
#define REC_o_circumflex    ((unsigned char)0x98)
#define REC_U_grave         ((unsigned char)0x99)
#define REC_u_grave         ((unsigned char)0x9a)
#define REC_U_circumflex    ((unsigned char)0x9b)
#define REC_u_circumflex    ((unsigned char)0x9c)
#define REC_e_umlaut        ((unsigned char)0x9d)
#define REC_N_numero        ((unsigned char)0x9e)
#define REC_n_numero        ((unsigned char)0x9f)

#define REC_A_angstrem      ((unsigned char)0xa0)
#define REC_a_angstrem      ((unsigned char)0xa1)

#define REC_Yenn_sign       ((unsigned char)0xa2)

#define REC_DblBrace_left   ((unsigned char)0xa3)
#define REC_DblBrace_right  ((unsigned char)0xa4)
//#define REC_Paragraph_sign  ((unsigned char)0xa5)
#define REC_Copyright_sign  ((unsigned char)0xa5)

#define REC_Y_umlaut        ((unsigned char)0xa6)
#define REC_y_umlaut        ((unsigned char)0xa7)
#define REC_N_tilda         ((unsigned char)0xa8)
#define REC_n_tilda         ((unsigned char)0xa9)

//#define REC_Cent_sign       ((unsigned char)0xaa)
#define REC_TradeName_sign  ((unsigned char)0xaa)

#define REC_Question_inv    ((unsigned char)0xab)
#define REC_Exclamation_inv ((unsigned char)0xac)

#define REC_A_acute         ((unsigned char)0xad)
#define REC_a_acute         ((unsigned char)0xae)
#define REC_I_acute         ((unsigned char)0xaf)
#define REC_i_acute         ((unsigned char)0xb0)
#define REC_I_grave         ((unsigned char)0xb1)
#define REC_i_grave         ((unsigned char)0xb2)
#define REC_O_acute         ((unsigned char)0xb3)
#define REC_o_acute         ((unsigned char)0xb4)
#define REC_O_grave         ((unsigned char)0xb5)
#define REC_o_grave         ((unsigned char)0xb6)
#define REC_U_acute         ((unsigned char)0xb7)
#define REC_u_acute         ((unsigned char)0xb8)
#define REC_A_tilda         ((unsigned char)0xb9)
#define REC_a_tilda         ((unsigned char)0xba)
#define REC_O_tilda         ((unsigned char)0xbb)
#define REC_o_tilda         ((unsigned char)0xbc)
#define REC_E_umlaut        ((unsigned char)0xbd)
#define REC_oe_letter       ((unsigned char)0xbe)
#define REC_OE_letter       ((unsigned char)0xbf)

#endif /*REC_...*/



#ifndef  OS_BritishPound

/*  These definitions are for Windows OS: */

#define OS_BritishPound    ((unsigned char)0xa3)
#define OS_A_umlaut        ((unsigned char)0xc4)
#define OS_a_umlaut        ((unsigned char)0xe4)
#define OS_O_umlaut        ((unsigned char)0xd6)
#define OS_o_umlaut        ((unsigned char)0xf6)
#define OS_U_umlaut        ((unsigned char)0xdc)
#define OS_u_umlaut        ((unsigned char)0xfc)
#define OS_ESZET           ((unsigned char)0xdf)

#define OS_A_grave         ((unsigned char)0xc0)
#define OS_a_grave         ((unsigned char)0xe0)
#define OS_A_circumflex    ((unsigned char)0xc2)
#define OS_a_circumflex    ((unsigned char)0xe2)
#define OS_C_cedilla       ((unsigned char)0xc7)
#define OS_c_cedilla       ((unsigned char)0xe7)
#define OS_E_grave         ((unsigned char)0xc8)
#define OS_e_grave         ((unsigned char)0xe8)
#define OS_E_acute         ((unsigned char)0xc9)
#define OS_e_acute         ((unsigned char)0xe9)
#define OS_E_circumflex    ((unsigned char)0xca)
#define OS_e_circumflex    ((unsigned char)0xea)
#define OS_I_circumflex    ((unsigned char)0xce)
#define OS_i_circumflex    ((unsigned char)0xee)
#define OS_I_umlaut        ((unsigned char)0xcf)
#define OS_i_umlaut        ((unsigned char)0xef)
#define OS_O_circumflex    ((unsigned char)0xd4)
#define OS_o_circumflex    ((unsigned char)0xf4)
#define OS_U_grave         ((unsigned char)0xd9)
#define OS_u_grave         ((unsigned char)0xf9)
#define OS_U_circumflex    ((unsigned char)0xdb)
#define OS_u_circumflex    ((unsigned char)0xfb)
#define OS_e_umlaut        ((unsigned char)0xeb)
#define OS_N_numero        ((unsigned char)0xaa)
#define OS_n_numero        ((unsigned char)0xba)

#define OS_A_angstrem      ((unsigned char)0xc5)
#define OS_a_angstrem      ((unsigned char)0xe5)

#define OS_Yenn_sign       ((unsigned char)0xa5)

#define OS_DblBrace_left   ((unsigned char)0xab)
#define OS_DblBrace_right  ((unsigned char)0xbb)
//#define OS_Paragraph_sign  ((unsigned char)0xb6)

#define OS_Copyright_sign  ((unsigned char)0xa9)

#define OS_Y_umlaut        ((unsigned char)0x9f)
#define OS_y_umlaut        ((unsigned char)0xff)
#define OS_N_tilda         ((unsigned char)0xd1)
#define OS_n_tilda         ((unsigned char)0xf1)

//#define OS_Cent_sign       ((unsigned char)0xa2)

#define OS_TradeName_sign  ((unsigned char)0xae)

#define OS_Question_inv    ((unsigned char)0xbf)
#define OS_Exclamation_inv ((unsigned char)0xa1)

#define OS_A_acute         ((unsigned char)0xc1)
#define OS_a_acute         ((unsigned char)0xe1)
#define OS_I_acute         ((unsigned char)0xcd)
#define OS_i_acute         ((unsigned char)0xed)
#define OS_I_grave         ((unsigned char)0xcc)
#define OS_i_grave         ((unsigned char)0xec)
#define OS_O_acute         ((unsigned char)0xd3)
#define OS_o_acute         ((unsigned char)0xf3)
#define OS_O_grave         ((unsigned char)0xd2)
#define OS_o_grave         ((unsigned char)0xf2)
#define OS_U_acute         ((unsigned char)0xda)
#define OS_u_acute         ((unsigned char)0xfa)
#define OS_A_tilda         ((unsigned char)0xc3)
#define OS_a_tilda         ((unsigned char)0xe3)
#define OS_O_tilda         ((unsigned char)0xd5)
#define OS_o_tilda         ((unsigned char)0xf5)
#define OS_E_umlaut        ((unsigned char)0xcb)
#define OS_oe_letter       ((unsigned char)0x9c)
#define OS_OE_letter       ((unsigned char)0x8c)

#if  0 /*Macintosh definitions - don't use them for now */
/* Definitions for OS (Macintosh Roman) international characters */

#define OS_BritishPound     ((unsigned char)0xa3)
#define OS_A_umlaut         ((unsigned char)0x80)
#define OS_a_umlaut         ((unsigned char)0x8a)
#define OS_O_umlaut         ((unsigned char)0x85)
#define OS_o_umlaut         ((unsigned char)0x9a)
#define OS_U_umlaut         ((unsigned char)0x86)
#define OS_u_umlaut         ((unsigned char)0x9f)
#define OS_ESZET            ((unsigned char)0xa7)

#define OS_A_grave          ((unsigned char)0xcb)
#define OS_a_grave          ((unsigned char)0x88)
#define OS_A_circumflex     ((unsigned char)0xe5)
#define OS_a_circumflex     ((unsigned char)0x89)
#define OS_C_cedilla        ((unsigned char)0x82)
#define OS_c_cedilla        ((unsigned char)0x8d)
#define OS_E_grave          ((unsigned char)0xe9)
#define OS_e_grave          ((unsigned char)0x8f)
#define OS_E_acute          ((unsigned char)0x83)
#define OS_e_acute          ((unsigned char)0x8e)
#define OS_E_circumflex     ((unsigned char)0xe6)
#define OS_e_circumflex     ((unsigned char)0x90)
#define OS_I_circumflex     ((unsigned char)0xeb)
#define OS_i_circumflex     ((unsigned char)0x94)
#define OS_I_umlaut         ((unsigned char)0xec)
#define OS_i_umlaut         ((unsigned char)0x95)
#define OS_O_circumflex     ((unsigned char)0xef)
#define OS_o_circumflex     ((unsigned char)0x99)
#define OS_U_grave          ((unsigned char)0xf4)
#define OS_u_grave          ((unsigned char)0x9d)
#define OS_U_circumflex     ((unsigned char)0xf3)
#define OS_u_circumflex     ((unsigned char)0x9e)
#define OS_e_umlaut         ((unsigned char)0x91)
#define OS_N_numero         ((unsigned char)0xbb)
#define OS_n_numero         ((unsigned char)0xbc)

#define OS_A_angstrem       ((unsigned char)0x81)
#define OS_a_angstrem       ((unsigned char)0x8c)

#define OS_Yenn_sign        ((unsigned char)0xb4)

#define OS_DblBrace_left    ((unsigned char)0xc7)
#define OS_DblBrace_right   ((unsigned char)0xc8)
#define OS_Paragraph_sign   ((unsigned char)0xa6)

#define OS_Y_umlaut         ((unsigned char)0xd9)
#define OS_y_umlaut         ((unsigned char)0xd8)
#define OS_N_tilda          ((unsigned char)0x84)
#define OS_n_tilda          ((unsigned char)0x96)

#define OS_Cent_sign        ((unsigned char)0xa2)
#define OS_Question_inv     ((unsigned char)0xc0)
#define OS_Exclamation_inv  ((unsigned char)0xc1)

#define OS_A_acute          ((unsigned char)0xe7)
#define OS_a_acute          ((unsigned char)0x87)
#define OS_I_acute          ((unsigned char)0xea)
#define OS_i_acute          ((unsigned char)0x92)
#define OS_I_grave          ((unsigned char)0xed)
#define OS_i_grave          ((unsigned char)0x93)
#define OS_O_acute          ((unsigned char)0xee)
#define OS_o_acute          ((unsigned char)0x97)
#define OS_O_grave          ((unsigned char)0xf1)
#define OS_o_grave          ((unsigned char)0x98)
#define OS_U_acute          ((unsigned char)0xf2)
#define OS_u_acute          ((unsigned char)0x9c)
#define OS_A_tilda          ((unsigned char)0xcc)
#define OS_a_tilda          ((unsigned char)0x8b)
#define OS_O_tilda          ((unsigned char)0xcd)
#define OS_o_tilda          ((unsigned char)0x9b)
#define OS_E_umlaut         ((unsigned char)0xe8)
#define OS_oe_letter        ((unsigned char)0xcf)
#define OS_OE_letter        ((unsigned char)0xce)

#endif  /*Macintosh definitions - don't use them for now */

#endif /*OS_...*/

/* Definitions for WIN (Extended ANSI) international characters */
#define WIN_BritishPound    ((unsigned char)0xa3)
#define WIN_A_umlaut        ((unsigned char)0xc4)
#define WIN_a_umlaut        ((unsigned char)0xe4)
#define WIN_O_umlaut        ((unsigned char)0xd6)
#define WIN_o_umlaut        ((unsigned char)0xf6)
#define WIN_U_umlaut        ((unsigned char)0xdc)
#define WIN_u_umlaut        ((unsigned char)0xfc)
#define WIN_ESZET           ((unsigned char)0xdf)

#define WIN_A_grave         ((unsigned char)0xc0)
#define WIN_a_grave         ((unsigned char)0xe0)
#define WIN_A_circumflex    ((unsigned char)0xc2)
#define WIN_a_circumflex    ((unsigned char)0xe2)
#define WIN_C_cedilla       ((unsigned char)0xc7)
#define WIN_c_cedilla       ((unsigned char)0xe7)
#define WIN_E_grave         ((unsigned char)0xc8)
#define WIN_e_grave         ((unsigned char)0xe8)
#define WIN_E_acute         ((unsigned char)0xc9)
#define WIN_e_acute         ((unsigned char)0xe9)
#define WIN_E_circumflex    ((unsigned char)0xca)
#define WIN_e_circumflex    ((unsigned char)0xea)
#define WIN_I_circumflex    ((unsigned char)0xce)
#define WIN_i_circumflex    ((unsigned char)0xee)
#define WIN_I_umlaut        ((unsigned char)0xcf)
#define WIN_i_umlaut        ((unsigned char)0xef)
#define WIN_O_circumflex    ((unsigned char)0xd4)
#define WIN_o_circumflex    ((unsigned char)0xf4)
#define WIN_U_grave         ((unsigned char)0xd9)
#define WIN_u_grave         ((unsigned char)0xf9)
#define WIN_U_circumflex    ((unsigned char)0xdb)
#define WIN_u_circumflex    ((unsigned char)0xfb)
#define WIN_e_umlaut        ((unsigned char)0xeb)
#define WIN_N_numero        ((unsigned char)0xaa)
#define WIN_n_numero        ((unsigned char)0xba)

#define WIN_A_angstrem      ((unsigned char)0xc5)
#define WIN_a_angstrem      ((unsigned char)0xe5)

#define WIN_Yenn_sign       ((unsigned char)0xa5)

#define WIN_DblBrace_left   ((unsigned char)0xab)
#define WIN_DblBrace_right  ((unsigned char)0xbb)
#define WIN_Copyright_sign  ((unsigned char)0xa9)

#define WIN_Y_umlaut        ((unsigned char)0x9f)
#define WIN_y_umlaut        ((unsigned char)0xff)
#define WIN_N_tilda         ((unsigned char)0xd1)
#define WIN_n_tilda         ((unsigned char)0xf1)

#define WIN_TradeName_sign  ((unsigned char)0xae)
#define WIN_Question_inv    ((unsigned char)0xbf)
#define WIN_Exclamation_inv ((unsigned char)0xa1)

#define WIN_A_acute         ((unsigned char)0xc1)
#define WIN_a_acute         ((unsigned char)0xe1)
#define WIN_I_acute         ((unsigned char)0xcd)
#define WIN_i_acute         ((unsigned char)0xed)
#define WIN_I_grave         ((unsigned char)0xcc)
#define WIN_i_grave         ((unsigned char)0xec)
#define WIN_O_acute         ((unsigned char)0xd3)
#define WIN_o_acute         ((unsigned char)0xf3)
#define WIN_O_grave         ((unsigned char)0xd2)
#define WIN_o_grave         ((unsigned char)0xf2)
#define WIN_U_acute         ((unsigned char)0xda)
#define WIN_u_acute         ((unsigned char)0xfa)
#define WIN_A_tilda         ((unsigned char)0xc3)
#define WIN_a_tilda         ((unsigned char)0xe3)
#define WIN_O_tilda         ((unsigned char)0xd5)
#define WIN_o_tilda         ((unsigned char)0xf5)
#define WIN_E_umlaut        ((unsigned char)0xcb)
#define WIN_oe_letter       ((unsigned char)0x9c)
#define WIN_OE_letter       ((unsigned char)0x8c)

#ifdef __cplusplus
extern "C" {        /* Assume C declarations for C++ */
#endif  /* __cplusplus */

/* Function definitions for WIN international charset */
unsigned short DTEToMAC(unsigned short usym);
unsigned short MACToDTE(unsigned short usym);

unsigned short DTEToANSI(unsigned short usym);
unsigned short ANSIToDTE(unsigned short usym);

unsigned short ANSIToMAC(unsigned short usym);
unsigned short MACToANSI(unsigned short usym);

#define OCToRec MACToDTE
#define RecToOC DTEToMAC

#ifdef __cplusplus
}                   /* End of extern "C" { */
#endif  /* __cplusplus */

#endif /* _XLAT_INCLUDE_ */
