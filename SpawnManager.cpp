#include "SpawnManager.h"
#include <cstdlib>

SpawnManager::SpawnManager(float obstacleSpeed, float platformSpeed)
    : m_obstacleSpeed(obstacleSpeed)
    , m_platformSpeed(platformSpeed)
    , m_obstacleTimer(-1.5f)   // matches your original delay
    , m_platformTimer(0.f)
{
}

void SpawnManager::update(float deltaTime, float gameSpeed,
    std::vector<Obstacle>& obstacles,
    std::vector<Platform>& platforms)
{
    m_obstacleTimer += deltaTime * gameSpeed;
    m_platformTimer += deltaTime * gameSpeed;

    // --- SPAWN PLATFORM ---
    if (m_platformTimer > m_platformSpawnTime)
    {
        m_platformTimer = 0;

        Platform candidate(m_spawnX);
        sf::FloatRect candidateBounds = candidate.getBounds();

        if (!isTooClose(candidateBounds, obstacles, platforms))
            platforms.push_back(std::move(candidate));
    }
    if (gameSpeed > 1.15f) {
        // --- SPAWN OBSTACLE ---
        if (m_obstacleTimer > m_obstacleSpawnTime)
        {
            m_obstacleTimer = 0;
            obstacleID++;
            // 30% chance to place obstacle on top of a nearby platform
            float platTop = findNearbyPlatformTop(platforms);
            bool onPlatform = (platTop > 0.f) && (rand() % 10 < 3);

            Obstacle candidate(m_spawnX, onPlatform ? platTop : -1.f, gameSpeed, obstacleID);
            sf::FloatRect candidateBounds = candidate.getBounds();

            if (!isTooClose(candidateBounds, obstacles, platforms))
                obstacles.push_back(std::move(candidate));
        }
    }
}

bool SpawnManager::isTooClose(sf::FloatRect newBounds,
    const std::vector<Obstacle>& obstacles,
    const std::vector<Platform>& platforms) const
{
    for (const auto& o : obstacles)
    {
        sf::FloatRect ob = o.getBounds();
        float gap = newBounds.position.x - (ob.position.x + ob.size.x);
        if (std::abs(gap) < m_minSpacing)
            return true;
    }
    for (const auto& p : platforms)
    {
        sf::FloatRect pb = p.getBounds();
        float gap = newBounds.position.x - (pb.position.x + pb.size.x);
        if (std::abs(gap) < m_minSpacing)
            return true;
    }
    return false;
}

float SpawnManager::findNearbyPlatformTop(const std::vector<Platform>& platforms) const
{
    // Look for a platform whose right edge is near the spawn point
    const float searchRange = 200.f;
    for (const auto& p : platforms)
    {
        sf::FloatRect pb = p.getBounds();
        float rightEdge = pb.position.x + pb.size.x;
        if (rightEdge > m_spawnX - searchRange && rightEdge < m_spawnX + searchRange)
            return pb.position.y; // top of that platform
    }
    return -1.f;
}