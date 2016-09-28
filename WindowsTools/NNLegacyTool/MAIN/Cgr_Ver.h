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

#ifndef _CGR_VER_H_INCLUDED
#define _CGR_VER_H_INCLUDED


#define VER_MAJOR_VERSION   1
#define VER_MINOR_VERSION   1
#define VER_RELEASE        44

#define VER_LITE_VERSION    0 // Version without recognizer by default(WACOM)
#define VER_TRIAL_VERSION   0 // Trial(restricted) version 

#define VER_FOR_WACOM       0 // Wacom specific strings/options

#define VER_CE              1
#define VER_95              2
#define VER_VER           VER_95

#define VER_RECPROTECTED    0

#define VER_PALK_DICT       0  // Switch between Elk dict (0) and Palk dict (1) format 

#define VER_RECINT_UNICODE  0  // Use UNICODE interface to the recognizer

// -------------- Rec engine defines -------------------------------------------

#define VER_DTI_COMPRESSED  0  // Include full blown DTI in engine build
#define VER_SNN_COMPRESSED  1  // Include expanded SNN net
#define VER_DICT_PEGDICT    0  // Include all dictionary-related functions in the Rec engine (file and spell func)

// -------------- Language selector --------------------------------------------
// Warning -- only one can be defined at a time

#define FOR_ENGLISH
//#define FOR_GERMAN
//#define FOR_FRENCH
//#define FOR_INTERNATIONAL

//------------------------------------------------------------------------------

#ifdef FOR_ENGLISH
#define VER_DEF_RECOGNIZER_NAME TEXT("CgrEnglish02.dll")
#define VER_DEF_MAIN_DICT_NAME  TEXT("CgrEng65k.dct")
#define VER_DEF_USER_DICT_NAME  TEXT("CgrUser.dct")
#endif //FOR_ENGLISH

#ifdef FOR_GERMAN
#define VER_DEF_RECOGNIZER_NAME TEXT("CgrGerman02.dll")
#define VER_DEF_MAIN_DICT_NAME  TEXT("CgrGer62k.dct")
#define VER_DEF_USER_DICT_NAME  TEXT("CgrUser.dct")
#endif //FOR_GERMAN

#ifdef FOR_FRENCH
#define VER_DEF_RECOGNIZER_NAME TEXT("CgrFrench02.dll")
#define VER_DEF_MAIN_DICT_NAME  TEXT("CgrFr144k.dct")
#define VER_DEF_USER_DICT_NAME  TEXT("CgrUser.dct")
#endif //FOR_FRENCH

#ifdef FOR_INTERNATIONAL
#define VER_DEF_RECOGNIZER_NAME TEXT("CgrInter02.dll")
#define VER_DEF_MAIN_DICT_NAME  TEXT("CgrInt65k.dct")
#define VER_DEF_USER_DICT_NAME  TEXT("CgrUser.dct")
#endif //INTERNATIONAL

// ------------------------ Redefines and errors -------------------------------
//--------- Support redifintions - no user serviceable defines inside! ---------
// -----------------------------------------------------------------------------

//------ Rec engine switches ---------------------------------------------------

#if VER_DTI_COMPRESSED
#define DTI_COMPRESSED_ON
#endif

#if !VER_SNN_COMPRESSED
#define SNN_FAT_NET_ON
#endif

#if VER_DICT_PEGDICT && !defined(PEGDICT)
#define PEGDICT
#endif


#if  VER_FOR_WACOM && !VER_LITE_VERSION
#error WACOM version should be declared as LITE!
#endif

#if VER_RECINT_UNICODE
#define PEG_RECINT_UNICODE
#endif


#endif // _CGR_VER_H_INCLUDED
/* ************************************************************************** */
/* * End of Versions.h                                                      * */
/* ************************************************************************** */
