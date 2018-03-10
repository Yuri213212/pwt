/***

pwt v0.2 Beta
Copyright (C) 2016-2018 Yuri213212
Site:https://github.com/Yuri213212/pwt
Email: yuri213212@vip.qq.com
License: CC BY-NC-SA 4.0
https://creativecommons.org/licenses/by-nc-sa/4.0/

***/

#define UNICODE
#define _UNICODE

#include <windows.h>
#include <wchar.h>
#include <stdlib.h>
#include <math.h>
#include "aidb.h"
#include "pwt_ui.h"
#include "pwt_help.h"

struct ptdata{
	int ix;
	int iy;
	HRGN hrgn;
};

const int pdt[10]={0x241,0x103,0xa2,0x206,0x144,0x2c,0x288,0x58,0x130,0x91};
const int tri[10][3]={
	{0,1,9},
	{2,3,1},
	{4,5,3},
	{6,7,5},
	{8,9,7},
	{5,8,2},
	{7,0,4},
	{9,2,6},
	{1,4,8},
	{3,6,0}
};

WCHAR wbuf[1024];
int turn=0,endf=0,endt,bdf=0,p1or=0,p1xor=0,p2or=0,p2xor=0,rot,rev,aiconf=0,i,j,k;
double c1,c3,s1,s3,ci;
int x[9],y[5],board[11],list[11],list1[11],list2[11];
struct ptdata point[10]={
	{4,0,NULL},
	{5,1,NULL},
	{8,1,NULL},
	{6,2,NULL},
	{7,4,NULL},
	{4,3,NULL},
	{1,4,NULL},
	{2,2,NULL},
	{0,1,NULL},
	{3,1,NULL}
};

int aidbbsch(int l,int r){
	int m,res;

	m=(l+r)/2;
	res=aidb[m]&0x3ffffc00;
	if (res==aiconf) return m;
	if (aiconf<res){
		if (m-1<l) return -1;
		return aidbbsch(l,m-1);
	}else{
		if (m+1>r) return -1;
		return aidbbsch(m+1,r);
	}
}

int decode(int x){
	for (i=0;i<=9;++i){
		if (x&(1<<i)) return i;
	}
	return -1;
}

void place(int x){
	switch (turn){
	case 0:
		rot=x&1?(x+5)%10:x;
		aiconf|=1<<(20+(x&1?5:0));
		break;
	case 1:
		rev=(10+x-rot)%10>5;
	default:
		aiconf|=1<<((turn&1?10:20)+(rev?(10+rot-x)%10:(10+x-rot)%10));
	}
	board[turn++]=x;
	bdf|=1<<x;
	if (turn&1){
		if ((endf=(p1or^p1xor)&pdt[x])){
			endt=decode(endf);
			endf=1;
		}else{
			p1or|=pdt[x];
			p1xor^=pdt[x];
		}
	}else{
		if ((endf=(p2or^p2xor)&pdt[x])){
			endt=decode(endf);
			endf=2;
		}else{
			p2or|=pdt[x];
			p2xor^=pdt[x];
		}
	}
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam){
	static int cxClient,cyClient,ctx,cty,r,mx,my;
	static HPEN hPenBlue,hPenPink;
	static HBRUSH hBrushBlue,hBrushPink;
	static POINT apt[8];
	HDC hdc;
	PAINTSTRUCT ps;

	switch (message){
	case WM_CREATE:
		hPenBlue=CreatePen(PS_SOLID,3,RGB(0,0,255));
		hPenPink=CreatePen(PS_SOLID,3,RGB(192,0,192));
		hBrushBlue=CreateSolidBrush(RGB(0,0,255));
		hBrushPink=CreateSolidBrush(RGB(192,0,192));
		for (i=0;i<10;++i){
			point[i].hrgn=CreateRectRgn(0,0,0,0);
		}
		SetTimer(hwnd,1,500,NULL);
		return 0;
	case WM_SIZE:
		cxClient=LOWORD(lParam);
		cyClient=HIWORD(lParam);
		ctx=cxClient/2;
		cty=cyClient*0.54;
		r=(cxClient>cyClient?cyClient:cxClient)*0.45;
		x[0]=ctx-r*c1;
		x[1]=ctx-r*c3;
		x[2]=ctx-r*c1*s1/s3;
		x[3]=ctx-r*c3*s1/s3;
		x[4]=ctx;
		x[5]=ctx+r*c3*s1/s3;
		x[6]=ctx+r*c1*s1/s3;
		x[7]=ctx+r*c3;
		x[8]=ctx+r*c1;
		y[0]=cty-r;
		y[1]=cty-r*s1;
		y[2]=cty+r*s1*s1/s3;
		y[3]=cty+r*s1/s3;
		y[4]=cty+r*s3;
		apt[0].x=x[0];
		apt[0].y=y[1];
		apt[1].x=x[8];
		apt[1].y=y[1];
		apt[2].x=x[1];
		apt[2].y=y[4];
		apt[3].x=x[4];
		apt[3].y=y[0];
		apt[4].x=x[7];
		apt[4].y=y[4];
		r/=2+log(r+1);
		for (i=0;i<10;++i){
			DeleteObject(point[i].hrgn);
			point[i].hrgn=CreateRoundRectRgn(x[point[i].ix]-r+1,y[point[i].iy]-r+1,x[point[i].ix]+r+1,y[point[i].iy]+r+1,r*2,r*2);
		}
		return 0;
	case WM_PAINT:
		hdc=BeginPaint(hwnd,&ps);
		SelectObject(hdc,GetStockObject(SYSTEM_FIXED_FONT));
		SetBkMode(hdc,TRANSPARENT);
		SelectObject(hdc,GetStockObject(NULL_BRUSH));
		Polygon(hdc,apt,5);
		for (i=0;i<turn;++i){
			if (i&1){
				SelectObject(hdc,hBrushPink);
			}else{
				SelectObject(hdc,hBrushBlue);
			}
			PaintRgn(hdc,point[board[i]].hrgn);
		}
		if (endf){
			apt[5].x=x[point[tri[endt][0]].ix];
			apt[5].y=y[point[tri[endt][0]].iy];
			apt[6].x=x[point[tri[endt][1]].ix];
			apt[6].y=y[point[tri[endt][1]].iy];
			apt[7].x=x[point[tri[endt][2]].ix];
			apt[7].y=y[point[tri[endt][2]].iy];
			SelectObject(hdc,GetStockObject(NULL_BRUSH));
			if (endf&1){
				SelectObject(hdc,hPenBlue);
				swprintf(wbuf,szInfo_P1lost);
			}else{
				SelectObject(hdc,hPenPink);
				swprintf(wbuf,szInfo_P2lost);
			}
			Polygon(hdc,apt+5,3);
		}else{
			if (turn<=9){
				swprintf(wbuf,szInfo_Pnturn,(turn&1)+1);
			}else{
				swprintf(wbuf,szInfo_Tie);
			}
		}
		if (turn){
			SelectObject(hdc,GetStockObject(WHITE_PEN));
			MoveToEx(hdc,x[point[board[turn-1]].ix]-3,y[point[board[turn-1]].iy]-3,NULL);
			LineTo(hdc,x[point[board[turn-1]].ix]+4,y[point[board[turn-1]].iy]+4);
			MoveToEx(hdc,x[point[board[turn-1]].ix]+3,y[point[board[turn-1]].iy]-3,NULL);
			LineTo(hdc,x[point[board[turn-1]].ix]-4,y[point[board[turn-1]].iy]+4);
		}
		TextOutW(hdc,0,0,wbuf,wcslen(wbuf));
		EndPaint(hwnd,&ps);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)){
		case 40001:
			p2or=p2xor=p1or=p1xor=bdf=endf=turn=0;
			aiconf=0;
			rev=rand()&1;
			InvalidateRect(hwnd,NULL,TRUE);
			return 0;
		case 40003:
			if (endf||turn>=10) return 0;
			k=aidbbsch(0,AIDBL-1);
			j=0;
			if (k>=0){
				k=aidb[k]&0x3ff;
				for (i=0;i<10;++i){
					if (k&(1<<i)){
						list[j++]=i;
					}
				}
			}
			if (j){
				i=list[rand()%j];
				place(rev?(10+rot-i)%10:(i+rot)%10);
				InvalidateRect(hwnd,NULL,TRUE);
				return 0;
			}
		case 40002:
			if (endf||turn>=10) return 0;
			j=k=0;
			for (i=0;i<10;++i){
				if (bdf&(1<<i)) continue;
				list1[j++]=i;
				if (turn&1){
					if ((p2or^p2xor)&pdt[i]) continue;
					list2[k++]=i;
				}else{
					if ((p1or^p1xor)&pdt[i]) continue;
					list2[k++]=i;
				}
			}
			if (k){
				place(list2[rand()%k]);
				InvalidateRect(hwnd,NULL,TRUE);
			}else{
				place(list1[rand()%j]);
				InvalidateRect(hwnd,NULL,TRUE);
			}
			return 0;
		case 40004:
			MessageBoxW(NULL,szHelp,szTitle,MB_ICONQUESTION);
			return 0;
		default:
			;
		}
		break;
	case WM_LBUTTONDOWN:
		if (endf||turn>=10) return 0;
		mx=LOWORD(lParam);
		my=HIWORD(lParam);
		for (i=0;i<10;++i){
			if (!PtInRegion(point[i].hrgn,mx,my)) continue;
			if (bdf&(1<<i)) break;
			place(i);
			InvalidateRect(hwnd,NULL,TRUE);
		}
		return 0;
	case WM_KEYDOWN:
		if (HIWORD(lParam)>>9) return 0;
		switch (wParam){
		case VK_ESCAPE:
			SendMessage(hwnd,WM_COMMAND,40001,0);
			return 0;
		case 'J':
			SendMessage(hwnd,WM_COMMAND,40002,0);
			return 0;
		case 'S':
			SendMessage(hwnd,WM_COMMAND,40003,0);
			return 0;
		case VK_F1:
			SendMessage(hwnd,WM_COMMAND,40004,0);
		default:
			return 0;
		}
	case WM_TIMER:
		rand();
		return 0;
	case WM_DESTROY:
		KillTimer(hwnd,1);
		DeleteObject(hPenBlue);
		DeleteObject(hPenPink);
		DeleteObject(hBrushBlue);
		DeleteObject(hBrushPink);
		for (i=0;i<10;++i){
			DeleteObject(point[i].hrgn);
		}
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd,message,wParam,lParam);
}

int main(){
	static int argc;
	static WCHAR **argv;
	static HINSTANCE hInstance;
	static int iCmdShow;
	static STARTUPINFOW si;
	static BYTE ANDmaskIcon[]={
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01,
		0x00,0x01
	};
	static BITMAP bitmapmask={0,16,16,2,1,1,ANDmaskIcon};
	static ICONINFO iconinfo={TRUE,0,0,NULL,NULL};
	LARGE_INTEGER l;
	HDC hdc,hdcMem;
	HBITMAP hBitmap,hBitmapMask;
	HBRUSH hBrushBlue,hBrushPink;
	HICON hIcon;
	WNDCLASSW wndclass;
	HMENU hMenu;
	HWND hwnd;
	MSG msg={};

	argv=CommandLineToArgvW(GetCommandLineW(),&argc);
	hInstance=GetModuleHandleW(NULL);
	GetStartupInfoW(&si);
	iCmdShow=si.dwFlags&STARTF_USESHOWWINDOW?si.wShowWindow:SW_SHOWNORMAL;
	hdc=GetDC(NULL);
	hdcMem=CreateCompatibleDC(hdc);
	hBitmap=CreateCompatibleBitmap(hdc,16,16);
	ReleaseDC(NULL,hdc);
	SelectObject(hdcMem,hBitmap);
	hBrushBlue=CreateSolidBrush(RGB(0,0,255));
	hBrushPink=CreateSolidBrush(RGB(192,0,192));
	SelectObject(hdcMem,GetStockObject(NULL_PEN));
	SelectObject(hdcMem,GetStockObject(WHITE_BRUSH));
	Rectangle(hdcMem,0,0,17,17);
	SelectObject(hdcMem,GetStockObject(BLACK_PEN));
	MoveToEx(hdcMem,15,0,NULL);
	LineTo(hdcMem,15,16);
	MoveToEx(hdcMem,7,2,NULL);
	LineTo(hdcMem,2,13);
	LineTo(hdcMem,12,13);
	LineTo(hdcMem,7,2);
	SelectObject(hdcMem,GetStockObject(NULL_PEN));
	SelectObject(hdcMem,hBrushBlue);
	Rectangle(hdcMem,6,0,10,6);
	Rectangle(hdcMem,5,1,11,5);
	SelectObject(hdcMem,hBrushPink);
	Rectangle(hdcMem,1,11,5,17);
	Rectangle(hdcMem,0,12,6,16);
	Rectangle(hdcMem,11,11,15,17);
	Rectangle(hdcMem,10,12,16,16);
	iconinfo.hbmColor=hBitmap;
	hBitmapMask=CreateBitmapIndirect(&bitmapmask);
	iconinfo.hbmMask=hBitmapMask;
	hIcon=CreateIconIndirect(&iconinfo);
	DeleteDC(hdcMem);
	DeleteObject(hBrushBlue);
	DeleteObject(hBrushPink);
	DeleteObject(hBitmap);
	DeleteObject(hBitmapMask);
	wndclass.style=CS_HREDRAW|CS_VREDRAW;
	wndclass.lpfnWndProc=WndProc;
	wndclass.cbClsExtra=0;
	wndclass.cbWndExtra=0;
	wndclass.hInstance=hInstance;
	wndclass.hIcon=hIcon;
	wndclass.hCursor=LoadCursor(NULL,IDC_ARROW);
	wndclass.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName=szAppName;
	wndclass.lpszClassName=szAppName;
	if (!RegisterClassW(&wndclass)){
		MessageBoxW(NULL,szErr_RegisterClass,argv[0],MB_ICONERROR);
		goto endmain;
	}
	c1=cos(0.1*M_PI);
	c3=cos(0.3*M_PI);
	s1=sin(0.1*M_PI);
	s3=sin(0.3*M_PI);
	ci=s1/s3;
	QueryPerformanceCounter(&l);
	srand(l.LowPart&RAND_MAX);
	rev=l.LowPart&1;
	hMenu=CreateMenu();
	AppendMenuW(hMenu,MF_STRING,40001,szMenu_NewGame);
	AppendMenuW(hMenu,MF_STRING,40002,szMenu_JuniorAI);
	AppendMenuW(hMenu,MF_STRING,40003,szMenu_SeniorAI);
	AppendMenuW(hMenu,MF_STRING,40004,szMenu_Help);
	hwnd=CreateWindowW(szAppName,szTitle,WS_OVERLAPPEDWINDOW,(GetSystemMetrics(SM_CXSCREEN)-400)/2,(GetSystemMetrics(SM_CYSCREEN)-400)/2,400,400,NULL,hMenu,hInstance,NULL);
	ShowWindow(hwnd,iCmdShow);
	UpdateWindow(hwnd);
	while (GetMessage(&msg,NULL,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
endmain:
	DestroyIcon(hIcon);
	LocalFree(argv);
	return msg.wParam;
}
