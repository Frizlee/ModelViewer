#include "GeometryBuffer.hpp"

GeometryBuffer::GeometryBuffer() : mRoot(new Node())
{
	gl::GenBuffers(1, &mVbo);
}

GeometryBuffer::GeometryBuffer(size_t size) : mRoot(new Node())
{
	gl::GenBuffers(1, &mVbo);
	setSize(size);
}

void GeometryBuffer::setSize(size_t size)
{
	bind();
	gl::BufferData(gl::ARRAY_BUFFER, size, nullptr, gl::DYNAMIC_DRAW);
	mRoot->size = size;
	mRoot->offset = 1;
}

void GeometryBuffer::bind()
{
	gl::BindBuffer(gl::ARRAY_BUFFER, mVbo);
}

void GeometryBuffer::update(uint8_t *bytes, size_t size, size_t position)
{
	bind();

	if (size + position > mRoot->size)
	{
		// TODO: Error handling.
		return;
	}
	gl::BufferSubData(gl::ARRAY_BUFFER, position, size, bytes);
}

void GeometryBuffer::update(std::vector<uint8_t> &bytes, size_t position)
{
	bind();

	if (bytes.size() + position > mRoot->size)
	{
		// TODO: Error handling.
		return;
	}
	gl::BufferSubData(gl::ARRAY_BUFFER, position, bytes.size(), bytes.data());
}


size_t GeometryBuffer::alloc(size_t size)
{
	return mRoot->insert(size)->offset;
}

void GeometryBuffer::free(size_t offset)
{
	mRoot->remove(offset);
}

void GeometryBuffer::Unbind()
{
	gl::BindBuffer(gl::ARRAY_BUFFER, 0);
}

GeometryBuffer::Node::~Node()
{
	if (child[0] != nullptr)
		delete child[0];
	if (child[1] != nullptr)
		delete child[1];
}

GeometryBuffer::Node* GeometryBuffer::Node::insert(size_t s)
{
	if (child[0] != nullptr && child[1] != nullptr)
	{
		Node* nd = child[0]->insert(s);

		if (nd == nullptr)
			return child[1]->insert(s);
	}

	if (free == false)
		return nullptr;

	if (size < s)
		return nullptr;

	if (size == s)
	{
		free = false;
		return this;
	}

	child[0] = new Node();
	child[1] = new Node();

	child[0]->offset = offset;
	child[0]->size = s;

	child[1]->offset = (size_t)offset + s;
	child[1]->size = size - s;

	return child[0]->insert(s);
}

void GeometryBuffer::Node::remove(size_t p)
{
	if (child[0] != nullptr && child[1] != nullptr)
	{
		if (child[1]->offset < p)
			child[0]->remove(p);
		else
			child[1]->remove(p);

		if (child[0]->free == true && child[1]->free == true)
		{
			delete child[0];
			delete child[1];
		}

		return;
	}

	if (offset == p)
		free = true;

	return;
}
