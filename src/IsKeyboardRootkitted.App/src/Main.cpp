//
// #include
//
#include "stdafx.h"
#include "../../Include/itsoftware-com.h"
#include "CApplication.h"

//
// #pragma
//
#pragma comment(linker, "/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture = '*' publicKeyToken = '6595b64144ccf1df' language = '*'\"")

//
// shared data segment for global instance count.
//
#pragma data_seg ("shrdEXE")
long g_lInstanceCount = 0;
#pragma data_seg()
#pragma comment(linker,"/section:shrdEXE,rws")

//
// using namespace
//
using namespace KKS;
using namespace ItSoftware;
using namespace ItSoftware::COM;

//
// Function: wWinMain
//
// (i): Application Entry Point
//
int APIENTRY wWinMain( _In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow )
{
	//
	// Check for multiple processes running.
	//
	long instanceCount = InterlockedIncrement( &g_lInstanceCount );
	if ( instanceCount > 1 )
	{
		InterlockedDecrement( &g_lInstanceCount );
		MessageBox( NULL, L"Only one instance of Is Keyboard Rootkitted is allowed to run at a single time.", L"Information", MB_OK | MB_ICONINFORMATION );
		return 1;
	}

	//
	// Initialize COM runtime
	//
	ComRuntime runtime( ComApartment::ApartmentThreaded );

	//
	// Create Application object and forward call
	//
	CApplication app;
	return app.WinMain( hInstance, hPrevInstance, lpCmdLine, nCmdShow );
}