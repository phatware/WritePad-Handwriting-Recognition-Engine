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

#ifndef HWR_DBG_DEFINED      /* This string suppresses the duplicate         */
                             /* declarations. (See #endif later.)            */
#define HWR_DBG_DEFINED

#include "bastypes.h"         /* Needed for the basic types declaration. */
#include "hwr_sys.h"          /* Needed for _HMEM declaration.           */
#include "stderr.h"
#include "obj.h"



/*              1. Constants.                                               */
/*              =============                                               */


/* Object types for GetStructure function */

#define     HWD_STRUCT_DL       0x00
#define     HWD_STRUCT_ITEM     0x01
#define     HWD_STRUCT_MARKER   0x02


/* Breakpoint flags */

#define     HWDBF_NOACTION      0x00        /* Do nothing                   */
#define     HWDBF_PRINT         0x01        /* Print message                */
#define     HWDBF_DRAW          0x02        /* Update image                 */
#define     HWDBF_BREAK         0x04        /* Enter dialog mode            */

#define     HWDBF_GROUP         0x0100      /* Group header                 */
#define     HWDBF_ENABLE        0x8000      /* Breakpoint enabled           */

/* Predefined breakpoints */
#define     HWDBPT_PRINT        1
#define     HWDBPT_DRAW         2
#define     HWDBPT_BREAK        3
#define     HWDBPT_MANUAL       4


#if 0

/*              2. Data types.                                              */
/*              ==============                                              */


/* Structure description */

typedef struct {
                p_CHAR  Name;               /* Structure name               */
                _WORD   Size;               /* Total size                   */
                _WORD   FieldCount;         /* Number of field              */
                struct {
                        p_CHAR  Name;       /* Field name                   */
                        _WORD   ItemCount;  /* Number of items (>1 if array)*/
                        _CHAR   ItemSize;   /* Size of item in bytes (<=4)  */
                        _CHAR   ItemType;   /* Item type flags              */
                } _PTR  Fields;

} _HWD_STRUCT, _PTR p_HWD_STRUCT;



/*              3. Debugger support functions.                              */
/*              ==============================                              */

p_HANDLE HWD_GetDLList( _VOID );
                                /* Returns a far pointer to zero-terminated */
                                /* list of DL handles.                      */

p_HWD_STRUCT HWD_GetStructure( _HANDLE Handle, _WORD Object_type );
                                /* Returns structure  of object,  specified */
                                /* by the Handle.                           */


#endif

/*              4. Debugger calls.                                          */
/*              ==================                                          */

_VOID HWD_Ready( _VOID );
                                /*   Inform  HWD  (if  it  is present) that */
                                /* some internal process if finished.   Has */
                                /* to   be   called    in   the   end    of */
                                /* initialization.                          */


_WORD HWD_RegisterBptGroup( p_CHAR szName, _WORD wParentGroupID );

                        /* This function registers a group of  breakpoints. */
                        /* The group will be  shown in a separate  debugger */
                        /* menu.   wGroupID is  an ID  of registered group; */
                        /* group _NULL is the root.                         */
                        /*                                                  */
                        /* Returns Group ID.                                */

_WORD HWD_RegisterBpt( p_CHAR szName, _WORD wGroupID,
                                _WORD  wFlags, _WORD wPause );

                        /* This function registers a breakpoint.            */
                        /* wGroupID is an ID of registered group.           */
                        /* wFlags   define   debugger   actions   when  the */
                        /* breakpoint occurs.                               */
                        /* wPause,  if   specified,  is   a  delay   before */
                        /* continuing  application.  wPause  is  ignored if */
                        /* HWDBF_BREAK flag is set.                         */
                        /*                                                  */
                        /* Returns Breakpoint ID                            */

_VOID HWD_RegisterView( _HANDLE hWnd );

                        /* Register active viewer window                    */

_VOID HWD_UnregisterView( _HANDLE hWnd );

                        /* Unregister active viewer window                  */

_WORD HWD_GetBptID( p_CHAR szPath );

                        /* This function looks for a BPT by name.           */
                        /* Path is defined just as in file system:          */
                        /* "group1\group2\breakpoint". Fool path from the   */
                        /* root has to be specified for all functions that  */
                        /* require a szPath parameter                       */
                        /*                                                  */
                        /* Returns Breakpoint ID                            */


_VOID HWD_SetBptFlags( _WORD wBptID, _WORD wFlags );

                        /* This  function  changes  flags  of  a registered */
                        /* breakpoint.                                      */

_VOID HWD_EnableBpt( _WORD wBptID );

                        /* This function enables breakpoint.                */

_VOID HWD_DisableBpt( _WORD wBptID );

                        /* This function disables breakpoint.               */

_VOID HWD_BptSetPassCount( _WORD wBptID, _WORD wFirst, _WORD wStep,
                                    _WORD wLast,  _WORD wCurrent );

                        /* This function modifies breakpoint parameters.    */

_WORD HWD_Bpt( _WORD wBptID, p_CHAR szPath, p_CHAR szFormat, ... );

                        /* This function activates breakpoint specified by  */
                        /* ID and/or name (path). If ID is not zero, name   */
                        /* is ignored; otherwise, breakpoint is found by    */
                        /* name.                                            */
                        /*                                                  */
                        /* Returns Breakpoint ID                            */


_VOID HWD_Printf( p_CHAR format, ... );

                                /* Formatted output into StdErr window      */


/*              5. Object mapping facility                                  */
/*              ==========================                                  */

#define HWDMAP_MAXID    0x3F    /* Max valid ID for HandleMap               */

_BOOL HWD_HandleMap( _HOBJ hObj, _HATOM ID );

                                /* Map object hObj to ID.                   */
                                /* The function returns zero if ID is       */
                                /* invalid.                                 */


_VALUE HWD_PassMessage( _HATOM ID, _WORD wCommand, _VALUE vParam );

                                /* Object Call an object associated with ID */

#endif   /* HWR_DBG_DEFINED */

/*  */
