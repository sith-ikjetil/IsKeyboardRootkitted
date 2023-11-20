//
// #include
//
#include "stdafx.h"
#include <string>
#include <sstream>
#include "../../Resource.h"
#include "CAboutDlg.h"
#include "../CApplication.h"

//
// using namespace
//
using namespace std;

//
// namespace
//
namespace KKS
{	
	//
	// Constructor
	//
	CAboutDlg::CAboutDlg( CApplication* pCApplication )
		: m_pCApplication(pCApplication)
	{		
	}

	//
	// Method: ShowDialog
	//
	void CAboutDlg::ShowDialog()
	{
		DialogBoxParam( this->m_pCApplication->m_hInstance, MAKEINTRESOURCE( IDD_ABOUTBOX ), this->m_pCApplication->m_hWnd, WndProc, reinterpret_cast<LPARAM>(this) );
	}

	//
	// Method: WndProc
	//
	INT_PTR CALLBACK CAboutDlg::WndProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
	{
		UNREFERENCED_PARAMETER( lParam );
		static CAboutDlg* pCAbout( nullptr );

		switch ( message )
		{
		case WM_INITDIALOG:
		{
			//LPCREATESTRUCT pCS = reinterpret_cast<LPCREATESTRUCT>(lParam);
			pCAbout = reinterpret_cast<CAboutDlg*>(lParam);

			RECT rectParent = { 0 };
			GetWindowRect( pCAbout->m_pCApplication->m_hWnd, &rectParent );

			RECT rectDlg = { 0 };
			GetWindowRect( hDlg, &rectDlg );

			LONG x = rectParent.left + ((rectParent.right - rectParent.left) / 2);
			LONG y = rectParent.top + ((rectParent.bottom - rectParent.top) / 2);

			x -= ((rectDlg.right - rectDlg.left) / 2);
			y -= ((rectDlg.bottom - rectDlg.top) / 2);

			MoveWindow( hDlg, x, y, (rectDlg.right - rectDlg.left), (rectDlg.bottom - rectDlg.top), TRUE );

			wstring &&version = pCAbout->m_pCApplication->GetFileVersion();
			wchar_t wszText[100];
			wcscpy_s( wszText, 100, ABOUT_DIALOG_VERSION_TEXT );
			wcscat_s( wszText, 100, version.c_str() );
			SetDlgItemText( hDlg, IDC_STATIC_VERSION, wszText );

			SYSTEMTIME st{0};
			GetSystemTime(&st);

			wstringstream ss;
			ss << L"Copyright (c) 2015-" << st.wYear << L" Kjetil Kristoffer Solberg" << L" @ IT Software";
			
			wstring copyright = ss.str();
			SetDlgItemText(hDlg, IDC_STATIC_COPYRIGHT, copyright.c_str());

			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
			if ( LOWORD( wParam ) == IDOK || LOWORD( wParam ) == IDCANCEL )
			{
				EndDialog( hDlg, LOWORD( wParam ) );
				return (INT_PTR)TRUE;
			}
			break;
		}
		return (INT_PTR)FALSE;
	}
}