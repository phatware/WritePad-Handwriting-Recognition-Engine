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

#ifndef __DONT_USE_WINDOWS_H

#undef LANG_DUTCH
#undef LANG_ENGLISH
#undef LANG_FINNISH
#undef LANG_GERMAN
#undef LANG_ITALIAN
#undef LANG_PORTUGUESE
#undef LANG_SPANISH 
#undef LANG_FRENCH

#include <windows.h>
#include <commdlg.h>
#endif // __DONT_USE_WINDOWS_H

#include <iostream>
#include <fstream>

#pragma hdrstop

#include <time.h>

#include "mlp.h"
#include "main.h"
#include "langid.h"

// --------------- Defines -----------------------------------------------------

#define MAIN_NSAMP_PV 20

#define LETTER_MIXUP_ARRAY                                                     \
{                                                         \
        {'o','0',4},{'O','0',4},                                 \
        {'e','l',3},{'e','c',2},                                 \
        {'s','5',2},{'S','5',2},                                 \
        {'v','r',3},{'v','u',3},                                 \
        {'u','n',2},                                             \
        {'y','g',2},                                             \
        {'t','+',3},{'t','f',2},                                 \
        {'G','6',2},                                             \
        {'I','l',3},                                             \
        {'9','g',2},{'4','y',2},{'1','I',3},{'1','l',3},         \
        {'2','r',2},{'2','z',2},                                 \
        {';',':',3},{';','!',3},{';','i',2},{':','!',3},         \
        {'/','I',2},{'/','1',2},{'/','l',2},                     \
        {'\\','I',2},{'\\','1',2},{'\\','l',2},                  \
        {'{','[',2},{'{','c',2},{'{','C',2},{'{','(',2},{'{','<',2}, \
        {'}',']',2},{'}',']',2},{'}',')',2},{'}','>',2},         \
        {'(','c',2},{'(','C',2},                                 \
        {'[','c',2},{'[','C',2},                                 \
        {'<','c',2},{'<','C',2},                                 \
        {'|','!',5},{'|',';',2},                                 \
        {0,0,0}                                                  \
};
//#ifdef  INTERNATIONAL                                                          \
//                      {'à','a',2},{'á','a',2},{'â','a',2},{'ã','a',2},{'ä','a',2},{'å','a',2},  \
//                      {'à','á',3},{'à','â',3},{'à','ã',3},{'à','ä',3},{'à','å',3},              \
//                      {'á','â',3},{'á','ã',3},{'á','ä',3},{'á','å',3},                          \
//                      {'â','ã',3},{'â','ä',3},{'â','å',3},                                      \
//                      {'ã','ä',3},{'ã','å',3},                                                  \
//                      {'ä','å',3},                                                              \
//#endif                                                                                          \

#define LETTER_SMCAP_ARRAY                                                     \
{                                                        \
        {3,4,5,6,8,9,10,-1},     /* a */                         \
        {0,-1},                  /* b */                         \
        {-1},                    /* c */                         \
        {1,2,6,-1},              /* d */                         \
        {0,1,3,5,9,10,11,12,-1}, /* e */                         \
        {6,8,9,12,-1},           /* f */                         \
        {2,3,4,-1},              /* g */                         \
        {0,1,2,4,-1},            /* h */                         \
        {2,3,5,6,7,-1},          /* i */                         \
        {1,2,4,-1},              /* j */                         \
        {0,1,6,7,10,-1},         /* k */                         \
        {1,-1},                  /* l */                         \
        {2,3,-1},                /* m */                         \
        {2,3,-1},                /* n */                         \
        {-1},                    /* o */                         \
        {2,-1},                  /* p */                         \
        {0,1,-1},                /* q */                         \
        {0,1,-1},                /* r */                         \
        {-1},                    /* s */                         \
        {6,8,-1},                /* t */                         \
        {2,-1},                  /* u */                         \
        {-1},                    /* v */                         \
        {-1},                    /* w */                         \
        {-1},                    /* x */                         \
        {2,-1},                  /* y */                         \
        {0,1,2,4,-1},            /* z */                         \
        {-1}};

using namespace std;

// --------------- File selector global structure ------------------------------

static const char version_id [] = "WritePad SDK Neural Net Experiments\nCopyright (c) 2008-2016 PhatWare Corp. All rights reserved.";
static const char log_name [] = "mlp_lrn.log";


#ifndef __DONT_USE_WINDOWS_H

static const char  STR_FILE_SELECTOR[] = "or ? for selector.";
static char    *szFilterSpec = "Data files\0*.stn\0All files\0*.*\0\0";
static char    szFileName[MAIN_FILENAME_LEN] = { 0 };
static char    szFileTitle[MAIN_FILENAME_LEN] = { 0 };
static char    szWindowOpen[MAIN_FILENAME_LEN] = { 0 };
static OPENFILENAMEA        ofn = { 0 };

#else

static const char  STR_FILE_SELECTOR[] = ".";

int  DrawCoeff(uchar rsym, float w, int num, samples_storage_class * samples, uchar * coeffs)
{
    // SUPPORTED ONLY ON WINDOWSe
    return 0;
}

#endif // __DONT_USE_WINDOWS_H

static _INT langID = LANGUAGE_ENGLISH;

// ---------------- Local prototypes -------------------------------------------

int CppNetImage(char * dat_name, p_mlp_net_type net);
int CppCellImage(FILE * file, mlp_cell_type * cell, int last);

/* **************************************************************************** */
/* *         Main entry point                                                 * */
/* **************************************************************************** */
int main()
{
    uchar key = '?';

#ifndef __DONT_USE_WINDOWS_H
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = NULL;
    ofn.lpstrFilter = szFilterSpec;
    ofn.lpstrCustomFilter = NULL;
    ofn.nMaxCustFilter = 0;
    ofn.nFilterIndex = 1;
    ofn.lpstrFile = szFileName;
    ofn.nMaxFile = MAIN_FILENAME_LEN;
    ofn.lpstrInitialDir = NULL;
    ofn.lpstrFileTitle = szFileTitle;
    ofn.nMaxFileTitle = MAIN_FILENAME_LEN;
    ofn.lpstrTitle = szWindowOpen;
    ofn.lpstrDefExt = NULL;
    ofn.Flags = 0;
    ofn.lpstrInitialDir = "";

    // for file open dialog
    CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    
#endif // __DONT_USE_WINDOWS_H

    cout << version_id << endl;

#if !MLP_LEARN_MODE
    cout << endl << "**** Compiled for test mode only !!!!! ****" << endl;
#endif
#if MLP_PRELOAD_MODE
    cout << endl << "**** Compiled with internal net -- no fileops! ****" << endl;
#endif

    while (key != 0)
    {
        cout << endl;
        cout << "Exit           - e, q" << endl;
        cout << "Create Net     - c" << endl;
        cout << "Learn  Net     - l" << endl;
        cout << "Test   Net     - t" << endl;
        cout << "View Errors    - v" << endl;
        cout << "Analyze data   - a" << endl;
        cout << "Subset data    - s" << endl;
        cout << "Dump Net image - d" << endl;
        cout << "Cpp Net image  - p" << endl;

        cout << "> ";
        cin >> key;
        cout << endl;

        try
        {
            switch (key)
            {
            case 'Q':
            case 'q':
            case 'E':
            case 'e':
                goto err;

            case 'C':
            case 'c':
                MainLearnNet(0);
                break;

            case 'L':
            case 'l':
                MainLearnNet(1);
                break;

            case 'T':
            case 't':
                MainTestNet(0);
                break;

            case 'V':
            case 'v':
                MainTestNet(1);
                break;

            case 'A':
            case 'a':
                MainAnalyzeData(1);
                break;

            case 'S':
            case 's':
                MainAnalyzeData(2);
                break;

            case 'D':
            case 'd':
                MainDumpNetImage(0);
                break;

            case 'P':
            case 'p':
                MainDumpNetImage(1);
                break;
            }
        }

        catch (int code)
        {

            switch (code)
            {
                case 8: 
                    cout << endl << "*** Net file load error *** " << endl << endl;
                    break;

                case 9:
                    cout << endl << "*** Net file save error *** " << endl << endl; 
                    break;

                default:
                    cout << endl << "*** Something went wrong here ... Code: " << code << " *** " << endl << endl;
                    break;
            }
        }
    }


    return 0;
err:
    return 1;
}

/* **************************************************************************** */
/* *           Learn more given net                                           * */
/* **************************************************************************** */

int MainLearnNet(int mode)
{
#if MLP_LEARN_MODE
    int		q, act;
    float	le, te, g_e = 1;
    samples_storage_class * samples = new samples_storage_class();
    samples_storage_class * tst_samples = new samples_storage_class();
    //  char  data_name[MAIN_FILENAME_LEN];
    char    net_bak_name[MAIN_FILENAME_LEN];
    char	net_name[MAIN_FILENAME_LEN];
    char *  p = NULL;
    float	shake = 0;
    fstream log_stream;
    mlp_data_type mlpd = { 0 };
    params_type pm = { 0 };

    if (InitParams(1, MAIN_MLP_INI_NAME, &pm))
        goto err;

    if ((mode & 0x01) == 0) // Create new net
    {
        cout << endl << "Create new net and begin learning" << endl << endl;

        cout << "Name the net: -> ";
        cin >> net_name;
        cout << endl;

        InitSnnData( _NULL, &mlpd, langID );
        InitNetWeights( 0, pm.weigth_init_coeff, &mlpd, langID );
    }
    else
    {
        cout << endl << "Learn existing net" << endl << endl;

        cout << "Enter Net file name (*.mlp), * to use internal net" << STR_FILE_SELECTOR << endl;
        cin >> net_name;
        
        if ( net_name[0] == '*' )
        {
            net_name[0] = 0;
        }
#ifndef __DONT_USE_WINDOWS_H
        else if (net_name[0] == '?')
        {
            strcpy(szWindowOpen, "Select Net file");
            strcpy(szFileName, "*.mlp");
            if (!GetOpenFileName((LPOPENFILENAME) &ofn))
            {
                cout << " No file selected." << endl;
                goto err;
            }
            strcpy(net_name, szFileName);
        }
#endif // __DONT_USE_WINDOWS_H

        if ( InitSnnData( (p_UCHAR)net_name, &mlpd, langID ) )
            goto err;
    }

    cout << "Memory allocated for net is " << sizeof(mlp_net_type) << endl;

#if !MLP_PRELOAD_MODE
    DumpNetData( &mlpd, (p_CHAR)MAIN_DMP_NET_NAME, 1, langID );
#endif

    if (LoadSamples(&pm.learn_data_file, samples, langID))
        goto err;

    if (LoadSamples(&pm.test_data_file, tst_samples, langID))
        goto err;

    log_stream.open(log_name, (ios::out | ios::app));
    if (log_stream.good())
    {
        log_stream << "Log started." << endl << endl;
        log_stream.close();
    }

    cout << "Initial testing..." << endl;
    te = TestNet( &mlpd, tst_samples, log_name, 0, &pm);

#if !MLP_PRELOAD_MODE
    strcpy( net_bak_name, net_name );
    if ((p = strrchr(net_bak_name, '.')) != 0)
        *p = 0;
    strcat(net_bak_name, ".bak");
    SaveNetData( & mlpd, net_bak_name, te, langID );
#endif

    // --------------------- Main Epoch Cycle ------------------------

    for (q = 1; q < 1000000; q++)
    {
        act = AskForParams(&pm, &shake);
        if (act == 2)
            ShakeNetWeights(0, shake, &mlpd, langID);
        if (act == 3) //  || (q % 100 == 99)
        {
            TestNet( &mlpd, samples, log_name, 0, &pm);
            InitNetWeights(1, pm.weigth_init_coeff, &mlpd, langID );
            cout << "\n ZC init with coefficient " << pm.weigth_init_coeff << " done. \n";
        }

        cout << "*** Epoch " << q << " Steps: ";
        for (int i = 1; i < MLP_NET_NUMLAYERS; i++)
            cout << pm.step_coeffs[i] << " ";
        cout << ", LFC: " << pm.logistic_coeff << " ***" << endl;

        if (act == 0)
            ShakeNetWeights(0, pm.weigth_init_coeff, &mlpd, langID); // Noise while learning

        le = LearnNet(&mlpd, samples, &pm, langID);

        fstream log_stream(log_name, (ios::out | ios::app));
        if (log_stream.good())
        {
            log_stream << "*** Epoch " << q << " ***  SQErr: " << le;
            log_stream << ", St: ";
            for (int i = 1; i < MLP_NET_NUMLAYERS; i++) 
                log_stream << pm.step_coeffs[i] << " ";
            log_stream << ", LFC: " << pm.logistic_coeff;
            log_stream << ", ESz: " << pm.epoch_size << endl;
            log_stream.close();
        }

#if !MLP_PRELOAD_MODE
        SaveNetData(&mlpd, net_name, le, langID );
#endif

        if ((q % pm.testing_rate) == 0)
            te = TestNet(&mlpd, tst_samples, log_name, 0, &pm);

#if !MLP_PRELOAD_MODE
        if (g_e > te)
        {
            SaveNetData( &mlpd, (p_CHAR)"best.mlp", te, langID );
            g_e = te;
        }

        if (pm.debug_mode == 1)
            DumpNetData(&mlpd, (p_CHAR)MAIN_DMP_NET_NAME, le, langID );
#endif

        AdjustStepCoeffs(&mlpd, &pm);
    }

    // --------------------- Main Epoch Cycle ------------------------

#if !MLP_PRELOAD_MODE
    if (mlpd.net)
        free( mlpd.net );
#endif
    if (samples)
        delete samples;
    if (tst_samples)
        delete tst_samples;

    return 0;
err:
#if !MLP_PRELOAD_MODE
    if (mlpd.net)
        free( mlpd.net );
#endif
    if (samples)
        delete samples;
    if (tst_samples)
        delete tst_samples;
    return 1;
#else
    return 1;
#endif
}

/* **************************************************************************** */
/* *           Load and test net                                              * */
/* **************************************************************************** */

int MainTestNet(int mode)
{
    char data_name[MAX_DATA_FILES][MAIN_FILENAME_LEN] = { 0 };
    char net_name[MAIN_FILENAME_LEN];
    mlp_data_type mlpd = { 0 };
    samples_storage_class * samples = new samples_storage_class();
    char  log_name [] = "mlp_test.log";
    params_type pm;

    cout << endl << "Test given net" << endl << endl;

    cout << "Enter Net file name (*.mlp), * to use internal net" << STR_FILE_SELECTOR << endl;
    cin >> net_name;
    
    if (InitParams(1, MAIN_MLP_INI_NAME, &pm))
        goto err;

    if ( net_name[0] == '*' )
    {
        net_name[0] = 0;
    }
#ifndef __DONT_USE_WINDOWS_H
    else if (net_name[0] == '?')
    {
        strcpy(szWindowOpen, "Select Net file");
        strcpy(szFileName, "*.mlp");
        if (!GetOpenFileName((LPOPENFILENAME) &ofn))
        {
            cout << " No file selected." << endl;
            goto err;
        }
        strcpy(net_name, szFileName);
    }
#endif // __DONT_USE_WINDOWS_H

    if ( InitSnnData((p_UCHAR) net_name, &mlpd, langID ) )
        goto err;
    cout << "Memory allocated for net is " << sizeof(mlp_net_type) << endl;

#if !MLP_PRELOAD_MODE
    DumpNetData(&mlpd, (p_CHAR)MAIN_DMP_NET_NAME, 1, langID );
#endif

    cout << "Input data file name (*.stn), or ? for selector." << endl;
    cin >> data_name[0];

#ifndef __DONT_USE_WINDOWS_H
    if (data_name[0][0] == '?')
    {
        strcpy(szWindowOpen, "Select data file");
        strcpy(szFileName, "*.stn");
        if (!GetOpenFileName((LPOPENFILENAME) &ofn))
        {
            cout << " No file selected." << endl;
            goto err;
        }
        strcpy(data_name[0], szFileName);
    }
#endif // __DONT_USE_WINDOWS_H

    if (LoadSamples(&data_name, samples, langID))
        goto err;

    TestNet(&mlpd, samples, log_name, mode, &pm);

#if !MLP_PRELOAD_MODE
    SaveNetData(&mlpd, (p_CHAR)"bak.net", 1, langID );
#endif

#if !MLP_PRELOAD_MODE
    if (mlpd.net)
        free( mlpd.net );
#endif
    if (samples)
        delete samples;

    return 0;
err:
#if !MLP_PRELOAD_MODE
    if (mlpd.net)
        free( mlpd.net );
#endif
    if (samples)
        delete samples;
    return 1;
}

/* **************************************************************************** */
/* *           Analyze data file                                              * */
/* **************************************************************************** */
int MainAnalyzeData(int mode)
{
    char data_name[MAX_DATA_FILES][MAIN_FILENAME_LEN] = { 0 };
    samples_storage_class * samples = new samples_storage_class();
    char  log_name [] = "ana_dat.log";
    char  delt_ext [] = ".stc";


    cout << "Input data file name (*.stn), or ? for selector -> ";
    cin >> data_name[0];
#ifndef __DONT_USE_WINDOWS_H
    if (data_name[0][0] == '?')
    {
        strcpy(szWindowOpen, "Select data file");
        strcpy(szFileName, "*.stn");
        if (!GetOpenFileName((LPOPENFILENAME) &ofn))
        {
            cout << " No file selected." << endl;
            goto err;
        }
        strcpy(data_name[0], szFileName);
    }
#endif // __DONT_USE_WINDOWS_H

    if (LoadSamples(&data_name, samples, langID))
        goto err;

    if (mode == 1)
    {
        cout << "Let's create some log ..." << endl;
        if (LogSamplesStat(samples, log_name))
            cout << "Could not create stat log " << log_name << endl;
        else cout << "Data stat log created in " << log_name << endl;
    }

    if (mode == 2) // Create 10000 sample subset from data file
    {
        char  name[MAIN_FILENAME_LEN];
        char *ptr;

        strcpy(name, data_name[0]);
        if ((ptr = strrchr(name, '.')) != 0)
            *ptr = 0;
        strcat(name, delt_ext);
        cout << "Lets's create some data subset ..." << endl;
        if (WriteBackSomeSamples(samples, name))
            cout << "Could not extract samples to " << name << endl;
        else cout << "Some random samples extracted to " << name << endl;
    }

    if (samples) delete samples;
    return 0;
err:
    if (samples) delete samples;
    return 1;
}

/* **************************************************************************** */
/* *         Create C file with Net image                                     * */
/* **************************************************************************** */
int MainDumpNetImage(int mode)
{
    char data_name[MAIN_FILENAME_LEN];
    char net_name[MAIN_FILENAME_LEN];
    char * ptr;
    mlp_data_type mlpd = { 0 };
    params_type pm;


    cout << endl << "Create C image of given net" << endl << endl;

    cout << "Enter Net file name (*.mlp), * to use internal net" << STR_FILE_SELECTOR << endl;
    cin >> net_name;
    
    if (InitParams(1, MAIN_MLP_INI_NAME, &pm))
        goto err;
    
    if ( net_name[0] == '*' )
    {
        net_name[0] = 0;
    }
#ifndef __DONT_USE_WINDOWS_H
    else if (net_name[0] == '?')
    {
        strcpy(szWindowOpen, "Select Net file");
        strcpy(szFileName, "*.mlp");
        if (!GetOpenFileName((LPOPENFILENAME) &ofn))
        {
            cout << " No file selected." << endl;
            goto err;
        }
        strcpy(net_name, szFileName);
    }
#endif // __DONT_USE_WINDOWS_H

    if ( InitSnnData( (p_UCHAR) net_name, &mlpd, langID ) )
        goto err;
    cout << "Memory allocated for net is " << sizeof(mlp_net_type) << endl;

#if !MLP_PRELOAD_MODE
    if (pm.debug_mode == 1)
        DumpNetData(&mlpd, (p_CHAR)MAIN_DMP_NET_NAME, 1, langID );
#endif

    strcpy(data_name, net_name);
    if ((ptr = strrchr(data_name, '.')) != 0)
        *ptr = 0;
    strcat(data_name, ".cpp");

    if (mode == 0)
        DumpNetImage(data_name, &mlpd);
    if (mode == 1)
        CppNetImage(data_name, (p_mlp_net_type) mlpd.net);

#if !MLP_PRELOAD_MODE
    if (mlpd.net)
        free( mlpd.net );
#endif
    return 0;
err:
#if !MLP_PRELOAD_MODE
    if (mlpd.net)
        free( mlpd.net );
#endif
    return 1;
}

/* **************************************************************************** */
/* *           Ask for parameter change                                       * */
/* **************************************************************************** */
int AskForParams(p_params_type pm, float * shake)
{
    int act = 0;
    uchar key = '?';

    while (key != 0)
    {
        cout << endl;
        cout << "Reload params   - r" << endl;
        cout << "Shake net       - s" << endl;
        cout << "Init zero cells - z" << endl;
        cout << "Change steps    - c" << endl;
        cout << ">> ";

        cin >> key;
        cout << endl;

        switch (key)
        {
        case 'r':
        case 'R':
        {
            InitParams(1, MAIN_MLP_INI_NAME, pm);
            act = 1;
            break;
        }

        case 's':
        case 'S':
        {
            cout << "Input shake coeff: ";
            cin >> *shake;
            cout << endl;
            act = 2;
            break;
        }

        case 'z':
        case 'Z':
        {
            cout << "Let's init zero cells..." << endl;
            act = 3;
            break;
        }

        case 'c':
        case 'C':
        {
            for (int i = 1; i < MLP_NET_NUMLAYERS; i++)
            {
                cout << "Input layer " << i << " step -> ";
                cin >> pm->step_coeffs[i];
            }

            act = 4;
            break;
        }
        }
    }

    return act;
}

/* **************************************************************************** */
/* *           Init parameters structure                                      * */
/* **************************************************************************** */
int InitParams(int mode, const char * fname, p_params_type pm)
{
    int   i, k;
    char  str[1024];
    char  name[1024];
    char  c;
    char *ptr;
    char  param_names [][32] = DEF_PARAM_NAMES;
    uchar s;
    int   v;

    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);

    if (mode == 1) // Read ini file
    {
        int num_read = 0;
        int num_params;
        int no = 0;
        int no23 = 0;
        int no24 = 0;

        for (num_params = 0; num_params < 100 && param_names[num_params][0]; num_params++);

        fstream fin(fname);

        if (fin.good())
        {
            while (!fin.eof())
            {
                fin >> c;

                if (c != '>') continue;

                fin >> name;

                for (i = 0; i < num_params; i++)
                {
                    strcpy(str, param_names[i]);
                    if ((ptr = strchr(str, ' ')) != 0)
                        *ptr = 0;
#ifndef __DONT_USE_WINDOWS_H
                    if ( _stricmp((const char *) str, (const char *) name) == 0 )
#else
                    if ( strcasecmp((const char *) str, (const char *) name) == 0 )
#endif
                    {
                        num_read++;

                        switch (i)
                        {
                        case  0:
                            fin >> pm->step_mode;
                            break;
                        case  1:
                            fin >> pm->step_size;
                            break;
                        case  2:
                            fin >> pm->step_decay_coeff;
                            break;
                        case  3:
                            fin >> pm->step_min;
                            break;
                        case  4:
                            fin >> pm->step_layer_coeffs[1];
                            break;
                        case  5:
                            fin >> pm->step_layer_coeffs[2];
                            break;
                        case  6:
                            fin >> pm->step_layer_coeffs[3];
                            break;
                        case  7:
                            fin >> pm->step_layer_coeffs[4];
                            break;
                        case  8:
                            fin >> pm->step_coeffs[1];
                            break;
                        case  9:
                            fin >> pm->step_coeffs[2];
                            break;
                        case 10:
                            fin >> pm->step_coeffs[3];
                            break;
                        case 11:
                            fin >> pm->step_coeffs[4];
                            break;
                        case 12:
                            fin >> pm->logistic_coeff;
                            break;
                        case 13:
                            fin >> pm->logistic_decay_coeff;
                            break;
                        case 14:
                            fin >> pm->logistic_max;
                            break;
                        case 15:
                            fin >> pm->weigth_init_coeff;
                            break;
                        case 16:
                            fin >> pm->inert_coeff;
                            break;
                        case 17:
                            fin >> pm->zero_ballance_coeff;
                            break;
                        case 18:
                            fin >> pm->epoch_size;
                            break;
                        case 19:
                            fin >> pm->epoch_size_decay;
                            break;
                        case 20:
                            fin >> pm->epoch_size_max;
                            break;
                        case 21:
                            fin >> pm->testing_rate;
                            break;
                        case 22:
                            fin >> pm->debug_mode;
                            break;
                        case 23:
                            fin >> pm->learn_data_file[no23];
                            if (no23 < MAX_DATA_FILES) no23++;
                            break;
                        case 24: fin >> pm->test_data_file[no24]; if (no24 < MAX_DATA_FILES) no24++;
                            break;
                        case 25: fin >> s; fin >> v; pm->symco[no] = s;
                            pm->bushes[no] = (uchar) v; no++;
                            break;

                        }
                    }
                }
            }

            fin.close();
            if (num_read != num_params + info.output - 1)
                cout << "============== !!!!! Not all params read from ini !!!! ========== " << endl;
        }
        else
        {
            cout << "Can't open " << fname << " file!" << endl;
            goto err;
        }
    }

    if (mode == 2)  // Write back ini file
    {
        ofstream fout(fname);
        p_UCHAR symco = GetSnnSymbol(langID);

        if (fout.good())
        {
            fout << "******** " << ver_h << " params file. *********" << endl << endl;

            for (i = 0; i < 100 && param_names[i][0]; i++)
            {
                fout << "> " << param_names[i];
                switch (i)
                {
                case  0:
                    fout << pm->step_mode;
                    break;
                case  1:
                    fout << pm->step_size;
                    break;
                case  2:
                    fout << pm->step_decay_coeff;
                    break;
                case  3:
                    fout << pm->step_min << endl;
                    break;
                case  4:
                    fout << pm->step_layer_coeffs[1];
                    break;
                case  5:
                    fout << pm->step_layer_coeffs[2];
                    break;
                case  6:
                    fout << pm->step_layer_coeffs[3];
                    break;
                case  7:
                    fout << pm->step_layer_coeffs[4] << endl;
                    break;
                case  8:
                    fout << pm->step_coeffs[1];
                    break;
                case  9:
                    fout << pm->step_coeffs[2];
                    break;
                case 10:
                    fout << pm->step_coeffs[3];
                    break;
                case 11:
                    fout << pm->step_coeffs[4] << endl;
                    break;
                case 12:
                    fout << pm->logistic_coeff;
                    break;
                case 13:
                    fout << pm->logistic_decay_coeff;
                    break;
                case 14:
                    fout << pm->logistic_max << endl;
                    break;
                case 15:
                    fout << pm->weigth_init_coeff;
                    break;
                case 16:
                    fout << pm->inert_coeff;
                    break;
                case 17:
                    fout << pm->zero_ballance_coeff << endl;
                    break;
                case 18:
                    fout << pm->epoch_size;
                    break;
                case 19:
                    fout << pm->epoch_size_decay;
                    break;
                case 20:
                    fout << pm->epoch_size_max << endl;
                    break;
                case 21:
                    fout << pm->testing_rate << endl;
                    break;
                case 22:
                    fout << pm->debug_mode << endl;
                    break;
                case 23:
                    fout << pm->learn_data_file[0] << endl;
                    for (k = 1; k < MAX_DATA_FILES && pm->learn_data_file[k][0] != 0; k++)
                        fout << "> " << param_names[i] << pm->learn_data_file[k] << endl;
                    break;
                case 24: fout << pm->test_data_file[0] << endl;
                    for (k = 1; k < MAX_DATA_FILES && pm->test_data_file[k][0] != 0; k++)
                        fout << "> " << param_names[i] << pm->test_data_file[k] << endl;
                    break;
                case 25: fout << (char) symco[0] << "      " << (int) pm->bushes[0] << endl;
                    for (k = 1; k < info.output; k++)
                        fout << "> " << param_names[i] << (char) symco[k] << "      " << (int) pm->bushes[k] << endl;
                    break;
                }

                fout << endl;
            }

            fout << endl << "************** End of params file. *************" << endl;

            fout.close();
        }
        else
        {
            cout << "Can't write " << fname << " file." << endl;
        }
    }


    return 0;
err:
    return 1;
}

#if MLP_LEARN_MODE
/* **************************************************************************** */
/* *          Learn net on the given samples                                  * */
/* **************************************************************************** */
float LearnNet(p_mlp_data_type mlpd, samples_storage_class * samples, p_params_type pm, _INT langID)
{
    int   ns, n;
    int   corr0 = 0, corr1 = 0, rej0 = 0, rej1 = 0;
    int   num0 = 0, num1 = 0;
    float a, e;
    float sum_error = 0;
    uchar what;
    uchar syms[MAIN_NUMSYMINSAMP + 2];
    uchar inps[MLP_NET_NUMINPUTS];
    float desired_outputs[MLP_NET_MAX_NUMOUTPUTS];
    uchar outputs[MLP_NET_MAX_NUMOUTPUTS];
    p_sample_type psamp;
    char  str[MAIN_FILENAME_LEN];
    time_t tp, t, dt = 0;
    time_t ttp = 0, tt;
    time_t stime, dtime, ctime, etime, atime, ptime;
    static int s_num_steps = 0;

    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);


    if (samples->num_samples <= 0)
        goto err;

    CountNetStats(3, mlpd, langID );

#if MLP_LEARN_MODE
    FillExpTable(pm->logistic_coeff, mlpd);
#endif

    stime = dtime = ctime = etime = atime = ptime = 0;
    ns = MAIN_NSAMP_PV; // <- Max Number of samples per variant
    GetInputsAtRandom(0x00, &what, inps, syms, &ns, samples);
    //  GetInputsSequencial(0x00, &what, inps, syms, &ns, samples);

    for (n = 0; n < 1000000; n++, s_num_steps++)
    {
        time(&tp);

        if (GetInputsAtRandom(0x01, &what, inps, syms, &ns, samples))
            break;
        //  if (GetInputsSequencial(0x01, &what, inps, desired_outputs, &ns, samples)) break;

        samples->GetSample(ns, &psamp);

        time(&t); stime += t - tp; tp = t;

        if (n % 500 == 0)
        {
            DrawCoeff(0, 0, ns, samples, inps);
            if (n % 10000 == 0)
            {
                time(&tt);
                if (n == 0)
                    ttp = tt;
                dt = tt - ttp;
                ttp = tt;
            }
            cout << "Learned " << n << " samples, " << dt << "s per 10000e.      " << "\r";
        }

        time(&t);
        dtime += t - tp; 
        tp = t;

        CreateDesiredOutputs((uchar) (psamp->nvs[0] & 0x0f), psamp->syms[0], syms, (p_UCHAR) GetSnnSymbol( langID ), desired_outputs);

        CountNetResult(inps, outputs, mlpd, langID );

        time(&t);
        ctime += t - tp;
        tp = t;

        CountNetError(desired_outputs, pm->zero_ballance_coeff, mlpd, langID);

        time(&t);
        etime += t - tp; 
        tp = t;

        ModifyNetDeltas(0, mlpd, langID);

        time(&t);
        atime += t - tp; 
        tp = t;

        if (s_num_steps % (int) pm->epoch_size == ((int) pm->epoch_size - 1))
        {
            AdjustNetWeights(1, &(pm->step_coeffs[0]), pm->inert_coeff, mlpd, langID);
            CountNetStats(2, mlpd, langID);
            //      ResetNetPageVars();
        }

        time(&t); ptime += t - tp;

        for (int i = 0; i < info.output; i++)
        {
            if (desired_outputs[i] == 0) continue;

            a = (float) outputs[i] / 256;
            e = desired_outputs[i] - a;

            if (desired_outputs[i] < 0.5 && desired_outputs[i] > 0.01)
                e /= (desired_outputs[i] * 100);

            sum_error += e*e; //MLP_ERR(e);

            if (desired_outputs[i] > 0.5)
            {
                if (a > 0.6) corr0++;
                else if (a > 0.4) rej0++;
                num0++;
            }
            else
            {
                if (a < 0.4) corr1++;
                else if (a < 0.6) rej1++;
                num1++;
            }
        }
    }

    sprintf(str, "E: %8.6f, C0:%6.2f, R0:%6.2f, C1:%6.2f, R1:%6.2f, N0: %d, N1: %d\n",
        sum_error / (num0 + num1), (100.0*corr0) / num0, (100.0*rej0) / (num0),
        (100.0*corr1) / num1, (100.0*rej1) / (num1), num0, num1);

    cout << str;

    sprintf(str, "Timing: Samp: %lds, Disp: %lds, Count: %lds, Err: %lds, Adj: %lds, Pp: %lds\n",
        stime, dtime, ctime, etime, atime, ptime);
    cout << str;

    return sum_error / (num0 + num1);
err:
    return 1;
}

#endif // MLP_TEST_NET

/* **************************************************************************** */
/* *         Test net on given samples                                        * */
/* **************************************************************************** */
int  CreateDesiredOutputs(uchar ncorvar, uchar corr_sym, uchar * syms, uchar * symco, float * desired_outputs)
{
    int     j, k, n;
    uchar   sym;
    uchar   pairs [][3] = LETTER_MIXUP_ARRAY;
    schar   smcap [][16] = LETTER_SMCAP_ARRAY;

    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);

    for (k = 0; k < info.output; k++)
    {
        sym = symco[k];

        //    for (i = 0, l1p = 0; syms[i] != 0; i ++)
        //      if (syms[i] == symco[k]) {l1p = 1; break;}
        //
        //    if (l1p == 0) desired_outputs[k] = 0.05;
        //     else
        {
            //      if (i == 0) // Correct answer
            if (sym == corr_sym) // Correct answer
            {
                desired_outputs[k] = 0.99f;
                if (islower(syms[0])) // Reduce desired for sm cap vars of sm letters
                {
                    for (j = 0, n = syms[0] - 'a'; j < 16 && smcap[n][j] != -1; j++)
                    {
                        if (smcap[n][j] == ncorvar)
                        {
                            desired_outputs[k] = 0.90f;
                            break;
                        }
                    }
                }
            }
            else       // Incorrect answer
            {
                desired_outputs[k] = 0.01f;

                if (isalpha(sym) && tolower(sym) == tolower(corr_sym))
                    desired_outputs[k] += 0.02f;
                else
                {
                    for (j = 0; j < 100 && pairs[j][0] != 0; j++)
                    {
                        if ((pairs[j][0] == corr_sym && pairs[j][1] == sym) ||
                            (pairs[j][0] == sym  && pairs[j][1] == corr_sym))
                        {
                            desired_outputs[k] += ((float) (pairs[j][2] - 1)) / 100.0f;
                            break;
                        }
                    }
                }
            }
        }
    }

    return 0;
}
/* **************************************************************************** */
/* *         Test net on given samples                                        * */
/* **************************************************************************** */

float TestNet(p_mlp_data_type mlpd, samples_storage_class * samples, const char * log_name, int mode, p_params_type pm)
{
    int   i, j, k, ns;
    int   dcorr0, dcorr1, drej0, drej1;
    int   num0, num1, dcount;
    float a, d, e;
    uchar what, sym;
    uchar syms[MAIN_NUMSYMINSAMP + 2];
    uchar inps[MLP_NET_NUMINPUTS];
    uchar outputs[MLP_NET_MAX_NUMOUTPUTS];
    float desired_outputs[MLP_NET_MAX_NUMOUTPUTS];
    float sum_error = 0;
    int   reject_err[2][11] = { 0 };
    int   corr_err[2][2][11] = { 0 };
    short let_stat[256][16][8] = { 0 };
    float reject_levels[11] = { 0.05f, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 0.95f };
    char  str[MAIN_FILENAME_LEN];
    p_sample_type psamp;
    char  sym_log_name [] = "sym_log.xll";
    uchar *symco = (p_UCHAR) GetSnnSymbol(langID);
    //  float lfc = pm->logistic_coeff;

    memset(reject_err, 0, sizeof(reject_err));
    memset(corr_err, 0, sizeof(corr_err));
    memset(let_stat, 0, sizeof(let_stat));

    uchar key = 0;

    mlp_size_info snn_sizeinfo;
    GetSnnSizeInfo(langID, &snn_sizeinfo);

    num0 = num1 = dcount = 0;
    dcorr0 = dcorr1 = drej0 = drej1 = 0;

    GetInputsSequencial(0x00, &what, inps, syms, &ns, samples);

#if MLP_LEARN_MODE
    FillExpTable(pm->logistic_coeff, mlpd);
    CountNetStats(0, mlpd, langID);
#endif

    while (GetInputsSequencial(0x01, &what, inps, syms, &ns, samples) == 0)
    {
        if ((num0+num1) % 256 == 0)
            PS_Yield();
        if ((num0 + num1) % 256 == 0)
            cout << "Tested " << (num0 + num1) << " samples " << "\r";

        samples->GetSample(ns, &psamp);

        CreateDesiredOutputs((uchar) (psamp->nvs[0] & 0x0f), psamp->syms[0], syms, symco, desired_outputs);

        //    net->CountNetResult(0x03, inps, &outputs[0], syms, &mlpd);
        CountNetResult(inps, &outputs[0], mlpd, langID);

#if MLP_LEARN_MODE
        CountNetStats(1, mlpd, langID);
#endif

        for (i = 0; i < snn_sizeinfo.output; i++)
        {
            a = (float) outputs[i] / 256.0f;
            d = desired_outputs[i];
            e = d - a;

            if (d == 0)
                continue;

            if (d < 0.5 && d > 0.01)
                e /= (d * 100);

            sum_error += e*e; //MLP_ERR(e);

            if (d > 0.5)
            {
                if (a > 0.6)
                    dcorr0++;
                else if (a > 0.4)
                    drej0++;
                else if (mode == 1)
                {
                    DrawCoeff(symco[i], a, ns, samples, inps);
                    cin >> key;
                }
                else
                {
                    dcount++;
                    if ((dcount % 32) == 0)
                        DrawCoeff(symco[i], a, ns, samples, inps);
                }

                for (k = 0; k < 11; k++)
                {
                    if (a < reject_levels[k])
                        reject_err[0][k] ++;
                    if (a > reject_levels[k])
                        corr_err[0][0][k] ++;
                    else
                        corr_err[0][1][k] ++;
                }

                num0++;
            }
            else
            {
                if (a < 0.4)
                    dcorr1++;
                else if (a < 0.6)
                    drej1++;
                else if (mode == 1)
                {
                    DrawCoeff(symco[i], a, ns, samples, inps);
                    cin >> key;
                }
                else
                {
                    dcount++;
                    if ((dcount % 32) == 0)
                        DrawCoeff(symco[i], a, ns, samples, inps);
                }

                for (k = 0; k < 11; k++)
                {
                    if (a > reject_levels[k])
                        reject_err[1][k] ++;
                    if (a < reject_levels[k])
                        corr_err[1][0][k] ++;
                    else
                        corr_err[1][1][k] ++;
                }

                num1++;
            }


            sym = symco[i];
            if (d > 0.5)
                if (a >= 0.5)
                    k = 0;
                else
                    k = 1;
            else
                if (a <= 0.5)
                    k = 2;
                else
                    k = 3;
            for (j = 0; j < MAIN_NUMSYMINSAMP; j++)
            {
                if (psamp->syms[j] == sym)
                    break;
            }
            j = (j % 2) ? psamp->nvs[j / 2] & 0x0f : psamp->nvs[j / 2] >> 4;
            let_stat[sym][j][k] ++;
            if (d > 0.5)
            {
                k = 0; if (a > 0.7) k = 4; if (a < 0.3) k = 5;
            }
            else
            {
                k = 0; if (a < 0.3) k = 6; if (a > 0.7) k = 7;
            }
            if (k)
                let_stat[sym][j][k] ++;
        }
    }

    ofstream log_stream(log_name, (ios::out | ios::app));

    sprintf(str, "E: %8.6f, C0:%6.2f, R0:%6.2f, C1:%6.2f, R1:%6.2f, N0: %d, N1: %d\n",
        sum_error / (num0 + num1), (100.0*dcorr0) / num0, (100.0*drej0) / (num0),
        (100.0*dcorr1) / num1, (100.0*drej1) / (num1), num0, num1);

    cout << str;

    if (log_stream) log_stream << str;

    if (num0 > 0 && num1 > 0)
    {
        for (i = 0; i < 11; i++)
        {
            sprintf(str, "%6.2f ", (float) reject_err[0][i] * 100. / num0);
            cout << str;
            if (log_stream) log_stream << str;
        }
        cout << endl;
        if (log_stream) log_stream << endl;

        for (i = 0; i < 11; i++)
        {
            sprintf(str, "%6.2f ", (float) reject_err[1][i] * 100. / num1);
            cout << str;
            if (log_stream) log_stream << str;
        }
        cout << endl;
        if (log_stream) log_stream << endl;

        if (pm->debug_mode == 1)
        {
            if (log_stream)
            {
                log_stream << endl << "Reject scale." << endl;
                for (i = 0; i < 11; i++)
                {
                    sprintf(str, "%6.2f ", (float) reject_levels[i]);
                    //      cout << str;
                    log_stream << str;
                }
                log_stream << endl;

                log_stream << "Incorrect sym reject distribution of diff. corr/err." << endl;
                for (i = 0; i < 11; i++)
                {
                    sprintf(str, "%6.2f ", (float) corr_err[0][0][i] * 100. / num0);
                    //      cout << str;
                    log_stream << str;
                }
                log_stream << endl;

                for (i = 0; i < 11; i++)
                {
                    sprintf(str, "%6.2f ", (float) corr_err[0][1][i] * 100. / num0);
                    //      cout << str;
                    log_stream << str;
                }
                log_stream << endl;

                log_stream << "Correct sym reject distribution of diff. corr/err." << endl;
                for (i = 0; i < 11; i++)
                {
                    sprintf(str, "%6.2f ", (float) corr_err[1][0][i] * 100. / num1);
                    //      cout << str;
                    log_stream << str;
                }
                log_stream << endl;

                for (i = 0; i < 11; i++)
                {
                    sprintf(str, "%6.2f ", (float) corr_err[1][1][i] * 100. / num1);
                    //      cout << str;
                    log_stream << str;
                }
                log_stream << endl;
            }

            if (log_stream) log_stream << endl;
        }
    }

    if (log_stream) log_stream << endl;
    if (log_stream) log_stream.close();

    if (pm->debug_mode == 1)
    {
        ofstream sl_stream(sym_log_name);

        if (sl_stream.good())
        {
            sl_stream << endl << endl << "Weighted Correct sample recognition:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i]; if (sym == '"') sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float) (let_stat[sym][k][0] + let_stat[sym][k][1]);
                    if (a > 0)
                        a = 100.0f * (float) let_stat[sym][k][0] / a;
                    else
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }
            sl_stream << endl << endl << "Weighted Correct symbol error:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i];
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float) (let_stat[sym][k][0] + let_stat[sym][k][1]);
                    if (a > 0)
                        a = 100.0f * (float) let_stat[sym][k][1] / a;
                    else
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }

            sl_stream << endl << endl << "Weighted Correct sample recognition with reject:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i];
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float)(let_stat[sym][k][0] + let_stat[sym][k][1]);
                    if (a > 0)
                        a = 100.0f * (float) let_stat[sym][k][4] / a;
                    else
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }
            sl_stream << endl << endl << "Weighted Correct symbol error with reject:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i]; if (sym == '"') sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float) (let_stat[sym][k][0] + let_stat[sym][k][1]);
                    if (a > 0)
                        a = 100.0f * (float) let_stat[sym][k][5] / a;
                    else
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }

            sl_stream << endl << endl << "UnWeighted Correct sample recognition:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i];
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float) (let_stat[sym][k][0] + let_stat[sym][k][1]);
                    if (a > 0)
                        a = (float) let_stat[sym][k][0];
                    else
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }
            sl_stream << endl << endl << "UnWeighted Correct symbol error:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i];
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float) (let_stat[sym][k][0] + let_stat[sym][k][1]);
                    if (a > 0)
                        a = (float) let_stat[sym][k][1];
                    else
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }

            sl_stream << endl << endl << "UnWeighted Correct sample recognition with reject:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i];
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float) (let_stat[sym][k][0] + let_stat[sym][k][1]);
                    if (a > 0)
                        a = (float) let_stat[sym][k][4];
                    else
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }
            sl_stream << endl << endl << "UnWeighted Correct symbol error with reject:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i]; 
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float)(let_stat[sym][k][0] + let_stat[sym][k][1]);
                    if (a > 0) 
                        a = (float) let_stat[sym][k][5];
                    else 
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }

            sl_stream << endl << endl << "Weighted Incorrect sample recognition:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i];
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float)(let_stat[sym][k][2] + let_stat[sym][k][3]);
                    if (a > 0) 
                        a = 100.0f * (float) let_stat[sym][k][2] / a; 
                    else a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }
            sl_stream << endl << endl << "Weighted Incorrect symbol error:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i]; 
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float)(let_stat[sym][k][2] + let_stat[sym][k][3]);
                    if (a > 0) 
                        a = 100.0f * (float) let_stat[sym][k][3] / a; 
                    else
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }

            sl_stream << endl << endl << "Weighted Incorrect sample recognition with reject:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i]; 
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float)(let_stat[sym][k][2] + let_stat[sym][k][3]);
                    if (a > 0)
                        a = 100.0f * (float) let_stat[sym][k][6] / a;
                    else 
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }
            sl_stream << endl << endl << "Weighted Incorrect symbol error with reject:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i]; 
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float)(let_stat[sym][k][2] + let_stat[sym][k][3]);
                    if (a > 0) 
                        a = 100.0f * (float) let_stat[sym][k][7] / a; 
                    else 
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }

            sl_stream << endl << endl << "Weighted correct samples reject:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                float c;

                sym = symco[i];
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float) (let_stat[sym][k][0] - let_stat[sym][k][4]);
                    a += (float) (let_stat[sym][k][1] - let_stat[sym][k][5]);
                    c = (float) (let_stat[sym][k][0] + let_stat[sym][k][1]);
                    if (c > 0)
                        a = 100.0f * a / c; 
                    else
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }

            sl_stream << endl << endl << "Weighted incorrect samples reject:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                float c;

                sym = symco[i];
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float) (let_stat[sym][k][2] - let_stat[sym][k][6]);
                    a += (float) (let_stat[sym][k][3] - let_stat[sym][k][7]);
                    c = (float) (let_stat[sym][k][2] + let_stat[sym][k][3]);
                    if (c > 0) 
                        a = 100.0f * a / c;
                    else
                        a = -1.0f;
                    sl_stream << "\t" << a;
                }
            }

            sl_stream << endl << endl << "UnWeighted correct samples reject:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i]; 
                if (sym == '"') 
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float) (let_stat[sym][k][0] - let_stat[sym][k][4]);
                    a += (float) (let_stat[sym][k][1] - let_stat[sym][k][5]);
                    sl_stream << "\t" << a;
                }
            }

            sl_stream << endl << endl << "UnWeighted incorrect samples reject:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i]; 
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float) (let_stat[sym][k][2] - let_stat[sym][k][6]);
                    a += (float) (let_stat[sym][k][3] - let_stat[sym][k][7]);
                    sl_stream << "\t" << a;
                }
            }

            sl_stream << endl << endl << "Number of Correct samples:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i]; 
                if (sym == '"')
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float) (let_stat[sym][k][0] + let_stat[sym][k][1]);
                    sl_stream << "\t" << a;
                }
            }

            sl_stream << endl << endl << "Number of Incorrect samples:" << endl << endl;
            for (i = 0; i < snn_sizeinfo.output; i++)
            {
                sym = symco[i];
                if (sym == '"') 
                    sym = '`';
                sl_stream << endl << sym;
                for (k = 0; k < 16; k++)
                {
                    a = (float)(let_stat[sym][k][2] + let_stat[sym][k][3]);
                    sl_stream << "\t" << a;
                }
            }

            sl_stream.close();
        }
    }

    return sum_error / (num0 + num1);
}

#if MLP_LEARN_MODE

/* **************************************************************************** */
/* *         Set new values for step coeffs                                   * */
/* **************************************************************************** */

int AdjustStepCoeffs(p_mlp_data_type mlpd, p_params_type pm)
{
    int   i;
    int   nz1, nz2, nz3;
    float delt[MLP_NET_NUMLAYERS];
    p_mlp_net_type net = (p_mlp_net_type) mlpd->net;

    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);

    const int MLP_NET_1L_NUMCELLS = info.net_cells_1;
    const int MLP_NET_2L_NUMCELLS = info.net_cells_2;
    const int MLP_NET_3L_NUMCELLS = info.net_cells_3;

    InitParams(1, MAIN_MLP_INI_NAME, pm);

    pm->epoch_size *= pm->epoch_size_decay;
    if (pm->epoch_size > pm->epoch_size_max) pm->epoch_size = pm->epoch_size_max;

    pm->step_size *= pm->step_decay_coeff;
    if (pm->step_size < pm->step_min) pm->step_size = pm->step_min;

    pm->logistic_coeff = pm->logistic_coeff * pm->logistic_decay_coeff;
    if (pm->logistic_coeff > pm->logistic_max) pm->logistic_coeff = pm->logistic_max;

    // ------------------------- Step auto adjust ----------------------------------

    delt[1] = (net->layers[1].num_sum) ? (pm->epoch_size*net->layers[1].sum_delt) / (float) net->layers[1].num_sum : 0;
    delt[2] = (net->layers[2].num_sum) ? (pm->epoch_size*net->layers[2].sum_delt) / (float) net->layers[2].num_sum : 0;
    delt[3] = (net->layers[3].num_sum) ? (pm->epoch_size*net->layers[3].sum_delt) / (float) net->layers[3].num_sum : 0;
    //  delt[4] = (net->layer4.num_sum) ? (pm->epoch_size*net->layer4.sum_delt)/(float)net->layer4.num_sum : 0;

    if (pm->step_mode == 1)
    {
        for (i = 1; i < MLP_NET_NUMLAYERS; i++)
        {
            pm->step_coeffs[i] = pm->step_size * pm->step_layer_coeffs[i];
        }
    }

    if (pm->step_mode == 2)
    {
        for (i = 1; i < net->num_layers; i++)
        {
            if (pm->step_coeffs[i] <= 0) pm->step_coeffs[i] = pm->step_size*pm->step_layer_coeffs[i];

            if (delt[i] > 0)
            {
                if (delt[i] > pm->step_size*pm->step_layer_coeffs[i])
                    pm->step_coeffs[i] /= 1.25;
                if (delt[i] < pm->step_size*pm->step_layer_coeffs[i])
                    pm->step_coeffs[i] *= 1.25;
                if (delt[i] > pm->step_size*pm->step_layer_coeffs[i] * 8)
                    pm->step_coeffs[i] /= 2;
                if (delt[i] < pm->step_size*pm->step_layer_coeffs[i] / 8)
                    pm->step_coeffs[i] *= 2;

                if (pm->step_coeffs[i] > MAIN_MAX_STEP)
                    pm->step_coeffs[i] = MAIN_MAX_STEP;
            }
        }
    }

    // ---------------------- Count zero change cells ------------------------------

    for (i = 0, nz1 = 0; i < MLP_NET_1L_NUMCELLS; i++)
    {
        if (net->cells[i].num_changes == 0)
            nz1++;
    }
    for (i = 0, nz2 = 0; i < MLP_NET_2L_NUMCELLS; i++)
    {
        if (net->cells[i + MLP_NET_1L_NUMCELLS].num_changes == 0)
            nz2++;
    }
    for (i = 0, nz3 = 0; i < MLP_NET_3L_NUMCELLS; i++)
    {
        if (net->cells[i + MLP_NET_1L_NUMCELLS + MLP_NET_2L_NUMCELLS].num_changes == 0)
            nz3++;
    }
    // ---------------------- Save INI and dump stat -------------------------------

    InitParams(2, MAIN_MLP_INI_NAME, pm);

    fstream log_stream(log_name, (ios::out | ios::app));
    if (log_stream.good())
    {
        log_stream << "W incr. by layer: ";
        cout << "W.Incr: ";
        for (int i = 1; i < MLP_NET_NUMLAYERS; i++)
        {
            log_stream << delt[i] << ", "; cout << delt[i] << " ";
        }
        cout << " NZcells: " << nz1 << " " << nz2 << " " << nz3 << endl;
        log_stream << " NZcells: " << nz1 << " " << nz2 << " " << nz3 << endl;

        log_stream.close();
    }

    return 0;
}
#endif // #if MLP_LEARN_MODE

/* ************************************************************************ */
/*   Yield execution to other processes                                     */
/* ************************************************************************ */

void err_msg(char * str)
{
    cout << "**** Err Msg ***** " << str << " ****\n";
}

/* **************************************************************************** */
/* *           Log data stat                                                  * */
/* **************************************************************************** */

int LogSamplesStat(samples_storage_class * samples, char * log_name)
{
    int   i, j, k;
    int   stat[256][16][2];
    char  str[64];
    p_sample_type samp;

    memset(stat, 0, sizeof(stat));

    for (i = 0; i < samples->num_samples; i++)
    {
        samples->GetSample(i, &samp);

        stat[samp->syms[0]][samp->nvs[0] >> 4][0] ++;
        for (j = 1; j < MAIN_NUMSYMINSAMP && samp->syms[j] != 0; j++)
        {
            int n = (j % 2) ? (samp->nvs[j / 2] & 0x0F) : (samp->nvs[j / 2] >> 4);
            stat[samp->syms[j]][n][1] ++;
        }
    }

    ofstream log_stream(log_name, (ios::out));

    if (log_stream.good())
    {
        for (i = 32; i < 256; i++)
        {
            for (j = 0, k = 0; j < 16; j++)
            {
                if (stat[i][j][0] == 0 && stat[i][j][1] == 0) continue;

                sprintf(str, "Sym: %c, Var: %x, Corr: %6d, Incorr: %6d\n",
                    (uchar) i, j, stat[i][j][0], stat[i][j][1]);
                log_stream << str;

                k++;
            }

            if (k) log_stream << endl;
        }
    }

    log_stream.close();

    return 0;
}

#if 0
/* **************************************************************************** */
/* *           Extract some samples to new file                               * */
/* **************************************************************************** */
int WriteBackSomeSamples(samples_storage_class * samples, char * dat_name)
{
    int   i, j;
    int   ns;
    uchar what;
    float inps[MLP_NET_NUMINPUTS];
    float desired_outputs[MLP_NET_MAX_NUMOUTPUTS];
    p_sample_type samp;

    ofstream dat_stream(dat_name);

    if (dat_stream.good())
    {
        ns = 0;
        GetInputsAtRandom(0x02, &what, inps, desired_outputs, &ns, samples);

        dat_stream << SNN_DATA_VER << endl << "Extracted data collection from " << dat_name << endl << endl;

        for (i = 0; i < 10000 && i < samples->num_samples; i++)
        {
            if (GetInputsAtRandom(0x01, &what, inps, desired_outputs, &ns, samples)) goto err;
            samples->GetSample(ns, &samp);

            dat_stream << "*";
            for (j = 0; j < 14; j++) dat_stream << " " << ((int) (samp->data[j] - 128));
            dat_stream << " " << (int) samp->data[14] << endl;

            for (j = 0; j < samp->nsym; j++)
            {
                int n = (j % 2) ? samp->nvs[j / 2] & 0x0f : samp->nvs[j / 2] >> 4;
                dat_stream << samp->syms[j] << n << ((j == 0) ? 1 : 0) << "17 ";
            }
            dat_stream << endl << endl;
        }

        dat_stream << SNN_DATA_VER << endl << "Extracted data collection from " << dat_name << " ends here." << endl << endl;

        dat_stream.close();
    }

    return 0;
err:
    return 1;
}
#endif

/* **************************************************************************** */
/* *           Extract some samples to new file                               * */
/* **************************************************************************** */
int WriteBackSomeSamples(samples_storage_class * samples, char * dat_name)
{
    int   i, j, n;
    int   ns;
    uchar what, sym, v;
    uchar syms[MAIN_NUMSYMINSAMP + 2];
    uchar inps[MLP_NET_NUMINPUTS];
    p_sample_type samp;

    ofstream dat_stream(dat_name);

    i = 0;

    if (dat_stream.good())
    {
        ns = 5;
        GetInputsAtRandom(0x00, &what, inps, syms, &ns, samples);

        dat_stream << SNN_DATA_VER << endl << "Extracted data collection from " << dat_name << endl << endl;

        for (i = 0; i < samples->num_samples; i++)
        {
            if (GetInputsAtRandom(0x01, &what, inps, syms, &ns, samples))
                break;
            samples->GetSample(ns, &samp);

            dat_stream << "*";
            for (j = 0; j < 14; j++) 
                dat_stream << " " << ((int) (samp->data[j] - 128));
            dat_stream << " " << (int) samp->data[14] << endl;

            for (j = 0, sym = 0; j < 8; j++) 
                sym |= (uchar) ((inps[j + MAIN_SIZE_SAMPLE] > 0.5) ? (0x01 << j) : 0);
            for (j = 0; j < samp->nsym; j++)
            {
                if (sym == samp->syms[j])
                    break;
            }
            n = samp->nvs[0] >> 4;
            v = (uchar) ((n < 10) ? ('0' + n) : ('a' + (n - 10)));
            dat_stream << samp->syms[0] << v << "117 ";
            if (j > 0)
            {
                n = (j % 2) ? samp->nvs[j / 2] & 0x0f : samp->nvs[j / 2] >> 4;
                v = (uchar) ((n < 10) ? ('0' + n) : ('a' + (n - 10)));
                dat_stream << samp->syms[j] << v << "016 ";
            }
            dat_stream << endl << endl;
        }

        dat_stream << SNN_DATA_VER << endl << "Extracted data collection from " << dat_name << " ends here." << endl << endl;

        dat_stream.close();
    }
    else cout << "Can't write to target file " << dat_name << endl;

    cout << "Extracted " << i << " of " << (samples->num_samples + samples->num_antisamp) << " samples to " << dat_name << ". " << endl;

    return 0;
}

/* **************************************************************************** */
/* *          Create C file from loaded net                                   * */
/* **************************************************************************** */

int DumpNetImage(char * dat_name, p_mlp_data_type mlpd)
{
    int     i, len;
    unsigned long * ptr;
    FILE *  file;

    if ((file = fopen(dat_name, "wt")) != 0)
    {
        fprintf(file, "// **************************************************************************\n");
        fprintf(file, "// *    This File is Generated by WritePad Nueral Net Utility               *\n");
        fprintf(file, "// *    DO NOT EDIT THIS FILE!                                              *\n");
        fprintf(file, "// *    Copyright (c) 2008-2014 PhatWare Corp. All rights reserved.         *\n");
        fprintf(file, "// **************************************************************************\n\n");

        fprintf(file, "\n#include \"ams_mg.h\"  \n");
        //    fprintf(file, "#include \"mlp.h\"  \n\n");

        fprintf(file, "// ****   NET body   ********************************************************\n");

        len = (sizeof(mlp_net_type)) / 4;
        fprintf(file, "_ULONG img_snet_body[%d] =  \n", len + 1);
        fprintf(file, " {  \n");
        for (i = 0, ptr = (unsigned long *) mlpd->net; i < len; i++, ptr++)
        {
            fprintf(file, "0x%08X", (int) (*ptr));
            if (i < len - 1)
                fprintf(file, ", ");
            if (i % 8 == 7)
                fprintf(file, "\n");
        }
        fprintf(file, " }; \n\n");

        fprintf(file, "// **************************************************************************\n");
        fprintf(file, "// *    END OF GENERATED FILE                                               *\n");
        fprintf(file, "// **************************************************************************\n");

        fclose(file);
    }

    return 0;
}

/* **************************************************************************** */
/* *          Create Cpp file from loaded net                                 * */
/* **************************************************************************** */
int CppNetImage(char * dat_name, mlp_net_type * net)
{
    int     i;
    FILE *  file;

    mlp_size_info info;
    GetSnnSizeInfo(langID, &info);


    if ((file = fopen(dat_name, "wt")) != 0)
    {
        fprintf(file, "// **************************************************************************\n");
        fprintf(file, "// *    This File is Generated by WritePad Nueral Net Utility               *\n");
        fprintf(file, "// *    DO NOT EDIT THIS FILE!                                              *\n");
        fprintf(file, "// *    Copyright (c) 2008-2014 PhatWare Corp. All rights reserved.         *\n");
        fprintf(file, "// **************************************************************************\n\n");

        fprintf(file, "\n");
        fprintf(file, "#include \"snn.h\"  \n\n");
        fprintf(file, "#if MLP_PRELOAD_MODE  \n\n");

        //    fprintf(file, "\n#include \"ams_mg.h\"  \n");
        fprintf(file, "#include \"mlp.h\"  \n\n");

        fprintf(file, "#if MLP_CELL_MAXINPUTS != %d || MLP_NET_NUMSIGNALS != %d \n", info.net_cells_0, info.net_signal);
        fprintf(file, " #error Wrong NET configuration!\n");
        fprintf(file, "#endif\n\n");


        fprintf(file, "// ****   NET body   ********************************************************\n\n");

        fprintf(file, "ROM_DATA mlp_net_type img_snet_body = \n");
        fprintf(file, " {\n");
        fprintf(file, "   {\"%s\"},\n", net->id_str);
        fprintf(file, "   %d,\n", (int) net->num_layers);
        fprintf(file, "   %d,\n", (int) net->num_inputs);
        fprintf(file, "   %d,\n", (int) net->num_outputs);
        fprintf(file, "   {%d", (int) net->exp_tabl[0]);
        for (i = 1; i < MLP_EXPTABL_SIZE; i++)
        {
            fprintf(file, ",%d", (int) net->exp_tabl[i]);
            if (i % 20 == 0)
                fprintf(file, "\n    ");
        }
        fprintf(file, "   },\n\n");

        fprintf(file, "   {\n");

        fprintf(file, "// *********** Cells of layer 1 *********************************************\n\n");

        //    fprintf(file, "     {\n");
        for (i = 0; i < info.net_cells_1; i++)
            CppCellImage(file, &net->cells[i], 1);
        //    fprintf(file, "     }\n");
        //    fprintf(file, "   },\n\n");

        fprintf(file, "// *********** Cells of layer 2 *********************************************\n\n");

        //    fprintf(file, "   {\n");
        //    fprintf(file, "     {\n");
        for (i = 0; i < info.net_cells_2; i++)
            CppCellImage(file, &net->cells[i + info.net_cells_1], 1);
        //    fprintf(file, "     }\n");
        //    fprintf(file, "   },\n\n");

        fprintf(file, "// *********** Cells of layer 3 *********************************************\n\n");

        //    fprintf(file, "   {\n");
        //    fprintf(file, "     {\n");
        for (i = 0; i < info.net_cells_3; i++)
            CppCellImage(file, &net->cells[i + info.net_cells_1 + info.net_cells_2], (i < info.net_cells_3 - 1));
        //    fprintf(file, "     }\n");

        fprintf(file, "   }\n\n");

        fprintf(file, " };\n");

        fprintf(file, "#endif /* MLP_PRELOAD_MODE */  \n\n");

        fprintf(file, "// **************************************************************************\n");
        fprintf(file, "// *    END OF GENERATED FILE                                               *\n");
        fprintf(file, "// **************************************************************************\n");

        fclose(file);
    }

    return 0;
}

/* **************************************************************************** */
/* *          Create Cpp file from loaded cell                                * */
/* **************************************************************************** */

int CppCellImage(FILE * file, mlp_cell_type * cell, int last)
{
    int     i;

    fprintf(file, "     {\n");
    fprintf(file, "      %d,\n", (int) cell->inp_ind);
    fprintf(file, "      %d,\n", (int) cell->bias);
    fprintf(file, "      {%d", (int) cell->weights[0]);

    for (i = 1; i < MLP_CELL_MAXINPUTS; i++)
        fprintf(file, ",%d", (int) cell->weights[i]);
    if (last)
        fprintf(file, "}\n     },\n"); else  fprintf(file, "}\n     }\n");

    return 0;
}

#ifndef __DONT_USE_WINDOWS_H

/* ************************************************************************ */
/*   Yield execution to other processes                                     */
/* ************************************************************************ */

void PS_Yield(void)
{
    MSG msg;
    
    while (PeekMessage((LPMSG)&msg, NULL,0,0,PM_NOREMOVE))
    {
        if (msg.message == WM_QUIT) 
            break;
        GetMessage((LPMSG)&msg, NULL,0,0);
        TranslateMessage((LPMSG)&msg);
        DispatchMessage((LPMSG)&msg);
    }
}

#else

void PS_Yield(void)
{
    // nothing
}

#endif // __DONT_USE_WINDOWS_H

