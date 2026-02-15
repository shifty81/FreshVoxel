#pragma once

#include <memory>
#include <vector>

#include <glm/glm.hpp>

namespace fresh
{

class RenderTexture;
class RenderShader;
class IRenderContext;

/**
 * @brief Post-processing effect types
 */
enum class PostProcessEffect {
    None,
    FXAA,                // Fast Approximate Anti-Aliasing
    Bloom,               // Bloom/glow effect
    ToneMapping,         // HDR to LDR tone mapping
    ColorGrading,        // Color adjustment
    Vignette,            // Dark corners effect
    ChromaticAberration, // Color separation
    DepthOfField,        // Blur based on depth
    MotionBlur,          // Motion-based blur
    SSAO,                // Screen-Space Ambient Occlusion
    SSR                  // Screen-Space Reflections
};

/**
 * @brief Tone mapping operator
 */
enum class ToneMappingOperator {
    None,             // No tone mapping
    Reinhard,         // Simple Reinhard
    ReinhardExtended, // Extended Reinhard
    Uncharted2,       // Uncharted 2 filmic
    ACES,             // ACES filmic
    ACESApprox        // ACES approximation (faster)
};

/**
 * @brief Post-processing settings
 */
struct PostProcessSettings {
    // FXAA
    bool enableFXAA = true;
    float fxaaQuality = 0.75f; // 0.0 - 1.0

    // Bloom
    bool enableBloom = true;
    float bloomThreshold = 1.0f;
    float bloomIntensity = 0.5f;
    float bloomRadius = 1.0f;
    int bloomIterations = 5;

    // Tone Mapping
    bool enableToneMapping = true;
    ToneMappingOperator toneMappingOperator = ToneMappingOperator::ACESApprox;
    float exposure = 1.0f;
    float gamma = 2.2f;

    // Color Grading
    bool enableColorGrading = false;
    float saturation = 1.0f;
    float contrast = 1.0f;
    float brightness = 1.0f;
    glm::vec3 colorFilter{1.0f, 1.0f, 1.0f};

    // Vignette
    bool enableVignette = false;
    float vignetteIntensity = 0.3f;
    float vignetteSmoothness = 0.5f;

    // Chromatic Aberration
    bool enableChromaticAberration = false;
    float chromaticAberrationIntensity = 0.5f;

    // Depth of Field
    bool enableDepthOfField = false;
    float dofFocusDistance = 10.0f;
    float dofFocusRange = 5.0f;
    float dofBokehRadius = 4.0f;

    // Motion Blur
    bool enableMotionBlur = false;
    float motionBlurIntensity = 0.5f;
    int motionBlurSamples = 8;

    // SSAO
    bool enableSSAO = false;
    float ssaoRadius = 0.5f;
    float ssaoBias = 0.025f;
    int ssaoSamples = 16;

    // SSR
    bool enableSSR = false;
    float ssrMaxDistance = 50.0f;
    float ssrStride = 2.0f;
    int ssrSteps = 32;
};

/**
 * @brief Post-processing pipeline
 *
 * Manages and applies post-processing effects to rendered frames.
 * Supports HDR, anti-aliasing, bloom, tone mapping, and more.
 */
class PostProcessing
{
public:
    PostProcessing();
    ~PostProcessing();

    /**
     * @brief Initialize post-processing system
     * @param renderContext Render context for creating resources
     * @param width Framebuffer width
     * @param height Framebuffer height
     * @return true if successful
     */
    bool initialize(IRenderContext* renderContext, int width, int height);

    /**
     * @brief Shutdown and cleanup
     */
    void shutdown();

    /**
     * @brief Resize framebuffers
     * @param width New width
     * @param height New height
     */
    void resize(int width, int height);

    /**
     * @brief Apply post-processing effects
     * @param inputTexture Input HDR texture
     * @param outputTexture Output LDR texture
     */
    void apply(RenderTexture* inputTexture, RenderTexture* outputTexture);

    /**
     * @brief Get/Set post-processing settings
     */
    PostProcessSettings& getSettings()
    {
        return settings;
    }
    const PostProcessSettings& getSettings() const
    {
        return settings;
    }
    void setSettings(const PostProcessSettings& s)
    {
        settings = s;
    }

    /**
     * @brief Enable/disable specific effect
     */
    void setEffectEnabled(PostProcessEffect effect, bool enabled);
    bool isEffectEnabled(PostProcessEffect effect) const;

    /**
     * @brief Get intermediate render targets (for debugging)
     */
    RenderTexture* getBloomTexture() const
    {
        return bloomTexture.get();
    }
    RenderTexture* getSSAOTexture() const
    {
        return ssaoTexture.get();
    }

private:
    void createRenderTargets();
    void createShaders();

    // Effect passes
    void applyFXAA(RenderTexture* input, RenderTexture* output);
    void applyBloom(RenderTexture* input, RenderTexture* output);
    void applyToneMapping(RenderTexture* input, RenderTexture* output);
    void applyColorGrading(RenderTexture* input, RenderTexture* output);
    void applyVignette(RenderTexture* input, RenderTexture* output);
    void applyChromaticAberration(RenderTexture* input, RenderTexture* output);
    void applyDepthOfField(RenderTexture* input, RenderTexture* depth, RenderTexture* output);
    void applyMotionBlur(RenderTexture* input, RenderTexture* velocity, RenderTexture* output);
    void applySSAO(RenderTexture* depth, RenderTexture* normal, RenderTexture* output);
    void applySSR(RenderTexture* input, RenderTexture* depth, RenderTexture* normal,
                  RenderTexture* output);

    // Utility
    void blitTexture(RenderTexture* src, RenderTexture* dst);
    void gaussianBlur(RenderTexture* input, RenderTexture* output, int iterations);

    IRenderContext* renderContext = nullptr;
    PostProcessSettings settings;

    int width = 0;
    int height = 0;

    // Render targets
    std::shared_ptr<RenderTexture> hdrBuffer;
    std::shared_ptr<RenderTexture> bloomTexture;
    std::shared_ptr<RenderTexture> ssaoTexture;
    std::shared_ptr<RenderTexture> ssrTexture;
    std::vector<std::shared_ptr<RenderTexture>> tempBuffers;

    // Shaders
    std::shared_ptr<RenderShader> fxaaShader;
    std::shared_ptr<RenderShader> bloomShader;
    std::shared_ptr<RenderShader> toneMappingShader;
    std::shared_ptr<RenderShader> colorGradingShader;
    std::shared_ptr<RenderShader> vignetteShader;
    std::shared_ptr<RenderShader> chromaticAberrationShader;
    std::shared_ptr<RenderShader> dofShader;
    std::shared_ptr<RenderShader> motionBlurShader;
    std::shared_ptr<RenderShader> ssaoShader;
    std::shared_ptr<RenderShader> ssrShader;
    std::shared_ptr<RenderShader> blitShader;
    std::shared_ptr<RenderShader> gaussianBlurShader;
};

} // namespace fresh
