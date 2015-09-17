#ifndef RECT_HPP
#define RECT_HPP
#include "Prerequisites.hpp"

template <typename T>
struct TRect
{
	T left;
	T bottom;
	T width;
	T height;

	T right() { return left + width; }
	T top() { return bottom + height; } // Works only with Y axis upwards.

	bool operator==(const TRect<T> &rc);
	TRect() {}
	TRect(T left, T bottom, T width, T height) : 
		left(left), bottom(bottom), width(width), height(height) {};
};

template<typename T>
inline bool TRect<T>::operator==(const TRect<T>& rc)
{
	return (left == rc.left &&
		bottom == rc.bottom &&
		width == rc.width &&
		height == rc.height);
}

#endif // RECT_HPP

