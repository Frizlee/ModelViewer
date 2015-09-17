#include "S3TCFlip.hpp"

void BC1Block::flipVerticaly()
{
	uint8_t temp8;

	temp8 = ponm;
	ponm = dcba;
	dcba = temp8;

	temp8 = lkji;
	lkji = hgfe;
	hgfe = temp8;
}

void BC4Block::flipVerticaly()
{
	uint32_t temp32;
	uint32_t *as32[2];

	as32[0] = reinterpret_cast<uint32_t*>(ahgfedcba);
	as32[1] = reinterpret_cast<uint32_t*>(aponmlkji);

	temp32 = *as32[0] & ((1 << 12) - 1);
	*as32[0] &= ~((1 << 12) - 1);
	*as32[0] |= (*as32[1] & (((1 << 12) - 1) << 12)) >> 12;
	*as32[1] &= ~(((1 << 12) - 1) << 12);
	*as32[1] |= temp32 << 12;

	temp32 = *as32[0] & (((1 << 12) - 1) << 12);
	*as32[0] &= ~(((1 << 12) - 1) << 12);
	*as32[0] |= (*as32[1] & ((1 << 12) - 1)) << 12;
	*as32[1] &= ~((1 << 12) - 1);
	*as32[1] |= temp32 >> 12;
}

void BC2Block::flipVerticaly()
{
	uint16_t temp16;

	temp16 = aponm;
	aponm = adcab;
	adcab = temp16;

	temp16 = alkji;
	alkji = ahgfe;
	ahgfe = temp16;

	c.flipVerticaly();
}

void BC3Block::flipVerticaly()
{
	a.flipVerticaly();
	c.flipVerticaly();
}

void BC5Block::flipVerticaly()
{
	r.flipVerticaly();
	a.flipVerticaly();
}
