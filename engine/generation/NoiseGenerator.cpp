#include "generation/NoiseGenerator.h"

#include <algorithm>
#include <cmath>
#include <random>

namespace fresh
{

NoiseGenerator::NoiseGenerator() : m_seed(0)
{
    setSeed(0);
}

NoiseGenerator::NoiseGenerator(int seed) : m_seed(seed)
{
    setSeed(seed);
}

NoiseGenerator::~NoiseGenerator() {}

void NoiseGenerator::setSeed(int seed)
{
    m_seed = seed;

    // Initialize permutation table
    m_permutation.resize(512);

    // Fill with values 0-255
    std::vector<int> p(256);
    for (int i = 0; i < 256; ++i) {
        p[i] = i;
    }

    // Shuffle using seed
    std::mt19937 rng(seed);
    std::shuffle(p.begin(), p.end(), rng);

    // Duplicate
    for (int i = 0; i < 256; ++i) {
        m_permutation[i] = p[i];
        m_permutation[256 + i] = p[i];
    }
}

// fade() and lerp() are now inline in the header for better performance

float NoiseGenerator::grad(int hash, float x, float y) const noexcept
{
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float NoiseGenerator::grad(int hash, float x, float y, float z) const noexcept
{
    int h = hash & 15;
    float u = h < 8 ? x : y;
    float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
    return ((h & 1) ? -u : u) + ((h & 2) ? -v : v);
}

float NoiseGenerator::perlin2D(float x, float y) const
{
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;

    x -= std::floor(x);
    y -= std::floor(y);

    float u = fade(x);
    float v = fade(y);

    int A = m_permutation[X] + Y;
    int B = m_permutation[X + 1] + Y;

    return lerp(
        v, lerp(u, grad(m_permutation[A], x, y), grad(m_permutation[B], x - 1, y)),
        lerp(u, grad(m_permutation[A + 1], x, y - 1), grad(m_permutation[B + 1], x - 1, y - 1)));
}

float NoiseGenerator::perlin3D(float x, float y, float z) const
{
    int X = static_cast<int>(std::floor(x)) & 255;
    int Y = static_cast<int>(std::floor(y)) & 255;
    int Z = static_cast<int>(std::floor(z)) & 255;

    x -= std::floor(x);
    y -= std::floor(y);
    z -= std::floor(z);

    float u = fade(x);
    float v = fade(y);
    float w = fade(z);

    int A = m_permutation[X] + Y;
    int AA = m_permutation[A] + Z;
    int AB = m_permutation[A + 1] + Z;
    int B = m_permutation[X + 1] + Y;
    int BA = m_permutation[B] + Z;
    int BB = m_permutation[B + 1] + Z;

    return lerp(
        w,
        lerp(v, lerp(u, grad(m_permutation[AA], x, y, z), grad(m_permutation[BA], x - 1, y, z)),
             lerp(u, grad(m_permutation[AB], x, y - 1, z),
                  grad(m_permutation[BB], x - 1, y - 1, z))),
        lerp(v,
             lerp(u, grad(m_permutation[AA + 1], x, y, z - 1),
                  grad(m_permutation[BA + 1], x - 1, y, z - 1)),
             lerp(u, grad(m_permutation[AB + 1], x, y - 1, z - 1),
                  grad(m_permutation[BB + 1], x - 1, y - 1, z - 1))));
}

float NoiseGenerator::fractalNoise2D(float x, float y, int octaves, float persistence,
                                     float lacunarity) const
{
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += perlin2D(x * frequency, y * frequency) * amplitude;

        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxValue;
}

float NoiseGenerator::fractalNoise3D(float x, float y, float z, int octaves, float persistence,
                                     float lacunarity) const
{
    float total = 0.0f;
    float frequency = 1.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        total += perlin3D(x * frequency, y * frequency, z * frequency) * amplitude;

        maxValue += amplitude;
        amplitude *= persistence;
        frequency *= lacunarity;
    }

    return total / maxValue;
}

} // namespace fresh
