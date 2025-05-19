// This file is part of the course TPV2@UCM - Samir Genaim

#pragma once


#include <string>
#include <SDL_mixer.h>
#include <cassert>

#define _CHECK_CHANNEL_(channel) \
	assert(channel >= -1 && channel < static_cast<int>(_channels));

class SoundEffect {
public:

	// cannot copy objects of this type!
	SoundEffect& operator=(SoundEffect &other) = delete;
	SoundEffect(const SoundEffect&) = delete;

	SoundEffect(const std::string &fileName) {
		_chunk = Mix_LoadWAV(fileName.c_str());
		assert(_chunk != nullptr);
	}

	SoundEffect(SoundEffect &&other) noexcept {
		_chunk = other._chunk;
		other._chunk = nullptr;
	}

	SoundEffect& operator=(SoundEffect &&other) noexcept {
		this->~SoundEffect();
		_chunk = other._chunk;
		other._chunk = nullptr;
		return *this;
	}

	virtual ~SoundEffect() {
		if (_chunk != nullptr)
			Mix_FreeChunk(_chunk);
	}

	inline int play(int loops = 0, int channel = -1) const {
		_CHECK_CHANNEL_(channel)
		assert(loops >= -1);
		return Mix_PlayChannel(channel, _chunk, loops);
	}

	inline int setVolume(int volume) {
		assert(volume >= 0 && volume <= 128);
		return Mix_VolumeChunk(_chunk, volume);
	}

	// static methods for sound effects
	//
	inline static void pauseChannel(int channel = -1) {
		_CHECK_CHANNEL_(channel)
		Mix_Pause(channel);
	}

	inline static void resumeChannel(int channel = -1) {
		_CHECK_CHANNEL_(channel)
		Mix_Resume(channel);
	}

	inline static void haltChannel(int channel = -1) {
		_CHECK_CHANNEL_(channel)
		Mix_HaltChannel(channel);
	}

	inline static int setChannelVolume(int volume, int channel = -1) {
		_CHECK_CHANNEL_(channel)
		assert(volume >= 0 && volume <= 128);
		return Mix_Volume(channel, volume);
	}

	inline static int setNumberofChannels(int n) {
		assert(n > 0);
		return _channels = Mix_AllocateChannels(n);
	}

private:

	inline static void checkChannel(int channel) {
		assert(channel >= -1 && channel < static_cast<int>(_channels));
	}

	Mix_Chunk *_chunk;
	static int _channels; // initialized in cpp
};

