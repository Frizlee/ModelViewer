#ifndef DDS_CODEC_HPP
#define DDS_CODEC_HPP
#include "../Prerequisites.hpp"
#include "ImageCodec.hpp"

class DDSCodec : public ImageCodec
{
public:
	DDSCodec();
	~DDSCodec();

	uint8_t getMipmapLevels(std::vector<uint8_t> &in);
	bool shouldBeFlippedVerticaly();
	//bool shouldBeFlippedHorizontaly();

	void decode(std::vector<uint8_t> &in,
		std::vector<uint8_t> *out, unsigned int *width,
		unsigned int *height, ColorFormat *format, uint8_t level);
	//void encode();
	
};				 
				 
#endif // DDS_CODEC_HPP

