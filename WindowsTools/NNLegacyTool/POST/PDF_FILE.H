/********************************************************************/
/*                                                                  */
/*  PDF_FILE.H                               Created: 12 Dec 1992.  */
/*                                                                  */
/*                                                                  */
/*       THE DESCRIPTION  OF  THE  POSTPROCESSING DATA (.PDF)       */
/*      ------------------------------------------------------      */
/*                           FILE FORMAT.                           */
/*                          --------------                          */
/*                                                                  */
/*    (All fields in the .PDF  file  have  MS-DOS  byte  order:     */
/*                  least significant byte first.)                  */
/*                                                                  */
/*  **************************************************************  */
/*    Although the  data  structures  of  the  file  theoretically  */
/*  permit  any  order  of file blocks,  these data structures are  */
/*  introduced with the possibility of the file  memory-mapping in  */
/*  mind,  so  that  all offset arrays could be used for the quick  */
/*  rule  location,  but  all  file  blocks  pointed  to  by   any  */
/*  variablesize array (say,  mainHeader.charOffset) should follow  */
/*  one another in the same order as the array elements,  and  all  */
/*  file  sub-blocks should be bound inside the upper-level block.  */
/*    Also, though data structures permit 32 variants  per letter,  */
/*  due to the internal file processing problems, the actual limit  */
/*  is only 31 variant per letter.                                  */
/*  **************************************************************  */
/*                                                                  */
/*       The file  contains  the  rules  for the postprocessing in  */
/*  both text and precompiled format. It has the following overall  */
/*  structure:                                                      */
/*       - the loading header, which is the only part of the  file  */
/*  which is not designed to be memory-mapped,  because it  exists  */
/*  only  in  memory  and  consists  of  two  pointers,  the first  */
/*  pointing to the ROMable part of the file,  and the second  one  */
/*  pointing  to  the array of shorts which are the right sides of  */
/*  comparisons qualifying for automatic learning and  are subject  */
/*  to changes during the recognition process.                      */
/*                                                                  */
/*       - the main header,  containing  the  information  of  the  */
/*         availability  and position of character blocks,  which,  */
/*         in  turn,  contain  the  information   concerning   the  */
/*         particular  character.  It  also  tells  if the file is  */
/*         'stripped', that is, does not contain the rules' source  */
/*         code information.                                        */
/*       - up to 256 character blocks.                              */
/*                                                                  */
/*       The character block consists of:                           */
/*       - character  block  header  with  the  information  about  */
/*         presence and position of the variant block.              */
/*       - up to 32 variant blocks.                                 */
/*                                                                  */
/*       The variant block:                                         */
/*       - the variant block header with the information about the  */
/*         presence  and  position of character connection blocks,  */
/*         which,  in turn,  contain  the  information  about  the  */
/*         connections  which  this  particular  variant  of  this  */
/*         particular character has with some another character.    */
/*       - the rule block,  which contains  'this  character-only'  */
/*         rules.                                                   */
/*       - up to 256 character connection blocks.                   */
/*                                                                  */
/*       The character connection block:                            */
/*       - the block header with the information which variants of  */
/*         the connected letter participate in  the  mutual  rules  */
/*         and the positions of the corresponding rule blocks.      */
/*       - up to 32 rule blocks.                                    */
/*                                                                  */
/*       Each rule block consists of:                               */
/*       - the header,  which contains  the  number  of  rules  in  */
/*         block,  the positions of the precompiled rules, and the  */
/*         lengths of the rules' source code (if the file  is  not  */
/*         stripped).                                               */
/*       - the precompiled code of the rules.                       */
/*       - the source code of  the  rules  (if  the  file  is  not  */
/*         stripped).                                               */
/*                                                                  */
/*       Here are the defineitions for the file fields:             */

#ifndef PDF_FILE_H
#define PDF_FILE_H

#ifndef _PTR
#define _PTR *
#endif

#define PDF_LAST_VAR   31          /*  The biggest  variant number  */
/* possible. Actually it cannot  */
/* be  used  for  the   variant  */
/* storage and is used only for  */
/* the internal purposes.        */
/*                                                                  */
/*       The main header:                                           */
/*       ----------------                                           */

typedef struct PDF_MAIN_HEADER
{
	unsigned long  ID;
	unsigned long  stripped;
	unsigned long  strippedByLoad;
	unsigned long  size;
	unsigned char  charPresent[32];
	unsigned long  charOffset[1];
} PDF_MAIN_HEADER, _PTR P_PDF_MAIN_HEADER;

/*       - ID:  contains  "PDF\0".  Helps to identify the file.     */
/*       - stripped:  if non-zero,  the file does not contain  the  */
/*         source code.                                             */
/*       - strippedByLoad:  has no meaning in the file.  If it  is  */
/*         filled  by non-zero during the load,  and 'stripped' is  */
/*         zero,  the HDFSaveFile cannot save the file unless  the  */
/*         secret  global  flag in HDFSaveFile is set to one.  The  */
/*         purpose of this feature is  to  forbid  the  accidental  */
/*         source code erasing from the file.                       */
/*       - size: the whole file size.                               */
/*       - charPresent:  the array of 256 bits, beginning from the  */
/*         most significant bit of the first byte. If the n-th bit  */
/*         is 1, the character has some rules associated with it.   */
/*       - charOffset:  the variable-size array. The size of array  */
/*         is  the number of 1-set bits in charPresent array,  and  */
/*         each element of the array contains the  offset  of  the  */
/*         corresponding character block from the beginning of the  */
/*         file.                                                    */
/*                                                                  */
/*       After this  array  the  file  contains  as many character  */
/*  blocks as there are 1-s in charPresent array.                   */
/*                                                                  */
/*                                                                  */
/*       The loading header:                                        */
/*       -------------------                                        */

typedef struct PDF_LOAD_HEADER
{
	P_PDF_MAIN_HEADER pMainHeader;
	p_SHORT           learningData;
	p_SHORT           wtData;
	_SHORT            learningSize;
	_SHORT            wtSize;
} PDF_LOAD_HEADER, _PTR P_PDF_LOAD_HEADER;

/*       - pMainHeader: points to the main header of  the  memory-  */
/*         mapped file.                                             */
/*       - learningData: the pointer to the array of shorts  which  */
/*         represent the right sides of comparisons qualifying for  */
/*         the learning.                                            */
/*       - learningSize: the   number   of   elements    in    the  */
/*         learningData array.                                      */
/*                                                                  */
/*         Basically the first pointer of this structure points to  */
/*         the  ROMable  part  of the data,  and the second to the  */
/*         part which should be in RAM  since  it  is  subject  to  */
/*         changes.                                                 */
/*                                                                  */
/*                                                                  */
/*     The character block:                                         */
/*     --------------------                                         */

typedef struct PDF_CHAR_HEADER
{
	unsigned long  size;
	unsigned char  varPresent[4];
	unsigned long  varPosition[1];
} PDF_CHAR_HEADER, _PTR P_PDF_CHAR_HEADER;

/*       - size: the whole character block size.                    */
/*       - varPresent: the array of 32 bits, which are set to 1 if  */
/*         the corresponding variant has any rules.                 */
/*       - varPosition:  the variable size array of the offsets of  */
/*         variant blocks - as many of them, as 1-s in varPresent.  */
/*         THE OFFSETS ARE CALCULATED FROM THE  BEGINNING  OF  THE  */
/*         CHARACTER BLOCK.                                         */
/*                                                                  */
/*       After the array goes the same number of variant blocks.    */
/*                                                                  */
/*                                                                  */
/*       The variant block:                                         */
/*       ------------------                                         */
/*                                                                  */
typedef struct PDF_VAR_HEADER
{
	unsigned long size;
	unsigned char connPresent[32];
	unsigned long ruleBlockSize;
	unsigned long connPosition[1];
} PDF_VAR_HEADER, _PTR P_PDF_VAR_HEADER;

/*       - size: the whole variant block size.                      */
/*       - connPresent: this bit array bits are 1 if the character  */
/*         to which this variant belongs has the mutual rules with  */
/*         the character corresponding to the bit number.           */
/*       - ruleBlockSize:  if non-zero,  it means that immediately  */
/*         after the header goes the rule block of that size which  */
/*         contains  the  rules,  which  are  specific  for   this  */
/*         character only.                                          */
/*       - connPosition: the variable-size array of the offsets of  */
/*         the  connection blocks CALCULATED FROM THE BEGINNING OF  */
/*         THE VARIANT BLOCK.                                       */
/*                                                                  */
/*       After this goes the rule block (if ruleBlockSize != 0).    */
/*                                                                  */
/*       After this are the connection blocks.                      */
/*                                                                  */
/*                                                                  */
/*       The connection block:                                      */

/*       ---------------------                                      */
/*                                                                  */
typedef struct PDF_CONNECTION_HEADER
{
	unsigned long size;
	unsigned char connVarPresent[4];
	unsigned long connVarPosition[1];
} PDF_CONNECTION_HEADER, _PTR P_PDF_CONNECTION_HEADER;

/*       - size: the whole connection block size;                   */
/*       - connVarPresent: bit array with the bits set to 1 if the  */
/*         mutual rule exists for the corresponding variant of the  */
/*         connected character.                                     */
/*       - connVarPosition: the variable-size array of rule blocks  */
/*         offsets FROM THE BEGINNING OF THE CONNECTION BLOCK.      */
/*                                                                  */
/*       After it there are rule blocks themselves.                 */

/*     The rule block:                                              */
/*     ---------------                                              */

typedef struct PDF_RULE_HEADER
{
	unsigned short nRules;
	unsigned short precompiledLen[5];
	unsigned short sourceLen[5];
	short          weight[5];
} PDF_RULE_HEADER, _PTR P_PDF_RULE_HEADER;

/*       - nRules: the number of rules in the block.                */
/*       - precompiledLen: lengths of precompiled rules.            */
/*       - sourceLen: length of rules' source codes.                */
/*       - weight: the weights of the rules.                        */
/*                                                                  */
/*       After this are all precompiled rules.                      */
/*                                                                  */
/*       After this there are all rules' source codes (if the file  */
/*  is not stripped.)                                               */
/*                                                                  */
/*       ATTENTION: All rules ARE NOT null-terminated !             */
/*                                                                  */
/********************************************************************/

_BOOL PDFLoadFile(p_CHAR file, _ULONG fileOffset, p_ULONG phPDFHeader);
_BOOL PDFUnloadFile(p_ULONG phPDFHeader, p_UCHAR _PTR pPDF);
_BOOL PDFSetRule(P_PDF_MAIN_HEADER _PTR ppPDFHeader,
                 _SHORT thisChar, _SHORT thisVar,
                 _SHORT otherChar, _SHORT otherVar,
                 p_CHAR _PTR rule, _SHORT _PTR ruleLen,
                 p_CHAR _PTR ruleSrc, _SHORT _PTR ruleSrcLen,
                 _SHORT _PTR weight, _SHORT nRules);
P_PDF_CHAR_HEADER PDFGetCharAddress(P_PDF_MAIN_HEADER pPDFHeader, _SHORT c);
_SHORT PDFReturnBitNumber(p_UCHAR bitArray, _SHORT c);
P_PDF_VAR_HEADER PDFGetVarAddress(P_PDF_CHAR_HEADER pCharHeader, _SHORT var);
P_PDF_CONNECTION_HEADER PDFGetConnectionAddress(P_PDF_VAR_HEADER pVarHeader,
        _SHORT otherChar);
P_PDF_RULE_HEADER PDFGetRuleAddress
(P_PDF_CONNECTION_HEADER pConnectionHeader, _SHORT otherVar);
_SHORT PDFReturnNumberOfBits(p_UCHAR bitArray, _SHORT size);
_SHORT PDFReturnIndex(p_UCHAR bitArray, _SHORT number);
void   PDFSetBit(p_UCHAR bitArray, _SHORT number, _BOOL bValue);
void   PDFClearBitArray(p_UCHAR bitArray, _SHORT size);
_BOOL  PDFGetRule(P_PDF_MAIN_HEADER pPDFHeader,
                  _SHORT thisChar, _SHORT thisVar,
                  _SHORT otherChar, _SHORT otherVar,
                  P_PDF_RULE_HEADER _PTR ppRuleHeader);
_BOOL  PDFRuleIsPresent(P_PDF_MAIN_HEADER pPDFHeader,
                        _SHORT thisChar, _SHORT thisVar,
                        _SHORT otherChar, _SHORT otherVar);
_BOOL  PDFSwapVariants(P_PDF_MAIN_HEADER pPDFHeader, _SHORT character,
                       _SHORT var1, _SHORT var2);
_SHORT PDFGetRuleSize(P_PDF_RULE_HEADER pRuleHeader);
_BOOL  PDFIsDangerousToDeleteVar(P_PDF_MAIN_HEADER pPDFHeader,
                                 _SHORT character, _SHORT var,
                                 p_BOOL pbShouldChangeRules);
_BOOL  PDFDeleteVariantRules(P_PDF_MAIN_HEADER _PTR pPDFHeader,
                             _SHORT character, _SHORT var, p_CHAR fileName);
void   PDFShiftXrsRight(P_PDF_MAIN_HEADER pPDFHeader,
                        _SHORT character, _SHORT var, _SHORT xrNum);
_BOOL  PDFShiftXrsLeft(P_PDF_MAIN_HEADER pPDFHeader,
                       _SHORT character, _SHORT var, _SHORT xrNum,
                       p_CHAR fileName);
_BOOL  PDFGetBit(p_UCHAR bitArray, _SHORT number);
void   PDFShiftBitArrayLeft(p_UCHAR bitArray, _SHORT number, _SHORT size);
void   PDFSetNameExtension(p_CHAR to, p_CHAR from, p_CHAR ext);
_VOID  PDFSwapFile(P_PDF_MAIN_HEADER pPDFHeader);
_VOID  PDFSwapCharBlock(P_PDF_CHAR_HEADER pCharHeader);
_VOID  PDFSwapVarBlock(P_PDF_VAR_HEADER pVarHeader);
_VOID  PDFSwapConnBlock(P_PDF_CONNECTION_HEADER pConnHeader);
_VOID  PDFSwapRuleBlock(P_PDF_RULE_HEADER pRuleHeader);

/*  The routines for strip-on-the-fly loading.  */
_BOOL  PDFLoadFileWithStripping(p_CHAR fileName,
                                P_PDF_MAIN_HEADER _PTR ppPDFHeader);
_SHORT PDFSourceCodeInRule(P_PDF_RULE_HEADER pRuleHeader);

#endif  /*  PDF_FILE_H  */
