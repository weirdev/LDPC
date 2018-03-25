// https://docs.microsoft.com/en-us/cpp/build/walkthrough-creating-and-using-a-dynamic-link-library-cpp

#include "stdafx.h"
#include "ecc-lib-testing.h"

int main()
{
	test_BCH_Encode();
	test_BCH_Decode();
	int s;
	std::cin >> s;
    return 0;
}

void test_BCH_Encode() {
	EccLib::BCH bch = EccLib::BCH(R"(C:\Users\Wesley\dev\ecc-lib\pyGF\4095_4047_matrix)",
		R"(C:\Users\Wesley\dev\ecc-lib\pyGF\4095_4047_check_matrix)",
		12, 4);
	int input_byte_len = (int)4047 / 8;
	if (input_byte_len % 8 != 0)
	{
		input_byte_len++;
	}

	std::vector<unsigned char> ibvector = randomdata(input_byte_len);
	unsigned char* input_bytes = &ibvector[0];
	auto encoded = bch.Encode(input_bytes);
	std::cout << "BCH encode sucessful" << std::endl;
}

void test_BCH_Decode(bool small)
{
	EccLib::BCH* bch;
	int input_bit_len;
	int encoded_bit_len;
	int t;
	int mbytes;
	if (small)
	{
		bch = new EccLib::BCH(R"(C:\Users\Wesley\dev\ecc-lib\pyGF\63_45_matrix)",
			R"(C:\Users\Wesley\dev\ecc-lib\pyGF\63_45_check_matrix)",
			6, 3);
		input_bit_len = 45;
		encoded_bit_len = 63;
		t = 3;
		mbytes = 1;
	}
	else
	{
		bch = new EccLib::BCH(R"(C:\Users\Wesley\dev\ecc-lib\pyGF\4095_4047_matrix)",
			R"(C:\Users\Wesley\dev\ecc-lib\pyGF\4095_4047_check_matrix)",
			12, 4);
		input_bit_len = 4047;
		encoded_bit_len = 4095;
		t = 4;
		mbytes = 2;
	}
	int input_byte_len = (int)input_bit_len / 8;
	if (input_bit_len % 8 != 0)
	{
		input_byte_len++;
	}
	int encoded_byte_len = (int)encoded_bit_len / 8;
	if (encoded_bit_len % 8 != 0)
	{
		encoded_byte_len++;
	}
	std::vector<unsigned char> ibvector = randomdata(input_byte_len);
	unsigned char* input_bytes = &ibvector[0];
	unsigned char* encoded = bch->Encode(input_bytes);
	unsigned char** syndrome = bch->ComputeSyndrome(encoded);
	if (!bch->CheckSyndrome(syndrome))
	{
		std::cout << "nonzero syndrome" << std::endl;
		for (int s = 0; s < 2 * t; s++)
		{
			for (int e = 0; e < mbytes; e++)
			{
				std::cout << std::hex << (int)syndrome[s][e];
			}
			std::cout << std::endl;
		}
	}
	else
	{
		std::cout << "zero syndrome" << std::endl;
		std::vector<unsigned char*> elp = bch->ComputErrorLocationPolynomial(syndrome);
		std::cout << "no error ELP len " << elp.size() << std::endl;
	}
	ibvector = randomdata(input_byte_len);
	input_bytes = &ibvector[0];
	encoded = bch->Encode(input_bytes);
	unsigned char* errored = standardnoise(encoded, encoded_byte_len);
	syndrome = bch->ComputeSyndrome(errored);
	if (bch->CheckSyndrome(syndrome))
	{
		std::cout << "zero syndrome on errored vector" << std::endl;
		for (int s = 0; s < 2 * t; s++)
		{
			for (int e = 0; e < mbytes; e++)
			{
				std::cout << std::hex << (int)syndrome[s][e];
			}
			std::cout << std::endl;
		}
	}
	else
	{
		std::cout << "nonzero syndrome errored vector" << std::endl;
		std::vector<unsigned char*> elp = bch->ComputErrorLocationPolynomial(syndrome);
		std::cout << "Error ELP len " << elp.size() << std::endl;
		std::cout << bch->GFPolynomialToStr(elp);
		std::cout << bch->GFPolynomialToStr(elp).size();
	}
}

void testdummyencode() {
	unsigned char* data = &randomdata(20)[0];
	unsigned char encodeddata[20];
	EccLib::Functions::DummyEncode(data, encodeddata);
	comparearrays(data, encodeddata, 20);
	unsigned char decodeddata[20];
	EccLib::Functions::DummyDecode(encodeddata, decodeddata);
	comparearrays(data, decodeddata, 20);
}

std::vector<unsigned char> randomdata(int len) 
{
	std::random_device rd;   // non-deterministic generator  
	std::mt19937 gen(rd());  // to seed mersenne twister.
	std::uniform_int_distribution<> dist(0,255);
	std::vector<unsigned char> data;
	data.reserve(len);
	for (int i = 0; i < len; ++i)
	{
		data.push_back(dist(gen));
	}
	return data;
}

// Applies a known error pattern (flips first bit) to a vector (previously encoded)
unsigned char* standardnoise(unsigned char* v, int size)
{
	unsigned char* witherror = new unsigned char[size];
	for (int i = 0; i < size; i++)
	{
		witherror[i] = v[i];
	}
	witherror[0] ^= 0x03;
	return witherror;
}

void comparearrays(unsigned char* v1, unsigned char* v2, unsigned int size)
{
	for (unsigned int i=0; i < size; ++i)
	{
		if (v1[i] != v2[i])
		{
			std::cout << "diffval" << std::endl;
			return;
		}
	}
	std::cout << "success" << std::endl;
}

void testload_matrix()
{
	EccLib::BinaryMatrix bm = *EccLib::BinaryMatrix::Load(R"(C:\Users\Wesley\dev\ecc-lib\pyGF\63_45_matrix)");
	for (int col = 0; col < 10; col++) {
		std::cout << "col " << col << std::endl;
		int row = 0;
		while (!bm.GetElement(row, col)) {
			row++;
		}
		std::cout << "True row " << row << std::endl;
	}
}

void test_matrix_encode()
{
	EccLib::BinaryMatrix bm = *EccLib::BinaryMatrix::Load(R"(C:\Users\Wesley\dev\ecc-lib\pyGF\63_45_matrix)");
	unsigned char* data = new unsigned char[6];
	for (int i = 0; i < 6; i++)
	{
		data[i] = (unsigned char)(i+1);
	}
	unsigned char* encoded = bm.MultiplyVector(data);
	for (int i = 0; i < 8; i++)
	{
		std::cout << (int)encoded[i] << std::endl;
	}
}

void testload_gfmatrix()
{
	EccLib::GFMatrix gfm = *EccLib::GFMatrix::Load(R"(C:\Users\Wesley\dev\ecc-lib\pyGF\63_45_check_matrix)");
	for (int row = 0; row < gfm.rows; row++) {
		std::cout << "row " << row << " col " << 1 << ": 0x";
		int ebytes = (gfm.m + ((8 - gfm.m % 8) % 8)) / 8;
		unsigned char* element = gfm.GetElement(row, 1);
		for (int e = 0; e < ebytes; e++)
		{
			std::cout << std::hex << (int)element[e];
		}
		std::cout << std::endl;
	}
}

