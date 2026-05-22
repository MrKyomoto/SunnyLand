#include <iostream>
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <SDL3_mixer/SDL_mixer.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <glm/glm.hpp>
#include <spdlog/spdlog.h>

int main(int, char **)
{

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("SunnyLand", 800, 600, 0);

    SDL_Renderer *renderer = SDL_CreateRenderer(window, NULL);

    // NOTE: SDL3 Image dont need to init
    SDL_Texture *texture = IMG_LoadTexture(renderer, "assets/textures/UI/Heart-bg.png");

    if (!MIX_Init())
    {
        std::cerr << "MIX_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }
    MIX_Mixer *mixer = MIX_CreateMixerDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);
    MIX_Audio *music = MIX_LoadAudio(mixer, "assets/audio/punch2a.mp3", true);
    MIX_Track *track = MIX_CreateTrack(mixer);
    MIX_SetTrackAudio(track, music);
    SDL_PropertiesID prop = SDL_CreateProperties();
    SDL_SetNumberProperty(prop, MIX_PROP_PLAY_LOOPS_NUMBER, -1);

    MIX_PlayTrack(track, prop);
    SDL_DestroyProperties(prop);

    if (!TTF_Init())
    {
        std::cerr << "TTF_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    TTF_Font *font = TTF_OpenFont("assets/fonts/VonwaonBitmap-16px.ttf", 24);
    SDL_Color color = {255, 255, 255};
    SDL_Surface *surface = TTF_RenderText_Solid(font, "Hello SDL, 中文也可以", 0, color);

    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, surface);

    while (true)
    {
        SDL_Event event;
        if (SDL_PollEvent(&event))
        {
            if (event.type == SDL_EVENT_QUIT)
            {
                break;
            }
        }

        SDL_RenderClear(renderer);

        SDL_FRect rect = {100, 100, 200, 200};
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        SDL_RenderFillRect(renderer, &rect);

        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

        SDL_FRect dstrect = {200, 200, 300, 300};
        SDL_RenderTexture(renderer, texture, NULL, &dstrect);

        SDL_FRect textDstrect = {400, 400, static_cast<float>(surface->w), static_cast<float>(surface->h)};
        SDL_RenderTexture(renderer, textTexture, NULL, &textDstrect);

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);

    MIX_DestroyAudio(music);
    MIX_DestroyTrack(track);
    MIX_DestroyMixer(mixer);
    MIX_Quit();

    SDL_DestroySurface(surface);
    SDL_DestroyTexture(textTexture);
    TTF_CloseFont(font);
    TTF_Quit();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}