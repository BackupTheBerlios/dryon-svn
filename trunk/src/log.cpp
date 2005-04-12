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
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include "log.h"


Logs::Logs(char *stream_name, char *c, char *fname) : disp(false),enabled(true)
{
	strncpy(filename, fname, sizeof(filename)-1);
	snprintf(id, sizeof(id)-1, stream_name);
	color= c;
}

Logs::~Logs()
{
	//printf("Logs::~Logs(%s)\n", id);
}

// do nothing (when logging disabled)
void Logs::DummyWrite(char *format, ...)
{

}

void Logs::Write(char *format, ...)
{
	if( enabled )
	{
		va_list va;
		char buff[1024], date[30];
		FILE *file;
		file= fopen(filename, "a");

		if( file != NULL )
		{
			va_start(va, format);
			vsnprintf(buff, sizeof(buff)-1, format, va);
			va_end(va);

			time_t now= time(NULL);
			strftime(date, sizeof(date)-1, "%H:%M:%S", localtime(&now));

			Output("%s[%s]<%s>: %s%s", color, date, id, buff, COLOR_RESET);

			//file << "[" << date << "] " << buff;
			//file.close();
			fprintf(file, "[%s] %s", date, buff);
			fclose(file);
		}
		else
		{
			Output("failed to open %s\n", filename);
		}
	}
}

/**/

