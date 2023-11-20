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
namespace ItSoftware::Win::Core
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
	// struct: ItsTimer
	//
	// (i): Timer wrapper.
	//
	struct ItsTimer
	{
	private:
		size_t m_start = 0;
		size_t m_end = 0;
		bool m_isRunning = false;
	public:
		void Start()
		{
			if (!this->IsRunning())
			{
				this->m_start = GetTickCount64();
				this->m_end = m_start;
				this->m_isRunning = true;
			}
		}

		void Stop()
		{
			this->m_end = GetTickCount64();
			this->m_isRunning = false;
		}

		bool IsRunning()
		{
			return this->m_isRunning;
		}

		size_t LapSeconds()
		{
			if (!this->IsRunning()) {
				return 0;
			}

			return (size_t)((GetTickCount64() - this->m_start) / CLOCKS_PER_SEC);
		}

		size_t LapMilliseconds()
		{
			if (!this->IsRunning()) {
				return 0;
			}

			return (size_t)((GetTickCount64() - this->m_start) / (size_t)(CLOCKS_PER_SEC / 1'000));
		}

		size_t LapMicroseconds()
		{
			if (!this->IsRunning()) {
				return 0;
			}

			return (size_t)((GetTickCount64() - this->m_start) / (size_t)(CLOCKS_PER_SEC / 1'000'000));
		}

		size_t LapTicks()
		{
			if (!this->IsRunning()) {
				return 0;
			}

			return (size_t)GetTickCount64() - this->m_start;
		}

		size_t GetSeconds()
		{
			if (this->IsRunning()) {
				return 0;
			}

			return (size_t)((this->m_end - this->m_start) / (size_t)CLOCKS_PER_SEC);
		}

		size_t GetMilliseconds()
		{
			if (this->IsRunning()) {
				return 0;
			}

			return (size_t)((this->m_end - this->m_start) / (size_t)(CLOCKS_PER_SEC / 1'000));
		}

		size_t GetMicroseconds()
		{
			if (this->IsRunning()) {
				return 0;
			}

			return (size_t)((this->m_end - this->m_start) / (size_t)(CLOCKS_PER_SEC / 1'000'000));
		}

		size_t GetTicks()
		{
			if (this->IsRunning()) {
				return 0;
			}

			return this->m_end - this->m_start;
		}
	};

	//
	// struct: ItsGuidFormat
	//
	// (i): Container for premade Guid format strings.
	//
	struct ItsGuidFormat {
		inline static constexpr const wchar_t* RegistryFormat{ L"{%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}" };
		inline static constexpr const wchar_t* RegistryFormatStripped{ L"%08lX-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X" };
		inline static constexpr const wchar_t* ConstFormat{ L"{ 0x%lx, 0x%x, 0x%x, { 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x } }" };
		inline static constexpr const wchar_t* CompactFormat{ L"%08lX%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X" };
		inline static constexpr const wchar_t* PrefixedCompactFormat{ L"GUID%08lX%04X%04x%02X%02X%02X%02X%02X%02X%02X%02X" };
	};

	//
	// struct: ItsGuid
	//
	// (i): Wrapper for Guid generation and rendering.
	//
	struct ItsGuid
	{
	public:
		static wstring CreateGuid()
		{
			GUID guid{ 0 };
			HRESULT hr = CoCreateGuid(&guid);
			if (FAILED(hr)) {
				return wstring(L"");
			}

			return ItsGuid::ToString(guid);
		}
		static bool CreateGuid(GUID* pGuid)
		{
			HRESULT hr = CoCreateGuid(pGuid);
			if (FAILED(hr)) {
				return false;
			}
			return true;
		}
		static wstring ToString(GUID& guid) {
			return ItsGuid::ToString(guid, ItsGuidFormat::RegistryFormat);
		}
		static wstring ToString(GUID& guid, wstring format) {
			wchar_t wcsBuffer[100];
			memset(wcsBuffer, 0, 100);

			wsprintf(wcsBuffer, format.c_str(),
				// first copy...
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
				guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

			return wstring(wcsBuffer);
		}
	};

			
	//
	// struct: ItsPath
	// 
	// (i): Path routines.
	//
	struct ItsPath
	{
	public:
		static const wchar_t VolumeSeparator = ':';
		static const wchar_t PathSeparator = '\\';
		static const wchar_t AlternatePathSeparator = '/';
		static const wchar_t ExtensionSeparator = '.';
		static wstring NormalizePath(wstring path)
		{
			if (path.size() == 0) {
				return wstring(L"");
			}

			wstring aps;
			aps += ItsPath::AlternatePathSeparator;

			wstring ps;
			ps += ItsPath::PathSeparator;

			path = ItsString::Replace(path, aps, ps);

			return path;

		}
		static const vector<wchar_t> GetInvalidPathCharacters()
		{
			vector<wchar_t> chars;
			chars.push_back(static_cast<wchar_t>(0x0022));
			chars.push_back(static_cast<wchar_t>(0x003c));
			chars.push_back(static_cast<wchar_t>(0x003e));
			chars.push_back(static_cast<wchar_t>(0x007c));
			chars.push_back(static_cast<wchar_t>(0x0000));
			chars.push_back(static_cast<wchar_t>(0x0001));
			chars.push_back(static_cast<wchar_t>(0x0002));
			chars.push_back(static_cast<wchar_t>(0x0003));
			chars.push_back(static_cast<wchar_t>(0x0004));
			chars.push_back(static_cast<wchar_t>(0x0005));
			chars.push_back(static_cast<wchar_t>(0x0006));
			chars.push_back(static_cast<wchar_t>(0x0007));
			chars.push_back(static_cast<wchar_t>(0x0008));
			chars.push_back(static_cast<wchar_t>(0x0009));
			chars.push_back(static_cast<wchar_t>(0x000a));
			chars.push_back(static_cast<wchar_t>(0x000b));
			chars.push_back(static_cast<wchar_t>(0x000c));
			chars.push_back(static_cast<wchar_t>(0x000d));
			chars.push_back(static_cast<wchar_t>(0x000e));
			chars.push_back(static_cast<wchar_t>(0x000f));
			chars.push_back(static_cast<wchar_t>(0x0010));
			chars.push_back(static_cast<wchar_t>(0x0011));
			chars.push_back(static_cast<wchar_t>(0x0012));
			chars.push_back(static_cast<wchar_t>(0x0013));
			chars.push_back(static_cast<wchar_t>(0x0014));
			chars.push_back(static_cast<wchar_t>(0x0015));
			chars.push_back(static_cast<wchar_t>(0x0016));
			chars.push_back(static_cast<wchar_t>(0x0017));
			chars.push_back(static_cast<wchar_t>(0x0018));
			chars.push_back(static_cast<wchar_t>(0x0019));
			chars.push_back(static_cast<wchar_t>(0x001a));
			chars.push_back(static_cast<wchar_t>(0x001b));
			chars.push_back(static_cast<wchar_t>(0x001c));
			chars.push_back(static_cast<wchar_t>(0x001d));
			chars.push_back(static_cast<wchar_t>(0x001e));
			chars.push_back(static_cast<wchar_t>(0x001f));
			return chars;
		}
		static const vector<wchar_t> GetInvalidFilenameCharacters()
		{
			vector<wchar_t> chars = ItsPath::GetInvalidPathCharacters();
			chars.push_back(static_cast<wchar_t>(0x003a));
			chars.push_back(static_cast<wchar_t>(0x002a));
			chars.push_back(static_cast<wchar_t>(0x003f));
			chars.push_back(static_cast<wchar_t>(0x005c));
			chars.push_back(static_cast<wchar_t>(0x002f));
			return chars;
		}
		static wstring Combine(wstring path1, wstring path2)
		{
			if (path1.size() == 0 && path2.size() == 0) {
				return wstring(L"");
			}

			if (path1.size() == 0) {
				return path2;
			}

			if (path2.size() == 0) {
				return path1;
			}

			wstring aps;
			aps += ItsPath::AlternatePathSeparator;

			wstring ps;
			ps += ItsPath::PathSeparator;

			path1 = ItsPath::NormalizePath(path1);
			path2 = ItsPath::NormalizePath(path2);

			wstringstream path;
			path << path1;
			if (path1[path1.size() - 1] != ItsPath::PathSeparator &&
				path2[0] != ItsPath::PathSeparator) {
				path << ItsPath::PathSeparator;
			}
			path << path2;

			wstring ret = path.str();
			return ret;
		}
		static bool Exists(wstring path)
		{
			return static_cast<bool>(PathFileExistsW(path.c_str()));
		}
		static bool IsFile(wstring path)
		{
			DWORD fileAttributes = GetFileAttributes(path.c_str());
			return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
		}
		static bool IsDirectory(wstring path)
		{
			DWORD fileAttributes = GetFileAttributes(path.c_str());
			return (fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
		}
		static wstring GetVolume(wstring path)
		{
			if (path.size() >= 2) {
				if (path[1] == ItsPath::VolumeSeparator) {
					wstring ret;
					ret += path[0];
					return ret;
				}
			}
			return wstring(L"");
		}
		static wstring GetDirectory(wstring path)
		{
			if (path.size() == 0) {
				return wstring(L"");
			}

			if (path.find(ItsPath::PathSeparator) == wstring::npos) {
				return wstring(L"");
			}

			path = ItsPath::NormalizePath(path);

			auto i = path.find_last_of(ItsPath::PathSeparator);
			if (i == std::wstring::npos) {
				return wstring(L"");
			}
			return path.substr(0, i + 1);
		}
		static wstring GetFilename(wstring path)
		{
			if (path.size() == 0) {
				return wstring(L"");
			}

			if (path.find(ItsPath::PathSeparator) == wstring::npos) {
				return wstring(L"");
			}

			path = ItsPath::NormalizePath(path);

			auto i = path.rfind(ItsPath::PathSeparator);
			if (i == std::wstring::npos) {
				return wstring(L"");
			}
			return path.substr(i + 1, path.size() - i - 1);
		}
		static wstring GetExtension(wstring path)
		{
			if (path.size() == 0) {
				return wstring(L"");
			}

			if (path.find(ItsPath::PathSeparator) == wstring::npos) {
				return wstring(L"");
			}

			path = ItsPath::NormalizePath(path);

			auto i = path.rfind(ItsPath::ExtensionSeparator);
			if (i == std::wstring::npos) {
				return wstring(L"");
			}
			return path.substr(i, path.size() - i);
		}
		static bool IsPathValid(wstring path)
		{
			if (path.size() == 0) {
				return false;
			}

			wstring volume = ItsPath::GetVolume(path);
			wstring directory = ItsPath::GetDirectory(path);
			wstring filename = ItsPath::GetFilename(path);

			auto invalidPathChars = ItsPath::GetInvalidPathCharacters();
			auto invalidFileChars = ItsPath::GetInvalidFilenameCharacters();

			if (directory[directory.size() - 1] != ItsPath::PathSeparator) {
				return false;
			}

			for (auto d : directory) {
				for (auto i : invalidPathChars) {
					if (d == i) {
						return false;
					}
				}
			}

			for (auto f : filename) {
				for (auto i : invalidFileChars) {
					if (f == i) {
						return false;
					}
				}
			}

			return true;
		}
		static bool HasExtension(wstring path, wstring extension)
		{
			if (path.size() == 0) {
				return false;
			}

			wstring ext = ItsPath::GetExtension(path);
			if (ext.size() == 0) {
				return false;
			}

			return (ext == extension);
		}
		static wstring ChangeExtension(wstring path, wstring newExtension)
		{
			if (path.size() == 0) {
				return wstring(L"");
			}

			if (newExtension.size() == 0) {
				return path;
			}

			path = ItsPath::NormalizePath(path);

			if (path[path.size() - 1] == ItsPath::PathSeparator) {
				return path;
			}

			wstring ext = ItsPath::GetExtension(path);
			if (ext.size() == 0) {
				if (newExtension[0] == ItsPath::ExtensionSeparator) {
					return path + newExtension;
				}
				else {
					return path + ItsPath::ExtensionSeparator + newExtension;
				}
			}

			auto pe = path.rfind(ext);
			if (pe == wstring::npos) {
				return path;
			}

			wstring retVal = path.replace(pe, path.size() - pe, newExtension);
			return retVal;
		}

		static wstring GetParentDirectory(wstring path)
		{
			if (path.size() == 0) {
				return wstring(L"");
			}
					
			path = ItsPath::NormalizePath(path);
			path = ItsPath::GetDirectory(path);

			size_t pos1 = path.rfind(ItsPath::PathSeparator);
			if (pos1 == wstring::npos) {
				return wstring(L"");
			}

			size_t pos2 = pos1;
			if (pos1 == (path.size() - 1)) {
				pos2 = path.rfind(ItsPath::PathSeparator, pos1 - 1);
				if (pos2 == wstring::npos) {
					pos2 = pos1;
				}
			}

			return path.substr(0, pos2+1);
		}
	};

	//
	// struct: ItsError
	//
	// (i): System error description generation.
	//
	struct ItsError
	{
		//
		// Get GetLastError() Error Description
		//
		static wstring GetLastErrorDescription()
		{
			LPVOID lpMsgBuf = nullptr;
			FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				GetLastError(),
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(wchar_t*)&lpMsgBuf,
				0,
				NULL);

			// Copy buffer contet to bstr...
			CComBSTR bstr;
			bstr += (wchar_t*)lpMsgBuf;

			// Free the buffer...
			LocalFree(lpMsgBuf);

			// Return value...
			return wstring(bstr.operator LPWSTR());;
		}// inline CComBSTR GetLastErrorDesc( )

			//
			// Get Error Description of Given Error Code
			//
		static wstring GetErrorDescription(DWORD dwError)
		{
			LPVOID lpMsgBuf = nullptr;
			FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
				FORMAT_MESSAGE_FROM_SYSTEM |
				FORMAT_MESSAGE_IGNORE_INSERTS,
				NULL,
				dwError,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
				(wchar_t*)&lpMsgBuf,
				0,
				NULL);

			// Copy buffer contet to bstr...
			CComBSTR bstr;
			bstr += (wchar_t*)lpMsgBuf;

			// Free the buffer...
			LocalFree(lpMsgBuf);

			// Return value...
			return wstring(bstr.operator LPWSTR());;
		}// inline CComBSTR GetLastErrorDesc( )

			//
			// Get COM IErrorInfo Error Description
			//
		static wstring GetCoLastErrorInfoDescription()
		{
			IErrorInfo* pErrInfo = nullptr;
			HRESULT hr = GetErrorInfo(NULL, (IErrorInfo**)&pErrInfo);
			CComBSTR bstr;
			if (hr == S_OK && pErrInfo)
			{
				pErrInfo->GetDescription(&bstr);
				pErrInfo->Release();
				return wstring(bstr.operator LPWSTR());
			}
			return wstring(L"");
		}// inline CComBSTR GetLastErrorDesc( )
	};

	//
	// class: unique_handle
	//
	// (i): Wrapper for many types of Windows handles.
	//
	template<typename Traits>
	class unique_handle
	{
	private:
		typedef typename Traits::pointer pointer;

		pointer m_value;

	protected:
	public:

		explicit unique_handle(pointer value = Traits::invalid()) noexcept
			: m_value{ value }
		{

		}

		~unique_handle() noexcept
		{
			close();
		}

		auto close() noexcept -> void
		{
			if (*this)
			{
				Traits::close(m_value);
				m_value = Traits::invalid();
			}
		}

		pointer p()
		{
			return m_value;
		}

		pointer* GetAddressOf()
		{
			return &m_value;
		}

		//
		// bool
		//
		explicit operator bool() const noexcept
		{
			return m_value != Traits::invalid();
		}

		//
		// HANDLE
		//
		operator HANDLE() const noexcept
		{
			return static_cast<HANDLE>(m_value);
		}

		void operator=(HANDLE handle)
		{
			if (this->m_value != Traits::invalid()) {
				this->close();
			}
			m_value = static_cast<typename Traits::pointer>(handle);
		}

		//
		// HWND
		//
		operator HWND() const noexcept
		{
			return static_cast<HWND>(m_value);
		}

		void operator=(HWND handle)
		{
			if (this->m_value != Traits::invalid()) {
				this->close();
			}
			m_value = static_cast<typename Traits::pointer>(handle);
		}

		//
		// HBITMAP
		//
		operator HBITMAP() const noexcept
		{
			return static_cast<HBITMAP>(m_value);
		}

		void operator=(HBITMAP handle)
		{
			if (this->m_value != Traits::invalid()) {
				this->close();
			}
			m_value = static_cast<typename Traits::pointer>(handle);
		}

		//
		// HGDIOBJ
		//
		//operator HGDIOBJ() const noexcept
		//{
		//	return static_cast<HGDIOBJ>(m_value);
		//}
		//
		//void operator=(HGDIOBJ handle)
		//{
		//	m_value = static_cast<Traits::pointer>(handle);
		//}

		//
		// HMENU
		//
		operator HMENU() const noexcept
		{
			return static_cast<HMENU>(m_value);
		}

		void operator=(HMENU handle)
		{
			if (this->m_value != Traits::invalid()) {
				this->close();
			}
			m_value = static_cast<typename Traits::pointer>(handle);
		}

		//
		// HIMAGELIST
		//
		operator HIMAGELIST() const noexcept
		{
			return static_cast<HIMAGELIST>(m_value);
		}

		void operator=(HIMAGELIST handle)
		{
			if (this->m_value != Traits::invalid()) {
				this->close();
			}
			m_value = static_cast<typename Traits::pointer>(handle);
		}

		//
		// HRGN
		//
		operator HRGN() const noexcept
		{
			return static_cast<HRGN>(m_value);
		}

		void operator=(HRGN handle)
		{
			if (this->m_value != Traits::invalid()) {
				this->close();
			}
			m_value = static_cast<typename Traits::pointer>(handle);
		}

		//
		// HBRUSH
		//
		operator HBRUSH() const noexcept
		{
			return static_cast<HBRUSH>(m_value);
		}

		void operator=(HBRUSH handle)
		{
			if (this->m_value != Traits::invalid()) {
				this->close();
			}
			m_value = static_cast<typename Traits::pointer>(handle);
		}

		//
		// HFONT
		//
		operator HFONT() const noexcept
		{
			return static_cast<HFONT>(m_value);
		}

		void operator=(HFONT handle)
		{
			if (this->m_value != Traits::invalid()) {
				this->close();
			}
			m_value = static_cast<typename Traits::pointer>(handle);
		}

		//
		// HICON
		//
		operator HICON() const noexcept
		{
			return static_cast<HICON>(m_value);
		}

		void operator=(HICON handle)
		{
			if (this->m_value != Traits::invalid()) {
				this->close();
			}
			m_value = static_cast<typename Traits::pointer>(handle);
		}

		//
		// IsInvalid
		//
		bool IsInvalid()
		{
			return (m_value == Traits::invalid());
		}

		//
		// IsValid
		//
		bool IsValid()
		{
			return (m_value != Traits::invalid());
		}
	};

	//
	// HANDLE traits
	//
	struct handle_handle_traits
	{
		typedef HANDLE pointer;
		static auto invalid() noexcept -> pointer
		{
			return INVALID_HANDLE_VALUE;
		}

		static auto close(pointer value) noexcept -> void
		{
			CloseHandle(value);
		}
	};


	//
	// FINDHANDLE traits
	//
	struct handle_findhandle_traits
	{
		typedef HANDLE pointer;
		static auto invalid() noexcept -> pointer
		{
			return nullptr;
		}

		static auto close(pointer value) noexcept -> void
		{
			::FindClose(value);
		}
	};

	//
	// FindChangeNotification HANDLE traits
	//
	struct handle_findchangehandle_traits
	{
		typedef HANDLE pointer;
		static auto invalid() noexcept -> pointer
		{
			return INVALID_HANDLE_VALUE;
		}

		static auto close(pointer value) noexcept -> void
		{
			::FindCloseChangeNotification(value);
		}
	};

	//
	// HWND traits
	//
	struct hwnd_handle_traits
	{
		typedef HWND pointer;
		static auto invalid() noexcept -> pointer
		{
			return nullptr;
		}

		static auto close(pointer value) noexcept -> void
		{
			if (value != invalid())
			{
				DestroyWindow(value);
			}
		}
	};

	//
	// HBITMAP traits
	//
	struct hbitmap_handle_traits
	{
		typedef HBITMAP pointer;
		static auto invalid() noexcept -> pointer
		{
			return nullptr;
		}

		static auto close(pointer value) noexcept -> void
		{
			if (value != invalid())
			{
				DeleteObject(value);
			}
		}
	};

	//
	// HMENU traits
	//
	struct hmenu_handle_traits
	{
		typedef HMENU pointer;
		static auto invalid() noexcept -> pointer
		{
			return nullptr;
		}

		static auto close(pointer value) noexcept -> void
		{
			if (value != invalid())
			{
				::DestroyMenu(value);
			}
		}
	};

	//
	// HIMAGELIST traits
	//
	struct himagelist_handle_traits
	{
		typedef HIMAGELIST pointer;
		static auto invalid() noexcept -> pointer
		{
			return nullptr;
		}

		static auto close(pointer value) noexcept -> void
		{
			if (value != invalid())
			{
				::ImageList_Destroy(value);
			}
		}
	};

	//
	// HRGN traits
	//
	struct hrgn_handle_traits
	{
		typedef HRGN pointer;
		static auto invalid() noexcept -> pointer
		{
			return nullptr;
		}

		static auto close(pointer value) noexcept -> void
		{
			if (value != invalid())
			{
				::DeleteObject(value);
			}
		}
	};

	//
	// HBRUSH traits
	//
	struct hbrush_handle_traits
	{
		typedef HBRUSH pointer;
		static auto invalid() noexcept -> pointer
		{
			return nullptr;
		}

		static auto close(pointer value) noexcept -> void
		{
			if (value != invalid())
			{
				::DeleteObject(value);
			}
		}
	};

	//
	// HFONT traits
	//
	struct hfont_handle_traits
	{
		typedef HFONT pointer;
		static auto invalid() noexcept -> pointer
		{
			return nullptr;
		}

		static auto close(pointer value) noexcept -> void
		{
			if (value != invalid())
			{
				::DeleteObject(value);
			}
		}
	};

	//
	// HICON traits
	//
	struct hicon_handle_traits
	{
		typedef HICON pointer;
		static auto invalid() noexcept -> pointer
		{
			return nullptr;
		}

		static auto close(pointer value) noexcept -> void
		{
			if (value != invalid())
			{
				::DestroyIcon(value);
			}
		}
	};

	//
	// unique_xxx_handle typedefs
	//
	typedef unique_handle<handle_handle_traits> unique_handle_handle;
	typedef unique_handle<handle_findhandle_traits> unique_findhandle_handle;
	typedef unique_handle<handle_findchangehandle_traits> unique_findchangehandle_handle;
	typedef unique_handle<hwnd_handle_traits> unique_hwnd_handle;
	typedef unique_handle<hbitmap_handle_traits> unique_hbitmap_handle;
	typedef unique_handle<hmenu_handle_traits> unique_hmenu_handle;
	typedef unique_handle<himagelist_handle_traits> unique_himagelist_handle;
	typedef unique_handle<hrgn_handle_traits> unique_hrgn_handle;
	typedef unique_handle<hbrush_handle_traits> unique_hbrush_handle;
	typedef unique_handle<hfont_handle_traits> unique_hfont_handle;
	typedef unique_handle<hicon_handle_traits> unique_hicon_handle;

	//
	// struct: ItsDirectory
	// 
	// (i): Windows directory routines.
	//
	struct ItsDirectory
	{
	public:
		static bool Exists(wstring directory)
		{
			DWORD fileAttributes = GetFileAttributes(directory.c_str());
			return (fileAttributes != INVALID_FILE_ATTRIBUTES && (fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
		}
		static bool CreateDirectory(wstring path)
		{
			return ::CreateDirectoryW(path.c_str(), nullptr);
		}
		static bool RemoveDirectory(wstring path)
		{
			return ::RemoveDirectoryW(path.c_str());
		}
		static bool SetCurrentDirectory(wstring path)
		{
			return ::SetCurrentDirectoryW(path.c_str());
		}
		static vector<wstring> GetDirectories(wstring path) {
			if (path.size() == 0) {
				return vector<wstring>();
			}

			path = ItsPath::NormalizePath(path);
			if (path[path.size() - 1] != ItsPath::PathSeparator)
			{
				if (path[path.size() - 1] != L'*') {
					path += ItsPath::PathSeparator;
					path += L"*";
				}
			}
			else {
				path += L'*';
			}

			WIN32_FIND_DATAW wfd{ 0 };
			vector<wstring> dirs;
			unique_findhandle_handle h(::FindFirstFile(path.c_str(), &wfd));
			if (h.IsValid()) {
				if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					if (wcscmp(wfd.cFileName, L".") != 0 &&
						wcscmp(wfd.cFileName, L"..") != 0)
					{
						dirs.push_back(wfd.cFileName);
					}
				}
				while (::FindNextFile(h, &wfd)) {
					if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
						if (wcscmp(wfd.cFileName, L".") != 0 &&
							wcscmp(wfd.cFileName, L"..") != 0)
						{
							dirs.push_back(wfd.cFileName);
						}
					}
				}
			}

			return dirs;
		}
		static vector<wstring> GetFiles(wstring path) {
			if (path.size() == 0) {
				return vector<wstring>();
			}

			path = ItsPath::NormalizePath(path);
			if (path[path.size() - 1] != ItsPath::PathSeparator)
			{
				if (path[path.size() - 1] != L'*') {
					path += ItsPath::PathSeparator;
					path += L"*";
				}
			}
			else {
				path += L'*';
			}

			WIN32_FIND_DATAW wfd{ 0 };
			vector<wstring> dirs;
			unique_findhandle_handle h(::FindFirstFile(path.c_str(), &wfd));
			if (h.IsValid()) {
				if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					dirs.push_back(wfd.cFileName);
				}
				while (::FindNextFile(h, &wfd)) {
					if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
						dirs.push_back(wfd.cFileName);
					}
				}
			}

			return dirs;
		}
		static vector<wchar_t> GetLogicalDrives()
		{
			const wchar_t wszDrives[27] = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
			vector<wchar_t> d;

			DWORD dw = ::GetLogicalDrives();

			for (DWORD i = 0; i < 26; i++) {
				DWORD dwBit = pow(2, (i));
				DWORD dwResult = dw & dwBit;
				if (dwResult) {
					d.push_back(wszDrives[i]);
				}
			}

			return d;
		}
	};

	//
	// class: ItsEvent
	//
	// (i): Windows event wrapper.
	//
	class ItsEvent
	{
	private:
		unique_handle_handle m_handle;
	protected:
	public:
		ItsEvent()
		{
			this->m_handle = CreateEvent(NULL, TRUE, FALSE, NULL);
		}

		ItsEvent(bool signaled)
		{
			this->m_handle = CreateEvent(NULL, TRUE, (signaled) ? TRUE : FALSE, NULL);
		}

		ItsEvent(ItsEvent const&) = delete;
		auto operator=(ItsEvent const&)->ItsEvent & = delete;

		operator HANDLE() noexcept
		{
			return this->m_handle.p();
		}

		void Clear()
		{
			ResetEvent(this->m_handle.p());
		}

		void Signal()
		{
			SetEvent(this->m_handle.p());
		}

		bool Wait(DWORD milliseconds = INFINITE)
		{
			auto result = WaitForSingleObject(this->m_handle.p(), milliseconds);
			return (result == WAIT_OBJECT_0 || result == WAIT_TIMEOUT);
		}
	};

	//
	// enum: ItsFileOpen
	//
	enum class ItsFileOpenCreation
	{
		CreateAlways,
		CreateNew,
		OpenAlways,
		OpenExisting,
		TruncateExisting
	};

	//
	// enum: ItsFilePosition
	//
	enum class ItsFilePosition
	{
		FileBegin,
		FileCurrent,
		FileEnd
	};

	//
	// enum: ItsFileTextType
	//
	enum class ItsFileTextType
	{
		Ansi,
		UTF8NoBOM,
		UTF8WithBOM,
		Unicode
	};

	//
	// class: ItsFileBase
	//
	// (i): ItsFile and ItsTextFile base class.
	//
	class ItsFileBase
	{
	private:
	protected:
		unique_handle_handle m_handle;

		//
		// Method: GetFileSize
		//
		bool GetFileSize(size_t* size)
		{
			LARGE_INTEGER s{ 0 };
			bool status = (bool)GetFileSizeEx(this->m_handle.p(), &s);
			*size = s.QuadPart;
			return status;
		}
	public:

		//
		// Method: Read
		//
		bool Read(void* pBuffer, DWORD dwBytesToRead, DWORD* dwBytesRead)
		{
			if (this->m_handle.IsInvalid())
			{
				return false;
			}

			return (bool)ReadFile(this->m_handle.p(), pBuffer, dwBytesToRead, dwBytesRead, NULL);
		}

		//
		// Method: Write
		//
		bool Write(const void* pBuffer, DWORD dwBytesToWrite, DWORD* dwBytesWritten)
		{
			if (this->m_handle.IsInvalid())
			{
				return false;
			}

			return (bool)WriteFile(this->m_handle.p(), pBuffer, dwBytesToWrite, dwBytesWritten, NULL);
		}

		//
		// Method: Close
		//
		bool Close()
		{
			if (this->m_handle.IsInvalid())
			{
				return false;
			}

			this->m_handle.close();

			return true;
		}

		//
		// Method: GetFilePosition
		//
		bool GetFilePosition(size_t* position)
		{
			if (this->m_handle.IsInvalid())
			{
				return false;
			}

			LARGE_INTEGER zero{ 0 };
			LARGE_INTEGER pos{ 0 };
			bool status = (bool)SetFilePointerEx(this->m_handle.p(), zero, &pos, FILE_CURRENT);
			*position = pos.QuadPart;
			return status;
		}

		//
		// Method: SetFilePosition
		//
		bool SetFilePosition(size_t distanceToMove, ItsFilePosition position)
		{
			if (this->m_handle.IsInvalid())
			{
				return false;
			}

			DWORD dwPosition = 0;
			if (position == ItsFilePosition::FileBegin)
			{
				dwPosition = FILE_BEGIN;
			}
			else if (position == ItsFilePosition::FileCurrent)
			{
				dwPosition = FILE_CURRENT;
			}
			else if (position == ItsFilePosition::FileEnd)
			{
				dwPosition = FILE_END;
			}

			LARGE_INTEGER distance{ 0 };
			distance.QuadPart = distanceToMove;
			return (bool)SetFilePointerEx(this->m_handle.p(), distance, NULL, dwPosition);
		}

		operator HANDLE() const
		{
			return this->m_handle.operator HANDLE();
		}

		bool IsValid()
		{
			return this->m_handle.IsValid();
		}

		bool IsInvalid()
		{
			return this->m_handle.IsInvalid();
		}

		bool SetFileSize(size_t pos)
		{
			if (!this->IsValid())
			{
				return false;
			}
			this->SetFilePosition(pos, ItsFilePosition::FileBegin);
			return (bool)SetEndOfFile(this->m_handle.p());
		}
	};

	//
	// class: ItsTextFile
	//
	// (i): Wrapper for text file on Windows.
	//
	class ItsTextFile : public ItsFileBase
	{
	private:
		ItsFileTextType m_textType = ItsFileTextType::Ansi;

		//
		// Method: OpenOrCreate
		//
		bool OpenOrCreate(wstring filename, wstring accessReadWrite, wstring shareReadWrite, ItsFileOpenCreation flagCreation)
		{
			if (this->m_handle.IsValid())
			{
				return false;
			}

			DWORD dwAccess = 0;
			if (accessReadWrite.size() != 0)
			{
				if (accessReadWrite.find(L"r") != wstring::npos) {
					dwAccess |= GENERIC_READ;
				}
				if (accessReadWrite.find(L"w") != wstring::npos) {
					dwAccess |= GENERIC_WRITE;
				}
			}

			DWORD dwShare = 0;
			if (shareReadWrite.size() != 0)
			{
				if (shareReadWrite.find(L"r") != wstring::npos) {
					dwShare |= FILE_SHARE_READ;
				}
				if (shareReadWrite.find(L"w") != wstring::npos) {
					dwShare |= FILE_SHARE_WRITE;
				}
				if (shareReadWrite.find(L"d") != wstring::npos) {
					dwShare |= FILE_SHARE_DELETE;
				}
			}

			DWORD dwFlags = 0;
			if (flagCreation == ItsFileOpenCreation::CreateAlways) {
				dwFlags = CREATE_ALWAYS;
			}
			else if (flagCreation == ItsFileOpenCreation::CreateNew) {
				dwFlags = CREATE_NEW;
			}
			else if (flagCreation == ItsFileOpenCreation::OpenAlways) {
				dwFlags = OPEN_ALWAYS;
			}
			else if (flagCreation == ItsFileOpenCreation::OpenExisting) {
				dwFlags = OPEN_EXISTING;
			}
			else if (flagCreation == ItsFileOpenCreation::TruncateExisting) {
				dwFlags = TRUNCATE_EXISTING;
			}

			this->m_handle = CreateFile(filename.c_str(), dwAccess, dwShare, NULL, dwFlags, FILE_ATTRIBUTE_NORMAL, NULL);

			return this->m_handle.IsValid();
		}

	protected:
	public:
		inline static constexpr const wchar_t* LineDelimiterWindows = L"\r\n";
		inline static constexpr const wchar_t* LineDelimiterUnix = L"\n";
		inline static const vector<BYTE> ByteOrderMarkUnicode = { 0xFF, 0xFE };
		inline static const vector<BYTE> ByteOrderMarkUTF8 = { 0xEF, 0xBB, 0xBF };

		//
		// Constructor
		//
		ItsTextFile()
		{
		}

		//
		// Method: OpenOrCreateText
		//
		bool OpenOrCreateText(wstring filename, wstring accessReadWrite, wstring shareReadWrite, ItsFileOpenCreation flagCreation, ItsFileTextType type)
		{
			if (this->m_handle.IsValid())
			{
				return false;
			}

			if (!this->OpenOrCreate(filename, accessReadWrite, shareReadWrite, flagCreation))
			{
				return false;
			}

			size_t size{ 0 };
			this->GetFileSize(&size);
			if (size > 2)
			{
				this->SetFilePosition(0, ItsFilePosition::FileBegin);

				unique_ptr<BYTE[]> bom = make_unique<BYTE[]>(3);

				DWORD dwRead{ 0 };
				this->Read(bom.get(), 3, &dwRead);

				if (dwRead > 2) {
					if (bom.get()[0] == 0xFF &&
						bom.get()[1] == 0xFE)
					{
						this->SetFilePosition(2, ItsFilePosition::FileBegin);

						this->m_textType = ItsFileTextType::Unicode;
						return true;
					}
					else if (bom.get()[0] == 0xEF &&
						bom.get()[1] == 0xBB &&
						bom.get()[2] == 0xBF)
					{
						this->SetFilePosition(3, ItsFilePosition::FileBegin);

						this->m_textType = ItsFileTextType::UTF8WithBOM;
						return true;
					}
				}
			}

			this->m_textType = type;

			if (type == ItsFileTextType::Ansi)
			{
				return true;
			}

			vector<BYTE> bom;
			if (type == ItsFileTextType::Unicode) {
				bom.push_back(0xFF);
				bom.push_back(0xFE);
			}
			else if (type == ItsFileTextType::UTF8WithBOM)
			{
				bom.push_back(0xEF);
				bom.push_back(0xBB);
				bom.push_back(0xBF);
			}

			this->SetFilePosition(0, ItsFilePosition::FileBegin);

			DWORD dwWritten{ 0 };
			return this->Write(bom.data(), (DWORD)bom.size(), &dwWritten);
		}

		//
		// Method: ReadTextAllLines
		//
		bool ReadTextAllLines(vector<wstring>& lines, wstring lineDelimiter = L"\r\n")
		{
			if (!this->m_handle.IsValid())
			{
				return false;
			}

			size_t size{ 0 };
			if (!this->GetFileSize(&size))
			{
				return false;
			}

			if (size == 0) {
				lines.clear();
				return true;
			}

			if (this->m_textType == ItsFileTextType::Ansi)
			{
				this->SetFilePosition(0, ItsFilePosition::FileBegin);

				unique_ptr<BYTE[]> text = make_unique<BYTE[]>(size + 1);

				const DWORD readBuffSize = 8192;
				size_t read_pos{ 0 };

				DWORD dwRead{ 0 };

				this->Read((BYTE*)text.get(), readBuffSize, &dwRead);

				read_pos += dwRead;

				while (dwRead > 0 && (read_pos + 0) < size)
				{
					this->Read((BYTE*)&(text.get()[read_pos]), readBuffSize, &dwRead);

					read_pos += dwRead;
				}

				text.get()[size] = '\0';

				string all((char*)text.get());
				wstring out_all(all.begin(), all.end());
				lines = ItsString::Split(out_all, lineDelimiter);
			}
			else if (this->m_textType == ItsFileTextType::Unicode)
			{
				this->SetFilePosition(2, ItsFilePosition::FileBegin);

				unique_ptr<BYTE[]> text = make_unique<BYTE[]>(size);

				const DWORD readBuffSize = 8192;
				size_t read_pos{ 0 };

				DWORD dwRead{ 0 };

				this->Read((BYTE*)text.get(), readBuffSize, &dwRead);

				read_pos += dwRead;

				while (dwRead > 0 && (read_pos + 2) < size)
				{
					this->Read((BYTE*)&(text.get()[read_pos]), readBuffSize, &dwRead);

					read_pos += dwRead;
				}

				text.get()[size - 2] = '\0';
				text.get()[size - 1] = '\0';

				wstring all((wchar_t*)text.get());
				lines = ItsString::Split(all, lineDelimiter);
			}
			else if (this->m_textType == ItsFileTextType::UTF8WithBOM)
			{
				this->SetFilePosition(3, ItsFilePosition::FileBegin);

				unique_ptr<BYTE[]> text = make_unique<BYTE[]>(size);

				const DWORD readBuffSize = 8192;
				size_t read_pos{ 0 };

				DWORD dwRead{ 0 };

				this->Read((BYTE*)text.get(), readBuffSize, &dwRead);

				read_pos += dwRead;

				while (dwRead > 0 && (read_pos + 3) < size)
				{
					this->Read((BYTE*)&(text.get()[read_pos]), readBuffSize, &dwRead);

					read_pos += dwRead;
				}

				text.get()[size - 3] = '\0';
				text.get()[size - 2] = '\0';
				text.get()[size - 1] = '\0';

				wstring all = ItSoftware::Encoding::UTF8::ToString((char*)text.get());
				lines = ItsString::Split(all, lineDelimiter);
			}
			else if (this->m_textType == ItsFileTextType::UTF8NoBOM)
			{
				this->SetFilePosition(0, ItsFilePosition::FileBegin);

				unique_ptr<BYTE[]> text = make_unique<BYTE[]>(size);

				const DWORD readBuffSize = 8192;
				size_t read_pos{ 0 };

				DWORD dwRead{ 0 };

				this->Read((BYTE*)text.get(), readBuffSize, &dwRead);

				read_pos += dwRead;

				while (dwRead > 0 && (read_pos + 3) < size)
				{
					this->Read((BYTE*)&(text.get()[read_pos]), readBuffSize, &dwRead);

					read_pos += dwRead;
				}

				text.get()[size - 3] = '\0';
				text.get()[size - 2] = '\0';
				text.get()[size - 1] = '\0';

				wstring all = ItSoftware::Encoding::UTF8::ToString((char*)text.get());
				lines = ItsString::Split(all, lineDelimiter);
			}
			else {
				return false;
			}

			return true;
		}

		//
		// Method: ReadTextAll
		//
		bool ReadTextAll(wstring& out)
		{
			if (!this->m_handle.IsValid())
			{
				return false;
			}

			size_t size{ 0 };
			if (!this->GetFileSize(&size))
			{
				return false;
			}

			if (size == 0) {
				return true;
			}

			if (this->m_textType == ItsFileTextType::Ansi)
			{
				this->SetFilePosition(0, ItsFilePosition::FileBegin);

				unique_ptr<BYTE[]> text = make_unique<BYTE[]>(size + 1);

				const DWORD readBuffSize = 8192;
				size_t read_pos{ 0 };

				DWORD dwRead{ 0 };

				this->Read((BYTE*)text.get(), readBuffSize, &dwRead);

				read_pos += dwRead;

				while (dwRead > 0 && (read_pos + 0) < size)
				{
					this->Read((BYTE*)&(text.get()[read_pos]), readBuffSize, &dwRead);

					read_pos += dwRead;
				}

				text.get()[size] = '\0';

				string all((char*)text.get());
				wstring out_all(all.begin(), all.end());
				out = out_all;
			}
			else if (this->m_textType == ItsFileTextType::Unicode)
			{
				this->SetFilePosition(2, ItsFilePosition::FileBegin);

				unique_ptr<BYTE[]> text = make_unique<BYTE[]>(size);

				const DWORD readBuffSize = 8192;
				size_t read_pos{ 0 };

				DWORD dwRead{ 0 };

				this->Read((BYTE*)text.get(), readBuffSize, &dwRead);

				read_pos += dwRead;

				while (dwRead > 0 && (read_pos + 2) < size)
				{
					this->Read((BYTE*)&(text.get()[read_pos]), readBuffSize, &dwRead);

					read_pos += dwRead;
				}

				text.get()[size - 2] = '\0';
				text.get()[size - 1] = '\0';

				wstring all((wchar_t*)text.get());
				out = all;
			}
			else if (this->m_textType == ItsFileTextType::UTF8WithBOM)
			{
				this->SetFilePosition(3, ItsFilePosition::FileBegin);

				unique_ptr<BYTE[]> text = make_unique<BYTE[]>(size);

				const DWORD readBuffSize = 8192;
				size_t read_pos{ 0 };

				DWORD dwRead{ 0 };

				this->Read((BYTE*)text.get(), readBuffSize, &dwRead);

				read_pos += dwRead;

				while (dwRead > 0 && (read_pos + 3) < size)
				{
					this->Read((BYTE*)&(text.get()[read_pos]), readBuffSize, &dwRead);

					read_pos += dwRead;
				}

				text.get()[size - 3] = '\0';
				text.get()[size - 2] = '\0';
				text.get()[size - 1] = '\0';

				wstring all = ItSoftware::Encoding::UTF8::ToString((char*)text.get());
				out = all;
			}
			else if (this->m_textType == ItsFileTextType::UTF8NoBOM)
			{
				this->SetFilePosition(0, ItsFilePosition::FileBegin);

				unique_ptr<BYTE[]> text = make_unique<BYTE[]>(size);

				const DWORD readBuffSize = 8192;
				size_t read_pos{ 0 };

				DWORD dwRead{ 0 };

				this->Read((BYTE*)text.get(), readBuffSize, &dwRead);

				read_pos += dwRead;

				while (dwRead > 0 && (read_pos + 3) < size)
				{
					this->Read((BYTE*)&(text.get()[read_pos]), readBuffSize, &dwRead);

					read_pos += dwRead;
				}

				text.get()[size - 3] = '\0';
				text.get()[size - 2] = '\0';
				text.get()[size - 1] = '\0';

				wstring all = ItSoftware::Encoding::UTF8::ToString((char*)text.get());
				out = all;
			}
			else {
				return false;
			}

			return true;
		}

		//
		// Method: WriteText
		//
		bool WriteText(string text)
		{
			wstring txt(text.begin(), text.end());
			return this->WriteText(txt);
		}

		//
		// Method: WriteText
		//
		bool WriteText(wstring text)
		{
			if (this->m_handle.IsInvalid())
			{
				return false;
			}

			if (this->m_textType == ItsFileTextType::Ansi)
			{
				string txt(text.begin(), text.end());
				DWORD dwWritten{ 0 };
				return this->Write((BYTE*)txt.data(), (DWORD)txt.size(), &dwWritten);
			}
			else if (this->m_textType == ItsFileTextType::Unicode) {
				DWORD dwWritten{ 0 };
				return this->Write((BYTE*)text.data(), (DWORD)(text.size() * sizeof(wchar_t)), &dwWritten);
			}
			else if (this->m_textType == ItsFileTextType::UTF8WithBOM) {
				long cbLength{ 0 };
				unique_ptr<BYTE[]> pData(ItSoftware::Encoding::UTF8::ToBytes(text, &cbLength));
				DWORD dwWritten{ 0 };
				return this->Write(pData.get(), cbLength, &dwWritten);
			}
			else if (this->m_textType == ItsFileTextType::UTF8NoBOM) {
				long cbLength{ 0 };
				unique_ptr<BYTE[]> pData(ItSoftware::Encoding::UTF8::ToBytes(text, &cbLength));
				DWORD dwWritten{ 0 };
				return this->Write(pData.get(), cbLength, &dwWritten);
			}

			return false;
		}

		//
		// Method: WriteTextLine
		//
		bool WriteTextLine(string text, string lineDelimiter = "\r\n")
		{
			wstring txt(text.begin(), text.end());
			wstring dl(lineDelimiter.begin(), lineDelimiter.end());
			return this->WriteTextLine(txt, dl);
		}

		//
		// Method: WriteTextLine
		//
		bool WriteTextLine(wstring text, wstring lineDelimiter = L"\r\n")
		{
			if (this->m_handle.IsInvalid())
			{
				return false;
			}

			if (!this->WriteText(text))
			{
				return false;
			}

			if (!this->WriteText(lineDelimiter))
			{
				return false;
			}

			return true;
		}
	};



	//
	// class: ItsFile
	//
	// (i): Wrapper for a file on Windows.
	//
	class ItsFile : public ItsFileBase
	{
	private:
	protected:
	public:

		//
		// Constructor
		//
		ItsFile()
		{
		}

		//
		// Method: OpenOrCreate
		//
		bool OpenOrCreate(wstring filename, wstring accessReadWrite, wstring shareReadWrite, ItsFileOpenCreation flagCreation)
		{
			if (this->m_handle.IsValid())
			{
				return false;
			}

			DWORD dwAccess = 0;
			if (accessReadWrite.size() != 0)
			{
				if (accessReadWrite.find(L"r") != wstring::npos) {
					dwAccess |= GENERIC_READ;
				}
				if (accessReadWrite.find(L"w") != wstring::npos) {
					dwAccess |= GENERIC_WRITE;
				}
			}

			DWORD dwShare = 0;
			if (shareReadWrite.size() != 0)
			{
				if (shareReadWrite.find(L"r") != wstring::npos) {
					dwShare |= FILE_SHARE_READ;
				}
				if (shareReadWrite.find(L"w") != wstring::npos) {
					dwShare |= FILE_SHARE_WRITE;
				}
				if (shareReadWrite.find(L"d") != wstring::npos) {
					dwShare |= FILE_SHARE_DELETE;
				}
			}

			DWORD dwFlags = 0;
			if (flagCreation == ItsFileOpenCreation::CreateAlways) {
				dwFlags = CREATE_ALWAYS;
			}
			else if (flagCreation == ItsFileOpenCreation::CreateNew) {
				dwFlags = CREATE_NEW;
			}
			else if (flagCreation == ItsFileOpenCreation::OpenAlways) {
				dwFlags = OPEN_ALWAYS;
			}
			else if (flagCreation == ItsFileOpenCreation::OpenExisting) {
				dwFlags = OPEN_EXISTING;
			}
			else if (flagCreation == ItsFileOpenCreation::TruncateExisting) {
				dwFlags = TRUNCATE_EXISTING;
			}

			this->m_handle = CreateFile(filename.c_str(), dwAccess, dwShare, NULL, dwFlags, FILE_ATTRIBUTE_NORMAL, NULL);

			return this->m_handle.IsValid();
		}

		//
		// Method: GetFileSize
		//
		static bool GetFileSize(wstring filename, size_t* size)
		{
			ItsFile file;
			if (!file.OpenOrCreate(filename, L"r", L"r", ItsFileOpenCreation::OpenExisting))
			{
				return false;
			}

			LARGE_INTEGER tmp{ 0 };
			bool result = (bool)GetFileSizeEx(file.m_handle.p(), &tmp);
			*size = tmp.QuadPart;
			return result;
		}

		//
		// Method: Delete
		//
		static bool Delete(wstring filename)
		{
			return (bool)DeleteFile(filename.c_str());
		}

		//
		// Method: Copy
		//
		static bool Copy(wstring existingFilename, wstring newFilename, bool failIfExists)
		{
			return (bool)CopyFile(existingFilename.c_str(), newFilename.c_str(), (failIfExists) ? TRUE : FALSE);
		}

		//
		// Method: Move
		//
		static bool Move(wstring existingFilename, wstring newFilename, bool failIfExists)
		{
			return (bool)MoveFileEx(existingFilename.c_str(), newFilename.c_str(), MOVEFILE_COPY_ALLOWED | ((failIfExists) ? MOVEFILE_REPLACE_EXISTING : 0));
		}

		//
		// Method: Exists
		//
		static bool Exists(wstring filename)
		{
			DWORD fileAttributes = GetFileAttributes(filename.c_str());
			return (fileAttributes != INVALID_FILE_ATTRIBUTES && !(fileAttributes & FILE_ATTRIBUTE_DIRECTORY));
		}

		//
		// Method: SetFileSize
		//
		static bool SetSize(wstring filename, size_t pos)
		{
			ItsFile file;
			if (!file.OpenOrCreate(filename, L"rw", L"", ItsFileOpenCreation::OpenExisting)) {
				return false;
			}
			return file.SetFileSize(pos);
		}

		static bool Shred(wstring filename, bool alsoDelete)
		{
			if (!ItsFile::Exists(filename))
			{
				return false;
			}

			size_t fileSize = 0;
			if (!ItsFile::GetFileSize(filename, &fileSize)) 
			{
				return false;
			}
			if (fileSize == 0)
			{
				return false;
			}

			ItsFile f;
			if (!f.OpenOrCreate(filename, L"rw", L"r", ItsFileOpenCreation::OpenExisting)) 
			{
				return false;
			}
			if (f.IsInvalid())
			{
				return false;
			}

			const uint32_t bufferSize = 2048;
			DWORD bytesWritten = 0;
			size_t totalWritten = 0;
			const unique_ptr<uint8_t[]> pdata = make_unique<uint8_t[]>(bufferSize);
			for (uint32_t i = 0; i < bufferSize; i++) {
				pdata[i] = 0xFF;
			}
			f.SetFilePosition(0,ItsFilePosition::FileBegin);
			while (totalWritten < fileSize) {
				f.Write(static_cast<const void*>(pdata.get()), static_cast<DWORD>(((fileSize - totalWritten) > bufferSize) ? bufferSize : (fileSize - totalWritten)), &bytesWritten);
				totalWritten += bytesWritten;
			}
			f.Close();

			if (alsoDelete)
			{
				return ItsFile::Delete(filename);
			}

			return true;
		}

		static bool ShredAndDelete(wstring filename) 
		{
			return ItsFile::Shred(filename, true);
		}
	};

#pragma warning(disable: 26812)
	enum EREGCLASS
	{
		EHKEY_CLASSES_ROOT = 0,
		EHKEY_CURRENT_USER = 1,
		EHKEY_LOCAL_MACHINE = 2,
		EHKEY_USERS = 3,
		EHKEY_PERFORMANCE_DATA = 4,
		EHKEY_CURRENT_CONFIG = 5,
		EHKEY_DYN_DATA = 6
	};
#pragma warning(error: 26812)
	//
	// class: ItsRegistry
	//
	// (i): Windows registry routines.
	//
	class ItsRegistry
	{
	private:
		//
		// Function: CheckHKey
		//
		// (i)	Checks to see wheather or not a valid REGCLASS value has been
		//		passed. If so the function returnes true, else it returnes false.
		//
		static bool CheckHKey(EREGCLASS a_regclass, HKEY& a_hKey)
		{
			switch (a_regclass)
			{
			case EREGCLASS(0):
				a_hKey = HKEY_CLASSES_ROOT;
				break;
			case EREGCLASS(1):
				a_hKey = HKEY_CURRENT_USER;
				break;
			case EREGCLASS(2):
				a_hKey = HKEY_LOCAL_MACHINE;
				break;
			case EREGCLASS(3):
				a_hKey = HKEY_USERS;
				break;
			case EREGCLASS(4):
				a_hKey = HKEY_PERFORMANCE_DATA;
				break;
			case EREGCLASS(5):
				a_hKey = HKEY_CURRENT_CONFIG;
				break;
			case EREGCLASS(6):
				a_hKey = HKEY_DYN_DATA;
				break;
			default:
				return false;
				break;
			};

			return true;
		}// bool CSettings::CheckHKey( long a_lhKey, HKEY &a_hKey )
	public:
		//
		// Function: EnumerateKeys
		//
		static bool EnumerateKeys(EREGCLASS eregclass, const wstring path, vector<wstring>* result)
		{
			HKEY hKey;
			if (!CheckHKey(eregclass, hKey) || path.size() == 0 || result == nullptr)
			{
				return false;
			}

			HKEY hOpenKey;
			LONG lResult(0);
			lResult = RegOpenKeyEx(hKey, path.c_str(), 0, KEY_ENUMERATE_SUB_KEYS, &hOpenKey);
			if (lResult != ERROR_SUCCESS)
			{
				return false;
			}

			result->clear();

			long lIndex(0);
			ULONG cbAvailable(0);
			wchar_t* pwcsName = new wchar_t[512];

			FILETIME ft;
			do
			{
				cbAvailable = 512;
				lResult = RegEnumKeyEx(hOpenKey, lIndex++, pwcsName, &cbAvailable, 0, NULL, 0, &ft);

				if (lResult == ERROR_SUCCESS)
				{
					result->push_back(pwcsName);
				}// if ( lResult == ERROR_SUCCESS ) {			

			} while (lResult == ERROR_SUCCESS);

			delete[] pwcsName;

			if (lResult != ERROR_NO_MORE_ITEMS)
			{
				RegCloseKey(hOpenKey);
				return false;
			}

			RegCloseKey(hOpenKey);

			return true;
		}

		//
		// Function: EnumerateValues
		//
		static bool EnumerateValues(EREGCLASS eregclass, wstring path, const wstring key, vector<wstring>* result)
		{
			HKEY hKey;
			if (!CheckHKey(eregclass, hKey) || (path.size() == 0 && key.size() == 0) || result == nullptr)
			{
				return false;
			}

			if (path.size() > 0 && path[path.size() - 1] != L'\\')
			{
				path += L'\\';
			}
			path += key;

			HKEY hOpenKey;
			LONG lResult(0);
			lResult = RegOpenKeyEx(hKey, path.c_str(), 0, KEY_QUERY_VALUE, &hOpenKey);
			if (lResult != ERROR_SUCCESS)
			{
				return false;
			}

			result->clear();

			long lIndex(0);
			ULONG cbAvailable(0);
			wchar_t* pwcsName = new wchar_t[512];

			do
			{
				cbAvailable = 512;
				lResult = RegEnumValue(hOpenKey, lIndex++, pwcsName, &cbAvailable, NULL, NULL, NULL, NULL);
				if (lResult == ERROR_SUCCESS)
				{
					result->push_back(pwcsName);
				}// if ( lResult == ERROR_SUCCESS ) {			
			} while (lResult == ERROR_SUCCESS);

			delete[] pwcsName;

			if (lResult != ERROR_NO_MORE_ITEMS)
			{
				return false;
			}

			RegCloseKey(hOpenKey);

			return true;
		}

		//
		// Function: ReadValue
		//
		static bool ReadValue(EREGCLASS eregclass, wstring path, wstring key, wstring value, wstring* data)
		{
			HKEY hKey;
			if (!CheckHKey(eregclass, hKey) || (path.size() == 0 && key.size() == 0) || data == nullptr)
			{
				return false;
			}

			if (path.size() > 0 && path[path.size() - 1] != L'\\')
			{
				path += L'\\';
			}
			path += key;

			HKEY hOpenKey;
			LONG lResult(0);
			lResult = RegOpenKeyEx(hKey, path.c_str(), 0, KEY_READ, &hOpenKey);
			if (lResult != ERROR_SUCCESS)
			{
				return false;
			}

			DWORD dwType(0);
			ULONG lSize(0);
			lResult = RegQueryValueEx(hOpenKey, value.c_str(), 0, &dwType, NULL, &lSize);
			if (lResult != ERROR_SUCCESS)
			{
				RegCloseKey(hOpenKey);
				return false;
			}

			if (dwType != REG_SZ)
			{
				RegCloseKey(hOpenKey);
				return false;
			}

			wchar_t* pszBuffer = new wchar_t[lSize / sizeof(wchar_t)];
			lResult = RegQueryValueEx(hOpenKey, value.c_str(), 0, &dwType, (BYTE*)pszBuffer, &lSize);
			if (lResult != ERROR_SUCCESS)
			{
				RegCloseKey(hOpenKey);
				delete[] pszBuffer;
				return false;
			}

			RegCloseKey(hOpenKey);

			*data = wstring(pszBuffer);

			return true;
		}

		//
		// Function: DeleteKey
		//
		static bool DeleteKey(EREGCLASS eregclass, wstring path, wstring key)
		{
			HKEY hKey;
			if (!CheckHKey(eregclass, hKey) || (path.size() == 0 && key.size() == 0))
			{
				return false;
			}

			if (path.size() > 0 && path[path.size() - 1] != L'\\')
			{
				path += L'\\';
			}
			path += key;

			DWORD dwResult(0);
			dwResult = ::SHDeleteKey(hKey, path.c_str());
			if (dwResult != ERROR_SUCCESS) {
				if (GetLastError() != 0) {
					return false;
				}
				else {
					return false;
				}
			}

			return true;
		}

		//
		// Function: CreateKey
		//
		static bool CreateKey(EREGCLASS eregclass, wstring path, wstring key, wstring default_data)
		{
			HKEY hKey;
			if (!CheckHKey(eregclass, hKey) || (path.size() == 0 && key.size() == 0))
			{
				return false;
			}

			if (path.length() > 0) {
				wchar_t wch = path.at(0);
				while (wch == L'\\') {
					path.erase(0);
					if (path.length() > 0) {
						wch = path.at(0);
					}
					else {
						break;
					}
				}
			}

			if (path.length() > 0) {
				if (path.at(path.length() - 1) != L'\\') {
					path += L'\\';
				}
			}

			path += key;

			// Create new key
			HKEY hNewKey;
			LONG lResult = RegCreateKeyEx(hKey, path.c_str(), 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hNewKey, NULL);
			if (lResult != ERROR_SUCCESS) {
				return false;
			}

			// Set default value.	
			if (default_data.size() > 0) {
				lResult = RegSetValueEx(hNewKey, NULL, 0, REG_SZ, (CONST BYTE*)default_data.data(), (static_cast<DWORD>(default_data.size()) * sizeof(wchar_t)) + 2);
				if (lResult != ERROR_SUCCESS) {
					RegCloseKey(hNewKey);	// Close New Key				
					return false;
				}
			}

			RegCloseKey(hNewKey);	// Close New Key

			return true;
		}

		//
		// Function: AddValue
		//
		static bool AddValue(EREGCLASS eregclass, wstring path, wstring key, wstring value, wstring data)
		{
			HKEY hKey;
			if (!CheckHKey(eregclass, hKey) || (path.size() == 0 && key.size() == 0) || value.size() == 0)
			{
				return false;
			}

			if (path.size() > 0 && path[path.size() - 1] != L'\\')
			{
				path += L'\\';
			}
			path += key;

			HKEY hOpenKey;
			LONG lResult(0);
			lResult = RegOpenKeyEx(hKey, path.c_str(), 0, KEY_ALL_ACCESS, &hOpenKey);
			if (lResult != ERROR_SUCCESS) {
				return false;
			}

			lResult = RegSetValueEx(hOpenKey, value.c_str(), 0, REG_SZ, reinterpret_cast<const BYTE*>(data.data()), (static_cast<DWORD>(data.size()) * sizeof(wchar_t)) + 2);
			if (lResult != ERROR_SUCCESS) {
				RegCloseKey(hOpenKey);
				return false;
			}

			RegCloseKey(hOpenKey);

			return true;
		}

		//
		// Function: DeleteValue
		//
		static bool DeleteValue(EREGCLASS eregclass, wstring path, wstring key, wstring value)
		{
			HKEY hKey;
			if (!CheckHKey(eregclass, hKey) || (path.size() == 0 && key.size() == 0) || value.size() == 0)
			{
				return false;
			}

			if (path.size() > 0 && path[path.size() - 1] != L'\\')
			{
				path += L'\\';
			}
			path += key;

			HKEY hOpenKey;
			LONG lResult(0);
			lResult = RegOpenKeyEx(hKey, path.c_str(), 0, KEY_ALL_ACCESS, &hOpenKey);
			if (lResult != ERROR_SUCCESS) {
				return false;
			}
			lResult = RegDeleteValue(hOpenKey, value.c_str());
			if (lResult != ERROR_SUCCESS) {
				RegCloseKey(hOpenKey);
				return false;
			}

			RegCloseKey(hOpenKey);

			return true;
		}
	};// ItsRegistry

	//
	// enum: ItsFileMonitorMask
	//
	// (i): mask for ItsFileMonitor
	//
	enum ItsFileMonitorMask : uint32_t {
		ChangeFileName = FILE_NOTIFY_CHANGE_FILE_NAME,
		ChangeDirName = FILE_NOTIFY_CHANGE_DIR_NAME,
		ChangeAttributes = FILE_NOTIFY_CHANGE_ATTRIBUTES,
		ChangeSize = FILE_NOTIFY_CHANGE_SIZE,
		ChangeLastWrite = FILE_NOTIFY_CHANGE_LAST_WRITE,
		ChangeSecurity = FILE_NOTIFY_CHANGE_SECURITY,
		ChangeCreation = FILE_NOTIFY_CHANGE_CREATION
	};

	struct ItsFileMonitorEvent {
		DWORD         Action;
		LARGE_INTEGER CreationTime;
		LARGE_INTEGER LastModificationTime;
		LARGE_INTEGER LastChangeTime;
		LARGE_INTEGER LastAccessTime;
		LARGE_INTEGER FileSize;
		DWORD         FileAttributes;
		DWORD         ReparsePointTag;
		LARGE_INTEGER FileId;
		LARGE_INTEGER ParentFileId;
		wstring		  FileName;
	};

	//
	// class: ItsFileMonitor
	//
	// (i): Monitors a given file folder
	//
	class ItsFileMonitor
	{
	private:

		uint32_t m_mask;
		thread m_thread;
		unique_handle_handle m_dirHandle;
		wstring m_pathname;
		DWORD m_dwBytesReturned;
		unique_ptr<BYTE[]> m_pbuffer;
		bool m_bWatchSubTree;
		bool m_bPaused;
		bool m_bStopped;
		OVERLAPPED m_o{ 0 };
		function<void(ItsFileMonitorEvent&)> m_func;
	protected:
		void ExecuteDispatchThread(function<void(ItsFileMonitorEvent&)> func)
		{
			this->m_o.Pointer = this;
			this->m_o.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
			BOOL bResult = ::ReadDirectoryChangesExW(this->m_dirHandle.operator HANDLE(),
				this->m_pbuffer.get(),
				sizeof(FILE_NOTIFY_EXTENDED_INFORMATION) + MAX_PATH,
				this->m_bWatchSubTree,
				this->m_mask,
				&this->m_dwBytesReturned,
				&this->m_o,
				NULL,
				ReadDirectoryNotifyExtendedInformation);
					
			if (!bResult) {
				this->Stop();
			}

			while (!this->m_bStopped) {
				if (this->m_o.hEvent != nullptr) {
					DWORD dw = WaitForSingleObject(this->m_o.hEvent, 0);
					if (dw == WAIT_OBJECT_0) {
						bResult = ::GetOverlappedResult(this->m_dirHandle, &this->m_o, &this->m_dwBytesReturned, FALSE);
						if (bResult) {
							FILE_NOTIFY_EXTENDED_INFORMATION* ptr = reinterpret_cast<FILE_NOTIFY_EXTENDED_INFORMATION*>(this->m_pbuffer.get());

							wstring name = ((ptr->FileNameLength != 0) ? wstring(ptr->FileName) : wstring{ L"." });

							DWORD action = ptr->Action;

							ItsFileMonitorEvent event;
							event.Action = action;
							event.CreationTime = ptr->CreationTime;
							event.LastModificationTime = ptr->LastModificationTime;
							event.LastChangeTime = ptr->LastChangeTime;
							event.LastAccessTime = ptr->LastAccessTime;
							event.FileSize = ptr->FileSize;
							event.FileAttributes = ptr->FileAttributes;
							event.ReparsePointTag = ptr->ReparsePointTag;
							event.FileId = ptr->FileId;
							event.ParentFileId = ptr->ParentFileId;
							event.FileName = name;

							this->m_func(event);

							memset(static_cast<void*>(this->m_pbuffer.get()), 0, sizeof(FILE_NOTIFY_EXTENDED_INFORMATION) + MAX_PATH);
						}

						::ResetEvent(this->m_o.hEvent);
				
						::ReadDirectoryChangesExW(this->m_dirHandle.operator HANDLE(),
							this->m_pbuffer.get(),
							sizeof(FILE_NOTIFY_EXTENDED_INFORMATION) + MAX_PATH,
							this->m_bWatchSubTree,
							this->m_mask,
							&this->m_dwBytesReturned,
							&this->m_o,
							NULL,
							ReadDirectoryNotifyExtendedInformation);
					}
				}

				if (this->m_o.hEvent != nullptr) {
					CloseHandle(this->m_o.hEvent);
				}
			}
		}

	public:
		ItsFileMonitor(const wstring pathname, bool watchSubTree, function<void(ItsFileMonitorEvent&)> func)
			: ItsFileMonitor(pathname, watchSubTree, ItsFileMonitorMask::ChangeLastWrite, func)
		{

		}
		ItsFileMonitor(const wstring pathname, bool watchSubTree, uint32_t mask, function<void(ItsFileMonitorEvent&)> func)
			: m_pathname(pathname),
			m_mask(mask),
			m_bWatchSubTree(watchSubTree),
			m_bPaused(false),
			m_bStopped(false),
			m_func(func)
		{
			if (ItsFile::Exists(this->m_pathname)) {
				this->m_dirHandle = CreateFile(this->m_pathname.c_str(),
					FILE_LIST_DIRECTORY,
					FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
					NULL);

				if (this->m_dirHandle.IsValid()) {
					this->m_pbuffer = make_unique<BYTE[]>(sizeof(FILE_NOTIFY_EXTENDED_INFORMATION) + MAX_PATH);
					this->m_thread = thread(&ItsFileMonitor::ExecuteDispatchThread, this, func);
				}
			}
		}
		void Pause() {
			this->m_bPaused = true;
		}
		void Resume() {
			this->m_bPaused = false;
		}
		bool IsPaused() {
			return this->m_bPaused;
		}
		void Stop() {
			this->m_bStopped = true;
		}
		bool IsStopped()
		{
			return this->m_bStopped;
		}
		~ItsFileMonitor()
		{
			this->Stop();

			if (this->m_thread.joinable()) {
				this->m_thread.join();
			}
		}
	};
}// namespace ItSoftware::Win::Core