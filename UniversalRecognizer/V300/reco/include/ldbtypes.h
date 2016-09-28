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

#ifndef _LDBTYPES_H
#define _LDBTYPES_H

#define LdbLast       0x20000000   /* This is last terminal in sequence */
#define LdbAllow      0x40000000   /* This state is allowing            */
#define LdbCont       0x80000000   /* This rule have alternatives       */
#define LdbMask       0x0fffffff

#define LdbFileSignature "LDB0"
#define SizeofLdbFileSignature 4

#define nStateLimDef    512

typedef struct tagLDBHeader
{
    _CHAR  sign[SizeofLdbFileSignature];
    _ULONG nRules;
    _ULONG fileSize;
    _ULONG extraBytes;
} LDBHeader, *PLDBHeader;

typedef struct tagLDBRule
{
    _ULONG strOffset;
    _ULONG state;
} LDBRule, *PLDBRule;

typedef struct tagAutoState
{
    _ULONG choice;
    _ULONG state;
} AutoState, _PTR Automaton;

typedef struct tagLdb
{
    struct tagLdb _PTR next;
    Automaton am;
} Ldb, _PTR p_Ldb;

typedef struct tagStatemap
{
    p_ULONG pulStateMap;  /* State remapping from abstract one to specific ones */
    _INT    nLdbs;        /* Number of ldbs in chain */
    _INT    nStateLim;    /* Number of state slots in pulStateMap */
    _INT    nStateMac;    /* Last allocated state */
    /* Helper fields */
    /* They are included in this structure because of similar life time */
    /* Logically, they are independent of first part */
    p_UCHAR sym;
    p_UCHAR l_status;
    p_ULONG pstate;
    _INT    nSyms;
} StateMap, _PTR p_StateMap;


//extern StateMap sm;

#endif /* _LDBTYPES_H */
