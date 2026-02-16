#include "ecs/Animation2DComponent.h"

namespace fresh
{
namespace ecs
{

Animation2DComponent::Animation2DComponent()
    : currentFrame(0)
    , frameTimer(0.0f)
    , playing(false)
    , paused(false)
    , playbackSpeed(1.0f)
{
}

void Animation2DComponent::addClip(const std::string& name, const AnimationClip& clip)
{
    clips[name] = clip;
}

void Animation2DComponent::addClipFromRange(const std::string& name, int startColumn, int startRow,
                                           int frameCount, int columnsPerRow, float frameRate, bool loop)
{
    AnimationClip clip;
    clip.name = name;
    clip.frameRate = frameRate;
    clip.loop = loop;
    
    // Generate frame sequence
    int column = startColumn;
    int row = startRow;
    
    for (int i = 0; i < frameCount; i++)
    {
        clip.frames.push_back(glm::ivec2(column, row));
        
        column++;
        if (column >= columnsPerRow)
        {
            column = 0;
            row++;
        }
    }
    
    clips[name] = clip;
}

void Animation2DComponent::play(const std::string& clipName)
{
    auto it = clips.find(clipName);
    if (it == clips.end())
        return;
        
    currentClip = clipName;
    currentFrame = 0;
    frameTimer = 0.0f;
    playing = true;
    paused = false;
}

void Animation2DComponent::stop()
{
    playing = false;
    paused = false;
    currentFrame = 0;
    frameTimer = 0.0f;
}

void Animation2DComponent::pause()
{
    paused = true;
}

void Animation2DComponent::resume()
{
    paused = false;
}

glm::ivec2 Animation2DComponent::getCurrentFrameCoords() const
{
    auto it = clips.find(currentClip);
    if (it == clips.end() || it->second.frames.empty())
        return glm::ivec2(0, 0);
        
    if (currentFrame < 0 || currentFrame >= static_cast<int>(it->second.frames.size()))
        return it->second.frames[0];
        
    return it->second.frames[currentFrame];
}

bool Animation2DComponent::hasFinished() const
{
    auto it = clips.find(currentClip);
    if (it == clips.end())
        return true;
        
    const AnimationClip& clip = it->second;
    
    // Looping animations never finish
    if (clip.loop)
        return false;
        
    // Non-looping: finished if on last frame
    return currentFrame >= static_cast<int>(clip.frames.size()) - 1;
}

void Animation2DComponent::reset()
{
    currentFrame = 0;
    frameTimer = 0.0f;
}

} // namespace ecs
} // namespace fresh
