#pragma once

#ifdef ECCLIB_EXPORTS
#define ECCLIB_API __declspec(dllexport)
#else
#define ECCLIB_API __declspec(dllimport)
#endif

#ifndef ECCLIB_H
#define ECCLIB_H

namespace EccLib
{
	class BinaryMatrix;
	class GFMatrix;

	class Functions
	{
	public:
		static ECCLIB_API void DummyEncode(unsigned char data[20], unsigned char encoded[20]);
		static ECCLIB_API void DummyDecode(unsigned char data[20], unsigned char decoded[20]);
	};

	class BCH
	{
	public:
		ECCLIB_API BCH();
		ECCLIB_API void Encode(unsigned char data[506], unsigned char encoded[512]);
		ECCLIB_API void Decode(unsigned char data[512], unsigned char decoded[506]);
	private:
		BinaryMatrix* _generatormatrix;
	};
}


#endif