///////////////////////////////////////////////////////////////////
//: Title        : itsoftware-cli.cpp
//: Product:     : Cpp.Include.Windows
//: Date         : 2020-05-01
//: Author       : "Kjetil Kristoffer Solberg" <post@ikjetil.no>
//: Version      : 1.0.0.0
//: Descriptions : Implementation of Cpp.Include.Windows.
#pragma once
//
// #include
//
#include <msclr\marshal.h>
#include <msclr\marshal_cppstd.h>

//
// namespace
//
namespace ItSoftware::CLI
{
	using namespace System;

	//
	// class: ItsCli
	//
	class ItsCli
	{
	public:
		static wstring ToString(String^ str)
		{
			return msclr::interop::marshal_as<std::wstring>(str);
		}
	};
}// namespace ItSoftware::CLI