#include <gtest/gtest.h>

#include "galaxy/GalaxyGenerator.h"
#include "galaxy/GalaxySector.h"

using namespace fresh::galaxy;

/**
 * Test enhanced galaxy generation features
 */
class GalaxyGeneratorEnhancedTest : public ::testing::Test
{
protected:
    GalaxyGenerator generator;

    void SetUp() override { generator.setSeed(12345); }
};

// Test asteroid count increase (1.5x)
TEST_F(GalaxyGeneratorEnhancedTest, AsteroidDensityIncrease)
{
    auto sector = generator.generateSector(0, 0);
    const auto& asteroids = sector->getAsteroids();

    // With increased density (15.0 vs 10.0), we expect more asteroids
    // Minimum expected: 7 asteroids (1.5x the minimum with lower bound)
    EXPECT_GE(asteroids.size(), 7)
        << "Asteroid count should reflect 1.5x density increase";
}

// Test asteroid shape variety
TEST_F(GalaxyGeneratorEnhancedTest, AsteroidShapeVariety)
{
    auto sector = generator.generateSector(0, 0);
    const auto& asteroids = sector->getAsteroids();

    ASSERT_GT(asteroids.size(), 0) << "Should have generated asteroids";

    // Verify each asteroid has a shape assigned
    for (const auto& asteroid : asteroids) {
        // Shape should be one of the valid enum values (0-4)
        int shapeValue = static_cast<int>(asteroid.shape);
        EXPECT_GE(shapeValue, 0);
        EXPECT_LE(shapeValue, 4);
    }
}

// Test asteroid stretch factors
TEST_F(GalaxyGeneratorEnhancedTest, AsteroidStretchFactors)
{
    auto sector = generator.generateSector(0, 0);
    const auto& asteroids = sector->getAsteroids();

    ASSERT_GT(asteroids.size(), 0) << "Should have generated asteroids";

    const float MAX_STRETCH_RATIO = 5.0f;

    for (const auto& asteroid : asteroids) {
        // Each dimension of stretch factor should be within limits
        EXPECT_GT(asteroid.stretchFactor.x, 0.0f);
        EXPECT_GT(asteroid.stretchFactor.y, 0.0f);
        EXPECT_GT(asteroid.stretchFactor.z, 0.0f);

        EXPECT_LE(asteroid.stretchFactor.x, MAX_STRETCH_RATIO);
        EXPECT_LE(asteroid.stretchFactor.y, MAX_STRETCH_RATIO);
        EXPECT_LE(asteroid.stretchFactor.z, MAX_STRETCH_RATIO);
    }
}

// Test station size multiplier (4-5x)
TEST_F(GalaxyGeneratorEnhancedTest, StationSizeMultiplier)
{
    // Generate multiple sectors to ensure we get at least one station
    for (int i = 0; i < 20; ++i) {
        auto sector = generator.generateSector(i, i);
        const auto& stations = sector->getStations();

        if (!stations.empty()) {
            for (const auto& station : stations) {
                // Size multiplier should be between 4.0 and 5.0
                EXPECT_GE(station.sizeMultiplier, 4.0f);
                EXPECT_LE(station.sizeMultiplier, 5.0f);
            }
            return; // Found at least one station, test passed
        }
    }
}

// Test station docking arms
TEST_F(GalaxyGeneratorEnhancedTest, StationDockingArms)
{
    // Generate multiple sectors to ensure we get at least one station
    for (int i = 0; i < 20; ++i) {
        auto sector = generator.generateSector(i, i);
        const auto& stations = sector->getStations();

        if (!stations.empty()) {
            for (const auto& station : stations) {
                // Each station should have 2-6 docking arms
                EXPECT_GE(station.dockingArms.size(), 2);
                EXPECT_LE(station.dockingArms.size(), 6);

                // Verify docking arm properties
                for (const auto& arm : station.dockingArms) {
                    EXPECT_GT(arm.length, 0.0f);
                    EXPECT_GE(arm.dockingBays, 1);
                    EXPECT_LE(arm.dockingBays, 4);
                }
            }
            return; // Found at least one station, test passed
        }
    }
}

// Test station services
TEST_F(GalaxyGeneratorEnhancedTest, StationServices)
{
    // Generate multiple sectors to ensure we get at least one station
    for (int i = 0; i < 20; ++i) {
        auto sector = generator.generateSector(i, i);
        const auto& stations = sector->getStations();

        if (!stations.empty()) {
            for (const auto& station : stations) {
                // All stations should offer at least basic services (Refuel, Information)
                EXPECT_GE(station.services.size(), 2);

                // Verify services are within valid range
                for (const auto& service : station.services) {
                    int serviceValue = static_cast<int>(service);
                    EXPECT_GE(serviceValue, 0);
                    EXPECT_LE(serviceValue, 7); // 8 service types (0-7)
                }
            }
            return; // Found at least one station, test passed
        }
    }
}

// Test station commodities
TEST_F(GalaxyGeneratorEnhancedTest, StationCommodities)
{
    // Generate multiple sectors to ensure we get at least one station
    for (int i = 0; i < 20; ++i) {
        auto sector = generator.generateSector(i, i);
        const auto& stations = sector->getStations();

        if (!stations.empty()) {
            for (const auto& station : stations) {
                // Stations should have at least basic commodities
                EXPECT_GE(station.commodities.size(), 3);

                // Verify commodities are within valid range
                for (const auto& commodity : station.commodities) {
                    int commodityValue = static_cast<int>(commodity);
                    EXPECT_GE(commodityValue, 0);
                    EXPECT_LE(commodityValue, 8); // 9 commodity types (0-8)
                }
            }
            return; // Found at least one station, test passed
        }
    }
}

// Test captain availability at stations
TEST_F(GalaxyGeneratorEnhancedTest, StationCaptains)
{
    // Generate multiple sectors to ensure we get at least one station with captains
    for (int i = 0; i < 30; ++i) {
        auto sector = generator.generateSector(i, i);
        const auto& stations = sector->getStations();

        for (const auto& station : stations) {
            // Only stations with HireCaptain service should have captains
            bool hasHireService = false;
            for (const auto& service : station.services) {
                if (service == StationService::HireCaptain) {
                    hasHireService = true;
                    break;
                }
            }

            if (hasHireService) {
                EXPECT_GE(station.availableCaptains.size(), 1);
                EXPECT_LE(station.availableCaptains.size(), 4);

                // Verify captain properties
                for (const auto& captain : station.availableCaptains) {
                    EXPECT_FALSE(captain.name.empty());
                    EXPECT_GE(captain.skillLevel, 1);
                    EXPECT_LE(captain.skillLevel, 10);
                    EXPECT_GT(captain.hiringCost, 0);
                    EXPECT_FALSE(captain.specialty.empty());
                }
                return; // Found station with captains, test passed
            }
        }
    }
}

// Test deterministic generation (same seed = same result)
TEST_F(GalaxyGeneratorEnhancedTest, DeterministicGeneration)
{
    generator.setSeed(42);
    auto sector1 = generator.generateSector(5, 5);

    generator.setSeed(42);
    auto sector2 = generator.generateSector(5, 5);

    // Same seed should produce same number of asteroids
    EXPECT_EQ(sector1->getAsteroids().size(), sector2->getAsteroids().size());

    // Same seed should produce same number of stations
    EXPECT_EQ(sector1->getStations().size(), sector2->getStations().size());

    // Same seed should produce same number of ships
    EXPECT_EQ(sector1->getShips().size(), sector2->getShips().size());
}
