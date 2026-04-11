#pragma once
#include <SFML/Graphics.hpp>
#include <AK/SoundEngine/Common/AkSoundEngine.h>

class Obstacle
{
public:
    sf::RectangleShape shape;
    uint64_t m_audioID = 0;
    bool active = false;

    // Called once at startup per pool slot
    void init(uint64_t audioID, uint64_t playerID);

    // Called each time this slot is reused
    void activate(float startX, float platformTopY, float gameSpeed, float floorY);

    // Called when obstacle goes offscreen
    void deactivate();

    void update(float deltaTime, float speedMultiplier, float obstacleSpeed, float playerX);
    void draw(sf::RenderWindow& window);
    bool isOffScreen() const;
    sf::FloatRect getBounds() const;

private:
    uint64_t m_playerID = 0;
    bool m_soundPlaying = false;
};
