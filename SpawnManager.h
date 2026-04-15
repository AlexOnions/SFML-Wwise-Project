#pragma once
#include <vector>
#include <SFML/Graphics.hpp>
#include "Obstacle.h"
#include "Platform.h"

class SpawnManager
{
public:
    SpawnManager(float obstacleSpeed, float platformSpeed, uint64_t playerID);

    void update(float deltaTime, float gameSpeed,
        std::vector<Obstacle>& obstaclePool,
        std::vector<Platform>& platforms,
        float floorY);

private:


    //Checks the positions of all obstacles and platforms, and checkcs new bounds (Obstacles) are too close to tehm
    bool isTooClose(sf::FloatRect newBounds,
        const std::vector<Obstacle>& obstaclePool,
        const std::vector<Platform>& platforms) const;

    //Finds the top of nearby platforms for obstacle placement
    float findNearbyPlatformTop(const std::vector<Platform>& platforms) const;

    float m_obstacleSpeed;
    float m_platformSpeed;
    float m_obstacleTimer;
    float m_platformTimer;
    uint64_t m_playerID;

    const float m_spawnX = 900.f;
    const float m_obstacleSpawnTime = 2.0f;
    const float m_platformSpawnTime = 3.0f;
    const float m_minSpacing = 150.f;
    int obstacleID = 0;
};
