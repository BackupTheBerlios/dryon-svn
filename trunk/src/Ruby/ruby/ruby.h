/**********************************************************************

  ruby.h -

  $Author: nobu $
  created at: Thu Jun 10 14:26:32 JST 1993

  Copyright (C) 1993-2003 Yukihiro Matsumoto
  Copyright (C) 2000  Network Applied Communication Laboratory, Inc.
  Copyright (C) 2000  Information-technology Promotion Agency, Japan

**********************************************************************/

#ifndef RUBY_H
#define RUBY_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "config.h"
#include "defines.h"

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

#ifdef HAVE_STRING_H
# include <string.h>
#else
# include <strings.h>
#endif

#ifdef HAVE_INTRINSICS_H
# include <intrinsics.h>
#endif

#include <stddef.h>
#include <stdio.h>

/* need to include <ctype.h> to use these macros */
#ifndef ISPRINT
#define ISASCII(c) isascii((int)(unsigned char)(c))
#undef ISPRINT
#define ISPRINT(c) (ISASCII(c) && isprint((int)(unsigned char)(c)))
#define ISSPACE(c) (ISASCII(c) && isspace((int)(unsigned char)(c)))
#define ISUPPER(c) (ISASCII(c) && isupper((int)(unsigned char)(c)))
#define ISLOWER(c) (ISASCII(c) && islower((int)(unsigned char)(c)))
#define ISALNUM(c) (ISASCII(c) && isalnum((int)(unsigned char)(c)))
#define ISALPHA(c) (ISASCII(c) && isalpha((int)(unsigned char)(c)))
#define ISDIGIT(c) (ISASCII(c) && isdigit((int)(unsigned char)(c)))
#define ISXDIGIT(c) (ISASCII(c) && isxdigit((int)(unsigned char)(c)))
#endif

#define NORETURN_STYLE_NEW 1
#ifndef NORETURN
# define NORETURN(x) x
#endif

#if defined(HAVE_ALLOCA_H)
#include <alloca.h>
#endif

#ifdef _AIX
#pragma alloca
#endif

#if defined(__VMS)
# pragma builtins
# define alloca __alloca
#endif

#if SIZEOF_LONG != SIZEOF_VOIDP
# error ---->> ruby requires sizeof(void*) == sizeof(long) to be compiled. <<----
#endif
typedef unsigned long VALUE;
typedef unsigned long ID;

#ifdef __STDC__
# include <limits.h>
#else
# ifndef LONG_MAX
#  ifdef HAVE_LIMITS_H
#   include <limits.h>
#  else
    /* assuming 32bit(2's compliment) long */
#   define LONG_MAX 2147483647
#  endif
# endif
# ifndef LONG_MIN
#  define LONG_MIN (-LONG_MAX-1)
# endif
# ifndef CHAR_BIT
#  define CHAR_BIT 8
# endif
#endif

#if HAVE_LONG_LONG
# ifndef LLONG_MAX
#  ifdef LONG_LONG_MAX
#   define LLONG_MAX  LONG_LONG_MAX
#  else
#   ifdef _I64_MAX
#    define LLONG_MAX _I64_MAX
#   else
    /* assuming 64bit(2's complement) long long */
#    define LLONG_MAX 9223372036854775807LL
#   endif
#  endif
# endif
# ifndef LLONG_MIN
#  ifdef LONG_LONG_MIN
#   define LLONG_MIN  LONG_LONG_MIN
#  else
#   ifdef _I64_MIN
#    define LLONG_MIN _I64_MIN
#   else
#    define LLONG_MIN (-LLONG_MAX-1)
#   endif
#  endif
# endif
#endif

#define FIXNUM_MAX (LONG_MAX>>1)
#define FIXNUM_MIN RSHIFT((long)LONG_MIN,1)

#define FIXNUM_FLAG 0x01
#define INT2FIX(i) ((VALUE)(((long)(i))<<1 | FIXNUM_FLAG))
#define LONG2FIX(i) INT2FIX(i)
#define rb_fix_new(v) INT2FIX(v)
EXPORT VALUE rb_int2inum _((long));
#define INT2NUM(v) rb_int2inum(v)
#define LONG2NUM(v) INT2NUM(v)
#define rb_int_new(v) rb_int2inum(v)
EXPORT VALUE rb_uint2inum _((unsigned long));
#define UINT2NUM(v) rb_uint2inum(v)
#define ULONG2NUM(v) UINT2NUM(v)
#define rb_uint_new(v) rb_uint2inum(v)

#if HAVE_LONG_LONG
EXPORT VALUE rb_ll2inum _((LONG_LONG));
#define LL2NUM(v) rb_ll2inum(v)
EXPORT VALUE rb_ull2inum _((unsigned LONG_LONG));
#define ULL2NUM(v) rb_ull2inum(v)
#endif

#if SIZEOF_OFF_T > SIZEOF_LONG && defined(HAVE_LONG_LONG)
# define OFFT2NUM(v) LL2NUM(v)
#else
# define OFFT2NUM(v) INT2NUM(v)
#endif

#define FIX2LONG(x) RSHIFT((long)x,1)
#define FIX2ULONG(x) (((unsigned long)(x))>>1)
#define FIXNUM_P(f) (((long)(f))&FIXNUM_FLAG)
#define POSFIXABLE(f) ((f) <= FIXNUM_MAX)
#define NEGFIXABLE(f) ((f) >= FIXNUM_MIN)
#define FIXABLE(f) (POSFIXABLE(f) && NEGFIXABLE(f))

#define IMMEDIATE_MASK 0x03
#define IMMEDIATE_P(x) ((VALUE)(x) & IMMEDIATE_MASK)

#define SYMBOL_FLAG 0x0e
#define SYMBOL_P(x) (((VALUE)(x)&0xff)==SYMBOL_FLAG)
#define ID2SYM(x) ((VALUE)(((long)(x))<<8|SYMBOL_FLAG))
#define SYM2ID(x) RSHIFT((long)x,8)

/* special contants - i.e. non-zero and non-fixnum constants */
#define Qfalse 0
#define Qtrue  2
#define Qnil   4
#define Qundef 6		/* undefined value for placeholder */

#define RTEST(v) (((VALUE)(v) & ~Qnil) != 0)
#define NIL_P(v) ((VALUE)(v) == Qnil)

#define CLASS_OF(v) rb_class_of((VALUE)(v))

#define T_NONE   0x00

#define T_NIL    0x01
#define T_OBJECT 0x02
#define T_CLASS  0x03
#define T_ICLASS 0x04
#define T_MODULE 0x05
#define T_FLOAT  0x06
#define T_STRING 0x07
#define T_REGEXP 0x08
#define T_ARRAY  0x09
#define T_FIXNUM 0x0a
#define T_HASH   0x0b
#define T_STRUCT 0x0c
#define T_BIGNUM 0x0d
#define T_FILE   0x0e

#define T_TRUE   0x20
#define T_FALSE  0x21
#define T_DATA   0x22
#define T_MATCH  0x23
#define T_SYMBOL 0x24

#define T_BLKTAG 0x3b
#define T_UNDEF  0x3c
#define T_VARMAP 0x3d
#define T_SCOPE  0x3e
#define T_NODE   0x3f

#define T_MASK   0x3f

#define BUILTIN_TYPE(x) (((struct RBasic*)(x))->flags & T_MASK)

#define TYPE(x) rb_type((VALUE)(x))

EXPORT void rb_check_type _((VALUE,int));
#define Check_Type(v,t) rb_check_type((VALUE)(v),t)

EXPORT VALUE rb_str_to_str _((VALUE));
EXPORT VALUE rb_string_value _((volatile VALUE*));
EXPORT char *rb_string_value_ptr _((volatile VALUE*));
EXPORT char *rb_string_value_cstr _((volatile VALUE*));

#define StringValue(v) rb_string_value(&(v))
#define StringValuePtr(v) rb_string_value_ptr(&(v))
#define StringValueCStr(v) rb_string_value_cstr(&(v))

void rb_check_safe_obj _((VALUE));
void rb_check_safe_str _((VALUE));
#define SafeStringValue(v) do {\
    StringValue(v);\
    rb_check_safe_obj(v);\
} while (0)
/* obsolete macro - use SafeStringValue(v) */
#define Check_SafeStr(v) rb_check_safe_str((VALUE)(v))

void rb_secure _((int));
RUBY_EXTERN int ruby_safe_level;
#define rb_safe_level() (ruby_safe_level)
void rb_set_safe_level _((int));
void rb_secure_update _((VALUE));

EXPORT long rb_num2long _((VALUE));
EXPORT unsigned long rb_num2ulong _((VALUE));
#define NUM2LONG(x) (FIXNUM_P(x)?FIX2LONG(x):rb_num2long((VALUE)x))
#define NUM2ULONG(x) rb_num2ulong((VALUE)x)
#if SIZEOF_INT < SIZEOF_LONG
EXPORT long rb_num2int _((VALUE));
#define NUM2INT(x) (FIXNUM_P(x)?FIX2INT(x):rb_num2int((VALUE)x))
EXPORT long rb_fix2int _((VALUE));
#define FIX2INT(x) rb_fix2int((VALUE)x)
EXPORT unsigned long rb_num2uint _((VALUE));
#define NUM2UINT(x) rb_num2uint(x)
EXPORT unsigned long rb_fix2uint _((VALUE));
#define FIX2UINT(x) rb_fix2uint(x)
#else
#define NUM2INT(x) ((int)NUM2LONG(x))
#define NUM2UINT(x) ((unsigned int)NUM2ULONG(x))
#define FIX2INT(x) ((int)FIX2LONG(x))
#define FIX2UINT(x) ((unsigned int)FIX2ULONG(x))
#endif

#if HAVE_LONG_LONG
EXPORT LONG_LONG rb_num2ll _((VALUE));
EXPORT unsigned LONG_LONG rb_num2ull _((VALUE));
# define NUM2LL(x) (FIXNUM_P(x)?FIX2LONG(x):rb_num2ll((VALUE)x))
#endif

#if HAVE_LONG_LONG && SIZEOF_OFF_T > SIZEOF_LONG
# define NUM2OFFT(x) ((off_t)NUM2LL(x))
#else
# define NUM2OFFT(x) NUM2LONG(x)
#endif

EXPORT double rb_num2dbl _((VALUE));
#define NUM2DBL(x) rb_num2dbl((VALUE)(x))

/* obsolete API - use StringValue() */
char *rb_str2cstr _((VALUE,long*));
/* obsolete API - use StringValuePtr() */
#define STR2CSTR(x) rb_str2cstr((VALUE)(x),0)

#define NUM2CHR(x) (((TYPE(x) == T_STRING)&&(RSTRING(x)->len>=1))?\
                     RSTRING(x)->ptr[0]:(char)(NUM2INT(x)&0xff))
#define CHR2FIX(x) INT2FIX((long)((x)&0xff))

EXPORT VALUE rb_newobj _((void));
#define NEWOBJ(obj,type) type *obj = (type*)rb_newobj()
#define OBJSETUP(obj,c,t) do {\
    RBASIC(obj)->flags = (t);\
    RBASIC(obj)->klass = (c);\
    if (rb_safe_level() >= 3) FL_SET(obj, FL_TAINT);\
} while (0)
#define CLONESETUP(clone,obj) do {\
    OBJSETUP(clone,rb_singleton_class_clone((VALUE)obj),RBASIC(obj)->flags);\
    rb_singleton_class_attached(RBASIC(clone)->klass, (VALUE)clone);\
    if (FL_TEST(obj, FL_EXIVAR)) rb_copy_generic_ivar((VALUE)clone,(VALUE)obj);\
} while (0)
#define DUPSETUP(dup,obj) do {\
    OBJSETUP(dup,rb_obj_class(obj),(RBASIC(obj)->flags)&(T_MASK|FL_EXIVAR|FL_TAINT));\
    if (FL_TEST(obj, FL_EXIVAR)) rb_copy_generic_ivar((VALUE)dup,(VALUE)obj);\
} while (0)

struct RBasic {
    unsigned long flags;
    VALUE klass;
};

struct RObject {
    struct RBasic basic;
    struct st_table *iv_tbl;
};

struct RClass {
    struct RBasic basic;
    struct st_table *iv_tbl;
    struct st_table *m_tbl;
    VALUE super;
};

struct RFloat {
    struct RBasic basic;
    double value;
};

#define ELTS_SHARED FL_USER2

struct RString {
    struct RBasic basic;
    long len;
    char *ptr;
    union {
	long capa;
	VALUE shared;
    } aux;
};

struct RArray {
    struct RBasic basic;
    long len;
    union {
	long capa;
	VALUE shared;
    } aux;
    VALUE *ptr;
};

struct RRegexp {
    struct RBasic basic;
    struct re_pattern_buffer *ptr;
    long len;
    char *str;
};

struct RHash {
    struct RBasic basic;
    struct st_table *tbl;
    int iter_lev;
    VALUE ifnone;
};

struct RFile {
    struct RBasic basic;
    struct OpenFile *fptr;
};

struct RData {
    struct RBasic basic;
    void (*dmark) _((void*));
    void (*dfree) _((void*));
    void *data;
};

#define DATA_PTR(dta) (RDATA(dta)->data)

/*
#define RUBY_DATA_FUNC(func) ((void (*)_((void*)))func)
*/
typedef void (*RUBY_DATA_FUNC) _((void*));

EXPORT VALUE rb_data_object_alloc _((VALUE,void*,RUBY_DATA_FUNC,RUBY_DATA_FUNC));

#define Data_Wrap_Struct(klass,mark,free,sval)\
    rb_data_object_alloc(klass,sval,(RUBY_DATA_FUNC)mark,(RUBY_DATA_FUNC)free)

#define Data_Make_Struct(klass,type,mark,free,sval) (\
    sval = ALLOC(type),\
    memset(sval, 0, sizeof(type)),\
    Data_Wrap_Struct(klass,mark,free,sval)\
)

#define Data_Get_Struct(obj,type,sval) do {\
    Check_Type(obj, T_DATA); \
    sval = (type*)DATA_PTR(obj);\
} while (0)

struct RStruct {
    struct RBasic basic;
    long len;
    VALUE *ptr;
};

struct RBignum {
    struct RBasic basic;
    char sign;
    long len;
    void *digits;
};

#define R_CAST(st)   (struct st*)
#define RBASIC(obj)  (R_CAST(RBasic)(obj))
#define ROBJECT(obj) (R_CAST(RObject)(obj))
#define RCLASS(obj)  (R_CAST(RClass)(obj))
#define RMODULE(obj) RCLASS(obj)
#define RFLOAT(obj)  (R_CAST(RFloat)(obj))
#define RSTRING(obj) (R_CAST(RString)(obj))
#define RREGEXP(obj) (R_CAST(RRegexp)(obj))
#define RARRAY(obj)  (R_CAST(RArray)(obj))
#define RHASH(obj)   (R_CAST(RHash)(obj))
#define RDATA(obj)   (R_CAST(RData)(obj))
#define RSTRUCT(obj) (R_CAST(RStruct)(obj))
#define RBIGNUM(obj) (R_CAST(RBignum)(obj))
#define RFILE(obj)   (R_CAST(RFile)(obj))

#define FL_SINGLETON FL_USER0
#define FL_MARK      (1<<6)
#define FL_FINALIZE  (1<<7)
#define FL_TAINT     (1<<8)
#define FL_EXIVAR    (1<<9)
#define FL_FREEZE    (1<<10)

#define FL_USHIFT    11

#define FL_USER0     (1<<(FL_USHIFT+0))
#define FL_USER1     (1<<(FL_USHIFT+1))
#define FL_USER2     (1<<(FL_USHIFT+2))
#define FL_USER3     (1<<(FL_USHIFT+3))
#define FL_USER4     (1<<(FL_USHIFT+4))
#define FL_USER5     (1<<(FL_USHIFT+5))
#define FL_USER6     (1<<(FL_USHIFT+6))
#define FL_USER7     (1<<(FL_USHIFT+7))

#define FL_UMASK  (0xff<<FL_USHIFT)

#define SPECIAL_CONST_P(x) (IMMEDIATE_P(x) || !RTEST(x))

#define FL_ABLE(x) (!SPECIAL_CONST_P(x))
#define FL_TEST(x,f) (FL_ABLE(x)?(RBASIC(x)->flags&(f)):0)
#define FL_SET(x,f) do {if (FL_ABLE(x)) RBASIC(x)->flags |= (f);} while (0)
#define FL_UNSET(x,f) do {if (FL_ABLE(x)) RBASIC(x)->flags &= ~(f);} while (0)
#define FL_REVERSE(x,f) do {if (FL_ABLE(x)) RBASIC(x)->flags ^= (f);} while (0)

#define OBJ_TAINTED(x) FL_TEST((x), FL_TAINT)
#define OBJ_TAINT(x) FL_SET((x), FL_TAINT)
#define OBJ_INFECT(x,s) do {if (FL_ABLE(x) && FL_ABLE(s)) RBASIC(x)->flags |= RBASIC(s)->flags & FL_TAINT;} while (0)

#define OBJ_FROZEN(x) FL_TEST((x), FL_FREEZE)
#define OBJ_FREEZE(x) FL_SET((x), FL_FREEZE)

#define ALLOC_N(type,n) (type*)xmalloc(sizeof(type)*(n))
#define ALLOC(type) (type*)xmalloc(sizeof(type))
#define REALLOC_N(var,type,n) (var)=(type*)xrealloc((char*)(var),sizeof(type)*(n))

#define ALLOCA_N(type,n) (type*)alloca(sizeof(type)*(n))

#define MEMZERO(p,type,n) memset((p), 0, sizeof(type)*(n))
#define MEMCPY(p1,p2,type,n) memcpy((p1), (p2), sizeof(type)*(n))
#define MEMMOVE(p1,p2,type,n) memmove((p1), (p2), sizeof(type)*(n))
#define MEMCMP(p1,p2,type,n) memcmp((p1), (p2), sizeof(type)*(n))

void rb_obj_infect _((VALUE,VALUE));

void rb_glob _((const char*,void(*)(const char*,VALUE),VALUE));
void rb_globi _((const char*,void(*)(const char*,VALUE),VALUE));

EXPORT VALUE rb_define_class _((const char*,VALUE));
EXPORT VALUE rb_define_module _((const char*));
EXPORT VALUE rb_define_class_under _((VALUE, const char*, VALUE));
EXPORT VALUE rb_define_module_under _((VALUE, const char*));

EXPORT void rb_include_module _((VALUE,VALUE));
EXPORT void rb_extend_object _((VALUE,VALUE));

EXPORT void rb_define_variable _((const char*,VALUE*));
EXPORT void rb_define_virtual_variable _((const char*,VALUE(*)(ANYARGS),void(*)(ANYARGS)));
EXPORT void rb_define_hooked_variable _((const char*,VALUE*,VALUE(*)(ANYARGS),void(*)(ANYARGS)));
EXPORT void rb_define_readonly_variable _((const char*,VALUE*));
EXPORT void rb_define_const _((VALUE,const char*,VALUE));
EXPORT void rb_define_global_const _((const char*,VALUE));

#define RUBY_METHOD_FUNC(func) ((VALUE (*)(ANYARGS))func)
EXPORT void rb_define_method _((VALUE,const char*,VALUE(*)(ANYARGS),int));
EXPORT void rb_define_module_function _((VALUE,const char*,VALUE(*)(ANYARGS),int));
EXPORT void rb_define_global_function _((const char*,VALUE(*)(ANYARGS),int));

EXPORT void rb_undef_method _((VALUE,const char*));
EXPORT void rb_define_alias _((VALUE,const char*,const char*));
EXPORT void rb_define_attr _((VALUE,const char*,int,int));

EXPORT void rb_global_variable _((VALUE*));
EXPORT void rb_gc_register_address _((VALUE*));
EXPORT void rb_gc_unregister_address _((VALUE*));

EXPORT ID rb_intern _((const char*));
EXPORT char *rb_id2name _((ID));
EXPORT ID rb_to_id _((VALUE));

EXPORT char *rb_class2name _((VALUE));
EXPORT char *rb_obj_classname _((VALUE));

EXPORT void rb_p _((VALUE));

EXPORT VALUE rb_eval_string _((const char*));
EXPORT VALUE rb_eval_string_protect _((const char*, int*));
EXPORT VALUE rb_eval_string_wrap _((const char*, int*));
EXPORT VALUE rb_funcall __((VALUE, ID, int, ...));
EXPORT VALUE rb_funcall2 _((VALUE, ID, int, const VALUE*));
EXPORT VALUE rb_funcall3 _((VALUE, ID, int, const VALUE*));
EXPORT int rb_scan_args __((int, const VALUE*, const char*, ...));
EXPORT VALUE rb_call_super _((int, const VALUE*));

EXPORT VALUE rb_gv_set _((const char*, VALUE));
EXPORT VALUE rb_gv_get _((const char*));
EXPORT VALUE rb_iv_get _((VALUE, const char*));
EXPORT VALUE rb_iv_set _((VALUE, const char*, VALUE));

EXPORT VALUE rb_equal _((VALUE,VALUE));

EXPORT VALUE ruby_verbose, ruby_debug;

EXPORT NORETURN(void rb_raise __((VALUE, const char*, ...)));
EXPORT NORETURN(void rb_fatal __((const char*, ...)));
EXPORT NORETURN(void rb_bug __((const char*, ...)));
EXPORT NORETURN(void rb_sys_fail _((const char*)));
EXPORT NORETURN(void rb_iter_break _((void)));
EXPORT NORETURN(void rb_exit _((int)));
EXPORT NORETURN(void rb_notimplement _((void)));

void rb_warning __((const char*, ...));		/* reports if `-w' specified */
void rb_sys_warning __((const char*, ...));	/* reports if `-w' specified */
void rb_warn __((const char*, ...));		/* reports always */

EXPORT VALUE rb_each _((VALUE));
EXPORT VALUE rb_yield _((VALUE));
EXPORT VALUE rb_yield_values __((int n, ...));
EXPORT VALUE rb_yield_splat _((VALUE));
EXPORT int rb_block_given_p _((void));
EXPORT VALUE rb_iterate _((VALUE(*)(VALUE),VALUE,VALUE(*)(ANYARGS),VALUE));
EXPORT VALUE rb_rescue _((VALUE(*)(ANYARGS),VALUE,VALUE(*)(ANYARGS),VALUE));
EXPORT VALUE rb_rescue2 __((VALUE(*)(ANYARGS),VALUE,VALUE(*)(ANYARGS),VALUE,...));
EXPORT VALUE rb_ensure _((VALUE(*)(ANYARGS),VALUE,VALUE(*)(ANYARGS),VALUE));
EXPORT VALUE rb_catch _((const char*,VALUE(*)(ANYARGS),VALUE));
NORETURN(void rb_throw _((const char*,VALUE)));

VALUE rb_require _((const char*));

EXPORT void ruby_init _((void));
EXPORT void ruby_options _((int, char**));
NORETURN(void ruby_run _((void)));

EXPORT VALUE rb_mKernel;
EXPORT VALUE rb_mComparable;
EXPORT VALUE rb_mEnumerable;
EXPORT VALUE rb_mPrecision;
EXPORT VALUE rb_mErrno;
EXPORT VALUE rb_mFileTest;
EXPORT VALUE rb_mGC;
EXPORT VALUE rb_mMath;
EXPORT VALUE rb_mProcess;

EXPORT VALUE rb_cObject;
EXPORT VALUE rb_cArray;
EXPORT VALUE rb_cBignum;
EXPORT VALUE rb_cClass;
EXPORT VALUE rb_cDir;
EXPORT VALUE rb_cData;
EXPORT VALUE rb_cFalseClass;
EXPORT VALUE rb_cFile;
EXPORT VALUE rb_cFixnum;
EXPORT VALUE rb_cFloat;
EXPORT VALUE rb_cHash;
EXPORT VALUE rb_cInteger;
EXPORT VALUE rb_cIO;
EXPORT VALUE rb_cModule;
EXPORT VALUE rb_cNilClass;
EXPORT VALUE rb_cNumeric;
EXPORT VALUE rb_cProc;
EXPORT VALUE rb_cRange;
EXPORT VALUE rb_cRegexp;
EXPORT VALUE rb_cString;
EXPORT VALUE rb_cSymbol;
EXPORT VALUE rb_cThread;
EXPORT VALUE rb_cTime;
EXPORT VALUE rb_cTrueClass;
EXPORT VALUE rb_cStruct;

EXPORT VALUE rb_eException;
EXPORT VALUE rb_eStandardError;
EXPORT VALUE rb_eSystemExit;
EXPORT VALUE rb_eInterrupt;
EXPORT VALUE rb_eSignal;
EXPORT VALUE rb_eFatal;
EXPORT VALUE rb_eArgError;
EXPORT VALUE rb_eEOFError;
EXPORT VALUE rb_eIndexError;
EXPORT VALUE rb_eRangeError;
EXPORT VALUE rb_eIOError;
EXPORT VALUE rb_eRuntimeError;
EXPORT VALUE rb_eSecurityError;
EXPORT VALUE rb_eSystemCallError;
EXPORT VALUE rb_eTypeError;
EXPORT VALUE rb_eZeroDivError;
EXPORT VALUE rb_eNotImpError;
EXPORT VALUE rb_eNoMemError;
EXPORT VALUE rb_eNoMethodError;
EXPORT VALUE rb_eFloatDomainError;

EXPORT VALUE rb_eScriptError;
EXPORT VALUE rb_eNameError;
EXPORT VALUE rb_eSyntaxError;
EXPORT VALUE rb_eLoadError;

EXPORT VALUE rb_stdin, rb_stdout, rb_stderr;
EXPORT VALUE ruby_errinfo;

static inline VALUE
#if defined(HAVE_PROTOTYPES)
rb_class_of(VALUE obj)
#else
rb_class_of(obj)
    VALUE obj;
#endif
{
    if (FIXNUM_P(obj)) return rb_cFixnum;
    if (obj == Qnil) return rb_cNilClass;
    if (obj == Qfalse) return rb_cFalseClass;
    if (obj == Qtrue) return rb_cTrueClass;
    if (SYMBOL_P(obj)) return rb_cSymbol;

    return RBASIC(obj)->klass;
}

static inline int
#if defined(HAVE_PROTOTYPES)
rb_type(VALUE obj)
#else
rb_type(obj)
   VALUE obj;
#endif
{
    if (FIXNUM_P(obj)) return T_FIXNUM;
    if (obj == Qnil) return T_NIL;
    if (obj == Qfalse) return T_FALSE;
    if (obj == Qtrue) return T_TRUE;
    if (obj == Qundef) return T_UNDEF;
    if (SYMBOL_P(obj)) return T_SYMBOL;
    return BUILTIN_TYPE(obj);
}

static inline int
#if defined(HAVE_PROTOTYPES)
rb_special_const_p(VALUE obj)
#else
rb_special_const_p(obj)
    VALUE obj;
#endif
{
    if (SPECIAL_CONST_P(obj)) return Qtrue;
    return Qfalse;
}

#include "missing.h"
#include "intern.h"

#if defined(EXTLIB) && defined(USE_DLN_A_OUT)
/* hook for external modules */
static char *dln_libs_to_be_linked[] = { EXTLIB, 0 };
#endif

#if defined(HAVE_LIBPTHREAD)
#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif
typedef pthread_t rb_nativethread_t;
# define NATIVETHREAD_CURRENT() pthread_self()
# define NATIVETHREAD_EQUAL(t1,t2) pthread_equal((t1),(t2))
# define HAVE_NATIVETHREAD
#elif defined(_WIN32) || defined(_WIN32_WCE)
typedef DWORD rb_nativethread_t;
# define NATIVETHREAD_CURRENT() GetCurrentThreadId()
# define NATIVETHREAD_EQUAL(t1,t2) ((t1) == (t2))
# define HAVE_NATIVETHREAD
#endif
#ifdef HAVE_NATIVETHREAD
RUBY_EXTERN int is_ruby_native_thread();
#else
#define is_ruby_native_thread() (1)
#endif

#if defined(__cplusplus)
}  /* extern "C" { */
#endif

#endif /* ifndef RUBY_H */
