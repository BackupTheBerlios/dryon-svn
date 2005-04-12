/*  SCLIB.C
 *
 *  This is an example file that shows how to embed the Small compiler into a
 *  program. This program contains the "default" implementation of all
 *  functions that the Small compiler calls for I/O.
 *
 *  This program also contains a main(), so it compiles, again, to a
 *  stand-alone compiler. This is for illustration purposes only
 *
 *  What this file does is (in sequence):
 *  1. Declare the NO_MAIN macro, so that the function main() and all
 *     "callback" functions that are in SC1.C are not compiled.
 *  2. Declare SC_FUNC and SC_VDEFINE as "static" so that all functions and
 *     global variables are "encapsulated" in the object file. This solves
 *     the global namespace polution problem.
 *  3. Declare the SC_SKIP_VDECL macro which is needed to avoid variables to
 *     be doubly declared when the C files are *not* independently compiled.
 *  4. And, the dirtiest trick of all, include the remaining C files. That is,
 *     the entire Small compiler compiles to a single object file (.OBJ in
 *     Windows). This is the only way to get rid of the global namespace
 *     polution.
 *
 *  Note that the interface of the Small compiler is subject to change.
 *
 *  Compilation:
 *      wcl386 /l=nt sclib.c
 *
 *  Copyright (c) ITB CompuPhase, 2000-2002
 *  This file may be freely used. No warranties of any kind.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define NO_MAIN
#define NOPROPLIST

#define SC_FUNC    static
#define SC_VDEFINE static
#define SC_SKIP_VDECL   /* skip variable "forward declaration" */
#include "amx/compiler/sc.h"

#include "amx/compiler/scvars.c"
#include "amx/compiler/sc1.c"
#include "amx/compiler/sc2.c"
#include "amx/compiler/sc3.c"
#include "amx/compiler/sc4.c"
#include "amx/compiler/sc5.c"
#include "amx/compiler/sc6.c"
#include "amx/compiler/sc7.c"
#include "amx/compiler/sclist.c"
#include "amx/compiler/scexpand.c"
#include "amx/compiler/sci18n.c"

#include "config.h"
#include "log.h"

char * AMXAPI amx_StrError(int errnum)
{
static char *messages[] = {
      /* AMX_ERR_NONE      */ "(none)",
      /* AMX_ERR_EXIT      */ "Forced exit",
      /* AMX_ERR_ASSERT    */ "Assertion failed",
      /* AMX_ERR_STACKERR  */ "Stack/heap collision (insufficient stack size)",
      /* AMX_ERR_BOUNDS    */ "Array index out of bounds",
      /* AMX_ERR_MEMACCESS */ "Invalid memory access",
      /* AMX_ERR_INVINSTR  */ "Invalid instruction",
      /* AMX_ERR_STACKLOW  */ "Stack underflow",
      /* AMX_ERR_HEAPLOW   */ "Heap underflow",
      /* AMX_ERR_CALLBACK  */ "No (valid) native function callback",
      /* AMX_ERR_NATIVE    */ "Native function failed",
      /* AMX_ERR_DIVIDE    */ "Divide by zero",
      /* AMX_ERR_SLEEP     */ "(sleep mode)",
      /* 13 */                "(reserved)",
      /* 14 */                "(reserved)",
      /* 15 */                "(reserved)",
      /* AMX_ERR_MEMORY    */ "Out of memory",
      /* AMX_ERR_FORMAT    */ "Invalid/unsupported P-code file format",
      /* AMX_ERR_VERSION   */ "File is for a newer version of the AMX",
      /* AMX_ERR_NOTFOUND  */ "File or function is not found",
      /* AMX_ERR_INDEX     */ "Invalid index parameter (bad entry point)",
      /* AMX_ERR_DEBUG     */ "Debugger cannot run",
      /* AMX_ERR_INIT      */ "AMX not initialized (or doubly initialized)",
      /* AMX_ERR_USERDATA  */ "Unable to set user data field (table full)",
      /* AMX_ERR_INIT_JIT  */ "Cannot initialize the JIT",
      /* AMX_ERR_PARAMS    */ "Parameter error",
    };
  if (errnum < 0 || errnum >= sizeof messages / sizeof messages[0])
    return "(unknown)";
  return messages[errnum];
}

// build a .amx file
int srun_BuildScript(const char *name)
{
	char *outfile;
	char *argv[8];
	int ret;

	outfile= strdup(name);

	argv[0]= "amxbot.compiler";
	argv[1]= "-d1";
	argv[2]= "-t0";
	argv[3]= "-iinclude";
	argv[4]= "-;+";
	argv[5]= "-(+";
	argv[6]= new char[50];
	argv[7]= new char[50];

	// file.sma
	snprintf(argv[6], 49, "%s", outfile);
	strcpy(outfile+strlen(outfile)-3, "amx");
	// file.amx
	snprintf(argv[7], 49, "-o%s", outfile);

	ret= sc_compile(8, argv);

	free(outfile);
	delete [] argv[6];
	delete [] argv[7];
	return ret;
}

size_t srun_ProgramSize(const char *filename)
{
  FILE *fp;
  AMX_HEADER hdr;

  if ((fp=fopen(filename,"rb")) == NULL)
    return 0;
  fread(&hdr, sizeof hdr, 1, fp);
  fclose(fp);

  amx_Align16(&hdr.magic);
  amx_Align32((uint32_t*)&hdr.stp);
  return (hdr.magic==AMX_MAGIC) ? (size_t)hdr.stp : 0;
}

int srun_LoadProgram(AMX *amx, const char *filename, void *memblock)
{
  FILE *fp;
  AMX_HEADER hdr;

  if ((fp = fopen(filename, "rb")) == NULL )
    return AMX_ERR_NOTFOUND;
  fread(&hdr, sizeof hdr, 1, fp);
  amx_Align32((uint32_t*)&hdr.size);
  rewind(fp);
  fread(memblock, 1, (size_t)hdr.size, fp);
  fclose(fp);

  //printf("%d Ko\n", hdr.size/1024);

  memset(amx, 0, sizeof *amx);
  return amx_Init(amx, memblock);
}

/* sc_printf
 * Called for general purpose "console" output. This function prints general
 * purpose messages; errors go through sc_error(). The function is modelled
 * after printf().
 */
int sc_printf(const char *message,...)
{
/*
	int ret;
	va_list argptr;

	va_start(argptr,message);
	ret=vprintf(message,argptr);
	va_end(argptr);

	return ret;
*/
	return 0;
}

/* sc_error
 * Called for producing error output.
 *    number      the error number (as documented in the manual)
 *    message     a string describing the error with embedded %d and %s tokens
 *    filename    the name of the file currently being parsed
 *    firstline   the line number at which the expression started on which
 *                the error was found, or -1 if there is no "starting line"
 *    lastline    the line number at which the error was detected
 *    argptr      a pointer to the first of a series of arguments (for macro
 *                "va_arg")
 * Return:
 *    If the function returns 0, the parser attempts to continue compilation.
 *    On a non-zero return value, the parser aborts.
 */
int sc_error(int number,char *message,char *filename,int firstline,int lastline,va_list argptr)
{
	static char *prefix[3]={ "Error", "Fatal", "Warning" };
	char tmp[200];
	char *color= "";

	if (number!=0)
	{
		char *pre;

		switch( number/100 )
		{
		case 0:
		case 1: color= COLOR_RED; break;
		case 2: color= COLOR_GREEN; break;
		}

		pre=prefix[number/100];
		if (firstline>=0)
			Output("%s%s(%d -> %d) %s: ",color,filename,firstline,lastline,pre);
		else
			Output("%s%s(%d) %s: ",color,filename,lastline,pre);
	} /* if */

	vsprintf(tmp, message,argptr);
	Output("%s%s", tmp, COLOR_RESET);

	//fflush(stdout);
	return 0;
}

/* sc_opensrc
 * Opens a source file (or include file) for reading. The "file" does not have
 * to be a physical file, one might compile from memory.
 *    filename    the name of the "file" to read from
 * Return:
 *    The function must return a pointer, which is used as a "magic cookie" to
 *    all I/O functions. When failing to open the file for reading, the
 *    function must return NULL.
 */
void *sc_opensrc(char *filename)
{
  return fopen(filename,"rt");
}

/* sc_closesrc
 * Closes a source file (or include file). The "handle" parameter has the
 * value that sc_opensrc() returned in an earlier call.
 */
void sc_closesrc(void *handle)
{
  assert(handle!=NULL);
  fclose((FILE*)handle);
}

/* sc_resetsrc
 * "position" may only hold a pointer that was previously obtained from
 * sc_getpossrc() */
void sc_resetsrc(void *handle,void *position)
{
  assert(handle!=NULL);
  fsetpos((FILE*)handle,(fpos_t *)position);
}

char *sc_readsrc(void *handle, unsigned char *target, int maxchars)
{
  return fgets((char*)target,maxchars,(FILE*)handle);
}

void *sc_getpossrc(void *handle)
{
  static fpos_t lastpos;

  fgetpos((FILE*)handle,&lastpos);
  return &lastpos;
}

int sc_eofsrc(void *handle)
{
  return feof((FILE*)handle);
}

/* should return a pointer, which is used as a "magic cookie" to all I/O
 * functions; return NULL for failure
 */
void *sc_openasm(char *filename)
{
  return fopen(filename,"w+t");
}

void sc_closeasm(void *handle, int deletefile)
{
  fclose((FILE*)handle);
  if (deletefile)
    unlink(outfname);
}

void sc_resetasm(void *handle)
{
  fflush((FILE*)handle);
  fseek((FILE*)handle,0,SEEK_SET);
}

int sc_writeasm(void *handle,char *st)
{
  return fputs(st,(FILE*)handle) >= 0;
}

char *sc_readasm(void *handle, char *target, int maxchars)
{
  return fgets(target,maxchars,(FILE*)handle);
}

/* Should return a pointer, which is used as a "magic cookie" to all I/O
 * functions; return NULL for failure.
 */
void *sc_openbin(char *filename)
{
  return fopen(filename,"wb");
}

void sc_closebin(void *handle,int deletefile)
{
  fclose((FILE*)handle);
  if (deletefile)
    unlink(binfname);
}

void sc_resetbin(void *handle)
{
  fflush((FILE*)handle);
  fseek((FILE*)handle,0,SEEK_SET);
}

int sc_writebin(void *handle,void *buffer,int size)
{
/*
  if( size == sizeof(AMX_HEADER) )
  {
  	AMX_HEADER *hdr= buffer;
  	printf("size: %u\n", hdr->stp);
  }
*/
  return fwrite(buffer,1,size,(FILE*)handle) == (unsigned int)size;
}

long sc_lengthbin(void *handle)
{
  return ftell((FILE*)handle);
}
