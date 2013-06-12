// MediaDisplay.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "MediaDisplay.h"
#include "commctrl.h"

#define DSHOW			WM_USER+20
#define MAX_LOADSTRING	100
#define FILE_FILTER_TEXT \
    TEXT("Windows Media Files (*.asf; *.wma; *.wmv)\0*.asf; *.wma; *.wmv\0") \
    TEXT("All Files (*.*)\0*.*;\0\0")

// Begin default media search at root directory
#define DEFAULT_MEDIA_PATH  TEXT("\\\0")


// Global Variables:
HINSTANCE hInst;								// current instance
HWND hWnd;										// current main window
HBRUSH brush;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
ProxyType<MediaStream> videoStream;

// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

void ResizeClip ()
{
	if (videoStream.IsValid())
	{
		RECT rcClient;
		//RECT rcToolbar;

		GetClientRect(hWnd, &rcClient);
		//GetClientRect(toolbar, &rcToolbar);

		//rcClient.top = rcToolbar.bottom;

		videoStream->SetVideoWindow(rcClient);
		SetForegroundWindow(hWnd);
	}
}

//------------------------------------------------------------------------------
// Name: GetClipFileName()
// Desc: Get a file name from the user.
//------------------------------------------------------------------------------
BOOL OpenClip()
{
    static OPENFILENAME ofn={0};
    static BOOL bSetInitialDir = FALSE;
	static TCHAR szName[MAX_PATH+1];

    // Reset filename
    *szName = 0;

    // Fill in standard structure fields
    ofn.lStructSize       = sizeof(OPENFILENAME);
    ofn.hwndOwner         = hWnd;
    ofn.lpstrFilter       = NULL;
    ofn.lpstrFilter       = FILE_FILTER_TEXT;
    ofn.lpstrCustomFilter = NULL;
    ofn.nFilterIndex      = 1;
    ofn.lpstrFile         = szName;
    ofn.nMaxFile          = MAX_PATH;
    ofn.lpstrTitle        = TEXT("Open Media File...\0");
    ofn.lpstrFileTitle    = NULL;
    ofn.lpstrDefExt       = TEXT("*\0");
    ofn.Flags             = OFN_FILEMUSTEXIST | OFN_READONLY | OFN_PATHMUSTEXIST;
    
    // Remember the path of the first selected file
    if (bSetInitialDir == FALSE)
    {
        ofn.lpstrInitialDir = DEFAULT_MEDIA_PATH;
        bSetInitialDir = TRUE;
    }
    else 
        ofn.lpstrInitialDir = NULL;

    // Create the standard file open dialog and return its result
    if (GetOpenFileName((LPOPENFILENAME)&ofn) == TRUE)
	{
		videoStream = MediaStream::GetInstance();
		if ( (videoStream.IsValid ()                 == true) &&
 			 (videoStream->AttachFile(ofn.lpstrFile) == true) &&
			 (videoStream->AttachWindow(hWnd, DSHOW) == true) )
		{
			ResizeClip ();
			videoStream->Play();
		}
	}

	return ((videoStream.IsValid()) && (videoStream->IsPlaying()));
}

//------------------------------------------------------------------------------
// Name: CloseClip()
// Desc: Stops playback and does some cleanup.
//------------------------------------------------------------------------------
void CloseClip()
{
    // Stop media playback
	if(videoStream.IsValid())
	{
		videoStream->Pause();
		videoStream = ProxyType<MediaStream>();

		// Reset the player window
		RECT rect;
		GetClientRect(hWnd, &rect);
		InvalidateRect(hWnd, &rect, TRUE);
	}
}

void PausePlayClip()
{
    // Stop media playback
	if(videoStream.IsValid())
	{
		if (videoStream->IsPaused())
		{
			videoStream->Play();
		}
		else if (videoStream->IsPlaying())
		{
			videoStream->Pause();
		}
	}
}

void MuteClip()
{
    // Stop media playback
	if(videoStream.IsValid())
	{
		videoStream->Mute(!videoStream->IsMuted());
	}
}

void ClipBegin()
{
    // Stop media playback
	if(videoStream.IsValid())
	{
		videoStream->SetPosition(0);
	}
}

void FullScreen ()
{
    // Stop media playback
	if(videoStream.IsValid())
	{
		if (videoStream->IsFullscreen() == false)
		{
			videoStream->Fullscreen(true);
		}
		else
		{
			videoStream->Fullscreen(false);

			SetForegroundWindow(hWnd);
		    SetFocus(hWnd);
		}
	}
}

void HandleEvents ()
{
	if (videoStream.IsValid())
	{
		long eventCode, parameter1, parameter2;

		while (videoStream->GetEvent(eventCode, parameter1, parameter2))
		{
			// If this is the end of the clip, reset to beginning
			switch (eventCode)
			{
			case EC_COMPLETE:
			{
				break;
			}
			case EC_ERRORABORT:
			{
				// hr = static_cast<HRESULT>(parameter1);
				break;
			}
			}
		}
	}
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MEDIADISPLAY, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MEDIADISPLAY));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage are only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MEDIADISPLAY));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MEDIADISPLAY);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

    DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;

	brush = CreateHatchBrush(HS_BDIAGONAL, RGB(0, 0x80, 0xFF));

	ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
    case WM_SIZE:
		 ResizeClip ();
		 break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			 DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			 break;

        case ID_OPEN_CLIP:
             OpenClip();
             break;

        case ID_CLOSE_CLIP:
             CloseClip();
             break;

		case ID_PAUSE:
             PausePlayClip();
             break;

        case ID_MUTE:
             MuteClip();
             break;

        case ID_SIZE_FULLSCREEN:
             FullScreen();
             break;

		case IDM_EXIT:
			 DestroyWindow(hWnd);
			 break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
    case WM_KEYDOWN:
        switch(toupper((int) wParam))
        {
            case 'P':
                PausePlayClip();
                break;

            case 'S':
                break;

            case 'M':
                MuteClip();
                break;

            case 'F':
            case VK_RETURN:
                FullScreen();
                break;

			case 'B':
				ClipBegin ();
				break;

			case 'H':
                //InitVideoWindow(1,2);
                //CheckSizeMenu(wParam);
                break;
            case 'N':
                //InitVideoWindow(1,1);
                //CheckSizeMenu(wParam);
                break;
            case 'D':
                //InitVideoWindow(2,1);
                //CheckSizeMenu(wParam);
                break;
            case 'T':
                //InitVideoWindow(3,4);
                //CheckSizeMenu(wParam);
                break;

            case VK_ESCAPE:
				if (videoStream.IsValid() && videoStream->IsFullscreen())
				{
                    FullScreen();
				}
                else
				{
                    CloseClip();
				}
                break;

            case VK_F12:
            case 'Q':
            case 'X':
                CloseClip();
                break;
        }
        break;
	case WM_PAINT:
		{
			if (videoStream.IsValid())
			{
				//videoStream->Repaint();
			}
			else
			{
				hdc = BeginPaint(hWnd, &ps);
				// TODO: Add any drawing code here...
				RECT rcClient;
				//RECT rcToolbar;

				GetClientRect(hWnd, &rcClient);
				//GetClientRect(toolbar, &rcToolbar);

				HRGN hRgn1 = CreateRectRgnIndirect(&rcClient);
				//HRGN hRgn2 = CreateRectRgnIndirect(&rcToolbar);

				//CombineRgn(hRgn1, hRgn1, hRgn2, RGN_DIFF);

				//::PaintRgn(hdc, hRgn2);

				FillRgn(hdc, hRgn1, brush);

				DeleteObject(hRgn1);
				//DeleteObject(hRgn2);

				EndPaint(hWnd, &ps);
			}
		break;
		}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case DSHOW:
        // HandleGraphEvent();
        break;


	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return FALSE;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

