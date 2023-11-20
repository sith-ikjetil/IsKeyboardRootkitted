#ifndef __CABOUTDLG_H__
#define __CABOUTDLG_H__

//
// #define
//
#define ABOUT_DIALOG_VERSION_TEXT	L"Is Keyboard Rootkitted, Version "

//
// namespace
//
namespace KKS
{
	class CApplication;

	//
	// Class: CAboutDlg
	//
	class CAboutDlg
	{
	private:		
		CApplication* m_pCApplication;
		HWND m_hWnd;
		static INT_PTR CALLBACK WndProc( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam );
	public:
		CAboutDlg( CApplication* pCApplication );
		void ShowDialog();
	};
}

#endif