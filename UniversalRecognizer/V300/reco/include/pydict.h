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

#ifndef PYDICT_H_INCLUDED
#define PYDICT_H_INCLUDED

//typedef unsigned char uchar;

#include "pzdict.h"

//-------------- Defines -------------------------------------------------------

#define PYDICT_ID_LEN         16
#define PYDICT_VER_ID         (('1' << 0) | ('.' << 8) | ('1' << 16) | ('0' << 24))
#define PYDICT_VER_ID_LEN     4
#define PYDICT_VER_ID_PREV    (('1' << 0) | ('.' << 8) | ('0' << 16) | ('1' << 24))
#define PYDICT_TREE_ID_LEN    16

typedef struct
{
    char            stamp[5];
    unsigned char   bType;
    unsigned short  usDataOffset;
    unsigned int    dwDataSize;
} SRecDictFileHeader;

#define PYDICT_NOERR            0
#define PYDICT_ERR              1
#define PYDICT_OLDWORD          2

#define PYDICT_MAX_WORDLEN      HW_MAXWORDLEN
#define PZDICT_MAX_WORDLEN      HW_MAXWORDLEN

#define  DICT_INIT              1      /* Empty vertex (the only vertex in empty voc)*/


#define  LHDR_STEP_LOG          4     //6
#define  LHDR_STEP_MASK         0x0F //0x3F                        //0011 1111
#define  LHDR_STEP              (1<<LHDR_STEP_LOG)                 //64

#define  MAX_CHSET_LEN          80
#define  MAX_DVSET_LEN          32

#define  MIN_LONG_DVSET_NUM     16
#define  MIN_LONG_CHSET_NUM     64

//------------------Vertex Flags and Masks----------------------------/

#define  ONE_BYTE_FLAG          ((_UCHAR)0x80)				//1000 0000
#define  END_WRD_FLAG           ((_UCHAR)0x40)				//0100 0000
#define  ATTR_MASK              0x30						//0011 0000
													// FOR MERGED TREE
#define  CODED_DVSET_FLAG       ((_UCHAR)0x20)		//0010 0000
#define  SHORT_DVSET_NUM_FLAG   ((_UCHAR)0x10)		//0001 0000
#define  DVSET_NUM_MASK         0x0F				//0000 1111
#define  DVSET_LEN_MASK         0x0F				//0000 1111
#define  CODED_CHSET_FLAG       ((_UCHAR)0x80)		//1000 0000
#define  SHORT_CHSET_NUM_FLAG   ((_UCHAR)0x40)		//0100 0000
#define  CHSET_NUM_MASK         0x3F                //0011 1111

#define  SHORT_CHSET_LEN_FLAG   0x20					//not used
#define  SHORT_CHSET_LEN_MASK   0x1F					//not used
#define  SHORT_ECHSET_LEN_FLAG  0x08					//used in Plain Tree
#define  SHORT_ECHSET_LEN_MASK  0x07					//used in Plain Tree 

//in chset                                                   
#define  LAST_SYM_FLAG          ((_UCHAR)0x80)
//in dvset                                                 
#define  SHORT_VADR_FLAG        0x80				//1000 0000

//--------- Macros'y --------------------------------------------------------------

#define PutPYDictID(pV) ( *((unsigned long *)pV) = (unsigned long)PYDICT_VER_ID )

#define VBeg(pV) ((_UCHAR *)pV+PYDICT_VER_ID_LEN)

#define IsTreeMerged(pV)        ( ( *(int *)VBeg(pV) > 0 ) ? 1 : 0 ) 
#define PutTreeMerge(pV,b)      ( *(int *)VBeg(pV) = (b) ? 1 : 0 )

#define IsVocChanged(pV)        ( ( *(int *)VBeg(pV) < 0 ) ? 1 : 0 )
#define PutVocIsChanged(pV)     { if (IsTreeMerged(pV)==0) *(int *)VBeg(pV)=-1; }
 

#define PYDictHeaderSize(IsMerged) ( (IsMerged) ?  \
            PYDICT_VER_ID_LEN+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(int) :   \
            PYDICT_VER_ID_LEN+sizeof(int)+sizeof(int) )

#define PYDictGetVocHeaderSize(pV) ( PYDictHeaderSize(IsTreeMerged(pV)) )

#define PYDictGetGraphSize(pV) (*(int *)( VBeg(pV)+sizeof(int) ))
#define PYDictPutGraphSize(pV,s) ( PYDictGetGraphSize(pV) = s )
#define PYDictGetChsetTablSize(pV) (*(int *)( VBeg(pV)+sizeof(int)+sizeof(int) ))
#define PYDictPutChsetTablSize(pV,s) ( PYDictGetChsetTablSize(pV) = s )
#define PYDictGetDvsetTablSize(pV) (*(int *)( VBeg(pV)+sizeof(int)+sizeof(int)+sizeof(int) ))
#define PYDictPutDvsetTablSize(pV,s) ( PYDictGetDvsetTablSize(pV) = s )

#define PYDictGetGraph(pV)  ( (_UCHAR *)pV + PYDictGetVocHeaderSize(pV) )
#define PYDictGetChsetTabl(pV)( (void *)((_UCHAR *)PYDictGetGraph(pV)+PYDictGetGraphSize(pV)) )
#define PYDictGetDvsetTabl(pV)( (void *)((_UCHAR *)PYDictGetGraph(pV)+PYDictGetGraphSize(pV)+PYDictGetChsetTablSize(pV)) )


//--------- Proto --------------------------------------------------------------


int PYDictGetNextSyms(void *cur_fw, void *fwb, void *pd);
int PYDictAddWord(_UCHAR *word, _UCHAR attr, void **pd);
int PYDictCreateDict(void **pd);
int PYDictFreeDict(void **pd);
int PYDictLoadDict(_UCHAR *name, void **pd);
int PYDictSaveDict(_UCHAR *name, void *pd);
int PYDictCheckWord(_UCHAR *word, _UCHAR *status, _UCHAR *attr,void *pd);
int PYDictGetDictStatus(int *len, void *pd);
int PYDictGetDictMemSize(void *pVoc);

_INT PYDictSaveDictToFile( _UCHAR *name, p_VOID pVoc );
_INT PYDictLoadDictFromFile( _UCHAR *name, p_VOID _PTR pd );

#endif //PYDICT_H_INCLUDED


/* *************************************************************************** */
/* *       BRIEF DESCRIPTION                                                 * */
/* *************************************************************************** *

There are 2 types of PYDICT dictionary: PLAIN TREE and MERGED TREE.
PLAIN TREE is usual uncompressed dictionary tree; this type is used for
User Voc, since new words can be easily added to PLAIN TREE.
PYDictCreateDict creates empty PLAIN TREE with PYDICT_MAX_WORDLEN levels;
PYDictAddWord adds words to it. Other PYDict functions work with both dict types.


MERGED TREE represents a Deterministic Finite State Machine with minimum number
of states generating list of words L,
i.e. it is a Labeled (i.e. with a letter on each edge) Directed Acyclic Graph G,
satisfying the following conditions:
(1) Every full path of G represents a word from list L;
(2) Every word from list L is represented by a full path of G;
(3) Any 2 edges with common starting node are labeled by different symbols;
(4) G has minimal (with respect to first 3 properties) number of nodes.

Merged Tree is constructed from Plain Tree first by merging leaves (rank 0),
then by merging appropriate nodes of rank 1, and so on, (here node rank is
defined by max path length from node to a leaf).

All edges of final graph G can be divided into 2 sets:
1) non-diagonal (or nd_childs): these are edges from initial tree,
each of them lead to a first-in-a-set-of-merging-nodes.
2) diagonal (or d_childs), which appear in the process of merging.

As graph G without diagonal edges form a tree structure, it can be represented
in a similar to PZDICT format:
All nodes are ordered with respect to this tree structure.
Graph header contains relative pointers to each level and number of
nodes in prev levels.
Each level header contains rel. pointer and number of prev nd_childs
for each LHDR_STEP-th node, thus # of first (and other) nd_child of a
node can be easily calculated by scanning only prev nodes in corresponding
segment of LHDR_STEP length.
Thus every node should contain only (a) list of symbols for all childs
(nd_childs symbols - first) [chset], (b) list of addresses (#-s in graph) for
d_childs [dvset].

Those chsets and dvsets, which are frequently used, are coded: sets are
extracted in ChsetTabl and DvsetTabl; corresponding nodes in Graph contain
only # of a set in a table. (# of a coded set, length of an uncoded
dvset and # of a vertex in a dvset can be written down in either long or
short form, with corresponding one bit flag).

Sets in Tabls are ordered according to their length; for each length
there is an entry in Tabl header, which contains length and # and rel.
pointer to the first set of this length.


Spec. notes:
1. In Plain Tree length of (uncoded) chset is indicated in a node before
the chset, either in short or long form.  In Merged Tree length is not
indicated, last sym in chset is marked by LAST_SYM_FLAG. Thus,
chsets, containing sym>=128, should be coded.
2. END_WRD_FLAG is instead additional '\n'-child.
3. One byte node has one child, non-diag, with sym<128; no END_WRD.

4. PLAIN TREE always has PYDICT_MAX_WORDLEN levels; MERGED TREE has only necessary
(non-empty) levels.

*************************************************************************** */



