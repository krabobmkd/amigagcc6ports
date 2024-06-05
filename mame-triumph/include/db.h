#ifndef DB_H
#define DB_H
/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id$
 *
 * $Log$
 *
 *************************************************************************/

#include <moo/moo.h>

typedef uint (*dbbuf_read_f)(void *dbbuf, void *buf, uint len);
typedef int  (*dbbuf_readstring_f)(void *dbbuf, char **strp);
typedef void (*dbbuf_unreadstring_f)(void *dbbuf);
typedef uint (*dbbuf_write_f)(void *dbbuf, void *buf, uint len);
typedef int  (*dbbuf_writestring_f)(void *dbbuf, char *str);

typedef struct
{
  obj_t                Obj;
  dbbuf_read_f         Read;
  dbbuf_readstring_f   ReadString;
  dbbuf_unreadstring_f UnReadString;
  dbbuf_write_f        Write;
  dbbuf_writestring_f  WriteString;
  char                 *CurrentString;
  uint                 Flags;
  string_t             *String;
  char                 Buf[512];
  char                 *BufPtr;
  uint                 BufLength;
  uint                 WriteCount;
} dbbuf_t;

#define DBBUFF_STRINGUNREAD 1

int     dbbufReadString(dbbuf_t *this, char **strp);
void    dbbufUnReadString(dbbuf_t *this);
int     dbbufWriteString(dbbuf_t *this, char *str);
void    dbbufDispose(dbbuf_t *this);
bool_t  dbbufInit(dbbuf_t *this, char *str);
dbbuf_t *dbbufNew(char *str);

/* dbbufReadString() return codes. */

#define DBRS_OK       0
#define DBRS_MORE     1
#define DBRS_NOSTRING 2

typedef struct dbitems
{
  struct dbitems *Next;
  uint           NumItems;
  char           Items[0];
} dbitems_t;

typedef bool_t (*dbobj_accept_f)(void *this, char *name);
typedef int    (*dbobj_read_f)(void *this, int type, int index, dbbuf_t *dbbuf);
typedef int    (*dbobj_write_f)(void *this, int type, int index, dbbuf_t *dbbuf);
typedef void   (*dbobj_setitemsdef_f)(void *this, void *items, uint num_items);
typedef bool_t (*dbobj_isdefault_f)(void *this, int index);
typedef void   (*dbobj_reset_f)(void *this);
typedef void   (*dbobj_foreachitem_f)(void *this, void *item);

typedef struct
{
  obj_t               Obj;
  dbobj_accept_f      Accept;
  dbobj_read_f        Read;
  dbobj_write_f       Write;
  dbobj_setitemsdef_f SetItemsDef;
  dbobj_isdefault_f   IsDefault;
  char                *Name;
  int                 Changed;
  uint                ItemSize;
  uint                MinItems;
  uint                NumItems;
  uint                CurrentItem;
  void                *CurrentItemAddr;
  dbitems_t           DBItems;
} dbobj_t;

#define DBOBJRT_BINARY 0
#define DBOBJRT_STRING 1

#define DBOBJWT_BINARY 0
#define DBOBJWT_STRING 1
#define DBOBJWT_HELP   2
#define DBOBJWT_NAME   3

bool_t dbobjAccept(dbobj_t *this, char *name);
void   dbobjForEachItem(dbobj_t *this, dbobj_foreachitem_f func);
void   *dbobjGetItemAddr(dbobj_t *this, uint index);
void   *dbobjGetItems(dbobj_t *this);
void   dbobjInitItems(dbobj_t *this);
void   dbobjDispose(dbobj_t *this);
bool_t dbobjInit(dbobj_t *this, char *name, uint num_items, uint min_items, uint item_size);
void   *dbobjNew(char *name, uint num_items, uint min_items, uint item_size);

#define DBAccept(obj, name)              (((dbobj_t *)(obj))->Accept(obj,name))
#define DBRead(obj, type, index, dbbuf)  (((dbobj_t *)(obj))->Read(obj,type,index,dbbuf))
#define DBWrite(obj, type, index, dbbuf) (((dbobj_t *)(obj))->Write(obj,type,index,dbbuf))
#define DBIsDefault(obj, index)          (((dbobj_t *)(obj))->IsDefault(obj,index))
#define DBReset(obj)                     (((dbobj_t *)(obj))->Reset(obj))

typedef struct
{
  dbobj_t *DBObj;
  uint    Index;
} dbobjref_t;

typedef struct
{
  dbbuf_t     DBBuf;
  char       *Name;
  FILE       *File;
  uint       NumDBObjs;
  dbobjref_t *DBObjRefs;
} dbfile_t;

/* dbfile API: */

dbfile_t *dbfileNew(char *name, void *dbobjs,...);
bool_t   dbfileInit(dbfile_t *this, char *name, void *dbobjs,...);
void     dbfileDispose(dbfile_t *this);
int      dbfileLoad(dbfile_t *this, char *name);
int      dbfileSaveAs(dbfile_t *this, char *name);
int      dbfileSave(dbfile_t *this);

typedef struct
{
  dbbuf_t    DBBuf;
  int        ArgC;
  char       **ArgV;
  int        ArgI;
  char       *Src;
  int        WriteCount;
  uint       NumDBObjs;
  dbobjref_t *DBObjRefs;
} dbarg_t;

/* dbarg API: */

dbarg_t *dbargNew(void *dbobjs,...);
bool_t  dbargInit(dbarg_t *this, void *dbobjs,...);
void    dbargDispose(dbarg_t *this);
int     dbargParse(dbarg_t *this, int argc, char **argv, uint index);
void    dbargPrintHelp(dbarg_t *this);

typedef struct
{
  dbobj_t DBObj;
  uint    NumDBObjs;
} dbsection_t;

/* dbsection API: */

dbsection_t *dbsectionNew(char *name, void *dbobjs,...);

typedef struct
{
  dbobj_t DBObj;
  char    **Names;
  uint    CurrentIndex;
  uint    NumDBObjs;
} dbsections_t;

/* dbsections API: */

dbsections_t *dbsectionsNew(char **names, void *dbobjs,...);

typedef struct
{
  dbobj_t DBObj;
  int     Def;
  int     Max;
  int     Min;
} dbint_t;

dbint_t *dbintNew(char *name, uint num_items, uint min_items, int def, int max, int min);
int     dbintSet(dbint_t *dbint, uint index, int val);
int     *dbintGetAddr(dbint_t *dbint, uint index);
int     dbintGet(dbint_t *dbint, uint index);

typedef struct
{
  dbobj_t   DBObj;
  uint    Def;
  uint    Max;
  uint    Min;
} dbuint_t;

dbuint_t *dbuintNew(char *name, uint num_items, uint min_items, uint def, uint max, uint min);
int      dbuintSet(dbuint_t *dbint, uint index, uint val);
uint     *dbuintGetAddr(dbuint_t *dbint, uint index);
uint     dbuintGet(dbuint_t *dbint, uint index);

typedef struct
{
  char  *Name;
  int   Val;
} dbenumdef_t;

typedef struct
{
  dbobj_t   DBObj;
  char    Def;
  dbenumdef_t *DBEnumDefs;
} dbenum_t;

/* dbenum API: */

dbenum_t *dbenumNew(char *name, uint num_items, uint min_items, char def, dbenumdef_t *dbenum_defs);
int      dbenumSet(dbenum_t *dbenum, uint index, char val);
char     *dbenumGetAddr(dbenum_t *dbenum, uint index);
char     dbenumGet(dbenum_t *dbenum, uint index);

typedef dbenum_t dbbool_t;

/* dbbool API: */

dbbool_t *dbboolNew(char *name, uint num_items, uint min_items, char def);
#define  dbboolSet(dbbool,index,val) dbenumSet(dbbool,index,val)
#define  dbboolGetAddr(dbbool,index) dbenumGetAddr(dbbool,index)
#define  dbboolGet(dbbool,index)     dbenumGet(dbbool,index)

typedef struct
{
  dbobj_t DBObj;
  char    *Def;
  char    **Array;
  uint    ArrayLength;
} dbstring_t;

dbstring_t *dbstringNew(char *name, uint num_items, uint min_items, char *def);
int        dbstringSet(dbstring_t *this, uint index, char *str);
string_t   **dbstringGetAddr(dbstring_t *this, uint index);
string_t   *dbstringGetString(dbstring_t *this, uint index);
char       *dbstringGet(dbstring_t *this, uint index);
char       **dbstringGetArray(dbstring_t *this);

#define DBEND ((void *)(~0))

#define DBE_OK            0
#define DBE_ERROR         1
#define DBE_NOSTRING      2
#define DBE_ILLEGALSTRING 3
#define DBE_MAXCLIPPED    4
#define DBE_MINCLIPPED    5
#define DBE_NOITEM        6
#define DBE_ILLEGALVALUE  7

#define DBFE_OK         0
#define DBFE_OPENFAILED 1

#endif
