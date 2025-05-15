
#pragma once

#include <iostream>
#include <cassert>


/*
 * This is an attempt to have a single Singleton class that can be used
 * via inheritance to make some other class Singleton. It is just to avoid
 * copying the Singleton declarations each time.
 *
 * >>> Requirements:
 *
 * A class T that inherits this class must have
 *
 *  - a private default constructor
 *  - a private method to initialize the object
 *  - a (private) destructor
 *  - Must be "friend" of Singleton<T>
 *
 * >>> Usage:
 *
 * class A : public Singleton<A> {
 *
 *    friend Singleton<A>; // this way Singleton can call the constructor of A
 *
 * private: // constructors are private
 *    A() {
 *      //....
 *    }
 *    bool init(...)
 *    virtual ~A() {
 *    }
 *
 * public:
 *    // the rest of the functionality
 * }
 *
 * >>> Other comments
 *
 * This is a revised version, done in the academic 2024-25 academic year,
 * incorporating comments of Pedro Pablo Goméz Martín.
 *
 */

template<typename T>
class Singleton {
protected:
	Singleton() {
	}

public:

	// Cannot copy objects of this type
	Singleton<T>& operator=(const Singleton<T> &o) = delete;
	Singleton(const Singleton<T> &o) = delete;

	virtual ~Singleton() {
	}

	// Initialization of the Singleton object. Should be called at
	// the beginning of the class.
	//
	template<typename ...Targs>
	inline static bool Init(Targs &&...args) {
		assert(!_instance);
		_instance = new T();
		if (_instance->init(std::forward<Targs>(args)...)) {
			return true; // all OK
		} else { // Something went wrong
			delete _instance;
			return false;
		}
	}

	inline static T* Instance() {
		assert(_instance != nullptr);
		return _instance;
	}

	inline static bool HasInstance() {
		return _instance != nullptr;
	}

	inline static void Release() {
		assert(_instance);
		delete _instance;
		_instance = nullptr;
	}

private:
	static T *_instance;
};

template<typename T> T *Singleton<T>::_instance;

