#include "Tracer.h"
#include "Vec3.h"
#include "Hit.h"
#include "mathutils.h"
#include "Prng.h"
#include "Mesh.h"

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <iostream>
#include <string>
#include <chrono>
#include <sstream>

bool g_stop = false;
bool g_debug_read = false;

Tracer g_tracer;
float exposure = 1;
float tonemap(float v) {
	return sqrt(clamp01(v * exposure));
	//return (v <= 0) ? 0 : std::tanh(std::sqrt(v * exposure));
}

constexpr uint32_t rgba(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
	return a << 24 | r << 16 | g << 8 | b;
}

constexpr uint32_t rgba(const Vec3& c) {
	return rgba(tonemap(c.x) * 255, tonemap(c.y) * 255, tonemap(c.z) * 255, 255);
}

void updateScreen(SDL_Renderer* renderer, SDL_Texture* framebuffer) {
    if (g_tracer.numSamples < 1) return;

	uint32_t* pixels = nullptr;
    Vec3* samples = g_tracer.buffer;
	int pitch = g_tracer.width;
	if (SDL_LockTexture(framebuffer, nullptr, (void**)&pixels, &pitch)) {
		std::cerr << SDL_GetError() << "\n";
		exit(1);
	}

    for (int i = 0; i < g_tracer.width * g_tracer.height; i++) {
		*pixels++ = rgba(*samples++ / g_tracer.numSamples);
	}

    SDL_UnlockTexture(framebuffer);
	SDL_RenderCopy(renderer, framebuffer, NULL, NULL);
	SDL_RenderPresent(renderer);
}

extern int thread_num_rays[8];

#ifdef _WIN32
#include <Windows.h>
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#else
int main(int argc, char** argv) {
#endif

	Prng prng(0);

	if (SDL_Init(SDL_INIT_VIDEO)) {
		std::cerr << "Error initializing SDL.\n";
		return 1;
	}

	auto window = SDL_CreateWindow("Tracer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, g_tracer.width, g_tracer.height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
	if (!window) {
		std::cerr << SDL_GetError();
		return 1;
	}

	auto renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer) {
		std::cerr << SDL_GetError();
		return 1;
	}

	auto framebuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_tracer.width, g_tracer.height);
	if (!framebuffer) {
		std::cerr << SDL_GetError();
		return 1;
	}

	while (!g_stop) {
		//Handle events on queue
		SDL_Event e;
		if (SDL_PollEvent(&e) != 0) {
			//User requests quit
			switch (e.type) {
			case SDL_QUIT:
				g_stop = true;
				break;

			case SDL_KEYDOWN:
				if (e.key.keysym.sym == SDLK_UP) {
					g_tracer.camera.position += 0.2f * g_tracer.camera.direction;
					g_tracer.clear();
				}
				else if (e.key.keysym.sym == SDLK_DOWN) {
					g_tracer.camera.position -= 0.2f * g_tracer.camera.direction;
					g_tracer.clear();
				}
				else if (e.key.keysym.sym == SDLK_LEFT) {
					g_tracer.camera.position -= 0.2f * g_tracer.camera.right;
					g_tracer.clear();
				}
				else if (e.key.keysym.sym == SDLK_RIGHT) {
					g_tracer.camera.position += 0.2f * g_tracer.camera.right;
					g_tracer.clear();
				}
				else if (e.key.keysym.sym == SDLK_PLUS) {
					exposure *= 1.5f;
				}
				else if (e.key.keysym.sym == SDLK_MINUS) {
					exposure /= 1.5f;
				}
				//g_tracer.scene.spheres[0].center = g_tracer.camera.position + Vec3(0, 0.5, 0);
				break;
				
            case SDL_WINDOWEVENT:
                if (e.window.windowID == SDL_GetWindowID(window)) {
                    switch (e.window.event) {
                        case SDL_WINDOWEVENT_SIZE_CHANGED:
                            g_tracer.resize(e.window.data1, e.window.data2);
                            SDL_DestroyTexture(framebuffer);
                            framebuffer = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, g_tracer.width, g_tracer.height);
                            if (!framebuffer) {
                                std::cerr << SDL_GetError();
                                return 1;
                            }
                            break;
                    }
                }
                break;

			case SDL_MOUSEBUTTONUP:
				if (e.button.which == 0) {
					Hit hit;
					if (g_tracer.scene.intersect(g_tracer.pixelToRay(e.button.x, e.button.y, std::tan(g_tracer.camera.horizontalFov / 2), prng), &hit)) {
						g_tracer.camera.focalLength = hit.distance;
					}
					else {
						g_tracer.camera.focalLength = 9999999;
					}
					g_tracer.clear();
				}
				break;

			case SDL_MOUSEWHEEL:
				g_tracer.camera.apertureSize = max(0.0f, g_tracer.camera.apertureSize + 0.01f * e.wheel.y);
				g_tracer.clear();
				break;

			case SDL_MOUSEMOTION:
				if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT)) {
					g_tracer.camera.pitch -= e.motion.yrel * 0.005f;
					g_tracer.camera.yaw += e.motion.xrel * 0.005f;
					g_tracer.clear();
				}
				break;
			}
		}
        else {
			auto start = std::chrono::high_resolution_clock::now();			
            g_tracer.sample();
			auto end = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
			if (duration == 0) duration = 1;
            updateScreen(renderer, framebuffer);
			std::stringstream sstr;
			long long rays = 0;
			for (int i = 0; i < numThreads; i++) {
				rays += thread_num_rays[i];
				thread_num_rays[i] = 0;
			}
			sstr << "Tracer | " << (rays / duration / 1000) << "MRays/s | " << duration << "ms/frame | " << g_tracer.width << "x" << g_tracer.height << " | " << numThreads << " Threads | " << g_tracer.numSamples << " samples | exposure: " << exposure;
			SDL_SetWindowTitle(window, sstr.str().c_str());
        }
	}

	SDL_DestroyTexture(framebuffer);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}