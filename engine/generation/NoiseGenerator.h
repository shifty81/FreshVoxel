#pragma once

#include <vector>

namespace fresh
{

/**
 * @brief Noise generation using Perlin and Simplex algorithms
 *
 * Provides various noise functions for procedural generation.
 */
class NoiseGenerator
{
public:
    NoiseGenerator();
    explicit NoiseGenerator(int seed);
    ~NoiseGenerator();

    /**
     * @brief Set random seed
     * @param seed Random seed value
     */
    void setSeed(int seed);

    /**
     * @brief Generate 2D Perlin noise
     * @param x X coordinate
     * @param y Y coordinate
     * @return Noise value in range [-1, 1]
     */
    [[nodiscard]] float perlin2D(float x, float y) const;

    /**
     * @brief Generate 3D Perlin noise
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @return Noise value in range [-1, 1]
     */
    [[nodiscard]] float perlin3D(float x, float y, float z) const;

    /**
     * @brief Generate fractal noise (multiple octaves)
     * @param x X coordinate
     * @param y Y coordinate
     * @param octaves Number of octaves
     * @param persistence Amplitude multiplier per octave
     * @param lacunarity Frequency multiplier per octave
     * @return Noise value
     */
    [[nodiscard]] float fractalNoise2D(float x, float y, int octaves = 4, float persistence = 0.5f,
                                       float lacunarity = 2.0f) const;

    /**
     * @brief Generate 3D fractal noise
     */
    [[nodiscard]] float fractalNoise3D(float x, float y, float z, int octaves = 4,
                                       float persistence = 0.5f, float lacunarity = 2.0f) const;

private:
    // Inline helper functions for better performance in hot paths
    inline float fade(float t) const noexcept
    {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }

    inline float lerp(float t, float a, float b) const noexcept
    {
        return a + t * (b - a);
    }

    float grad(int hash, float x, float y) const noexcept;
    float grad(int hash, float x, float y, float z) const noexcept;

private:
    std::vector<int> m_permutation;
    int m_seed;
};

} // namespace fresh
