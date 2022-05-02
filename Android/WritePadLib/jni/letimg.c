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

#include <stdlib.h>
#include <ctype.h>
#include "RecognizerWrapper.h"
#include "recotypes.h"

#include "ShapeAPI.h"
#include "liintern.h"

#define LIG_NUMBITS					(sizeof((*((LIGStatesType*)0))[0]) * 8)
#define LIG_BITINDEX(let, gr)		((((let) - LIG_FIRST_LETTER) * LIG_LET_NUM_GROUPS + (gr)) * LIG_NUM_BITS_PER_GROUP)
#define LIG_BITSSHIFT(bitindex)		(LIG_NUMBITS - LIG_NUM_BITS_PER_GROUP - bitindex % LIG_NUMBITS)

static const char K_PAIRED_CHARS[][3] = {
	0xc7, 0xe7, 0,
	"()",
	"<>",
	",.",
	"\"'",
	"[]",
	"{}",
	0xab, 0xbb, 0,
	"/\\",
	0xa3, 0xa5, 0,
	0xaE, 0xa9, 0
};

static const char K_Y_DEPENDANT_CHARS[] = {'Y', 0xd0, 0x9f, 0};
static const char K_A_DEPENDANT_CHARS[] = {'A', 0xC0, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0};
static const char K_E_DEPENDANT_CHARS[] = {'E', 0xC8, 0xC9, 0xCA, 0xCB, 0};
static const char K_I_DEPENDANT_CHARS[] = {'I', 0xCC, 0xCD, 0xCE, 0xCF, 0};
static const char K_N_DEPENDANT_CHARS[] = {'N', 0xd1, 0};
static const char K_O_DEPENDANT_CHARS[] = {'O', 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0};
static const char K_U_DEPENDANT_CHARS[] = {'U', 0xD9, 0xDA, 0xDB, 0xDC, 0};
static const char K_a_DEPENDANT_CHARS[] = {'a', 0xE0, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0};
static const char K_e_DEPENDANT_CHARS[] = {'e', 0xE8, 0xE9, 0xEA, 0xEB, 0};
static const char K_i_DEPENDANT_CHARS[] = {'i', 0xEC, 0xED, 0xEE, 0xEF, 0};
static const char K_n_DEPENDANT_CHARS[] = {'n', 0xf1, 0};
static const char K_o_DEPENDANT_CHARS[] = {'o', 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0};
static const char K_u_DEPENDANT_CHARS[] = {'u', 0xF9, 0xFA, 0xFB, 0xFC, 0};
static const char K_y_DEPENDANT_CHARS[] = {'y', 0xfd, 0xff, 0};

static const char *K_DEPENDANT_CHARS[] = {
	K_A_DEPENDANT_CHARS,
	K_E_DEPENDANT_CHARS,
	K_I_DEPENDANT_CHARS,
	K_N_DEPENDANT_CHARS,
	K_O_DEPENDANT_CHARS,
	K_U_DEPENDANT_CHARS,
	K_Y_DEPENDANT_CHARS,
	K_a_DEPENDANT_CHARS,
	K_e_DEPENDANT_CHARS,
	K_i_DEPENDANT_CHARS,
	K_n_DEPENDANT_CHARS,
	K_o_DEPENDANT_CHARS,
	K_u_DEPENDANT_CHARS,
	K_y_DEPENDANT_CHARS	
};

const LIInfoType* 
LIGetLetterInfo(const LIDBType *lidb, int letter)
{
	US8_t	*letptr;
	int		nvar;
	long	nlet;
	long	i;

	if(lidb == (const LIDBType*)0) 
	{
		return ((const LIInfoType*)0);
	}
	letptr = (US8_t*)lidb;
	nlet = M_GetNumLetters(letptr);
	letptr += kLIHeaderSize;

	for (i = 0; i < nlet && M_GetLetHLetField(letptr) != letter; i++)
	{
		nvar = M_GetLetHNVarField(letptr);
		letptr += M_CalcLetHSize(nvar);
	}

	if (i >= nlet)
	{
		letptr = (US8_t*)0;
	}

	return (const LIInfoType*)letptr;
}

int
LIGetLetNumVar(const LIInfoType *letI)
{
	if (letI != (const LIInfoType*)0)
	{
		return M_GetLetHNVarField(letI);
	}
	return -1;
}

const LIVarType* 
LIGetVariantInfo(const LIDBType *lidb, const LIInfoType *letI, int variantIndex)
{
	if (lidb != (const LIDBType*)0 && letI != (const LIInfoType*)0 &&
		M_GetLetHNVarField(letI) > variantIndex)
	{
		return (const LIVarType*)((US8_t*)lidb + M_GetLetHOffsetField(letI, variantIndex));
	}
	return (const LIVarType*)0;
}   

int            
LIGetGroup(const LIVarType *letV)
{
	if (letV != (const LIVarType*)0) 
	{
		return M_GetLVGroupNumField(letV);
	}
	return -1;
}

int            
LIGetNumStrokes(const LIVarType *letV)
{
	if (letV != (const LIVarType*)0)
	{
		return M_GetLVStrkNumField(letV);
	}
	return -1;
}

const LIStokeType*     
LIGetStrokeInfo(const LIDBType *lidb, const LIVarType *letV, int strokeIndex)
{
	if (lidb != (const LIDBType*)0 && letV != (const LIVarType*)0 &&
		M_GetLVStrkNumField(letV) > strokeIndex) 
	{
		return (LIStokeType*)((US8_t*)lidb + M_GetLVStrokeOffsetField(letV, strokeIndex));
	}
	return (const LIStokeType*)0;
}

int            
LIGetNumPoints(const LIStokeType *letS)
{
	if (letS != (const LIStokeType*)0)
	{
		return M_GetLSNumPointsField(letS);
	}
	return -1;
}

int            
LIGetPointX(const LIStokeType *letS, int pointIndex)
{
	if (letS != (const LIStokeType*)0 && M_GetLSNumPointsField(letS) > pointIndex)
	{
		return M_GetLSStrkPtXField(letS, pointIndex);
	}
	return -1;
}

int            
LIGetPointY(const LIStokeType *letS, int pointIndex)
{
	if (letS != (const LIStokeType*)0 && M_GetLSNumPointsField(letS) > pointIndex)
	{
		return M_GetLSStrkPtYField(letS, pointIndex);
	}
	return -1;
}

int            
LIGetVariantBBox(const LIDBType *lidb, const LIVarType *letV, LIRectType *bbox)
{
#ifdef __CALCULATE_LETTER_BBOX
	int                i,j,h,nstrks,npts;
	const LIStokeType  *letS;
#endif /* __CALCULATE_LETTER_BBOX */

	if(letV == (const LIVarType*)0 || lidb == (const LIDBType*)0)
	{
		return LIError;
	}
#ifndef __CALCULATE_LETTER_BBOX
	bbox->top    = LItop;
	bbox->bottom = LIbottom;
	bbox->left   = LIleft;
	bbox->right  = LIright;
#else
	bbox->top    = LItop;
	bbox->bottom = LIbottom;
	bbox->left   = 0x7FFF;
	bbox->right  = -(0x7FFF);
   
	nstrks = M_GetLVStrkNumField(letV);
	for(i=0; i < nstrks; i++) 
	{   
		letS = LIGetStrokeInfo(lidb, letV, i);
		if(letS == 0) 
		{
			return LIError;
		}

		npts = M_GetLSNumPointsField(letS);
		for(j=0; j < npts; j++) 
		{
			h = M_GetLSStrkPtXField(letS,j);
			if(h < bbox->left)    bbox->left   = h;
			if(h > bbox->right)   bbox->right  = h;
		}
	}
	if ( bbox->left == bbox->right )   
		bbox->right++;
#endif /* __CALCULATE_LETTER_BBOX */

	return LINoError;
}

int            
LIGetVariantBaseLine(const LIDBType *lidb, const LIVarType *letV, LIRectType *baseRect)
{
	if(letV == (const LIVarType *)0 || lidb == (const LIDBType*)0 ||
		LIGetVariantBBox(lidb, letV, baseRect) == LIError)
	{
		return(LIError);
	}

	baseRect->top    = LItop+(LIbottom-LItop)/3;
	baseRect->bottom = LIbottom-(LIbottom-LItop)/3;

	return LINoError;   
}
 
int            
LICalcLetterLayout(
				   const LIDBType *inLidb,
				   int            inLetter,
				   LILayoutType   *outLayout,
				   LIRectType     *ioDestRect,
				   int            inLetHSize,
				   int            inLetVSize,
				   int            inLineHeight,
				   int            inGroupHSpace,
				   int            inSelOvalSize)
{
	const LIInfoType *leti;
	const LIVarType	 *letv;
	int              i;
	int              k;
	int              m;
	int              nvar;
	int              sorted;
	int              group;
	int              selinset;
	int              groupspace;
	int              lwidth;
	LIRectType       destrect;
	LIRectType       currect;
	LIVarSortType    letgroup[LI_MAX_LET_IMG];
	LIVarSortType    scratch;
	
	
	outLayout->letter = 0;
	outLayout->numVar = 0;
	outLayout->selectedGroupIndex = -1;
	if ((leti = LIGetLetterInfo(inLidb, inLetter)) == 0) 
	{
		return (LIError); /* no such letter */
	}
	
	if ((nvar = LIGetLetNumVar(leti)) > LI_MAX_LET_IMG) 
	{
		return(LIError);
	}
	for (k = 0; k < nvar; k++)
	{
		if ((letv = LIGetVariantInfo(inLidb, leti, k)) == 0)
		{
			return(LIError);
		}
		letgroup[k].group = LIGetGroup(letv);
		letgroup[k].index = k;
	}
	
	/* Sort by group number */
	for (sorted = 0; !sorted; )
	{
		for (sorted = 1, k = 1; k < nvar; k++)
		{
			if (letgroup[k].group < letgroup[k-1].group)
			{
				scratch = letgroup[k-1];
				letgroup[k-1] = letgroup[k];
				letgroup[k] = scratch;
				sorted = 0;
			}
		}
	}
	
	selinset = inSelOvalSize / 2;
	destrect = *ioDestRect;
	destrect.left += selinset + SELECTION_FRAME_PEN_SIZE / 2;
	destrect.top += selinset  + SELECTION_FRAME_PEN_SIZE / 2;
	destrect.right -= selinset + SELECTION_FRAME_PEN_SIZE / 2;
	destrect.bottom -= selinset + SELECTION_FRAME_PEN_SIZE / 2;
	ioDestRect->right = ioDestRect->left;
	ioDestRect->bottom = ioDestRect->top;
	
	lwidth = inLetHSize; 
	groupspace = inGroupHSpace + 2 * selinset;
	currect.left = destrect.left - groupspace;
	currect.right = currect.left + lwidth;
	currect.top = destrect.top;
	currect.bottom = currect.top + inLetVSize;
	for (m = 0, k = 0; k < nvar; m++) 
	{
		currect.left += groupspace;
		currect.right += groupspace;
		
		group = letgroup[k].group;
		for (i = k + 1; i < nvar && group == letgroup[i].group; i++)
		{}
		i -= k;
		/* Do we need to start a new line ? */
		if (k > 0 && currect.left + i * lwidth > destrect.right) 
		{
			currect.left  = destrect.left;
			currect.right = currect.left + lwidth;
			currect.top    += inLineHeight + 2 * selinset;
			currect.bottom += inLineHeight + 2 * selinset;
		}
		
		outLayout->groupRect[m].left = currect.left - selinset;
		outLayout->groupRect[m].top  = currect.top - selinset;
		outLayout->groupRect[m].right  = currect.left + i * lwidth + selinset;
		outLayout->groupRect[m].bottom  = currect.bottom + selinset;
		if (ioDestRect->right < outLayout->groupRect[m].right) 
		{
			ioDestRect->right = outLayout->groupRect[m].right;
		}
		if (ioDestRect->bottom < outLayout->groupRect[m].bottom + selinset) 
		{
			ioDestRect->bottom = outLayout->groupRect[m].bottom + selinset;
		}
		for (; k < nvar && group == letgroup[k].group; k++) 
		{
			outLayout->letterVar[k] = letgroup[k].index;
			outLayout->letterRect[k] = currect;
			currect.left += lwidth;
			currect.right += lwidth;
		}
	}
	outLayout->numGroup = m;
	outLayout->letter = inLetter;
	outLayout->numVar = nvar;
	outLayout->selOvalSize = inSelOvalSize;
	ioDestRect->right += SELECTION_FRAME_PEN_SIZE / 2;
	ioDestRect->bottom += SELECTION_FRAME_PEN_SIZE / 2;

	return(LINoError);
}

static char GetPairedChar( char inChar)
{
	char outch = 0;
    int   i_inChar;
	
	if (inChar == 0) 
    {
		return outch;
	}
    
    i_inChar = (0x00FF & inChar);
	if ( (outch = (char)tolower( i_inChar )) == inChar &&
		(outch = (char)toupper( i_inChar )) == inChar )
    {
		outch = 0;
		for ( int i = 0; i < LI_ARRAY_LENGTH(K_PAIRED_CHARS); i++) 
        {
			if (inChar == K_PAIRED_CHARS[i][0]) 
            {
				outch = K_PAIRED_CHARS[i][1];
				break;
			}
			if (inChar == K_PAIRED_CHARS[i][1]) 
            {
				outch = K_PAIRED_CHARS[i][0];
				break;
			}
		}
	}
	return outch;
}

int LIGetLetNumOfGroup( const LILayoutType *inLayout)
{
	return(inLayout->numGroup);
}

int LIGetDTELetGroup(
				 const LIDBType     *inLidb,
				 const LILayoutType *inLayout,
				 int                inGroupIndex)
{
	return GetSetLetGroupParms(inLidb, (LILayoutType*)inLayout, inGroupIndex,
	                           LI_RARE, LI_GET_DTEGROUP);
}

int LIGetLetGroupState(
				   const LIDBType     *inLidb,
				   const LILayoutType *inLayout,
				   int                inGroupIndex)
{
	return GetSetLetGroupParms(inLidb, (LILayoutType*)inLayout, inGroupIndex,
	                           LI_RARE, LI_GET_GROUP_STATE);
}

int LISetLetGroupState(
				   const LIDBType *inLidb,
				   LILayoutType   *ioLayout,
				   int            inGroupIndex,
				   enum E_LI_LETSTATE  inLetState)
{
	return GetSetLetGroupParms(inLidb, ioLayout, inGroupIndex,
	                           inLetState, LI_SET_GROUP_STATE );
}

int LIHitTestLetterLayout(
					  const LILayoutType *inLayout,
					  int                inX,
					  int                inY,
					  int                inIsTestGroup)
{
	const LIRectType *lirect;
	int              nrect;
	int              i;
	
	if (inIsTestGroup) 
	{
		lirect = inLayout->groupRect;
		nrect = inLayout->numGroup;
	}
	else 
	{
		lirect = inLayout->letterRect;
		nrect = inLayout->numVar;
	}
	
	for (i = 0; i < nrect; i++, lirect++) 
	{
		if (inX >= lirect->left && inX <= lirect->right &&
			inY >= lirect->top && inY <= lirect->bottom) 
		{
			return i;
		}
	}
	
	return -1;
}


int GetSetLetGroupParms(
					const LIDBType  *inLidb,
					LILayoutType    *ioLayout,
					int             inGroupIndex,
					enum E_LI_LETSTATE   inLetState,
					enum E_LET_GROUPMODE inMode)
{
	const LIInfoType *leti;
	const LIVarType	 *letv;
	int              groupindex;
	int              group;
	int              prevgroup;
	int              i;
	
	if (ioLayout->numGroup <= inGroupIndex ||
	    (leti = LIGetLetterInfo(inLidb, ioLayout->letter)) == 0)
	{
		return (LIError); /* no such letter */
	}
	
	groupindex = -1;
	prevgroup = -1;
	for (i = 0; i < ioLayout->numVar && groupindex <= inGroupIndex; i++) 
	{
		if ((letv = LIGetVariantInfo(inLidb, leti, ioLayout->letterVar[i])) == 0)
		{
			return (LIError);
		}
		if ((group = LIGetGroup(letv)) != prevgroup)
		{
			prevgroup = group;
			groupindex++;
		}
		if (groupindex == inGroupIndex) 
		{
			if (inMode == LI_SET_GROUP_STATE) 
			{
				ioLayout->letState[i] = inLetState;
			} 
			else if (inMode == LI_GET_GROUP_STATE) 
			{
				return ioLayout->letState[i];
			}
			else if (inMode == LI_GET_DTEGROUP)
			{
				return group;
			}
			else 
			{
				return(LIError);
			}
		}
	}
	return(LINoError);
}

int LIGetLetGroupRect(
				  const LILayoutType *inLayout,
				  int                inGroupIndex,
				  LIRectType         *outRect)
{
	if (inGroupIndex < 0 ||
	    inGroupIndex >= inLayout->numGroup ||
		inGroupIndex > LI_MAX_LET_IMG)
	{
		outRect->left = 0;
		outRect->top = 0;
		outRect->right = 0;
		outRect->bottom = 0;
		return LIError;
	}
	
	*outRect = inLayout->groupRect[inGroupIndex];
	return(LINoError);
}

int LISelelectGroup( LILayoutType  *ioLayout, int inGroupIndex)
{
	if (inGroupIndex > 0 &&
	    (inGroupIndex >= ioLayout->numGroup || inGroupIndex > LI_MAX_LET_IMG))
	{
		return LIError;
	}
	else if (inGroupIndex < 0) 
	{
		ioLayout->selectedGroupIndex = -1;
	}
	else 
	{
		ioLayout->selectedGroupIndex = inGroupIndex;
	}
	return(LINoError);
}

int LIGetSelectedGroup( const LILayoutType  *inLayout)
{
	return(inLayout->selectedGroupIndex);
}

static E_LIG_STATE GetGroupState(
				 const LIGStatesType *inGStates,
				 int                 inLetter,
				 int                 inGroup)
{
	int i;
	int shift;
	
	if (inLetter < LIG_FIRST_LETTER ||
	    inLetter > LIG_LAST_LETTER  ||
		inGroup < 0 ||
		inGroup > LIG_LET_NUM_GROUPS)
	{
		return LIG_STATE_UNDEF;
	}
	
	i = LIG_BITINDEX(inLetter, inGroup),
	shift = LIG_BITSSHIFT(i);
	i /= LIG_NUMBITS;
	return ((E_LIG_STATE)(((*inGStates)[i] >> shift) & LIG_NUM_BIT_GROUP_MASK));
}

		
static void DTEGetGroupStates(
					   const LIDBType      *inLidb,
					   int                 inLetter,
					   LILayoutType        *ioLayout,
					   const LIGStatesType *inGStates)
{
	int           i;
	int           numgroup;
	int           dtegroup;
	E_LIG_STATE   ligs;
		
	numgroup = LIGetLetNumOfGroup(ioLayout);
	for ( i = 0; i < numgroup; i++ ) 
    {
        dtegroup = LIGetDTELetGroup(inLidb, ioLayout, i);
        ligs = GetGroupState(inGStates, inLetter, dtegroup);
		
        LISetLetGroupState(inLidb, ioLayout, i,
									   ligs == LIG_STATE_OFTEN ? LI_OFTEN :
						   (ligs == LIG_STATE_RARELY ? LI_SOMETIMES : LI_RARE) );
    }
}

static int DTESetGroupState(
				 LIGStatesType *ioGStates,
				 int           inLetter,
				 int           inGroup,
				 E_LIG_STATE   inGroupState)
{
	int i;
	int shift;
	
	if (inLetter < LIG_FIRST_LETTER ||
	    inLetter > LIG_LAST_LETTER  ||
		inGroup < 0 ||
		inGroup > LIG_LET_NUM_GROUPS) 
	{
		return -1;
	}
	
	i = LIG_BITINDEX(inLetter, inGroup);
	shift = LIG_BITSSHIFT(i);
	i /= LIG_NUMBITS;
	(*ioGStates)[i] &= ~(LIG_NUM_BIT_GROUP_MASK << shift);
	(*ioGStates)[i] |= (inGroupState << shift);
	
	return 0;
}

		
int LIIntersectRect( const LIRectType *inSrc1, const LIRectType *inSrc2 )
{
	return (inSrc1->right > inSrc1->left && inSrc1->bottom > inSrc1->top &&
			inSrc2->right > inSrc2->left && inSrc2->bottom > inSrc2->top &&
			
			((inSrc1->left <= inSrc2->left && inSrc1->right >= inSrc2->left)   ||
			 (inSrc1->left <= inSrc2->right && inSrc1->right >= inSrc2->right) ||
			 (inSrc2->left <= inSrc1->left && inSrc2->right >= inSrc1->left)   ||
			 (inSrc2->left <= inSrc1->right && inSrc2->right >= inSrc1->right)) &&
			
			((inSrc1->top <= inSrc2->top && inSrc1->bottom >= inSrc2->top)       ||
			 (inSrc1->top <= inSrc2->bottom && inSrc1->bottom >= inSrc2->bottom) ||
			 (inSrc2->top <= inSrc1->top && inSrc2->bottom >= inSrc1->top)       ||
			 (inSrc2->top <= inSrc1->bottom && inSrc2->bottom >= inSrc1->bottom)));
}

enum E_LI_LETSTATE SelectNextGroupDtate( const LIDBType *lidb, LILetImgDrawType *ioLIdraw, LIGStatesType  *ioGStates )
{
	int            i;
	int            k;
	int            state, curstate;
	int            grind;
	int            numoften;
	int            dtegr;
	int            sym;
	E_LIG_STATE    grstate;
	LIRectType     lirect;
	char           let;
	LILayoutType   *layout;
	int            numgroup;
		
	for ( grind = -1, i = 0, layout = ioLIdraw->letimg;
		 i < LI_ARRAY_LENGTH(ioLIdraw->letter) &&
		 ioLIdraw->letter[i] != 0;
		 i++, layout++) 
    {
		if ((grind = LIGetSelectedGroup(layout)) >= 0) 
        {
			let = ioLIdraw->letter[i];
			break;
		}
	}
	
	if (grind < 0) 
    {
		return LI_OFTEN;
	}
	
	curstate = LIGetLetGroupState( lidb, layout, grind );
	if ( LI_OFTEN == curstate )
		state = LI_SOMETIMES;
	else if ( LI_SOMETIMES == curstate )
		state = LI_RARE;
	else
		state = LI_OFTEN;
	numgroup = LIGetLetNumOfGroup(layout);
	/* At least one group should be set to often */
	if (curstate == LI_OFTEN) 
	{
		for (numoften = 0, i = 0; i < numgroup; i++) 
		{
			if (LIGetLetGroupState(lidb, layout, i) == LI_OFTEN) 
			{
				numoften++;
			}
		}
		if (numoften <= 1) 
		{
			return curstate;
		}
	}
	LISetLetGroupState(lidb, layout, grind, state);
	dtegr = LIGetDTELetGroup(lidb, layout, grind);
	grstate = (state == LI_OFTEN) ? LIG_STATE_OFTEN : ((state == LI_SOMETIMES) ? LIG_STATE_RARELY : LIG_STATE_NEVER);
	
	for ( i = 0; i < LI_ARRAY_LENGTH(K_DEPENDANT_CHARS); i++) 
	{
		for ( k = 0; K_DEPENDANT_CHARS[i][k] != 0 && (int)K_DEPENDANT_CHARS[i][k] != let; k++ )
		{
		}
		
		if ( K_DEPENDANT_CHARS[i][k] != 0 ) 
		{
			break;
		}
	}
	if ( i < LI_ARRAY_LENGTH(K_DEPENDANT_CHARS))
	{
		for ( k = 0; K_DEPENDANT_CHARS[i][k] != 0; k++ ) 
		{
			sym = (int)(unsigned char)(K_DEPENDANT_CHARS[i][k]);
			if ( GetGroupState( (const LIGStatesType *)ioGStates, sym, dtegr ) == LIG_STATE_UNDEF ) 
			{
				continue;
			}
			DTESetGroupState(ioGStates, sym, dtegr, grstate);
		}
	} 
	else 
	{
		DTESetGroupState(ioGStates, (int)(unsigned char)(let), dtegr, grstate);
	}
	
	LIGetLetGroupRect(layout, grind, &lirect);
	return state;
}



void CalcLetterLayout(
				   LIRectType         *ioDestRect,
				   char                inLetter,
				   LILetImgDrawType    *outLayout,
				   const LIDBType      *inLidb,
				   const LIGStatesType *inGStates)
{
	LIRectType  dest={0};
	int         i;
	
	outLayout->letter[0] = inLetter;
	outLayout->letter[1] = GetPairedChar(inLetter);
	for (i = 0; i < LI_ARRAY_LENGTH(outLayout->letter) && outLayout->letter[i] != 0; i++) 
	{
		if (i == 0) 
		{
			dest.left = ioDestRect->left + LI_LET_IMG_LEFT_OFFSET;
			dest.top = ioDestRect->top;
			dest.right = ioDestRect->right;
			dest.bottom = ioDestRect->bottom;
		} 
		else 
		{
			outLayout->sepline[0].y = dest.bottom + LI_PAIRED_LET_V_SPACE / 2;
			outLayout->sepline[1].y = outLayout->sepline[0].y;
			outLayout->sepline[0].x = ioDestRect->left;
			outLayout->sepline[1].x = dest.right; 
			dest.top = dest.bottom + LI_PAIRED_LET_V_SPACE;
			dest.right = ioDestRect->right;
			dest.bottom = ioDestRect->bottom;
		}
		
		LICalcLetterLayout(inLidb,
							(int)(unsigned char)outLayout->letter[i],
							outLayout->letimg + i,
							&dest,
							LI_LET_SELL_HEIGHT,
							LI_LET_SELL_WIDTH,
							LI_LET_SELL_HEIGHT + LI_LET_LINE_V_SPACE,
							LI_LET_GROUP_H_SPACE,
							LI_LET_SEL_OVAL_SIZE );
		DTEGetGroupStates(inLidb,
		                  (int)(unsigned char)outLayout->letter[i],
					      outLayout->letimg + i,
						  inGStates);
		if (i == 0 || outLayout->sepline[1].x < dest.right) 
		{
			outLayout->sepline[1].x = dest.right;
		}
	}
	ioDestRect->bottom = dest.bottom;
	ioDestRect->right = outLayout->sepline[1].x;
	outLayout->framerect = *ioDestRect;
    outLayout->framerect.right++; //CHE - otherwise the dots are left on changind to the "shorter" letter
	
	LISelelectGroup(outLayout->letimg, 0);
}

/*
BOOL SetSelLetState(
					enum E_LI_LETSTATE    inState,
					LILetImgDrawType *ioLIdraw,
					LIGStatesType    *ioGStates )
{
	int            i;
	int            k;
	int            curstate;
	int            grind;
	int            numoften;
	int            dtegr;
	int            sym;
	E_LIG_STATE    grstate;
	LIRectType     lirect;
	char           let;
	LILayoutType   *layout;
	const LIDBType *lidb;
	int            numgroup;
	
	for (grind = -1, i = 0, layout = ioLIdraw->letimg;
		 i < LI_ARRAY_LENGTH(ioLIdraw->letter) &&
		 ioLIdraw->letter[i] != 0;
		 i++, layout++) 
    {
		if ((grind = LIGetSelectedGroup(layout)) >= 0) 
        {
			let = ioLIdraw->letter[i];
			break;
		}
	}
	
	if (grind < 0) 
    {
		return 1;
	}
	
	lidb = LIGetLetterDB( HWR_GetLanguageID() );
	curstate = LIGetLetGroupState(lidb, layout, grind);
	if (curstate != inState) 
    {
		numgroup = LIGetLetNumOfGroup(layout);
		// At least one group should be set to often 
		if (curstate == LI_OFTEN) 
        {
			for (numoften = 0, i = 0; i < numgroup; i++) 
            {
				if (LIGetLetGroupState(lidb, layout, i) == LI_OFTEN) 
                {
					numoften++;
				}
			}
			if (numoften <= 1) 
            {
				return 0;
			}
		}
		LISetLetGroupState(lidb, layout, grind, inState);
		dtegr = LIGetDTELetGroup(lidb, layout, grind);
		grstate = inState == LI_OFTEN ? LIG_STATE_OFTEN : 
		(inState == LI_SOMETIMES ? LIG_STATE_RARELY : LIG_STATE_NEVER);
		
		for ( i = 0; i < LI_ARRAY_LENGTH(K_DEPENDANT_CHARS); i++) 
        {
			for ( k = 0; K_DEPENDANT_CHARS[i][k] != 0 && (int)K_DEPENDANT_CHARS[i][k] != let; k++ )
			{
            }
			
			if ( K_DEPENDANT_CHARS[i][k] != 0 ) 
            {
				break;
			}
		}
		if ( i < LI_ARRAY_LENGTH(K_DEPENDANT_CHARS))
        {
			for ( k = 0; K_DEPENDANT_CHARS[i][k] != 0; k++ ) 
            {
				sym = (int)((unsigned char)K_DEPENDANT_CHARS[i][k]);
				if ( GetGroupState( (const LIGStatesType *)ioGStates, sym, dtegr) == LIG_STATE_UNDEF ) 
                {
					continue;
				}
				DTESetGroupState(ioGStates, sym, dtegr, grstate );
			}
		} 
        else 
        {
			DTESetGroupState(ioGStates, (int)(unsigned char)let, dtegr, grstate);
		}
		
		LIGetLetGroupRect(layout, grind, &lirect);
	}
	return 1;
}
*/

static long DivRound( long inNum, long inDenom )
{
	return (inNum / inDenom + 2 * (inNum % inDenom) / inDenom);
}

int CalculateScreenRect(
					const LIRectType *inBBox,
					const LIRectType *inDestRect,
					LIRectType       *outScreenRect)
{
	long   xDSize;
	long   yDSize;
	long   xSSize;
	long   ySSize;
	long   size;
	
	xDSize = (long)(inDestRect->right  - inDestRect->left - 2);
	yDSize = (long)(inDestRect->bottom - inDestRect->top - 2);
	xSSize = (long)(inBBox->right  - inBBox->left);
	ySSize = (long)(inBBox->bottom - inBBox->top);
	if(xDSize <= 0 || yDSize <= 0 || xSSize < 0 || ySSize < 0)
		return(LIError);
	
	if(ySSize == 0 && xSSize == 0)
	{
		outScreenRect->left   = (inDestRect->left + inDestRect->right)/2 -1;  /* Center Alignment */
		outScreenRect->right  = (inDestRect->left + inDestRect->right)/2 +1;  /* Center Alignment */
		outScreenRect->top    = (inDestRect->top + inDestRect->bottom)/2 -1;  /* Center Alignment */
		outScreenRect->bottom = (inDestRect->top + inDestRect->bottom)/2 +1;  /* Center Alignment */
		
		return(LINoError); 
	}
	if(xSSize == 0) xSSize++;
	if(ySSize == 0) ySSize++; 
	
	if( (xDSize<<16)/xSSize > (yDSize<<16)/ySSize)
	{
		if(xSSize == 1) 
            size = xDSize/2-1;
		else           
            size = (xDSize - xSSize*yDSize/ySSize) / 2;
		
		outScreenRect->left   = inDestRect->left  +size;  /* Center Alignment */
		outScreenRect->right  = inDestRect->right -size;  /* Center Alignment */
		outScreenRect->top    = inDestRect->top;
		outScreenRect->bottom = inDestRect->bottom;  
	}
	else
	{
		/* if XScale < YScale, => CoordScale = XScale */
		
		if(ySSize == 1) size = yDSize/2-1;
		else            size = (yDSize - ySSize*xDSize/xSSize) / 2;
		
		outScreenRect->left   = inDestRect->left;
		outScreenRect->right  = inDestRect->right;
		outScreenRect->top    = inDestRect->top    +size; /* Center Alignment */
		outScreenRect->bottom = inDestRect->bottom -size; /* Center Alignment */
	}
	
	return(LINoError);  
}

int ConvertToScreenCoord(
					 LIPointType      *ioPt,
					 const LIRectType *inSrcRect,
					 const LIRectType *inDestRect)
{
	long   xDSize;
	long   yDSize;
	long   xSSize;
	long   ySSize;
	
	xDSize = (long)(inDestRect->right  - inDestRect->left);
	yDSize = (long)(inDestRect->bottom - inDestRect->top);
	xSSize = (long)(inSrcRect->right   - inSrcRect->left);
	ySSize = (long)(inSrcRect->bottom  - inSrcRect->top);
	if(xSSize == 0) xSSize++;
	if(ySSize == 0) ySSize++;    
	
	if(xDSize <= 0 || yDSize <= 0 || xSSize <= 0 || ySSize <= 0) return(LIError);
	
	ioPt->x = (int)DivRound(((long)ioPt->x - (long)inSrcRect->left)* xDSize, xSSize) +
	inDestRect->left;
	ioPt->y = (int)DivRound(((long)ioPt->y - (long)inSrcRect->top) * yDSize, ySSize) +
	inDestRect->top;
	
	return(LINoError); 
}
