#pragma once
#include <string>
#include <vector>

struct MIX_Mixer;
struct MIX_Track;

namespace engine::resource {
class ResourceManager;
}

namespace engine::audio {

/**
 * @brief 引擎层音频播放器。
 *
 * 封装 SDL3_mixer 的 MIX_Track 模型：
 *   - 短音效 (SFX)：维护一个 track 池，支持多路同时播放
 *   - 背景音乐 (Music)：一个专用 track，同一时间只播一首
 *   播放参数通过 SDL_PropertiesID 传递（loop、fade 等）。
 */
class AudioPlayer final {
private:
    engine::resource::ResourceManager *resource_manager_;
    MIX_Mixer *mixer_;

    // SFX track 池 —— 预创建，每次 playSound 找一个空闲的复用
    static constexpr int kMaxSfxChannels = 16;
    std::vector<MIX_Track *> sfx_tracks_;

    // 音乐专用 track
    MIX_Track *music_track_ = nullptr;
    std::string current_music_;

    float sfx_gain_ = 1.0f;
    float music_gain_ = 1.0f;

    int findFreeSfxTrack();

public:
    explicit AudioPlayer(engine::resource::ResourceManager *resource_manager,
                         MIX_Mixer *mixer);
    ~AudioPlayer();

    AudioPlayer(const AudioPlayer &) = delete;
    AudioPlayer &operator=(const AudioPlayer &) = delete;
    AudioPlayer(AudioPlayer &&) = delete;
    AudioPlayer &operator=(AudioPlayer &&) = delete;

    // --- SFX ---
    /// 播放短音效一次，返回使用的 track 索引，-1 表示失败
    int playSound(const std::string &sound_path, int channel = -1);

    // --- Music ---
    bool playMusic(const std::string &music_path, int loops = -1,
                   int fade_in_ms = 0);
    void stopMusic(int fade_out_ms = 0);
    void pauseMusic();
    void resumeMusic();

    // --- Volume (0.0 ~ 1.0) ---
    void setSoundVolume(float volume);
    void setMusicVolume(float volume);
    float getSoundVolume() const;
    float getMusicVolume() const;
};

} // namespace engine::audio