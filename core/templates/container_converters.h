/**************************************************************************/
/*  container_converters.h                                                */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#ifndef CONTAINER_CONVERTERS_H
#define CONTAINER_CONVERTERS_H

#include "core/templates/list.h"
#include "core/templates/vector.h"
#include "core/variant/typed_array.h"

template <typename T>
List<T, DefaultAllocator> Vector<T>::to_list() const {
	List<T> ret;

	for (const T &E : *this) {
		ret.push_back(E);
	}
	return ret;
}

template <typename T, typename A>
Vector<T> List<T, A>::to_vector() const {
	Vector<T> ret;
	// FIXME: resize() can cause unnecessary initialization of elements if they are not trivially destructible.
	// Something like LocalVector's reserve() would be better, but Vector does not support it yet.
	ret.resize(size());

	T *write = ret.ptrw();
	for (const T &E : *this) {
		*write = E;
		write++;
	}
	return ret;
}

template <typename T>
TypedArray<T> Vector<T>::to_typed_array() const {
	TypedArray<T> ret;
	ret.resize(size());

	Array::Iterator itr = ret.begin();
	for (const T &E : *this) {
		*itr = E;
		++itr;
	}
	return ret;
}

template <typename T, typename A>
template <typename R>
TypedArray<R> List<T, A>::to_typed_array() const {
	TypedArray<R> ret;
	ret.resize(size());

	Array::Iterator itr = ret.begin();
	for (const T &E : *this) {
		const R value = E; // *itr uses Variant, so this allows better conversion.
		*itr = value;
		++itr;
	}
	return ret;
}

#endif // CONTAINER_CONVERTERS_H
