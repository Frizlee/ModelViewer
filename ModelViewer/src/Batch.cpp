#include "Batch.hpp"
#include "GeometryBuffer.hpp"

static const size_t ELEMENT_BUFFER_SIZE = 1024;

Batch::Batch()
	: mDirty(true), mBuffer(nullptr), mVao(0), mEbo(0)
{
	gl::GenVertexArrays(1, &mVao);
}

Batch::Batch(GeometryBuffer *buffer, size_t size, BatchDesc &desc)
	: mDirty(true), mBuffer(nullptr), mEbo(0)
{
	gl::GenVertexArrays(1, &mVao);
	setBuffer(buffer, size, desc);
}

Batch::~Batch()
{
	cleanup();
	
	gl::DeleteVertexArrays(1, &mVao);
}

void Batch::setBuffer(GeometryBuffer *buffer, size_t size, BatchDesc &desc)
{
	cleanup();

	mOffset = buffer->alloc(size);

	if (mOffset == 0)
		throw 1; // TODO: Error handling.

	mBuffer = buffer;
	mDesc = desc;
	mSize = size;

	gl::BindVertexArray(mVao);
	mBuffer->bind();
	
	if (hasElementArray() == true)
	{
		gl::GenBuffers(1, &mEbo);
		gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, mEbo);
		gl::BufferData(gl::ELEMENT_ARRAY_BUFFER, ELEMENT_BUFFER_SIZE, nullptr, gl::DYNAMIC_DRAW);
	}
	
	size_t offset = 0;
	GLenum type;
	size_t compSize;

	for (uint32_t i = 0; i < mDesc.layout->sizes.size(); ++i)
	{
		gl::EnableVertexAttribArray(i);

		switch (mDesc.layout->types[i])
		{
		case GeometryLayout::Component::BYTE:
			type = gl::BYTE;
			compSize = sizeof(int8_t);
			break;

		case GeometryLayout::Component::UBYTE:
			type = gl::UNSIGNED_BYTE;
			compSize = sizeof(uint8_t);
			break;

		case GeometryLayout::Component::SHORT:
			type = gl::SHORT;
			compSize = sizeof(int16_t);
			break;

		case GeometryLayout::Component::USHORT:
			type = gl::UNSIGNED_SHORT;
			compSize = sizeof(uint16_t);
			break;

		case GeometryLayout::Component::INT:
			type = gl::INT;
			compSize = sizeof(int32_t);
			break;

		case GeometryLayout::Component::UINT:
			type = gl::UNSIGNED_INT;
			compSize = sizeof(uint32_t);
			break;

		case GeometryLayout::Component::HALF_FLOAT:
			type = gl::HALF_FLOAT;
			compSize = sizeof(int16_t);
			break;

		case GeometryLayout::Component::FLOAT:
			type = gl::FLOAT;
			compSize = sizeof(float);
			break;

		case GeometryLayout::Component::DOUBLE:
			type = gl::DOUBLE;
			compSize = sizeof(double);
			break;
		}

		gl::VertexAttribPointer(i, mDesc.layout->sizes[i], type,
			(mDesc.layout->normalized[i] == 0) ? gl::FALSE_ : gl::TRUE_, 
			mDesc.layout->vertexSize, (GLvoid*)(mOffset + offset));
		offset += mDesc.layout->sizes[i] * compSize;
	}

	mBuffer->Unbind();
	gl::BindVertexArray(0);
}

uint32_t Batch::newPart(GeometryPartSharedPtr part)
{
	PartList::iterator it;

	for (it = mParts.begin(); it != mParts.end(); ++it)
	{
		if ((*it)->mId == part->mId)
			throw 1; // TODO: Error handling.
	}

	mParts.push_back(part);
	part->mBatch = this;

	if (part->mHidden == false)
		mDirty = true;

	return part->mId;
}

GeometryPartSharedPtr Batch::findPart(uint32_t partId)
{
	PartList::iterator it;

	for (it = mParts.begin(); it != mParts.end(); ++it)
	{
		if ((*it)->mId == partId)
			return *it;
	}

	return nullptr;
}

void Batch::update()
{
	if (mDirty = true)
	{
		size_t offset = mOffset;
		PartList::iterator it;
		bool hasElements = hasElementArray();
		size_t elementOffset = 0;
		mAllVertices = 0;

		if (hasElements)
		{
			mMultiDraw.baseVertices.clear();
			mMultiDraw.counts.clear();
			mMultiDraw.startElements.clear();
			gl::BindBuffer(gl::ELEMENT_ARRAY_BUFFER, mEbo);
		}

		for (it = mParts.begin(); it != mParts.end(); ++it)
		{
			if ((*it)->mHidden == true)
				continue;

			mBuffer->update((*it)->getBytes(),
				mDesc.layout->vertexSize * (*it)->getVertexCount(), offset);

			offset += mDesc.layout->vertexSize * (*it)->getVertexCount();
			mAllVertices += (*it)->getVertexCount();

			if (hasElements == false)
				continue;

			gl::BufferSubData(gl::ELEMENT_ARRAY_BUFFER, elementOffset,
				(*it)->getElementCount() * sizeof(uint16_t), (*it)->getElements());

			mMultiDraw.counts.push_back((*it)->getElementCount());
			mMultiDraw.startElements.push_back(
				reinterpret_cast<void*>(elementOffset * sizeof(uint16_t)));
			mMultiDraw.baseVertices.push_back(mAllVertices - (*it)->getVertexCount());

			elementOffset += (*it)->getElementCount() * sizeof(uint16_t);
		}

		mDirty = false;
	}
}

void Batch::deletePart(GeometryPartSharedPtr part)
{
	deletePart(part->mId);
}

void Batch::deletePart(uint32_t partId)
{
	PartList::iterator it;
	
	for (it = mParts.begin(); it != mParts.end(); ++it)
	{
		if ((*it)->mId == partId)
		{
			if ((*it)->mHidden == true)
				mDirty = true;

			mParts.erase(it);
			break;
		}
	}
}

void Batch::render()
{
	gl::BindVertexArray(mVao);

	switch (mDesc.drawType)
	{
	case BatchDesc::DrawType::TRIANGLES:
		gl::DrawArrays(gl::TRIANGLES, 0, mAllVertices);
		break;

	case BatchDesc::DrawType::TRIANGLE_ELEMENTS:
		//gl::DrawElementsBaseVertex(gl::TRIANGLES, mMultiDraw.counts[0], gl::UNSIGNED_SHORT,
		//	mMultiDraw.startElements[0], mMultiDraw.baseVertices[0]);
		gl::MultiDrawElementsBaseVertex(gl::TRIANGLES, &mMultiDraw.counts[0],
			gl::UNSIGNED_SHORT, &mMultiDraw.startElements[0], mMultiDraw.counts.size(),
			&mMultiDraw.baseVertices[0]);
		break;
	}

}

void Batch::setDirty(bool dirty)
{
	mDirty = dirty;
}

void Batch::cleanup()
{
	if (mBuffer != nullptr)
		mBuffer->free(mOffset);

	if (mEbo != 0)
		gl::DeleteBuffers(1, &mEbo);
}

bool Batch::hasElementArray()
{
	switch (mDesc.drawType)
	{
	case BatchDesc::DrawType::TRIANGLE_ELEMENTS:
		return true;

	default:
		return false;
	}
}

