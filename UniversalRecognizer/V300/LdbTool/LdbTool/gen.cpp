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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "mytypes.h"
#include "table.h"
#include "parser.h"
#include "ldbtypes.h"
#include "gen.h"

static FILE *ofile = NULL;

static void CalcSizes(PParseResult pPR, ulong *pulSCSize, ulong *pnRules)
{
    ulong ulSCSize;
    ulong nRules;
    PProduction pRule, pPart;
    PSymbolClass pSC;
    
    ulSCSize = 0L;
    nRules   = 0L;
    pRule = pPR->pPList;
    while (pRule != Null)
    {
        pPart = pRule;
        while (pPart != Null)
        {
            pSC = pPart->pSC;
            pSC->usage++;
            pPart->num = nRules;
            nRules++;
            if (pSC->usage == 1)
            {
                pSC->offset = ulSCSize;
                ulSCSize += (ulong) strlen(pSC->str) + 1L;
            }
            pPart = pPart->nextPart;
        }
        pRule = pRule->nextRule;
    }
    *pulSCSize = ulSCSize;
    *pnRules   = nRules;
} /* CalcSizes */

Bool GenInit( FILE * outFile )
{
    ofile = outFile;
    return True;
} /* GenInit */

void GenDone(void)
{
} /* GenDone */

Bool GenGenerate(PParseResult pPR)
{
    PSymbolClass pSC;
    PProduction pRule, pPart;
    ulong ulSCSize, nRules, fileSize;
    char *ldbImage;
    PLDBHeader pldbh;
    PLDBRule prules;
    ulong strOffset;
    ulong state;
    char  *pstr;
    
    ulong extraBytes;
    
    CalcSizes(pPR, &ulSCSize, &nRules);
    
    extraBytes = 0L; /* Must be multiple of 4 (sizeof(long)) */
    fileSize = sizeof(LDBHeader) + extraBytes + nRules*sizeof(LDBRule) + ulSCSize;
    ldbImage = (char *) malloc( fileSize + 4 );
    if (ldbImage == Null)
        return False;
    pldbh = (PLDBHeader) ldbImage;
    strncpy(pldbh->sign, LdbFileSignature, SizeofLdbFileSignature);
    pldbh->nRules  = nRules;
    pldbh->fileSize = fileSize;
    pldbh->extraBytes = extraBytes;
    prules = (PLDBRule) (ldbImage + sizeof(LDBHeader) + extraBytes);
    /* Offset from beginning of rules to strings */
    strOffset = nRules*sizeof(LDBRule);
    pstr = (char *) prules + strOffset;
    
    /* Write rules into buffer */
    pRule = pPR->pPList;
    while (pRule != Null)
    {
        pPart = pRule;
        while (pPart != Null)
        {
            prules->strOffset = strOffset + pPart->pSC->offset;
            if (pPart->pRight != Null)
            {
                state = (((PProduction)pPart->pRight->p)->num & LdbMask);
            }
            else
            {
                state = LdbMask;
            }
            prules->state = state | pPart->flags;
            prules++;
            pPart = pPart->nextPart;
        }
        pRule = pRule->nextRule;
    }
    
    /* Write strings into buffer */
    pSC = pPR->pSList;
    while (pSC != Null)
    {
        if (pSC->usage != 0)
        {
            strcpy(pstr+pSC->offset, pSC->str);
        }
        pSC = pSC->next;
    }
    fwrite(ldbImage, sizeof(char), (size_t) fileSize, ofile);
    return True;
} /* GenGenerate */
