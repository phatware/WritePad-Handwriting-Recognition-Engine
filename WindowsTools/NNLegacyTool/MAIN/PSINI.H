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

#ifndef _PSINI_H
#define _PSINI_H

#include "bastypes.h"

_UCHAR OrtographFlag[]        = "Ortograph Flag";
_UCHAR OrtographBase[]        = "Ortograph Base";
_UCHAR Reject[]               = "Reject";

_UCHAR Recognition_mode[]     = "Recognition mode";
_UCHAR Word_algorithm[]       = "Word algorithm";

_UCHAR Enabled_charset[]      = "Enabled charset";
_UCHAR Enabled_writing_ways[] = "Enabled writing ways";
_UCHAR Enabled_languages[]    = "Enabled languages";

_UCHAR Sure_level[]           = "Sure level";
_UCHAR Answer_level[]         = "Answer level";
_UCHAR Answer_allow[]         = "Answer allow";

_UCHAR Caps_mode[]            = "Caps mode";
_UCHAR Use_length_limits[]    = "Use length limits";

_UCHAR Xcorr_amnesty[]        = "Xcorr amnesty";
_UCHAR Compare_mode[]         = "Compare mode";

_UCHAR XrWord_mode[]          = "XrWord mode";
_UCHAR XrWord_tag_size[]      = "XrWord tag size";
_UCHAR Bad_distance[]         = "Bad distance";

_UCHAR Xrw_min_wlen[]         = "Min word len";
_UCHAR Xrw_max_wlen[]         = "Max word len";

_UCHAR Fly_learn[]            = "Fly learn";
_UCHAR Use_vars_info[]        = "Use vars info";

_UCHAR Lrn_class_level[]      = "Lrn class level";
_UCHAR Min_class_size[]       = "Min class size";
_UCHAR Learn_suff_vars[]      = "Learn suff. vars";
_UCHAR Voc_var_reward[]       = "Voc. var. reward";

_UCHAR DTE_file_name[]        = "DTE file name";
_UCHAR Triads_name[]          = "Triads name";
_UCHAR Vocabulary_name[]      = "Vocabulary name";
_UCHAR PrefVoc_name[]         = "PrefixVoc name";
_UCHAR SuffVoc_name[]         = "SuffixVoc name";
_UCHAR UserVoc_name[]         = "UserVoc name";

/* VIC May 21 1994 */
_UCHAR LexDict_name[]         = "LexDict name";
_UCHAR _Alpha_charset[]       = "Alpha charset";
_UCHAR _Number_charset[]      = "Number charset";
_UCHAR _Math_charset[]        = "Math charset";
_UCHAR _Leading_puctuation[]  = "Leading puctuation";
_UCHAR _Ending_puctuation[]   = "Ending puctuation";
_UCHAR _Other_symbols[]       = "Other symbols";

_UCHAR _Alpha_no_diacritics[] = "Alpha no diacritics";
_UCHAR _Alpha_international[] = "Alpha international";

_UCHAR Bedit_border[]         = "Bedit border";
_UCHAR num_border[]           = "num border";
_UCHAR sep_letter[]           = "separate letter";
_UCHAR small_caps[]           = "small capitals";

_UCHAR twordcut[]             = "wordcut";
_UCHAR cut_fly[]              = "cut_fly";
_UCHAR Wordcut_Tune[]         = "Wordcut Tune";

_UCHAR post[]                 = "post";
_UCHAR PPR_file_name[]        = "PPR file name";
_UCHAR PPR_min_weight[]       = "PPR min weight";
_UCHAR PPR_max_distance[]     = "PPR max distance";

_UCHAR modetext[]             = "t";
_UCHAR modeformula[]          = "f";

_UCHAR _XrWS[]                = "XrWS";
_UCHAR _XrSpl[]               = "XrSpl";
_UCHAR _XrSL[]                = "XrSl";
_UCHAR _XrLWS[]               = "XrLWS";
_UCHAR _XrLW[]                = "XrLW";
_UCHAR _XrExp[]               = "XrExp";

_UCHAR defcharset[]           = "nleo";
_UCHAR defxrwsmode[]          = "vc";
_UCHAR def_eww[]              = "g";
_UCHAR def_languages[]        = "e";

_UCHAR defacharset[]          = "abcdefghijklmnopqrstuvwxyz'";
_UCHAR defaintcharset[]       = "abcdefghijklmnopqrstuvwxyz\x8A\x9A\x8C'";
_UCHAR defandcharset[]        = "abcdefghijklmnopqrstuvwxyz\xA7\x8D'";
_UCHAR defncharset[]          = "012345689";
_UCHAR defmcharset[]          = "+-=()$%*/.,";
_UCHAR deflcharset[]          = "\'\"-(";
_UCHAR defecharset[]          = "\'\".,!-\?:);";
_UCHAR defocharset[]          = ":.-%,+=$@#&£()";

_UCHAR none[]                 = "NONE";
/* VIC Mar 31 1994 Begin */
_UCHAR Use_chunk_processor[]  = "Use chunk processor";
_UCHAR Chunk_letters[]        = "Chunk letters";
/* VIC Mar 31 1994 End */
_UCHAR multiply_des[]         = "multiply des";
#endif /* _PSINI_H */
