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
#include <stdlib.h>
#include <string.h>

#ifndef __DONT_USE_WINDOWS_H_AND_FSTREAMS_H

#include <windows.h>
#include <commdlg.h>

#else

#endif /* _DONT_USE_WINDOWS_H_AND_STREAMS */

#include <iostream>
#include "hwr_sys.h"
#include "ams_mg.h"
#include "xrwdict.h"
#include "pydict.h"
#include "WritePadDictUtil.h"

#define NAME_SIZE               300
#define DEF_CMPR_DICT_SUF_OLD   ".pvc"
#define DEF_CMPR_DICT_SUF       ".dct"
#define DEF_UNCMPR_DICT_SUF     ".pvu"
#define DEF_CMPR_FDICT_SUF      ".pfc"      //FREQUENCY DICT
#define DEF_UNCMPR_FDICT_SUF    ".pfu"
#define DEF_LIST_SUF            ".txt"
#define DEF_FREQ_SUF            ".lst"
#define DEF_NLIST_SUF           ".txt"
#define DEF_IMG_SUF             ".cpp"


typedef unsigned char uchar;

const char  version_id[] = "WritePad SDK Dictionary utility program. Ver. 4.5 03/04/2016";

static int MainCreateDict( uchar mode, char * filename );
static int MainListDict( char * filename );
static int MainAddWord(void);
static int MainAddWordList( char * dictname, char * listname  );
static int MainCreateImage( char * filename );

#ifndef __DONT_USE_WINDOWS_H_AND_FSTREAMS_H

static const char FILE_SELECTOR[] = " or ? to browse for file:";

static const char    szFilterSpec[] = "Data files\0*.stn\0All files\0*.*\0\0";
static char    szFileName[NAME_SIZE]  = {0};
static char    szWindowOpen[NAME_SIZE] = {0};
static OPENFILENAMEA         ofn = {0};

static OPENFILENAMEA * getOFN()
{
    memset(&ofn, 0, sizeof(OPENFILENAMEA));
    ofn.lStructSize = sizeof(OPENFILENAMEA);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = szFilterSpec;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = NAME_SIZE;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrFileTitle = NULL;//szFileTitle;
    ofn.nMaxFileTitle = NAME_SIZE;
    ofn.lpstrTitle = szWindowOpen;
    ofn.lpstrDefExt = NULL;
    ofn.Flags = 0;
    ofn.lpstrInitialDir = "";
    return &ofn;
}

#else

static const char FILE_SELECTOR[] = ":";

#endif 

using namespace std;

/* *************************************************************************** */
/* *         Main entry point                                                 * */
/* **************************************************************************** */

int main( int argc, char *argv[] )
{
    uchar key = '?';
    int code = 0x10;

#ifndef __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
	CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
#endif

    cout << version_id << endl;
    cout << "Copyright (c) 1997-2016 PhatWare Corp. All rights reserved." << endl << endl;

    if ( argc >= 2 )
    {
        if ( argc < 3 )
        {
            cout << "Invalid arguments: -l/c/u/f/w/i filename [filename2]." << endl;
            goto err;
        }
        char * command = argv[1];
        if ( *command != '-' || strlen( command ) != 2 )
        {
            cout << "Invalid arguments: -l/c/u/f/w/i filename [filename2]." << endl;
            goto err;
        }
        switch ( command[1] )
        {
            case 'L':
            case 'l':
                code = MainListDict( argv[2] );
                break;

            case 'C':
            case 'c':
                code = MainCreateDict( (uchar)MERGE_FLAG, argv[2] );
                break;

            case 'F':
            case 'f':
                code = MainCreateDict( (uchar)(MERGE_FLAG | FREQ_FLAG), argv[2] );
                break;

            case 'U':
            case 'u':
                code = MainCreateDict( (uchar)0, argv[2] );
                break;

            case 'W':
            case 'w':
                if ( argc != 4 )
                {
                    cout << "Invalid arguments: -w dictname listname." << endl;
                    goto err;
                }
                code = MainAddWordList( argv[2] , argv[3] );
                break;

            case 'I':
            case 'i':
                code = MainCreateImage( argv[2] );
                break;

            default :
                cout << "Invalid arguments: -l/c/u/f/w/i filename." << endl;
                goto err;
        }

        switch (code)
        {
            case 0:
                break;

            case 0x0001:
                cout << endl << "Create Dictionary failed !!!" << endl;
                break;

            case 0x0002:
                cout << endl << "List Dictionary Words failed !!!" << endl;
                break;

            case 0x0003:
                cout << endl << "Add Word failed !!!" << endl;
                break;

            case 0x0004:
                cout << endl << "Create Dictionary Image failed !!!" << endl;
                break;

            default:
                cout << endl << "Something went wrong ... Code: " << code << endl;
                break;
        }
    }
    else
    {
        while (key != 0)
        {
            cout << endl << endl;
            cout << "Create Compressed Dictionary:" << endl;
            cout << "   branches ordered alphbetically            - c" << endl;
            cout << "   branches ordered by Frequency             - f" << endl;
            cout << "Create Uncompressed Dictionary:" << endl;
            cout << "   branches ordered alphbetically            - u" << endl;
            cout << "Word List Commands:" << endl;
            cout << "   Add New Word to Uncompressed Dictionary   - a" << endl;
            cout << "   Add Word List to Uncompressed Dictionary  - w" << endl;
            cout << "   Create Word List from dictionary          - l" << endl;
            cout << "Other Commands:" << endl;
            cout << "   Create Binary Image (.cpp) of Dictionary  - i" << endl;
            cout << "   Exit Program                              - e, q, ^c" << endl << endl;
            
            cout << "> ";
            cin  >> key;
            cout << endl;

            switch (key)
            {
                case 'C':
                case 'c':
                    code = MainCreateDict( (uchar)MERGE_FLAG, NULL );
                    break;
                    
                case 'U':
                case 'u':
                    code = MainCreateDict( (uchar)0, NULL );
                    break;
                    
                case 'F':
                case 'f':
                    code = MainCreateDict( (uchar)(MERGE_FLAG | FREQ_FLAG), NULL );
                    break;
                    
                case 'I':
                case 'i':
                    code = MainCreateImage( NULL );
                    break;
                    
                case 'L':
                case 'l':
                    code = MainListDict( NULL );
                    break;
                    
                case 'A':
                case 'a':
                    code = MainAddWord();
                    break;
                    
                case 'W':
                case 'w':
                    code = MainAddWordList( NULL , NULL );
                    break;
                    
                case 'Q':
                case 'q':
                case 'E':
                case 'e':
                    goto err;
            }

            switch (code)
            {
                case 0:
                    break;
                    
                case 0x0001:
                    cout << endl << "Create Dictionary failed !!!" << endl;
                    break;
                    
                case 0x0002:
                    cout << endl << "List Dictionary Words failed !!!" << endl;
                    break;
                    
                case 0x0003:
                    cout << endl << "Add Word failed !!!" << endl;
                    break;
                    
                case 0x0004:
                    cout << endl << "Create Dictionary Image failed !!!" << endl;
                    break;
                    
                default:
                    cout << endl << "Something went wrong ... Code: " << code << endl;
                    break;
            }
        } //while (key)
    }

#ifndef __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
	CoUninitialize();
#endif
    return 0;
err:
#ifndef __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
	CoUninitialize();
#endif

	return 1;
}


int MainCreateDict( uchar mode, char * filename )
{
    void *  pd = NULL;
    char    list_name[NAME_SIZE];
    char    freq_name[NAME_SIZE];
    char    dict_name[NAME_SIZE];
    char *  ptr;
    
    if ((mode&MERGE_FLAG) && !(mode&FREQ_FLAG))
        cout << endl << "Create Compressed Dictionary with alphbetical order of branches" << endl;
    else if ((mode&MERGE_FLAG) && (mode&FREQ_FLAG))
        cout << endl << "Create Compressed Dictionary with frequency order of branches" << endl;
    else if (!(mode&MERGE_FLAG) && !(mode&FREQ_FLAG))
        cout << endl << "Create Uncompressed Dictionary with alphbetical order of branches" << endl << endl;
    else //if (!(mode&MERGE_FLAG) && (mode&FREQ_FLAG))
        cout << endl << "Create Uncompressed Dictionary with frequency order of branches" << endl << endl;

    if ( filename != NULL && *filename != 0 )
    {
        strcpy( list_name, filename );
    }
    else
    {
        cout << "Enter dictionary listing file name" << FILE_SELECTOR << endl;
        cin  >> list_name;
    }
#ifndef __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
    if (list_name[0] == '?')
    {
        strcpy(szWindowOpen, "Select WordList file");
        strcpy(szFileName, "*" DEF_LIST_SUF);
        if (!GetOpenFileNameA(getOFN()))
        {
            cout << " No file selected." << endl;
            goto err;
        }
        strcpy((char *)list_name, (char *)szFileName);
    }
#endif
    
    if (mode&FREQ_FLAG)
    {
        cout << "Enter frequency dictionary file name" << FILE_SELECTOR << endl;
        cin  >> freq_name;
        
#ifndef __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
        if (freq_name[0] == '?')
        {
            strcpy(szWindowOpen, "Select WordList file");
            strcpy(szFileName, "*" DEF_FREQ_SUF);
            if (!GetOpenFileNameA(getOFN()))
            {
                cout << " No file selected." << endl;
                goto err;
            }
            strcpy((char *)freq_name, (char *)szFileName);
        }
#endif
    } //FREQ_FLAG
    
    if ( CreatePYDict(list_name,((mode&FREQ_FLAG) ? freq_name : NULL), mode,&pd) == PYDICT_ERR )
        goto err;
    
    strcpy((char *)dict_name, (char *)list_name);
    if ((ptr = strrchr((char *)dict_name, '.')) != 0)
        *ptr = 0;
    if ((mode&MERGE_FLAG) && !(mode&FREQ_FLAG))
        strcat((char *)dict_name, DEF_CMPR_DICT_SUF);
    else if ((mode&MERGE_FLAG) && (mode&FREQ_FLAG))
        strcat((char *)dict_name, DEF_CMPR_FDICT_SUF);
    else if (!(mode&MERGE_FLAG) && !(mode&FREQ_FLAG))
        strcat((char *)dict_name, DEF_UNCMPR_DICT_SUF);
    else //if (!(mode&MERGE_FLAG) & (mode&FREQ_FLAG))
        strcat((char *)dict_name, DEF_UNCMPR_FDICT_SUF);
    
    if ( save_dictionary_to_file( dict_name, pd ) == PYDICT_ERR )
        goto err;
    
    cout << "Dictionary " << dict_name << " is created" << endl;
    
    PYDictFreeDict(&pd);
    return(0);
err:
    PYDictFreeDict(&pd);
    return(1);
}


int MainListDict( char * filename )
{
    void *  pd=NULL;
    char    list_name[NAME_SIZE];
    char    dict_name[NAME_SIZE];
    char *  ptr;
    
    cout << endl << "List Dictionary:" << endl << endl;
    
    if ( filename != NULL && *filename != 0 )
    {
        strcpy( dict_name, filename );
    }
    else
    {
        cout << "Enter dictionary file name" << FILE_SELECTOR << endl;
        cin  >> dict_name;
    }

#ifndef __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
    if (dict_name[0] == '?')
    {
		strcpy(szWindowOpen, "Select Dictionary file");
        strcpy(szFileName, "*" DEF_CMPR_DICT_SUF_OLD "; *" DEF_UNCMPR_DICT_SUF "; *" DEF_CMPR_DICT_SUF);
        if (!GetOpenFileNameA(getOFN()))
        {
            cout << " No file selected." << endl;
            goto err;
        }		
        strcpy((char *)dict_name, (char *)szFileName);
    }
#endif //
    
    cout << "Open Dictionary: " << dict_name << endl;
    
    if ( load_dictionary_from_file( dict_name, &pd) == PYDICT_ERR )
        goto err;
    
    cout << "Dictionary loaded. Generating listing.\n";
    
    strcpy((char *)list_name, (char *)dict_name);
    if ((ptr = strrchr((char *)list_name, '.')) != 0)
        *ptr = 0;
    strcat((char *)list_name, DEF_NLIST_SUF);
    
    
    if ( generate_listing(pd,list_name) == PYDICT_ERR )
        goto err;
    
    cout << dict_name << " is listed" << endl;
    
    PYDictFreeDict(&pd);
    return(0);
err:
    PYDictFreeDict(&pd);
    return(2);
}

/* ******************************************************************* */
/* *        Add a single word to existing dictionary                 * */
/* ******************************************************************* */

int MainAddWord(void)
{
    void *  pd=NULL;
    char    dict_name[NAME_SIZE];
    uchar   word[128];
    int     res, attr;
    
    cout << endl << "Add Word To Dictionary:" << endl << endl;
    
    cout << "Enter dictionary file name" << FILE_SELECTOR << endl;
    cin  >> dict_name;
    
#ifndef __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
    if (dict_name[0] == '?')
    {
        strcpy(szWindowOpen, "Select Dictionary file");
        strcpy(szFileName, "*" DEF_UNCMPR_DICT_SUF);
        if (!GetOpenFileNameA(getOFN()))
        {
            cout << " No file selected." << endl;
            goto err;
        }
        strcpy((char *)dict_name, (char *)szFileName);
    }
#endif // __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
    
    if ( load_dictionary_from_file( dict_name, &pd ) == PYDICT_ERR )
        goto err;
    
    if ( IsTreeMerged( pd ) )
    {
        cout << "Can't work with compressed dict!" << endl;
        goto err;
    }
    
    cout << "Input word" <<endl;
    cin >> word;
    
    cout << "Input attribute" <<endl;
    cin >> attr;
    
    res = PYDictAddWord( word, (uchar)attr, &pd );
    if ( res == PYDICT_ERR )
        goto err;
    
    if ( save_dictionary_to_file( dict_name, pd ) == PYDICT_ERR)
        goto err;
    
    cout << "Word \'" << word << ((res == PYDICT_OLDWORD) ? "\' is already in the " : "\' is added to ") << dict_name << endl;
    
    PYDictFreeDict(&pd);
    return(0);
err:
    PYDictFreeDict(&pd);
    return(3);
}

/* ******************************************************************* */
/* *        Add a word list to the existing dictionary               * */
/* ******************************************************************* */
int MainAddWordList( char * dictname, char * listname )
{
    int     res, i, n, added = 0, nadded = 0, existing = 0;
    void *  pd=NULL;
    char    dict_name[NAME_SIZE], list_name[NAME_SIZE], add_name[NAME_SIZE], nad_name[NAME_SIZE];
    uchar   word[128];
    int     attr;
    _CHAR   str[256];
    _CHAR * ptr;
    FILE  * flist = NULL;
    FILE  * fadd  = NULL;
    FILE  * fnad  = NULL;
    
    cout << endl << "Add a Word List To the Dictionary:" << endl << endl;
    
    if ( dictname != NULL && *dictname != 0 )
    {
        strcpy( dict_name, dictname );
    }
    else
    {
        cout << "Enter dictionary file name" << FILE_SELECTOR << endl;
        cin  >> dict_name;
    }

#ifndef __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
    if (dict_name[0] == '?')
    {
        strcpy(szWindowOpen, "Select Dictionary file");
        strcpy(szFileName, "*" DEF_UNCMPR_DICT_SUF);
        if (!GetOpenFileNameA(getOFN()))
        {
            cout << " No file selected." << endl;
            goto err;
        }
        strcpy((char *)dict_name, (char *)szFileName);
    }
#endif // __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
    
    if ( load_dictionary_from_file( dict_name, &pd ) == PYDICT_ERR)
        goto err;
    
    if (IsTreeMerged(pd))
    {
        cout << "Can't work with compressed dict!" << endl;
        goto err;
    }
    
    if ( listname != NULL && *listname != 0 )
    {
        strcpy( list_name, listname );
    }
    else
    {
        cout << "Enter word list file name" << FILE_SELECTOR << endl;
        cin  >> list_name;
    }

#ifndef __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
    if (list_name[0] == '?')
    {
        strcpy(szWindowOpen, "Select WordList file");
        strcpy(szFileName, "*" DEF_LIST_SUF);
        if (!GetOpenFileNameA(getOFN()))
        {
            cout << " No file selected." << endl;
            goto err;
        }
        strcpy((char *)list_name, (char *)szFileName);
    }
#endif // __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
    
    strcpy((char *)add_name, (char *)list_name);
    if ((ptr = strrchr((char *)add_name, '.')) != 0)
        *(ptr) = 0;
    strcat((char *)add_name, ".add.txt");
    
    strcpy((char *)nad_name, (char *)list_name);
    if ((ptr = strrchr((char *)nad_name, '.')) != 0)
        *(ptr) = 0;
    strcat((char *)nad_name, ".nad.txt");
    
    if ((flist = fopen((char *)list_name, "rt")) == _NULL)
    {
        cout << "Can't open " << list_name << endl;
        goto err;
    }
    if ((fadd  = fopen((char *)add_name, "wt")) == _NULL)
    {
        cout << "Can't open " << add_name << endl;
        goto err;
    }
    if ((fnad  = fopen((char *)nad_name, "wt")) == _NULL)
    {
        cout << "Can't open " << nad_name << endl;
        goto err;
    }
    
    str[0] = 0;
    for (i = n = added = nadded = 0; i < 1000000; i ++)
    {
        if (!fgets((char *)str, 80, flist))
            break;
        
        n ++;
        attr = 0;
        word[0] = 0;
        
        if (sscanf((char *)str, "%s %d", word, &attr) < 1) continue;
        
        // ---------- Filter ----------------------------------------------------
        
        //    if (HWRStrCmp((_STR)&word[HWRStrLen((_STR)word)-2], "'s") == 0) continue;
        
        // ----------------------------------------------------------------------
        
        res = PYDictAddWord( word, (_UCHAR)attr, &pd);
        if ( res == PYDICT_ERR )
        {
            fprintf( fnad, "%-12s %d\n", word, attr);
            nadded++;
        }
        else
        {
            fprintf( fadd, "%s %-12s %d\n", (res == PYDICT_OLDWORD ? " " : "+"), word, attr);
            if ( res == PYDICT_OLDWORD )
                existing++;
            else
                added++;
        }
        
        if ((n % 100) == 0)
        {
            cout << "Read " << n << " words;  added " << added << "      \r";
        }
    }
    
    if (flist)
        fclose(flist);
    if (fadd)
        fclose(fadd);
    if (fnad)
        fclose(fnad);
    
    if ( save_dictionary_to_file( dict_name,pd ) == PYDICT_ERR)
        goto err;
    
    cout << added << " word(s) have been added; " << existing << " word(s) already in the dictionary; " << nadded << " error(s)." << endl;
    
    PYDictFreeDict(&pd);
    return(0);
err:
    PYDictFreeDict(&pd);
    return(3);
}

int MainCreateImage( char * filename )
{
    void *pd=NULL;
    char   dict_name[NAME_SIZE];
    char   img_name[NAME_SIZE];
    FILE *file;
    int size,nl,rl,i;
    char *p;
    unsigned long *ptr,last=0l;
    
    cout << endl << "Create Image Of Dictionary:" << endl << endl;

    if ( filename != NULL && *filename != 0 )
    {
        strcpy( dict_name, filename );
    }
    else
    {
        cout << "Enter dictionary file name" << FILE_SELECTOR << endl;
        cin  >> dict_name;
    }

#ifndef __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
    if (dict_name[0] == '?')
    {
        strcpy(szWindowOpen, "Select Dictionary file");
        strcpy(szFileName, "*" DEF_CMPR_DICT_SUF "; *" DEF_UNCMPR_DICT_SUF "; *" DEF_CMPR_DICT_SUF);
        if (!GetOpenFileNameA(getOFN()))
        {
            cout << " No file selected." << endl;
            goto err;
        }
        strcpy(dict_name, szFileName);
    }
#endif // __DONT_USE_WINDOWS_H_AND_FSTREAMS_H
    
    strcpy(img_name, dict_name);
    if ((p = strrchr(img_name, '.')) != 0)
        *p = 0;
    strcat(img_name, DEF_IMG_SUF);
    
    if ( load_dictionary_from_file( dict_name, &pd ) == PYDICT_ERR )
        goto err;
    
    size = PYDictGetDictMemSize( pd );
    nl = size / sizeof(unsigned long);
    rl = size % sizeof(unsigned long);
    
    if ((file = fopen(img_name, "wt")) != NULL)
    {
        fprintf(file, "// **************************************************************************\n");
        fprintf(file, "// *    This File is Generated by WritePad Dictionary Utility               *\n");
        fprintf(file, "// *    DO NOT EDIT THIS FILE!                                              *\n");
        fprintf(file, "// *    Copyright (c) 2008-2016 PhatWare Corp. All rights reserved.         *\n");
        fprintf(file, "// **************************************************************************\n\n");
        
#ifndef __DONT_EMIT_VOC_PTR_STRUCT
        fprintf(file, "\n#include \"ams_mg.h\"\n");
        fprintf(file, "\n#include \"vocutilp.h\"\n\n");
#else
        fprintf(file, "\n#include \"bastypes.h\"\n\n");
#endif /* __DONT_EMIT_VOC_PTR_STRUCT */
        
        fprintf(file, "// ****  DICTIONARY body   **************************************************\n\n");
        
        //    len = elk->size/4; // len was in bytes
        fprintf(file, "ROM_DATA _ULONG img_voc_body[%d] =  \n", nl + 1);
        fprintf(file, " {  \n");
        for (i = 0, ptr = (unsigned long *)pd; i < nl; i ++, ptr ++)
        {
            fprintf(file, "0x%08lX", *ptr);
            if (i < nl-1)
                fprintf(file, ", ");
            if (i%8 == 7)
                fprintf(file, "\n");
        }
        if (rl>0)
        {
            memcpy(&last,ptr,rl); 
            fprintf(file, ", ");
            if (i%8 == 7)
                fprintf(file, "\n");
            fprintf(file, "0x%08lX", last);
        }
        fprintf(file, " }; \n\n");
        
        fprintf(file, "// **************************************************************************\n");
        fprintf(file, "// *    END OF GENERATED FILE                                               *\n");
        fprintf(file, "// **************************************************************************\n");
        
        fclose(file);
    }
    
    cout << "Dictionary " << dict_name << " saved to " << img_name << ".\n";
    
    PYDictFreeDict((void**)&pd);
    
    return (0);
err:
    return (4);
    
}






