#ifndef MOO_H
#define MOO_H
/**************************************************************************
 *
 * Copyright (C) 1999 Mats Eirik Hansen (mats.hansen@triumph.no)
 *
 * $Id$
 *
 * $Log$
 *
 *************************************************************************/

#include <stdio.h>

typedef enum
{
  false,
  true
} bool_t;

typedef void *(*clone_f)(void *obj, int clone_state);
typedef void (*dispose_f)(void *obj);

typedef struct
{
  clone_f   Clone;
  dispose_f Dispose;
  size_t    Size;
  uint      Flags;
  int       RefCount;
} obj_t;

#define OBJF_ALLOCATED 1
#define OBJF_DISPOSED  2

obj_t  *objClone(obj_t *this, bool_t clone_state);
void   objDispose(obj_t *this);
bool_t objPostInit(obj_t *this);
void   objSetSize(obj_t *this, size_t size);
bool_t objInit(obj_t *this);
obj_t  *objNew(size_t size);

#define Dispose(obj) (((obj_t *)(obj))->Dispose(obj))

/* If clone_state = true then the entire state of the object is cloned so
 * the new object is identical to the original. Otherwise only the initial
 * state is cloned. */

#define Clone(obj, clone_state) (((obj_t *)(obj))->Clone(obj, clone_state))

/* If a class does Dispose() on objects that weren't created by the class
 * itself then IncRefCount() must be used on such object. If this is not done
 * then an object will be free while it still may be in use somewhere else. */

#define IncRefCount(obj) (++(((obj_t *)(obj))->RefCount))

/* All classes should do a if(DecRefCount(this) <= 0) in the dispose function to make
 * sure that the object is only freed when it's no longer in use. */

#define DecRefCount(obj) (--(((obj_t *)(obj))->RefCount))

typedef struct
{
  obj_t Obj;
  char  *Buf;
  uint  Length;
  uint  MaxLength;
  uint  MinLength;
} string_t;

#define STRE_OK    0
#define STRE_NOMEM 1

/* string API: */

bool_t   stringInit(string_t *this, char *str, uint min_len);
string_t *stringNew(char *str, uint min_len);
char     *stringGet(string_t *string);
int      stringAdd(string_t *string, uint tail, char *strs,...);
void     stringClear(string_t *string);
int      stringSet(string_t *string, char *str);
#define  stringAddHead(string,str) stringAdd(string,0,str,(char *)NULL)
#define  stringAddTail(string,str) stringAdd(string,1,str,(char *)NULL)

/* DEBUG_OBJVALID should be used at the top of every method except New, Init and Dispose.
 * DEBUG_DISPOSEOBJVALID should be used in Dispose because it's normal that the RefCount
 * is < 0 in Dispose. */

#ifdef DEBUG
#define DEBUG_OBJVALID(funcname,obj)    if(!(obj) || (((obj_t *)(obj))->RefCount < 0)) \
{ \
  printf("DEBUG_ERROR: Invalid object pointer in "funcname" (\""__FILE__"\" line %d).\n", __LINE__); \
  if(obj) puts("RefCount < 0."); \
  else puts("NULL pointer."); \
  puts("Press [Return] to continue."); \
  getchar(); \
}
#define DEBUG_DISPOSEOBJVALID(class,obj)  if(!(obj)) \
{ \
  printf("DEBUG_ERROR: Invalid object pointer in "class"Dispose (\""__FILE__"\" line %d).\nNULL pointer.\nPress [Return] to continue", __LINE__); \
  getchar(); \
}
#else
#define DEBUG_OBJVALID(funcname,obj)
#define DEBUG_DISPOSEOBJVALID(class,obj)
#endif

#endif
