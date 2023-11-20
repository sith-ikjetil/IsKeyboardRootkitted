///////////////////////////////////////////////////////////////////
//: Title        : itsoftware.h
//: Product:     : Cpp.Include.Windows
//: Date         : 2020-05-01
//: Author       : "Kjetil Kristoffer Solberg" <post@ikjetil.no>
//: Version      : 1.0.0.0
//: Descriptions : Implementation of Cpp.Include.Windows.
#pragma once
//
// #include
//
#include <windows.h>
#include <wingdi.h>
#include <crtdbg.h>
#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <exception>
#include <algorithm>
#include "atlcomcli.h"		
#include <string>
#include <vector>
#include <codecvt>
#include <random>
#include <functional>
#include <stdlib.h>
#include <time.h>
#include "itsoftware-exceptions.h"

//
// namespace
//
namespace ItSoftware
{
	//
	// using
	//
	using std::string;
	using std::wstring;
	using std::vector;
	using std::stringstream;
	using std::wstringstream;
	using std::endl;
	using ItSoftware::Exceptions::ItsException;
	using ItSoftware::Exceptions::ItsArgumentException;
	using ItSoftware::Exceptions::ItsArgumentNullException;

	// 
	// Macros
	//
	#define DECIMALTOSTRING2(d) #d
	#define DECIMALTOSTRING1(d) DECIMALTOSTRING2(d)
	#define ERR_AT_LOCATION(e) __FILE__ "(" DECIMALTOSTRING1(__LINE__) "): " #e 
	#define ERROR_HERE __FILE__ "(" DECIMALTOSTRING2(__LINE__) "): "  
	#define CHAR_BIT 8
	#define BitsCount( val ) ( sizeof( val ) * CHAR_BIT )
	#define Shift( val, steps ) ( steps % BitsCount( val ) )
	#define ROL( val, steps ) ( ( val << Shift( val, steps ) ) | ( val >> ( BitsCount( val ) - Shift( val, steps ) ) ) )
	#define ROR( val, steps ) ( ( val >> Shift( val, steps ) ) | ( val << ( BitsCount( val ) - Shift( val, steps ) ) ) )	
	#define AddFlag( val, flag ) (val | flag)
	#define RemoveFlag(val, flag) (val & ~flag)
	
	//
	// Function: as_const
	//
	template<typename T>
	const T& as_const(const T& item) { return item; }

	//
	// Function: ItsAddFlag
	//
	template<typename T>
	T ItsAddFlag(T value, T flag)
	{
		return value | flag;
	}

	//
	// Function:: ItsRemoveFlag
	//
	template<typename T>
	T ItsRemoveFlag(T value, T flag)
	{
		return value & ~flag;
	}
	
	//
	// Encoding
	//
	namespace Encoding
	{
		//
		// struct: UTF8
		//
		// (i): UTF8 wrappers routines.
		//
		struct UTF8
		{

			static std::wstring ToString( const std::string& str )
			{
				/*typedef std::codecvt_utf8<wchar_t> convert_typeX;
				std::wstring_convert<convert_typeX, wchar_t> converterX;

				return converterX.from_bytes( str );*/
				if (str.empty()) return std::wstring();
				int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
				std::wstring wstrTo(size_needed, 0);
				MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
				return wstrTo;
			}

			static std::string ToString( const std::wstring& wstr )
			{
				/*typedef std::codecvt_utf8<wchar_t> convert_typeX;
				std::wstring_convert<convert_typeX, wchar_t> converterX;

				return converterX.to_bytes( wstr );*/
				if (wstr.empty()) return std::string();
				int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
				std::string strTo(size_needed, 0);
				WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
				return strTo;
			}

			static unsigned char* ToBytes( const std::wstring& wstr, long* cbLength )
			{
				/*typedef std::codecvt_utf8<wchar_t> convert_typeX;
				std::wstring_convert<convert_typeX, wchar_t> converterX;

				std::string s = converterX.to_bytes( wstr );*/

				std::string s = UTF8::ToString(wstr);

				size_t length = s.length( ) * sizeof( char );
				unsigned char* pBytes = new ( std::nothrow ) unsigned char[length];
				if ( pBytes == nullptr )
				{
					*cbLength = 0;
					return nullptr;
				}

				memcpy( (void*) pBytes, s.data( ), length );

				*cbLength = (long) length;

				return pBytes;
			}
		};
	}
	
	//
	// struct: ItsTime
	//
	// (i): Split milliseconds or render milliseconds to a friendly string.
	//
	struct ItsTime
	{
		static void SplitMsToHourMinuteSeconds( size_t time_ms, size_t* part_hours, size_t* part_minutes, size_t* part_seconds, size_t* part_ms )
		{
			size_t seconds = time_ms / 1000;

			*part_hours = 0;
			*part_minutes = 0;;
			*part_seconds = 0;
			*part_ms = 0;

			*part_hours = seconds / 3600;
			*part_minutes = ( seconds - ( *part_hours * 3600 ) ) / 60;
			*part_seconds = seconds - ( *part_hours * 3600 ) - ( *part_minutes * 60 );
			*part_ms = time_ms - ( *part_seconds * 1000 ) - ( *part_minutes * 60 * 1000 ) - ( *part_hours * 3600 * 1000 );
		}		

		static void SplitHourToYearWeekDayHour( size_t houIn, size_t* houRest, size_t* day, size_t* week, size_t* year )
		{
			*houRest = houIn;

			*day = houIn / 24;
			*week = *day / 7;
			*year = *week / 52;

			*day -= ( *week * 7 );

			*week -= ( *year * 52 );

			*houRest -= *week * 7 * 24;
			*houRest -= *day * 24;
			*houRest -= *year * 52 * 7 * 24;
		}		

		static wstring RenderMsToFullString(size_t milliseconds, bool bWithMilliseconds)
		{			
			size_t hou = 0;
			size_t min = 0;
			size_t sec = 0;
			size_t ms = 0;
			ItsTime::SplitMsToHourMinuteSeconds(milliseconds, &hou, &min, &sec, &ms);

			size_t houRest = 0;
			size_t day = 0;
			size_t week = 0;
			size_t year = 0;
			ItsTime::SplitHourToYearWeekDayHour(hou, &houRest, &day, &week, &year);

			wstringstream ss;
			if (year > 0) {
				if (year == 1)
				{
					ss << year << L" year ";
				}
				else
				{
					ss << year << L" years ";
				}
			}
			if (week > 0 || year > 0) {
				if (week == 1 || week == 0) {
					ss << week << L" week ";
				}
				else
				{
					ss << week << L" weeks ";
				}
			}
			if (day > 0 || week > 0 || year > 0) {
				if (day == 1 || day == 0)
				{
					ss << day << L" day ";
				}
				else
				{
					ss << day << L" days ";
				}
			}
			if (houRest > 0 || day > 0 || week > 0 || year > 0)
			{
				if (houRest == 1 || houRest == 0)
				{
					ss << houRest << L" hour ";
				}
				else
				{
					ss << houRest << L" hours ";
				}
			}

			if (min < 10) {
				ss << L"0" << min << L":";
			}
			else
			{
				ss << min << L":";
			}
			if (sec < 10) {
				ss << L"0" << sec;
			}
			else
			{
				ss << sec;
			}

			if (bWithMilliseconds)
			{
				if (ms < 10) {
					ss << L".00" << ms;
				}
				else if (ms < 100) {
					ss << L".0" << ms;
				}
				else {
					ss << L"." << ms;
				}
			}

			wstring str = ss.str();

			return str;
		}
	};

	//
	// enum ItsExpandDirection
	//
	// (i): Where text in expanded string input text should be aligned.
	//
	enum class ItsExpandDirection
	{
		Left,
		Middle,
		Right
	};

	//
	// struct: ItsString
	//
	// (i): Misc. string routines in one place.
	//
	struct ItsString
	{
		static wstring WidthExpand(wstring source, size_t width, wchar_t fill, ItsExpandDirection direction)
		{
			if (source.size() == 0) {
				return wstring(L"");
			}
			if (width <= 0) {
				return wstring(L"");
			}

			if (source.size() >= width) {
				return source.substr(0, width);
			}

			wstringstream result;
			if (direction == ItsExpandDirection::Left) 
			{
				for (size_t i = 0; i < (width - source.size()); i++)
				{
					result << fill;
				}
				result << source;
			}
			else if (direction == ItsExpandDirection::Middle)
			{
				for (size_t i = 0; i < ((width - source.size()) / 2); i++)
				{
					result << fill;
				}

				result << source;

				for (size_t i = result.str().size(); i < width; i++)
				{
					result << fill;
				}
			}
			else if (direction == ItsExpandDirection::Right) 
			{
				result << source;

				for (size_t i = 0; i < (width - source.size()); i++)
				{
					result << fill;
				}
			}
			else {
				return wstring(L"");
			}

			wstring retVal = result.str();
			return retVal;
		}

		static vector<wstring> Split( wstring data, wstring token )
		{
			vector<wstring> output;
			size_t pos = wstring::npos; // size_t to avoid improbable overflow
			do
			{
				pos = data.find( token );
				output.push_back( data.substr( 0, pos ) );
				if ( wstring::npos != pos )
					data = data.substr( pos + token.size( ) );
			} while ( wstring::npos != pos );

			return output;
		}

		static wstring ToLowerCase( wstring s )
		{
			std::transform( s.begin( ), s.end( ), s.begin( ), tolower );
			return s;
		}

		static wstring ToUpperCase( wstring s )
		{
			std::transform( s.begin( ), s.end( ), s.begin( ), toupper );
			return s;
		}

		static wstring TrimLeft( wstring s, const wchar_t* t = L" \t\n\r\f\v" )
		{
			s.erase( 0, s.find_first_not_of( t ) );
			return s;
		}

		static wstring TrimRight( wstring s, const wchar_t* t = L" \t\n\r\f\v" )
		{
			s.erase( s.find_last_not_of( t ) + 1 );
			return s;
		}

		static wstring Trim( wstring s, const wchar_t* t = L" \t\n\r\f\v" )
		{
			wstring right = TrimRight(s, t);
			return TrimLeft( right, t );
		}

		static wstring Left( wstring s, unsigned int count )
		{
			if ( s.size( ) == 0 || count <= 0 )
			{
				return wstring( L"" );
			}

			if ( count >= s.size( ) )
			{
				return s;
			}

			wstringstream ss;
			for ( unsigned int i = 0; i < count; i++ )
			{
				ss << s[i];
			}

			wstring str = ss.str( );
			return str;
		}
		
		static wstring Mid( wstring s, size_t index, size_t count )
		{
			if ( s.size( ) == 0 || count <= 0 || index < 0 || index >= s.size( ) )
			{
				return wstring( L"" );
			}

			if ( index + count >= s.size( ) )
			{
				wstringstream ss;
				for ( size_t i = index; i < s.size( ); i++ )
				{
					ss << s[i];
				}
				wstring str = ss.str( );
				return str;
			}

			wstringstream ss;
			for ( size_t i = index; i < index + count; i++ )
			{
				ss << s[i];
			}

			wstring str = ss.str( );
			return str;
		}
		
		static wstring Right( wstring s, unsigned int count )
		{
			if ( s.size( ) == 0 || count <= 0 )
			{
				return wstring( L"" );
			}

			if ( count >= s.size( ) )
			{
				return s;
			}

			wstringstream ss;
			for ( size_t i = s.size( ) - count; i < s.size( ); i++ )
			{
				ss << s[i];
			}

			wstring str = ss.str( );
			return str;
		}


		static wstring Replace( wstring s, wstring replace, wstring replace_with )
		{
			if ( s.size( ) == 0 || replace.size( ) == 0 || replace.size( ) > s.size( ) )
			{
				return wstring( L"" );
			}

			wstring retVal = s;
			size_t index = retVal.find( replace );
			if ( index == wstring::npos )
			{
				return retVal;
			}
			while ( index != wstring::npos )
			{
				wstringstream ss;
				ss << ItsString::Left( retVal, (int)index );
				ss << replace_with;
				ss << ItsString::Right( retVal, (int)(retVal.size( ) - index - replace.size( )) );
				
				wstring temp = ss.str();
				retVal = temp;

				index = retVal.find( replace, index + replace.size( ) );
			}

			return retVal;
		}
	};

	//
	// enum: DataSizeStringType
	//
	// (i): Enum for string representation of ToDataSizeString ItsConvert function.
	//
	enum class ItsDataSizeStringType {
		Recommended,
		IEC
	};

	//
	// struct: ItsConvert
	//
	// (i): Misc. convertion routines in one place.
	//
	struct ItsConvert
	{
		static wstring ToDataSizeString(size_t size)
		{
			return ItsConvert::ToDataSizeString(size, 0);
		}
		static wstring ToDataSizeString(size_t size, int digits)
		{
			return ItsConvert::ToDataSizeString(size, digits, ItsDataSizeStringType::Recommended);
		}
		static wstring ToDataSizeString(size_t size, int digits, ItsDataSizeStringType type)
		{
			if (digits < 0)
			{
				digits = 0;
			}
			else if (digits > 3)
			{
				digits = 3;
			}

			wstringstream ss;
			double dSize = (double)size;

			int index = 0;
			while (dSize >= 1024)
			{
				dSize /= 1024;
				index++;
			}

			vector<wstring> szSize;
			if (type == ItsDataSizeStringType::IEC) {
				szSize = { L"Bi", L"KiB", L"MiB", L"GiB", L"TiB", L"PiB", L"EiB", L"ZiB", L"YiB", L"BB", L"GP" };
			}
			else {//(type == ItsDataSizeStringType::Recommended) { // defaults to this notation
				szSize = { L"B", L"KB", L"MB", L"GB", L"TB", L"PB", L"EB", L"ZB", L"YB", L"BB", L"GP" };
			}
	
			size_t tst = (size_t)dSize;
			ss << tst;
			if (digits > 0) {
				double t = dSize - tst;
				wstring ws = ItsConvert::ToString<double>(t);
				if (ws[0] == '0' && t != 0.0) {
					ws += L"000";
				}
				else {
					ws += L".000";
				}
				ss << ws.substr(1, static_cast<size_t>(digits)+1);
			}
			ss << L" ";
			ss << ((index > (szSize.size() - 1) || index < 0) ? L"?" : szSize[index]);
			
			wstring retVal = ss.str();
			return retVal;
		}

#pragma warning(disable:4244)
		template<typename Numeric>
		static Numeric ToNumber(const wstring& str)
		{
			if (std::is_same_v<Numeric, char>) 
			{
				return static_cast<char>(std::stoi(str));
			}
			else if (std::is_same_v<Numeric, unsigned char>)
			{
				return static_cast<unsigned char>(std::stoi(str));
			}
			else if (std::is_same_v<Numeric, short>)
			{
				return static_cast<short>(std::stoi(str));
			}
			else if (std::is_same_v<Numeric, unsigned short>)
			{
				return static_cast<unsigned short>(std::stoi(str));
			}
			else if (std::is_same_v<Numeric, float>)
			{
				return std::stof(str);
			}
			else if (std::is_same_v<Numeric, double>)
			{
				return std::stod(str);
			}
			else if (std::is_same_v <Numeric, long double>)
			{
				return std::stold(str);
			}
			else if (std::is_same_v<Numeric, int>)
			{
				return std::stoi(str);
			}
			else if (std::is_same_v<Numeric, long>)
			{
				return std::stol(str);
			}
			else if (std::is_same_v<Numeric, long long>)
			{
				return std::stoll(str);
			}
			else if (std::is_same_v<Numeric, unsigned int>)
			{
				return static_cast<unsigned int>(std::stoul(str));
			}
			else if (std::is_same_v<Numeric, unsigned long>)
			{
				return std::stoul(str);
			}
			else if (std::is_same_v<Numeric, unsigned long long>)
			{
				return std::stoull(str);
			}

			Numeric x = std::stoull(str);
			return x;
		}
#pragma warning(error: 4244)
		static long ToLongFromHex(const wstring& str)
		{
			wchar_t* p;
			long n = wcstol(str.c_str(), &p, 16);
			if (*p != 0) {
				return 0;
			}
			return n;
		}

		static wstring ToString(const bool flag)
		{
			if (flag)
			{
				return wstring(L"True");
			}
			return wstring(L"False");
		}

		template<typename Numeric>
		static wstring ToString(Numeric number)
		{
			wstringstream ss;
			if (std::is_same_v<Numeric, float>)
			{
				ss << std::setprecision(std::numeric_limits<float>::digits10);
			}
			else if (std::is_same_v<Numeric, double>)
			{
				ss << std::setprecision(std::numeric_limits<double>::digits10);
			}
			else if (std::is_same_v<Numeric, long double>)
			{
				ss << std::setprecision(std::numeric_limits<long double>::digits10);
			}
			ss << number;

			wstring tos = ss.str();
			return tos;
		}

		static wstring ToStringFormatted( size_t number, wchar_t thousandSep = L',')
		{
			wstring txt = ItsConvert::ToString( number );
			
			wstringstream ss;

			int groupSize = 3;			
			int i = groupSize;
			size_t iCount = 0;
			size_t iSize = txt.size( );
			for ( auto itr = txt.rbegin( ); itr < txt.rend( ); itr++ )
			{
				if ( i-- > 0 )
				{
					ss << *itr;
					iCount++;
				}
				else
				{
					if ( iCount < iSize )
					{
						ss << thousandSep;
						ss << *itr;
						iCount++;
						i = groupSize - 1;
					}					
					else {
						i = groupSize;
					}
				}
				
			}

			txt = ss.str( );
			wstringstream ss2;
			for ( auto itr = txt.rbegin( ); itr < txt.rend( ); itr++ )
			{
				ss2 << *itr;
			}

			wstring retVal = ss2.str( );

			return retVal;
		}

		template<typename Numeric>
		static string ToHexString(Numeric number, bool uppercase = true, uint32_t width = 0, wchar_t fillChar = L'0', bool showBase = false)
		{
			wstringstream ss;
			ss << std::hex;
			if (showBase)
			{
				ss << "0x";
			}
			if (uppercase)
			{
				ss << std::uppercase;
			}
			if (width > 0)
			{
				ss << std::setfill(fillChar) << std::setw(width);
			}
			ss << number;

			wstring tos = ss.str();
			return tos;
		}
		
		static wstring ToString( tm dateTime ) {
			wstringstream ss;
			ss << std::setfill( L'0' ) << std::setw( 4 ) << ( dateTime.tm_year + 1900 );
			ss << L"-" << std::setfill( L'0' ) << std::setw( 2 ) << ( dateTime.tm_mon + 1 );
			ss << L"-" << std::setfill( L'0' ) << std::setw(2) << dateTime.tm_mday;						
			ss << L"T";
			ss << std::setfill( L'0' ) << std::setw( 2 ) << dateTime.tm_hour;
			ss << L":" << std::setfill( L'0' ) << std::setw( 2 ) << dateTime.tm_min;
			ss << L":" << std::setfill( L'0' ) << std::setw( 2 ) << dateTime.tm_sec;			

			wstring tos = ss.str( );
			return tos;
		}

		static tm ToTM( wstring dateTime ) 
		{
			tm t = { 0 };

			t.tm_year = ItsConvert::ToNumber<int>( dateTime.substr( 0, 4 ) ) - 1900;
			t.tm_mon = ItsConvert::ToNumber<int>( dateTime.substr( 5, 2 ) ) - 1;
			t.tm_mday = ItsConvert::ToNumber<int>(dateTime.substr( 8, 2 ));
			t.tm_hour = ItsConvert::ToNumber<int>( dateTime.substr( 11, 2 ) );
			t.tm_min = ItsConvert::ToNumber<int>( dateTime.substr( 14, 2 ) );
			t.tm_sec = ItsConvert::ToNumber<int>( dateTime.substr( 17, 2 ) );

			return t;
		}

		static bool ToBool( wstring flag ) {
						
			flag = ItsString::ToLowerCase( flag );

			if ( !wcscmp( flag.c_str( ), L"true" ) ) {
				return true;
			}
			else if ( !wcscmp( flag.c_str( ), L"1" ) ) {
				return true;
			}
			else if ( !wcscmp( flag.c_str( ), L"yes" ) ) {
				return true;
			}
			else if (!wcscmp(flag.c_str(), L"on")) {
				return true;
			}
			return false;
		}

		static COLORREF ToRGB( wstring color )
		{
			auto rgb = ItsString::Split( color, L"," );
			return (COLORREF)(RGB( ItsConvert::ToNumber<int>(rgb[0]), ItsConvert::ToNumber<int>(rgb[1]), ItsConvert::ToNumber<int>(rgb[2]) ));
		}

		static wstring ToString( vector<long long> pks ) 
		{
			wstringstream ss;
			bool bAddSep = false;
			for ( auto&pk : pks ) {
				if ( bAddSep ) {
					ss << L";";
				}
				ss << pk;
				bAddSep = true;
			}
			wstring tos = ss.str( );
			return tos;
		}

		static vector<long long> ToPK( wstring pks ) 
		{
			//using convert_typeX = std::codecvt_utf8<wchar_t>;
			//std::wstring_convert<convert_typeX, wchar_t> converterX;

			stringstream ss;
			//string s = converterX.to_bytes(pks);
			std::string s = ItSoftware::Encoding::UTF8::ToString(pks);
			ss << s;

			string item;
			vector<long long> topk;
			while ( getline( ss, item, ';' ) ) 
			{
				if ( !item.empty( ) ) 
				{
					topk.push_back( ItsConvert::ToNumber<long long>( wstring(item.begin(), item.end())) );
				}
			}

			return topk;
		}		

		static SYSTEMTIME ToSYSTEMTIME( tm dateTime ) 
		{
			SYSTEMTIME st = { 0 };
			
			st.wYear = static_cast<WORD>(dateTime.tm_year + 1900);
			st.wMonth = static_cast<WORD>(dateTime.tm_mon + 1);
			st.wDay = static_cast<WORD>(dateTime.tm_mday);
			st.wHour = static_cast<WORD>(dateTime.tm_hour);
			st.wMinute = static_cast<WORD>(dateTime.tm_min);
			st.wSecond = static_cast<WORD>(dateTime.tm_sec);

			return st;
		}

		static tm ToTM( SYSTEMTIME st )
		{
			tm t = { 0 };
			
			t.tm_year = st.wYear - 1900;
			t.tm_mon = st.wMonth - 1;
			t.tm_mday = st.wDay;
			t.tm_hour = st.wHour;
			t.tm_min = st.wMinute;
			t.tm_sec = st.wSecond;

			return t;
		}
	};	
	
	//
	// Function: ItsRandom
	//
	// (i): A random number wrapper.
	//
	template<typename Numeric, typename Generator = std::mt19937>
	Numeric ItsRandom(Numeric from, Numeric to)
	{
		thread_local static Generator gen(std::random_device{}());

		using dist_type = typename std::conditional
			<
			std::is_integral<Numeric>::value
			, std::uniform_int_distribution<Numeric>
			, std::uniform_real_distribution<Numeric>
			>::type;

		thread_local static dist_type dist{};

		return dist(gen, typename dist_type::param_type{ from, to });
	}

	//
	// struct: ItsDateTime
	//
	// (i): Represents a date and time.
	//
	struct ItsDateTime
	{		
	private:
		tm m_tm;

	public:
		ItsDateTime( tm timeDate ) 
		{
			this->m_tm = timeDate;
		}

		ItsDateTime(ItsDateTime& dateTime)
		{
			this->m_tm = dateTime.m_tm;
		}

		ItsDateTime(ItsDateTime&& dateTime) noexcept
		{
			this->m_tm = dateTime.m_tm;
		}

		static ItsDateTime Now( ) 
		{
			time_t t;
			time( &t );
			tm tm2;
			localtime_s( &tm2, &t );

			return ItsDateTime( tm2 );
		}

		bool operator==(const ItsDateTime& other)
		{
			if (this->m_tm.tm_year == other.m_tm.tm_year &&
				this->m_tm.tm_mon == other.m_tm.tm_mon &&
				this->m_tm.tm_mday == other.m_tm.tm_mday &&
				this->m_tm.tm_hour == other.m_tm.tm_hour &&
				this->m_tm.tm_min == other.m_tm.tm_min &&
				this->m_tm.tm_sec == other.m_tm.tm_sec &&
				this->m_tm.tm_wday == other.m_tm.tm_wday &&
				this->m_tm.tm_isdst == other.m_tm.tm_isdst &&
				this->m_tm.tm_yday == other.m_tm.tm_yday) {
				return true;
			}
			return false;
		}

		bool operator!=(const ItsDateTime& other)
		{
			if (this->m_tm.tm_year == other.m_tm.tm_year &&
				this->m_tm.tm_mon == other.m_tm.tm_mon &&
				this->m_tm.tm_mday == other.m_tm.tm_mday &&
				this->m_tm.tm_hour == other.m_tm.tm_hour &&
				this->m_tm.tm_min == other.m_tm.tm_min &&
				this->m_tm.tm_sec == other.m_tm.tm_sec &&
				this->m_tm.tm_wday == other.m_tm.tm_wday &&
				this->m_tm.tm_isdst == other.m_tm.tm_isdst &&
				this->m_tm.tm_yday == other.m_tm.tm_yday) {
				return false;
			}
			return true;
		}

		const ItsDateTime& operator=(const ItsDateTime& other)
		{
			this->m_tm.tm_year = other.m_tm.tm_year;
			this->m_tm.tm_mon = other.m_tm.tm_mon;
			this->m_tm.tm_mday = other.m_tm.tm_mday;
			this->m_tm.tm_hour = other.m_tm.tm_hour;
			this->m_tm.tm_min = other.m_tm.tm_min;
			this->m_tm.tm_sec = other.m_tm.tm_sec;
			this->m_tm.tm_wday = other.m_tm.tm_wday;
			this->m_tm.tm_isdst = other.m_tm.tm_isdst;
			this->m_tm.tm_yday = other.m_tm.tm_yday;
			
			return *this;
		}

		explicit operator wstring() {
			return this->ToString(L"s");
		}

		wstring ToString( )
		{
			wstringstream ss;
			ss << std::setfill( L'0' ) << std::setw( 4 ) << ( this->m_tm.tm_year + 1900 );
			ss << L"-" << std::setfill( L'0' ) << std::setw( 2 ) << ( this->m_tm.tm_mon + 1 );
			ss << L"-" << std::setfill( L'0' ) << std::setw( 2 ) << this->m_tm.tm_mday;
			ss << L" ";
			ss << std::setfill( L'0' ) << std::setw( 2 ) << this->m_tm.tm_hour;
			ss << L":" << std::setfill( L'0' ) << std::setw( 2 ) << this->m_tm.tm_min;
			ss << L":" << std::setfill( L'0' ) << std::setw( 2 ) << this->m_tm.tm_sec;

			wstring tos = ss.str( );
			return tos;
		}

		wstring ToString( wstring option )
		{
			if ( option == L"s" || option == L"S" )
			{
				wstringstream ss;
				ss << std::setfill( L'0' ) << std::setw( 4 ) << ( this->m_tm.tm_year + 1900 );
				ss << L"-" << std::setfill( L'0' ) << std::setw( 2 ) << ( this->m_tm.tm_mon + 1 );
				ss << L"-" << std::setfill( L'0' ) << std::setw( 2 ) << this->m_tm.tm_mday;
				ss << L"T";
				ss << std::setfill( L'0' ) << std::setw( 2 ) << this->m_tm.tm_hour;
				ss << L":" << std::setfill( L'0' ) << std::setw( 2 ) << this->m_tm.tm_min;
				ss << L":" << std::setfill( L'0' ) << std::setw( 2 ) << this->m_tm.tm_sec;

				wstring tos = ss.str( );
				return tos;
			}
			
			// day
			int d = this->GetDay( );
			wstring str_d = ItsConvert::ToString( d );
			wstring str_dd;
			if ( d < 10 )
			{
				str_dd = L"0";
				str_dd += str_d;
			}
			else
			{
				str_dd = str_d;
			}

			// month
			int m = this->GetMonth( );
			wstring str_m = ItsConvert::ToString( m );
			wstring str_mm;
			if ( m < 10 )
			{
				str_mm = L"0";
				str_mm += str_m;
			}
			else
			{
				str_mm = str_m;
			}

			// year
			int y = this->GetYear( );
			wstring str_yyyy = ItsConvert::ToString( y );
			wstring str_yy = ItsString::Right( str_yyyy, 2 );

			// hour			
			int h = this->GetHour( );
			wstring str_h = ItsConvert::ToString( h );
			wstring str_hh;
			if ( h < 10 )
			{
				str_hh = L"0";
				str_hh += str_h;
			}
			else
			{
				str_hh = str_h;
			}

			// minute
			int min = this->GetMinute( );
			wstring str_min = ItsConvert::ToString( min );
			wstring str_minmin;
			if ( min < 10 )
			{
				str_minmin = L"0";
				str_minmin += str_min;
			}
			else
			{
				str_minmin = str_min;
			}

			// second
			int s = this->GetSecond( );
			wstring str_s = ItsConvert::ToString( s );
			wstring str_ss;
			if ( s < 10 )
			{
				str_ss = L"0";
				str_ss += str_s;
			}
			else
			{
				str_ss = str_s;
			}

			wstring wdd = wstring(L"dd");
			wstring wd = wstring(L"d");
			option = ItsString::Replace( option, wdd, str_dd );
			option = ItsString::Replace( option, wd, str_d );

			wstring wMM = wstring(L"MM");
			wstring wM = wstring(L"M");
			option = ItsString::Replace( option, wMM, str_mm );
			option = ItsString::Replace( option, wM, str_m );

			wstring wyyyy = wstring(L"yyyy");
			wstring wyy = wstring(L"yy");
			option = ItsString::Replace( option, wyyyy, str_yyyy );
			option = ItsString::Replace( option, wyy, str_yy );

			wstring wHH = wstring(L"HH");
			wstring wH = wstring(L"H");
			option = ItsString::Replace( option, wHH, str_hh );
			option = ItsString::Replace( option, wH, str_h );

			wstring wmm = wstring(L"mm");
			wstring wm = wstring(L"m");
			option = ItsString::Replace( option, wmm, str_minmin );
			option = ItsString::Replace( option, wm, str_min );

			wstring wss = wstring(L"ss");
			wstring ws = wstring(L"s");
			option = ItsString::Replace( option, wss, str_ss );
			option = ItsString::Replace( option, ws, str_s );

			return option;			
		}

		const tm& TM( )
		{
			return this->m_tm;
		}

		void AddYears( int year )
		{
			this->m_tm.tm_year += year;			

			mktime( &this->m_tm );
		}

		void AddMonths( int month )
		{			
			this->m_tm.tm_mon += month;

			mktime( &this->m_tm );
		}

		void AddDays( int day )
		{
			this->m_tm.tm_mday += day;

			mktime( &this->m_tm );
		}

		void AddHours( int hour )
		{
			this->m_tm.tm_hour += hour;

			mktime( &this->m_tm );
		}

		void AddMinutes( int min )
		{
			this->m_tm.tm_min += min;

			mktime( &this->m_tm );
		}

		void AddSeconds( int sec )
		{
			this->m_tm.tm_sec += sec;

			mktime( &this->m_tm );
		}

		void Add( int year, int month, int day, int hour, int min, int sec ) 
		{
			this->m_tm.tm_year += year;
			this->m_tm.tm_mon += month;
			this->m_tm.tm_mday += day;
			this->m_tm.tm_hour += hour;
			this->m_tm.tm_min += min;
			this->m_tm.tm_sec += sec;

			mktime( &this->m_tm );
		}

		void SubtractYears( int year )
		{
			this->m_tm.tm_year -= year;			

			mktime( &this->m_tm );
		}

		void SubtractMonths( int month )
		{
			this->m_tm.tm_mon -= month;
		
			mktime( &this->m_tm );
		}

		void SubtractDays( int day )
		{
			this->m_tm.tm_mday -= day;

			mktime( &this->m_tm );
		}

		void SubtractHours( int hour )
		{
			this->m_tm.tm_hour -= hour;

			mktime( &this->m_tm );
		}

		void SubtractMinutes( int min )
		{
			this->m_tm.tm_min -= min;

			mktime( &this->m_tm );		
		}

		void SubtractSeconds( int sec )
		{
			this->m_tm.tm_sec -= sec;

			mktime( &this->m_tm );
		}

		void Subtract( int year, int month, int day, int hour, int min, int sec )
		{
			this->m_tm.tm_year -= year;
			this->m_tm.tm_mon -= month;
			this->m_tm.tm_mday -= day;
			this->m_tm.tm_hour -= hour;
			this->m_tm.tm_min -= min;
			this->m_tm.tm_sec -= sec;

			mktime( &this->m_tm );
		}

		int GetYear( )
		{
			return this->m_tm.tm_year + 1900;
		}

		int GetMonth( )
		{
			return this->m_tm.tm_mon + 1;
		}

		int GetDay( )
		{
			return this->m_tm.tm_mday;
		}

		int GetHour( )
		{
			return this->m_tm.tm_hour;
		}

		int GetMinute( )
		{
			return this->m_tm.tm_min;
		}

		int GetSecond( ) 
		{
			return this->m_tm.tm_sec;
		}

		ItsDateTime& operator=( const tm& timeDate )
		{
			this->m_tm = timeDate;
			return *this;
		}
	};

	//
	// ItsLogType
	//
	// (i): Log type for ItsLogItem
	//
	enum class ItsLogType
	{
		Information,
		Warning,
		Error,
		Other,
		Debug
	};

	//
	// ItsLogUtil
	//
	// (i): Convertion misc. methods.
	//
	struct ItsLogUtil
	{
		static constexpr const char* LogTypeToString(ItsLogType t)
		{
			switch (t)
			{
			case ItsLogType::Information:
				return "Information";
				break;
			case ItsLogType::Warning:
				return "Warning";
				break;
			case ItsLogType::Error:
				return "Error";
				break;
			case ItsLogType::Other:
				return "Other";
				break;
			case ItsLogType::Debug:
				return "Debug";
				break;
			default:
				break;
			}
			return "<UNKNOWN>";
		}
	};

	//
	// struct: ItsLogItem
	//
	// (i): Log item for ItsLog
	//
	struct ItsLogItem 
	{
		ItsLogType Type{ ItsLogType::Information };
		wstring	Description{L""};
		tm When{0};

		wstring ToString()
		{
			wstringstream ss;
			wstring nl1(L"\r\n");
			wstring nl2(L"\n");
			wstring s1(L":");
			wstring rep_nl(L" ");
			wstring rep_s(L";");

			auto description = ItsString::Replace(this->Description, nl1, rep_nl);
			description = ItsString::Replace(description, nl2, rep_nl);
			description = ItsString::Replace(description, s1, rep_s);
			ss << L"Type=" << ItsLogUtil::LogTypeToString(this->Type) << L" " << L"When=" << ItsDateTime(this->When).ToString(L"s") << L" " << L"Description=" << description;

			wstring retVal = ss.str();
			return retVal;
		}
	};

	//
	// struct: ItsLog
	//
	// (i): CUstom application event log.
	//
	struct ItsLog
	{
	private:
		vector<ItsLogItem> m_items;
		wstring m_sourceName;
		bool m_bLogToEventLog;

		enum struct EEVENTLOGTYPE : int
		{
			EET_SUCCESS = 0,
			EET_ERROR_TYPE = 1,
			EET_WARNING_TYPE = 2,
			EET_INFORMATION_TYPE = 3,
			EET_AUDIT_SUCCESS = 4,
			EET_AUDIT_FAILURE = 5
		};

		WORD ConvertEnumToType(EEVENTLOGTYPE type)
		{
			WORD wType(0);
			switch (type)
			{
			case EEVENTLOGTYPE::EET_SUCCESS:
				wType = EVENTLOG_SUCCESS;
				break;
			case EEVENTLOGTYPE::EET_ERROR_TYPE:
				wType = EVENTLOG_ERROR_TYPE;
				break;
			case EEVENTLOGTYPE::EET_WARNING_TYPE:
				wType = EVENTLOG_WARNING_TYPE;
				break;
			case EEVENTLOGTYPE::EET_INFORMATION_TYPE:
				wType = EVENTLOG_INFORMATION_TYPE;
				break;
			case EEVENTLOGTYPE::EET_AUDIT_SUCCESS:
				wType = EVENTLOG_AUDIT_SUCCESS;
				break;
			case EEVENTLOGTYPE::EET_AUDIT_FAILURE:
				wType = EVENTLOG_AUDIT_FAILURE;
				break;
			default:
				break;
			};

			return wType;
		}

		int ReportEvent( EEVENTLOGTYPE eeventlogtype, const wstring description)
		{
			if (this->m_sourceName.size() == 0 || description.size() == 0) {
				return -1;
			}

			HANDLE hEventLog = ::RegisterEventSource(NULL, this->m_sourceName.c_str());
			if (hEventLog == NULL) {
				return -1;
			}
			
			CComBSTR bstr(description.c_str());
			BOOL bStatus = ::ReportEvent(hEventLog, ConvertEnumToType(eeventlogtype), 0, 0, NULL, 1, 0, (LPCWSTR*)&bstr, NULL);
			if (!bStatus) {
				::DeregisterEventSource(hEventLog);
				return -1;
			}

			::DeregisterEventSource(hEventLog);

			return 0;
		}
	public:
		ItsLog(wstring sourceName, bool logToEventLog)
			:	m_sourceName(sourceName),
				m_bLogToEventLog(logToEventLog)
		{
		}
		~ItsLog()
		{
		}
		void LogInformation(wstring description)
		{
			ItsLogItem item;
			item.When = ItsDateTime::Now().TM();
			item.Description = description;
			item.Type = ItsLogType::Information;

			this->m_items.push_back(item);

			if (this->m_bLogToEventLog) {
				this->ReportEvent(ItsLog::EEVENTLOGTYPE::EET_INFORMATION_TYPE, item.ToString());
			}
		}

		void LogWarning(wstring description)
		{
			ItsLogItem item;
			item.When = ItsDateTime::Now().TM();
			item.Description = description;
			item.Type = ItsLogType::Warning;

			this->m_items.push_back(item);

			if (this->m_bLogToEventLog) {
				this->ReportEvent(ItsLog::EEVENTLOGTYPE::EET_WARNING_TYPE, item.ToString());
			}
		}

		void LogError(wstring description)
		{
			ItsLogItem item;
			item.When = ItsDateTime::Now().TM();
			item.Description = description;
			item.Type = ItsLogType::Error;

			this->m_items.push_back(item);

			if (this->m_bLogToEventLog) {
				this->ReportEvent(ItsLog::EEVENTLOGTYPE::EET_ERROR_TYPE, item.ToString());
			}
		}

		void LogOther(wstring description)
		{
			ItsLogItem item;
			item.When = ItsDateTime::Now().TM();
			item.Description = description;
			item.Type = ItsLogType::Other;

			this->m_items.push_back(item);

			if (this->m_bLogToEventLog) {
				this->ReportEvent(ItsLog::EEVENTLOGTYPE::EET_INFORMATION_TYPE, item.ToString());
			}
		}

		void LogDebug(wstring description)
		{
			ItsLogItem item;
			item.When = ItsDateTime::Now().TM();
			item.Description = description;
			item.Type = ItsLogType::Debug;

			this->m_items.push_back(item);

			if (this->m_bLogToEventLog) {
				this->ReportEvent(ItsLog::EEVENTLOGTYPE::EET_INFORMATION_TYPE, item.ToString());
			}
		}

		const vector<ItsLogItem>& GetItems() 
		{
			return this->m_items;
		}

		size_t Count()
		{
			return this->m_items.size();
		}

		void Clear()
		{
			this->m_items.clear();
		}

		wstring ToString()
		{
			wstringstream ss;
			for (auto i : this->m_items)
			{
				ss << i.ToString() << endl;
			}

			wstring retVal = ss.str();
			return retVal;
		}

		wstring ToString(uint32_t tailN)
		{
			wstringstream ss;
			if (this->m_items.size() > tailN) {
				auto ptr = this->m_items.end();
				ptr -= tailN;

				do
				{
					ss << (*ptr).ToString() << endl;
				} while (++ptr != this->m_items.end());
			}
			else {
				for (auto i : this->m_items)
				{
					ss << i.ToString() << endl;
				}
			}

			wstring retVal = ss.str();
			return retVal;
		}
	};

	//
	// enum: ItsCreateIDOptions
	//
	// (i): ItsID::CreateID options.
	//
	enum class ItsCreateIDOptions
	{
		LowerCase,
		UpperCase,
		LowerAndUpperCase
	};

	//
	// struct: ItsID
	// 
	// (i): Generates unique ID's.
	//
	struct ItsID
	{
	public:
		static wstring CreateID(size_t count, ItsCreateIDOptions options, bool includeNumbers)
		{
			if (count <= 0)
			{
				count = 16;
			}

			wstring dataMin = L"abcdefghijklmnopqrstuvwxyz";
			wstring dataMaj = L"ABCDEFGHIJKLMNOPQRSTUVWXYZ";
			wstring dataMinMaj = L"aAbBcCdDeEfFgGhHiIjJkKlLmMnNoOpPqQrRsStTuUvVwWxXyYzZ";
			wstring dataMinNum = L"a0b1c2d3e4f5g6h7i8j9k0l1m2n3o4p5q6r7s8t9u0v1w2x3y4z5";
			wstring dataMajNum = L"A0B1C2D3E4F5G6G7I8J9K0L1M2N3O4P5Q6R7S8T9U0V1W2X3Y4Z5";
			wstring dataMinMajNum = L"aA0bB1cC2dD3eE4fF5gG6hH7iI8jJ9kK0lL1mM2nN3oO4pP5qQ6rR7sS8tT9uU0vV1wW2xX3yY4zZ5";

			wstring data = dataMin;
			if (options == ItsCreateIDOptions::LowerAndUpperCase && includeNumbers)
			{
				data = dataMinMajNum;
			}
			if (options == ItsCreateIDOptions::LowerAndUpperCase && !includeNumbers)
			{
				data = dataMinMaj;
			}
			else if (options == ItsCreateIDOptions::LowerCase && includeNumbers)
			{
				data = dataMinNum;
			}
			else if (options == ItsCreateIDOptions::LowerCase && !includeNumbers)
			{
				data = dataMin;
			}
			else if (options == ItsCreateIDOptions::UpperCase && includeNumbers)
			{
				data = dataMajNum;
			}
			else if (options == ItsCreateIDOptions::UpperCase && !includeNumbers)
			{
				data = dataMaj;
			}

			wstringstream id;
			int i = 0;
			do
			{
				size_t j = ItsRandom<size_t>(0, data.size() - 1);
				id << data[j];

			} while (++i < count);
			
			wstring retVal = id.str();
			return retVal;
		}
	};
}// namespace ItSoftware
