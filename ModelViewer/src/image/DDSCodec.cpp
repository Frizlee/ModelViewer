#include "DDSCodec.hpp"
#include "Image.hpp"
#include "S3TCFlip.hpp"

static const uint32_t DDS_MAGIC = 0x20534444; // (DDS )
static const uint32_t DDS_BC1 = 0x31545844; // (DXT1)
static const uint32_t DDS_BC2 = 0x33545844; // (DXT3)
static const uint32_t DDS_BC3 = 0x35545844; // (DXT5)
static const uint32_t DDS_BC4U = 0x55344342; // (BC4U)
static const uint32_t DDS_BC4S = 0x53344342; // (BC4S)
static const uint32_t DDS_BC5U = 0x32495441; // (ATI2)
static const uint32_t DDS_BC5S = 0x53354342; // (BC5S)
static const uint32_t DDS_DX10 = 0x30315844; // (DX10)

enum DDSFlags
{
	DDPF_ALPHAPIXELS	= 0x1,
	DDPF_ALPHA			= 0x2,
	DDPF_FOURCC			= 0x4,
	DDPF_RGB			= 0x40,
	DDPF_YUV			= 0x200,
	DDPF_LUMINANCE		= 0x20000,

	DDSD_CAPS			= 0x1,
	DDSD_HEIGHT			= 0x2,
	DDSD_WIDTH			= 0x4,
	DDSD_PITCH			= 0x8,
	DDSD_PIXELFORMAT	= 0x1000,
	DDSD_MIPMAPCOUNT	= 0x20000,
	DDSD_LINEARSIZE		= 0x80000,
	DDSD_DEPTH			= 0x800000,

	DDSCAPS_COMPLEX		= 0x8, 
	DDSCAPS_MIPMAP		= 0x400000,
	DDSCAPS_TEXTURE		= 0x1000,

	DDSCAPS2_CUBEMAP			= 0x200,
	DDSCAPS2_CUBEMAP_POSITIVEX	= 0x400,
	DDSCAPS2_CUBEMAP_NEGATIVEX	= 0x800,
	DDSCAPS2_CUBEMAP_POSITIVEY	= 0x1000,
	DDSCAPS2_CUBEMAP_NEGATIVEY	= 0x2000,
	DDSCAPS2_CUBEMAP_POSITIVEZ	= 0x4000,
	DDSCAPS2_CUBEMAP_NEGATIVEZ	= 0x8000,
	DDSCAPS2_VOLUME				= 0x200000,
};

struct RGBAMask
{
	uint32_t r;
	uint32_t g;
	uint32_t b;
	uint32_t a;
	
	RGBAMask(uint32_t r, uint32_t g, uint32_t b, uint32_t a) 
		: r(r), g(g), b(b), a(a) {}
	RGBAMask(const uint32_t *bytes)
	{
		r = bytes[0];
		g = bytes[1];
		b = bytes[2];
		a = bytes[3];
	}
	
	bool operator==(const RGBAMask &m)
	{
		return (r == m.r && g == m.g && b == m.b && a == m.a);
	}
};

static const RGBAMask BGR8mask	  = { 0x00ff0000, 0x0000ff00, 0x000000ff, 0x00000000 };
static const RGBAMask BGRA8mask   = { 0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000 };
static const RGBAMask B2G3R3mask  = { 0x000000e0, 0x0000001c, 0x00000003, 0x00000000 };
static const RGBAMask B5G6R5mask  = { 0x0000f800, 0x000007e0, 0x0000001f, 0x00000000 };
static const RGBAMask BGRA4mask   = { 0x00000f00, 0x000000f0, 0x0000000f, 0x0000f000 };
static const RGBAMask BGR5A1mask  = { 0x00007c00, 0x000003e0, 0x0000001f, 0x00008000 };
static const RGBAMask BGR10A2mask = { 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000 };

#pragma pack(push, 1)

struct DDSPixelFormat
{
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwFourCC;
	uint32_t dwRGBBitCount;
	uint32_t dwRBitMask;
	uint32_t dwGBitMask;
	uint32_t dwBBitMask;
	uint32_t dwABitMask;
};

struct DDSHeader
{
	uint32_t dwSize;
	uint32_t dwFlags;
	uint32_t dwHeight;
	uint32_t dwWidth;
	uint32_t dwPitchOrLinearSize;
	uint32_t dwDepth;
	uint32_t dwMipmapCount;
	uint32_t dwReserved1[11];
	DDSPixelFormat pixelFormat;
	uint32_t dwCaps;
	uint32_t dwCaps2;
	uint32_t dwCaps3;
	uint32_t dwCaps4;
	uint32_t dwReserved2;
};

#pragma pack(pop)

void CopyUncompressed(uint8_t *in, std::vector<uint8_t> *out, 
	RGBAMask mask, uint32_t bitsPerPixel, uint32_t width, uint32_t height);
void CopyCompressed(uint8_t *in, std::vector<uint8_t> *out,
	ColorFormat fmt, uint32_t blockWidth, uint32_t blockHeight, size_t blockSize);
void Flip(uint8_t *in, ColorFormat fmt);
uint32_t Popcount(uint32_t i);

DDSCodec::DDSCodec()
{
}

DDSCodec::~DDSCodec()
{
}

uint8_t DDSCodec::getMipmapLevels(std::vector<uint8_t> &in)
{
	if (*reinterpret_cast<uint32_t*>(&in[0]) != DDS_MAGIC)
	{
		// TODO: Error handling.
		return 0;
	}

	DDSHeader *header = reinterpret_cast<DDSHeader*>(&in[sizeof(DDS_MAGIC)]);

	if (header->dwFlags & DDSD_MIPMAPCOUNT)
		return header->dwMipmapCount;
	else
		return 0;
}

bool DDSCodec::shouldBeFlippedVerticaly()
{
	return false;
}

void DDSCodec::decode(std::vector<uint8_t> &in, std::vector<uint8_t> *out, 
	unsigned int *width, unsigned int *height, ColorFormat *format, uint8_t level)
{
	if (*reinterpret_cast<uint32_t*>(&in[0]) != DDS_MAGIC)
	{
		// TODO: Error handling.
		return;
	}

	DDSHeader *header = reinterpret_cast<DDSHeader*>(&in[sizeof(DDS_MAGIC)]);

	if (!((header->dwFlags & DDSD_CAPS) &&
		(header->dwFlags & DDSD_HEIGHT) &&
		(header->dwFlags & DDSD_WIDTH) &&
		(header->dwFlags & DDSD_PIXELFORMAT)))
	{
		// TODO: Error handling.
		return;
	}

	DDSPixelFormat *pixelFormat = &header->pixelFormat;

	if (pixelFormat->dwFlags & DDPF_FOURCC)
	{
		// Compressed.
		ColorFormat fmt;
		size_t blockSize;

		switch (pixelFormat->dwFourCC)
		{
		case DDS_BC1:
			fmt = ColorFormat::BC1_RGB;
			blockSize = sizeof(BC1Block);
			break;

		case DDS_BC2:
			fmt = ColorFormat::BC2_RGBA;
			blockSize = sizeof(BC2Block);
			break;

		case DDS_BC3:
			fmt = ColorFormat::BC3_RGBA;
			blockSize = sizeof(BC3Block);
			break;

		case DDS_BC4S:
			fmt = ColorFormat::BC4_SIGNED_R;
			blockSize = sizeof(BC4Block);
			break;

		case DDS_BC4U:
			fmt = ColorFormat::BC4_R;
			blockSize = sizeof(BC4Block);
			break;

		case DDS_BC5S:
			fmt = ColorFormat::BC5_SIGNED_RG;
			blockSize = sizeof(BC5Block);
			break;

		case DDS_BC5U:
			fmt = ColorFormat::BC5_RG;
			blockSize = sizeof(BC5Block);
			break;

		default:
			// TODO: Error handling.
			return;
		}

		uint32_t blockWidth = (header->dwWidth + 3) / 4;
		uint32_t blockHeight = (header->dwHeight + 3) / 4;

		out->resize(blockWidth * blockHeight * blockSize);
		CopyCompressed(&in.at(sizeof(DDS_MAGIC) + sizeof(DDSHeader)), out, 
			fmt, blockWidth, blockHeight, blockSize);
		*width = header->dwWidth;
		*height = header->dwHeight;
		*format = fmt;
	}
	else if (pixelFormat->dwFlags & DDPF_RGB)
	{
		// Uncompressed.
		bool hasAlpha = pixelFormat->dwFlags & DDPF_ALPHAPIXELS;
		RGBAMask mask(&pixelFormat->dwRBitMask);
		ColorFormat fmt;
		
		if (hasAlpha == false)
			mask.a = 0;

		if (mask == BGR8mask)
			fmt = ColorFormat::RGB8;
		else if (mask == BGRA8mask)
			fmt = ColorFormat::RGBA8;
		else if (mask == B2G3R3mask)
			fmt = ColorFormat::R3G3B2;
		else if (mask == B5G6R5mask)
			fmt = ColorFormat::R5G6B5;
		else if (mask == BGRA4mask)
			fmt = ColorFormat::RGBA4;
		else if (mask == BGR5A1mask)
			fmt = ColorFormat::RGB5A1;
		else if (mask == BGR10A2mask)
			fmt = ColorFormat::RGB10A2;
		else
		{
			// TODO: Error handling.
			return;
		}

		out->resize(header->dwWidth * header->dwHeight * pixelFormat->dwRGBBitCount / 8);
		CopyUncompressed(&in.at(sizeof(DDS_MAGIC) + sizeof(DDSHeader)), out, 
			mask, pixelFormat->dwRGBBitCount, header->dwWidth, header->dwHeight);
		*width = header->dwWidth;
		*height = header->dwHeight;
		*format = fmt;
	}
}

void CopyUncompressed(uint8_t *in, std::vector<uint8_t> *out, 
	RGBAMask mask, uint32_t bitsPerPixel, uint32_t width, uint32_t height)
{
	uint32_t bytesPerPixel = bitsPerPixel / 8;
	union
	{
		uint8_t pixel[4];
		uint32_t pixel32;
	};
	pixel[0] = 0;
	pixel[1] = 0;
	pixel[2] = 0;
	pixel[3] = 0;
	
	uint32_t comp[4]; // 0 - red, 1 - green, 2 - blue, 3 - red;
	int8_t shifts[3]; // 0 - red, 1 - green, 2 - blue;

	shifts[0] = Popcount(mask.g | mask.b);
	shifts[1] = Popcount(mask.b) - Popcount(mask.r);
	shifts[2] = Popcount(mask.r | mask.g);


	for (uint32_t i = 0; i < height; ++i)
	{
		uint32_t row1 = i * width * bytesPerPixel;
		uint32_t row2 = (height - i - 1) * width * bytesPerPixel;

		for (uint32_t j = 0; j < width * bytesPerPixel; j += bytesPerPixel)
		{
			for (uint8_t k = 0; k < bytesPerPixel; ++k)
				pixel[k] = in[row2 + j + k];

			comp[0] = pixel32 & mask.r;
			comp[1] = pixel32 & mask.g;
			comp[2] = pixel32 & mask.b;
			comp[3] = pixel32 & mask.a;
			
			pixel32 = comp[0] >> shifts[0] |
				comp[2] << shifts[2] |
				comp[3];

			if (shifts[1] > 0)
				pixel32 |= comp[1] >> shifts[1];
			else if (shifts[1] < 0)
				pixel32 |= comp[1] << -shifts[1];
			else
				pixel32 |= comp[1];
			
			memcpy(&out->at(row1 + j), pixel, bytesPerPixel);
		}
	}
}

void CopyCompressed(uint8_t *in, std::vector<uint8_t> *out,
	ColorFormat fmt, uint32_t blockWidth, uint32_t blockHeight, size_t blockSize)
{
	uint8_t *block;

	for (uint32_t i = 0; i < blockHeight; ++i)
	{
		uint32_t row1 = i * blockWidth * blockSize;
		uint32_t row2 = (blockHeight - i - 1) * blockWidth * blockSize;

		for (uint32_t j = 0; j < blockWidth * blockSize; j += blockSize)
		{
			block = &in[row1 + j];
			Flip(block, fmt);
			memcpy(&out->at(row2 + j), block, blockSize);
		}
	}
}

void Flip(uint8_t *in, ColorFormat fmt)
{
	switch (fmt)
	{
	case ColorFormat::BC1_RGB:
		reinterpret_cast<BC1Block*>(in)->flipVerticaly();
		break;

	case ColorFormat::BC2_RGBA:
		reinterpret_cast<BC2Block*>(in)->flipVerticaly();
		break;

	case ColorFormat::BC3_RGBA:
		reinterpret_cast<BC3Block*>(in)->flipVerticaly();
		break;
		
	case ColorFormat::BC4_SIGNED_R:
	case ColorFormat::BC4_R:
		reinterpret_cast<BC1Block*>(in)->flipVerticaly();
		break;

	case ColorFormat::BC5_SIGNED_RG:
	case ColorFormat::BC5_RG:
		reinterpret_cast<BC5Block*>(in)->flipVerticaly();
		break;
	}
}

uint32_t Popcount(uint32_t i)
{
	i = i - ((i >> 1) & 0x55555555);
	i = (i & 0x33333333) + ((i >> 2) & 0x33333333);
	return (((i + (i >> 4)) & 0x0F0F0F0F) * 0x01010101) >> 24;
}


