#ifndef DB_H
#define DB_H

#include <moo/moo.h>
#if __cplusplus
extern "C" {
#endif

typedef uint32_t	(*dbbuf_read_f)(void *dbbuf, void *buf, uint32_t len);
typedef int		(*dbbuf_readstring_f)(void *dbbuf, char **strp);
typedef void	(*dbbuf_unreadstring_f)(void *dbbuf);
typedef uint32_t	(*dbbuf_write_f)(void *dbbuf, void *buf, uint32_t len);
typedef int		(*dbbuf_writestring_f)(void *dbbuf, char *str);

typedef struct
{
	obj_t					Obj;
	dbbuf_read_f			Read;
	dbbuf_readstring_f		ReadString;
	dbbuf_unreadstring_f	UnReadString;
	dbbuf_write_f			Write;
	dbbuf_writestring_f		WriteString;
	char					*CurrentString;
	uint32_t					Flags;
	string_t				*String;
	char					Buf[512];
	char					*BufPtr;
	uint32_t					BufLength;
	uint32_t					WriteCount;
} dbbuf_t;

#define DBBUFF_STRINGUNREAD	1

int		dbbufReadString(dbbuf_t *tthis, char **strp);
void	dbbufUnReadString(dbbuf_t *tthis);
int		dbbufWriteString(dbbuf_t *tthis, char *str);
void	dbbufDispose(dbbuf_t *tthis);
int		dbbufInit(dbbuf_t *tthis, char *str);
dbbuf_t	*dbbufNew(char *str);

/* dbbufReadString() return codes. */

#define DBRS_OK			0
#define DBRS_MORE		1
#define DBRS_NOSTRING	2

typedef struct dbitems
{
	struct dbitems	*Next;
	uint32_t			NumItems;
	char			Items[0];
} dbitems_t;

typedef int		(*dbobj_accept_f)(void *tthis, char *name);
typedef int		(*dbobj_read_f)(void *tthis, int type, int index, dbbuf_t *dbbuf);
typedef int		(*dbobj_write_f)(void *tthis, int type, int index, dbbuf_t *dbbuf);
typedef void	(*dbobj_setitemsdef_f)(void *tthis, void *items, uint32_t num_items);
typedef int		(*dbobj_isdefault_f)(void *tthis, int index);
typedef void	(*dbobj_reset_f)(void *tthis);
typedef void	(*dbobj_foreachitem_f)(void *tthis, void *item);

typedef struct
{
	obj_t				Obj;
	dbobj_accept_f		Accept;
	dbobj_read_f		Read;
	dbobj_write_f		Write;
	dbobj_setitemsdef_f	SetItemsDef;
	dbobj_isdefault_f	IsDefault;
	char 				*Name;
	int					Changed;
	uint32_t				ItemSize;
	uint32_t				MinItems;
	uint32_t				NumItems;
	uint32_t				CurrentItem;
	void				*CurrentItemAddr;
	dbitems_t			DBItems;
} dbobj_t;

#define DBOBJRT_BINARY	0
#define DBOBJRT_STRING	1

#define DBOBJWT_BINARY	0
#define DBOBJWT_STRING	1
#define DBOBJWT_HELP	2
#define DBOBJWT_NAME	3

int		dbobjAccept(dbobj_t *tthis, char *name);
void	dbobjForEachItem(dbobj_t *tthis, dbobj_foreachitem_f func);
void	*dbobjGetItemAddr(dbobj_t *tthis, uint32_t index);
void	*dbobjGetItems(dbobj_t *tthis);
void	dbobjInitItems(dbobj_t *tthis);
void	dbobjDispose(dbobj_t *tthis);
int		dbobjInit(dbobj_t *tthis, char *name, uint32_t num_items, uint32_t min_items, uint32_t item_size);
void	*dbobjNew(char *name, uint32_t num_items, uint32_t min_items, uint32_t item_size);

#define DBAccept(obj, name)					(((dbobj_t *)(obj))->Accept(obj,name))
#define DBRead(obj, type, index, dbbuf)		(((dbobj_t *)(obj))->Read(obj,type,index,dbbuf))
#define DBWrite(obj, type, index, dbbuf)	(((dbobj_t *)(obj))->Write(obj,type,index,dbbuf))
#define DBIsDefault(obj, index)				(((dbobj_t *)(obj))->IsDefault(obj,index))
#define DBReset(obj)						(((dbobj_t *)(obj))->Reset(obj))

typedef struct
{
	dbobj_t	*DBObj;
	uint32_t	Index;
} dbobjref_t;

typedef struct
{
	dbbuf_t		DBBuf;
	char		*Name;
	FILE		*File;
	uint32_t		NumDBObjs;
	dbobjref_t	*DBObjRefs;
} dbfile_t;

/* dbfile API: */

dbfile_t	*dbfileNew(char *name, void *dbobjs,...);
int			dbfileInit(dbfile_t *tthis, char *name, void *dbobjs,...);
void		dbfileDispose(dbfile_t *tthis);
int			dbfileLoad(dbfile_t *tthis, char *name);
int			dbfileSaveAs(dbfile_t *tthis, char *name);
int			dbfileSave(dbfile_t *tthis);

typedef struct
{
	dbbuf_t		DBBuf;
	int			ArgC;
	char		**ArgV;
	int			ArgI;
	char		*Src;
	int			WriteCount;
	uint32_t		NumDBObjs;
	dbobjref_t	*DBObjRefs;
} dbarg_t;

/* dbarg API: */

dbarg_t	*dbargNew(void *dbobjs,...);
int		dbargInit(dbarg_t *tthis, void *dbobjs,...);
void	dbargDispose(dbarg_t *tthis);
int		dbargParse(dbarg_t *tthis, int argc, char **argv, uint32_t index);
void	dbargPrintHelp(dbarg_t *tthis);

typedef struct
{
	dbobj_t	DBObj;
	uint32_t	NumDBObjs;
} dbsection_t;

/* dbsection API: */

dbsection_t	*dbsectionNew(char *name, void *dbobjs,...);

typedef struct
{
	dbobj_t	DBObj;
	char	**Names;
	uint32_t	CurrentIndex;
	uint32_t	NumDBObjs;
} dbsections_t;

/* dbsections API: */

dbsections_t	*dbsectionsNew(char **names, void *dbobjs,...);

typedef struct
{
	dbobj_t		DBObj;
	int			Def;
	int			Max;
	int			Min;
} dbint_t;

dbint_t	*dbintNew(char *name, uint32_t num_items, uint32_t min_items, int def, int max, int min);
int		dbintSet(dbint_t *dbint, uint32_t index, int val);
int		*dbintGetAddr(dbint_t *dbint, uint32_t index);
int		dbintGet(dbint_t *dbint, uint32_t index);

typedef struct
{
	dbobj_t		DBObj;
	uint32_t		Def;
	uint32_t		Max;
	uint32_t		Min;
} dbuint_t;

dbuint_t	*dbuintNew(char *name, uint32_t num_items, uint32_t min_items, uint32_t def, uint32_t max, uint32_t min);
int			dbuintSet(dbuint_t *dbint, uint32_t index, uint32_t val);
uint32_t		*dbuintGetAddr(dbuint_t *dbint, uint32_t index);
uint32_t		dbuintGet(dbuint_t *dbint, uint32_t index);

typedef struct
{
	char	*Name;
	int		Val;
} dbenumdef_t;

typedef struct
{
	dbobj_t		DBObj;
	char		Def;
	dbenumdef_t	*DBEnumDefs;
} dbenum_t;

/* dbenum API: */

dbenum_t	*dbenumNew(char *name, uint32_t num_items, uint32_t min_items, char def, dbenumdef_t *dbenum_defs);
int			dbenumSet(dbenum_t *dbenum, uint32_t index, char val);
char		*dbenumGetAddr(dbenum_t *dbenum, uint32_t index);
char		dbenumGet(dbenum_t *dbenum, uint32_t index);

typedef dbenum_t dbbool_t;

/* dbbool API: */

dbbool_t	*dbboolNew(char *name, uint32_t num_items, uint32_t min_items, char def);
#define		dbboolSet(dbbool,index,val)	dbenumSet(dbbool,index,val)
#define		dbboolGetAddr(dbbool,index)	dbenumGetAddr(dbbool,index)
#define		dbboolGet(dbbool,index)		dbenumGet(dbbool,index)

typedef struct
{
	dbobj_t		DBObj;
	char		*Def;
} dbstring_t;

dbstring_t	*dbstringNew(char *name, uint32_t num_items, uint32_t min_items, char *def);
int			dbstringSet(dbstring_t *tthis, uint32_t index, char *str);
string_t	**dbstringGetAddr(dbstring_t *tthis, uint32_t index);
string_t	*dbstringGetString(dbstring_t *tthis, uint32_t index);
char		*dbstringGet(dbstring_t *tthis, uint32_t index);

#define DBEND	((void *)(~0))

#define DBE_OK				0
#define DBE_ERROR			1
#define DBE_NOSTRING		2
#define DBE_ILLEGALSTRING	3
#define DBE_MAXCLIPPED		4
#define DBE_MINCLIPPED		5
#define DBE_NOITEM			6
#define DBE_ILLEGALVALUE	7

#define DBFE_OK			0
#define DBFE_OPENFAILED	1

#if __cplusplus
}
#endif

#endif

