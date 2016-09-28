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

#pragma once

static const char * g_szDefWords_ITL[] = {

	"anzich\xe8",		"anzich\xe9",
	"aquistare",	"acquistare",
	"areobica",		"aerobica",
	"areodinamica",	"aerodinamica",
	"areonauta",	"aeronauta",
	"areonautica",	"aeronautica",
	"areoplano",	"aeroplano",
	"areoporto",	"aeroporto",
	"autocoscenza",	"autocoscienza",
	"caff\xe9",		"caff\xe8",
	"caffe",		"caff\xe8",
	"ciliege",		"ciliegie",
	"cio\xe9",			"cio\xe8",
	"coefficente",	"coefficiente",
	"conoscienza",	"conoscenza",
	"coscente",		"cosciente",
	"coscenza",		"coscienza",
	"daccordo",		"d\x92\x61\x63\x63ordo",
	"daltronde",	"d'altronde",
	"dassi",		"dessi",
	"deficente",	"deficiente",
	"deficenza",	"deficienza",
	"dellaltra",	"dell'altra",
	"dellaltro",	"dell'altro",
	"domestichezza",	"dimestichezza",
	"d\xf2",			"do",
	"eccezzionale",	"eccezionale",
	"eccezzione",	"eccezione",
	"efficacie",	"efficace",
	"efficente",	"efficiente",
	"efficenza",	"efficienza",
	"essicare",		"essiccare",
	"fantascenza",	"fantascienza",
	"f\xe0",			"fa",
	"Htp",			"http",
	"Http",			"http",
	"http:\\",		"http://",
	"inbuto",		"imbuto",
	"incoscente",	"incosciente",
	"incoscenza",	"incoscienza",
	"inefficente",	"inefficiente",
	"inportare",	"importare",
	"laltro",		"l'altro",
	"metereologia",	"meteorologia",
	"metereologico",	"meteorologico",
	"nonch\xe8",		"nonch\xe9",
	"n\xe8",			"n\xe9",
	"oroficeria",	"oreficeria",
	"ostoporosi",	"osteoporosi",
	"perch\xe8",		"perch\xe9",
	"perlappunto",	"per l'appunto",
	"poich\xe8",		"poich\xe9",
	"pressapoco",	"pressappoco",
	"propio",		"proprio",
	"p\xf2",			"po\x92",
	"qual'\xe8",		"qual \xe8",
	"qualcun'altro",	"qualcun altro",
	"qualcunaltra",	"qualcun'altra",
	"qualcunaltro",	"qualcun altro",
	"qu\xe0",			"qua",
	"qu\xec",			"qui",
	"renumerare",	"remunerare",
	"scenza",		"scienza",
	"sopratutto",	"soprattutto",
	"sucessivo",	"successivo",
	"sufficente",	"sufficiente",
	"sufficenza",	"sufficienza",
	"s\xe0",			"sa",
	"s\xe8",			"s\xe9",
	"s\xf2",			"so",
	"s\xf9",			"su",
	"t\xe9",			"t\xe8",
	"un'altro",		"un altro",
	"un'unico",		"un unico",
	"vadi",			"vada",

	// TODO: add more  words...


};

#define DEFWORDCNT_ITL  (sizeof(g_szDefWords_ITL)/sizeof(g_szDefWords_ITL[0]))


