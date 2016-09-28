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
#include "parser.h"
#include "gen.h"

#define MYNAME "LDBTOOL"

int main(int argc, char *argv[])
{
    FILE *  inFile;
    FILE *  outFile;
    PParseResult pPR;
    
    printf( "WritePad SDK Lexical parser. Creates LDB files from LEX.\n" );
    printf( "Copyright (c) 1997-2016 PhatWare Corp. All rights reserved.\n\n" );
    
    if (argc < 3)
    {
        printf("Usage: %s infile.lex outfile.ldb\n", MYNAME);
        exit(1);
    }
    inFile = fopen(argv[1], "rt");
    if (inFile == NULL)
    {
        printf("%s Error: Can't open file %s\n", MYNAME, argv[1]);
        exit(1);
    }
    outFile = fopen(argv[2], "wb");
    if ( outFile == NULL )
    {
        printf("%s Error: Can't create file %s\n", MYNAME, argv[2]);
        exit(1);
    }
    
    // init parser
    if ( ! TblInit())
    {
        printf("%s Error: Can't initialize tables\n", MYNAME);
        exit(1);
    }
    if (!ScnInit(inFile))
    {
        printf("%s Error: Can't initialize scanner\n", MYNAME);
        exit(1);
    }
    if (!PrsInit())
    {
        printf("%s Error: Can't initialize parser\n", MYNAME);
        exit(1);
    }
    if ( ! GenInit( outFile ))
    {
        printf("%s Error: Can't initialize generator\n", MYNAME);
        exit(1);
    }
    
    // parse file
    if ( PrsParse( &pPR ) )
    {
        // save file
        if ( GenGenerate( pPR) )
        {
            printf("%s: Success! File %s has been created!\n", MYNAME, argv[2] );
        }
    }
    else
    {
        // unable to parse
        printf("%s Error: unable to parse %s file, please check the file content and try again.\n", MYNAME, argv[1] );
        remove( argv[2] );
    }
    
    GenDone();
    PrsDone();
    ScnDone();
    TblDone();
    
    return 0;
} /* main */
