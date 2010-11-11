/* REminiscence - Flashback interpreter
 * Copyright (C) 2005-2007 Gregory Montoir
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <SDL.h>
#include "scaler.h"
#include "systemstub.h"


struct SystemStub_SDL : SystemStub {
	enum {
		MAX_BLIT_RECTS = 200,
		SOUND_SAMPLE_RATE = 11025,
		JOYSTICK_COMMIT_VALUE = 3200
	};

	uint8 *_offscreen;
	SDL_Surface *_screen;
	SDL_Surface *_sclscreen;
	bool _fullscreen;
	uint8 _scaler;
	uint8 _overscanColor;
	uint16 _pal[256];
	uint16 _screenW, _screenH;
	SDL_Joystick *_joystick;
	SDL_Rect _blitRects[MAX_BLIT_RECTS];
	uint16 _numBlitRects;

	virtual ~SystemStub_SDL() {}
	virtual void init(const char *title, uint16 w, uint16 h);
	virtual void destroy();
	virtual void setPalette(const uint8 *pal, uint16 n);
	virtual void setPaletteEntry(uint8 i, const Color *c);
	virtual void getPaletteEntry(uint8 i, Color *c);
	virtual void setOverscanColor(uint8 i);
	virtual void copyRect(int16 x, int16 y, uint16 w, uint16 h, const uint8 *buf, uint32 pitch);
	virtual void updateScreen(uint8 shakeOffset);
	virtual void processEvents();
	virtual void sleep(uint32 duration);
	virtual uint32 getTimeStamp();
	virtual void startAudio(AudioCallback callback, void *param);
	virtual void stopAudio();
	virtual uint32 getOutputSampleRate();
	virtual void *createMutex();
	virtual void destroyMutex(void *mutex);
	virtual void lockMutex(void *mutex);
	virtual void unlockMutex(void *mutex);

	void prepareGfxMode();
	void cleanupGfxMode();
	void switchGfxMode(bool fullscreen, uint8 scaler);
	void flipGfx();
	void forceGfxRedraw();
	void drawRect(SDL_Rect *rect, uint8 color, uint16 *dst, uint16 dstPitch);
};

SystemStub *SystemStub_SDL_create() {
	return new SystemStub_SDL();
}

void SystemStub_SDL::init(const char *title, uint16 w, uint16 h) {
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK);
	SDL_ShowCursor(SDL_DISABLE);
	SDL_WM_SetCaption(title, NULL);
	memset(&_pi, 0, sizeof(_pi));
	_screenW = w;
	_screenH = h;
	// allocate some extra bytes for the scaling routines
	int size_offscreen = (w + 2) * (h + 2) * 2;
	_offscreen = (uint8 *)malloc(size_offscreen);
	if (!_offscreen) {
		error("SystemStub_SDL::init() Unable to allocate offscreen buffer");
	}
	memset(_offscreen, 0, size_offscreen);
	_fullscreen = false;
	_scaler = 0;
	memset(_pal, 0, sizeof(_pal));
	prepareGfxMode();
	_joystick = NULL;
	if (SDL_NumJoysticks() > 0) {
		_joystick = SDL_JoystickOpen(0);
	}
}

void SystemStub_SDL::destroy() {
	cleanupGfxMode();
	if (SDL_JoystickOpened(0)) {
		SDL_JoystickClose(_joystick);
	}
	SDL_Quit();
}

void SystemStub_SDL::setPalette(const uint8 *pal, uint16 n) {
	assert(n <= 256);
	for (int i = 0; i < n; ++i) {
		uint8 r = pal[i * 3 + 0];
		uint8 g = pal[i * 3 + 1];
		uint8 b = pal[i * 3 + 2];
		_pal[i] = SDL_MapRGB(_screen->format, r, g, b);
	}
}

void SystemStub_SDL::setPaletteEntry(uint8 i, const Color *c) {
	uint8 r = (c->r << 2) | (c->r & 3);
	uint8 g = (c->g << 2) | (c->g & 3);
	uint8 b = (c->b << 2) | (c->b & 3);
	_pal[i] = SDL_MapRGB(_screen->format, r, g, b);
}

void SystemStub_SDL::getPaletteEntry(uint8 i, Color *c) {
	SDL_GetRGB(_pal[i], _screen->format, &c->r, &c->g, &c->b);
	c->r >>= 2;
	c->g >>= 2;
	c->b >>= 2;
}

void SystemStub_SDL::setOverscanColor(uint8 i) {
	_overscanColor = i;
}

void SystemStub_SDL::copyRect(int16 x, int16 y, uint16 w, uint16 h, const uint8 *buf, uint32 pitch) {
	if (_numBlitRects >= MAX_BLIT_RECTS) {
		warning("SystemStub_SDL::copyRect() Too many blit rects, you may experience graphical glitches");
	} else {
		// extend the dirty region by 1 pixel for scalers accessing 'outer' pixels
		--x;
		--y;
		w += 2;
		h += 2;

		if (x < 0) {
			x = 0;
		}
		if (y < 0) {
			y = 0;
		}
		if (x + w > _screenW) {
			w = _screenW - x;
		}
		if (y + h > _screenH) {
			h = _screenH - y;
		}

		SDL_Rect *br = &_blitRects[_numBlitRects];

		br->x = _pi.mirrorMode ? _screenW - (x + w) : x;
		br->y = y;
		br->w = w;
		br->h = h;
		++_numBlitRects;

		uint16 *p = (uint16 *)_offscreen + (br->y + 1) * _screenW + (br->x + 1);
		buf += y * pitch + x;

		if (_pi.mirrorMode) {
			while (h--) {
				for (int i = 0; i < w; ++i) {
					p[i] = _pal[buf[w - 1 - i]];
				}
				p += _screenW;
				buf += pitch;
			}
		} else {
			while (h--) {
				for (int i = 0; i < w; ++i) {
					p[i] = _pal[buf[i]];
				}
				p += _screenW;
				buf += pitch;
			}
		}
		if (_pi.dbgMask & PlayerInput::DF_DBLOCKS) {
			drawRect(br, 0xE7, (uint16 *)_offscreen + _screenW + 1, _screenW * 2);
		}
	}
}

void SystemStub_SDL::updateScreen(uint8 shakeOffset) {
	//SDL_Flip(_screen);
	
	const int mul = _scalers[_scaler].factor;
	if (shakeOffset == 0) {
		for (int i = 0; i < _numBlitRects; ++i) {
			SDL_Rect *br = &_blitRects[i];
			int16 dx = br->x * mul;
			int16 dy = br->y * mul;
			SDL_LockSurface(_sclscreen);
			uint16 *dst = (uint16 *)_sclscreen->pixels + dy * _sclscreen->pitch / 2 + dx;
			const uint16 *src = (uint16 *)_offscreen + (br->y + 1) * _screenW + (br->x + 1);
			(*_scalers[_scaler].proc)(dst, _sclscreen->pitch, src, _screenW, br->w, br->h);
			SDL_UnlockSurface(_sclscreen);
			br->x *= mul;
			br->y *= mul;
			br->w *= mul;
			br->h *= mul;
			SDL_BlitSurface(_sclscreen, br, _screen, br);
		}
		SDL_UpdateRects(_screen, _numBlitRects, _blitRects);
	} else {
		SDL_LockSurface(_sclscreen);
		uint16 w = _screenW;
		uint16 h = _screenH - shakeOffset;
		uint16 *dst = (uint16 *)_sclscreen->pixels;
		const uint16 *src = (uint16 *)_offscreen + _screenW + 1;
		(*_scalers[_scaler].proc)(dst, _sclscreen->pitch, src, _screenW, w, h);
		SDL_UnlockSurface(_sclscreen);

		SDL_Rect bsr, bdr;
		bdr.x = 0;
		bdr.y = 0;
		bdr.w = _screenW * mul;
		bdr.h = shakeOffset * mul;
		SDL_FillRect(_screen, &bdr, _pal[_overscanColor]);

		bsr.x = 0;
		bsr.y = 0;
		bsr.w = _screenW * mul;
		bsr.h = (_screenH - shakeOffset) * mul;
		bdr.x = 0;
		bdr.y = shakeOffset * mul;
		bdr.w = bsr.w;
		bdr.h = bsr.h;
		SDL_BlitSurface(_sclscreen, &bsr, _screen, &bdr);

		bdr.x = 0;
		bdr.y = 0;
		bdr.w = _screenW * mul;
		bdr.h = _screenH * mul;
		SDL_UpdateRects(_screen, 1, &bdr);
	}
	_numBlitRects = 0;
	
}

void SystemStub_SDL::processEvents() {
	SDL_Event ev;
	while (SDL_PollEvent(&ev)) {
		switch (ev.type) {
		case SDL_QUIT:
			_pi.quit = true;
			break;
		case SDL_JOYHATMOTION:
			_pi.dirMask = 0;
			if (ev.jhat.value & SDL_HAT_UP) {
				_pi.dirMask |= PlayerInput::DIR_UP;
			}
			if (ev.jhat.value & SDL_HAT_DOWN) {
				_pi.dirMask |= PlayerInput::DIR_DOWN;
			}
			if (ev.jhat.value & SDL_HAT_LEFT) {
				_pi.dirMask |= PlayerInput::DIR_LEFT;
			}
			if (ev.jhat.value & SDL_HAT_RIGHT) {
				_pi.dirMask |= PlayerInput::DIR_RIGHT;
			}
			break;
		case SDL_JOYAXISMOTION:
			switch (ev.jaxis.axis) {
			case 0:
				if (ev.jaxis.value > JOYSTICK_COMMIT_VALUE) {
					_pi.dirMask |= PlayerInput::DIR_RIGHT;
					if (_pi.dirMask & PlayerInput::DIR_LEFT) {
						_pi.dirMask &= ~PlayerInput::DIR_LEFT;
					}
				} else if (ev.jaxis.value < -JOYSTICK_COMMIT_VALUE) {
					_pi.dirMask |= PlayerInput::DIR_LEFT;
					if (_pi.dirMask & PlayerInput::DIR_RIGHT) {
						_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
					}
				} else {
					_pi.dirMask &= ~(PlayerInput::DIR_RIGHT | PlayerInput::DIR_LEFT);
				}
				break;
			case 1:
				if (ev.jaxis.value > JOYSTICK_COMMIT_VALUE) {
					_pi.dirMask |= PlayerInput::DIR_DOWN;
					if (_pi.dirMask & PlayerInput::DIR_UP) {
						_pi.dirMask &= ~PlayerInput::DIR_UP;
					}
				} else if (ev.jaxis.value < -JOYSTICK_COMMIT_VALUE) {
					_pi.dirMask |= PlayerInput::DIR_UP;
					if (_pi.dirMask & PlayerInput::DIR_DOWN) {
						_pi.dirMask &= ~PlayerInput::DIR_DOWN;
					}
				} else {
					_pi.dirMask = 0;
				}
				break;
			}
			break;
		case SDL_JOYBUTTONDOWN:
			switch (ev.jbutton.button) {
			case 0:
				_pi.space = true;
				break;
			case 1:
			    _pi.shift = true;
			    break;
			case 2:
			    _pi.enter = true;
			    break;
			case 3:
			    _pi.backspace = true;
			    break;
			}
			break;
		case SDL_JOYBUTTONUP:
			switch (ev.jbutton.button) {
			case 0:
				_pi.space = false;
				break;
			case 1:
			    _pi.shift = false;
			    break;
			case 2:
			    _pi.enter = false;
			    break;
			case 3:
			    _pi.backspace = false;
			    break;
			}
			break;
		case SDL_KEYUP:
			switch (ev.key.keysym.sym) {
			case SDLK_LEFT:
				_pi.dirMask &= ~PlayerInput::DIR_LEFT;
				break;
			case SDLK_RIGHT:
				_pi.dirMask &= ~PlayerInput::DIR_RIGHT;
				break;
			case SDLK_UP:
				_pi.dirMask &= ~PlayerInput::DIR_UP;
				break;
			case SDLK_DOWN:
				_pi.dirMask &= ~PlayerInput::DIR_DOWN;
				break;
			case SDLK_SPACE:
				_pi.space = false;
				break;
			case SDLK_RSHIFT:
			case SDLK_LSHIFT:
				_pi.shift = false;
				break;
			case SDLK_RETURN:
				_pi.enter = false;
				break;
			case SDLK_ESCAPE:
				_pi.escape = false;
				break;
			default:
				break;
			}
			break;
		case SDL_KEYDOWN:
			if (ev.key.keysym.mod & KMOD_ALT) {
				if (ev.key.keysym.sym == SDLK_RETURN) {
					switchGfxMode(!_fullscreen, _scaler);
				} else if (ev.key.keysym.sym == SDLK_KP_PLUS) {
					uint8 s = _scaler + 1;
					if (s < NUM_SCALERS) {
						switchGfxMode(_fullscreen, s);
					}
				} else if (ev.key.keysym.sym == SDLK_KP_MINUS) {
					int8 s = _scaler - 1;
					if (_scaler > 0) {
						switchGfxMode(_fullscreen, s);
					}
				}
				break;
			} else if (ev.key.keysym.mod & KMOD_CTRL) {
				if (ev.key.keysym.sym == SDLK_f) {
					_pi.dbgMask ^= PlayerInput::DF_FASTMODE;
				} else if (ev.key.keysym.sym == SDLK_b) {
					_pi.dbgMask ^= PlayerInput::DF_DBLOCKS;
				} else if (ev.key.keysym.sym == SDLK_i) {
					_pi.dbgMask ^= PlayerInput::DF_SETLIFE;
				} else if (ev.key.keysym.sym == SDLK_m) {
					_pi.mirrorMode = !_pi.mirrorMode;
					flipGfx();
				} else if (ev.key.keysym.sym == SDLK_s) {
					_pi.save = true;
				} else if (ev.key.keysym.sym == SDLK_l) {
					_pi.load = true;
				} else if (ev.key.keysym.sym == SDLK_KP_PLUS) {
					_pi.stateSlot = 1;
				} else if (ev.key.keysym.sym == SDLK_KP_MINUS) {
					_pi.stateSlot = -1;
				} else if (ev.key.keysym.sym == SDLK_r) {
					_pi.inpRecord = true;
				} else if (ev.key.keysym.sym == SDLK_p) {
					_pi.inpReplay = true;
				}
			}
			_pi.lastChar = ev.key.keysym.sym;
			switch (ev.key.keysym.sym) {
			case SDLK_LEFT:
				_pi.dirMask |= PlayerInput::DIR_LEFT;
				break;
			case SDLK_RIGHT:
				_pi.dirMask |= PlayerInput::DIR_RIGHT;
				break;
			case SDLK_UP:
				_pi.dirMask |= PlayerInput::DIR_UP;
				break;
			case SDLK_DOWN:
				_pi.dirMask |= PlayerInput::DIR_DOWN;
				break;
			case SDLK_BACKSPACE:
			case SDLK_TAB:
				_pi.backspace = true;
				break;
			case SDLK_SPACE:
				_pi.space = true;
				break;
			case SDLK_RSHIFT:
			case SDLK_LSHIFT:
				_pi.shift = true;
				break;
			case SDLK_RETURN:
				_pi.enter = true;
				break;
			case SDLK_ESCAPE:
				_pi.escape = true;
				break;
			default:
				break;
			}
			break;
		default:
			break;
		}
	}
}

void SystemStub_SDL::sleep(uint32 duration) {
	SDL_Delay(duration);
}

uint32 SystemStub_SDL::getTimeStamp() {
	return SDL_GetTicks();
}

void SystemStub_SDL::startAudio(AudioCallback callback, void *param) {
	SDL_AudioSpec desired;
	memset(&desired, 0, sizeof(desired));
	desired.freq = SOUND_SAMPLE_RATE;
	desired.format = AUDIO_S16;
	desired.channels = 1;
	desired.samples = 2048;
	desired.callback = callback;
	desired.userdata = param;
	if (SDL_OpenAudio(&desired, NULL) == 0) {
		SDL_PauseAudio(0);
	} else {
		error("SystemStub_SDL::startAudio() Unable to open sound device");
	}
}

void SystemStub_SDL::stopAudio() {
	SDL_CloseAudio();
}

uint32 SystemStub_SDL::getOutputSampleRate() {
	return SOUND_SAMPLE_RATE;
}

void *SystemStub_SDL::createMutex() {
	return SDL_CreateMutex();
}

void SystemStub_SDL::destroyMutex(void *mutex) {
	SDL_DestroyMutex((SDL_mutex *)mutex);
}

void SystemStub_SDL::lockMutex(void *mutex) {
	SDL_mutexP((SDL_mutex *)mutex);
}

void SystemStub_SDL::unlockMutex(void *mutex) {
	SDL_mutexV((SDL_mutex *)mutex);
}

void SystemStub_SDL::prepareGfxMode() {
	
	int w = _screenW * _scalers[_scaler].factor;
	int h = _screenH * _scalers[_scaler].factor;
	
	//_screen = SDL_SetVideoMode(w, h, 16, _fullscreen ? (SDL_FULLSCREEN | SDL_HWSURFACE) : SDL_HWSURFACE);
	debug(DBG_INFO, "Requesting video %dx%d", w, h);
	_screen = SDL_SetVideoMode(w, h, 16, SDL_SWSURFACE);
	if (!_screen) {
		error("SystemStub_SDL::prepareGfxMode() Unable to allocate _screen buffer");
	}
	const SDL_PixelFormat *pf = _screen->format;
	// Android TODO: get rid of filthy scaler and draw directly to Android surface
	_sclscreen = SDL_CreateRGBSurface(SDL_SWSURFACE, w, h, 16, pf->Rmask, pf->Gmask, pf->Bmask, pf->Amask);
	//_sclscreen = _screen; // Android hack
	if (!_sclscreen) {
		error("SystemStub_SDL::prepareGfxMode() Unable to allocate _sclscreen buffer");
	}
	forceGfxRedraw();
}

void SystemStub_SDL::cleanupGfxMode() {
	if (_offscreen) {
		free(_offscreen);
		_offscreen = 0;
	}
	
	if (_sclscreen) {
		SDL_FreeSurface(_sclscreen);
		_sclscreen = 0;
	}
	
	if (_screen) {
		// freed by SDL_Quit()
		_screen = 0;
	}
}

void SystemStub_SDL::switchGfxMode(bool fullscreen, uint8 scaler) {
	
	SDL_Surface *prev_sclscreen = _sclscreen;
	SDL_FreeSurface(_screen);
	_fullscreen = fullscreen;
	_scaler = scaler;
	prepareGfxMode();
	SDL_BlitSurface(prev_sclscreen, NULL, _sclscreen, NULL);
	SDL_FreeSurface(prev_sclscreen);
}

void SystemStub_SDL::flipGfx() {
	uint16 scanline[256];
	assert(_screenW <= 256);
	uint16 *p = (uint16 *)_offscreen + _screenW + 1;
	for (int y = 0; y < _screenH; ++y) {
		p += _screenW;
		for (int x = 0; x < _screenW; ++x) {
			scanline[x] = *--p;
		}
		memcpy(p, scanline, _screenW * sizeof(uint16));
		p += _screenW;
	}
	forceGfxRedraw();
}

void SystemStub_SDL::forceGfxRedraw() {
	_numBlitRects = 1;
	_blitRects[0].x = 0;
	_blitRects[0].y = 0;
	_blitRects[0].w = _screenW;
	_blitRects[0].h = _screenH;
}

void SystemStub_SDL::drawRect(SDL_Rect *rect, uint8 color, uint16 *dst, uint16 dstPitch) {
	dstPitch >>= 1;
	int x1 = rect->x;
	int y1 = rect->y;
	int x2 = rect->x + rect->w - 1;
	int y2 = rect->y + rect->h - 1;
	assert(x1 >= 0 && x2 < _screenW && y1 >= 0 && y2 < _screenH);
	for (int i = x1; i <= x2; ++i) {
		*(dst + y1 * dstPitch + i) = *(dst + y2 * dstPitch + i) = _pal[color];
	}
	for (int j = y1; j <= y2; ++j) {
		*(dst + j * dstPitch + x1) = *(dst + j * dstPitch + x2) = _pal[color];
	}
}
