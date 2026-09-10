module;
#include <algorithm>
#include <cstdint>
export module Graphics.Scene.Models.Playback;
import Assets.Cache.Animations;
import Graphics.Scene.Models.ClipSampling;
import Graphics.Scene.Models.AnimationRotation;
import Graphics.Scene.Models.Hierarchy;
import Graphics.Scene.AffineTransform;

namespace Graphics {

// Frame-authored playback uses endpoint wrapping, including the original
// one-wrap limit for large elapsed times. It is distinct from time sampling.
export enum class ModelPlaybackMode {
    Manual, Loop, Once, PingPong, LoopBackwards, OnceBackwards
};

export class ModelPlayback final {
public:
    explicit ModelPlayback(Assets::AnimationCache& cache) : m_cache(cache) {}
    ModelPlayback(const ModelPlayback&) = delete;
    ModelPlayback& operator=(const ModelPlayback&) = delete;
    ~ModelPlayback() { Reset(); }

    void Reset() {
        m_cache.Release(m_first);
        m_cache.Release(m_second);
        m_first = {};
        m_second = {};
        m_blended = false;
    }

    void Set(Assets::AnimationAssetHandle clip, float frame,
        ModelPlaybackMode mode, std::uint32_t milliseconds) {
        // Replacement can be the last remaining reference to the same clip.
        const bool retained = m_cache.Retain(clip);
        Reset();
        if (!retained) return;
        m_first = clip;
        m_frame = frame;
        m_mode = mode;
        m_last_time = milliseconds;
        m_multiplier = 1;
        m_direction = mode >= ModelPlaybackMode::LoopBackwards ? -1.f : 1.f;
    }

    void Blend(Assets::AnimationAssetHandle first, float first_frame,
        Assets::AnimationAssetHandle second, float second_frame, float percentage) {
        const bool retained_first = m_cache.Retain(first);
        const bool retained_second = m_cache.Retain(second);
        Reset();
        m_first = retained_first ? first : Assets::AnimationAssetHandle{};
        m_second = retained_second ? second : Assets::AnimationAssetHandle{};
        m_frame = first_frame;
        m_second_frame = second_frame;
        m_percentage = percentage;
        m_blended = true;
    }

    bool Is_Blended() const noexcept { return m_blended; }
    bool Is_Advancing() const noexcept {
        return m_first && !m_blended && m_mode != ModelPlaybackMode::Manual;
    }
    Assets::AnimationAssetHandle Clip() const noexcept {
        return m_blended ? Assets::AnimationAssetHandle{} : m_first;
    }
    float Frame() const noexcept { return m_frame; }
    float Multiplier() const noexcept { return m_multiplier; }
    ModelPlaybackMode Mode() const noexcept { return m_mode; }
    void Set_Multiplier(float multiplier) noexcept { m_multiplier = multiplier; }

    float Current_Frame(std::uint32_t milliseconds, float* new_direction = nullptr) const {
        float direction = m_direction;
        float frame = 0;
        if (m_first && !m_blended) {
            frame = m_frame;
            if (m_mode != ModelPlaybackMode::Manual) {
                const auto* clip = m_cache.Resolve(m_first);
                const float elapsed = milliseconds - m_last_time;
                const float animation_ms = clip->frame_rate * m_multiplier * m_direction * elapsed;
                frame += animation_ms * .001f;
                const int last = static_cast<int>(clip->frame_count) - 1;
                switch (m_mode) {
                case ModelPlaybackMode::Once:
                    if (frame >= last) frame = static_cast<float>(last);
                    break;
                case ModelPlaybackMode::Loop:
                    if (frame >= last) {
                        frame -= last;
                        if (frame >= last) frame = 0;
                    }
                    break;
                case ModelPlaybackMode::OnceBackwards:
                    if (frame < 0) frame = 0;
                    break;
                case ModelPlaybackMode::LoopBackwards:
                    if (frame < 0) {
                        frame += last;
                        if (frame < 0) frame = static_cast<float>(last);
                    }
                    break;
                case ModelPlaybackMode::PingPong:
                    if (m_direction >= 1) {
                        if (frame >= last) {
                            frame = last * 2 - frame;
                            if (frame >= last - 1) frame = static_cast<float>(last);
                            direction = -m_direction;
                        }
                    } else if (frame < 0) {
                        frame = -frame;
                        if (frame >= last) frame = 0;
                        direction = -m_direction;
                    }
                    break;
                case ModelPlaybackMode::Manual: break;
                }
            }
        }
        if (new_direction) *new_direction = direction;
        return frame;
    }

    void Advance(std::uint32_t milliseconds) {
        if (!Is_Advancing()) return;
        m_frame = Current_Frame(milliseconds, &m_direction);
        m_last_time = milliseconds;
    }

    bool Is_Complete() const {
        if (!m_first || m_blended) return false;
        if (m_mode == ModelPlaybackMode::Once)
            return m_frame == static_cast<int>(m_cache.Resolve(m_first)->frame_count) - 1;
        return m_mode == ModelPlaybackMode::OnceBackwards && m_frame == 0;
    }

    void Evaluate(ModelHierarchy& hierarchy, const RenderTransform& root,
        std::uint32_t milliseconds) {
        if (!m_first && !m_blended) {
            hierarchy.Evaluate_Rest(root);
            return;
        }
        Advance(milliseconds);
        const auto* first = m_cache.Resolve(m_first);
        const auto* second = m_cache.Resolve(m_second);
        if (!first || (m_blended && !second)) return;
        const auto count = m_blended ? (std::min)(first->bone_count, second->bone_count) : first->bone_count;
        hierarchy.Evaluate(root, [&](int bone) {
            BoneMotion sample;
            if (static_cast<std::uint32_t>(bone) >= count) return sample;
            sample.translation = Sample_Clip_Translation(m_cache, m_first, bone, m_frame);
            sample.orientation = Sample_Clip_Rotation(m_cache, m_first, bone, m_frame);
            sample.visible = Sample_Clip_Visibility(m_cache, m_first, bone, m_frame);
            if (m_blended) {
                const auto translation = Sample_Clip_Translation(m_cache, m_second, bone, m_second_frame);
                const float first_weight = static_cast<float>(1.0 - m_percentage);
                for (unsigned axis = 0; axis < 3; ++axis)
                    sample.translation[axis] = first_weight * sample.translation[axis] + m_percentage * translation[axis];
                sample.orientation = Interpolate_Animation_Rotation(sample.orientation,
                    Sample_Clip_Rotation(m_cache, m_second, bone, m_second_frame), m_percentage);
                sample.visible = sample.visible || Sample_Clip_Visibility(m_cache, m_second, bone, m_second_frame);
            }
            sample.translate = sample.rotate = sample.set_visibility = true;
            return sample;
        });
    }

    bool Evaluate_Bone(const ModelHierarchy& hierarchy, int bone, float frame,
        const RenderTransform& root, RenderTransform& result) const {
        if (m_blended) { result = root; return false; }
        return hierarchy.Evaluate_Bone(bone, root, [&](int index) {
            return m_first ? Sample_Clip_Transform(m_cache, m_first, index, frame) : Affine_Identity();
        }, result);
    }

private:
    Assets::AnimationCache& m_cache;
    Assets::AnimationAssetHandle m_first{};
    Assets::AnimationAssetHandle m_second{};
    float m_frame = 0;
    float m_second_frame = 0;
    float m_percentage = 0;
    float m_multiplier = 1;
    float m_direction = 1;
    std::uint32_t m_last_time = 0;
    ModelPlaybackMode m_mode = ModelPlaybackMode::Manual;
    bool m_blended = false;
};
}
