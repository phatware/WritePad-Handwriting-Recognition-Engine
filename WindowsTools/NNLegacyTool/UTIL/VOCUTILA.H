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

#ifndef VOCUTILA_INCLUDED

#define VOCUTILA_INCLUDED

#if AIRUS_VOC

typedef struct
{
	_HMEM         hvoc;
	_UCHAR        vocname[80];

	_UCHAR        n_ddict;
	_UCHAR        n_idict;
	_ULONG        dir_IDs[MAX_DICT];
	_ULONG        inv_IDs[MAX_DICT];

	_CHAR         Buffer[80];

	_HMEM         h_attribute_table;
	p_UCHAR huge  attribute_table;

	_HMEM         h_secloc_table;
	p_UCHAR huge  secloc_table;

	_HMEM         h_seccount_table;
	p_UCHAR huge  seccount_table;

	_HMEM         h_parc_buf;
	p_UCHAR huge  parc_buf;

	AirusAParmBlock p;
	ParcSpeller     s;
} vocptr_type;

_SHORT    voc_load(p_UCHAR vocname, _VOID _PTR _PTR vp);
_SHORT    voc_save(p_CHAR ptr, vocptr_type _PTR vp);

_BOOL     LockVocabularies( p_VOID vp );
_BOOL     UnlockVocabularies( p_VOID vp );

_SHORT    voc_unload(_VOID _PTR _PTR vp);

_SHORT    add_word(p_UCHAR inp_word, _SHORT stat, vocptr_type _PTR vp);
_SHORT    del_word(p_UCHAR inp_word, vocptr_type _PTR vp);

_USHORT   word_search(p_UCHAR word, p_SHORT attr, vocptr_type _PTR vp);

#endif

#endif  /*  VOCUTILA_INCLUDED.  */

/* ************************************************************************* */
/* * End of all                                                            * */
/* ************************************************************************* */
