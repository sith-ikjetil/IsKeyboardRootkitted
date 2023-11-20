#pragma once
#ifndef __CAPPLICATION_H__
#define __CAPPLICATION_H__
//
// #include
//
#include "../stdafx.h"
#include <vector>
#include <memory>
#include <shellapi.h>
#include <windows.h>
#include <CommCtrl.h>
#include "./Dialog/CAboutDlg.h"
//#include "../../Steam/sdk/public/steam/steam_api.h"

//
// using namespace
//
using namespace std;

//
// #define
//
#define MAX_LOADSTRING								100
#define RACECORE24_APPLICATION_NAME					L"Is Keyboard Rootkitted Application x64 { by Kjetil Kristoffer Solberg (post@ikjetil.no) }"
#define RACECORE24_NOTIFYICON_CALLBACK_MESSAGE		WM_USER + 3
#define RACECORE24_NOTIFYICON_ICONID				WM_USER + 3

//
// namespace
//
namespace KKS
{
	//
	// Class: CApplication
	//
	class CApplication
	{
	private:
		//
		// Variables
		//
		HINSTANCE					m_hInstance; 
		HWND						m_hWnd;
		HWND						m_hWndListView;
		HWND						m_hWndStatusBar;
		HWND						m_hWndProgressBar;
		HWND						m_hWndToolBar;
		HWND						m_hWndToolBarMenu;
		HWND						m_hWndReBar;
		int							m_iItemSelected;
		WCHAR						m_wszTitle[MAX_LOADSTRING];                  // The title bar text
		WCHAR						m_wszWindowClass[MAX_LOADSTRING];            // the main window class name
		NOTIFYICONDATA				m_notifyIconData;
		HBITMAP						m_hBmpCpu;
		HBITMAP						m_hBmpMemory;
		CRITICAL_SECTION			m_hCriticalSection;
		HICON						m_hIconStatusArea;
		UINT						m_uID;
		HIMAGELIST					m_hImageListToolBar;
		HIMAGELIST					m_hImageListListView;
		HBITMAP						m_hBmpFlagGreen;
		HBITMAP						m_hBmpRefresh;
		HBITMAP						m_hBmpCalibrate;		
		HMENU						m_hMenu;
		HMENU						m_hMenuContext;
		HMENU						m_hSubMenuContext;
		UINT						m_uTaskbarRestart;		
		BOOL						m_bPerformUpdate;
		// STEAM -- begin
		/*HAuthTicket					m_hAuthTicketIGaveThisUser;
		uint32						m_cubTicketIGaveThisUser;
		uint8						m_rgubTicketIGaveThisUser[1024];*/
		bool						m_bSteamWeAreAuthenticated;
		// STEAM -- end
		HANDLE						m_hUpdaterMutex;
		unique_ptr<CAboutDlg>		m_pCAboutDlg;

		//
		// Methods
		//
		ATOM MyRegisterClass(HINSTANCE hInstance);
		BOOL InitInstance(HINSTANCE hInstance, int nCmdShow);

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		bool AuthenticateSteamUser();
		bool ReleaseSteamUser();
		
		void PopulateListView();

		void CreateListView();
		void CreateStatusBar();
		
		
		

		void SetStatusBarText( wchar_t* pwszText );
		void SetStatusBarText( const wchar_t* pwszText );
		
		
		
		void AddStatusAreaIcon();
		void RemoveStatusBarIcon();

		

		wstring GetFileVersion();
		void GetFileVersion( wstring fileName, unsigned short& major, unsigned short& minor, unsigned short& build, unsigned short& revision );
		

		//
		// Window Message Handlers
		//
		void OnWM_CREATE( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		bool OnWM_COMMAND( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		void OnWM_PAINT( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		void OnWM_SYSCOMMAND( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		void OnWM_CLOSE( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		void OnWM_DESTROY( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );
		void OnWM_SIZE( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );

		LRESULT OnRACECORE24_NOTIFYICON_CALLBACK_MESSAGE( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam );				

		bool GetDevicePath( LPCGUID GUID_DEVINTEFACE, PWCHAR DevicePath, size_t BufLen );
		
		
		friend class CAboutDlg;
		

	protected:
	public:
		

		//
		// Methods
		//
		int WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,LPWSTR lpCmdLine, int nCmdShow);
	};
}

#endif