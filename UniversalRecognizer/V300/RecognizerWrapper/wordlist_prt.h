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
/* *    PhatWare WritePad handwriting recognition engine                               * */
/* *    Copyright (c) 1997-2014 PhatWare(r) Corp. All rights reserved.                 * */
/* ************************************************************************************* */

/* ************************************************************************************* *
 *
 * File: Wordlist
 *
 * Unauthorized distribution of this code is prohibited.
 * Contractor/manufacturer is PhatWare Corp.
 * 1314 S. Grand Blvd. Ste. 2-175 Spokane, WA 99202
 *
 * ************************************************************************************* */

#pragma once

static const char * g_szDefWords_PRT[] = {
	
	"adimiss\xe3o",	"admiss\xe3o",
	"adimitir",	"admitir",
	"adivogado",	"advogado",
	"aer",	"era",
	"algmu",	"algum",
	"ams",	"mas",
	"an",		"na",
	"aqeule",	"aquele",	
	"aqiulo",	"aquilo",
	"asim",	"assim",
	"assin",	"assim",
	"aue",	"que",
	"augum",	"algum",
	"augun",	"algum",
	"ben",	"bem",
	"bon",	"bom",
	"cafe",	"caf\xe9",
	"cliche",	"clich\xe9",
	"comun",	"comum",
	"con",	"com",
	"cppara",	"para",
	"danca",	"dan\xe7\x61",
	"decer",	"descer",
	"definitamente",	"definitivamente",
	"detale",	"detalhe",
	"deven",	"devem",
	"distingeu",	"distingue",
	"dsa",	"das",
	"dze",	"dez",
	"eent\xe3o",	"e ent\xe3o",
	"emb",	"bem",
	"ems",	"sem",
	"emu",	"meu",
	"equ",	"que",
	"ero",	"erro",
	"erv",	"ver",
	"esata",	"exacta",
	"ese",	"esse",
	"esu",	"seu",
	"euq",	"que",
	"excess\xe3o",	"excep\xe7\xe3o",
	"exeplo",	"exemplo",
	"ezata",	"exacta",
	"ezemplo",	"exemplo",
	"fas",	"faz",
	"faser",	"fazer",
	"foce",	"fosse",
	"ha",	"h\xe1",
	"interesado",	"interessado",
	"interese",	"interesse",
	"iso",	"isso",
	"isot",	"isto",
	"ja",	"j\xe1",
	"jente",	"gente",
	"linah",	"linha",
	"maz",	"mas",
	"mbe",	"bem",
	"mco",	"com",
	"melior",	"melhor",
	"mema",	"mesma",
	"mes",	"m\xeas",
	"miu",	"mil",
	"mque",	"quem",
	"msa",	"mas",
	"mte",	"tem",
	"mu",	"um",
	"mue",	"meu",
	"muinto",	"muito",
	"nao",	"n\xe3o",
	"nehum",	"nenhum",
	"nenhun",	"nenhum",
	"nesa",	"nessa",
	"nese",	"nesse",
	"nimho",	"ninho",
	"noso",	"nosso",
	"oa",	"ao",
	"obiter",	"obter",
	"observacao",	"observa\xe7\xe3o",
	"ocm",	"com",
	"opr",	"por",
	"orijem",	"origem",
	"orijen",	"origem",
	"otra",	"outra",
	"otro",	"outro",
	"pesquizar",	"pesquisar",
	"poden",	"podem",
	"precisan",	"precisam",
	"propio",	"pr\xf3prio",
	"propo",	"propor",
	"pr\xf3pio",	"pr\xf3prio",
	"qaundo",	"quando",
	"qeu",	"que",
	"qmue",	"quem",
	"qque",	"que",
	"quado",	"quando",
	"quiz",	"quis",
	"rae",	"era",
	"rev",	"ver",
	"rpo",	"por",
	"rse",	"ser",
	"sao",	"s\xe3o",
	"sau",	"sal",
	"sbo",	"sob",
	"sda",	"das",
	"sen",	"sem",
	"sia",	"sai",
	"significatimente",	"significativamente",
	"sinceranete",	"sinceramente",
	"sre",	"ser",
	"tanbem",	"tamb\xe9m",
	"tanb\xe9m",	"tamb\xe9m",
	"tda",	"toda",
	"tdo",	"todo",
	"ten",	"tem",
	"tit\xfalo",	"t\xedtulo",
	"ue",	"eu",
	"unica",	"\xfanica",
	"unico",	"\xfanico",
	"utilisar",	"utilizar",
	"utilise",	"utilize",
	"uza",	"usar",
	"uzo",	"uso",
	"vega",	"veja",
	"ven",	"vem",
	"viage",	"viagem",
	"vja",	"veja",
	"wue",	"que",

};

#define DEFWORDCNT_PRT  (sizeof(g_szDefWords_PRT)/sizeof(g_szDefWords_PRT[0]))


