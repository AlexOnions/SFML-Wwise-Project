#include "SpawnManager.h"
#include <cstdlib>
#include <iostream>

SpawnManager::SpawnManager(float obstacleSpeed, float platformSpeed, uint64_t playerID)
    : m_obstacleSpeed(obstacleSpeed)
    , m_platformSpeed(platformSpeed)
    , m_obstacleTimer(-1.5f)
    , m_platformTimer(0.f)
    , m_playerID(playerID)
{
}

void SpawnManager::update(float deltaTime, float gameSpeed,
    std::vector<Obstacle>& obstaclePool,
    std::vector<Platform>& platforms,
    float floorY)
{
    m_obstacleTimer += deltaTime * gameSpeed;
    m_platformTimer += deltaTime * gameSpeed;

    // --- SPAWN PLATFORM ---
    if (m_platformTimer > m_platformSpawnTime)
    {
        m_platformTimer = 0;
        Platform candidate(m_spawnX, floorY);
        sf::FloatRect candidateBounds = candidate.getBounds();
        if (!isTooClose(candidateBounds, obstaclePool, platforms))
            platforms.push_back(std::move(candidate));
    }

    // --- SPAWN OBSTACLE ---
    if (gameSpeed > 1.15f && m_obstacleTimer > m_obstacleSpawnTime)
    {
        m_obstacleTimer = 0;

        // Find an inactive pool slot
        Obstacle* slot = nullptr;
        for (auto& o : obstaclePool)
        {
            if (!o.active)
            {
                slot = &o;
                break;
            }
        }

        if (slot == nullptr)
        {
            std::cout << "Obstacle pool exhausted, skipping spawn." << std::endl;
            return;
        }

        float platTop = findNearbyPlatformTop(platforms);
        bool onPlatform = (platTop > 0.f) && (rand() % 10 < 3);
        float spawnY = onPlatform ? platTop : -1.f;

        // Check spacing using a temporary bounds estimate
        sf::RectangleShape tempShape({ 50, 50 });
        tempShape.setPosition({ m_spawnX, 0.f });
        sf::FloatRect candidateBounds = tempShape.getGlobalBounds();

        if (!isTooClose(candidateBounds, obstaclePool, platforms))
        {
            slot->activate(m_spawnX, spawnY, gameSpeed, floorY);
        }
    }
}

bool SpawnManager::isTooClose(sf::FloatRect newBounds,
    const std::vector<Obstacle>& obstaclePool,
    const std::vector<Platform>& platforms) const
{
    for (const auto& o : obstaclePool)
    {
        if (!o.active) continue;
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
    const float horizontalSnapRange = 250.f;
    for (const auto& p : platforms)
    {
        sf::FloatRect pb = p.getBounds();
        bool horizontallyAligned =
            pb.position.x < m_spawnX &&
            pb.position.x + pb.size.x > m_spawnX - horizontalSnapRange;
        if (horizontallyAligned)
            return pb.position.y;
    }
    return -1.f;
}
