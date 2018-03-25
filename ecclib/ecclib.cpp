// ecclib.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "ecclib.h"
#include "BinaryMatrix.h"
#include "GFMatrix.h"
#include "GaloisField.h"

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
		this->m_bytes = m / 8;
		if (m % 8 != 0)
		{
			this->m_bytes++;
		}
		this->_gf = new GaloisField(this->_paritycheckmatrix->primitive_polynomial, m);
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
	std::vector<unsigned char*> BCH::ComputErrorLocationPolynomial(unsigned char** syndrome)
	{
		std::vector<std::vector<unsigned char*>> errorlocpolys;
		errorlocpolys.resize(this->t + 2);
		std::vector<unsigned char*> discreps;
		discreps.resize(this->t + 1);
		std::vector<int> stepdegreediff;
		stepdegreediff.resize(this->t + 2);

		// NOTE: For below k=-1/2 => 0 and all other k => k + 1 (k literature => k array indices)
		// k=-1/2 partial error location polynomial = 1
		errorlocpolys[0].push_back(new unsigned char[this->m_bytes]);
		errorlocpolys[0][0][0] = 1;
		for (int e = 1; e < this->m_bytes; e++)
		{
			errorlocpolys[0][0][e] = 0;
		}
		// k=-1/2 discrepancy = 1
		discreps[0] = new unsigned char[this->m_bytes];
		discreps[0][0] = 1;
		for (int e = 1; e < this->m_bytes; e++)
		{
			discreps[0][e] = 0;
		}
		// k=-1/2 step/degree difference = -1
		stepdegreediff[0] = -1;

		// k=0 partial ELP = 1 (same as k=-1/2)
		errorlocpolys[1] = errorlocpolys[0];

		// k=0 discrepancy = S[0]
		discreps[1] = syndrome[0];
		stepdegreediff[1] = 0;

		// Index of max step/degree difference where the discrepency != 0
		int maxstepdegdiffloc = 0;
		
		for (int k = 1; k < this->t+1; k++)
		{
			discreps[k] = syndrome[2 * (k - 1)];
			for (int d = 1; d < errorlocpolys[k].size(); d++)
			{
				discreps[k] = this->_gf->MultiplyGFElements(errorlocpolys[k][d], syndrome[2 * (k - 1) - d]);
			}
			if (this->_gf->GFElementsEqual(discreps[k], this->_gf->GF[0])) // if discreps[k] == 0 (GF)
			{
				errorlocpolys[k + 1] = errorlocpolys[k];
				stepdegreediff[k + 1] = stepdegreediff[k] + 2;
			}
			else 
			{
				int maxstepdegdiff = stepdegreediff[maxstepdegdiffloc];
				for (int i = maxstepdegdiffloc; i < k; i++)
				{
					if (stepdegreediff[i] > maxstepdegdiff)
					{
						if (!this->_gf->GFElementsEqual(discreps[i], this->_gf->GF[0])) // discreps[i] != 0 (GF)
						{
							maxstepdegdiff = stepdegreediff[i];
							maxstepdegdiffloc = i;
						}
					}
				}

				unsigned char* product = new unsigned char[this->m_bytes]; 
				product = this->_gf->MultiplyGFElements(discreps[k], this->_gf->InvertGFElement(discreps[maxstepdegdiffloc]));

				std::vector<unsigned char*> correctionterm = std::vector<unsigned char*>();
				correctionterm.reserve(errorlocpolys[maxstepdegdiffloc].size());
				int correctionshift = 2 * (k - maxstepdegdiffloc);
				for (int i = 0; i < correctionshift; i++)
				{
					correctionterm.push_back(this->_gf->GF[0]);
				}
				for (int i = 0; i < errorlocpolys[maxstepdegdiffloc].size(); i++)
				{
					correctionterm.push_back(this->_gf->MultiplyGFElements(errorlocpolys[maxstepdegdiffloc][i], product));
				}
				std::cout << GFPolynomialToStr(correctionterm);
				errorlocpolys[k + 1] = SumGFPolynomials(errorlocpolys[k], correctionterm);
				// Find degree of new poly
				int degree = 0;
				for (int i = errorlocpolys[k + 1].size() - 1; i >= 0; i--)
				{
					if (!this->_gf->GFElementsEqual(errorlocpolys[k + 1][i], this->_gf->GF[0]))
					{
						degree = i;
					}
				}
				stepdegreediff[k + 1] = degree;
			}
		}
		return errorlocpolys[this->t + 1];
	}

	ECCLIB_API std::string BCH::GFPolynomialToStr(std::vector<unsigned char*> p)
	{
		std::string s = "";
		for (int i = 0; i < p.size(); i++)
		{
			s += this->_gf->GFElementToStr(p[i]);
			s += "\n";
		}
		return s;
	}

	std::vector<unsigned char*> BCH::SumGFPolynomials(std::vector<unsigned char*> p1, std::vector<unsigned char*> p2)
	{
		std::vector<unsigned char*> larger;
		std::vector<unsigned char*> smaller;
		if (p1.size() > p2.size())
		{
			larger = p1;
			smaller = p2;
		}
		else
		{
			larger = p2;
			smaller = p1;
		}
		std::vector<unsigned char*> sum;
		int i;
		for (i = 0; i < smaller.size(); i++)
		{
			unsigned char* termsum = new unsigned char[this->m_bytes];
			for (int e = 0; e < this->m_bytes; e++)
			{
				termsum[e] = larger[i][e] ^ smaller[i][e];
			}
			sum.push_back(termsum);
		}
		for (; i < larger.size(); i++)
		{
			sum.push_back(larger[i]);
		}
		return sum;
	}
}