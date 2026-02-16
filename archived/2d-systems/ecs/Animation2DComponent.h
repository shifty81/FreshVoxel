#pragma once
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include "ecs/IComponent.h"

namespace fresh
{
namespace ecs
{

/**
 * @brief Component for 2D sprite animation
 *
 * Manages sprite sheet animations with multiple animation clips,
 * frame timing, and playback control.
 */
class Animation2DComponent : public IComponent
{
public:
    /**
     * @brief Animation clip data
     */
    struct AnimationClip
    {
        std::string name;               ///< Animation name
        std::vector<glm::ivec2> frames; ///< Frame indices in sprite sheet
        float frameRate;                ///< Frames per second
        bool loop;                      ///< Should animation loop?
        
        AnimationClip()
            : frameRate(10.0f), loop(true) {}
    };

    Animation2DComponent();
    virtual ~Animation2DComponent() = default;

    COMPONENT_TYPE(Animation2DComponent)

    // Animation clips
    std::unordered_map<std::string, AnimationClip> clips;
    
    // Playback state
    std::string currentClip;        ///< Currently playing animation
    int currentFrame;               ///< Current frame index in clip
    float frameTimer;               ///< Time accumulator for frame progression
    bool playing;                   ///< Is animation playing?
    bool paused;                    ///< Is animation paused?
    float playbackSpeed;            ///< Speed multiplier (1.0 = normal)

    /**
     * @brief Add an animation clip
     */
    void addClip(const std::string& name, const AnimationClip& clip);

    /**
     * @brief Add a simple animation clip from a frame range
     */
    void addClipFromRange(const std::string& name, int startColumn, int startRow, 
                         int frameCount, int columnsPerRow, float frameRate, bool loop = true);

    /**
     * @brief Play an animation clip
     */
    void play(const std::string& clipName);

    /**
     * @brief Stop current animation
     */
    void stop();

    /**
     * @brief Pause current animation
     */
    void pause();

    /**
     * @brief Resume paused animation
     */
    void resume();

    /**
     * @brief Get current frame coordinates for sprite
     */
    glm::ivec2 getCurrentFrameCoords() const;

    /**
     * @brief Check if current animation has finished (non-looping)
     */
    bool hasFinished() const;

    /**
     * @brief Reset current animation to first frame
     */
    void reset();
};

} // namespace ecs
} // namespace fresh
