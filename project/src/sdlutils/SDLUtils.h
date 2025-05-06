
#pragma once


#include <SDL.h>
#include <string>
#include <unordered_map>

#include "../utils/Singleton.h"
#include "RandomNumberGenerator.h"
#include "Font.h"
#include "Music.h"
#include "SoundEffect.h"
#include "Texture.h"
#include "VirtualTimer.h"

class SDLUtils: public Singleton<SDLUtils> {

	friend Singleton<SDLUtils> ; // needed to give access to private constructors

public:

	// We abstract away the actual data structure we use for
	// tables. All we assume is that is has the following
	// methods
	//
	//   emplace(key,value)
	//   at(key)
	//   reserve(unsigned int)
	//   clear()
	//
	template<typename T>
	using sdl_resource_table = std::unordered_map<std::string,T>;

	// just a wrapper for accessing a map, to protect the table and
	// to throw a meaningful exception when the key is not found
	// in the original map
	//
	template<typename T>
	class map_access_wrapper {
		sdl_resource_table<T> &map_;
		std::string desc_;
	public:
		map_access_wrapper(sdl_resource_table<T> &map, std::string desc) :
				map_(map), desc_(desc) {
		}

		inline T& at(const std::string &key) {
			try {
				return map_.at(key);
			} catch (std::out_of_range &e) {
				throw "Key '" + key + "' does not exists in '" + desc_ + "'" + //
					desc_ + "'. Original error was: " + e.what();
			} catch (std::exception &e) {
				throw "Error when accessing key '" + key + "' of '" + //
						desc_ + "'. Original error was: " + e.what();
			} catch (...) {
				throw "Unknown error when accessing key '" + key + "' of '" + //
						desc_ + "'";
			}
		}

		inline T& operator[](const std::string &key) {
			return at(key);
		}
	};

	virtual ~SDLUtils() override;

	// cannot copy/move
	SDLUtils(SDLUtils&) = delete;
	SDLUtils(SDLUtils&&) = delete;
	SDLUtils& operator=(SDLUtils&) = delete;
	SDLUtils& operator=(SDLUtils&&) = delete;

	// access to the underlying SDL_Window -- in principle not needed
	inline SDL_Window* window() {
		return _window;
	}

	// access to the underlying SDL_Renderer -- needed when creating textures
	// other than those initialised in this class
	inline SDL_Renderer* renderer() {
		return _renderer;
	}

	// clear the renderer with a given SDL_Color
	inline void clearRenderer(SDL_Color bg = build_sdlcolor(0xAAAAAFF)) {
		SDL_SetRenderDrawColor(_renderer, COLOREXP(bg));
		SDL_RenderClear(_renderer);
	}

	// present the current content of the renderer
	inline void presentRenderer() {
		SDL_RenderPresent(_renderer);
	}

	// the window's width
	inline int width() const {
		return _width;
	}
	// the window's height
	inline int height() const {
		return _height;
	}

	inline int &width() {
		return _width;
	}
	// the window's height
	inline int &height() {
		return _height;
	}


	// toggle to full-screen/window mode
	inline void toggleFullScreen() {
		auto flags = SDL_GetWindowFlags(_window);
		if (flags & SDL_WINDOW_FULLSCREEN) {
			SDL_SetWindowFullscreen(_window, 0);
		} else {
			SDL_SetWindowFullscreen(_window, SDL_WINDOW_FULLSCREEN);
		}
	}

	// show the cursor when mouse is over the window
	inline void showCursor() {
		SDL_ShowCursor(1);
	}

	// hide the cursor when mouse is over the window
	inline void hideCursor() {
		SDL_ShowCursor(0);
	}

	// All resource maps can be modified from outside, this way you can store
	// your own dynamically. Be careful when modifying them!

	// fonts map
	inline auto& fonts() {
		return _fontsAccessWrapper;
	}

	// images map
	inline auto& images() {
		return _imagesAccessWrapper;
	}

	// messages map
	inline auto& msgs() {
		return _msgsAccessWrapper;
	}

	// sound effects map
	inline auto& soundEffects() {
		return _soundsAccessWrapper;
	}

	// musics maps
	inline auto& musics() {
		return _musicsAccessWrapper;
	}

	// Access to the random number generator. It is important to always
	// use this generator, this way you can regenerate the same sequence
	// if you start from the same seed
	inline RandomNumberGenerator& rand() {
		return _random;
	}

	// Access to the virtual timer, it is useful when you allow to 'pause'
	// your game, also for synchronising clocks of players (when using sdlnet)
	inline VirtualTimer& virtualTimer() {
		return _timer;
	}

	// Access to real time -- the one of SDL_GetTicks
	inline Uint32 currRealTime() const {
		return SDL_GetTicks();
	}

	// Reset the current time to the current real time, and delta-time to 0. Should
	// be called before entering the game loop.
	inline void resetTime() {
		_currTime = currRealTime();
		_deltaTime = 0;
	}

	// Register current time. It meant to be called at the begging of the game
	// loop, so objects can use the time of the current "simulation" step -- because
	// all objects should see the same time.
	//
	// It also calculate and store the delta-time which is the difference between the
	// current and last time.
	//
	inline Uint32 regCurrTime() {
		Uint32 currTime = currRealTime();
		_deltaTime = _currTime - currTime;
		return _currTime = currTime;
	}

	// Return the last registered time
	inline Uint32 currTime() const {
		return _currTime;
	}

	// Access the delta time, i.e., the difference between the last two game
	// loop iterations -- last two calls to regCurrTime.
	inline Uint32 deltaTime() const {
		return _deltaTime;
	}

private:

	SDLUtils();
	bool init(std::string windowTitle, int width, int height);
	bool init(std::string windowTitle, int width, int height,
			std::string filename);

	void initWindow();
	void closeWindow();
	void initSDLExtensions(); // initialize resources (fonts, textures, audio, etc.)
	void closeSDLExtensions(); // free resources the
	void loadReasources(std::string filename); // load resources from the json file

	std::string _windowTitle; // window title
	int _width; // window width
	int _height; // window height

	SDL_Window *_window; // the window
	SDL_Renderer *_renderer; // the renderer

	sdl_resource_table<Font> _fonts; // fonts map (string -> font)
	sdl_resource_table<Texture> _images; // textures map (string -> texture)
	sdl_resource_table<Texture> _msgs; // textures map (string -> texture)
	sdl_resource_table<SoundEffect> _sounds; // sounds map (string -> sound)
	sdl_resource_table<Music> _musics; // musics map (string -> music)

	map_access_wrapper<Font> _fontsAccessWrapper;
	map_access_wrapper<Texture> _imagesAccessWrapper;
	map_access_wrapper<Texture> _msgsAccessWrapper;
	map_access_wrapper<SoundEffect> _soundsAccessWrapper;
	map_access_wrapper<Music> _musicsAccessWrapper;

	RandomNumberGenerator _random; // (pseudo) random numbers generator
	VirtualTimer _timer; // virtual timer

	Uint32 _currTime;
	Uint32 _deltaTime;

};

// This macro defines a compact way for using the singleton SDLUtils, instead of
// writing SDLUtils::instance()->method() we write sdlutils().method()
//
inline SDLUtils& sdlutils() {
	return *SDLUtils::Instance();
}
