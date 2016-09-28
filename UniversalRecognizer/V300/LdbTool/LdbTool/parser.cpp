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
#include "scanner.h"
#include "ldbtypes.h"
#include "parser.h"

#define PrsTypeRule         1
#define PrsTypeSymbol       2


#define PrsTypeMBSymbol     3    /* Reference ahead, Must Be Symbol */
#define PrsTypeProduction   4
#define PrsTypeMBProduction 5    /* Reference ahead, Must Be Production */

static PNode pSymbolToken;
static PNode pStartToken;
static PNode pProductionToken;
static PNode pStartSymbolToken;

static PProduction  pPList;
static PSymbolClass pSList;
static PSymbolClass pSLast;

static Bool NewSymbolClass(char *str, PSymbolClass *ppSC)
{
    PSymbolClass pSC;
    size_t len;
    
    pSC = (PSymbolClass) malloc(sizeof(SymbolClass));
    if (pSC == Null)
        return False;
    len = strlen(str);
    pSC->str = (char *)malloc((len+1) * sizeof(char));
    if (pSC->str == Null)
    {
        free(pSC);
        return False;
    }
    pSC->next = Null;
    strcpy(pSC->str, str);
    pSC->usage = 0;
    
    pSLast->next = pSC;
    pSLast = pSC;
    *ppSC = pSC;
    return True;
} /* NewSymbolClass */

static Bool MergeSymbolClass( PSymbolClass pSC1, PSymbolClass pSC2, PSymbolClass *ppSC)
{
    char buffer[256];
    PSymbolClass pSC;
    Bool fFound;
    
    strcpy(buffer, pSC2->str);
    strcat(buffer, pSC1->str);
    fFound = False;
    for (pSC = pSList->next; pSC != Null; pSC = pSC->next)
    {
        if (strcmp(buffer, pSC->str) == 0)
        {
            fFound = True;
            break;
        }
    }
    if (fFound)
    {
        *ppSC = pSC;
        return True;
    }
    strcpy(buffer, pSC1->str);
    strcat(buffer, pSC2->str);
    fFound = False;
    for (pSC = pSList->next; pSC != Null; pSC = pSC->next)
    {
        if (strcmp(buffer, pSC->str) == 0)
        {
            fFound = True;
            break;
        }
    }
    if (fFound)
    {
        *ppSC = pSC;
        return True;
    }
    return NewSymbolClass(buffer, ppSC);
} /* MergeSymbolClass */

static Bool ParseSymbol(void)
{
    Symbol symbol;
    PNode  pNode;
    int    n;
    char   buffer[256];
    PSymbolClass pSC;
    
    if (!ScnRead(ScnModeName, &symbol))
        return False;
    pNode = symbol.pNode;
    if (pNode->type != TblTypeUnspec && pNode->type != PrsTypeMBSymbol)
        return False; /* Duplicate symbol def */
    for (n = 0; ; n++)
    {
        if (!ScnRead(ScnModeChar, &symbol))
            return False;
        if (symbol.type == ScnTypeEol)
            break;
        buffer[n] = (char) symbol.val;
    }
    buffer[n] = '\0';
    if (n == 0) return False;
    if (!NewSymbolClass(buffer, &pSC)) return False;
    
    pNode->type = PrsTypeSymbol;
    pNode->p    = pSC;
    return True;
} /* ParseSymbol */

static Bool ParseStart(void)
{
    Symbol symbol;
    PNode  pNode;
    
    if (pStartSymbolToken != Null)
        return False; /* Duplicate START statement */
    if (!ScnRead(ScnModeName, &symbol))
        return False;
    if (symbol.type != ScnTypeName)
        return False; /* Premature EOL */
    pNode = symbol.pNode;
    if (pNode->type != PrsTypeProduction)
        pNode->type = PrsTypeMBProduction;
    pStartSymbolToken = pNode;
    if (!ScnRead(ScnModeSkip, &symbol))
        return False; /* Premature EOF */
    return True;
} /* ParseStart */

static Bool ParseProduction(void)
{
    Symbol symbol;
    PNode  pNodeL, pNodeS, pNodeR;
    PProduction pProd, pInsert;
    
    if (!ScnRead(ScnModeName, &symbol))
        return False;
    if (symbol.type != ScnTypeName)
        return False; /* Premature EOL */
    pNodeL = symbol.pNode;
    if (pNodeL->type != TblTypeUnspec &&
        pNodeL->type != PrsTypeProduction &&
        pNodeL->type != PrsTypeMBProduction)
        return False; /* Name is not valid */
    
    if (!ScnRead(ScnModeName, &symbol))
        return False;
    if (symbol.type != ScnTypeName)
        return False; /* Premature EOL */
    pNodeS = symbol.pNode;
    
    if (!ScnRead(ScnModeName, &symbol))
        return False;
    if (symbol.type != ScnTypeEol)
    {
        pNodeR = symbol.pNode;
        if (pNodeR->type != TblTypeUnspec &&
            pNodeR->type != PrsTypeProduction &&
            pNodeR->type != PrsTypeMBProduction)
            return False; /* Name is not valid */
        if (pNodeR->type != PrsTypeProduction) pNodeR->type = PrsTypeMBProduction;
        if (!ScnRead(ScnModeSkip, &symbol))
            return False;
    }
    else
    {
        pNodeR = Null;
    }
    pProd = (PProduction) malloc(sizeof(Production));
    if (pProd == Null)
        return False;
    /* Fill production structure */
    pProd->nextRule = Null;
    pProd->nextPart = Null;
    pProd->pName    = pNodeL;
    pProd->pSC      = (PSymbolClass) pNodeS->p;
    pProd->pRight   = pNodeR;
    pProd->flags    = 0L;
    pNodeL->type    = PrsTypeProduction;
    /* Insert it in list */
    for (pInsert = pPList;
         pInsert->nextRule != Null && pInsert->pName != pNodeL;
         pInsert = pInsert->nextRule)
        ;
    if (pInsert->pName == pNodeL)
    {
        for (;pInsert->nextPart != Null; pInsert = pInsert->nextPart)
            ;
        pInsert->nextPart = pProd;
    }
    else
    {
        pInsert->nextRule = pProd;
        /* Node points to first part of the rule */
        pNodeL->p = pProd;
    }
    return True;
} /* ParseProduction */

static Bool ParseLine(void)
{
    Symbol symbol;
    int    mode;
    
    mode = ScnModeName;
    if ( ! ScnRead( mode, &symbol ) )
        return False;
    if (symbol.type == ScnTypeEol)
    {
        /* Do nothing */
    }
    else if (symbol.type == ScnTypeName)
    {
        if (symbol.pNode == pSymbolToken)
        {
            if (!ParseSymbol())
                return False;
        }
        else if (symbol.pNode == pStartToken)
        {
            if (!ParseStart())
                return False;
        }
        else if (symbol.pNode == pProductionToken)
        {
            if (!ParseProduction())
                return False;
        }
        else
        {
            /* Error - line can not begin with arbitrary name */
            return False;
        }
    }
    else
    {
        /* Some error in scanner logic */
        return False;
    }
    return True;
} /* ParseLine */

static Bool PrepareRule(PProduction pRule)
{
    PProduction  pPr, pPr0, pPr1;
    PSymbolClass pSC;
    PNode        pRight;
    ulong        flags;
    
    /* Find parts of rule with same symbol-class and merge them */
    for (pPr = pRule; pPr != Null; pPr = pPr->nextPart)
    {
        pSC = pPr->pSC;
        for (pPr0 = pPr->nextPart; pPr0 != Null;)
        {
            if (pSC == pPr0->pSC)
            {
                /* One and only one of parts should have empty right side */
                if ((pPr->pRight != Null && pPr0->pRight != Null) ||
                    (pPr->pRight == Null && pPr0->pRight == Null))
                    return False;
                /* Merge rules, marking it as AUTO_ALLOW */
                if (pPr->pRight == Null) pPr->pRight = pPr0->pRight;
                pPr->flags |= LdbAllow;
                for (pPr1 = pPr; pPr1->nextPart != pPr0; pPr1 = pPr1->nextPart)
                    ;
                pPr1->nextPart = pPr0->nextPart;
                free(pPr0);
                pPr0 = pPr1->nextPart;
            }
            else
            {
                pPr0 = pPr0->nextPart;
            }
        }
    }
    
    /* Find rules with the same right part and merge them */
    for (pPr = pRule; pPr != Null; pPr = pPr->nextPart) {
        pRight = pPr->pRight;
        flags = pPr->flags;
        for (pPr0 = pPr->nextPart; pPr0 != Null; pPr0 = pPr0->nextPart)
        {
            if (pRight == pPr0->pRight && flags == pPr0->flags)
            {
                if (!MergeSymbolClass(pPr->pSC, pPr0->pSC, &pSC))
                    return False;
                pPr->pSC = pSC;
                for (pPr1 = pPr; pPr1->nextPart != pPr0; pPr1 = pPr1->nextPart)
                    ;
                pPr1->nextPart = pPr0->nextPart;
                free(pPr0);
                break;
            }
        }
    }
    
    /* Mark rules without right part as LdbLast */
    for (pPr = pRule; pPr != Null; pPr = pPr->nextPart)
    {
        if (pPr->pRight == Null) pPr->flags |= LdbLast;
    }
    
    /* Mark all rules except last with LdbCont */
    for (pPr = pRule; pPr != Null; pPr = pPr->nextPart)
    {
        if (pPr->nextPart != Null) pPr->flags |= LdbCont;
    }
    return True;
} /* PrepareRule */

Bool PrsInit()
{
    if (!TblLookup( (char *)"SYMBOL", 6, TblModeInsert, &pSymbolToken))
        return False;
    pSymbolToken->type = PrsTypeRule;
    
    if (!TblLookup( (char *)"START", 5, TblModeInsert, &pStartToken))
        return False;
    pStartToken->type = PrsTypeRule;
    
    if (!TblLookup( (char *)"PRODUCTION", 10, TblModeInsert, &pProductionToken))
        return False;
    pProductionToken->type = PrsTypeRule;
    
    pStartSymbolToken = Null;
    
    /* Allocate head of production list */
    pPList = (PProduction) malloc(sizeof(Production));
    if (pPList == Null) return False;
    pPList->nextRule  = Null;
    pPList->nextPart  = Null;
    pPList->pName     = Null;
    pPList->pSC       = Null;
    pPList->pRight    = Null;
    pPList->flags     = 0L;
    
    /* Allocate head of symbol class list */
    pSList = (PSymbolClass) malloc(sizeof(SymbolClass));
    if (pSList == Null) return False;
    pSList->next = Null;
    pSList->str  = Null;
    pSList->usage = 0;
    pSLast = pSList;
    return True;
} /* PrsInit */

void PrsDone(void)
{
} /* PrsDone */

Bool PrsParse(PParseResult *ppPR)
{
    PParseResult pPR;
    PProduction pRule;
    PProduction pStartRule;
    
    /* Allocate ParseResult structure */
    pPR = (PParseResult) malloc(sizeof(ParseResult));
    if (pPR == Null)
        return False;
    
    /* Parse text */
    while (ParseLine());
    
    /* Check for presence of START production and put it in the head of list */
    if (pStartSymbolToken == Null)
        return False; /* No START directive */
    if (pStartSymbolToken->p == Null)
        return False; /* No start production */
    pStartRule = (PProduction) pStartSymbolToken->p;
    for (pRule = pPList;
         pRule->nextRule != pStartRule;
         pRule = pRule->nextRule) ;
    /* Delete from list */
    pRule->nextRule = pStartRule->nextRule;
    /* And insert at the beginning */
    pStartRule->nextRule = pPList->nextRule;
    pPList->nextRule = pStartRule;
    
    
    /* Prepare rules for code generation */
    pRule = pPList->nextRule;
    while (pRule != Null)
    {
        if (!PrepareRule(pRule)) return False; /* Error in rule */
        pRule = pRule->nextRule;
    }
    pPR->pPList = pPList->nextRule;
    pPR->pSList = pSList->next;
    free(pPList);
    free(pSList);
    *ppPR = pPR;
    return True;
} /* PrsParse */
