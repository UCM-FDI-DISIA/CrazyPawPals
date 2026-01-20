// This file is part of the course TPV2@UCM - Samir Genaim

#pragma once


template<typename T>
struct ptr_iterator {
	T *_ptr;
	ptr_iterator(T *ptr) {
		_ptr = ptr;
	}

	T& operator*() {
		return *_ptr;
	}

	ptr_iterator& operator++() {
		_ptr++;
		return *this;
	}

	ptr_iterator operator++(int) {
		auto tmp = *this;
		++(*this);
		return tmp;
	}

	bool operator==(ptr_iterator &o) {
		return _ptr == o._ptr;
	}

	bool operator!=(ptr_iterator &o) {
		return _ptr != o._ptr;
	}

};

