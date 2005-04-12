/*
********************************************************************************
*                                 Dryon project
********************************************************************************
*
*  This is free software under the GPL license.
*  Since this software uses Small from Compuphase, it also use its license.
*  Copy of the two licenses are in the main dir (gpl.txt and amx_license.txt
*  Coded by Ammous Julien known as Anthalir
*
********************************************************************************
*/

#include "config.h"
#include <string.h>
#include <assert.h>
#include "amx.h"
#include "natives.h"
#include "tokens.h"


#if !defined(__FreeBSD__)

size_t strlcat(char *d, const char *s, size_t bufsize)
{
	size_t len1 = strlen(d);
	size_t len2 = strlen(s);
	size_t ret = len1 + len2;

	if (len1+len2 >= bufsize) {
		len2 = bufsize - (len1+1);
	}
	if (len2 > 0) {
		memcpy(d+len1, s, len2);
		d[len1+len2] = 0;
	}
	return ret;
}


int asprintf(char **ptr, const char *format, ...)
{
	va_list ap;
	int ret, size;
	char buffer[2 * 1024];

	*ptr = NULL;
	va_start(ap, format);
	size= vsnprintf(buffer, sizeof(buffer)-1, format, ap);
	*ptr= (char*)malloc(sizeof(char)*size);
	ret = vsprintf(*ptr, format, ap);
	va_end(ap);

	return ret;
}
#endif


// return true if found
bool findCharInAMXString(AMX *amx, cell str, char c)
{
	cell *cstr;
	int len;
	bool ret;

	amx_GetAddr(amx, str, &cstr);
	amx_StrLen(cstr, &len);

	char *buff= new char[len+1];
	amx_GetString(buff, cstr, 0);

	ret= (strchr(buff, c)!=NULL);
	delete buff;
	return ret;
}

// allocate memory and copy the string
void copyStringFromAMX(AMX *amx, cell param, char **out)
{
	cell *cstr;
	int len;
	int ret;

	if( (ret= amx_GetAddr(amx, param, &cstr)) != AMX_ERR_NONE)
	{
		//AmxDebug("%samx_GetAddr: <%#x> <%d>%s\n", COLOR_RED, param, amx->curline, COLOR_RESET);
		*out= new char[2];
		*out[1]= ' ';
		*out[0]= '\0';
		return;
	}
	amx_StrLen(cstr, &len);
	*out= new char[len+2];	// ??? (amx_GetString is strange => +1)

	assert(cstr!=0);

	amx_GetString(*out, cstr, 0);
}

void copyStringToAMX(AMX *amx, cell dest, const char *input, int maxsize, int packed)
{
	cell *cptr;
	char *input2= const_cast<char*>(input);

	amx_GetAddr( amx, dest, &cptr);

	if( maxsize==-1 )
	{
		amx_SetString( cptr, input2, 0, 0);
	}
	else
	{
		char *tmp;
		int size= MIN((uint)maxsize-1,strlen(input2));
		tmp= new char[ size + 2 ];
		strncpy(tmp, input2, size);
		tmp[size]= '\0';
		amx_SetString( cptr, tmp, packed, 0);
		delete [] tmp;
	}
}

char buildString_buffer[1024];

/****h* Natives/VarArgFormat
* MODULE DESCRIPTION
*	handling of variable parameters count
*
*	%% : '%' character
*	%d : integer
*	%c : character
*	%x : hexadecimal number
*	%s : string
***/

static int dochar(AMX *amx, char ch, cell param)
{
	cell *cptr;
	char tmp[200];
	char s[3]= "%?";

	amx_GetAddr(amx, param, &cptr);

	switch (ch)
	{
	case '%':
		strlcat(buildString_buffer, "%", sizeof(buildString_buffer));
		return 0;

	case 'c':
	case 'd':
	case 'x':
		s[1]= ch;
		//asprintf(&tmp, s, (long)*cptr);
		snprintf(tmp, sizeof(tmp)-1, s, (long)*cptr);
		strlcat(buildString_buffer, tmp, sizeof(buildString_buffer));
		//free(tmp);
		return 1;

	case 's':
		buildString(amx,cptr,NULL,0);
		return 1;

	}
	/* error in the string format, try to repair */
	strcpy(s, "% ");
	s[1]= ch;
	strlcat(buildString_buffer, s, sizeof(buildString_buffer));
	return 0;
}

/*********************/
/*
static int dochar(AMX *amx,TCHAR ch,cell param,char sign,char decpoint,int width,int digits)
{
  cell *cptr;
  #if defined FLOATPOINT || defined FIXEDPOINT
    TCHAR formatstring[40];
  #endif

  switch (ch) {
  case '%':
    amx_putchar(ch);
    return 0;

  case 'c':
    amx_GetAddr(amx,param,&cptr);
    width--;            // single character itself has a with of 1
    if (sign!='-')
      while (width-->0)
        amx_putchar(__T(' '));
    amx_putchar((TCHAR)*cptr);
    while (width-->0)
      amx_putchar(__T(' '));
    return 1;

  case 'd': {
    cell value;
    int length=1;
    amx_GetAddr(amx,param,&cptr);
    value=*cptr;
    if (value<0 || sign=='+') {
      length++;
      value=-value;
    }
    while (value>=10) {
      length++;
      value/=10;
    }
    width-=length;
    if (sign!='-')
      while (width-->0)
        amx_putchar(__T(' '));
    amx_printf(__T("%ld"),(long)*cptr);
    while (width-->0)
      amx_putchar(__T(' '));
    return 1;
  }

#if defined FLOATPOINT
  case 'f': // 32-bit floating point number
  case 'r': // if floating point is enabled, %r == %f
    // build a format string
    _tcscpy(formatstring,__T("%"));
    if (sign!='\0')
      _stprintf(formatstring+_tcslen(formatstring),__T("%c"),sign);
    if (width>0)
      _stprintf(formatstring+_tcslen(formatstring),__T("%d"),width);
    _stprintf(formatstring+_tcslen(formatstring),__T(".%df"),digits);
    // ??? decimal comma?
    amx_GetAddr(amx,param,&cptr);
    amx_printf(formatstring,*(float*)cptr);
    return 1;
#endif

#if defined FIXEDPOINT
  #define FIXEDMULT 1000
  case 'q': // 32-bit fixed point number
#if !defined FLOATPOINT
  case 'r': // if fixed point is enabled, and floating point is not, %r == %q
#endif
    amx_GetAddr(amx,param,&cptr);
    // format the number
    formatfixed(formatstring,*cptr,sign,decpoint,digits);
    assert(_tcslen(formatstring)<sizeof formatstring);
    amx_printf(__T("%*s"),width,formatstring);
    return 1;
#endif

#if !defined FLOATPOINT && !defined FIXEDPOINT
  case 'f':
  case 'q':
  case 'r':
    amx_printf(__T("(no rational number support)"));
    return 0; // flag this as an error
#endif

  case 's':
    amx_GetAddr(amx,param,&cptr);
    printstring(amx,cptr,NULL,0);
    return 1;

  case 'x': {
    ucell value;
    int length=1;
    amx_GetAddr(amx,param,&cptr);
    value=*(ucell*)cptr;
    while (value>=0x10) {
      length++;
      value>>=4;
    }
    width-=length;
    if (sign!='-')
      while (width-->0)
        amx_putchar(__T(' '));
    amx_printf(__T("%lx"),(long)*cptr);
    while (width-->0)
      amx_putchar(__T(' '));
    return 1;
  }

  }
  // error in the string format, try to repair
  amx_putchar(ch);
  return 0;
}

enum {
  FMT_NONE,   // not in format state; accept '%'
  FMT_START,  // found '%', accept '+', ' ' (START), digit (WIDTH), '.' (DECIM) or format letter (done)
  FMT_WIDTH,  // found digit after '%' or sign, accept digit (WIDTH), '.' (DECIM) or format letter (done)
  FMT_DECIM,  // found digit after '.', accept accept digit (DECIM) or format letter (done)
};

static int formatstate(char c, int *state, char *sign, char *decpoint, int *width,int *digits)
{
	switch( *state )
	{
	case FMT_NONE:
		if( c=='%' )
		{
			*state=FMT_START;
			*sign='\0';
			*decpoint='.';
			*width=0;
			*digits=3;
		}
		else
		{
			return -1;  // print a single character
		}
		break;

	case FMT_START:
		if( c=='+' || c==' ' )
		{
			*sign= ' ';
		}
		else if( c>='0' && c<='9' )
		{
			*width=(int)(c-'0');
			*state=FMT_WIDTH;
		}
		else if( c=='.' || c==',' )
		{
			*decpoint=c;
			*digits=0;
			*state=FMT_DECIM;
		}
		else
		{
			return 1; // print formatted character
		}
		break;

	case FMT_WIDTH:
		if( c>='0' && c<='9' )
		{
			*width= *width*10+(int)(c-'0');
		}
		else if( c=='.' || c==',' )
		{
			*decpoint=c;
			*digits=0;
			*state=FMT_DECIM;
		}
		else
		{
			return 1; // print formatted character
		}
		break;

	case FMT_DECIM:
		if( c>='0' && c<='9' )
		{
			*digits=*digits*10+(int)(c-'0');
		}
		else
		{
			return 1; // print formatted character
		}
		break;

	}

	return 0;
}
*/
/*********************/

// parse string for %s, %d, %x, %c, %%
int buildString(AMX *amx, cell *cstr, cell *params, int num)
{
	int i= 0;
	int informat= 0,paramidx= 0;

	assert(cstr!=NULL);

	if( params!=NULL )
		buildString_buffer[0]= '\0';
		//memset(buffer, '\0', sizeof(buffer));

	/* check whether this is a packed string */
	if( (ucell)*cstr>UCHAR_MAX )
	{
		int j= sizeof(cell)-sizeof(char);
		char c;
		/* the string is packed */

		while(1)
		{
			c=(char)((ucell)cstr[i] >> 8*j);
			if( c==0 )
				break;

			if (informat)
			{
				assert(params!=NULL);
				paramidx+= dochar(amx, c, params[paramidx]);
				informat= 0;
			}
			else if (params!=NULL && c=='%')
			{
				informat= 1;
			}
			else
			{
				char s[2];
				s[0]= c;
				s[1]= '\0';
				strlcat(buildString_buffer, s, sizeof(buildString_buffer));
			}

			if( j==0 )
				i++;

			j= (j+sizeof(cell)-sizeof(char)) % sizeof(cell);
		}
	}
	else
	{
		/* the string is unpacked */
		for(; cstr[i]!=0; i++)
		{
			if( informat )
			{
				assert(params!=NULL);
				paramidx+=dochar(amx, (char)cstr[i], params[paramidx]);
				informat=0;
			}
			else if (params!=NULL && (char)cstr[i]=='%')
			{
				if( (paramidx<num) || (num==0) )
				  informat=1;
				else
				  amx_RaiseError(amx, AMX_ERR_NATIVE);
			}
			else
			{
				char s[2];
				s[0]= (char)cstr[i];
				s[1]= '\0';
				strlcat(buildString_buffer, s, sizeof(buildString_buffer));
			}
		}
	}
	return paramidx;
}


// for MySQL datetime 	(2003-12-20 18:52:00)
// or DATE 				(2003-12-03)
int parseMySQLDateTime(string datetime_str, struct tm &ret, bool parse_time /*= true*/)
{
	//string tmp[6];
	vector<string> tmp;
	time_t now= time(NULL);
	struct tm *tm2= localtime(&now);

	memcpy(&ret, tm2, sizeof(struct tm));

	Tokenize(datetime_str, tmp, "-: ");
	if( (tmp.size() != 3) && (tmp.size() != 6) )
	{
		//Error("Wrong date format: %s (r=%d)\n", datetime_str.c_str(), tmp.size());
		return -1;
	}

	// no time info
	if( tmp.size() == 3 )
		parse_time= false;

	ret.tm_mday= 	atoi( tmp[2].c_str() );
	ret.tm_mon= 	atoi( tmp[1].c_str() ) - 1;
	ret.tm_year= 	atoi( tmp[0].c_str() ) - 1900;
	ret.tm_sec= 	0;

	if( parse_time )
	{
		ret.tm_hour= atoi( tmp[3].c_str() );
		ret.tm_min= atoi( tmp[4].c_str() );
	}
	else
	{
		ret.tm_hour= 0;
		ret.tm_min= 0;
	}

	return mktime(&ret);
}

/**/

