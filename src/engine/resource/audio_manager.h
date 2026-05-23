#pragma once
#include <memory>
#include <string>
#include <unordered_map>

#include <SDL3_mixer/SDL_mixer.h>

namespace engine::resource
{
    using std::string;
    /**
     * @brief 管理 MIX_Audio 资源的加载,存储和检索
     *
     * 在构造时初始化,使用文件路径作为key,确保纹理只加载一次并正确释放
     */
    class AudioManager
    {
        // 友元, private的接口只会暴露给它
        friend class ResourceManager;

    private:
        // SDL_Texture 的删除器函数对象,用于智能指针管理
        struct SDLMixAudioDeleter
        {
            void operator()(MIX_Audio *audio) const
            {
                if (audio)
                {
                    MIX_DestroyAudio(audio);
                }
            }
        };

        struct SDLMixTrackDeleter
        {
            void operator()(MIX_Track *track) const
            {
                if (track)
                {
                    MIX_DestroyTrack(track);
                }
            }
        };
        std::unordered_map<string, std::unique_ptr<MIX_Audio, SDLMixAudioDeleter>> long_audios_;
        std::unordered_map<string, std::unique_ptr<MIX_Audio, SDLMixAudioDeleter>> short_audios_;
        MIX_Mixer *mixer_;

    public:
        explicit AudioManager();
        ~AudioManager();

        AudioManager(const AudioManager &) = delete;
        AudioManager &operator=(const AudioManager &) = delete;
        AudioManager(const AudioManager &&) = delete;
        AudioManager &operator=(const AudioManager &&) = delete;

    private:
        // 短音效适合预解码,长音效不适合
        MIX_Audio *loadLongAudio(const string &file_path);
        MIX_Audio *loadShortAudio(const string &file_path);
        MIX_Audio *getLongAudio(const string &file_path);
        MIX_Audio *getShortAudio(const string &file_path);
        void unloadLongAudio(const string &file_path);
        void unloadShortAudio(const string &file_path);
        void clearLongAudios();
        void clearShortAudios();
        void clearAudios();
    };
}