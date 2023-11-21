//
// #include
//
#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <sstream>
#include <algorithm>
#include <Windows.h>
#include <cfgmgr32.h>
#include <strsafe.h>
#include "CApplication.h"
#include "../../Include/itsoftware.h"
#include "Main.h"
#include "../../Include/itsoftware-com.h"
#include <winerror.h>
#include "../../Include/IsKbdRkt.h"
#include "../../Include/IS_KEYBOARD_RKT_OUTPUT_DATA.h"
#include <uxtheme.h>
#include <guiddef.h>
//#include "../../Steam/sdk/public/steam/steam_api.h"
// sdkencryptedappticket64.lib steam_api64.lib 
//#include "../../Steam/sdk/public/steam/cegclient.h"
//
// #define
//
#define MAIN_WINDOW_SIZE_X			1280
#define MAIN_WINDOW_SIZE_Y			450
#define EVENT_LOG_SOURCE_NAME					L"KKS Is Keyboard Rootkitted"

//
// const
//
const COLORREF g_LISTVIEW_BK_COLOR = 0xFFFFFFFF;//EBC09A;
const COLORREF g_LISTVIEW_TEXT_BK_COLOR = 0xFFFFFFFF;
const COLORREF g_LISTVIEW_TEXT_COLOR = 0x00101010;

//
// GUID DEVINTERFACE ROOTKITTED DRIVER
//
//DEFINE_GUID(GUID_DEVINTERFACE_IsKeyboardRootkittedDriver,
//	0x676b766c, 0x5442, 0x4550, 0x8d, 0xb9, 0xff, 0xd5, 0x93, 0xbb, 0x3d, 0x70);
//
const GUID GUID_DEVINTERFACE_IsKeyboardRootkittedDriver \
= { 0x676b766c, 0x5442, 0x4550, { 0x8d, 0xb9,  0xff,  0xd5,  0x93,  0xbb,  0x3d,  0x70 } };

//
// using namespace
//
using namespace ItSoftware;
using namespace ItSoftware::COM;

//
// namespace
//
namespace IsKeyboardRootkitted
{
	//
	// Function: wWinMain
	//
	int CApplication::WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow )
	{		
		//
		// Use dummy data or not.
		// Suppresses live data if set to TRUE.
		//
		this->m_bUseDummyData = FALSE;

		//
		// Initialize common controls.
		//
		INITCOMMONCONTROLSEX icex;
		icex.dwSize = sizeof( INITCOMMONCONTROLSEX );
		icex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES | ICC_LISTVIEW_CLASSES | ICC_STANDARD_CLASSES;
		InitCommonControlsEx( &icex );

		InitializeCriticalSection( &this->m_hCriticalSection );		

		//
		// New up Dialogs
		//			
		this->m_pCAboutDlg = make_unique<CAboutDlg>( this );

		UNREFERENCED_PARAMETER( hPrevInstance );
		UNREFERENCED_PARAMETER( lpCmdLine );

		// TODO: Place code here.
		this->m_hIconStatusArea = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_ICON_APP ) );

		// Initialize global strings
		LoadStringW( hInstance, IDS_APP_TITLE, m_wszTitle, MAX_LOADSTRING );
		LoadStringW( hInstance, IDC_KKSISKEYBOARDRKTAPP, m_wszWindowClass, MAX_LOADSTRING );
		MyRegisterClass( hInstance );

		// Perform application initialization:
		if ( !InitInstance( hInstance, nCmdShow ) )
		{
			return FALSE;
		}

		HACCEL hAccelTable = LoadAccelerators( hInstance, MAKEINTRESOURCE( IDC_KKSISKEYBOARDRKTAPP ) );

		MSG msg;

		// Main message loop:
		while ( GetMessage( &msg, nullptr, 0, 0 ) )
		{
			if ( !TranslateAccelerator( msg.hwnd, hAccelTable, &msg ) )
			{
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}

		this->RemoveStatusBarIcon();

		return (int)msg.wParam;		
	}

	

	//
	// Function: AddStatusAreaIcon
	//
	void CApplication::AddStatusAreaIcon()
	{
		Shell_NotifyIcon( NIM_ADD, &this->m_notifyIconData );
		Shell_NotifyIcon( NIM_SETVERSION, &this->m_notifyIconData );
	}

	

	//
	//  Function: MyRegisterClass
	//
	ATOM CApplication::MyRegisterClass( HINSTANCE hInstance )
	{
		WNDCLASSEXW wcex;

		wcex.cbSize = sizeof( WNDCLASSEX );

		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_ICON_APP ) );
		wcex.hCursor = LoadCursor( nullptr, IDC_ARROW );
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = MAKEINTRESOURCEW( IDC_KKSISKEYBOARDRKTAPP );
		wcex.lpszClassName = m_wszWindowClass;
		wcex.hIconSm = LoadIcon( wcex.hInstance, MAKEINTRESOURCE( IDI_ICON_APP ) );

		return RegisterClassExW( &wcex );
	}

	//
	// Function: InitInstance(HINSTANCE, int)
	//
	// (i): Saves instance handle and creates main window
	//	
	BOOL CApplication::InitInstance( HINSTANCE hInstance, int nCmdShow )
	{
		m_hInstance = hInstance; // Store instance handle in our global variable		

		int x = (GetSystemMetrics( SM_CXSCREEN ) - MAIN_WINDOW_SIZE_X) / 2;
		int y = (GetSystemMetrics( SM_CYSCREEN ) - MAIN_WINDOW_SIZE_Y) / 2;

		HWND hWnd = CreateWindow( m_wszWindowClass, m_wszTitle, WS_CAPTION | WS_SYSMENU | WS_SIZEBOX | WS_MINIMIZEBOX | WS_MAXIMIZEBOX,
			x, y, MAIN_WINDOW_SIZE_X, MAIN_WINDOW_SIZE_Y, nullptr, nullptr, hInstance, reinterpret_cast<LPVOID>(this) );

		if ( !hWnd )
		{
			return FALSE;
		}

		ShowWindow( hWnd, nCmdShow );
		UpdateWindow( hWnd );

		return TRUE;
	}

	//
	// Function: OnWM_CREATE
	//
	void CApplication::OnWM_CREATE( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		this->m_hMenu = GetMenu( hWnd );
		this->m_hMenuContext = this->m_hMenu;

		//
		// Prepare to add an icon to the status area.
		//
		this->m_notifyIconData.cbSize = sizeof( NOTIFYICONDATA );
		this->m_notifyIconData.hIcon = this->m_hIconStatusArea;
		this->m_notifyIconData.hWnd = hWnd;
		wcscpy_s( this->m_notifyIconData.szTip, L"Is Keyboard Rootkitted" );
		this->m_notifyIconData.uCallbackMessage = RACECORE24_NOTIFYICON_CALLBACK_MESSAGE;
		this->m_notifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		this->m_notifyIconData.uVersion = NOTIFYICON_VERSION;
		this->m_notifyIconData.dwState = NIS_SHAREDICON;
		this->m_notifyIconData.dwStateMask = NIS_SHAREDICON;
		this->m_notifyIconData.uID = RACECORE24_NOTIFYICON_ICONID;

		this->AddStatusAreaIcon();

		this->m_uTaskbarRestart = RegisterWindowMessage( L"TaskbarCreated" );

		this->m_hWnd = hWnd;				

		this->CreateStatusBar();		
		this->CreateListView();
		this->PopulateListView();		
	}

	
	//
	// Function: OnWM_COMMAND
	//
	bool CApplication::OnWM_COMMAND( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch ( LOWORD( wParam ) )
		{
			case IDM_ABOUT:
			{
				this->m_pCAboutDlg->ShowDialog();
				break;
			}
			case IDM_REFRESH:			
			{				
				this->PopulateListView();
				break;
			}			
			case IDM_EXIT:
				DestroyWindow( hWnd );
				break;
			default:
				return true;
		}
		return false;
	}

	//
	// Function: OnWM_PAINT
	//
	void CApplication::OnWM_PAINT( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint( hWnd, &ps );
		// TODO: Add any drawing code that uses hdc here...
		EndPaint( hWnd, &ps );
	}

	//
	// Function: OnWM_SYSCOMMAND
	//
	void CApplication::OnWM_SYSCOMMAND( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch ( wParam )
		{
			case SC_MINIMIZE:
			{
				/*ShowWindow( hWnd, SW_HIDE );
				SetWindowLongPtr( hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW );
				SetWindowLongPtr( hWnd, GWL_STYLE, WS_CAPTION | WS_SYSMENU );*/
				break;
			}
		}
	}

	//
	// Function: OnWM_CLOSE
	//
	void CApplication::OnWM_CLOSE( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		ShowWindow( hWnd, SW_HIDE );
	}

	//
	// Function: OnWM_DESTROY
	//
	void CApplication::OnWM_DESTROY( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{		
		PostQuitMessage( 0 );
	}

	//
	// Method: OnWM_SIZE
	//
	void CApplication::OnWM_SIZE( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		RECT rectParent;
		GetClientRect( this->m_hWnd, &rectParent );

		RECT rectStatusBar;
		GetClientRect( this->m_hWndStatusBar, &rectStatusBar );

		MoveWindow( this->m_hWndStatusBar, 0, rectParent.bottom - (rectStatusBar.bottom - rectStatusBar.top), rectParent.right - rectParent.left, rectStatusBar.bottom - rectStatusBar.top, TRUE );

		RECT rectListView;
		GetClientRect( this->m_hWndListView, &rectListView );

		MoveWindow( this->m_hWndListView, 0, 0, rectParent.right - rectParent.left, rectParent.bottom - (rectStatusBar.bottom - rectStatusBar.top), TRUE );
	}

	//
	// Function: OnRACECORE24_NOTIFYICON_CALLBACK_MESSAGE
	//
	LRESULT CApplication::OnRACECORE24_NOTIFYICON_CALLBACK_MESSAGE( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch ( lParam )
		{
			case WM_RBUTTONDOWN:
			{
				if ( this->m_hMenuContext != NULL )
				{
					SendMessage( hWnd, WM_CANCELMODE, 0, 0 );
					DestroyMenu( this->m_hMenuContext );
					this->m_hMenuContext = NULL;
					this->m_hSubMenuContext = NULL;
				}
				return 0;
				break;
			}// WM_RBUTTONDOWN
			case WM_LBUTTONDBLCLK:
				//SetWindowLongPtr( hWnd, GWL_EXSTYLE, WS_EX_APPWINDOW );
				//SetWindowLongPtr( hWnd, GWL_STYLE, WS_CAPTION | WS_SYSMENU );
				ShowWindow( hWnd, SW_NORMAL );
				break;
			case WM_CONTEXTMENU:
			{
				POINT pt;
				::GetCursorPos( &pt );

				this->m_hMenuContext = ::LoadMenu( this->m_hInstance, MAKEINTRESOURCE( IDR_MENU_STATUSAREA_MAIN ) );
				this->m_hSubMenuContext = ::GetSubMenu( this->m_hMenuContext, 0 );

				wchar_t wcsBuffer[1024];
				wcscpy_s( wcsBuffer, L"" );

				SetForegroundWindow( hWnd );
				DWORD dwItem = ::TrackPopupMenuEx( this->m_hSubMenuContext, TPM_RIGHTALIGN | TPM_BOTTOMALIGN | TPM_RETURNCMD, pt.x, pt.y, hWnd, NULL );
				switch ( dwItem )
				{
					case ID_NOTIFY_EXIT:
					{
						int dialogResult = MessageBox( GetForegroundWindow(), L"Do you really want to quit Is Keyboard Rootkitted?", L"Confirmation", MB_YESNO | MB_ICONINFORMATION );
						if ( dialogResult == IDYES )
						{							
							::PostQuitMessage( 0 );
						}
						break;
					}// ID_NOADVERT_EXIT					 
					case ID_NOTIFY_OPEN:
					{					
						ShowWindow( hWnd, SW_NORMAL );
						break;
					}// ID_NOADVERT_SETTINGS					
					default:
						break;
				}// switch ( dwItem )													
				DestroyMenu( this->m_hMenuContext );
				this->m_hMenuContext = NULL;
				this->m_hSubMenuContext = NULL;
				break;
			}// WM_CONTEXTMENU				
			default:
				//
				// Check if taskbar has been restarted.
				//
				if ( message == this->m_uTaskbarRestart )
				{
					this->AddStatusAreaIcon();
				}
				break;
		}// switch
		return 0;
	}

	//
	// Function: WndProc(HWND, UINT, WPARAM, LPARAM)
	//
	// (i): Processes messages for the main window.
	//	
	LRESULT CALLBACK CApplication::WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		static CApplication* pCApplication( nullptr );

		switch ( message )
		{
			case WM_CREATE:
			{
				LPCREATESTRUCT pCS = reinterpret_cast<LPCREATESTRUCT>(lParam);
				pCApplication = reinterpret_cast<CApplication*>(pCS->lpCreateParams);

				pCApplication->OnWM_CREATE( hWnd, message, wParam, lParam );								

				break;
			}						
			case WM_COMMAND:
			{
				if ( pCApplication->OnWM_COMMAND( hWnd, message, wParam, lParam ) )
				{
					return DefWindowProc( hWnd, message, wParam, lParam );
				}
				break;
			}
			case WM_PAINT:
			{
				pCApplication->OnWM_PAINT( hWnd, message, wParam, lParam );
				break;
			}
			case WM_SIZE:
			{
				pCApplication->OnWM_SIZE( hWnd, message, wParam, lParam );
				break;
			}
			case WM_SYSCOMMAND:
			{
				pCApplication->OnWM_SYSCOMMAND( hWnd, message, wParam, lParam );
				return DefWindowProc( hWnd, message, wParam, lParam );
			}
			case WM_CLOSE:
			{
				pCApplication->OnWM_CLOSE( hWnd, message, wParam, lParam );
				break;
			}
			case WM_DESTROY:
			{
				pCApplication->OnWM_DESTROY( hWnd, message, wParam, lParam );
				break;
			}
			case RACECORE24_NOTIFYICON_CALLBACK_MESSAGE:
			{
				return pCApplication->OnRACECORE24_NOTIFYICON_CALLBACK_MESSAGE( hWnd, message, wParam, lParam );
				break;
			}// SNAPTO_NOTIFYICON_CALLBACK_MESSGAE				
			case WM_NOTIFY:
			{				
				switch ( ((LPNMHDR)lParam)->code )
				{
					case NM_CUSTOMDRAW:
					{
						LPNMLVCUSTOMDRAW  lplvcd = (LPNMLVCUSTOMDRAW)lParam;
						switch ( lplvcd->nmcd.dwDrawStage )
						{													
							case CDDS_PREPAINT:
							{
								return CDRF_NOTIFYITEMDRAW;
							}
							case CDDS_ITEMPREPAINT:
							{
								//ListView_GetItemState(pCApplication->m_hWndListView,lplvcd->nmcd.dwItemSpec,LVIS_FOCUSED|LVIS_SELECTED) == (LVIS_FOCUSED|LVIS_SELECTED)
								if ( lplvcd->nmcd.hdr.hwndFrom == pCApplication->m_hWndListView )
								{
									UINT itemState = ListView_GetItemState( pCApplication->m_hWndListView, lplvcd->nmcd.dwItemSpec, LVIS_FOCUSED | LVIS_SELECTED );
									

									wchar_t wszItemText[128];

									LV_ITEM lvItem = { 0 };
									lvItem.mask = LVIF_TEXT;
									lvItem.iItem = (int)lplvcd->nmcd.dwItemSpec;
									lvItem.cchTextMax = 127;
									lvItem.pszText = wszItemText;
									lvItem.iSubItem = 1;
									ListView_GetItem( pCApplication->m_hWndListView, &lvItem );									

									RECT textRect;
									//HFONT oldFont, newFont;
									//oldFont = (HFONT)SelectObject( lplvcd->nmcd.hdc, newFont );

									RECT rectItem;
									ListView_GetItemRect( pCApplication->m_hWndListView, lplvcd->nmcd.dwItemSpec, &rectItem, LVIR_BOUNDS );
									
									int iHeight = rectItem.bottom - rectItem.top;
									int iPartHeight = iHeight / 2;

									rectItem.left += 18;
									if ( wcslen( wszItemText ) > 0 )
									{
										rectItem.left += 16;
									}

									HRGN bgRgn = CreateRectRgnIndirect( &rectItem );
									HBRUSH MyBrush = (itemState == (LVIS_FOCUSED | LVIS_SELECTED)) ? CreateSolidBrush( RGB( 220, 255, 210 ) ) : CreateSolidBrush( RGB( 255, 255, 255 ) );
									FillRgn( lplvcd->nmcd.hdc, bgRgn, MyBrush );
									DeleteObject( MyBrush );

									/*
									HRGN bgRgn = CreateRectRgnIndirect( &rectItem );
									HBRUSH MyBrush = CreateSolidBrush( RGB( 100, 100, 100 ) );
									FillRgn( lplvcd->nmcd.hdc, bgRgn, MyBrush );
									DeleteObject( MyBrush );

									SetTextColor( lplvcd->nmcd.hdc, RGB( 255, 255, 255 ) );

									newFont = CreateFont( 16, 0, 0, 0, FW_NORMAL, TRUE, FALSE, FALSE,
									EASTEUROPE_CHARSET, 0, 0, CLEARTYPE_NATURAL_QUALITY,
									DEFAULT_PITCH | FF_DONTCARE, L"Times New Roman" );
									oldFont = (HFONT)SelectObject( lplvcd->nmcd.hdc, newFont );
									*/


									
									int iWidth( 0 );
									
									//
									// Draw iSubItem 1 Text
									//
									POINT pt = { 0 };
									ListView_GetItemPosition( pCApplication->m_hWndListView, 0, &pt );
									if ( pt.x >= 0 && wcslen(wszItemText) == 0)
									{
										HDC	hMemDC = CreateCompatibleDC( lplvcd->nmcd.hdc );
										SelectObject( hMemDC, pCApplication->m_hBmpCpu );

										BitBlt( lplvcd->nmcd.hdc, 0, rectItem.top+iPartHeight-8, 16, 16, hMemDC, 0, 0, SRCCOPY );

										DeleteDC( hMemDC );
									}
									else if ( pt.x >= 0 && wcslen(wszItemText) > 0) 
									{
										HDC	hMemDC = CreateCompatibleDC( lplvcd->nmcd.hdc );
										SelectObject( hMemDC, pCApplication->m_hBmpMemory );

										BitBlt( lplvcd->nmcd.hdc, 16, rectItem.top+iPartHeight-8, 16, 16, hMemDC, 0, 0, SRCCOPY );

										DeleteDC( hMemDC );
									}
									ListView_GetSubItemRect( pCApplication->m_hWndListView, lplvcd->nmcd.dwItemSpec, 0, LVIR_BOUNDS, &rectItem );
									rectItem.left += 16;
									if ( wcslen(wszItemText) > 0 ) 
									{
										rectItem.left += 16;
									}
									ListView_GetItemText( pCApplication->m_hWndListView, lplvcd->nmcd.dwItemSpec, 0, wszItemText, 128 );
									SetRect( &textRect, rectItem.left + 3, rectItem.top + 2, rectItem.right, rectItem.bottom );
									DrawText( lplvcd->nmcd.hdc, wszItemText, -1, &textRect, DT_LEFT | DT_NOCLIP );

									//
									// Draw iSubItem 2 Text
									//
									ListView_GetSubItemRect( pCApplication->m_hWndListView, lplvcd->nmcd.dwItemSpec, 1, LVIR_BOUNDS, &rectItem );
									ListView_GetItemText( pCApplication->m_hWndListView, lplvcd->nmcd.dwItemSpec, 1, wszItemText, 128 );
									SetRect( &textRect, rectItem.left + 3, rectItem.top + 2, rectItem.right, rectItem.bottom );
									DrawText( lplvcd->nmcd.hdc, wszItemText, -1, &textRect, DT_LEFT | DT_NOCLIP );

									//
									// Draw iSubItem 3 Text
									//
									ListView_GetSubItemRect( pCApplication->m_hWndListView, lplvcd->nmcd.dwItemSpec, 2, LVIR_BOUNDS, &rectItem );
									ListView_GetItemText( pCApplication->m_hWndListView, lplvcd->nmcd.dwItemSpec, 2, wszItemText, 128 );
									SetRect( &textRect, rectItem.left + 3, rectItem.top + 2, rectItem.right, rectItem.bottom );
									DrawText( lplvcd->nmcd.hdc, wszItemText, -1, &textRect, DT_LEFT | DT_NOCLIP );

									//SelectObject( lplvcd->nmcd.hdc, oldFont );
									//DeleteObject( newFont );									

									return CDRF_SKIPDEFAULT;
								}
							}
						}
						break;
					}
				}
				break;
			}
			default:
				return DefWindowProc( hWnd, message, wParam, lParam );
		}
		return 0;
	}

	//
	// Function: RemoveStatusBarIcon
	//
	void CApplication::RemoveStatusBarIcon()
	{
		Shell_NotifyIcon( NIM_DELETE, &this->m_notifyIconData );
	}


	//
	// Function: CreateStatusBar
	//
	void CApplication::CreateStatusBar()
	{
		RECT rcClient;                       // The parent window's client area.

		GetClientRect( this->m_hWnd, &rcClient );

		// 
		// Create ListView
		//
		this->m_hWndStatusBar = CreateWindowEx( 0,
			STATUSCLASSNAME,
			NULL,
			WS_CHILD | WS_VISIBLE | SBARS_SIZEGRIP,
			0, 0,
			0,
			0,
			this->m_hWnd,
			NULL,//(this->m_hMenuContext)IDM_CODE_SAMPLES,
			this->m_hInstance,
			NULL );

		//
		// Initialize Status Bar
		//
		int StatusBarParts[] = { 550, -1 };

		SendMessage( this->m_hWndStatusBar, SB_SETPARTS, (WPARAM)(sizeof( StatusBarParts ) / sizeof( int )), (LPARAM)StatusBarParts );
	}

	//
	// Function: SetStatusBarText
	//
	void CApplication::SetStatusBarText( wchar_t* pwszText )
	{
		if ( pwszText != nullptr )
		{
			SendMessage( this->m_hWndStatusBar, SB_SETTEXT, (WPARAM)(INT)0 | 0, (LPARAM)pwszText );
		}
	}

	//
	// Function: SetStatusBarText
	//
	void CApplication::SetStatusBarText( const wchar_t* pwszText )
	{
		if ( pwszText != nullptr )
		{
			SendMessage( this->m_hWndStatusBar, SB_SETTEXT, (WPARAM)(INT)0 | 0, (LPARAM)pwszText );
		}
	}

	//
	// Function: CreateListView
	//
	void CApplication::CreateListView()
	{
		RECT rcClient = { 0 };                       // The parent window's client area.		
		GetClientRect( this->m_hWnd, &rcClient );

		RECT rectStatusBar { 0 };
		GetClientRect( this->m_hWndStatusBar, &rectStatusBar );

		RECT rectReBar = { 0 };
		GetClientRect( this->m_hWndReBar, &rectReBar );

		// 
		// Create ListView
		//
		this->m_hWndListView = CreateWindow( WC_LISTVIEW,
			L"",
			WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SINGLESEL | LVS_SHOWSELALWAYS,
			0, rectReBar.bottom,
			rcClient.right - rcClient.left,
			rcClient.bottom - rcClient.top - (rectStatusBar.bottom - rectStatusBar.top) - (rectReBar.bottom - rectReBar.top),
			this->m_hWnd,
			NULL,//(this->m_hMenuContext)IDM_CODE_SAMPLES,
			this->m_hInstance,
			NULL );

		HWND hwndHeader = ListView_GetHeader( this->m_hWndListView );
		SetWindowTheme( hwndHeader, NULL, L"TrackBar" );//Combobox, GenericPreviewPaneControl::Edit, Communications::Combobox

		//
		// Initialize ListView
		//			
		ListView_SetExtendedListViewStyleEx( this->m_hWndListView, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT );
		ListView_SetBkColor( this->m_hWndListView, g_LISTVIEW_BK_COLOR );
		ListView_SetTextBkColor( this->m_hWndListView, g_LISTVIEW_TEXT_BK_COLOR );
		ListView_SetTextColor( this->m_hWndListView, g_LISTVIEW_TEXT_COLOR );

		//
		// Create the image list.
		//		
		this->m_hImageListListView = ImageList_Create( 16, // size x
			16, // size y
			ILC_COLOR24 | ILC_MASK,   // Ensures transparent background.
			1,	// 2 buttons
			0 );

		//
		// Load Bitmap Refresh and add to image list.
		//
		this->m_hBmpCpu = LoadBitmap( this->m_hInstance, MAKEINTRESOURCE( IDB_CPU ) );
		ImageList_Add( this->m_hImageListListView, this->m_hBmpCpu, NULL );
		this->m_hBmpMemory = LoadBitmap( this->m_hInstance, MAKEINTRESOURCE( IDB_MEMORY ) );
		ImageList_Add( this->m_hImageListListView, this->m_hBmpMemory, NULL );
		ListView_SetImageList( this->m_hWndListView, this->m_hImageListListView, LVSIL_SMALL );

		//
		// Setup Columns
		//
		LVCOLUMN col0 = { 0 };
		col0.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_MINWIDTH | LVCF_FMT;
		col0.fmt = LVCFMT_FIXED_WIDTH;
		col0.pszText = const_cast<LPWSTR>(L"Type");
		col0.cchTextMax = 12;				
		col0.cx = 190;
		col0.cxMin = 190;
		ListView_InsertColumn( this->m_hWndListView, 1, &col0 );

		//LVCOLUMN col1 = { 0 };
		//col1.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_MINWIDTH | LVCF_FMT;
		//col1.fmt = LVCFMT_FIXED_WIDTH;
		//col1.pszText = L"@";
		//col1.cchTextMax = 10;
		//col1.cx = 90;
		//col1.cxMin = 90;
		//ListView_InsertColumn( this->m_hWndListView, 2, &col1 );

		LVCOLUMN col2 = { 0 };
		col2.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_MINWIDTH | LVCF_FMT;
		col2.fmt = LVCFMT_FIXED_WIDTH;
		col2.pszText = const_cast<LPWSTR>(L"Value");
		col2.cchTextMax = 6;
		col2.cx = 320;
		col2.cxMin = 320;
		ListView_InsertColumn( this->m_hWndListView, 2, &col2 );

		LVCOLUMN col3 = { 0 };
		col3.mask = LVCF_TEXT | LVCF_WIDTH | LVCF_MINWIDTH | LVCF_FMT;
		col3.fmt = LVCFMT_FIXED_WIDTH;
		col3.pszText = const_cast<LPWSTR>(L"Status");
		col3.cchTextMax = 14;
		col3.cx = 560;
		col3.cxMin = 560;
		ListView_InsertColumn( this->m_hWndListView, 3, &col3 );		
	}

	bool CApplication::GetDevicePath( LPCGUID GUID_DEVINTEFACE, PWCHAR DevicePath, size_t BufLen )
	{
		CONFIGRET cr = CR_SUCCESS;
		PWSTR deviceInterfaceList = NULL;
		ULONG deviceInterfaceListLength = 0;
		PWSTR nextInterface;
		HRESULT hr = E_FAIL;
		bool bRet = true;

		cr = CM_Get_Device_Interface_List_Size(
			&deviceInterfaceListLength,
			const_cast<LPGUID>(GUID_DEVINTEFACE),
			NULL,
			CM_GET_DEVICE_INTERFACE_LIST_PRESENT );
		if ( cr != CR_SUCCESS )
		{
			wsprintfW( DevicePath, L"Error 0x%X retrieving device interface list size.", cr );
			goto clean0;
		}

		if ( deviceInterfaceListLength <= 1 )
		{
			bRet = FALSE;
			wsprintfW( DevicePath, L"No active device interfaces found. Is the driver loaded?" );
			goto clean0;
		}

		deviceInterfaceList = (PWSTR)malloc( deviceInterfaceListLength * sizeof( WCHAR ) );
		if ( deviceInterfaceList == NULL )
		{
			wsprintfW( DevicePath, L"Error allocating memory for device interface list." );
			goto clean0;
		}
		ZeroMemory( deviceInterfaceList, deviceInterfaceListLength * sizeof( WCHAR ) );

		cr = CM_Get_Device_Interface_List(
			const_cast<LPGUID>(GUID_DEVINTEFACE),
			NULL,
			deviceInterfaceList,
			deviceInterfaceListLength,
			CM_GET_DEVICE_INTERFACE_LIST_PRESENT );
		if ( cr != CR_SUCCESS )
		{
			wsprintfW( DevicePath, L"Error 0x%X retrieving device interface list.", cr );
			goto clean0;
		}

		nextInterface = deviceInterfaceList + wcslen( deviceInterfaceList ) + 1;
		if ( *nextInterface != UNICODE_NULL )
		{
			wsprintfW( DevicePath, L"Warning: More than one device interface instance found. Selecting first matching device." );
		}

		hr = StringCchCopyW( DevicePath, BufLen, deviceInterfaceList );
		if ( FAILED( hr ) )
		{
			bRet = FALSE;
			wsprintfW( DevicePath, L"Error: StringCchCopy failed with HRESULT 0x%x", hr );
			goto clean0;
		}

	clean0:
		if ( deviceInterfaceList != NULL )
		{
			free( deviceInterfaceList );
		}
		if ( CR_SUCCESS != cr )
		{
			bRet = false;
		}

		return bRet;
	}// GetDevicePath

	//
	// Function: PopulateListView
	//
	void CApplication::PopulateListView()
	{		
		EnterCriticalSection( &this->m_hCriticalSection );

		//
		// Remove all items
		//
		ListView_DeleteAllItems( this->m_hWndListView );

		//
		// Fetch Values
		//
		IS_KEYBOARD_RKT_OUTPUT_DATA outputData = { 0 };

		if (this->m_bUseDummyData) 
		{
			outputData.cbSize = sizeof(IS_KEYBOARD_RKT_OUTPUT_DATA);
			outputData.dwApicBaseAddress = 0xFEE00C00;
			outputData.dwRootComplexBaseAddress = 0xFFFFFC00;
			outputData.dwIoApicBaseAddress = 0xFEC00000;			
			outputData.qwIOAPIC_REDTBL[1] = 0x0000000000010000;
			outputData.qwIOTRn[0] = 0x80b9ffca83000012;
			outputData.qwIOTRn[1] = 0x00001261e800003;
			outputData.qwIOTRn[2] = 0x8CE8B00000320BE;
			outputData.qwIOTRn[3] = 0x020000D00001222;			
		}
		else
		{
			wchar_t completeDeviceName[MAX_PATH];
			if (!GetDevicePath(const_cast<LPCGUID>(&GUID_DEVINTERFACE_IsKeyboardRootkittedDriver), completeDeviceName, sizeof(completeDeviceName) / sizeof(completeDeviceName[0])))
			{
				wstring text(L"Get device path failed.\n");
				text += L"GetLastError=";
				text += ItsError::GetLastErrorDescription();				
				text += completeDeviceName;				
				MessageBox(NULL, text.c_str(), L"Error", MB_OK | MB_ICONERROR);
				LeaveCriticalSection(&this->m_hCriticalSection);
				return;
			}

			HANDLE hDevice = CreateFileW(completeDeviceName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_READ_ACCESS, NULL);
			if (hDevice == INVALID_HANDLE_VALUE)
			{
				MessageBox(NULL, L"Error creating file for device", L"Error", MB_OK | MB_ICONERROR);
				LeaveCriticalSection(&this->m_hCriticalSection);
				return;
			}

			DWORD dwNumBytesRead(0);
			BOOL bResult = ReadFile(hDevice, &outputData, sizeof(IS_KEYBOARD_RKT_OUTPUT_DATA), &dwNumBytesRead, NULL);
			if (!bResult)
			{
				CloseHandle(hDevice);
				MessageBox(NULL, L"Communicating with Device... [ERROR]", L"Error", MB_OK | MB_ICONERROR);
				LeaveCriticalSection(&this->m_hCriticalSection);
				return;
			}

			if (dwNumBytesRead != sizeof(IS_KEYBOARD_RKT_OUTPUT_DATA))
			{
				MessageBox(NULL, L"Checking data returned of correct size... [ERROR]", L"Error", MB_OK | MB_ICONERROR);
				CloseHandle(hDevice);
				LeaveCriticalSection(&this->m_hCriticalSection);
				return;
			}

			if (outputData.ntStatusCode != 0)
			{
				MessageBox(NULL, L"Checking if Driver call was a success... [ERROR]", L"Error", MB_OK | MB_ICONERROR);
				CloseHandle(hDevice);
				LeaveCriticalSection(&this->m_hCriticalSection);
				return;
			}
		}

		//
		// Insert Items
		//
		bool bKbdIsTrapped( false );

		int row = 0;

		wstring txtRowCol1( L"I/O APIC" );
		LVITEM item0 = { 0 };
		item0.mask = LVIF_COLUMNS | LVIF_DI_SETITEM | LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT;
		item0.iItem = row;
		item0.iSubItem = 0;
		item0.iImage = 0;		
		item0.iIndent = 0;
		item0.cchTextMax = (int)txtRowCol1.size() + 1;
		item0.pszText = const_cast<wchar_t*>(txtRowCol1.c_str());
		ListView_InsertItem( this->m_hWndListView, &item0 );

		row++;

		for ( int i = 0; i < IO_APIC_IRQ_COUNT; row++, i++ )
		{			
			if ( i != 1 )
			{
				row--;
				continue;
			}

			wstringstream	txtRowCol2;
			txtRowCol2 << L"IRQ" << i;
			wstring txtCol2 = txtRowCol2.str();

			LVITEM item1 = { 0 };
			item1.mask = LVIF_COLUMNS | LVIF_DI_SETITEM | LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT;
			item1.iItem = row;
			item1.iSubItem = 0;
			item1.iIndent = 1;
			item1.iImage = 1;
			item1.cchTextMax = (int)txtCol2.size() + 1;
			item1.pszText = const_cast<wchar_t*>(txtCol2.c_str());
			ListView_InsertItem( this->m_hWndListView, &item1 );			

			wstringstream txtRowCol3;
			txtRowCol3 << L"0x" << std::hex << std::setw( 16 ) << std::setfill( L'0' ) << std::uppercase << outputData.qwIOAPIC_REDTBL[i];
			wstring txt = txtRowCol3.str();

			LVITEM item2 = { 0 };
			item2.mask = LVIF_DI_SETITEM | LVIF_TEXT;
			item2.iItem = row;
			item2.iSubItem = 1;			
			item2.iIndent = 1;			
			item2.cchTextMax = (int)txt.size() + 1;
			item2.pszText = const_cast<wchar_t*>(txt.c_str());
			ListView_SetItem( this->m_hWndListView, &item2 );

			wstringstream status;
			if ((outputData.qwIOAPIC_REDTBL[i] & 0b0111'0000'0000) == 0b0010'0000'0000) 
			{				
				status << L"Keyboard Is Trapped by SMI Handler on I/O APIC - IRQ" << i;
				bKbdIsTrapped = true;
			}			
			else
			{
				status << L"OK";
			}

			wstring txtStatus = status.str();

			LVITEM item3 = { 0 };
			item3.mask = LVIF_DI_SETITEM | LVIF_TEXT;
			item3.iItem = row;
			item3.iSubItem = 2;
			item3.iIndent = 1;
			item3.cchTextMax = (int)txtStatus.size() + 1;
			item3.pszText = const_cast<wchar_t*>(txtStatus.c_str());
			ListView_SetItem( this->m_hWndListView, &item3 );
		}		
		
		wstring txtRowCol1_2( L"IOTRn" );
		LVITEM item0_2 = { 0 };
		item0_2.mask = LVIF_COLUMNS | LVIF_DI_SETITEM | LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT;
		item0_2.iItem = row;
		item0_2.iSubItem = 0;
		item0_2.iImage = 0;		
		item0_2.iIndent = 0;
		item0_2.cchTextMax = (int)txtRowCol1_2.size() + 1;
		item0_2.pszText = const_cast<wchar_t*>(txtRowCol1_2.c_str());
		ListView_InsertItem( this->m_hWndListView, &item0_2 );

		row++;

		for ( int i = 0; i < IOTRn_COUNT; row++, i++ )
		{			
			wstringstream txtRowCol2;
			txtRowCol2 << L"IOTR" << i;
			wstring txtCol2 = txtRowCol2.str();

			LVITEM item1 = { 0 };
			item1.mask = LVIF_COLUMNS | LVIF_DI_SETITEM | LVIF_TEXT | LVIF_IMAGE | LVIF_INDENT;
			item1.iItem = row;
			item1.iSubItem = 0;
			item1.iIndent = 1;
			item1.iImage = 1;
			item1.cchTextMax = (int)txtCol2.size() + 1;
			item1.pszText = const_cast<wchar_t*>(txtCol2.c_str());
			ListView_InsertItem( this->m_hWndListView, &item1 );
			
			wstringstream txtRowCol3;
			txtRowCol3 << L"0x" << std::hex << std::setw( 16 ) << std::setfill( L'0' ) << std::uppercase << outputData.qwIOTRn[i];
			wstring txt = txtRowCol3.str();

			LVITEM item2 = { 0 };
			item2.mask = LVIF_DI_SETITEM | LVIF_TEXT;
			item2.iItem = row;
			item2.iSubItem = 1;
			item2.cchTextMax = (int)txt.size() + 1;
			item2.pszText = const_cast<wchar_t*>(txt.c_str());
			ListView_SetItem( this->m_hWndListView, &item2 );

			wstringstream status; 
			if ( (DWORD)outputData.qwIOTRn[row] == 0x61 )
			{
				status << "Keyboard Is Trapped by SMI Handler on " << L"IOTR" << i << L" I/O Address(IOAD) 0x61";
				bKbdIsTrapped = true;
			}
			else if ( (DWORD)outputData.qwIOTRn[row] == 0x65 )
			{
				status << "Keyboard Is Trapped by SMI Handler on " << L"IOTR" << i << L" I/O Address(IOAD) 0x65";
			}
			else
			{
				status << L"OK";
			}

			wstring txtStatus = status.str();

			LVITEM item3 = { 0 };
			item3.mask = LVIF_DI_SETITEM | LVIF_TEXT;
			item3.iItem = row;
			item3.iSubItem = 2;
			item3.cchTextMax = (int)txtStatus.size() + 1;
			item3.pszText = const_cast<wchar_t*>(txtStatus.c_str());
			ListView_SetItem( this->m_hWndListView, &item3 );
		}
		
		LeaveCriticalSection( &this->m_hCriticalSection );

		if ( bKbdIsTrapped )
		{
			this->SetStatusBarText( L"WARNING! A SMI Keyboard Handler was found!" );
		}
		else
		{
			this->SetStatusBarText( L"Did not detect any trapped SMI Keyboard Handler!" );
		}
	}

	//
	// Methoid: GetFileVersion
	//
	void CApplication::GetFileVersion(wstring fileName, unsigned short& major, unsigned short& minor, unsigned short& build, unsigned short& revision)
	{
		DWORD dwSize = GetFileVersionInfoSizeEx( FILE_VER_GET_NEUTRAL, fileName.c_str(), NULL );
		BYTE* pData = new BYTE[dwSize];
		GetFileVersionInfoEx( FILE_VER_GET_NEUTRAL, fileName.c_str(), NULL, dwSize, pData );

		LPVOID pVersionInfo( NULL );
		UINT uiLen( 0 );
		VerQueryValue( pData, L"\\", &pVersionInfo, &uiLen );

		VS_FIXEDFILEINFO* pVerInfo = (VS_FIXEDFILEINFO*)pVersionInfo;

		DWORD dwMajorVersionMsb = HIWORD( pVerInfo->dwFileVersionMS );
		DWORD dwMajorVersionLsb = LOWORD( pVerInfo->dwFileVersionMS );
		DWORD dwMinorVersionMsb = HIWORD( pVerInfo->dwFileVersionLS );
		DWORD dwMinorVersionLsb = LOWORD( pVerInfo->dwFileVersionLS );

		major = (unsigned short)dwMajorVersionMsb;
		minor = (unsigned short)dwMajorVersionLsb;
		build = (unsigned short)dwMinorVersionMsb;
		revision = (unsigned short)dwMinorVersionLsb;
	}

	//
	// Method: GetFileVersion
	//
	wstring CApplication::GetFileVersion()
	{
		wchar_t wszFileName[MAX_PATH];
		GetModuleFileName( this->m_hInstance, wszFileName, MAX_PATH );

		DWORD dwSize = GetFileVersionInfoSizeEx( FILE_VER_GET_NEUTRAL, wszFileName, NULL );
		BYTE* pData = new BYTE[dwSize];
		GetFileVersionInfoEx( FILE_VER_GET_NEUTRAL, wszFileName, NULL, dwSize, pData );

		LPVOID pVersionInfo( NULL );
		UINT uiLen( 0 );
		VerQueryValue( pData, L"\\", &pVersionInfo, &uiLen );

		VS_FIXEDFILEINFO* pVerInfo = (VS_FIXEDFILEINFO*)pVersionInfo;

		DWORD dwMajorVersionMsb = HIWORD( pVerInfo->dwFileVersionMS );
		DWORD dwMajorVersionLsb = LOWORD( pVerInfo->dwFileVersionMS );
		DWORD dwMinorVersionMsb = HIWORD( pVerInfo->dwFileVersionLS );
		DWORD dwMinorVersionLsb = LOWORD( pVerInfo->dwFileVersionLS );

		wstringstream wss;
		wss << dwMajorVersionMsb << L"." << dwMajorVersionLsb << L"." << dwMinorVersionMsb << L"." << dwMinorVersionLsb;

		wstring retVal;
		wss >> retVal;

		delete[] pData;

		return retVal;
	}


}// namespace