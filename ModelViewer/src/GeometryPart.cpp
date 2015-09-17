#include "GeometryPart.hpp"
#include "Batch.hpp"

GeometryPart::GeometryPart() : mHidden(false), mBatch(nullptr),
	mId(reinterpret_cast<unsigned int>(this)) 
{
}

GeometryPart::~GeometryPart()
{
}

void GeometryPart::setHidden(bool state)
{
	if (mHidden != state)
	{
		mHidden = state;
		
		markBatchDirty();
	}
}

void GeometryPart::toggleHidden()
{
	mHidden = !mHidden;

	markBatchDirty();
}

uint16_t* GeometryPart::getElements()
{
	return mElements.data();
}

uint32_t GeometryPart::getElementCount()
{
	return mElements.size();
}

void GeometryPart::markBatchDirty()
{
	if (mBatch != nullptr)
		mBatch->setDirty(true);
}

