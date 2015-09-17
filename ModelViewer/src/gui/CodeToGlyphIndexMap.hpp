#ifndef CODE_TO_GLYPH_INDEX_MAP
#define CODE_TO_GLYPH_INDEX_MAP
#include "../Prerequisites.hpp"

class CodeToGlyphIndexMap
{
public:
	CodeToGlyphIndexMap() : mGlyphIndices(nullptr) {}

	~CodeToGlyphIndexMap()
	{
		clear();
	}

	void clear()
	{
		for (int j = 0; mGlyphIndices && j < 128; j++)
		{
			for (int i = 0; mGlyphIndices[j] && i < 128; i++)
			{
				delete[] mGlyphIndices[j][i];
				mGlyphIndices[j][i] = 0;
			}
			delete[] mGlyphIndices[j];
			mGlyphIndices[j] = 0;
		}
		delete[] mGlyphIndices;
		mGlyphIndices = 0;
	}

	int find(unsigned int charCode)
	{
		int outerIdx = (charCode >> 14) & 0b1111111;
		int innerIdx = (charCode >> 7)	& 0b1111111;
		int offset = charCode			& 0b1111111;

		if (charCode >= 0x110000 || !mGlyphIndices || 
			!mGlyphIndices[outerIdx] || !mGlyphIndices[outerIdx][innerIdx])
			return -1;

		return mGlyphIndices[outerIdx][innerIdx][offset];
	}

	void insert(unsigned int charCode, int glyphIndex)
	{
		int outerIdx = (charCode >> 14) & 127;
		int innerIdx = (charCode >> 7)	& 127;
		int offset = charCode			& 127;

		if (charCode >= 0x110000)
			return;

		if (!mGlyphIndices)
		{
			mGlyphIndices = new int**[128];
			for (int i = 0; i < 128; i++)
				mGlyphIndices[i] = 0;
		}

		if (!mGlyphIndices[outerIdx])
		{
			mGlyphIndices[outerIdx] = new int*[128];

			for (int i = 0; i < 128; i++)
				mGlyphIndices[outerIdx][i] = 0;
		}

		if (!mGlyphIndices[outerIdx][innerIdx])
		{
			mGlyphIndices[outerIdx][innerIdx] = new int[128];

			for (int i = 0; i < 128; i++)
				mGlyphIndices[outerIdx][innerIdx][i] = -1;
		}

		mGlyphIndices[outerIdx][innerIdx][offset] = glyphIndex;
	}

private:
	int ***mGlyphIndices;
};

#endif // CODE_TO_GLYPH_INDEX_MAP

