#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <optional>
#include <algorithm>
#include <cstdlib>

#include "WwiseWrapper.h"
#include "Obstacle.h"

int main()
{
    WwiseWrapper wwise;

    if (!wwise.initSoundEngine(AKTEXT("SFML Wwise Project/GeneratedSoundBanks/Windows")))
    {
        std::cout << "Could not initialise Wwise. Exiting." << std::endl;
        return 1;
    }

    AkBankID mainBankId;
    if (AK::SoundEngine::LoadBank(AKTEXT("MainSoundbank"), mainBankId) != AK_Success)
    {
        std::cout << "Could not load soundbank." << std::endl;
        return 1;
    }

    const uint64_t playerID = 1;
    AK::SoundEngine::RegisterGameObj(playerID);

    sf::RenderWindow window(sf::VideoMode({ 800,400 }), "Geometry Dash Prototype");
    window.setFramerateLimit(60);

    // PLAYER
    sf::RectangleShape player({ 40,40 });
    player.setFillColor(sf::Color::Cyan);
    player.setPosition({ 100,300 });

    float velocityY = 0;
    const float gravity = 2000.f;
    const float jumpForce = -700.f;
    bool onGround = true;

    // FLOOR
    sf::RectangleShape floor({ 800,40 });
    floor.setPosition({ 0,340 });
    floor.setFillColor(sf::Color(100, 100, 100));

    // OBSTACLES
    std::vector<Obstacle> obstacles;

    float obstacleTimer = -1.5f;
    float obstacleSpawnTime = 2.0f;
    float obstacleSpeed = 300.f;

    // SPEED
    float gameSpeed = 1.0f;
    const float maxSpeed = 5.0f;
    const float speedIncreaseRate = 0.01f;

    // FONT
    sf::Font font;
    font.openFromFile("C:/Users/Alexo/source/repos/AlexOnions/SFML-Wwise-Project/SFML Wwise Project/Assets/arial.ttf");

    // SPEED TEXT
    sf::Text speedText(font);
    speedText.setCharacterSize(20);
    speedText.setFillColor(sf::Color::White);

    // SCORE
    float score = 0.f;
    float scoreIncreaseAmount = 5;

    sf::Text scoreText(font);
    scoreText.setCharacterSize(20);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({ 10,10 });

    sf::Clock clock;

    AK::SoundEngine::PostEvent(AKTEXT("Loop"), playerID);

    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        // SCORE
        score += deltaTime * scoreIncreaseAmount;
        scoreText.setString("Score: " + std::to_string((int)score));

        // SPEED
        gameSpeed += deltaTime * speedIncreaseRate;
        if (gameSpeed > maxSpeed)
            gameSpeed = maxSpeed;

        speedText.setString("Speed: " + std::to_string(gameSpeed).substr(0, 4) + "x");
        speedText.setPosition({ 650,10 });

        // EVENTS
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                    window.close();

                if (keyPressed->scancode == sf::Keyboard::Scancode::Space && onGround)
                {
                    std::cout << "Jump event triggered\n";
                    AK::SoundEngine::PostEvent(AKTEXT("Play_Jump"), playerID);
                    velocityY = jumpForce;
                    onGround = false;

                }
            }
        }

        // PLAYER PHYSICS
        velocityY += gravity * deltaTime;
        player.move({ 0, velocityY * deltaTime });

        if (player.getPosition().y >= 300)
        {
            player.setPosition({ 100,300 });
            velocityY = 0;
            onGround = true;
        }

        // SPAWN OBSTACLE
        obstacleTimer += deltaTime * gameSpeed;

        if (obstacleTimer > obstacleSpawnTime)
        {
            obstacleTimer = 0;
            obstacles.emplace_back(900);
        }

        // UPDATE OBSTACLES
        for (auto& obstacle : obstacles)
            obstacle.update(deltaTime, gameSpeed, obstacleSpeed);

        // COLLISION
        for (auto& obstacle : obstacles)
        {
            if (player.getGlobalBounds().findIntersection(obstacle.getBounds()))
            {
                std::cout << "Game Over\n";
                AK::SoundEngine::PostEvent(AKTEXT("Hit"), playerID);
                window.close();
            }
        }

        // REMOVE OFFSCREEN
        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(),
                [](Obstacle& o) { return o.isOffScreen(); }),
            obstacles.end()
        );

        // AUDIO
        AK::SoundEngine::RenderAudio();

        // DRAW
        window.clear(sf::Color::Black);

        window.draw(floor);
        window.draw(player);

        for (auto& obstacle : obstacles)
            obstacle.draw(window);

        window.draw(speedText);
        window.draw(scoreText);

        window.display();
    }

    wwise.terminateSoundEngine();

    return 0;
}