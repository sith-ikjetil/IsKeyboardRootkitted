///////////////////////////////////////////////////////////////////
//: Title        : itsoftware-com.cpp
//: Product:     : Cpp.Include.Windows
//: Date         : 2020-05-01
//: Author       : "Kjetil Kristoffer Solberg" <post@ikjetil.no>
//: Version      : 1.0.0.0
//: Descriptions : Implementation of Cpp.Include.Windows. Uses Shlwapi.lib.
#pragma once
//
// #include
//
#include <windows.h>
#include <atlbase.h>
#include <string>
#include <lmerr.h>
#include "itsoftware-win-core.h"

//
// #define
//
#define ERRMSGBUFFERSIZE 512

//
// namespace
//
namespace ItSoftware::COM
{
	//
	// using namespace
	//
	//using namespace ATL;

	//
	// using
	//
	using ItSoftware::Win::Core::ItsError;

	// 
	// class: ComException.
	//
	class ItsComException
	{
	private:
		HRESULT  m_hr;
		CComBSTR m_message;
		CComBSTR m_errorInfoMessage;
		CComBSTR m_systemMessage;

		CComBSTR GetSystemMessage( HRESULT hr )
		{
			wchar_t wszMsg[1024];
			::FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, 0, wszMsg, 1024, NULL );
			CComBSTR bstr( wszMsg );
			return bstr;
		}

		CComBSTR GetHResultName( HRESULT hr )
		{
			if ( hr == S_OK )
			{
				return CComBSTR( L"S_OK" );
			}
			else if ( hr == S_FALSE )
			{
				return CComBSTR( L"S_FALSE" );
			}
			else if ( hr == E_FAIL )
			{
				return CComBSTR( L"E_FAIL" );
			}
			else if ( hr == E_OUTOFMEMORY )
			{
				return CComBSTR( L"E_OUTOFMEMORY" );
			}
			else if ( hr == E_HANDLE )
			{
				return CComBSTR( L"E_HANDLE" );
			}
			else if ( hr == E_INVALIDARG )
			{
				return CComBSTR( L"E_INVALIDARG" );
			}
			else if ( hr == CLASS_E_CLASSNOTAVAILABLE )
			{
				return CComBSTR( L"CLASS_E_CLASSNOTAVAILABLE" );
			}
			else if ( hr == E_NOINTERFACE )
			{
				return CComBSTR( L"E_NOINTERFACE" );
			}
			return CComBSTR( L"?" );
		}

	public:


		ItsComException( HRESULT hr )
			: m_hr( hr ),
			m_message( CComBSTR( L"" ) ),
			m_systemMessage( GetSystemMessage( hr ) ),
			m_errorInfoMessage( ItsError::GetErrorDescription( static_cast<DWORD>(hr) ).c_str() )
		{
		}

		ItsComException( HRESULT hr, CComBSTR message )
			: m_hr( hr ),
			m_message( message ),
			m_systemMessage( GetSystemMessage( hr ) ),
			m_errorInfoMessage(ItsError::GetErrorDescription(static_cast<DWORD>(hr)).c_str())
		{
		}

		CComBSTR GetMessage()
		{
			return this->m_message;
		}

		CComBSTR GetSystemMessage()
		{
			return this->m_systemMessage;
		}

		CComBSTR ToString()
		{
			std::wstring result;
			result.append( L"## ComException thrown. ##\n" );
			result.append( L"\n# HRESULT" );
			result.append( L"\n  0x" );
			wchar_t wszBuffer[20]{ 0 };
			_ltow_s( this->m_hr, wszBuffer, 16 );
			result.append( wszBuffer );
			result.append( L" (" ); result.append( GetHResultName( this->m_hr ) ); result.append( L")" );
			result.append( L"\n" );
			if ( this->m_systemMessage.Length() > 0 )
			{
				result.append( L"\n# System\n  " ); result.append( this->m_systemMessage.operator LPWSTR() );
				if ( result.at( result.length() - 1 ) != L'\n' )
				{
					result.append( L"\n" );
				}
			}
			if ( this->m_message.Length() > 0 )
			{
				result.append( L"\n# Message\n  " ); result.append( this->m_message.operator LPWSTR() );
				result.append( L"\n" );
			}
			if ( this->m_errorInfoMessage.Length() > 0 )
			{
				result.append( L"\n# IErrorInfo\n  " ); result.append( this->m_errorInfoMessage.operator LPWSTR() );
			}
			return CComBSTR( result.c_str() );
		}
	};

	//
	// Marshalling wrapper of interface using CoMarshalInterface
	//
	template <typename T, const IID* piid>
	class ItsMarshalPtr
	{
	private:
		bool				m_bHasMarshaled;
		bool				m_bHasUnMarshaled;
		IStream*			m_pStream;

		HRESULT ResetStreamPosition()
		{
			if (this->m_bHasMarshaled) {
				LARGE_INTEGER start = { 0 };
				ULARGE_INTEGER offset = { 0 };
				HRESULT hr = this->m_pStream->Seek(start, 0, &offset);
				if (FAILED(hr)) {
					return hr;
				}
				return S_OK;
			}
			return E_FAIL;
		}
	protected:
	public:
		ItsMarshalPtr()
		{
			this->m_bHasMarshaled = false;
			this->m_bHasUnMarshaled = false;
			this->m_pStream = nullptr;
		}

		IID GetIID()
		{
			return this->m_riid;
		}

		bool GetHasMarshaled()
		{
			return this->m_bHasMarshaled;
		}

		bool GetHasUnMarshaled()
		{
			return this->m_bHasUnMarshaled;
		}

		HRESULT Marshal(T* ptr)
		{
			if (!this->m_bHasMarshaled)
			{
				HRESULT hr = CreateStreamOnHGlobal(0, TRUE, &this->m_pStream);
				if (FAILED(hr)) {
					return hr;
				}

				hr = CoMarshalInterface(this->m_pStream, (*piid), ptr, MSHCTX_LOCAL, 0, MSHLFLAGS_NORMAL);
				if (FAILED(hr)) {
					this->m_pStream->Release();
					return hr;
				}

				this->m_bHasMarshaled = true;

				return S_OK;
			}

			return E_FAIL;
		}

		HRESULT UnMarshal(T** ptr)
		{
			if (this->m_bHasMarshaled && !this->m_bHasUnMarshaled)
			{
				this->ResetStreamPosition();

				HRESULT hr = ::CoUnmarshalInterface(this->m_pStream, (*piid), (void**)ptr);
				if (FAILED(hr)) {
					return hr;
				}
				this->m_pStream->Release();
				
				this->m_bHasUnMarshaled = true;

				return S_OK;
			}

			return E_FAIL;
		}

		HRESULT UnMarshal(CComPtr<T>& ptr)
		{
			if (this->m_bHasMarshaled && !this->m_bHasUnMarshaled)
			{
				this->ResetStreamPosition();

				T* p = nullptr;
				HRESULT hr = ::CoUnmarshalInterface(this->m_pStream, (*piid), (void**)&p);
				if (FAILED(hr)) {
					return hr;
				}
				this->m_pStream->Release();
				ptr = p;

				this->m_bHasUnMarshaled = true;

				return S_OK;
			}

			return E_FAIL;
		}
	};

	//
	// Marshalling wrapper of interface using GlobalInterfaceTable
	//
	template <typename T, const IID* piid>
	class ItsMarshalGITPtr
	{
	private:
		bool				m_bHasMarshaled;
		bool				m_bHasUnMarshaled;
		DWORD				m_dwCookie;
	protected:
	public:
		ItsMarshalGITPtr()
		{
			this->m_dwCookie = 0;
			this->m_bHasMarshaled = false;
			this->m_bHasUnMarshaled = false;
		}

		DWORD GetCookie()
		{
			return this->m_dwCookie;
		}

		IID GetIID()
		{
			return this->m_riid;
		}

		bool GetHasMarshaled()
		{
			return this->m_bHasMarshaled;
		}

		bool GetHasUnMarshaled()
		{
			return this->m_bHasUnMarshaled;
		}

		HRESULT Marshal(T* ptr)
		{
			if (!this->m_bHasMarshaled)
			{
				CComPtr<IGlobalInterfaceTable> pIGIT;
				HRESULT hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (void**)&pIGIT);
				if (FAILED(hr)) {
					return hr;
				}

				hr = pIGIT->RegisterInterfaceInGlobal(ptr, (*piid), &this->m_dwCookie);
				if (FAILED(hr)) {
					this->m_dwCookie = 0;
					return hr;
				}

				this->m_bHasMarshaled = true;

				return S_OK;
			}

			return E_FAIL;
		}

		HRESULT UnMarshal(T** ptr)
		{
			if (this->m_bHasMarshaled && !this->m_bHasUnMarshaled)
			{
				CComPtr<IGlobalInterfaceTable> pIGIT;
				HRESULT hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (void**)&pIGIT);
				if (FAILED(hr)) {
					return hr;
				}

				hr = pIGIT->GetInterfaceFromGlobal(this->m_dwCookie, (*piid), (void**)ptr);
				if (FAILED(hr)) {
					return hr;
				}

				this->m_bHasUnMarshaled = true;

				return S_OK;
			}

			return E_FAIL;
		}

		HRESULT UnMarshal(CComPtr<T>& ptr)
		{
			if (this->m_bHasMarshaled && !this->m_bHasUnMarshaled)
			{
				CComPtr<IGlobalInterfaceTable> pIGIT;
				HRESULT hr = CoCreateInstance(CLSID_StdGlobalInterfaceTable, NULL, CLSCTX_INPROC_SERVER, IID_IGlobalInterfaceTable, (void**)&pIGIT);
				if (FAILED(hr)) {
					return hr;
				}

				T* p = nullptr;
				hr = pIGIT->GetInterfaceFromGlobal(this->m_dwCookie, (*piid), (void**)&p);
				if (FAILED(hr)) {
					return hr;
				}
				ptr = p;
					
				this->m_bHasUnMarshaled = true;

				return S_OK;
			}

			return E_FAIL;
		}
	};

	//
	// struct: ComUtil
	//
	struct ItsComUtil
	{
		//
		// HR inline function for handling HRESULT codes.
		//
		static void HR( HRESULT hr )
		{
			if ( FAILED( hr ) )
			{
				throw ItsComException( hr );
			}
		}

		static void HR( HRESULT hr, HRESULT ... )
		{
			if ( hr == S_OK )
			{
				return;
			}

			va_list list;
			va_start( list, hr );

			bool failed = true;
			for ( ;;)
			{
				HRESULT argHR = va_arg( list, HRESULT );
				if ( argHR == 0 )
				{
					break;
				}
				else if ( hr == argHR )
				{
					failed = false;
					break;
				}
			}

			if ( failed )
			{
				throw ItsComException( hr );
			}
		}
		//
		// HR inline function for handling HRESULT codes.
		//

		static void HR( HRESULT hr, CComBSTR message )
		{
			if ( FAILED( hr ) )
			{
				throw ItsComException( hr, message );
			}
		}

		/*static void HR(HRESULT hr, CComBSTR message, HRESULT ...)
		{
			if ( hr == S_OK )
			{
				return;
			}

			va_list list;
			va_start( list, message );

			bool failed = true;
			for ( ;;)
			{
				HRESULT argHR = va_arg( list, HRESULT );
				if ( argHR == 0 )
				{
					break;
				}
				else if ( hr == argHR )
				{
					failed = false;
					break;
				}
			}

			if ( failed )
			{
				throw ComException( hr, message );
			}
		}*/
	};

	// 
	// enum: ComApartment.
	//
	// (i) COM apartment types.
	//
	enum class ItsComApartment
	{
		ApartmentThreaded = COINIT_APARTMENTTHREADED,
		MultiThreaded = COINIT_MULTITHREADED
	};

	//
	// class: ComRuntime.
	//
	// (i) Auto CoInitializeEx and CoUninitialize.
	//
	class ItsComRuntime
	{
	private:
		ItsComApartment m_apartment;
		bool		 m_isInitialized;
	public:
		ItsComRuntime( ItsComApartment apartment )
			: m_apartment( apartment ),
			m_isInitialized( false )
		{
			HRESULT hr = ::CoInitializeEx( nullptr, static_cast<DWORD>(apartment) );
			this->m_isInitialized = true;
		}
		~ItsComRuntime()
		{
			if ( this->m_isInitialized )
			{
				::CoUninitialize();
				this->m_isInitialized = false;
			}
		}
	};
}// namespace ItSoftware::COM