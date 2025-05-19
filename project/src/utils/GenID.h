
#pragma once

#include <cstdint>
#include <iostream>
#include <type_traits>

/*
 * This code generates unique IDs for type at runtime.
 * The template parameter G is simply to allow using it
 * in the same program to generate two set of IDs, etc.
 *
 */

template<typename IDTYPE = uint8_t, uint8_t G = 0>
class GenID {

	static_assert(std::is_unsigned_v<IDTYPE> && std::is_integral_v<IDTYPE>);

public:
	GenID() {
	}
	virtual ~GenID() {
	}

	template<typename T>
	static int get_id() {
		static int my_id = _id++;
		return my_id;
	}

private:
	static IDTYPE _id;

};

template<typename IDTYPE, uint8_t G>
IDTYPE GenID<IDTYPE, G>::_id = 100;
