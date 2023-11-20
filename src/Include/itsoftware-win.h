///////////////////////////////////////////////////////////////////
//: Title        : itsoftware-win.h
//: Product:     : Cpp.Include.Windows
//: Date         : 2020-05-01
//: Author       : "Kjetil Kristoffer Solberg" <post@ikjetil.no>
//: Version      : 1.0.0.0
//: Descriptions : Implementation of Cpp.Include.Windows. Uses Shlwapi.lib.
#pragma once
//
// #include
//
#include <Windows.h>
#include <WinUser.h>
#include <CommCtrl.h>
#include <memory>
#include <string>
#include <wincrypt.h>
#include <Shlwapi.h>
#include <vector>
#include <functional>
#include <thread>
#include "itsoftware.h"
#include "itsoftware-exceptions.h"
#include <time.h>

//
// #pragma
//
#pragma warning(disable: 4244)

//
// namespace
//
namespace ItSoftware::Win
{
	//
	// using
	//
	using std::unique_ptr;
	using std::vector;
	using std::wstring;
	using std::make_unique;
	using std::function;
	using std::thread;
	using ItSoftware::ItsString;

	//
	// Variable: hCBTHook for ItsWin
	//
	inline HHOOK g_hCBTHook = nullptr;

	//
	// struct: ItsWin
	//
	// (i): Misc Windows routines.
	//
	struct ItsWin
	{
	private:
		//
		// Function: CBTMsgBox
		//
		static LRESULT CALLBACK CBTMsgBox(int code, WPARAM wParam, LPARAM lParam)
		{
			if (HCBT_ACTIVATE != code || -1 == code)
			{
				// only when HCBT_ACTIVATE 
				return CallNextHookEx(g_hCBTHook, code, wParam, lParam);
			}

			HWND hWnd = (HWND)wParam;
			UnhookWindowsHookEx(g_hCBTHook);
			CenterToItsParentWindow(hWnd);

			return CallNextHookEx(g_hCBTHook, code, wParam, lParam);
		}

	public:

		//
		//
		//
		static wstring ToRGBString(COLORREF color)
		{
			wstringstream ss;

			ss << (unsigned char)((color & 0x0000FF));
			ss << L",";
			ss << (unsigned char)((color & 0x00FF00) >> 8);
			ss << L",";
			ss << (unsigned char)((color & 0xFF0000) >> 16);

			return ss.str();
		}

		//
		// Function: CenterToParentWindow
		//
		static void CenterToItsParentWindow(HWND hDlg)
		{
			RECT rcDlg;
			RECT rcOwner;

			//center
			HWND hWndOwner = ::GetWindow(hDlg, GW_OWNER);
			GetWindowRect(hDlg, &rcDlg);
			GetWindowRect(hWndOwner, &rcOwner);

			int iLeft = rcOwner.left + ((rcOwner.right - rcOwner.left)
				- (rcDlg.right - rcDlg.left)) / 2;

			int iTop = rcOwner.top + ((rcOwner.bottom - rcOwner.top)
				- (rcDlg.bottom - rcDlg.top)) / 2;

			SetWindowPos(hDlg, hWndOwner, iLeft, iTop, 0, 0, SWP_NOSIZE | SWP_FRAMECHANGED);
		}

		//
		// ## ShowInformationMessageXXX #################################
		//

		//
		// Function: ShowInformationMessage
		//
		static void ShowInformationMessage(wstring message)
		{
			ShowInformationMessage(NULL, L"Information!", message);
		}

		//
		// Function: ShowInformationMessage
		//
		static void ShowInformationMessage(wstring title, wstring message)
		{
			ShowInformationMessage(NULL, title, message);
		}

		//
		// Function: ShowInformationMessage
		//
		static void ShowInformationMessage(HWND hWnd, wstring message)
		{
			ShowInformationMessage(hWnd, L"Information!", message);
		}

		//
		// Function: ShowInformationMessage
		//
		static void ShowInformationMessage(HWND hWnd, wstring title, wstring message)
		{
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				MessageBox(hWnd, message.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				MessageBox(hWnd, message.c_str(), title.c_str(), MB_OK | MB_ICONINFORMATION);
			}
		}

		//
		// Function: ShowInformationMessageYesNo
		//
		static int ShowInformationMessageYesNo(wstring message)
		{
			return ShowInformationMessageYesNo(NULL, L"Information!", message);
		}

		//
		// Function: ShowInformationMessageYesNo
		//
		static int ShowInformationMessageYesNo(wstring title, wstring message)
		{
			return ShowInformationMessageYesNo(NULL, title, message);
		}

		//
		// Function: ShowInformationMessageYesNo
		//
		static int ShowInformationMessageYesNo(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONINFORMATION | MB_YESNO);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONINFORMATION | MB_YESNO);
			}

			return result;
		}

		//
		// Function: ShowInformationMessageYesNoCancel
		//
		static int ShowInformationMessageYesNoCancel(wstring message)
		{
			return ShowInformationMessageYesNoCancel(NULL, L"Information!", message);
		}

		//
		// Function: ShowInformationMessageYesNoCancel
		//
		static int ShowInformationMessageYesNoCancel(wstring title, wstring message)
		{
			return ShowInformationMessageYesNoCancel(NULL, title, message);
		}

		//
		// Function: ShowInformationMessageYesNoCancel
		//
		static int ShowInformationMessageYesNoCancel(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONINFORMATION | MB_YESNOCANCEL);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONINFORMATION | MB_YESNOCANCEL);
			}

			return result;
		}

		//
		// Function: ShowInformationMessageAbortRetryIgnore
		//
		static int ShowInformationMessageAbortRetryIgnore(wstring message)
		{
			return ShowInformationMessageAbortRetryIgnore(NULL, L"Information!", message);
		}

		//
		// Function: ShowInformationMessageAbortRetryIgnore
		//
		static int ShowInformationMessageAbortRetryIgnore(wstring title, wstring message)
		{
			return ShowInformationMessageAbortRetryIgnore(NULL, title, message);
		}

		//
		// Function: ShowWarningMessageAbortRetryIgnore
		//
		static int ShowInformationMessageAbortRetryIgnore(HWND hWnd, wstring message)
		{
			return ShowInformationMessageAbortRetryIgnore(hWnd, L"Information!", message);
		}

		//
		// Function: ShowInformationMessageAbortRetryIgnore
		//
		static int ShowInformationMessageAbortRetryIgnore(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONINFORMATION | MB_ABORTRETRYIGNORE);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONINFORMATION | MB_ABORTRETRYIGNORE);
			}

			return result;
		}

		//
		// ## ShowWarningMessageXXX ##################################
		//

		//
		// ShowWarningMessage
		//
		static void ShowWarningMessage(wstring message)
		{
			ShowWarningMessage(NULL, L"Warning!", message);
		}

		//
		// ShowWarningMessage
		//
		static void ShowWarningMessage(wstring title, wstring message)
		{
			ShowWarningMessage(NULL, title, message);
		}

		//
		// ShowWarningMessage
		//
		static void ShowWarningMessage(HWND hWnd, wstring message)
		{
			ShowWarningMessage(hWnd, L"Warning!", message);
		}

		//
		// Function: ShowWarningMessage
		//
		static void ShowWarningMessage(HWND hWnd, wstring title, wstring message)
		{
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				MessageBox(hWnd, message.c_str(), title.c_str(), MB_OK | MB_ICONWARNING);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				MessageBox(hWnd, message.c_str(), title.c_str(), MB_OK | MB_ICONWARNING);
			}
		}

		//
		// Function: ShowWarningMessageYesNo
		//
		static int ShowWarningMessageYesNo(wstring message)
		{
			return ShowWarningMessageYesNo(NULL, L"Warning!", message);
		}

		//
		// Function: ShowWarningMessageYesNo
		//
		static int ShowWarningMessageYesNo(wstring title, wstring message)
		{
			return ShowWarningMessageYesNo(NULL, title, message);
		}

		//
		// Function: ShowWarningMessageYesNo
		//
		static int ShowWarningMessageYesNo(HWND hWnd, wstring message)
		{
			return ShowWarningMessageYesNo(hWnd, L"Warning!", message);
		}

		//
		// Function: ShowWarningMessageYesNo
		//
		static int ShowWarningMessageYesNo(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONWARNING | MB_YESNO);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONWARNING | MB_YESNO);
			}

			return result;
		}

		//
		// Function: ShowWarningMessageYesNoCancel
		//
		static int ShowWarningMessageYesNoCancel(wstring message)
		{
			return ShowWarningMessageYesNoCancel(NULL, L"Warning!", message);
		}

		//
		// Function: ShowWarningMessageYesNoCancel
		//
		static int ShowWarningMessageYesNoCancel(wstring title, wstring message)
		{
			return ShowWarningMessageYesNoCancel(NULL, title, message);
		}

		//
		// Function: ShowWarningMessageYesNoCancel
		//
		static int ShowWarningMessageYesNoCancel(HWND hWnd, wstring message)
		{
			return ShowWarningMessageYesNoCancel(hWnd, L"Warning!", message);
		}

		//
		// Function: ShowWarningMessageYesNoCancel
		//
		static int ShowWarningMessageYesNoCancel(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONWARNING | MB_YESNO);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONWARNING | MB_YESNO);
			}

			return result;
		}

		//
		// Function: ShowWarningMessageAbortRetryIgnore
		//
		static int ShowWarningMessageAbortRetryIgnore(wstring message)
		{
			return ShowWarningMessageAbortRetryIgnore(NULL, L"Warning!", message);
		}

		//
		// Function: ShowWarningMessageAbortRetryIgnore
		//
		static int ShowWarningMessageAbortRetryIgnore(wstring title, wstring message)
		{
			return ShowWarningMessageAbortRetryIgnore(NULL, title, message);
		}

		//
		// Function: ShowWarningMessageAbortRetryIgnore
		//
		static int ShowWarningMessageAbortRetryIgnore(HWND hWnd, wstring message)
		{
			return ShowWarningMessageAbortRetryIgnore(hWnd, L"Warning!", message);
		}

		//
		// Function: ShowWarningMessageAbortRetryIgnore
		//
		static int ShowWarningMessageAbortRetryIgnore(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONWARNING | MB_ABORTRETRYIGNORE);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONWARNING | MB_ABORTRETRYIGNORE);
			}

			return result;
		}

		//
		// ## ShowHandMessageXXX #################################
		//

		//
		// Function: ShowHandMessage
		//
		static void ShowHandMessage(wstring message)
		{
			ShowHandMessage(NULL, L"Hand!", message);
		}

		//
		// Function: ShowHandMessage
		//
		static void ShowHandMessage(wstring title, wstring message)
		{
			ShowHandMessage(NULL, title, message);
		}

		//
		// Function: ShowHandMessage
		//
		static void ShowHandMessage(HWND hWnd, wstring message)
		{
			ShowHandMessage(hWnd, L"Hand!", message);
		}

		//
		// Function: ShowHandMessage
		//
		static void ShowHandMessage(HWND hWnd, wstring title, wstring message)
		{
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				MessageBox(hWnd, message.c_str(), title.c_str(), MB_OK | MB_ICONHAND);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				MessageBox(hWnd, message.c_str(), title.c_str(), MB_OK | MB_ICONHAND);
			}
		}

		//
		// Function: ShowHandMessageYesNo
		//
		static int ShowHandMessageYesNo(wstring message)
		{
			return ShowHandMessageYesNo(NULL, L"Hand!", message);
		}

		//
		// Function: ShowHandMessageYesNo
		//
		static int ShowHandMessageYesNo(wstring title, wstring message)
		{
			return ShowHandMessageYesNo(NULL, title, message);
		}

		//
		// Function: ShowHandMessageYesNo
		//
		static int ShowHandMessageYesNo(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONHAND | MB_YESNO);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONHAND | MB_YESNO);
			}

			return result;
		}

		//
		// Function: ShowHandMessageYesNoCancel
		//
		static int ShowHandMessageYesNoCancel(wstring message)
		{
			return ShowHandMessageYesNoCancel(NULL, L"Hand!", message);
		}

		//
		// Function: ShowHandMessageYesNoCancel
		//
		static int ShowHandMessageYesNoCancel(wstring title, wstring message)
		{
			return ShowHandMessageYesNoCancel(NULL, title, message);
		}

		//
		// Function: ShowHandMessageYesNoCancel
		//
		static int ShowHandMessageYesNoCancel(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONHAND | MB_YESNOCANCEL);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONHAND | MB_YESNOCANCEL);
			}

			return result;
		}

		//
		// Function: ShowHandMessageAbortRetryIgnore
		//
		static int ShowHandMessageAbortRetryIgnore(wstring message)
		{
			return ShowHandMessageAbortRetryIgnore(NULL, L"Hand!", message);
		}

		//
		// Function: ShowHandMessageAbortRetryIgnore
		//
		static int ShowHandMessageAbortRetryIgnore(wstring title, wstring message)
		{
			return ShowHandMessageAbortRetryIgnore(NULL, title, message);
		}

		//
		// Function: ShowHandMessageAbortRetryIgnore
		//
		static int ShowHandMessageAbortRetryIgnore(HWND hWnd, wstring message)
		{
			return ShowHandMessageAbortRetryIgnore(hWnd, L"Hand!", message);
		}

		//
		// Function: ShowHandMessageAbortRetryIgnore
		//
		static int ShowHandMessageAbortRetryIgnore(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONHAND | MB_ABORTRETRYIGNORE);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONHAND | MB_ABORTRETRYIGNORE);
			}

			return result;
		}

		//
		// ## ShowErrorMessageXXX ################################
		//

		//
		// Function: ShowErrorMessage
		//
		static void ShowErrorMessage(wstring message)
		{
			ShowErrorMessage(NULL, L"Error!", message);
		}

		//
		// Function: ShowErrorMessage
		//
		static void ShowErrorMessage(wstring title, wstring message)
		{
			ShowErrorMessage(NULL, title, message);
		}

		//
		// Function: ShowErrorMessage
		//
		static void ShowErrorMessage(HWND hWnd, wstring message)
		{
			ShowErrorMessage(hWnd, L"Error!", message);
		}

		//
		// Function: ShowErrorMessage
		//
		static void ShowErrorMessage(HWND hWnd, wstring title, wstring message)
		{
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				MessageBox(hWnd, message.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				MessageBox(hWnd, message.c_str(), title.c_str(), MB_OK | MB_ICONERROR);
			}
		}

		//
		// Function: ShowErrorMessageYesNo
		//
		static int ShowErrorMessageYesNo(wstring message)
		{
			return ShowErrorMessageYesNo(NULL, L"Error!", message);
		}

		//
		// Function: ShowErrorMessageYesNo
		//
		static int ShowErrorMessageYesNo(wstring title, wstring message)
		{
			return ShowErrorMessageYesNo(NULL, title, message);
		}

		//
		// Function: ShowErrorMessageYesNo
		//
		static int ShowErrorMessageYesNo(HWND hWnd, wstring message)
		{
			return ShowErrorMessageYesNo(hWnd, L"Error!", message);
		}

		//
		// Function: ShowErrorMessageYesNo
		//
		static int ShowErrorMessageYesNo(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONERROR | MB_YESNO);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONERROR | MB_YESNO);
			}

			return result;
		}

		//
		// Function: ShowErrorMessageYesNoCancel
		//
		static int ShowErrorMessageYesNoCancel(wstring message)
		{
			return ShowErrorMessageYesNoCancel(NULL, L"Error!", message);
		}

		//
		// Function: ShowErrorMessageYesNoCancel
		//
		static int ShowErrorMessageYesNoCancel(wstring title, wstring message)
		{
			return ShowErrorMessageYesNoCancel(NULL, title, message);
		}

		//
		// Function: ShowErrorMessageYesNoCancel
		//
		static int ShowErrorMessageYesNoCancel(HWND hWnd, wstring message)
		{
			return ShowErrorMessageYesNoCancel(hWnd, L"Error!", message);
		}

		//
		// Function: ShowErrorMessageYesNoCancel
		//
		static int ShowErrorMessageYesNoCancel(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONERROR | MB_YESNOCANCEL);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONERROR | MB_YESNOCANCEL);
			}

			return result;
		}

		//
		// Function: ShowErrorMessageAbortRetryIgnore
		//
		static int ShowErrorMessageAbortRetryIgnore(wstring message)
		{
			return ShowErrorMessageAbortRetryIgnore(NULL, L"Error!", message);
		}

		//
		// Function: ShowErrorMessageAbortRetryIgnore
		//
		static int ShowErrorMessageAbortRetryIgnore(wstring title, wstring message)
		{
			return ShowErrorMessageAbortRetryIgnore(NULL, title, message);
		}

		//
		// Function: ShowErrorMessageAbortRetryIgnore
		//
		static int ShowErrorMessageAbortRetryIgnore(HWND hWnd, wstring message)
		{
			return ShowErrorMessageAbortRetryIgnore(hWnd, L"Error!", message);
		}

		//
		// Function: ShowErrorMessageAbortRetryIgnore
		//
		static int ShowErrorMessageAbortRetryIgnore(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONERROR | MB_ABORTRETRYIGNORE);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONERROR | MB_ABORTRETRYIGNORE);
			}

			return result;
		}

		//
		// ## ShowQuestionMessageXXX
		//

		//
		// Function: ShowQuestionMessageYesNo
		//
		static int ShowQuestionMessageYesNo(wstring message)
		{
			return ShowQuestionMessageYesNo(NULL, L"Question!", message);
		}

		//
		// Function: ShowQuestionMessageYesNo
		//
		static int ShowQuestionMessageYesNo(wstring title, wstring message)
		{
			return ShowQuestionMessageYesNo(NULL, title, message);
		}

		//
		// Function: ShowQuestionMessageYesNo
		//
		static int ShowQuestionMessageYesNo(HWND hWnd, wstring message)
		{
			return ShowQuestionMessageYesNo(hWnd, L"Question!", message);
		}

		//
		// Function: ShowQuestionMessageYesNo
		//
		static int ShowQuestionMessageYesNo(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONQUESTION | MB_YESNO);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONQUESTION | MB_YESNO);
			}

			return result;
		}

		//
		// Function: ShowQuestionMessageYesNoCancel
		//
		static int ShowQuestionMessageYesNoCancel(wstring message)
		{
			return ShowQuestionMessageYesNoCancel(NULL, L"Question!", message);
		}

		//
		// Function: ShowQuestionMessageYesNoCancel
		//
		static int ShowQuestionMessageYesNoCancel(wstring title, wstring message)
		{
			return ShowQuestionMessageYesNoCancel(NULL, title, message);
		}

		//
		// Function: ShowQuestionMessageYesNoCancel
		//
		static int ShowQuestionMessageYesNoCancel(HWND hWnd, wstring message)
		{
			return ShowQuestionMessageYesNoCancel(hWnd, L"Question!", message);
		}

		//
		// Function: ShowQuestionMessageYesNoCancel
		//
		static int ShowQuestionMessageYesNoCancel(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONQUESTION | MB_YESNOCANCEL);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONQUESTION | MB_YESNOCANCEL);
			}

			return result;
		}

		//
		// Function: ShowQuestionMessageAbortRetryIgnore
		//
		static int ShowQuestionMessageAbortRetryIgnore(wstring message)
		{
			return ShowQuestionMessageAbortRetryIgnore(NULL, L"Question!", message);
		}

		//
		// Function: ShowQuestionMessageAbortRetryIgnore
		//
		static int ShowQuestionMessageAbortRetryIgnore(wstring title, wstring message)
		{
			return ShowQuestionMessageAbortRetryIgnore(NULL, title, message);
		}

		//
		// Function: ShowQuestionMessageAbortRetryIgnore
		//
		static int ShowQuestionMessageAbortRetryIgnore(HWND hWnd, wstring message)
		{
			return ShowQuestionMessageAbortRetryIgnore(hWnd, L"Question!", message);
		}

		//
		// Function: ShowQuestionMessageAbortRetryIgnore
		//
		static int ShowQuestionMessageAbortRetryIgnore(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONQUESTION | MB_ABORTRETRYIGNORE);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONQUESTION | MB_ABORTRETRYIGNORE);
			}

			return result;
		}

		//
		// ## ShowStopMessageXXX
		//

		//
		// Function: ShowStopMessageYesNo
		//
		static int ShowStopMessageYesNo(wstring message)
		{
			return ShowStopMessageYesNo(NULL, L"Stop!", message);
		}

		//
		// Function: ShowStopMessageYesNo
		//
		static int ShowStopMessageYesNo(wstring title, wstring message)
		{
			return ShowStopMessageYesNo(NULL, title, message);
		}

		//
		// Function: ShowStopMessageYesNo
		//
		static int ShowStopMessageYesNo(HWND hWnd, wstring message)
		{
			return ShowStopMessageYesNo(hWnd, L"Stop!", message);
		}

		//
		// Function: ShowStopMessageYesNo
		//
		static int ShowStopMessageYesNo(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONSTOP | MB_YESNO);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONSTOP | MB_YESNO);
			}

			return result;
		}

		//
		// Function: ShowStopMessageYesNoCancel
		//
		static int ShowStopMessageYesNoCancel(wstring message)
		{
			return ShowStopMessageYesNoCancel(NULL, L"Stop!", message);
		}

		//
		// Function: ShowStopMessageYesNoCancel
		//
		static int ShowStopMessageYesNoCancel(wstring title, wstring message)
		{
			return ShowStopMessageYesNoCancel(NULL, title, message);
		}

		//
		// Function: ShowStopMessageYesNoCancel
		//
		static int ShowStopMessageYesNoCancel(HWND hWnd, wstring message)
		{
			return ShowStopMessageYesNoCancel(hWnd, L"Stop!", message);
		}

		//
		// Function: ShowStopMessageYesNoCancel
		//
		static int ShowStopMessageYesNoCancel(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONSTOP | MB_YESNOCANCEL);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONSTOP | MB_YESNOCANCEL);
			}

			return result;
		}

		//
		// Function: ShowStopMessageAbortRetryIgnore
		//
		static int ShowStopMessageAbortRetryIgnore(wstring message)
		{
			return ShowStopMessageAbortRetryIgnore(NULL, L"Stop!", message);
		}

		//
		// Function: ShowStopMessageAbortRetryIgnore
		//
		static int ShowStopMessageAbortRetryIgnore(wstring title, wstring message)
		{
			return ShowStopMessageAbortRetryIgnore(NULL, title, message);
		}

		//
		// Function: ShowStopMessageAbortRetryIgnore
		//
		static int ShowStopMessageAbortRetryIgnore(HWND hWnd, wstring message)
		{
			return ShowStopMessageAbortRetryIgnore(hWnd, L"Stop!", message);
		}

		//
		// Function: ShowStopMessageYesNoCancel
		//
		static int ShowStopMessageAbortRetryIgnore(HWND hWnd, wstring title, wstring message)
		{
			int result = 0;
			if (hWnd != NULL)
			{
				g_hCBTHook = SetWindowsHookEx(WH_CBT, CBTMsgBox, NULL, ::GetCurrentThreadId());
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONSTOP | MB_ABORTRETRYIGNORE);
				UnhookWindowsHookEx(g_hCBTHook);
			}
			else
			{
				result = MessageBox(hWnd, message.c_str(), title.c_str(), MB_ICONSTOP | MB_ABORTRETRYIGNORE);
			}

			return result;
		}

		//
		// ReportSuccessEventLogEntry ###########################################
		//

		//
		// Function: ReportSuccessEventLogEntry
		//
		static bool ReportSuccessEventLogEntry(const wstring sourcename, const wstring text)
		{
			if (sourcename.size() == 0)
			{
				return false;
			}

			return ReportSuccessEventLogEntry(sourcename, 0, 0, 1, text.c_str());
		}

		//
		// Function: ReportSuccessEventLogEntry
		//
		static bool ReportSuccessEventLogEntry(wstring sourcename, short category, long eventid, short numberofstrings, const wchar_t* description)
		{
			if (sourcename.size() == 0 || (numberofstrings > 0 && wcslen(description) == 0))
			{
				return false;
			}

			HANDLE hEventLog = ::RegisterEventSource(NULL, sourcename.c_str());
			if (hEventLog == NULL)
			{
				return false;
			}

			BOOL bStatus = ::ReportEvent(hEventLog, EVENTLOG_SUCCESS, category, eventid, NULL, numberofstrings, 0, (LPCWSTR*)&description, NULL);
			if (!bStatus)
			{
				::DeregisterEventSource(hEventLog);
				return false;
			}

			::DeregisterEventSource(hEventLog);

			return true;
		}

		//
		// ReportErrorEventLogEntry ############################################
		//

		//
		// Function: ReportErrorEventLogEntry
		//
		static bool ReportErrorEventLogEntry(const wstring sourcename, const wstring text)
		{
			if (sourcename.size() == 0)
			{
				return false;
			}

			return ReportErrorEventLogEntry(sourcename, 0, 0, 1, text.c_str());
		}

		//
		// Function: ReportErrorEventLogEntry
		//
		static bool ReportErrorEventLogEntry(const wstring sourcename, short category, long eventid, short numberofstrings, const wchar_t* description)
		{
			if (sourcename.size() == 0 || (numberofstrings > 0 && wcslen(description) == 0))
			{
				return false;
			}

			HANDLE hEventLog = ::RegisterEventSource(NULL, sourcename.c_str());
			if (hEventLog == NULL)
			{
				return false;
			}

			BOOL bStatus = ::ReportEvent(hEventLog, EVENTLOG_ERROR_TYPE, category, eventid, NULL, numberofstrings, 0, (LPCWSTR*)&description, NULL);
			if (!bStatus)
			{
				::DeregisterEventSource(hEventLog);
				return false;
			}

			::DeregisterEventSource(hEventLog);

			return true;
		}

		//
		// ReportWarningEventLogEntry #########################################################
		//

		//
		// Function: ReportWarningEventLogEntry
		//
		static bool ReportWarningEventLogEntry(const wstring sourcename, const wstring text)
		{
			if (sourcename.size() == 0)
			{
				return false;
			}

			return ReportWarningEventLogEntry(sourcename, 0, 0, 1, text.c_str());
		}

		//
		// Function: ReportWarningEventLogEntry
		//
		static bool ReportWarningEventLogEntry(const wstring sourcename, short category, long eventid, short numberofstrings, const wchar_t* description)
		{
			if (sourcename.size() == 0 || (numberofstrings > 0 && wcslen(description) == 0))
			{
				return false;
			}

			HANDLE hEventLog = ::RegisterEventSource(NULL, sourcename.c_str());
			if (hEventLog == NULL)
			{
				return false;
			}

			BOOL bStatus = ::ReportEvent(hEventLog, EVENTLOG_WARNING_TYPE, category, eventid, NULL, numberofstrings, 0, (LPCWSTR*)&description, NULL);
			if (!bStatus)
			{
				::DeregisterEventSource(hEventLog);
				return false;
			}

			::DeregisterEventSource(hEventLog);

			return true;
		}

		//
		// ReportInformationEventLogEntry #########################################################
		//

		//
		// Function: ReportInformationEventLogEntry
		//
		static bool ReportInformationEventLogEntry(const wstring sourcename, const wstring text)
		{
			if (sourcename.size() == 0)
			{
				return false;
			}

			return ReportInformationEventLogEntry(sourcename, 0, 0, 1, text.c_str());
		}

		//
		// Function: ReportInformationEventLogEntry
		//
		static bool ReportInformationEventLogEntry(const wstring sourcename, short category, long eventid, short numberofstrings, const wchar_t* description)
		{
			if (sourcename.size() == 0 || (numberofstrings > 0 && wcslen(description) == 0))
			{
				return false;
			}

			HANDLE hEventLog = ::RegisterEventSource(NULL, sourcename.c_str());
			if (hEventLog == NULL)
			{
				return false;
			}

			BOOL bStatus = ::ReportEvent(hEventLog, EVENTLOG_INFORMATION_TYPE, category, eventid, NULL, numberofstrings, 0, (LPCWSTR*)&description, NULL);
			if (!bStatus)
			{
				::DeregisterEventSource(hEventLog);
				return false;
			}

			::DeregisterEventSource(hEventLog);

			return true;
		}

		//
		// ReportAuditSuccessEventLogEntry #########################################################
		//

		//
		// Function: ReportAuditSuccessEventLogEntry
		//
		static bool ReportAuditSuccessEventLogEntry(const wstring sourcename, const wstring text)
		{
			if (sourcename.size() == 0)
			{
				return false;
			}

			return ReportAuditSuccessEventLogEntry(sourcename, 0, 0, 1, text.c_str());
		}

		//
		// Function: ReportAuditSuccessEventLogEntry
		//
		static bool ReportAuditSuccessEventLogEntry(const wstring sourcename, short category, long eventid, short numberofstrings, const wchar_t* description)
		{
			if (sourcename.size() == 0 || (numberofstrings > 0 && wcslen(description) == 0))
			{
				return false;
			}

			HANDLE hEventLog = ::RegisterEventSource(NULL, sourcename.c_str());
			if (hEventLog == NULL)
			{
				return false;
			}

			BOOL bStatus = ::ReportEvent(hEventLog, EVENTLOG_AUDIT_SUCCESS, category, eventid, NULL, numberofstrings, 0, (LPCWSTR*)&description, NULL);
			if (!bStatus)
			{
				::DeregisterEventSource(hEventLog);
				return false;
			}

			::DeregisterEventSource(hEventLog);

			return true;
		}

		//
		// ReportAuditFailureEventLogEntry #########################################################
		//

		//
		// Function: ReportAuditFailureEventLogEntry
		//
		static bool ReportAuditFailureEventLogEntry(const wstring sourcename, const wstring text)
		{
			if (sourcename.size() == 0)
			{
				return false;
			}

			return ReportAuditFailureEventLogEntry(sourcename, 0, 0, 1, text.c_str());
		}

		//
		// Function: ReportAuditFailureEventLogEntry
		//
		static bool ReportAuditFailureEventLogEntry(const wstring sourcename, short category, long eventid, short numberofstrings, const wchar_t* description)
		{
			if (sourcename.size() == 0 || (numberofstrings > 0 && wcslen(description) == 0))
			{
				return false;
			}

			HANDLE hEventLog = ::RegisterEventSource(NULL, sourcename.c_str());
			if (hEventLog == NULL)
			{
				return false;
			}

			BOOL bStatus = ::ReportEvent(hEventLog, EVENTLOG_AUDIT_FAILURE, category, eventid, NULL, numberofstrings, 0, (LPCWSTR*)&description, NULL);
			if (!bStatus)
			{
				::DeregisterEventSource(hEventLog);
				return false;
			}

			::DeregisterEventSource(hEventLog);

			return true;
		}


		//
		// Function: FromBase64String
		//
		static vector<BYTE> FromBase64String(wstring str)
		{
			DWORD cbData(0);
			DWORD dwFlags(0);

			if (str.size() == 0)
			{
				return vector<BYTE>();
			}

			if (!CryptStringToBinary(str.c_str(), (DWORD)str.size(), CRYPT_STRING_BASE64, NULL, &cbData, NULL, &dwFlags))
			{
				throw ItsException(L"CryptStringToBinary failed");
			}

			BYTE* pReturnValue = new BYTE[cbData];
			if (!CryptStringToBinary(str.c_str(), (DWORD)str.size(), CRYPT_STRING_BASE64, pReturnValue, &cbData, NULL, &dwFlags))
			{
				delete[] pReturnValue;
				throw ItsException(L"CryptStringToBinary failed");
			}

			auto retVal = vector<BYTE>(cbData);
			for (DWORD i = 0; i < cbData; i++) {
				retVal[i] = pReturnValue[i];
			}

			delete[] pReturnValue;

			return retVal;
		}

		//
		// Function: ToBase64String
		//
		static wstring ToBase64String(vector<BYTE> data)
		{
			DWORD dwLength(static_cast<DWORD>(data.size()));
			DWORD cchString(0);
			CryptBinaryToString(data.data(), dwLength, CRYPT_STRING_BASE64, NULL, &cchString);
			unique_ptr<wchar_t[]> buffer = make_unique<wchar_t[]>(static_cast<size_t>(cchString) + 1);
			CryptBinaryToString(data.data(), dwLength, CRYPT_STRING_BASE64, buffer.get(), &cchString);
			buffer[cchString] = L'\0';

			return wstring(buffer.get());
		}

		//
		// Function: FromBase64StringToString
		//
		static wstring FromBase64StringToString(wstring str)
		{
			DWORD cbData(0);
			DWORD dwFlags(0);

			if (str.size() == 0)
			{
				return L"";
			}

			if (!CryptStringToBinary(str.c_str(), (DWORD)str.size(), CRYPT_STRING_BASE64, NULL, &cbData, NULL, &dwFlags))
			{
				throw ItsException(L"CryptStringToBinary failed");
			}

			BYTE* pReturnValue = new BYTE[cbData + sizeof(wchar_t)];
			if (!CryptStringToBinary(str.c_str(), (DWORD)str.size(), CRYPT_STRING_BASE64, pReturnValue, &cbData, NULL, &dwFlags))
			{
				throw ItsException(L"CryptStringToBinary failed");
			}

			pReturnValue[cbData] = 0;
			pReturnValue[cbData + 1] = 0;

			auto retVal = wstring(reinterpret_cast<wchar_t*>(pReturnValue));

			delete[] pReturnValue;

			return retVal;
		}

		//
		// Function: ToBase64StringFromString
		//
		static wstring ToBase64StringFromString(wstring str)
		{
			if (str.size() == 0) {
				return L"";
			}

			BYTE* pData = reinterpret_cast<BYTE*>(const_cast<wchar_t*>(str.c_str()));
			size_t dwSize = str.size() * sizeof(wchar_t);

			vector<BYTE> buffer;
			for (size_t i = 0; i < dwSize; i++) {
				buffer.push_back(pData[i]);
			}

			return ToBase64String(buffer);
		}
	};
}// namespace ItSoftware::Win