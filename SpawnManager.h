#pragma once
#include <vector>
#include "Obstacle.h"
#include "Platform.h"

struct SpawnEntry
{
    sf::FloatRect bounds;
    bool isOnPlatform;
};

class SpawnManager
{
public:
    SpawnManager(float obstacleSpeed, float platformSpeed);

    void update(float deltaTime, float gameSpeed,
        std::vector<Obstacle>& obstacles,
        std::vector<Platform>& platforms, 
        float floorY);

private:
    int obstacleID = 1000;
    float m_obstacleSpeed;
    float m_platformSpeed;

    float m_obstacleTimer;
    float m_platformTimer;

    const float m_obstacleSpawnTime = 2.0f;
    const float m_platformSpawnTime = 3.0f;
    const float m_minSpacing = 80.f;   // min horizontal gap between any two spawns
    const float m_spawnX = 900.f;

    bool isTooClose(sf::FloatRect newBounds,
        const std::vector<Obstacle>& obstacles,
        const std::vector<Platform>& platforms) const;

    // Returns the top-Y of a platform near spawnX, or -1 if none found
    float findNearbyPlatformTop(const std::vector<Platform>& platforms) const;
};