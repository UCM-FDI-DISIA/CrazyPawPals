
#pragma once


/*
 * This class stores an error message, just to avoid
 * using exceptions and pass a boolean around indicating
 * if an operation was successful.
 */

class SDLUtilsError {
public:
	SDLUtilsError() = delete;
	virtual ~SDLUtilsError() = delete;

	inline static const char* get() {
		return _error;
	}

	inline static void set(const char *error) {
		_error = error;
	}

private:

	static const char *_error;

};

