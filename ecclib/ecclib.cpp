// ecclib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ecclib.h"

namespace EccLib
{
	void Functions::DummyEncode(unsigned char data[20], unsigned char encoded[20])
	{
		for (int i = 0; i < 20; i++)
		{
			encoded[i] = data[i];
		}
	}

	void Functions::DummyDecode(unsigned char data[20], unsigned char decoded[20])
	{
		for (int i = 0; i < 20; i++)
		{
			decoded[i] = data[i];
		}
	}

	BCH::BCH()
	{
		this->_generatormatrix = &BinaryMatrix::Load(R"(C:\Users\Wesley\dev\ecc-lib\pyGF\4095_4047_matrix)");
	}

	// TODO: Use redundant only generating matrix and return only redundant data
	void BCH::Encode(unsigned char data[506], unsigned char encoded[512])
	{
		unsigned char* enc = this->_generatormatrix->MultiplyVector(data);;
		for (int i = 0; i < 512; i++)
		{
			encoded[i] = enc[i];
		}
	}

	void BCH::Decode(unsigned char data[512], unsigned char decoded[506])
	{

	}
}