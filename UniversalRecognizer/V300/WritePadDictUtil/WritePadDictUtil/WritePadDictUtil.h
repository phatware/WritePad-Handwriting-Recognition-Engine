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

#ifndef __WritePadDictUtil_h__
#define __WritePadDictUtil_h__

#define MERGE_FLAG 0x01
#define FREQ_FLAG  0x02

#define LAST_SYM   1
#define MID_SYM    0

#define  MAX_CHSET_NUM   ((CHSET_NUM_MASK<<8)+0xFF)
#define  MAX_DVSET_NUM   ((DVSET_NUM_MASK<<8)+0xFF)


#define XRWD_INIT                  1      /* Empty vertex (the only vertex in empty voc)*/
#define XRWD_MIDWORD               2      /* There is no end of word in block db;*/
#define XRWD_WORDEND               3      /* There is end of word in block db;   */
#define XRWD_BLOCKEND              4      /* There is no tails in block db;      */

int     CreatePYDict( char * list_name, char * freq_name, unsigned char TreeType, void ** ppd );
int     find_dvset_size( unsigned char *p,int dvset_len );
unsigned char *find_dvset( char *pDvsetTabl,int dvset_num,int *pdvset_len );
int     find_dvset_len( char *pDvsetTabl,int dvset_num );
char *  find_chset( char *pChsetTabl,int chset_num,int *pchsetlen );
int     find_chset_len( char *pChsetTabl,int chset_num );
int     copy_dvset( int *dvset,unsigned char *p,int dvset_len );
char *  decode_vert( void *pVoc,char *vert,int *dvset,int *pdvset_len, char *chset,int *pchset_len );
char *  pass_vert( void *pVoc, char *vert );
unsigned char find_vert_status_and_attr( char *vert,unsigned char *pattr );
char *  pass_vert_and_find_setslen( void *pVoc,char *vert,int *pdvset_len,int *pchset_len ) ;
char *  find_vert( void *pVoc,int rank,int num_in_layer );
int     find_vert_rank( void *pVoc,int nvert,int *pnum_in_layer) ;
int     find_first_nd_child_num( void *pVoc,int rank,int num_in_layer );
char *  find_next_nd_child( void *pVoc,int rank,int num_in_layer,
                         char *prev_ndch_vert,int *pndch_num_in_layer,
                         unsigned char *pstatus,unsigned char *pattr );
int     generate_listing( void *pd, char *NLstFileName);
int     load_dictionary_from_file( char * pName, void ** ppd );
int     save_dictionary_to_file( char * pName, void * pDict );


#endif // __WritePadDictUtil_h__

