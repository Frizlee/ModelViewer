#ifndef BATCH_HPP
#define BATCH_HPP
#include "Prerequisites.hpp"
#include "GeometryPart.hpp"

struct BatchDesc
{
	enum class DrawType
	{
		TRIANGLES,
		TRIANGLE_ELEMENTS
	};

	DrawType drawType;
	std::shared_ptr<GeometryLayout> layout;
};



class Batch
{
public:
	Batch();
	Batch(GeometryBuffer *buffer, size_t size, BatchDesc &desc);
	~Batch();
	void setBuffer(GeometryBuffer *buffer, size_t size, BatchDesc &desc);
	uint32_t newPart(GeometryPartSharedPtr part);
	GeometryPartSharedPtr findPart(uint32_t partId);
	void update();
	void deletePart(GeometryPartSharedPtr part);
	void deletePart(uint32_t partId);
	void render();
	void setDirty(bool dirty);
	
private:
	void cleanup();
	bool hasElementArray();
	typedef std::list<GeometryPartSharedPtr> PartList;

	GLuint mVao;
	GLuint mEbo;
	GeometryBuffer *mBuffer;
	BatchDesc mDesc;
	PartList mParts;
	uint32_t mAllVertices;
	size_t mOffset;
	size_t mSize;
	bool mDirty;

	struct MultiDraw
	{
		std::vector<GLsizei> counts;
		std::vector<void *> startElements;
		std::vector<GLint> baseVertices;
	} mMultiDraw;
};

#endif // BATCH_HPP

