/*  Simple terminal for Microsoft Windows
 *
 *  Copyright (c) ITB CompuPhase, 2004
 *  This file may be freely used. No warranties of any kind.
 *
 *  Version: $Id: termwin.c,v 1.2 2004-02-20 15:36:30+01 thiadmer Exp thiadmer $
 *
 *	Heavily edited by Anthalir
 */

#include "config.h"
#include <string.h>

#include <windows.h>
#include <commctrl.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "termwin.h"
#include "dryon.h"
#include "../MSVC/resource.h"

extern DryonBot bot;
extern ConsoleThread console;
extern bool test_mode;
extern DWORD main_thread;
extern bool hidden_launch;

#define UWM_SYSTRAY		21

#define DEFLINES        30
#define DEFCOLUMNS      120
#define DEFFONTHEIGHT   14
#define BUFFERSIZE      (DEFLINES*DEFCOLUMNS*2)

#define IDM_SCROLLBAR	250

struct refresh_line {
	int from;
	int to;
};

static CONSOLE consoleroot={ NULL };
static HWND activeconsole=NULL;
static HWND dummy_win= NULL;
static HINSTANCE ghinst= NULL;
static bool state_hidden= false;

#if defined __WIN32__ || defined _WIN32 || defined WIN32
  #define EXPORT
#else
  #define EXPORT  _export
#endif

long CALLBACK EXPORT ConsoleFunc(HWND hwnd,unsigned message,WPARAM wParam, LPARAM lParam);

bool ConsoleThread::stop= false;
int  ConsoleThread::status_num= 0;

ConsoleThread::~ConsoleThread()
{
	requestEnd();
	waitEnd();
}

bool ConsoleThread::InitWindowClass(HINSTANCE hinst)
{
	static bool initok= false;
	if( !initok )
	{
		/*
		WNDCLASS wc;
		memset(&wc, 0, sizeof wc);
		wc.style= CS_GLOBALCLASS;
		wc.lpfnWndProc= (WNDPROC)ConsoleFunc;
		wc.hInstance= hinst;
		wc.hCursor= LoadCursor(NULL, IDC_ARROW);
		//wc.hbrBackground= (HBRUSH)(COLOR_BACKGROUND);
		wc.hbrBackground= (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszClassName= "TermWin:Console";
		initok= RegisterClass(&wc);
		*/

		WNDCLASSEX wc;
		memset(&wc, 0, sizeof(wc));
		wc.cbSize= sizeof(WNDCLASSEX);
		wc.lpfnWndProc= (WNDPROC)ConsoleFunc;
		wc.hInstance= hinst;
		wc.hIcon= LoadIcon(hinst, MAKEINTRESOURCE(IDI_SYSTRAY));
		wc.hCursor= LoadCursor(hinst, IDC_ARROW);
		wc.hbrBackground= (HBRUSH)GetStockObject(BLACK_BRUSH);
		wc.lpszClassName= "TermWin:Console";
		wc.hIconSm= (HICON)LoadImage(hinst, MAKEINTRESOURCE(IDI_SYSTRAY), IMAGE_ICON,
						GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);

		initok= RegisterClassEx(&wc);

	}
	return initok;
}

static CONSOLE *Hwnd2Console(HWND hwnd)
{
	CONSOLE *con;
	for(con= consoleroot.next; con!= NULL && con->hwnd!=hwnd; con= con->next){ /* nothing */}
	return con;
}

void ConsoleThread::DoDeleteConsole(CONSOLE *con)
{
	CONSOLE *prev;

	assert(con!=NULL);

	// unlink first, to avoid a recursive delete when destroying the window
	for(prev= &consoleroot; prev->next!= NULL && prev->next!=con; prev= prev->next){}
	if( prev->next==con )
		prev->next=con->next;

	// free memory and resources
	if (con->hwnd!=NULL && IsWindow(con->hwnd))
		DestroyWindow(con->hwnd);

	if (con->hfont!=0)
		DeleteObject(con->hfont);

	if (con->buffer!=NULL)
		delete [] con->buffer;

	delete con;
}

HWND ConsoleThread::CreateConsole(HINSTANCE hinst,HWND hwndParent,int columns,int lines,int fontsize)
{
	NOTIFYICONDATA nid;
	CONSOLE *con, *prev;

	InitCommonControls();
	ghinst= hinst;

	/* allocate a structure to hold the information */
	con= new CONSOLE;
	if( con==NULL )
		return NULL;

	memset(con, 0, sizeof(CONSOLE));

	/* link to the list */
	for(prev= &consoleroot; prev->next!= NULL; prev= prev->next){/* nothing */}
	prev->next= con;

	/* fill in information */
	con->disp_lines= lines;
	con->cur_lines= 0;
	con->disp_columns= columns;
	con->cheight= fontsize;
	con->autowrap= TRUE;
	con->buffer= new char[(lines*4)*columns];

	if (con->buffer!=NULL)
		memset(con->buffer, ' ', (lines*4)*columns);

	if( hinst==NULL )
		hinst= GetModuleHandle(NULL);

	DWORD	style= WS_THICKFRAME | WS_CAPTION | WS_CLIPCHILDREN | WS_CLIPSIBLINGS  | WS_SYSMENU  | /*WS_VSCROLL |*/ WS_SIZEBOX /*| WS_MINIMIZEBOX*/;
	if( !hidden_launch )
		style|= WS_VISIBLE;

	state_hidden= hidden_launch;

	// hidden windnow (to hide button in taskbar)
	dummy_win= CreateWindow("BUTTON", "", 0,0,0,0,0, hwndParent, NULL, hinst, NULL);

	/* create the window */
	InitWindowClass(hinst);
	//con->hwnd= CreateWindow( "TermWin:Console", "Dryon console", style, 0,0,0,0, hwndParent, NULL, hinst, NULL);
	con->hwnd= CreateWindowEx(0, "TermWin:Console", "Dryon Console", style, 0,0,0,0, dummy_win, NULL, hinst, 0);

	memset(&nid, 0, sizeof(nid));
	nid.cbSize= sizeof(NOTIFYICONDATA);
	nid.hWnd= con->hwnd;
	nid.uID= 1;
	nid.uFlags= NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nid.uCallbackMessage= UWM_SYSTRAY;
	nid.hIcon= (HICON)LoadImage(hinst, MAKEINTRESOURCE(IDI_SYSTRAY), IMAGE_ICON,
					GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON), 0);
	strcpy(nid.szTip, "Dryon");
	if( !Shell_NotifyIcon(NIM_ADD, &nid) )
		MessageBox(con->hwnd, "Unable to display systray icon", "Error", MB_OK);


	con->hStatus= CreateWindowEx(0, STATUSCLASSNAME, NULL, WS_CHILD|SBARS_SIZEGRIP, 0,0,200,20, con->hwnd, NULL, hinst, NULL);
	con->hScrollV= CreateWindow("SCROLLBAR", "scrollV", WS_CHILD|WS_VISIBLE|SBS_VERT|SBS_RIGHTALIGN , 50,0,20,100, con->hwnd, NULL, hinst, NULL);

	status_num= 1;
/*
	setStatusBarPartCount(con, 4);
	setStatusBarText(con, 0, "1st");
	setStatusBarText(con, 1, "2nd");
	setStatusBarText(con, 2, "3rd");
	setStatusBarText(con, 3, "4th");
*/
	ShowWindow(con->hStatus, SW_SHOW);

	/* check whether all is ok */
	if( IsWindow(con->hwnd) && con->buffer!=NULL )
		return con->hwnd;
	/* when we arrive here, something was initialized correctly */
	DoDeleteConsole(con);
	return NULL;
}
/*
BOOL DeleteConsole(HWND hconsole)
{
	CONSOLE *con= Hwnd2Console(hconsole);
	if( con!=NULL )
		DoDeleteConsole(con);

	return con!=NULL;
}
*/
BOOL SetActiveConsole(HWND hconsole)
{
	CONSOLE *con= Hwnd2Console(hconsole);
	if( con!=NULL )
		activeconsole=con->hwnd;

	return con!=NULL;
}

HWND GetConsoleByIndex(int index)
{
	CONSOLE *con;

	for(con= consoleroot.next; con!=NULL && index>0; con= con->next)
		index--;

	if( con!=NULL )
		return con->hwnd;

	return NULL;
}

CONSOLE *ConsoleThread::ActiveConsole()
{
	CONSOLE *con;

	for(con= consoleroot.next; con!=NULL && con->hwnd!=activeconsole; con= con->next){/* nothing */}

	if( con==NULL )
	{      /* active console "disappeared", switch to first console */
		/* create a console if there are none left */
		if( consoleroot.next==NULL )
			CreateConsole(NULL, HWND_DESKTOP, DEFCOLUMNS, DEFLINES, DEFFONTHEIGHT);
		con= consoleroot.next;
		activeconsole= (con!=NULL) ? con->hwnd : NULL;
	}
	/* if "con" still is NULL here, then the following holds:
	* 1. the "active console" (if there was one) disappeared
	* 2. there are no consoles left at all
	* 3. a new console could not be created
	*/
	return con;
}

static void SetConsoleFont(CONSOLE *con,int height)
{
	static char *ConsoleFonts[] = {
	 "Andante",
	 "ProggyClean",
	 "Bitstream Vera Sans Mono",
	 "Lucida Console",
	 "Monaco",
	 "Andale Mono",
	 "Courier New",
	 "FixedSys",
	NULL };
	HDC hdc;
	SIZE size;
	HFONT hfontOrg;
	int weight, index;

	assert(con!=NULL);

	/* remove the existing font (if any) */
	if( con->hfont!=0 )
		DeleteObject(con->hfont);

	/* make a new font */
	weight= con->boldfont ? FW_BOLD : FW_NORMAL;
	index= 0;
	do
	{
		con->hfont=CreateFont(height, 0, 0, 0, weight, FALSE, 0, 0, ANSI_CHARSET,
		                      OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY,
		                      FIXED_PITCH|FF_DONTCARE, ConsoleFonts[index]);
	}
	while(con->hfont==NULL && ConsoleFonts[++index]!=NULL );
	/* get character size */
	hdc= GetDC(con->hwnd);
	hfontOrg= (HFONT)SelectObject(hdc, con->hfont);
	GetTextExtentPoint32(hdc, "x", 1, &size);
	SelectObject(hdc, hfontOrg);
	ReleaseDC(con->hwnd, hdc);
	con->cwidth= (int)size.cx;
	con->cheight= (int)size.cy;
}

static void ClampToScreen(RECT *rc)
{
	int cx= GetSystemMetrics(SM_CXSCREEN);
	int cy= GetSystemMetrics(SM_CYSCREEN);

	if( rc->left<0 )
		OffsetRect(rc,-rc->left,0);

	if( rc->top<0 )
		OffsetRect(rc,0,-rc->top);

	if( rc->right>cx )
	{
		/* first try to move left */
		OffsetRect(rc, cx-rc->right, 0);
		/* also check not to exceed the left edge */
		if( rc->left<0 )
			rc->left= 0;
	}
	if( rc->bottom>cy )
	{
		/* first try to move up */
		OffsetRect(rc, 0, cy-rc->bottom);
		/* also check not to exceed the top edge */
		if (rc->top<0)
			rc->top= 0;
	} /* if */
}
/*
static void RefreshCaretPos(CONSOLE *con)
{
	assert(con!=NULL);
	assert(IsWindow(con->hwnd));
	SetCaretPos(con->cwidth*con->csrx-GetScrollPos(con->hwnd,SB_HORZ),
		con->cheight*(con->csry+1)-2-GetScrollPos(con->hwnd,SB_VERT));
}
*/

static void RefreshScreen(CONSOLE *con,int startline,int endline)
{
	RECT rect;
	assert(con!=NULL);
	assert(IsWindow(con->hwnd));
	GetClientRect(con->hwnd, &rect);
	rect.top= startline*con->cheight;
	rect.bottom= endline*con->cheight;
	InvalidateRect(con->hwnd, &rect, false);
//	RefreshCaretPos(con);
}

void ConsoleThread::ScrollScreen(CONSOLE *con, int dx, int dy)
{
	int x, y, linesize, pos;

	assert(con!=NULL);
	assert(con->buffer!=NULL);

	/* vertical scrolling */
	if( dy!=0 )
	{
		linesize= con->disp_columns;
		/* a positive value scrolls up */
		if( dy>=0 )
		{
			for(y= 0; y< con->disp_lines; y++)
				memcpy(con->buffer+y*linesize, con->buffer+(y+1)*linesize, linesize*sizeof(char));

			pos= (con->disp_lines-1)*con->disp_columns;
			for(x= 0; x< con->disp_columns; x++)
				con->buffer[pos+x]= ' ';
		}
		else
		{
			for(y= con->disp_lines-1; y> 0; y++)
				memcpy(con->buffer+y*linesize, con->buffer+(y-1)*linesize, linesize*sizeof(char));

			pos= (con->disp_lines-1)*con->disp_columns;
			for(x= 0; x< con->disp_columns; x++)
				con->buffer[pos+x]= ' ';
		}
		con->csry-= dy;
		if( con->csry< 0 )
		  con->csry= 0;
/*
		if( con->csry >= con->disp_lines )
		  con->csry= con->disp_lines-1;
*/
	}

	/* horizontal scrolling */
	/* ??? to be implemented */
/*
	refresh_line *rl= new refresh_line;
	rl->from= 0;
	rl->to= con->disp_lines;
	PostThreadMessage(console.tid, WM_USER, 0, (long)rl);
*/
}

void ConsoleThread::updateScrollbars(CONSOLE *con)
{
	int ry= (con->cur_lines > con->disp_lines) ? con->cur_lines-con->disp_lines : 0;

	// adjust scrolling position, if necessary
	if( GetScrollPos(con->hScrollV, SB_CTL) > ry )
	{
		SetScrollPos(con->hScrollV, SB_CTL, ry, FALSE);
		InvalidateRect(con->hScrollV, NULL, FALSE);
	}

	SetScrollRange(con->hScrollV, SB_CTL, 0, ry, TRUE);
}

long CALLBACK ConsoleThread::ConsoleFunc(HWND hwnd,unsigned message,WPARAM wParam, LPARAM lParam)
{
	POINT pt;
	HMENU hmenu, hpopup;
	NOTIFYICONDATA nid;
	CONSOLE *con= NULL;
	PAINTSTRUCT ps;
	RECT rect= {0,0,0,0}/*, rect2= {0,0,0,0}*/;

	memset(&nid, 0, sizeof(nid));

	switch (message)
	{
	case WM_CLOSE:
		if( !test_mode )
			bot.quit("window closed");
		stop= true;
		break;

	case WM_DESTROY:
		nid.cbSize= sizeof(NOTIFYICONDATA);
		nid.hWnd= hwnd;
		nid.uID= 1;
		Shell_NotifyIcon(NIM_DELETE, &nid);
		if( (con=Hwnd2Console(hwnd))!=NULL )
			DoDeleteConsole(con);
		break;

/*
	case WM_MOVE:
		if ((con=Hwnd2Console(NULL))!=NULL)
		{
			SendMessage(con->sbar, message, wParam, lParam);
		}
		break;
*/

	case WM_CREATE:
		/* The "hwnd" member of the CONSOLE structure has not yet been set, which
		 * means that Hwnd2Console() cannot work on the real "hwnd". There should
		 * at every instant be only one CONSOLE structure with a NULL handle,
		 * however.
		 */
		if( (con=Hwnd2Console(NULL)) != NULL )
		{
			int screenw= GetSystemMetrics(SM_CXSCREEN);
			int screenh= GetSystemMetrics(SM_CYSCREEN);

			SetConsoleFont(con, con->cheight);

			int win_w= con->cwidth*con->disp_columns;
			int win_h= con->cheight*con->disp_lines;

			rect.left= (screenw - win_w)/2;
			rect.top= (screenh - win_h)/2;
			rect.right= rect.left + win_w;
			rect.bottom= rect.top + win_h;
			rect.bottom+= 50/* status bar */;
			AdjustWindowRect(&rect, GetWindowLong(hwnd, GWL_STYLE), false);


			ClampToScreen(&rect);
			SetWindowPos(hwnd, NULL, rect.left ,rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOZORDER);
		}
		return 0;

	case WM_RBUTTONUP:

		GetCursorPos(&pt);
		hmenu= LoadMenu(ghinst, MAKEINTRESOURCE(IDR_MENU1));
		hpopup= GetSubMenu(hmenu, 0);
		
		switch( TrackPopupMenu(hpopup, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL) )
		{
		case IDM_HIDESHOW:
			ShowWindow(hwnd, SW_HIDE);
			state_hidden= true;
			break;
		}
		break;

		DestroyMenu(hmenu);
		break;


	// systray icon
	case UWM_SYSTRAY:
		switch( lParam )
		{
		case WM_LBUTTONUP:
			if( !state_hidden )
				SetForegroundWindow(hwnd);
			break;

		case WM_RBUTTONUP:
			GetCursorPos(&pt);
			hmenu= LoadMenu(ghinst, MAKEINTRESOURCE(IDR_MENU1));
			hpopup= GetSubMenu(hmenu, 0);

			if( state_hidden )
				ModifyMenu(hpopup, 0, MF_BYPOSITION | MF_STRING, IDM_HIDESHOW, "Show");
			
			switch( TrackPopupMenu(hpopup, TPM_RETURNCMD | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL) )
			{
			case IDM_HIDESHOW:
				if( state_hidden )
				{
					ShowWindow(hwnd, SW_SHOW);
					state_hidden= false;
				}
				else
				{
					ShowWindow(hwnd, SW_HIDE);
					state_hidden= true;
				}
				break;
			}
			break;

			DestroyMenu(hmenu);
		}
		break;

/*
	r= red
	g= green
	b= blue
	y= yellow
	m= magenta
	c= cyan

	z= reset

*/

	case WM_PAINT:
//		HideCaret(hwnd);
		BeginPaint(hwnd, &ps);
		if( (con=Hwnd2Console(hwnd))!=NULL && con->buffer!=NULL )
		{
			int l;
			HFONT hfontOrg;
			int scrolly= GetScrollPos(con->hScrollV, SB_CTL);
			GetClientRect(hwnd, &rect);

			rect.right-= 18;

			FillRect(ps.hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
			updateScrollbars(con);

			hfontOrg= (HFONT)SelectObject(ps.hdc,con->hfont);
			SetBkMode(ps.hdc, TRANSPARENT);

			int inc= 0;
			//int pos= 0;

			SetTextColor(ps.hdc, RGB(255,255,255));

			for(l= scrolly; l< scrolly+con->disp_lines; l++)
			{
				const char *string= &con->buffer[l*con->disp_columns];


				//SetBkColor(ps.hdc, RGB(0,0,0));

				// \003r
				int c= 0, start= 0, dpos= 0;
				for(c= 0; c< con->disp_columns; c++)
				{
					if( inc == 1 )
					{
						switch(string[c])
						{
						case 'b': SetTextColor(ps.hdc, RGB(0,0,255));		break;
						case 'c': SetTextColor(ps.hdc, RGB(0,255,255));		break;
						case 'm': SetTextColor(ps.hdc, RGB(255,0,255));		break;
						case 'r': SetTextColor(ps.hdc, RGB(255,0,0));		break;
						case 'g': SetTextColor(ps.hdc, RGB(0,255,0));		break;
						case 'y': SetTextColor(ps.hdc, RGB(255,255,0));		break;
						case 'z': SetTextColor(ps.hdc, RGB(255,255,255));	break;
						default: break;
						}

						inc= 0;
						start= c+1;
					}
					else if(string[c] == '\033')
					{
						if( c-start > 0 )
						{
							TextOut(ps.hdc, dpos*con->cwidth, (l-scrolly)*con->cheight, &string[start], c-start);
							dpos+= c-start;
						}
						inc= 1;
					}
				}
				TextOut(ps.hdc, dpos*con->cwidth, (l-scrolly)*con->cheight, &string[start], c-start);
			}
			SelectObject(ps.hdc,hfontOrg);
		}
		EndPaint(hwnd, &ps);
		// status bar
		SendMessage(con->hStatus, message, wParam, lParam);
		//ShowCaret(hwnd);
		break;

	case WM_PARENTNOTIFY:
		con= Hwnd2Console(hwnd);
		if( con!=NULL )
		{
			char tmp[200];
			GetWindowText((HWND)lParam, tmp, 199);

			if( LOWORD(wParam) == WM_CREATE )
			{
				if( strcmp(tmp, "scrollV") == 0 )
				{
					GetClientRect(hwnd, &rect);
					SetWindowPos((HWND)lParam, NULL, rect.right-18, rect.top, 18, rect.bottom-20, SWP_NOZORDER);
					SetScrollRange((HWND)lParam, SB_CTL, 0, 0, TRUE);
				}
			}
		}
		return 0;

	case WM_SIZING:
		return 0;

	case WM_SIZE:
		con=Hwnd2Console(hwnd);
		if( con!=NULL )
		{
			if( wParam == SIZE_MINIMIZED )
			{
				ShowWindow(con->hwnd, SW_HIDE);
				state_hidden= true;
			}
			else
			{

				GetClientRect(hwnd, &rect);
				con->disp_lines= rect.bottom / con->cheight - 1;

				SetWindowPos(con->hScrollV, NULL, rect.right-18, rect.top, 18, rect.bottom-20, SWP_NOZORDER);
				updateScrollbars(con);

				int newpos= con->cur_lines - con->disp_lines;
				if( con->cur_lines+1 > con->disp_lines )
				{
					SetScrollPos(con->hScrollV, SB_CTL, newpos, TRUE);
					InvalidateRect(hwnd, NULL, FALSE);
				}


				// status bar
				SendMessage(con->hStatus, WM_SIZE, 0, 0);
				setStatusBarPartCount(con, status_num);
			}
		}
		return 0;

/*
	case WM_HSCROLL:
	{
		int scrollx= GetScrollPos(hwnd, SB_HORZ);
		int oldpos= scrollx;
		int min, max;
		GetScrollRange(hwnd, SB_HORZ, &min, &max);

		switch( LOWORD(wParam) )
		{
		case SB_TOP: 		scrollx= min; 	break;
		case SB_BOTTOM: 	scrollx= max;	break;
		case SB_LINELEFT:	scrollx= (scrollx>min) ? scrollx-1 : min; 	break;
		case SB_LINERIGHT:	scrollx= (scrollx<max) ? scrollx+1 : max;	break;
		case SB_PAGELEFT:	scrollx= (scrollx>min) ? scrollx-50 : min;	break;
		case SB_PAGERIGHT:	scrollx= (scrollx<max) ? scrollx+50 : max;	break;
		case SB_THUMBTRACK:	scrollx= (int)HIWORD(wParam);	break;
		}

		if( oldpos!=scrollx )
		{
			RECT rect;
			GetClientRect(hwnd, &rect);
			SetScrollPos(hwnd, SB_HORZ, scrollx, true);
			InvalidateRect(hwnd, &rect, false);
			//RefreshCaretPos(con);
		}
	}
	break;
*/

	case WM_VSCROLL:
	{
		con=Hwnd2Console(hwnd);
		if( con!=NULL )
		{
			int scrolly= GetScrollPos(con->hScrollV, SB_CTL);
			int oldpos= scrolly;
			int min, max;
			GetScrollRange(con->hScrollV, SB_CTL, &min, &max);

			switch( LOWORD(wParam) )
			{
			case SB_TOP: 		scrolly= min;	break;
			case SB_BOTTOM:		scrolly= max;	break;
			case SB_LINELEFT:	scrolly= (scrolly>min) ? scrolly-1 : min;	break;
			case SB_LINERIGHT:	scrolly= (scrolly<max) ? scrolly+1 : max;	break;
			case SB_PAGELEFT:	scrolly= (scrolly>min) ? scrolly-50 : min;	break;
			case SB_PAGERIGHT:	scrolly= (scrolly<max) ? scrolly+50 : max;	break;
			case SB_THUMBTRACK:	scrolly= (int)HIWORD(wParam);				break;
			}

			if( oldpos!=scrolly )
			{
				RECT rect;
				GetClientRect(hwnd, &rect);
				SetScrollPos(con->hScrollV, SB_CTL, scrolly, true);
				InvalidateRect(hwnd, &rect, false);
				//RefreshCaretPos(con);
			}
		}
	}
	break;

	case WM_GETMINMAXINFO:
		if( (con=Hwnd2Console(hwnd))!=NULL )
		{
			LPMINMAXINFO lpmmi= (LPMINMAXINFO)lParam;
			int min, max, hsize, vsize= 0;

			GetScrollRange(con->hScrollV, SB_CTL, &min, &max);
			hsize= (max>0) ? GetSystemMetrics(SM_CXVSCROLL) : 0;

			//GetScrollRange(hwnd, SB_HORZ, &min, &max);
			//vsize= (max>0) ? GetSystemMetrics(SM_CYHSCROLL) : 0;

			rect.left= 	 0;
			rect.top= 	 0;
			rect.right=  con->cwidth*con->disp_columns+hsize;
			rect.bottom= con->cheight*con->disp_lines+vsize;

			//SetRect(&rect, 0, 0, con->cwidth*con->disp_columns+hsize,con->cheight*con->disp_lines+vsize);
			AdjustWindowRect(&rect, GetWindowLong(hwnd,GWL_STYLE), false);
			//lpmmi->ptMaxTrackSize.x= rect.right - rect.left;
			//lpmmi->ptMaxTrackSize.y= rect.bottom - rect.top;
			lpmmi->ptMaxSize= lpmmi->ptMaxTrackSize;
		}
		break;
/*
	// character is typed on keyboard
	case WM_CHAR:
		if( (con=Hwnd2Console(hwnd))!=NULL )
		{
			// store in a key queue
			if((con->keyq_end+1)%KEYQUEUE_SIZE==con->keyq_start)
			{
				MessageBeep(MB_OK);
				break;
			}
			con->keyqueue[con->keyq_end]= (short)wParam;
			con->keyq_end= (con->keyq_end+1)%KEYQUEUE_SIZE;
		}
		break;
*/
	case WM_SYSKEYDOWN:
	case WM_KEYDOWN:
		if( (con=Hwnd2Console(hwnd))!=NULL )
		{
			char str[20];
			int i;
			str[0]= '\0';

			switch( LOWORD(wParam) )
			{
				case VK_ESCAPE:
					//PostMessage(hwnd, WM_CLOSE, 0, 0);
					if( !test_mode )
						bot.quit("escape");
					return 0;

				case VK_F1:
				case VK_F2:
				case VK_F3:
				case VK_F4:
				case VK_F5:
				case VK_F6:
				case VK_F7:
				case VK_F8:
				case VK_F9:
				case VK_F10:
				case VK_F11:
				case VK_F12:
					if( LOWORD(wParam)<=VK_F5 )
						sprintf(str, "\033[%d~\n", LOWORD(wParam)-VK_F1+11);
					else if( LOWORD(wParam)==VK_F10 )
						sprintf(str, "\033[%d~\n", LOWORD(wParam)-VK_F6+17);
					else
						sprintf(str, "\033[%d~\n", LOWORD(wParam)-VK_F11+23);
					break;

				case VK_ADD:
				case VK_SUBTRACT:
					/* check Ctrl key */
					if( (GetKeyState(VK_CONTROL) & 0x8000)!=0 )
					{
						POINT pt;
						int newheight= con->cheight;
						int oldheight= newheight;
						int incr= (LOWORD(wParam)==VK_SUBTRACT) ? -1 : 1;
						do
						{
							newheight+= incr;
							/* make a new font, re-create a caret and redraw everything */
							SetConsoleFont(con, newheight);
						}
						while( newheight>5 && (oldheight==con->cheight || con->hfont==NULL) );

						if( con->hfont==NULL ) /* reset to original on failure */
							SetConsoleFont(con,oldheight);

						GetClientRect(hwnd, &rect);
						//DestroyCaret();
						//CreateCaret(hwnd, NULL, con->cwidth, 2);
						//RefreshCaretPos(con);
						/* redraw the window */
						InvalidateRect(hwnd,NULL,TRUE);
						/* resize the window */
						rect.left= 		0;
						rect.top= 		0;
						rect.right= 	con->cwidth*con->disp_columns;
						rect.bottom= 	con->cheight*con->disp_lines;

						//SetRect(&rect,0,0,con->cwidth*con->disp_columns,con->cheight*con->disp_lines);
						AdjustWindowRect(&rect, GetWindowLong(hwnd,GWL_STYLE), false);
						pt.x= pt.y= 0;
						ClientToScreen(hwnd, &pt);
						OffsetRect(&rect, pt.x, pt.y);
						ClampToScreen(&rect);
						SetWindowPos(hwnd, NULL, rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top, SWP_NOZORDER);
					}
					break;

				case VK_UP: 	strcpy(str, "\033[A");	break;
				case VK_DOWN:	strcpy(str, "\033[B");	break;
				case VK_RIGHT:	strcpy(str, "\033[C");	break;
				case VK_LEFT:	strcpy(str, "\033[D");	break;
				case VK_HOME:	strcpy(str, "\033[1~");	break;
				case VK_END:	strcpy(str, "\033[4~");	break;
				case VK_INSERT:	strcpy(str, "\033[2~");	break;
				case VK_DELETE:	strcpy(str, "\033[3~");	break;
				case VK_PRIOR:	strcpy(str, "\033[5~");	break;	/* PageUp */
				case VK_NEXT:	strcpy(str, "\033[6~");	break;	/* PageDown */

				default:
					return DefWindowProc(hwnd,message,wParam,lParam);
			}

			for(i= 0; str[i]!= '\0'; i++)
			{
				if( (con->keyq_end+1)%KEYQUEUE_SIZE!=con->keyq_start )
				{
					con->keyqueue[con->keyq_end]= (short)str[i];
					con->keyq_end= (con->keyq_end+1)%KEYQUEUE_SIZE;
				}
			}
		}
		break;
/*
	case WM_KILLFOCUS:
		//HideCaret(hwnd);
		//DestroyCaret();
		break;

	case WM_SETFOCUS:
		if( (con=Hwnd2Console(hwnd))!=NULL )
		{
			//CreateCaret(hwnd, NULL, con->cwidth, 2);
			//RefreshCaretPos(con);
			//ShowCaret(hwnd);
		}
		break;

	case WM_LBUTTONDOWN:
		SetFocus(hwnd);
		break;
*/
	default:
		break;
	}

	return DefWindowProc(hwnd,message,wParam,lParam);
}

int ConsoleThread::amx_printf(const char *format,...)
{
	int cnt= 0;
	CONSOLE *con;

	if( console.end )
	{
		int n= 2;
		n=+ 1;
	}
	else if( (con=ActiveConsole())!=NULL)
	{
		char buffer[BUFFERSIZE];
		int starty= con->csry;
		va_list argptr;
		int pos, i;

		va_start(argptr,format);
		cnt=vsprintf(buffer,format,argptr);
		va_end(argptr);

		assert(con->buffer!=NULL);

		for(i= 0; buffer[i]!=  '\0'; i++)
		{
			// current cursor position
			pos= con->csry*con->disp_columns + con->csrx;
			if( con->csry >= BUFFERSIZE/con->disp_columns )
			{
				int j;
				//ScrollScreen(con, 0, 1);
				for(j= 0; j< BUFFERSIZE/con->disp_columns; j++)
				{
					memcpy(con->buffer+j*con->disp_columns, con->buffer+(j+1)*con->disp_columns, con->disp_columns);
				}
				con->csry--;
				con->cur_lines--;
				pos= con->csry*con->disp_columns + con->csrx;
				InvalidateRect(con->hwnd, NULL, FALSE);
			}

			if( buffer[i] == '\r' )
			{
				con->csrx= 0;
			}
			else if( buffer[i] == '\n' )
			{
				con->csrx= 0;
				con->csry++;
				con->cur_lines++;
				/*
				if( con->csry >= con->disp_lines )
				{
					con->cur_lines++;
					con->csry--;
					//ScrollScreen(con,0,1);
				}
				*/
			}
			else if( buffer[i] == '\b' )
			{
				if( con->csrx > 0 )
				{
					con->csrx--;
					pos--;
					con->buffer[pos]= ' ';
				}
			}
			else
			{
				con->buffer[pos]= buffer[i];
				pos++;
				con->csrx++;
				if( (con->csrx >= con->disp_columns) && con->autowrap )
				{
					con->csrx= 0;
					con->csry++;
					con->cur_lines++;
					/*
					if( con->csry >= con->disp_lines )
					{
						con->cur_lines++;
						con->csry--;
						//ScrollScreen(con, 0, 1);
					}
					*/
				}
			}
		}

		if( !stop )
		{
			refresh_line *rl= new refresh_line;
			rl->from= starty;
			rl->to= con->csry;
			PostThreadMessage(console.tid, WM_USER, 0, (long)rl);
			PostThreadMessage(console.tid, WM_USER, 1, con->cur_lines);
		}
		else
		{
			int n= 2;
			n+= 5;
		}
	}
	return cnt;
}

/*
int ConsoleThread::amx_printf(const char *format,...)
{
	int cnt= 0;
	CONSOLE *con;
	if ((con=ActiveConsole())!=NULL)
	{
		char buffer[BUFFERSIZE];
		int starty= con->csry;
		va_list argptr;
		int pos, i;

		va_start(argptr,format);
		cnt=vsprintf(buffer,format,argptr);
		va_end(argptr);

		// current cursor position
		pos= con->csry*con->disp_columns + con->csrx;

		assert(con->buffer!=NULL);

		for(i= 0; buffer[i]!=  '\0'; i++)
		{
			if( (con->csry < con->disp_lines) && (con->csrx < con->disp_columns) )
			{
				if( buffer[i] == '\r' )
				{
					con->csrx= 0;
					pos= con->csry*con->disp_columns + con->csrx;
				}
				else if( buffer[i] == '\n' )
				{
					con->csrx= 0;
					con->csry++;
					if( con->csry >= con->disp_lines )
					{
						con->cur_lines++;
						con->csry--;
						//ScrollScreen(con,0,1);
					}
					pos= con->csry*con->disp_columns + con->csrx;
				}
				else if( buffer[i] == '\b' )
				{
					if( con->csrx > 0 )
					{
						con->csrx--;
						pos--;
						con->buffer[pos]= ' ';
					}
				}
				else
				{
					con->buffer[pos]= buffer[i];
					pos++;
					con->csrx++;
					if( (con->csrx >= con->disp_columns) && con->autowrap )
					{
						con->csrx= 0;
						con->csry++;
						if( con->csry >= con->disp_lines )
						{
							con->cur_lines++;
							con->csry--;
							//ScrollScreen(con, 0, 1);
						}
						pos= con->csry*con->disp_columns + con->csrx;
					}
				}
			}
		}

		refresh_line *rl= new refresh_line;
		rl->from= starty;
		rl->to= con->csry;
		PostThreadMessage(console.tid, WM_USER, 0, (long)rl);
	}
	return cnt;
}
*/
/*
int amx_kbhit(void)
{
	CONSOLE *con;

	if( (con=ActiveConsole())!=NULL )
		return con->keyq_start!=con->keyq_end;

	return 0;
}

int amx_getch(void)
{
	CONSOLE *con;
	int c=-1;

	if ((con=ActiveConsole())!=NULL)
	{
		MSG msg;
		while (con->keyq_start==con->keyq_end)
		{
			while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		c= con->keyqueue[con->keyq_start];
		con->keyq_start=(con->keyq_start+1)%KEYQUEUE_SIZE;
	}
	return c;
}
*/
/*
TCHAR *amx_gets(char *string,int size)
{
	int c=-1,num=0;

	if (ActiveConsole()!=NULL)
	{
		while (num+1<size && !(c== '\r' || c== '\n'))
		{
			c=amx_getch();
			if (c<0)
			break;
			if (c== '\r')
			c= '\n';    // translate carriage return to newline
			if (c== '\b')
			{
				if (num>0)
				  string[--num]= ' ';
			}
			else
			{
				string[num++]=(char)c;
			}
			amx_putchar(c);   // echo the character read
		}
		if (num<size)
		  string[num]= '\0';
		return string;
	}
	return 0;
}
*/
/*
int amx_termctl(int cmd,int value)
{
	switch( cmd )
	{
	case 0:
		return 1;           // simple "is terminal support available" check

	case 1:
	{
		CONSOLE *con;
		if( (con= ActiveConsole())!=NULL )
		{
			con->autowrap= (BOOL)value;
			return 1;
		}
		return 0;
	}

	case 2:
	{
		HWND hconsole= GetConsoleByIndex(value);
		while( hconsole==NULL )
		{
			CreateConsole(NULL, HWND_DESKTOP, DEFCOLUMNS, DEFLINES, DEFFONTHEIGHT, 0);
			hconsole= GetConsoleByIndex(value);
		}
		return SetActiveConsole(hconsole);
	}

	case 3:
	{
		CONSOLE *con;
		if( (con=ActiveConsole())!=NULL )
		{
			con->boldfont= (BOOL)value;
			SetConsoleFont(con, con->cheight);
			return 1;
		}
		return 0;
	}

	case 4:
	{
		MSG msg;
		while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		return (GetConsoleByIndex(value)!=NULL);
	}

	default:
		return 0;
	}
}
*/

void ConsoleThread::amx_clrscr(void)
{
	CONSOLE *con;
	if((con=ActiveConsole())!=NULL)
	{
		int i;
		int size= con->disp_lines*con->disp_columns;
		assert(con->buffer!=NULL);
		for(i= 0; i< size; i++)
			con->buffer[i]= ' ';

		con->csrx= con->csry= 0;

		refresh_line *rl= new refresh_line;
		rl->from= 0;
		rl->to= con->disp_lines;
		PostThreadMessage(console.tid, WM_USER, 0, (long)rl);
		//RefreshScreen(con, 0, con->disp_lines);
	}
}
/*
void amx_clreol(void)
{
	CONSOLE *con;
	if ((con=ActiveConsole())!=NULL)
	{
		int size= con->disp_columns-con->csrx;
		int pos= con->csry*con->disp_columns + con->csrx;
		assert(con->buffer!=NULL);

		for(int i= 0; i< size; i++)
			con->buffer[pos+i]= ' ';

		RefreshScreen(con, con->csry, con->csry+1);
	}
}
*/
/*
void amx_gotoxy(int x,int y)
{
	CONSOLE *con;
	if ((con=ActiveConsole())!=NULL)
	{
		if( x>0 && x<=con->disp_columns )
			con->csrx= x-1;

		if( y>0 && y<=con->disp_lines )
			con->csry= y-1;
		RefreshScreen(con,0,0);
	}
}

void amx_wherexy(int *x,int *y)
{
	CONSOLE *con;
	if ((con=ActiveConsole())!=NULL)
	{
		if( x!=NULL )
			*x= con->csrx+1;

		if( y!=NULL )
			*y= con->csry+1;
	}
}
*/

struct statusbar_text {
	int n;
	string str;
};


// outside thread --------------

void ConsoleThread::setStatusBarPartCountFromExt(int n)
{
	PostThreadMessage(console.tid, WM_USER, 2, n);
}

void ConsoleThread::setStatusBarTextFromExt(int idx, const char *txt)
{
	statusbar_text *sbt= new statusbar_text;
	sbt->n= idx;
	sbt->str= txt;
	PostThreadMessage(console.tid, WM_USER, 3, (LPARAM)sbt);
}


// insied thread ----------------

void ConsoleThread::setStatusBarPartCount(CONSOLE *con, int n)
{
	RECT rect;
	GetClientRect(con->hwnd, &rect);
	int Sizes[256];
	int last= 0;

	status_num= n;

	for( int i= 0; i< n; i++)
	{
		last= Sizes[i]= last + rect.right/n;
	}

	SendMessage(con->hStatus, SB_SETPARTS, n, (LPARAM)Sizes);
}

void ConsoleThread::setStatusBarText(CONSOLE *con, int idx, const char *txt)
{
	SendMessage(con->hStatus, SB_SETTEXT, idx, (LPARAM)txt);
}

void ConsoleThread::user_func()
{
	CONSOLE *con= ActiveConsole();
	MSG msg;

	//stop= false;

	while( !stop )
	{
		Sleep(25);
		// peek incoming messages
		while( PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) )
		{
			//stop= GetConsoleByIndex(0)==NULL;
			if( !stop )
			{
				if( msg.message == WM_USER )
				{
					switch( msg.wParam )
					{
					// refresh line(s)
					case 0:
						{
							refresh_line *rl= (refresh_line*)msg.lParam;
							RefreshScreen(con, rl->from, rl->to);
							delete rl;
							break;
						}

					// line added
					case 1:
						updateScrollbars(con);
						SetScrollPos(con->hScrollV, SB_CTL, msg.lParam - con->disp_lines, TRUE);
						InvalidateRect(con->hwnd, NULL, FALSE);
						break;

					// set statusbar parts count
					case 2:
						setStatusBarPartCount(con, msg.lParam);
						break;

					case 3:
						{
							statusbar_text *sbt= (statusbar_text *)msg.lParam;
							setStatusBarText(con, sbt->n, sbt->str.c_str());
							delete sbt;
							break;
						}


					default:
						break;
					}
				}
				else
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
		}
		//cancel_point();
	}
}

#if defined(TEST)
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	amx_printf("test bidon: %d\n", 2);
	amx_printf("salut");

	while (amx_termctl(4,0)){/* nothing */}
	return 0;
}
#endif

/**/



