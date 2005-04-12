/*  Simple terminal for Microsoft Windows
 *
 *  Copyright (c) ITB CompuPhase, 2004
 *  This file may be freely used. No warranties of any kind.
 *
 *  Version: $Id: termwin.h,v 1.0 2004-02-17 11:15:01+01 thiadmer Exp thiadmer $
 */

#if !defined TERMWIN_H_INCLUDED
#define TERMWIN_H_INCLUDED

#if !defined WM_NULL
# include <windows.h>
#endif

#include "thread.h"

#define KEYQUEUE_SIZE   32

typedef struct tagCONSOLE {
	struct tagCONSOLE *next;
	HWND hwnd, hStatus;
	HWND hScrollV;
	char *buffer;                /* text buffer */
	int disp_lines, disp_columns;
	int cur_lines;
	int csrx,csry;                /* cursor position */
	short keyqueue[KEYQUEUE_SIZE];
	int keyq_start,keyq_end;
	BOOL autowrap;
	BOOL boldfont;
	HFONT hfont;
	HBRUSH back;
	int cwidth,cheight;           /* character width and height */
} CONSOLE;

class ConsoleThread : public PThread
{
private:
	static void ScrollScreen(CONSOLE *con, int dx, int dy);
	static long CALLBACK ConsoleFunc(HWND hwnd,unsigned message,WPARAM wParam, LPARAM lParam);
	static bool InitWindowClass(HINSTANCE hinst);
	static HWND CreateConsole(HINSTANCE hinst,HWND hwndParent,int columns,int lines,int fontsize);
	static CONSOLE *ActiveConsole();
	static void updateScrollbars(CONSOLE *);

	// called when outside of the window thread
	static void setStatusBarPartCountFromExt(int n);

	static void setStatusBarPartCount(CONSOLE *con, int n);
	static void setStatusBarText(CONSOLE *con, int idx, const char *txt);


	static bool stop;
	static int status_num;

public:
	~ConsoleThread();
	void user_func();
	static int amx_printf(const char*,...);
	static void DoDeleteConsole(CONSOLE *con);
	static void amx_clrscr(void);

	static void setStatusBarTextFromExt(int idx, const char *txt);
};

//HWND CreateConsole(HINSTANCE hinst,HWND hwndParent,int columns,int lines,int fontsize,DWORD style);
BOOL SetActiveConsole(HWND hconsole);
//BOOL DeleteConsole(HWND hconsole);
HWND GetConsoleByIndex(int index);


//int      amx_putchar(int);
//int      amx_fflush(void);
//int      amx_kbhit(void);
//int      amx_getch(void);
//char*    amx_gets(TCHAR*,int);
//int      amx_termctl(int,int);
//void     amx_clreol(void);
//void     amx_gotoxy(int x,int y);
//void     amx_wherexy(int *x,int *y);
//unsigned int amx_setattr(int foregr,int backgr,int highlight){ return 0; };

#endif /* TERMWIN_H_INCLUDED */
