#ifndef PART_HPP
#define PART_HPP
#include "Prerequisites.hpp"

struct GeometryLayout
{
	enum class Component
	{
		BYTE,
		UBYTE,
		INT,
		UINT,
		SHORT,
		USHORT,
		HALF_FLOAT,
		FLOAT,
		DOUBLE
	};

	std::vector<uint8_t> sizes;
	std::vector<Component> types;
	std::vector<uint8_t> normalized;
	size_t vertexSize;

	GeometryLayout(std::vector<uint8_t> &sizes, std::vector<Component> &types,
		std::vector<uint8_t> &normalized, size_t vertexSize)
		: sizes(sizes), types(types), 
		normalized(normalized), vertexSize(vertexSize) {}
};

class GeometryPart
{
	friend class Batch;

public:
	GeometryPart();
	virtual ~GeometryPart();
	void setHidden(bool state);
	void toggleHidden();
	uint16_t* getElements();
	uint32_t getElementCount();

	virtual uint8_t* getBytes() = 0;
	virtual uint32_t getVertexCount() = 0;
	virtual void clear() = 0;

protected:
	std::vector<uint16_t> mElements;
	void markBatchDirty();

private:
	Batch *mBatch;
	bool mHidden;
	uint32_t mId;
	// TODO: materials, textures etc.
};

typedef std::shared_ptr<GeometryPart> GeometryPartSharedPtr;

#endif // PART_HPP

