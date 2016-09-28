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
#include <memory.h>
#include <string.h>

#include "pydict.h"
#include "bastypes.h"
#include "WritePadDictUtil.h"


static char wrd[132] = "";
static int  wrdlen = 0;
static int  export_words_count = 0;

static char * develop_vertex( void *pVoc, int rank, int num_in_layer, char *vert, FILE *nlst_file );

int generate_sym( char sym, int attr, FILE *nlst_file )
{
    wrd[wrdlen-1] = sym;
    if ( sym == '\n' )
    {
        int nspace,i;
        wrd[wrdlen-1] = 0;
        fprintf( nlst_file, "%s", wrd );
        nspace = 20 - wrdlen;
        if (nspace<1)
            nspace=1;
        for ( i = 0; i < nspace; i++ )
            fprintf( nlst_file," " );
        fprintf( nlst_file, "%d\n", attr );
        export_words_count++;
        return(LAST_SYM);
    }
    return(MID_SYM);
}


void develop_d_child( void *pVoc,char sym,int attr,FILE *nlst_file,int dch_num )
{
    int     dch_rank, dch_num_in_layer;
    char *  dch_vert;
    
    if (generate_sym( sym, attr, nlst_file ) != LAST_SYM )
    {
        dch_rank = find_vert_rank(pVoc,dch_num,&dch_num_in_layer);
        dch_vert = find_vert( pVoc, dch_rank, dch_num_in_layer );
        develop_vertex( pVoc, dch_rank, dch_num_in_layer, dch_vert, nlst_file );
    }
    return;
}


char *develop_nd_child( void *pVoc,int rank,int num_in_layer,
                       char sym,int attr,FILE *nlst_file,char *ndch_vert,int *pndch_num_in_layer )
{
    if (ndch_vert==NULL)
    {
        *pndch_num_in_layer=find_first_nd_child_num(pVoc,rank,num_in_layer);
        ndch_vert=find_vert(pVoc,rank+1,*pndch_num_in_layer);
    }
    if (generate_sym(sym, attr, nlst_file)!=LAST_SYM)
        ndch_vert=develop_vertex(pVoc,rank+1,*pndch_num_in_layer,ndch_vert,nlst_file);
    else
        ndch_vert=pass_vert(pVoc,ndch_vert);
    (*pndch_num_in_layer)++;
    
    return(ndch_vert);
}

//develops cur vertex and returns pointer to the next vertex in the layer

static char * develop_vertex(void *pVoc, int rank, int num_in_layer, char *vert, FILE *nlst_file)
{
    char chset[MAX_CHSET_LEN];
    int dvset[MAX_DVSET_LEN];
    char *p;
    char *pnd_sym,*pd_sym;
    char *ndch_vert=NULL;
    int ndch_num_in_layer;
    int num_nd_childs,num_d_childs,chsetlen,dvsetlen;
    int ind,id;
    unsigned char status;
    unsigned char attr;
    
    wrdlen++;
    
    status = find_vert_status_and_attr( vert, &attr );
    if ( status == XRWD_WORDEND || status == XRWD_BLOCKEND )
    {
        generate_sym( '\n', attr, nlst_file );
    }
    
    p = decode_vert( pVoc, vert, dvset, &dvsetlen, chset, &chsetlen );
    
    num_nd_childs = chsetlen-dvsetlen;
    num_d_childs = dvsetlen;
    pnd_sym = chset;
    pd_sym = chset+num_nd_childs;
    ind = 0;
    id = 0;
    
    while( ind + id < chsetlen )
    {
        if ( id >= num_d_childs || (ind < num_nd_childs && (unsigned char)pnd_sym[ind]<(unsigned char)pd_sym[id]) )
        {
            ndch_vert = develop_nd_child(pVoc,rank,num_in_layer,pnd_sym[ind],attr,nlst_file,ndch_vert,&ndch_num_in_layer);
            ind++;
        }
        else if ( ind >= num_nd_childs || (id < num_d_childs && (unsigned char)pd_sym[id]<(unsigned char)pnd_sym[ind]) )
        {
            develop_d_child( pVoc,pd_sym[id],attr,nlst_file,dvset[id] );
            id++;
        }
    }
    
    //EXIT:
    wrdlen--;
    return(p);
}


int generate_listing(void *pVoc, char *NLstFileName)
{
    FILE *  nlst_file = NULL;
    char *  vert;
    
    if (pVoc==NULL)
        goto err;
    
    if ( (nlst_file = fopen( NLstFileName,"wt" )) == NULL )
        goto err;;
    
    vert = find_vert(pVoc,0,0);
    wrdlen = 0;
    export_words_count = 0;
 
    develop_vertex(pVoc, 0, 0, vert, nlst_file);
    
    printf( "%d words exported.\n", export_words_count );
    
    if (nlst_file!=NULL)
        fclose(nlst_file);
    return(PYDICT_NOERR);
err:
    if (nlst_file!=NULL)
        fclose(nlst_file);
    return(PYDICT_ERR);
}

int save_dictionary_to_file( char * pName, void * pDict )
{
    int res = PYDictSaveDictToFile( (unsigned char *)pName, pDict );
    return res;
}

int load_dictionary_from_file( char * pName, void ** ppd )
{
    int res = PYDictLoadDictFromFile( (unsigned char *)pName, ppd );;
    return res;
}

bool create_voc_and_generate_listing( char *DataFileName,char *NLstFileName )
{
    bool retval=false;
    FILE *nlst_file=NULL, *data_file = NULL;
    char *vert;
    void *pVoc;
    int attr,i;
    
    if ( PYDictCreateDict( (void **)&pVoc) == PYDICT_ERR )
        goto EXIT;
    
    if ( (data_file=fopen( DataFileName,"rt")) == NULL )
        goto EXIT;
    
    while (fgets(wrd,128,data_file)!=NULL)
    {
        i=0;
        while (wrd[i]!=' ' && wrd[i]!='\n') i++;
        if (wrd[i]==' ')
        {
            sscanf( &wrd[i+1],"%d",&attr );
            if ( attr < 0 || attr > 3 )
                printf( "Stat Error\n" );
        }
        else attr=0;
        wrd[i]=0;
        if ( strlen(wrd) < PYDICT_MAX_WORDLEN )
        {
            if ( PYDictAddWord((unsigned char *)wrd, (unsigned char)attr, (void **)&pVoc) == PYDICT_ERR )
                goto EXIT;
        }
    }
    if ( (nlst_file = fopen(NLstFileName,"wt")) == NULL )
        goto EXIT;
    
    vert = find_vert(pVoc,0,0);
    wrdlen = 0;
    export_words_count = 0;

    develop_vertex(pVoc, 0, 0, vert, nlst_file);
    
    printf( "%d words exported.\n", export_words_count );
    retval = true;
    
EXIT:
    if (nlst_file!=NULL)
        fclose(nlst_file);
    if (data_file!=NULL)
        fclose(data_file);
    PYDictFreeDict((void **)&pVoc);
    
    return(retval);
}

