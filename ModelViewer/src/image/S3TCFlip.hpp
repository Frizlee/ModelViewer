#ifndef S3TC_FLIP_HPP
#define S3TC_FLIP_HPP
#include "../Prerequisites.hpp"

#pragma pack(push, 1)

struct BC1Block
{
	uint16_t c[2];
	uint8_t dcba, hgfe, lkji, ponm;

	void flipVerticaly();
};

struct BC4Block
{
	uint8_t c[2];
	uint8_t ahgfedcba[3];
	uint8_t aponmlkji[3];

	void flipVerticaly();
};

struct BC2Block
{
	uint16_t adcab, ahgfe, alkji, aponm;
	BC1Block c;

	void flipVerticaly();
};

struct BC3Block
{
	BC4Block a;
	BC1Block c;

	void flipVerticaly();
};

struct BC5Block
{
	BC4Block r;
	BC4Block a;

	void flipVerticaly();
};

#pragma pack(pop)

#endif // S3TC_FLIP_HPP

