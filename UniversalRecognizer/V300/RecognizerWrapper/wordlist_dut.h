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

static const char * g_szDefWords_DUT[] = {
	"apropos",	"\xe0 propos",	
	"aanrijken",	"aanreiken",	
	"abbonnee",		"abonnee",	
	"abbonnement",		"abonnement",	
	"abcent",		"absent",	
	"abusiefelijk",		"abusievelijk",	
	"accellereren",		"accelereren",	
	"accomodatie",		"accommodatie",	
	"acorderen",		"accorderen",	
	"adendum",		"addendum",	
	"aggressie",		"agressie",	
	"agregaat",		"aggregaat",	
	"akademikus",		"academicus",	
	"aksepteren",		"accepteren",	
	"altans",		"althans",	
	"amandement",		"amendement",	
	"anderzins",		"anderszins",	
	"apartement",		"appartement",	
	"apiritief",		"aperitief",	
	"applaudiseren",		"applaudisseren",	
	"asperine",		"aspirine",	
	"barbeque",		"barbecue",	
	"barri\xebre",		"barri\xe8re",	
	"beamte",		"beambte",	
	"bedriegelijk",		"bedrieglijk",	
	"bedrijfsschap",		"bedrijfschap",	
	"bonefide",		"bonafide",	
	"brilliant",		"briljant",	
	"burgelijk",		"burgerlijk",	
	"carri\xebre",		"carri\xe8re",	
	"commit\xe9",		"comit\xe9",
	"concureren",		"concurreren",	
	"contr\xf4le",		"controle",	
	"dichtsbijzijnde",		"dichtstbijzijnde",	
	"eensgezinswoning",		"eengezinswoning",	
	"enigzins",		"enigszins",	
	"ergenis",		"ergernis",	
	"ethnisch",		"etnisch",	
	"extravegant",		"extravagant",	
	"fabricant",		"fabrikant",	
	"faillisement",		"faillissement",	
	"falikant",		"faliekant",	
	"feitenlijk",		"feitelijk",	
	"frusteren",		"frustreren",	
	"geboortenbeperking",		"geboortebeperking",	
	"gedistileerd",		"gedistilleerd",	
	"gerechterlijke",	"gerechtelijke",	
	"gevoegelijk",		"gevoeglijk",	
	"geweldadig",		"gewelddadig",	
	"gezamelijk",		"gezamenlijk",	
	"hardstikke",		"hartstikke",	
	"hoogelijk",		"hooglijk",	
	"hopenlijk",		"hopelijk",	
	"houwvast",		"houvast",	
	"hte",		"het",	
	"impressario",		"impresario",	
	"interesant",		"interessant",	
	"intervieuw",		"interview",	
	"krediteren",		"crediteren",	
	"leiddraad",		"leidraad",	
	"milimeter",		"millimeter",	
	"millard",		"miljard",	
	"millieu",		"milieu",	
	"minitieus",		"minutieus",	
	"neit",		"niet",	
	"niemandalletje",		"niemendalletje",	
	"nochthans",		"nochtans",	
	"nummeriek",		"numeriek",	
	"ommelet",		"omelet",	
	"onmiddelijk",		"onmiddellijk",	
	"onsteltenis",		"ontsteltenis",	
	"permiteren",		"permitteren",	
	"pitoresk",		"pittoresk",	
	"posthuum",		"postuum",	
	"proced\xe9",		"proc\xe9\x64\xe9",	
	"recencent",		"recensent",	
	"recu",		"re\xe7u",	
	"repressaille",	"represaille",	
	"rethorische",		"retorische",	
	"revenue",		"revenu",	
	"sabbotage",		"sabotage",	
	"sateliet",		"satelliet",	
	"stylistisch",		"stilistisch",	
	"sumier",		"summier",	
	"tafreel",		"tafereel",	
	"uitwijden",		"uitweiden",	
	"voorwensel",		"voorwendsel",	
	"wezelijk",		"wezenlijk",	
	"woordvoerdster",		"woordvoerster",
};

#define DEFWORDCNT_DUT  (sizeof(g_szDefWords_DUT)/sizeof(g_szDefWords_DUT[0]))


