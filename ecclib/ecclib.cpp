// ecclib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ecclib.h"
#include "BinaryMatrix.h"
#include "GFMatrix.h"

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

	BCH::BCH(std::string generatormatrixfile, std::string paritycheckmatrixfile, int m, int t)
	{
		this->_generatormatrix = BinaryMatrix::Load(generatormatrixfile);
		this->_paritycheckmatrix = GFMatrix::Load(paritycheckmatrixfile);
		this->t = t;
		this->m = m;
	}

	// TODO: Use redundant only generating matrix and return only redundant data
	unsigned char* BCH::Encode(unsigned char* data)
	{
		return this->_generatormatrix->MultiplyVector(data);
	}

	unsigned char* BCH::Decode(unsigned char* data)
	{
		return nullptr;
	}

	unsigned char** BCH::ComputeSyndrome(unsigned char* data)
	{
		return this->_paritycheckmatrix->MultiplyVector(data);
	}

	bool BCH::CheckSyndrome(unsigned char** syndrome) 
	{
		for (int i = 0; i < this->t * 2; i++)
		{
			if (!GFMatrix::ElementZero(syndrome[i], this->m))
			{
				return false;
			}
		}
		return true;
	}
	
	// Implements the Berlekamp-Massey algorithm
	ECCLIB_API unsigned char* BCH::ComputErrorLocationPolynomial(unsigned char** syndrome)
	{
		return nullptr;
	}
}