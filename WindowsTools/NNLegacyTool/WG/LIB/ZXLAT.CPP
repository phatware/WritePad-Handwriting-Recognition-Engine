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

#include "zxlat.h"
/* --------------------------------------------------------------- */
static unsigned char OSToRecXlatTable [][2] =
{
	{ OS_BritishPound, REC_BritishPound },
	{ OS_A_umlaut, REC_A_umlaut },
	{ OS_a_umlaut, REC_a_umlaut },
	{ OS_O_umlaut, REC_O_umlaut },
	{ OS_o_umlaut, REC_o_umlaut },
	{ OS_U_umlaut, REC_U_umlaut },
	{ OS_u_umlaut, REC_u_umlaut },
	{ OS_ESZET, REC_ESZET },
	{ OS_A_grave, REC_A_grave },
	{ OS_a_grave, REC_a_grave },
	{ OS_A_circumflex, REC_A_circumflex },
	{ OS_a_circumflex, REC_a_circumflex },
	{ OS_C_cedilla, REC_C_cedilla },
	{ OS_c_cedilla, REC_c_cedilla },
	{ OS_E_grave, REC_E_grave },
	{ OS_e_grave, REC_e_grave },
	{ OS_E_acute, REC_E_acute },
	{ OS_e_acute, REC_e_acute },
	{ OS_E_circumflex, REC_E_circumflex },
	{ OS_e_circumflex, REC_e_circumflex },
	{ OS_I_circumflex, REC_I_circumflex },
	{ OS_i_circumflex, REC_i_circumflex },
	{ OS_I_umlaut, REC_I_umlaut },
	{ OS_i_umlaut, REC_i_umlaut },
	{ OS_O_circumflex, REC_O_circumflex },
	{ OS_o_circumflex, REC_o_circumflex },
	{ OS_U_grave, REC_U_grave },
	{ OS_u_grave, REC_u_grave },
	{ OS_U_circumflex, REC_U_circumflex },
	{ OS_u_circumflex, REC_u_circumflex },
	{ OS_e_umlaut, REC_e_umlaut },
	{ OS_N_numero, REC_N_numero },
	{ OS_n_numero, REC_n_numero },
	{ OS_A_angstrem, REC_A_angstrem },
	{ OS_a_angstrem, REC_a_angstrem },
	{ OS_Yenn_sign, REC_Yenn_sign },
	{ OS_DblBrace_left, REC_DblBrace_left },
	{ OS_DblBrace_right, REC_DblBrace_right },
	{ OS_Copyright_sign, REC_Copyright_sign },
	{ OS_Y_umlaut, REC_Y_umlaut },
	{ OS_y_umlaut, REC_y_umlaut },
	{ OS_N_tilda, REC_N_tilda },
	{ OS_n_tilda, REC_n_tilda },
	{ OS_TradeName_sign, REC_TradeName_sign },
	{ OS_Question_inv, REC_Question_inv },
	{ OS_Exclamation_inv, REC_Exclamation_inv },
	{ OS_A_acute, REC_A_acute },
	{ OS_a_acute, REC_a_acute },
	{ OS_I_acute, REC_I_acute },
	{ OS_i_acute, REC_i_acute },
	{ OS_I_grave, REC_I_grave },
	{ OS_i_grave, REC_i_grave },
	{ OS_O_acute, REC_O_acute },
	{ OS_o_acute, REC_o_acute },
	{ OS_O_grave, REC_O_grave },
	{ OS_o_grave, REC_o_grave },
	{ OS_U_acute, REC_U_acute },
	{ OS_u_acute, REC_u_acute },
	{ OS_A_tilda, REC_A_tilda },
	{ OS_a_tilda, REC_a_tilda },
	{ OS_O_tilda, REC_O_tilda },
	{ OS_o_tilda, REC_o_tilda },
	{ OS_E_umlaut, REC_E_umlaut },
	{ OS_oe_letter, REC_oe_letter },
	{ OS_OE_letter, REC_OE_letter },
	{ 0, 0 }
};

#ifndef _FULL_XLAT_TABLE
/* --------------------------------------------------------------- */
static unsigned char OSToWinXlatTable [][2] =
{
	{ OS_BritishPound, WIN_BritishPound },
	{ OS_A_umlaut, WIN_A_umlaut },
	{ OS_a_umlaut, WIN_a_umlaut },
	{ OS_O_umlaut, WIN_O_umlaut },
	{ OS_o_umlaut, WIN_o_umlaut },
	{ OS_U_umlaut, WIN_U_umlaut },
	{ OS_u_umlaut, WIN_u_umlaut },
	{ OS_ESZET, WIN_ESZET },
	{ OS_A_grave, WIN_A_grave },
	{ OS_a_grave, WIN_a_grave },
	{ OS_A_circumflex, WIN_A_circumflex },
	{ OS_a_circumflex, WIN_a_circumflex },
	{ OS_C_cedilla, WIN_C_cedilla },
	{ OS_c_cedilla, WIN_c_cedilla },
	{ OS_E_grave, WIN_E_grave },
	{ OS_e_grave, WIN_e_grave },
	{ OS_E_acute, WIN_E_acute },
	{ OS_e_acute, WIN_e_acute },
	{ OS_E_circumflex, WIN_E_circumflex },
	{ OS_e_circumflex, WIN_e_circumflex },
	{ OS_I_circumflex, WIN_I_circumflex },
	{ OS_i_circumflex, WIN_i_circumflex },
	{ OS_I_umlaut, WIN_I_umlaut },
	{ OS_i_umlaut, WIN_i_umlaut },
	{ OS_O_circumflex, WIN_O_circumflex },
	{ OS_o_circumflex, WIN_o_circumflex },
	{ OS_U_grave, WIN_U_grave },
	{ OS_u_grave, WIN_u_grave },
	{ OS_U_circumflex, WIN_U_circumflex },
	{ OS_u_circumflex, WIN_u_circumflex },
	{ OS_e_umlaut, WIN_e_umlaut },
	{ OS_N_numero, WIN_N_numero },
	{ OS_n_numero, WIN_n_numero },
	{ OS_A_angstrem, WIN_A_angstrem },
	{ OS_a_angstrem, WIN_a_angstrem },
	{ OS_Yenn_sign, WIN_Yenn_sign },
	{ OS_DblBrace_left, WIN_DblBrace_left },
	{ OS_DblBrace_right, WIN_DblBrace_right },
	{ OS_Copyright_sign, WIN_Copyright_sign },
	{ OS_Y_umlaut, WIN_Y_umlaut },
	{ OS_y_umlaut, WIN_y_umlaut },
	{ OS_N_tilda, WIN_N_tilda },
	{ OS_n_tilda, WIN_n_tilda },
	{ OS_TradeName_sign, WIN_TradeName_sign },
	{ OS_Question_inv, WIN_Question_inv },
	{ OS_Exclamation_inv, WIN_Exclamation_inv },
	{ OS_A_acute, WIN_A_acute },
	{ OS_a_acute, WIN_a_acute },
	{ OS_I_acute, WIN_I_acute },
	{ OS_i_acute, WIN_i_acute },
	{ OS_I_grave, WIN_I_grave },
	{ OS_i_grave, WIN_i_grave },
	{ OS_O_acute, WIN_O_acute },
	{ OS_o_acute, WIN_o_acute },
	{ OS_O_grave, WIN_O_grave },
	{ OS_o_grave, WIN_o_grave },
	{ OS_U_acute, WIN_U_acute },
	{ OS_u_acute, WIN_u_acute },
	{ OS_A_tilda, WIN_A_tilda },
	{ OS_a_tilda, WIN_a_tilda },
	{ OS_O_tilda, WIN_O_tilda },
	{ OS_o_tilda, WIN_o_tilda },
	{ OS_E_umlaut, WIN_E_umlaut },
	{ OS_oe_letter, WIN_oe_letter },
	{ OS_OE_letter, WIN_OE_letter },
	{ 0, 0 }
};

#else /* _FULL_XLAT_TABLE */

#error  Not intended to work - table may not correspond to the one above!!!

static unsigned char Ansi2MacXlatTable[128] =
{
	0x0,  0x0,  0xe2, 0xc4, 0xe3, 0xc9, 0xa0, 0xe0,
	0xf6, 0xe4, 0x0,  0xdc, 0xce, 0x0,  0x0,  0x0,
	0x0,  0xd4, 0xd5, 0xd2, 0xd3, 0xa5, 0xd0, 0xd1,
	0xf7, 0xaa, 0x0,  0xdd, 0xcf, 0x0,  0x0,  0xd9,
	0x0,  0xc1, 0xa2, 0xa3, 0xdb, 0xb4, 0x0,  0xa4,
	0xac, 0xa9, 0xbb, 0xc7, 0xc2, 0x0,  0xa8, 0xf8,
	0xa1, 0xb1, 0x0,  0x0,  0xab, 0xb5, 0xa6, 0xe1,
	0xfc, 0x0,  0xbc, 0xc8, 0x0,  0x0,  0x0,  0xc0,
	0xcb, 0xe7, 0xe5, 0xcc, 0x80, 0x81, 0xae, 0x82,
	0xe9, 0x83, 0xe6, 0xe8, 0xed, 0xea, 0xeb, 0xec,
	0x0,  0x84, 0xf1, 0xee, 0xef, 0xcd, 0x85, 0x0,
	0xaf, 0xf4, 0xf2, 0xf3, 0x86, 0x0, 0x0,  0xa7,
	0x88, 0x87, 0x89, 0x8b, 0x8a, 0x8c, 0xbe, 0x8d,
	0x8f, 0x8e, 0x90, 0x91, 0x93, 0x92, 0x94, 0x95,
	0x0,  0x96, 0x98, 0x97, 0x99, 0x9b, 0x9a, 0xd6,
	0xbf, 0x9d, 0x9c, 0x9e, 0x9f, 0x0,  0x0,  0xd8
};

static unsigned char Mac2AnsiXlatTable[] =
{
	0xc4, 0xc5, 0xc7, 0xc9, 0xd1, 0xd6, 0xdc, 0xe1,
	0xe0, 0xe2, 0xe4, 0xe3, 0xe5, 0xe7, 0xe9, 0xe8,
	0xea, 0xeb, 0xed, 0xec, 0xee, 0xef, 0xf1, 0xf3,
	0xf2, 0xf4, 0xf6, 0xf5, 0xfa, 0xf9, 0xfb, 0xfc,
	0x86, 0xb0, 0xa2, 0xa3, 0xa7, 0x95, 0xb6, 0xdf,
	0xae, 0xa9, 0x99, 0xb4, 0xa8, 0x0,  0xc6, 0xd8,
	0x0,  0xb1, 0x0,  0x0,  0xa5, 0xb5, 0x0,  0x0,
	0x0,  0x0,  0x0,  0xaa, 0xba, 0x0,  0xe6, 0xf8,
	0xbf, 0xa1, 0xac, 0x0,  0x83, 0x0,  0x0,  0xab,
	0xbb, 0x85, 0x0,  0xc0, 0xc3, 0xd5, 0x8c, 0x9c,
	0x96, 0x97, 0x93, 0x94, 0x91, 0x92, 0xf7, 0x0,
	0xff, 0x9f, 0x0,  0xa4, 0x8b, 0x9b, 0x0,  0x0,
	0x87, 0xb7, 0x82, 0x84, 0x89, 0xc2, 0xca, 0xc1,
	0xcb, 0xc8, 0xcd, 0xce, 0xcf, 0xcc, 0xd3, 0xd4,
	0x0,  0xd2, 0xda, 0xdb, 0xd9, 0x00, 0x88, 0x98,
	0xaf, 0x0,  0x0,  0x0,  0xb8, 0x0,  0x0,  0x0
};
#endif /* _FULL_XLAT_TABLE */

/* --------------------------------------------------------------- */
unsigned short MACToDTE(unsigned short usym)
{
	int i;
	usym &= 0xff;
	if (usym < 0x80)
	{
		return usym;
	}
	i = 0;
	while (OSToRecXlatTable[i][0] != 0)
	{
		if (usym == (unsigned short) OSToRecXlatTable[i][0])
		{
			usym = (unsigned short) OSToRecXlatTable[i][1];
			break;
		}
		i++;
	}
	return usym;
}

/* --------------------------------------------------------------- */
unsigned short DTEToMAC(unsigned short usym)
{
	int i;
	usym &= 0xff;
	if (usym < 0x80)
	{
		return usym;
	}
	i = 0;
	while (OSToRecXlatTable[i][0] != 0)
	{
		if (usym == (unsigned short) OSToRecXlatTable[i][1])
		{
			usym = (unsigned short) OSToRecXlatTable[i][0];
			break;
		}
		i++;
	}
	return usym;
}

/* --------------------------------------------------------------- */
unsigned short MACToANSI(unsigned short usym)
{
#ifndef _FULL_XLAT_TABLE
	int i;
	usym &= 0xff;
	if (usym < 0x80)
	{
		return usym;
	}
	i = 0;
	while (OSToWinXlatTable[i][0] != 0)
	{
		if (usym == (unsigned short) OSToWinXlatTable[i][0])
		{
			usym = (unsigned short) OSToWinXlatTable[i][1];
			break;
		}
		i++;
	}
	return usym;
#else /*_FULL_XLAT_TABLE */
	usym &= 0xff;
	if (usym < 0x80)
	{
		return usym;
	}
	else
	{
		return (unsigned short)Mac2AnsiXlatTable[usym&0x7f];
	}
#endif /*_FULL_XLAT_TABLE */
} /* MACToANSI */

/* --------------------------------------------------------------- */
unsigned short ANSIToMAC(unsigned short usym)
{
#ifndef _FULL_XLAT_TABLE
	int i;
	usym &= 0xff;
	if (usym < 0x80)
	{
		return usym;
	}
	i = 0;
	while (OSToRecXlatTable[i][0] != 0)
	{
		if (usym == (unsigned short) OSToWinXlatTable[i][1])
		{
			usym = (unsigned short) OSToWinXlatTable[i][0];
			break;
		}
		i++;
	}
	return usym;
#else /*_FULL_XLAT_TABLE */
	usym &= 0xff;
	if (usym < 0x80)
	{
		return usym;
	}
	else
	{
		return (unsigned short)Ansi2MacXlatTable[usym&0x7f];
	}
#endif /*_FULL_XLAT_TABLE */
} /* ANSIToMAC */

/* --------------------------------------------------------------- */
unsigned short DTEToANSI(unsigned short usym)
{
	return MACToANSI(DTEToMAC(usym));
}

/* --------------------------------------------------------------- */
unsigned short ANSIToDTE(unsigned short usym)
{
	return MACToDTE(ANSIToMAC(usym));
}

/* --------------------------------------------------------------- */
