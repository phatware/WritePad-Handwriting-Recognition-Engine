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
#include <math.h>
#include "hwr_sys.h"
#include "bastypes.h"

#include "PYDict.h"
#include "WritePadDictUtil.h"

//#define TREE_TYPE PLAIN_TREE

//#define NumberOfWords 70000

#define INCLUDE_LAST 1


#define MAX_NUM_LEVELS   PYDICT_MAX_WORDLEN //40
#define MAX_NUM_SETS     100000  //64000  //These number determines only amount of memory allocated
//#define MAX_NUM_IN_LEVEL 40000

static int cur_place;

static char word[PYDICT_MAX_WORDLEN];
static int  wordlen;

static int up_rank;
static int num_levels;
static int nvertex_in_level[MAX_NUM_LEVELS];
static int max_nv_in_level;

static int nvertex_before_level[MAX_NUM_LEVELS];

static int NumWords=0;
static int WordLen=0;
static int MaxWordLen=0;

static int UncodedDvsetsSize=0;
static int UncodedDvsetsNVert=0;
static int UncodedDvsetsNPntr=0;
static int EndDvsetsSize=0;
static int NumEndVert=0;
static int EndDvsetsNPntr=0;

typedef struct
{
    int bytes_less;
    int nd_len;
    int d_len;
    int num_vert;
    char chset[128];
}  CHARSET;

typedef struct
{
    int bytes_less;
    int len;
    int num_vert;
    void * dvset[64];
    //   bool nonzero_stat;
}  DVSET;

class EDGE
{
public:
    void * node;
    unsigned char symbol;
    bool  diag;
    float freq;
    class EDGE * next;
    class EDGE * prev;
    
    EDGE(void)
    {
        node=NULL;
        symbol=0;
        diag=false;
        freq=0.f;
        next=NULL;
        prev=NULL;
    }
    
    int insert_in_place(EDGE *prv)
    {
        EDGE *nxt;
        if (prv==NULL)
            return(1);
        nxt=prv->next;
        prv->next=this;
        this->prev=prv;
        this->next=nxt;
        if (nxt!=NULL)
            nxt->prev=this;
        return(0);
    }
    
    void insert_in_list(EDGE **pbeg_list)
    {
        EDGE *tmp;
        if (*pbeg_list==NULL)
            *pbeg_list=new(EDGE);
        tmp=*pbeg_list;
        while (tmp->next!=NULL && tmp->next->symbol<this->symbol)
            tmp=tmp->next;
        insert_in_place(tmp);
        return;
    }
};


class VERTEX
{
    
public:
    EDGE * children;
    EDGE *parents;
    int rank;
    int num_in_level;
    int charset_num;
    int dvset_num;
    bool one_child_nd;
    bool mid_word;
    unsigned char end_word;
    unsigned char attr;
    float freq;
    
    VERTEX(void)
    {
        children=NULL;
        parents=NULL;
        charset_num=0;
        dvset_num=0;
        one_child_nd=false;
        mid_word=false;
        end_word=0;
        rank=0;
        attr=0;
        freq=0.f;
    };
    
    ~VERTEX(void)
    {
        EDGE * tmp;
        
        if (children!=NULL)
        {
            tmp=children;
            while (tmp->next!=NULL)
            {
                if (tmp->next->diag==false)
                    delete (VERTEX*)tmp->next->node;
                tmp=tmp->next;
                delete(tmp->prev);
            }
            delete(tmp);
        }
        if (parents!=NULL)
        {
            tmp=parents;
            while (tmp->next!=NULL)
            {
                tmp=tmp->next;
                delete(tmp->prev);
            }
            delete(tmp);
        }
    };
    
    VERTEX *insert_vertex_or_edge(char sym,VERTEX * vert)
    {
        VERTEX *new_vert;
        EDGE *new_dn_edge,*new_up_edge;
        
        if (vert==NULL) new_vert=new VERTEX;
        else new_vert=vert;
        
        new_dn_edge=new(EDGE);
        new_dn_edge->node=(void *)new_vert;
        new_dn_edge->symbol=(unsigned char)sym;
        
        new_dn_edge->insert_in_list(&children);
        
        new_up_edge=new(EDGE);
        new_up_edge->node=(void *)this;
        new_up_edge->symbol=(unsigned char)sym;
        
        new_up_edge->insert_in_list(&new_vert->parents);
        
        return(new_vert);
    }
    
    int count_children(void)
    {
        int num_ch=0;
        EDGE *tmp;
        
        if (children==NULL)
            return(0);
        tmp=children;
        while(tmp->next!=NULL)
        {
            tmp=tmp->next;
            if (INCLUDE_LAST || tmp->symbol!='\n')
                num_ch++;
        }
        
        return(num_ch);
    }
    
    int count_nd_children(void)
    {
        int num_nd_ch=0;
        EDGE *tmp;
        
        if (children==NULL)
            return(0);
        tmp=children;
        while(tmp->next!=NULL)
        {
            tmp=tmp->next;
            if (tmp->diag==false && tmp->symbol!='\n')
                num_nd_ch++;
        }
        
        return(num_nd_ch);
    }
    
};

int find_or_insert_word(VERTEX *start_vert, char *str, int attr, int *pnedges)
{
    VERTEX *next_start_vert = nullptr;
    EDGE *tmp;
    char sym=*str;
    int NOTexist=1;
    
    if(sym==0)
    {
        NumWords++;
        if (WordLen>MaxWordLen) MaxWordLen=WordLen;
        WordLen=0;
        return(0);
    }
    WordLen++;
    if(start_vert->children!=NULL)
    {
        tmp=start_vert->children;
        NOTexist=1;
        while((tmp->next)!=NULL)
        {
            if((tmp->next->symbol)==(unsigned char)sym)
            {
                next_start_vert=(VERTEX*)(tmp->next->node);
                NOTexist=0; break;
            }
            tmp=tmp->next;
        }
    }
    if(NOTexist)
    {
        if (INCLUDE_LAST || sym!='\n') { (*pnedges)++; }
        next_start_vert=start_vert->insert_vertex_or_edge(sym,NULL);
    }
    if (sym=='\n')
        next_start_vert->attr=(unsigned char)attr;
    str++;
    find_or_insert_word(next_start_vert,str,attr,pnedges);
    return(0);
}


int find_wrd_and_put_freq(VERTEX *start_vert,char *str,float freq)
{
    VERTEX *next_start_vert;
    EDGE *tmp;
    char sym=*str;
    
    if (sym==0)
    {
        if (start_vert->children==NULL)
        {
            start_vert->freq+=freq;
        }
        return(0);
    }
    if (sym=='\n' && start_vert->children!=NULL)
    {
        tmp=start_vert->children;
        while((tmp->next)!=NULL)
        {
            tmp=tmp->next;
            if(tmp->symbol=='\n')
                tmp->freq+=freq;
        }
    }
    
    if (start_vert->children!=NULL)
    {
        tmp=start_vert->children;
        while((tmp->next)!=NULL)
        {
            tmp=tmp->next;
            if((tmp->symbol)==(unsigned char)sym)
            {
                next_start_vert=(VERTEX *)tmp->node;
                str++;
                find_wrd_and_put_freq(next_start_vert,str,freq);
            }
        }
    }
    return(0);
}

void traverse_and_find_zero_freq(VERTEX *root,int *pnzero_freq)
{
    EDGE *tmp;
    
    if(root->children==NULL)
    {
        if (root->freq==0.f)
            (*pnzero_freq)++;
    }
    else
    {
        tmp=root->children;
        while((tmp->next)!=NULL)
        {
            tmp=tmp->next;
            word[wordlen]=tmp->symbol;
            wordlen++;
            traverse_and_find_zero_freq((VERTEX*)tmp->node,pnzero_freq);
        }
    }
    wordlen--;
    return;
}

int traverse_and_find_rank(VERTEX *root,int *nvertex_in_rank)
{
    EDGE *tmp;
    int max,tempint;
    
    if(root->children==NULL)
        root->rank=0;
    else
    {
        tmp=root->children;
        max=0;
        while((tmp->next)!=NULL)
        {
            tempint=traverse_and_find_rank((VERTEX*)tmp->next->node,nvertex_in_rank);
            if(tempint>max)
                max=tempint;
            tmp=tmp->next;
        }
        root->rank=max+1;
    }
    nvertex_in_rank[root->rank]++;
    return(root->rank);
}

void find_and_place_given_level(VERTEX *root,int nlevel,VERTEX **level)
{
    EDGE *tmp;
    
    if (root->rank==nlevel)
    {
        level[cur_place]=root;
        cur_place++;
    }
    if(root->children==NULL)
        return;
    tmp=root->children;
    while((tmp->next)!=NULL)
    {
        tmp=tmp->next;
        if ( tmp->diag==false && (INCLUDE_LAST || tmp->symbol!='\n') )
            find_and_place_given_level( (VERTEX*)tmp->node,nlevel,level );
    }
    return;
}


int compare(const void *ppv1,const void *ppv2)
{
    EDGE *tmp2,*tmp1;
    VERTEX *pv1=*((VERTEX **)ppv1);
    VERTEX *pv2=*((VERTEX **)ppv2);
    
    tmp1=pv1->children;
    tmp2=pv2->children;
    
    if((tmp1==NULL)&&(tmp2==NULL))
    {
        if (pv1->attr>pv2->attr)
            return(1);
        if (pv1->attr<pv2->attr)
            return(-1);
        return(0);
    }
    if((tmp1!=NULL)&&(tmp2==NULL))
        return(1);
    if((tmp1==NULL)&&(tmp2!=NULL))
        return(-1);
    
    while(tmp1->next!=NULL && tmp2->next!=NULL)
    {
        tmp1=tmp1->next; tmp2=tmp2->next;
        if(tmp1->symbol>tmp2->symbol)
            return(1);
        if(tmp1->symbol<tmp2->symbol)
            return(-1);
        if((unsigned long)tmp1->node>(unsigned long)tmp2->node)
            return(1);
        if((unsigned long)tmp1->node<(unsigned long)tmp2->node)
            return(-1);
    }
    
    if((tmp1->next!=NULL)&&(tmp2->next==NULL))
        return(1);
    if((tmp1->next==NULL)&&(tmp2->next!=NULL))
        return(-1);
    return(0);
    
}


int merge(VERTEX *v1,VERTEX *v2)
{
    EDGE *prnt, *tch, *tch1, *tch2;
    
    prnt=v2->parents;
    if(prnt==NULL) return (1);
    prnt=prnt->next;                         //Redirect edge from parent
    if (prnt==NULL) return(2);
    if (prnt->next!=NULL) return(3);
    
    tch=((VERTEX *)prnt->node)->children;
    if(tch==NULL) return(4);
    while(tch!=NULL && tch->node!=v2)
        tch=tch->next;
    if (tch==NULL) return(5);
    tch->node=v1;
    tch->diag=true;
    
    tch1=v1->children;
    tch2=v2->children;
    while (tch1!=NULL)
    {
        if (tch2==NULL) return(6);
        if (tch2->diag==false) tch1->diag=false; //Don't loose non-diag edge to child!!
        tch2->diag=true;         //for deleting
        tch1->freq=tch1->freq+tch2->freq;
        tch1=tch1->next;
        tch2=tch2->next;
    }
    
    delete v2;
    
    return(0);
}


void generate_listing(VERTEX *root,FILE *ndata)
{
    EDGE *child=root->children;
    int nspace,i;
    if (child==NULL)
    {
        word[wordlen-1]=0;       //Remove \n
        fprintf(ndata,"%s",word);
        nspace=14-wordlen; if (nspace<1) nspace=1;
        for (i=0; i<nspace; i++)
            fprintf(ndata," ");
        fprintf(ndata,"%d\n",root->attr);
        wordlen--;
        return;
    }
    
    while(child->next!=NULL)
    {
        child=child->next;
        word[wordlen]=child->symbol;
        wordlen++;
        generate_listing((VERTEX *)child->node,ndata);
    }
    wordlen--;
    return;
}



void traverse_and_calc_vert(VERTEX *root,int *pnvert)
{
    EDGE *tmp;
    
    (*pnvert)++;
    if(root->children!=NULL)
    {
        tmp=root->children;
        while((tmp->next)!=NULL)
        {
            tmp=tmp->next;
            if (tmp->diag==false && (INCLUDE_LAST || tmp->symbol!='\n'))
                traverse_and_calc_vert((VERTEX *)tmp->node,pnvert);
        }
    }
    return;
}



void traverse_and_calc_diag_edges(VERTEX *root,EDGE **d_edges,int *pnd_edges)
{
    EDGE *tmp;
    
    if(root->children!=NULL)
    {
        tmp=root->children;
        while ( (tmp->next) != NULL )
        {
            tmp = tmp->next;
            if (tmp->diag==true && (INCLUDE_LAST || tmp->symbol!='\n'))
            {
                if (d_edges!=NULL) d_edges[*pnd_edges]=tmp;
                (*pnd_edges)++;
            }
            else if (tmp->diag==false && (INCLUDE_LAST || tmp->symbol!='\n'))
            {
                traverse_and_calc_diag_edges((VERTEX *)tmp->node,d_edges,pnd_edges);
            }
        }
    }
    return;
}


/*
 void traverse_and_calc_mltp_vert(VERTEX *root,int *pn_mltp_vert)
 {
 EDGE *tmp;
 
 if (root->n_coparents>0)  (*pn_mltp_vert)++;
 if(root->children!=NULL)
 {
 tmp=root->children;
 while((tmp->next)!=NULL)
 {
 tmp=tmp->next;
 if (tmp->diag==false && (INCLUDE_LAST || tmp->symbol!='\n'))
 traverse_and_calc_mltp_vert((VERTEX *)tmp->node,pn_mltp_vert);
 }
 }
 return;
 }
 */

void traverse_and_calc_vert_status(VERTEX *root)
{
    EDGE *tmp;
    
    if(root->children!=NULL)
    {
        tmp=root->children;
        while( (tmp->next) != NULL )
        {
            tmp=tmp->next;
            if (tmp->symbol=='\n')
                root->end_word=(unsigned char)(1+((VERTEX *)(tmp->node))->attr);
            else
                root->mid_word=true;
            if ( tmp->diag == false && (INCLUDE_LAST || tmp->symbol!='\n') )
            {
                traverse_and_calc_vert_status((VERTEX *)tmp->node);
            }
        }
    }
    return;
}



void traverse_and_calc_one_child_vert( VERTEX *root, int *pn_one_child_vert )
{
    EDGE *  tmp;
    int     nch;
    
    if(root->children!=NULL)
    {
        tmp = root->children;
        while((tmp->next)!=NULL)
        {
            tmp=tmp->next;
            if (tmp->diag==false && (INCLUDE_LAST || tmp->symbol!='\n'))
            {
                traverse_and_calc_one_child_vert( (VERTEX *)tmp->node, pn_one_child_vert );
            }
        }
        nch = root->count_children();
        if ( nch == 1)
        {
            (*pn_one_child_vert)++;
        }
        if ( nch == 1 && ((root->children->next->diag == false && (unsigned char)root->children->next->symbol<128)
             || root->children->next->symbol=='\n') )
        {
            root->one_child_nd=true;
        }
    }
    return;
}


int compare_edge(const void *ppe1,const void *ppe2)
{
    EDGE *pe1, *pe2;
    
    pe1 = *((EDGE **)ppe1);
    pe2 = *((EDGE **)ppe2);
    
    if (pe1->symbol>pe2->symbol)
        return(1);
    if (pe1->symbol<pe2->symbol)
        return(-1);
    if ((unsigned long)pe1->node>(unsigned long)pe2->node)
        return(1);
    if ((unsigned long)pe1->node<(unsigned long)pe2->node)
        return(-1);
    return(0);
}


void traverse_and_find_up_rank(VERTEX *root)
{
    EDGE *tmp;
    
    if(root->children!=NULL)
    {
        root->rank=up_rank;
        if (up_rank+1>num_levels)
            num_levels=up_rank+1;
        root->num_in_level=nvertex_in_level[up_rank];
        nvertex_in_level[up_rank]++;
        if (nvertex_in_level[up_rank]>max_nv_in_level)
            max_nv_in_level=nvertex_in_level[up_rank];
        
        up_rank++;
        tmp=root->children;
        while((tmp->next)!=NULL)
        {
            tmp=tmp->next;
            if (tmp->diag==false && (INCLUDE_LAST || tmp->symbol!='\n'))
                traverse_and_find_up_rank((VERTEX*)tmp->node);
        }
        up_rank--;
    }
    else root->rank=10000;
    return;
}



int traverse_and_place_charsets(VERTEX *root, CHARSET *charsets, int flag)
{
    EDGE *tmp;
    int i=0;
    unsigned char take_diag=flag&0x2;
    unsigned char take_nondiag=flag&0x1;
    unsigned char WithoutLast=flag&0x4;
    
    if (root->children!=NULL)
    {
        /*
         if (WithoutLast)
         {
         tmp=root->children;
         while((tmp->next)!=NULL)
         {
         tmp=tmp->next;
         if (tmp->symbol=='\n')  root->end_word=true;
         else                    root->mid_word=true;
         }
         */
        
        if (root->one_child_nd==false)
        {
            if (cur_place>=MAX_NUM_SETS) return(0);
            
            tmp=root->children;
            while((tmp->next)!=NULL)
            {
                tmp=tmp->next;
                if ((!WithoutLast || tmp->symbol!='\n') && tmp->diag==false && take_nondiag)
                {
                    charsets[cur_place].chset[i] = (tmp->symbol!='\n') ? tmp->symbol : (char)1;
                    i++;
                }
            }
            charsets[cur_place].nd_len=i;
            tmp=root->children;
            while((tmp->next)!=NULL)
            {
                tmp=tmp->next;
                if ((!WithoutLast || tmp->symbol!='\n') && tmp->diag==true && take_diag)
                {
                    charsets[cur_place].chset[i] = (tmp->symbol!='\n') ? tmp->symbol : (char)1;
                    i++;
                }
            }
            charsets[cur_place].d_len=i-charsets[cur_place].nd_len;
            
            cur_place++;
        }
        
        
        tmp=root->children;
        while((tmp->next)!=NULL)
        {
            tmp=tmp->next;
            if (tmp->diag==false && (INCLUDE_LAST || tmp->symbol!='\n'))
            {
                if (!traverse_and_place_charsets((VERTEX *)tmp->node,charsets,flag))
                    return(0);
            }
        }
        
    }
    return(1);
}

int compare_charsets(const void *pcs1,const void *pcs2)
{
    CHARSET *cs1,*cs2;
    int i;
    
    cs1=(CHARSET *)pcs1;
    cs2=(CHARSET *)pcs2;
    
    if (cs1->nd_len<cs2->nd_len)
        return(-1);
    if (cs1->nd_len>cs2->nd_len)
        return(1);
    for (i=0; i<cs1->nd_len; i++)
    {
        if ((unsigned char)cs1->chset[i]<(unsigned char)cs2->chset[i])
            return(-1);
        if ((unsigned char)cs1->chset[i]>(unsigned char)cs2->chset[i])
            return(1);
    }
    
    if (cs1->d_len<cs2->d_len)
        return(-1);
    if (cs1->d_len>cs2->d_len)
        return(1);
    for (i=cs1->nd_len; i<cs1->nd_len+cs1->d_len; i++)
    {
        if ((unsigned char)cs1->chset[i]<(unsigned char)cs2->chset[i])
            return(-1);
        if ((unsigned char)cs1->chset[i]>(unsigned char)cs2->chset[i])
            return(1);
    }
    
    return(0);
}

int compare_int(const void *pi1,const void *pi2)
{
    int i1=*((int *)pi1);
    int i2=*((int *)pi2);
    
    if (i1<i2)
        return(1);
    if (i1>i2)
        return(-1);
    return(0);
}

int compare_chsetlen(const void *pcs1,const void *pcs2)
{
    int ndl1=((CHARSET *)pcs1)->nd_len;
    int ndl2=((CHARSET *)pcs2)->nd_len;
    int dl1=((CHARSET *)pcs1)->d_len;
    int dl2=((CHARSET *)pcs2)->d_len;
    
    if (ndl1<ndl2)
        return(1);
    if (ndl1>ndl2)
        return(-1);
    if (dl1<dl2)
        return(1);
    if (dl1>dl2)
        return(-1);
    return(0);
}

int compare_chsetslen(const void *pcs1,const void *pcs2)
{
    CHARSET *cs1=(CHARSET *)pcs1;
    CHARSET *cs2=(CHARSET *)pcs2;
    int l1=cs1->nd_len+cs1->d_len;
    int l2=cs2->nd_len+cs2->d_len;
    
    if (l1<l2)
        return(-1);
    if (l1>l2)
        return(1);
    /*
     if (l1==1 && l2==1)
     {
     if (cs1->chset[0]>0 && cs2->chset[0]<0) return(-1);
     if (cs2->chset[0]>0 && cs1->chset[0]<0) return(1);
     }
     */
    return(0);
}

int compare_chsetnv(const void *pcs1,const void *pcs2)
{
    CHARSET *cs1=(CHARSET *)pcs1;
    CHARSET *cs2=(CHARSET *)pcs2;
    int nv1=cs1->num_vert;
    int nv2=cs2->num_vert;
    
    if (nv1<nv2)
        return(1);
    if (nv1>nv2)
        return(-1);
    
    return(0);
}

int compare1_chsetnv(const void *pcs1,const void *pcs2)
{
    CHARSET *cs1=(CHARSET *)pcs1;
    CHARSET *cs2=(CHARSET *)pcs2;
    int nv1=cs1->num_vert;
    int nv2=cs2->num_vert;
    
    if (nv1<nv2)
        return(-1);
    if (nv1>nv2)
        return(1);

    return(0);
}

int compare_chsetbl(const void *pcs1,const void *pcs2)
{
    CHARSET *cs1=(CHARSET *)pcs1;
    CHARSET *cs2=(CHARSET *)pcs2;
    int bl1=cs1->bytes_less;
    int bl2=cs2->bytes_less;
    
    if (bl1<bl2)
        return(-1);
    if (bl1>bl2)
        return(1);
    
    return(0);
}



int count_num_charests(VERTEX *root, CHARSET *charsets, int flag)
{
    CHARSET *pnew_charset;
    int  n_charsets,j;
    
    cur_place=0;
    if (!traverse_and_place_charsets(root,charsets,flag))
        return(-1);
    qsort((void*)charsets,cur_place,sizeof(CHARSET),compare_charsets);
    
    n_charsets=1;
    pnew_charset=&charsets[n_charsets-1];
    pnew_charset->num_vert=1;
    for (j=1; j<cur_place; j++)
    {
        if (compare_charsets((void *)&charsets[j],(void *)pnew_charset)!=0)
        {
            n_charsets++;
            pnew_charset=&charsets[n_charsets-1];
            if (j>n_charsets-1)
                memcpy((void *)pnew_charset,(void *)&charsets[j],sizeof(CHARSET));
            pnew_charset->num_vert=1;
            if (pnew_charset->chset[0]==0)
                break;
        }
        else
        {
            pnew_charset->num_vert++;
        }
    }
    
    //     qsort((void *)charsets,n_charsets,sizeof(CHARSET),compare_chsetlen);
    
    return(n_charsets);
}


void extract_dvset(VERTEX *root,DVSET *cur_dvset,unsigned char WithoutLast)
{
    EDGE *tmp;
    int i=0;
    
    tmp=root->children;
    while((tmp->next)!=NULL)
    {
        tmp=tmp->next;
        if (tmp->diag==true && (!WithoutLast || tmp->symbol!='\n'))
        {
            cur_dvset->dvset[i] = tmp->node;
            i++;
        }
    }
    cur_dvset->len=i;
    return;
}

int traverse_and_place_dvsets(VERTEX *root, DVSET *dvsets, int flag)
{
    EDGE *tmp;
    unsigned char WithoutLast=flag&0x04;    //now always true!!!
    unsigned char EndWordDvsets=flag&0x08;
    
    if (root->children!=NULL)
    {
        if (root->one_child_nd==false &&
            ((!EndWordDvsets && root->end_word==0) || (EndWordDvsets && root->end_word>1)) )
        {
            if (cur_place>=MAX_NUM_SETS)
                return(0);
            
            extract_dvset(root,&dvsets[cur_place],WithoutLast);
            cur_place++;
        }
        
        tmp=root->children;
        while((tmp->next)!=NULL)
        {
            tmp=tmp->next;
            if (tmp->diag==false && (INCLUDE_LAST || tmp->symbol!='\n'))
            {
                if (!traverse_and_place_dvsets((VERTEX *)tmp->node,dvsets,flag))
                    return(0);
            }
        }
    }
    return(1);
}

int compare_dvsets(const void *p1,const void *p2)
{
    void **dvs1,**dvs2;
    int i;
    
    dvs1=((DVSET *)p1)->dvset;
    dvs2=((DVSET *)p2)->dvset;
    
    for (i=0; i<20; i++)
    {
        if ((unsigned long)dvs1[i]<(unsigned long)dvs2[i])
            return(-1);
        if ((unsigned long)dvs1[i]>(unsigned long)dvs2[i])
            return(1);
        if (dvs1[i]==NULL)
            break;
    }
    return(0);
}

int compare_dvsetlen(const void *p1,const void *p2)
{
    int len1=((DVSET *)p1)->len;
    int len2=((DVSET *)p2)->len;
    
    if (len1<len2)
        return(-1);
    if (len1>len2)
        return(1);
    
    return(0);
}

int compare_dvsetnv(const void *p1,const void *p2)
{
    DVSET *dvs1=(DVSET *)p1;
    DVSET *dvs2=(DVSET *)p2;
    
    int nv1=dvs1->num_vert;
    int nv2=dvs2->num_vert;
    
    
    if (nv1<nv2)
        return(1);
    if (nv1>nv2)
        return(-1);
    
    return(0);
}

int compare1_dvsetnv(const void *p1,const void *p2)
{
    int nv1=((DVSET *)p1)->num_vert;
    int nv2=((DVSET *)p2)->num_vert;
    
    if (nv1<nv2)
        return(-1);
    if (nv1>nv2)
        return(1);
    
    return(0);
}

int compare_dvsetbl(const void *p1,const void *p2)
{
    int bl1=((DVSET *)p1)->bytes_less;
    int bl2=((DVSET *)p2)->bytes_less;
    
    if (bl1<bl2)
        return(-1);
    if (bl1>bl2)
        return(1);
    
    return(0);
}


int count_num_dvsets(VERTEX *root, DVSET *dvsets,int flag)
{
    DVSET *pnew_dvset;
    int  n_dvsets,j;
    
    cur_place=0;
    if (!traverse_and_place_dvsets(root,dvsets,flag)) return(-1);
    qsort((void*)dvsets,cur_place,sizeof(DVSET),compare_dvsets);
    
    n_dvsets=1;
    pnew_dvset=&dvsets[0];
    pnew_dvset->num_vert=1;
    for (j=1; j<cur_place; j++)
    {
        if (compare_dvsets((void *)&dvsets[j],(void *)pnew_dvset)!=0)
        {
            n_dvsets++;
            pnew_dvset=&dvsets[n_dvsets-1];
            if (j>n_dvsets-1)
                memcpy((void *)pnew_dvset,(void *)&dvsets[j],sizeof(DVSET));
            pnew_dvset->num_vert=1;
            if (pnew_dvset->dvset[0]==NULL)
                break;
        }
        else
        {
            pnew_dvset->num_vert++;
        }
    }
    
    //     qsort((void *)dvsets,n_dvsets,sizeof(DVSET),compare_dvsetlen);
    
    return(n_dvsets);
}


void calc_childs_freq(VERTEX *root,int nlevel)
{
    EDGE *tmp;
    
    if (root->children==NULL) return;
    
    if (root->rank==nlevel)
    {
        tmp=root->children;
        while((tmp->next)!=NULL)
        {
            tmp=tmp->next;
            tmp->freq=((VERTEX *)tmp->node)->freq;
        }
    }
    tmp=root->children;
    while((tmp->next)!=NULL)
    {
        tmp=tmp->next;
        if (tmp->diag==false && (INCLUDE_LAST || tmp->symbol!='\n'))
            calc_childs_freq((VERTEX*)tmp->node,nlevel);
    }
    return;
    
}

void calc_vert_freq(VERTEX *root,int nlevel)
{
    EDGE *tmp;
    
    if (root->children==NULL) return;
    
    if (root->rank==nlevel)
    {
        tmp=root->children;
        while((tmp->next)!=NULL)
        {
            tmp=tmp->next;
            root->freq+=tmp->freq;
        }
    }
    tmp=root->children;
    while((tmp->next)!=NULL)
    {
        tmp=tmp->next;
        if (tmp->diag==false && (INCLUDE_LAST || tmp->symbol!='\n'))
            calc_vert_freq((VERTEX*)tmp->node,nlevel);
    }
    return;
    
}

int nsort(VERTEX **varray, int N) //sort for end-of-word vertexes
//sort by attr:0,1,2,3
{
    int n[4],i,k;
    VERTEX **pw[4], **warray;
    
    memset(n,0,sizeof(int)*4);
    for (i=0; i<N; i++)
    {
        k=varray[i]->attr; if (k<0 || k>3) goto err;
        n[k]++;
    }
    
    warray=(VERTEX **)malloc(sizeof(VERTEX *)*N);
    if (warray==NULL) goto err;
    
    pw[0]=warray;
    for (k=1; k<4; k++)
        pw[k]=pw[k-1]+n[k-1];
    
    for (i=0; i<N; i++)
    {
        k=varray[i]->attr;
        *(pw[k])=varray[i];
        (pw[k])++;
    }
    memcpy(varray,warray,sizeof(VERTEX *)*N);
    
    if ( warray!=NULL )
        free( warray );
    return(1);
err:
    if (warray!=NULL)
        free(warray);
    return(0);
}


int CreatePYDict( char *list_name, char *freq_name, unsigned char TreeType,void **ppd )
{
    int convert_voc(unsigned char TreeType,VERTEX *root,void **ppd);//,char *voc_name);
    
    VERTEX *root=new VERTEX;
    int slen;
    int i,n_one_child_vert=0;
    int nedges=0, n_vert=0;
    //#if TREE_TYPE==MERGED_TREE
    VERTEX **level=NULL;
    EDGE **d_edges=NULL;
    VERTEX *new_vert;
    int nvertex_in_rank[PYDICT_MAX_WORDLEN+1];
    int j,maxrank,maxnv;
    int edges_less, n_diag_edges=0;
    //#endif
    char  c[128];
    FILE * data;//,* ndata;
    //FREQ_ORDER
    char c1[32];
    FILE * fdata;
    int freq,nf;
    int nzero_freq,len,fsc;
    
    //  char VocFileName[64];
    //  char NLstFileName[64];
    //  char DataFileName[64];
    int attr;
    
    //Globals
    NumWords=0;
    WordLen=0;
    MaxWordLen=0;
    
    UncodedDvsetsSize=0;
    UncodedDvsetsNVert=0;
    UncodedDvsetsNPntr=0;
    EndDvsetsSize=0;
    NumEndVert=0;
    EndDvsetsNPntr=0;
    
    
    if ((data = fopen(list_name,"rt"))==NULL)// || (tmpf = fopen("tmp.txt","wt"))==NULL)
    {
        goto err;
    }
    while (fgets(c,128,data)!=NULL)
    {
        slen = (int)strlen(c);
        i=0;
        while (i<slen && c[i]!=' ' && c[i]!='\n') i++;
        if (i<slen-1 && c[i]==' ')
        {
            c[i]='\n';
            sscanf(&c[i+1],"%d",&attr);
            if (attr<0 || attr>3)
            {
                printf("Stat Error\n");
            }
            c[i+1]=0;
            slen = (int)strlen(c);
        }
        else attr=0;
        
        if (slen <= PYDICT_MAX_WORDLEN && slen>0)
        {
            if (c[slen-1]!='\n')
            {
                c[slen]='\n';
                c[slen+1]=0;
            }
            find_or_insert_word(root,c,attr,&nedges);
        }
        else
            continue;
    }
    fclose(data);
    
    printf("Num Words: %d, Max Word Len: %d\n",NumWords,MaxWordLen);
    
    
    if (TreeType&FREQ_FLAG)
    {
        if (freq_name==NULL || (fdata = fopen(freq_name,"rt"))==NULL)
        {
            printf("NO FREQ DATA FILE");
            goto err;
        }
        fgets(c,127,fdata);
        while ((fsc=fscanf(fdata,"%d %s %s %d",&freq,c,c1,&nf))!=EOF)
        {
            if (fsc!=4)
            {
                printf("ERROR IN FREQ DATA FILE; fsc: %d\n",fsc);
                break;
            }
            len = (int)strlen(c);
            c[len]='\n';
            c[len+1]=0;
            find_wrd_and_put_freq(root,c,(float)freq);
        }
        fclose(fdata);
        
        
        nzero_freq=0;
        wordlen=0;
        traverse_and_find_zero_freq(root,&nzero_freq);
        
        printf("Num words with no freq: %d\n",nzero_freq);
        
    } //FREQ_FLAG
    
    if ((TreeType&MERGE_FLAG))
    {
        memset(nvertex_in_rank,0,sizeof(int)*(PYDICT_MAX_WORDLEN+1));
        
        maxrank=traverse_and_find_rank(root,nvertex_in_rank);
        
        for (i=0,maxnv=0; i<maxrank; i++)
        {
            if (nvertex_in_rank[i]>maxnv)
                maxnv=nvertex_in_rank[i];
        }
        level = (VERTEX **)malloc((maxnv + 1)*sizeof(VERTEX*));
        if ( level == NULL )
            goto err;
        for (i=0,edges_less=0; i<=maxrank; i++)
        {
            if (i>1) calc_childs_freq(root,i);
            cur_place=0;
            find_and_place_given_level(root,i,level);
            if (i==0)
            {
                if (!nsort(level,nvertex_in_rank[i])) goto err;
            }
            else //i>0
                qsort((void *)level,nvertex_in_rank[i],sizeof(VERTEX*),compare);
            for (j=1,new_vert=level[0]; j<nvertex_in_rank[i]; j++)
            {
                if(compare(&level[j],&new_vert)==0)
                {
                    edges_less+=level[j]->count_children();
                    merge(new_vert,level[j]);
                }
                else
                    new_vert=level[j];
            }
            calc_vert_freq(root,i);
        }
        
        traverse_and_calc_diag_edges(root,NULL,&n_diag_edges);
        d_edges=(EDGE **)malloc(sizeof(EDGE *)*(n_diag_edges+1));
        if (d_edges==NULL)
            goto err;
        n_diag_edges=0;
        traverse_and_calc_diag_edges(root,d_edges,&n_diag_edges);
    }
    
    traverse_and_calc_vert(root,&n_vert);
    
    traverse_and_calc_vert_status(root);
    
    traverse_and_calc_one_child_vert(root, &n_one_child_vert);
    
    
    /*
     if ((ndata = fopen(NEWLST_FILE_NAME,"wt"))==NULL)
     {
     printf("NO RES FILE");
     goto EXIT;
     }
     wordlen=0;
     generate_listing(root,ndata);
     fclose(ndata);
     */
    
    printf("Num edges: %d, Num vert %d\n",nedges,n_vert);
    
    if ( convert_voc( TreeType, root, ppd) == PYDICT_ERR )
        goto err;//,voc_name);
    
    printf("Dictionary converted\n");
    
    if (level!=NULL)
        free(level);
    if (d_edges!=NULL)
        free(d_edges);

    delete(root);
    return(PYDICT_NOERR);
err:
    if (level!=NULL)
        free(level);
    if (d_edges!=NULL)
        free(d_edges);
    delete(root);
    return(PYDICT_ERR);
    
}

int find_set_num(void *cur_set, void *sets, int set_size, int n_sets,
                 int (*compare_sets)(const void *,const void *))
{
    int i;
    for (i=0; i<n_sets; i++)
    {
        if (compare_sets(cur_set,sets)==0) break;
        sets=(void *)((char *)sets+set_size);
    }
    return(i);
}

void traverse_and_put_set_nums(VERTEX *root,CHARSET *cur_charset,CHARSET *all_charsets,int n_all_charsets,
                               DVSET *cur_dvset,DVSET *dvsets,int n_dvsets)
{
    EDGE *tmp;
    int i=0,j=0;
    
    memset(cur_charset,0,sizeof(CHARSET));
    memset(cur_dvset,0,sizeof(DVSET));
    if (root->children!=NULL)
    {
        if (root->one_child_nd==false)
        {
            tmp=root->children;
            while((tmp->next)!=NULL)
            {
                tmp=tmp->next;
                if (tmp->diag==false && tmp->symbol!='\n')
                {
                    cur_charset->chset[i] = tmp->symbol;
                    i++;
                }
            }
            cur_charset->nd_len=i;
            
            tmp=root->children;
            while((tmp->next)!=NULL)
            {
                tmp=tmp->next;
                if (tmp->diag==true && tmp->symbol!='\n')
                {
                    cur_charset->chset[i] = tmp->symbol;
                    i++;
                    cur_dvset->dvset[j] = tmp->node;
                    j++;
                }
            }
            cur_charset->d_len=i-cur_charset->nd_len;
            cur_dvset->len=j;
            
            root->charset_num=find_set_num((void *)cur_charset,(void *)all_charsets,sizeof(CHARSET),n_all_charsets,
                                           compare_charsets);
            root->dvset_num=find_set_num((void *)cur_dvset,(void *)dvsets,sizeof(DVSET),n_dvsets,
                                         compare_dvsets);
            
            if (root->charset_num>=n_all_charsets || root->dvset_num>n_dvsets)
                printf("Wrong nums\n");
        }
        
        
        tmp=root->children;
        while((tmp->next)!=NULL)
        {
            tmp=tmp->next;
            if (tmp->diag==false && (INCLUDE_LAST || tmp->symbol!='\n'))
            {
                traverse_and_put_set_nums((VERTEX *)tmp->node,cur_charset,all_charsets,n_all_charsets,
                                          cur_dvset,dvsets,n_dvsets);
            }
        }
        
    }
    return;
}



int form_vert_address(VERTEX *cvert,unsigned char *vert_adr)
{
    
    int num_bytes;
    
    int num_in_graph=nvertex_before_level[cvert->rank]+cvert->num_in_level;
    
    if ( num_in_graph < (1<<15) )
    {
        num_bytes=2;
        vert_adr[0]=(unsigned char)(SHORT_VADR_FLAG | (num_in_graph>>8)); //SHORT_VADR_FLAG: 1000 0000
        vert_adr[1]=(unsigned char)(num_in_graph&0xFF);
    }
    else
    {
        num_bytes=3;
        vert_adr[0]=(unsigned char)(num_in_graph>>16);
        vert_adr[1]=(unsigned char)((num_in_graph>>8)&0xFF);
        vert_adr[2]=(unsigned char)(num_in_graph&0xFF);
    }
    return(num_bytes);
}



int count_dvset_size(DVSET *cur_dvset)
{
    int i;
    VERTEX *cvert;
    int num_bytes=0;
    unsigned char vert_adr[3];
    
    for (i=0; i<cur_dvset->len; i++)
    {
        cvert=(VERTEX *)(cur_dvset->dvset[i]);
        num_bytes+=form_vert_address(cvert,vert_adr);
    }
    return(num_bytes);
}


int count_vert_size(unsigned char TreeType,VERTEX *cvert,
                    DVSET *dvsets,int first_in_dvtable,CHARSET *chsets,int first_in_chtable)
//                      int *pn_syms_in_graph,int *pnv_syms_in_graph)
{
    int vsize=0;
    
    if (cvert->one_child_nd==true)
        vsize+=1;
    else if (!(TreeType & MERGE_FLAG))
    {
        int len=cvert->count_nd_children();
        if (len<=SHORT_ECHSET_LEN_MASK)
            vsize+=(len+1);  //0x07;
        else
            vsize+=(len+2);
    }
    else // TreeType==MERGED_TREE
    {
        int dvset_num=cvert->dvset_num;
        int chset_num=cvert->charset_num;
        DVSET cur_dvset;
        
        if (cvert->end_word>0)
        {
            extract_dvset(cvert,&cur_dvset,1);
            vsize+=(1+count_dvset_size(&cur_dvset));
        }
        else
        {
            if (dvset_num>=first_in_dvtable)
            {
                vsize+=1;
                if (dvset_num-first_in_dvtable>=MIN_LONG_DVSET_NUM)
                    vsize+=1;
            }
            else vsize+=(1+count_dvset_size(&dvsets[dvset_num]));
        }
        
        if (chset_num>=first_in_chtable)
        {
            vsize+=1;
            if (chset_num-first_in_chtable>=MIN_LONG_CHSET_NUM)
                vsize+=1;
        }
        else
        {
            vsize+=(chsets[chset_num].d_len+chsets[chset_num].nd_len);
        }
    }
    
    return(vsize);
}

int form_vert(unsigned char TreeType,VERTEX *cvert,unsigned char *vert,
              DVSET *dvsets,int first_in_dvtable,CHARSET *chsets,int first_in_chtable)
{
    int vsize=0,len;
    unsigned char *p=vert;
    unsigned char symb;
    int i;
    
    
    if (cvert->one_child_nd==true)
    {
        vsize+=1;
        symb=(unsigned char)cvert->children->next->symbol;
        if (cvert->end_word>0)//symb=='\n'
            symb=(unsigned char)cvert->end_word;
        
        *p =
        (symb | ONE_BYTE_FLAG);//0x80); //1000 0000
    }
    else if (!(TreeType&MERGE_FLAG))
    {
		EDGE *tmp;
        
        if (cvert->end_word>0)           //end word is present: in this case Dvset is always Uncoded
        {
            *p=END_WRD_FLAG;                  //0x40 //0100 0000
            // next 2 bits - statistics;
            *p|=(unsigned char)((cvert->end_word-1)<<4);
        }
        else
        {
            *p=0;
        }
        
        len=cvert->count_nd_children();
        if (len<=SHORT_ECHSET_LEN_MASK)          //0x07;
        {
            *p|=(unsigned char)SHORT_ECHSET_LEN_FLAG;
            *p|=(unsigned char)(len&SHORT_ECHSET_LEN_MASK);
            p++;
            vsize+=1;
        }
        else
        {
            *p|=(unsigned char)(len>>8);  p++;
            *p=(unsigned char)(len&0xFF);
            p++;
            vsize+=2;
        }
        tmp=cvert->children;
        i=0;
        while(tmp->next!=NULL)
        {
            tmp=tmp->next;
            if (tmp->symbol!='\n')
            {
                *p=tmp->symbol;
                p++;
                i++;
            }
        }
        if (i!=len)
        {
            printf("form_vert: Mistake\n");
        }
        vsize+=len;
    }
    else //TreeType==MERGED_TREE
    {
   	    int vasize;
	    int dvset_num,chset_num,d_len;
        unsigned char vert_adr[3];
        DVSET cur_dvset;
        
        dvset_num=cvert->dvset_num;
        chset_num=cvert->charset_num;
        
        if (cvert->end_word>0)           //end word is present: in this case Dvset is always Uncoded
        {
            *p=END_WRD_FLAG;                  //0x40 //0100 0000
            // next 2 bits - statistics;
            *p|=(unsigned char)((cvert->end_word-1)<<4);
            
            extract_dvset(cvert,&cur_dvset,1);
            d_len=cur_dvset.len;//chsets[chset_num].d_len;
            *p|=(unsigned char)d_len;           //<= 0000 1111
            p++;
            vsize+=1;
            UncodedDvsetsNVert++;
            NumEndVert++;
            for (i=0; i<d_len; i++)
            {
                vasize=form_vert_address((VERTEX *)cur_dvset.dvset[i],vert_adr);
                memcpy(p,vert_adr,vasize);
                p+=vasize;
                vsize+=vasize;
                UncodedDvsetsSize+=vasize;
                UncodedDvsetsNPntr++;
                EndDvsetsSize+=vasize;
                EndDvsetsNPntr++;
            }
        }
        else
        {
            *p=0;
            if (dvset_num>=first_in_dvtable && dvset_num<first_in_dvtable+MIN_LONG_DVSET_NUM)    //dvset_num<=0001 1111
            {
                *p|=((unsigned char)(dvset_num-first_in_dvtable)
                     | CODED_DVSET_FLAG | SHORT_DVSET_NUM_FLAG);             //0x30);   // | 0011 0000
                p++;
                vsize+=1;
            }
            else if (dvset_num>=first_in_dvtable+MIN_LONG_DVSET_NUM)                                                        //dvset_num<=0011 1111 1111 1111  !!!
            {
                *p|=((unsigned char)((dvset_num-first_in_dvtable)/256) | CODED_DVSET_FLAG); // &0x20;   // 0010 0000
                p++;
                *p=(unsigned char)((dvset_num-first_in_dvtable)%256);
                p++;
                vsize+=2;
            }
            else // dvset_num<first_in_dvtable
            {
                d_len=dvsets[dvset_num].len;//chsets[chset_num].d_len;
                *p|=(unsigned char)d_len;           //<= 0000 1111
                p++;
                vsize+=1;
                UncodedDvsetsNVert++;
                for (i=0; i<d_len; i++)
                {
                    vasize=form_vert_address((VERTEX *)dvsets[dvset_num].dvset[i],vert_adr);
                    memcpy(p,vert_adr,vasize);
                    p+=vasize;
                    vsize+=vasize;
                    UncodedDvsetsSize+=vasize;
                    UncodedDvsetsNPntr++;
                }
            }
		}
        
        if (chset_num>=first_in_chtable && chset_num<first_in_chtable+MIN_LONG_CHSET_NUM)
        {
            *p=(unsigned char)(chset_num-first_in_chtable) | CODED_CHSET_FLAG | SHORT_CHSET_NUM_FLAG;  //&0xC0 // 1100 0000
            vsize+=1;
        }
        else if (chset_num>=first_in_chtable+MIN_LONG_CHSET_NUM)
        {
            *p=(unsigned char)((chset_num-first_in_chtable)/256) | CODED_CHSET_FLAG; // &0x80;   // 1000 0000
            p++;
            *p=(unsigned char)((chset_num-first_in_chtable)%256);
            vsize+=2;
        }
        else   // chset_num<first_in_chtable
        {
            
            len=chsets[chset_num].nd_len+chsets[chset_num].d_len;
            for (i=0; i<len; i++)
            {
                *p=chsets[chset_num].chset[i];
                p++;
            }
            (*(p-1))|=LAST_SYM_FLAG;                 //&0x80; //1000 0000  len>=2 in uncoded chsets
            vsize+=len;
        }
    }
    //#endif
    
    return(vsize);
}

int x_count_vert_size(VERTEX *cvert)
{
    int len;
    
    len=cvert->count_children();
    return(len+1);
}


int x_form_vert(VERTEX *cvert, unsigned char *vert)
{
    int vsize=0,len;
    unsigned char *p=vert;
    EDGE *tmp;
    int i;
    
    len=cvert->count_children();      //including \n
    *p=(unsigned char)len;
    p++;
    vsize+=1;
    tmp=cvert->children;
    i=0;
    while(tmp->next!=NULL)
    {
        tmp=tmp->next;
        if (tmp->symbol!='\n')
            *p=tmp->symbol;
        else
            *p=(unsigned char)(cvert->end_word-1);
        p++;
        i++;
    }
    if (i!=len)
    {
        printf("x_form_vert: Mistake\n");
    }
    vsize+=len;
    
    return(vsize);
}


unsigned long count_level_size(unsigned char TreeType,VERTEX **level,int nv,
                               DVSET *dvsets,int first_in_dvtable,CHARSET *chsets,int first_in_chtable)
//                        int *pn_syms_in_graph,int *pnv_syms_in_graph)
{
    int i,vsize;
    unsigned long lsize=0;
    VERTEX *cvert;
    for(i=0; i<nv; i++)
    {
        cvert=level[i];
        vsize=count_vert_size(TreeType,cvert,dvsets,first_in_dvtable,chsets,first_in_chtable);//,pn_syms_in_graph,pnv_syms_in_graph);
        lsize+=vsize;
    }
    return(lsize);
}


int convert_graph(unsigned char TreeType,VERTEX *root,void **ppGraph,DVSET *dvsets,int first_in_dvtable,CHARSET *chsets,int first_in_chtable)
{
    int gsize,ghsize,lheader_len,lhsize,lsize,vsize,lshift[MAX_NUM_LEVELS];
    //#if TREE_TYPE==MERGED_TREE
    int num_prev_nd_childs = 0,lbshift = 0;
    //#endif
    int i,j;
    //   VERTEX *(*level)[MAX_NUM_LEVELS][MAX_NUM_IN_LEVEL];
    VERTEX **level[MAX_NUM_LEVELS];
    VERTEX *cvert;
    char *plv=NULL;
    unsigned long *plh;
    int *pgh;
    unsigned char vert[256];
    int nl;
    //   int n_syms_in_graph=0,nv_syms_in_graph=0;
    
    
    nl = (TreeType&MERGE_FLAG) ? num_levels : MAX_NUM_LEVELS;
    ghsize=2*sizeof(int)*nl;
    //   level=(VERTEX *(*)[MAX_NUM_LEVELS][MAX_NUM_IN_LEVEL])malloc(sizeof(*level));
    //   if (level==NULL) goto err;
    memset(level,0,sizeof(VERTEX**)*MAX_NUM_LEVELS);
    for (i=0; i<nl; i++)  if (nvertex_in_level[i]>0)
    {
        level[i]=(VERTEX**)malloc(sizeof(VERTEX*)*nvertex_in_level[i]);
        if (level[i]==NULL) goto err;
    }
    for (i=0,gsize=ghsize; i<nl; i++)
    {
        lshift[i]=gsize;
        if (i<num_levels)
        {
            cur_place=0;
            //         find_and_place_given_level(root,i,(VERTEX **)(*level)[i]);
            find_and_place_given_level(root,i,(VERTEX **)level[i]);
            lheader_len = (TreeType&MERGE_FLAG) ? 2*((nvertex_in_level[i]+LHDR_STEP-1)/LHDR_STEP) : 0;
            lhsize=sizeof(unsigned long)*lheader_len;
            //         lsize=lhsize+count_level_size(TreeType,(VERTEX **)(*level)[i],nvertex_in_level[i],
            lsize = (int)(lhsize + count_level_size(TreeType,(VERTEX **)level[i],nvertex_in_level[i],
                                          dvsets,first_in_dvtable,chsets,first_in_chtable));
            //                                    &n_syms_in_graph,&nv_syms_in_graph);
            gsize+=lsize;
        }
    }
    
    *ppGraph=malloc(gsize);
    if (*ppGraph==NULL) goto err;
    pgh=(int *)(*ppGraph);
    for (i=0; i<nl; i++)
    {
        pgh[2*i]=lshift[i];
        pgh[2*i+1]=nvertex_before_level[i];
        plh=(unsigned long *)((char *)(*ppGraph)+lshift[i]);
        if ( i>0 && (char *)plh!=plv )
        {
            printf( "convert_graph: Mistake\n" );
        }
        lheader_len = (TreeType&MERGE_FLAG) ?
	    2*((nvertex_in_level[i]+LHDR_STEP-1)/LHDR_STEP) : 0;
        plv=(char *)(plh+lheader_len);
        if (TreeType&MERGE_FLAG)
        {
            lbshift=sizeof(unsigned long)*lheader_len;
            num_prev_nd_childs=0;
        }
        for (j=0; j<nvertex_in_level[i]; j++)
        {
            if (TreeType&MERGE_FLAG)
            {
                if (j%LHDR_STEP==0)
                {
                    plh[0]=(unsigned long)lbshift;
                    plh[1]=(unsigned long)num_prev_nd_childs;
                    plh+=2;
                }
            }
            //         cvert=(*level)[i][j];
            cvert=level[i][j];
            vsize=form_vert(TreeType,cvert,vert,dvsets,first_in_dvtable,chsets,first_in_chtable);
            memcpy(plv,vert,vsize);
            plv+=vsize;
            if (TreeType&MERGE_FLAG)
            {
                lbshift+=vsize;
                num_prev_nd_childs+=cvert->count_nd_children();
            }
        }
    }
    //   if (level!=NULL) free(level);
    for (i=0; i<nl; i++)
    {
        if (level[i]!=NULL)
            free(level[i]);
    }
    return(gsize);
err:
    //   if (level!=NULL) free(level);
    for (i=0; i<nl; i++)
    {
        if (level[i]!=NULL)
            free(level[i]);
    }
    return(0);
}



char *fill_chset_tabl(char *pth,unsigned long thsize,CHARSET *sets,int i0,int in)
{
    char *pt=pth+thsize;
    unsigned long tbshift=thsize;
    int i,j;
    unsigned int cur_len;
    for (i=i0,cur_len=0; i<in; i++)
    {
        if (strlen(sets[i].chset)!=cur_len)
        {
            cur_len = (int)strlen(sets[i].chset);
            *((short *)pth)=(short)i;
            pth+=2;  //MAX_CHSET_NUM<1<<15-1!!!
            if (tbshift>(1<<31))
            {
                printf("fill_chset_tabl: too large shift!\n");
                goto err;
            }
            *((unsigned long *)pth)=tbshift;
            pth+=sizeof(unsigned long);//2;
            *((unsigned char *)pth)=(unsigned char)cur_len;
            pth++;
        }
        for (j=0; j<(int)cur_len; j++)
        {
            *pt=sets[i].chset[j];
            pt++;
        }
        tbshift+=cur_len;
    }
    return(pt);
err:
    return(NULL);
}



int convert_chset_tabl(CHARSET *sets,int n_sets,void **ppTabl)
{
    unsigned long t1hsize,t2hsize,t1size=0,t2size=0;
    unsigned long t1_shift=sizeof(unsigned int),t2_shift;
    int i;
    unsigned int cur_len;
    int num_len, tsize = 0;
    char *p;
    unsigned long *pth;
    char *pt1h,*pt2h;
    int NumShortChsets = (MIN_LONG_CHSET_NUM<n_sets) ?
    MIN_LONG_CHSET_NUM : n_sets;
    
    for (i=0,cur_len=0,num_len=0; i<NumShortChsets; i++)
    {
        if (strlen(sets[i].chset)!=cur_len)
        {
            cur_len = (int)strlen(sets[i].chset);
            num_len++;
        }
        t1size+=(cur_len);
    }
    t1hsize=(sizeof(short)+sizeof(unsigned long)+sizeof(unsigned char))*num_len; //03_16_00
    
    for (i=MIN_LONG_CHSET_NUM,cur_len=0,num_len=0; i<n_sets; i++)
    {
        if (strlen(sets[i].chset)!=cur_len)
        {
            cur_len = (int)strlen(sets[i].chset);
            num_len++;
        }
        t2size+=(cur_len);
    }
    t2hsize = (sizeof(short)+sizeof(unsigned long)+sizeof(unsigned char))*num_len;
    
    tsize = (int)(t1_shift+t1hsize+t1size+t2hsize+t2size);
    
    *ppTabl=malloc(tsize);
    if (*ppTabl==NULL)
        goto err;
    pth = (unsigned long *)(*ppTabl);
    t2_shift = t1_shift+t1hsize+t1size;
    *pth = t2_shift;
    pt1h = (char *)(pth+1);
    pt2h = fill_chset_tabl(pt1h,t1hsize,sets,0,NumShortChsets);
    
    if ( pt2h == NULL )
        goto err;
    if ( pt2h != (char *)(*ppTabl)+t2_shift )
    {
        printf("convert_chset_tabl: t2_shift: Mistake\n");
    }
    p=pt2h;
    if (n_sets>MIN_LONG_CHSET_NUM)
    {
        p = fill_chset_tabl( pt2h,t2hsize,sets,MIN_LONG_CHSET_NUM,n_sets );
    }
    if (p==NULL)
        goto err;
    if (p!=(char *)(*ppTabl) + tsize)
    {
        printf("convert_chset_tabl: tsize: Mistake\n");
    }
    return(tsize);
err:
    return(-1);
}


char * fill_dvset_tabl(char *pth,unsigned int thsize,DVSET *sets,int i0,int in)
{
    char *pt = pth+thsize;
    char *pt0 = pth+thsize;
    unsigned int tbshift=thsize;
    int i,j;
    int cur_len;
    int vasize;
    unsigned char vert_adr[3];
    
    for (i=i0,cur_len=0; i<in; i++)
    {
        if (sets[i].len!=cur_len)
        {
            cur_len=sets[i].len;
            *((short *)pth)=(short)i;
            pth+=2;  //MAX_DVSET_NUM<1<<15-1!!!
            if (tbshift>(1<<31))
            {
                printf("fill_dvset_tabl: too large shift!\n");
                goto err;
            }
            *((unsigned long *)pth)=tbshift;
            pth+=sizeof(unsigned long);//2;
            *((unsigned char *)pth)=(unsigned char)cur_len;
            pth++;
        }
        for (j=0; j<cur_len; j++)
        {
            vasize=form_vert_address((VERTEX *)sets[i].dvset[j],vert_adr);
            memcpy(pt,vert_adr,vasize);
            pt+=vasize;
            tbshift+=vasize;
        }
    }
    if ((char *)pth!=pt0)
    {
        printf("fill_dvset_tabl: Mistake\n");
    }
    return((char *)pt);
err:
    return(NULL);
}

int convert_dvset_tabl(DVSET *sets,int n_sets,void **ppTabl)
{
    unsigned int tsize,t1hsize,t2hsize,t1size=0,t2size=0;
    unsigned long t1_shift=sizeof(unsigned long),t2_shift;
    int i;
    int cur_len,num_len;
    char *p;
    unsigned long *pth;
    char *pt1h,*pt2h;
    int NumShortDvsets = (MIN_LONG_DVSET_NUM<n_sets) ?
    MIN_LONG_DVSET_NUM : n_sets;
    
    for (i=0,cur_len=0,num_len=0; i<NumShortDvsets; i++)
    {
        if (sets[i].len!=cur_len)
        {
            cur_len=sets[i].len;
            num_len++;
        }
        t1size+=count_dvset_size(&sets[i]);//sizeof(VERTEX_ADR)*cur_len;
    }
    t1hsize=(sizeof(short)+sizeof(unsigned long)+sizeof(unsigned char))*num_len;
    
    for (i=MIN_LONG_DVSET_NUM,cur_len=0,num_len=0; i<n_sets; i++)
    {
        if (sets[i].len!=cur_len)
        {
            cur_len=sets[i].len;
            num_len++;
        }
        t2size+=count_dvset_size(&sets[i]);
    }
    t2hsize=(sizeof(short)+sizeof(unsigned long)+sizeof(unsigned char))*num_len;
    
    tsize = (unsigned int)(t1_shift+t1hsize+t1size+t2hsize+t2size);
    
    
    *ppTabl=malloc(tsize);
    if (*ppTabl==NULL)
        goto err;
    pth=(unsigned long *)(*ppTabl);
    t2_shift=t1_shift+t1hsize+t1size;
    *pth=t2_shift;
    pt1h=(char *)(pth+1);
    pt2h=fill_dvset_tabl(pt1h,t1hsize,sets,0,NumShortDvsets);
    
    if (pt2h==NULL)
        goto err;
    if (pt2h!=(char *)(*ppTabl)+t2_shift)
    {
        printf("convert_dvset_tabl: t2_shift: Mistake\n");
    }
    p=pt2h;
    if (n_sets>MIN_LONG_DVSET_NUM)
    {
        p=fill_dvset_tabl(pt2h,t2hsize,sets,MIN_LONG_DVSET_NUM,n_sets);
    }
    if (pt2h==NULL)
        goto err;
    if (p!=(char *)(*ppTabl)+tsize)
    {
        printf("convert_dvset_tabl: tsize: Mistake\n");
    }
    return(tsize);
err:
    return(-1);
}


void count_dvsets_bytes_less(DVSET *dvsets,int n_dvsets)
{
    int i, dvset_size;
    for (i=0; i<n_dvsets; i++)
    {
        dvset_size=count_dvset_size(&dvsets[i]);
        dvsets[i].bytes_less=(dvset_size-1)*dvsets[i].num_vert-dvset_size;
        if (dvsets[i].len>DVSET_LEN_MASK)
            dvsets[i].bytes_less=10000;
    }
    return;
}


void count_charsets_bytes_less(CHARSET *charsets,int n_charsets)
{
    int i, j, cur_len,bytes_less;
    for (i=0; i<n_charsets; i++)
    {
        cur_len=charsets[i].nd_len+charsets[i].d_len;
        bytes_less=(cur_len-2)*charsets[i].num_vert-cur_len;
        if (cur_len==1)
            bytes_less=10000;
        for (j=0; j<cur_len; j++)
        {
            if ((unsigned char)charsets[i].chset[j]>=128)
                bytes_less=10000;
        }
        charsets[i].bytes_less=bytes_less;
    }
    return;
}

/*
 double count_sym_freq(char *pChsetTabl,int chset_tabl_size)
 {
 short t1shift,t2shift;
 char *pt1b,*pt2b,*pt1s,*pt2s,*p;
 char *pt1h,*pt2h;
 int n1sym,n2sym,nsym,i;
 unsigned char isym;
 double freq[256],entr=0.;
 unsigned short t1bshift,t2bshift;
 
 memset(freq,0,256*sizeof(double));
 
 t1shift=sizeof(short);
 t2shift=*((short *)pChsetTabl);
 
 pt1b=pChsetTabl+t1shift;
 pt1h=pt1b;
 t1bshift=*((unsigned short *)(pt1h+2));
 pt1s=pt1b+t1bshift;
 n1sym=t2shift-(t1shift+t1bshift);
 
 pt2b=pChsetTabl+t2shift;
 pt2h=pt2b;
 t2bshift=*((unsigned short *)(pt2h+2));
 pt2s=pt2b+t2bshift;
 n2sym=chset_tabl_size-(t2shift+t2bshift);
 
 for (i=0,p=pt1s; i<n1sym; i++)
 {
 isym=(unsigned char)p[i];
 freq[isym]+=1.f;
 }
 
 for (i=0,p=pt2s; i<n2sym; i++)
 {
 isym=(unsigned char)p[i];
 freq[isym]+=1.;
 }
 
 nsym=n1sym+n2sym;
 for (i=0; i<256; i++)
 freq[i]/=nsym;
 
 for (i=0; i<256; i++)
 if (freq[i]!=0.)
 entr+=(freq[i]*log(freq[i])/log(2.));
 
 return(entr);
 }
 */


void move(EDGE *new_prev,EDGE *cur_to_insert)   //BACKWARD MOVE; new_prev!=NULL
{
    if (cur_to_insert->next!=NULL)
        cur_to_insert->next->prev=cur_to_insert->prev;
    cur_to_insert->prev->next = cur_to_insert->next;
    
    new_prev->next->prev = cur_to_insert;
    cur_to_insert->next = new_prev->next;
    
    new_prev->next = cur_to_insert;
    cur_to_insert->prev = new_prev;
    
    return;
}


void resort_list_by_freq(EDGE *list_head)
{
    EDGE *cur_to_insert=list_head->next, *next_to_insert, *cur_in_sorted_list;
    
    while (cur_to_insert!=NULL)
    {
        next_to_insert=cur_to_insert->next;
        cur_in_sorted_list=cur_to_insert->prev;
        while (cur_in_sorted_list!=list_head && cur_in_sorted_list->freq<cur_to_insert->freq)
            cur_in_sorted_list=cur_in_sorted_list->prev;
        if (cur_in_sorted_list!=cur_to_insert->prev)
            move(cur_in_sorted_list,cur_to_insert);
        cur_to_insert=next_to_insert;
    }
    return;
}


void resort_graph_by_freq(VERTEX *root)
{
    EDGE *tmp;
    
    if (root->children==NULL) return;
    resort_list_by_freq(root->children);
    tmp=root->children;
    while(tmp->next!=NULL)
    {
        tmp=tmp->next;
        if (tmp->diag==false)
            resort_graph_by_freq((VERTEX *)tmp->node);
    }
    return;
}


int convert_voc(unsigned char TreeType,VERTEX *root,void **ppd)//,char *voc_name)
{
    void *pVoc;
    int i;
    CHARSET *all_charsets=NULL;
    DVSET   *dvsets=NULL;
    int first_in_dvtabl,first_in_chtabl;
    int n_all_charsets,n_dvsets;
    int NumTablChsets,NumTablShortChsets,
    NumTablDvsets,NumTablShortDvsets;
    CHARSET cur_charset;
    DVSET   cur_dvset;
    int gsize,chset_tabl_size=0,dvset_tabl_size=0;
    int vhsize,vsize;
    void *pGraph=NULL, *pChsetTabl=NULL, *pDvsetTabl=NULL;
    //   double entr;
    //   FILE *voc_file;
    
    
    if (TreeType&FREQ_FLAG)
        resort_graph_by_freq(root);
    
    /*
     if (ppd==NULL) goto err;
     
     if ((*ppd=HWRMemoryAlloc(sizeof(VOC)))==NULL) goto err;
     pVoc=(VOC *)(*ppd);
     
     memset(pVoc,0,sizeof(VOC));
     */
    
    up_rank=0;
    num_levels=0;
    max_nv_in_level=0;
    memset(nvertex_in_level,0,sizeof(int)*MAX_NUM_LEVELS);
    traverse_and_find_up_rank(root);
    /*
     if (max_nv_in_level>MAX_NUM_IN_LEVEL)
     {
     printf("Too many vertexes in level!\n");
     goto err;
     }
     */
    
    nvertex_before_level[0]=0;
    for (i=1; i<MAX_NUM_LEVELS; i++)
    {
        nvertex_before_level[i] = nvertex_before_level[i-1]+nvertex_in_level[i-1];
    }
    
    if (TreeType&MERGE_FLAG)
    {
        all_charsets=(CHARSET *)malloc(sizeof(CHARSET)*MAX_NUM_SETS);
        if (all_charsets==NULL)
            goto err;
        memset(all_charsets,0,sizeof(CHARSET)*MAX_NUM_SETS);
        n_all_charsets=count_num_charests(root,all_charsets,7);
        if (n_all_charsets<0)
        {
            printf("Too many char sets!!!\n");
            goto err;
        }
        
        
        dvsets=(DVSET *)malloc(sizeof(DVSET)*MAX_NUM_SETS);
        if (dvsets==NULL)
            goto err;
        memset(dvsets,0,sizeof(DVSET)*MAX_NUM_SETS);
        n_dvsets=count_num_dvsets(root,dvsets,4);
        if (n_dvsets<0)
        {
            printf("Too many dvsets!!!\n");
            goto err;
        }
        
        count_charsets_bytes_less(all_charsets,n_all_charsets);
        
        //   qsort(all_charsets,n_all_charsets,sizeof(CHARSET),compare_chsetslen);
        
        i=0;
        qsort((void *)all_charsets,n_all_charsets,sizeof(CHARSET),compare_chsetbl);
        while(i<n_all_charsets && all_charsets[i].bytes_less<=0)
            i++;
        first_in_chtabl=i;
        
        if (n_all_charsets-first_in_chtabl>MAX_CHSET_NUM)
        {
            first_in_chtabl=n_all_charsets-MAX_CHSET_NUM;
        }
        
        NumTablChsets=n_all_charsets-first_in_chtabl;
        NumTablShortChsets = (MIN_LONG_CHSET_NUM<NumTablChsets) ? MIN_LONG_CHSET_NUM : NumTablChsets;
        qsort(&all_charsets[first_in_chtabl],NumTablChsets,sizeof(CHARSET),compare_chsetnv);
        qsort(&all_charsets[first_in_chtabl],NumTablShortChsets, sizeof(CHARSET),compare_chsetslen);
        
        if ( NumTablChsets>NumTablShortChsets )
        {
            qsort(&all_charsets[first_in_chtabl+NumTablShortChsets],
                  NumTablChsets-NumTablShortChsets,
                  sizeof(CHARSET),compare_chsetslen);
        }
        
        count_dvsets_bytes_less(dvsets,n_dvsets);
        
        //   qsort((void *)dvsets,n_dvsets,sizeof(DVSET),compare_dvsetlen);
        
        i=0;
        qsort((void *)dvsets,n_dvsets,sizeof(DVSET),compare_dvsetbl);
        while (i<n_dvsets && dvsets[i].bytes_less<=0)
            i++;
        first_in_dvtabl=i;
        if (n_dvsets-first_in_dvtabl>MAX_DVSET_NUM)
            first_in_dvtabl=n_dvsets-MAX_DVSET_NUM;
        
        NumTablDvsets=n_dvsets-first_in_dvtabl;
        NumTablShortDvsets = (MIN_LONG_DVSET_NUM<NumTablDvsets) ? MIN_LONG_DVSET_NUM : NumTablDvsets;
        qsort((void *)&dvsets[first_in_dvtabl],NumTablDvsets,sizeof(DVSET),compare_dvsetnv);
        qsort(&dvsets[first_in_dvtabl],NumTablShortDvsets, sizeof(DVSET),compare_dvsetlen);
        
        if (NumTablDvsets>NumTablShortDvsets)
        {
            qsort(&dvsets[first_in_dvtabl+NumTablShortDvsets],
                  NumTablDvsets-NumTablShortDvsets,
                  sizeof(DVSET),compare_dvsetlen);
        }
        traverse_and_put_set_nums(root,&cur_charset,all_charsets,n_all_charsets,&cur_dvset,dvsets,n_dvsets);
        
        
        dvset_tabl_size= convert_dvset_tabl(&dvsets[first_in_dvtabl],n_dvsets-first_in_dvtabl,&pDvsetTabl);
        if (dvset_tabl_size<0)
            goto err;
        
        printf( "Dvset tabl size: %d\n", dvset_tabl_size);
        
        chset_tabl_size= convert_chset_tabl(&all_charsets[first_in_chtabl],n_all_charsets-first_in_chtabl,&pChsetTabl);
        if (chset_tabl_size<0)
            goto err;
        
        printf( "Charset tabl size: %d\n", chset_tabl_size);
        
        
        if ( (gsize=convert_graph(TreeType,root,&pGraph,dvsets,first_in_dvtabl,all_charsets,first_in_chtabl)) == 0 ) goto err;
    }
    else if ((gsize=convert_graph(TreeType,root,&pGraph,NULL,0,NULL,0)) == 0)
    {
        goto err;
    }
    
    printf("Graph size: %d\n",gsize);

    vhsize = PYDictHeaderSize(TreeType&MERGE_FLAG);
    vsize = (TreeType&MERGE_FLAG) ? vhsize+gsize+chset_tabl_size+dvset_tabl_size : vhsize+gsize;
    
    if ((*ppd=malloc(vsize))==NULL)
        goto err;
    pVoc = *ppd;
    
    PutTreeMerge(pVoc,TreeType&MERGE_FLAG);
    PYDictPutGraphSize(pVoc,gsize);
    if (TreeType&MERGE_FLAG)
    {
        PYDictPutChsetTablSize(pVoc,chset_tabl_size);
        PYDictPutDvsetTablSize(pVoc,dvset_tabl_size);
    }
    memcpy(PYDictGetGraph(pVoc),pGraph,gsize); 
    if (TreeType&MERGE_FLAG)
    {
        memcpy(PYDictGetChsetTabl(pVoc),pChsetTabl,chset_tabl_size); 
        memcpy(PYDictGetDvsetTabl(pVoc),pDvsetTabl,dvset_tabl_size); 
    }

     if (all_charsets!=NULL)
        free(all_charsets);
    if (dvsets!=NULL)
        free(dvsets);
    if (pGraph!=NULL)
        free(pGraph);
    if (pDvsetTabl!=NULL)
        free(pDvsetTabl);
    if (pChsetTabl!=NULL)
        free(pChsetTabl);
    
    return(PYDICT_NOERR);
    
err:
    if (all_charsets!=NULL)
        free(all_charsets);
    if (dvsets!=NULL)
        free(dvsets);
    if (pGraph!=NULL)
        free(pGraph);
    if (pDvsetTabl!=NULL)
        free(pDvsetTabl);
    if (pChsetTabl!=NULL)
        free(pChsetTabl);
    
    return(PYDICT_ERR);
}

