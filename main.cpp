#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <optional>
#include <algorithm>
#include <cstdlib>

#include "WwiseWrapper.h"
#include "Obstacle.h"
#include "Platform.h"
#include "ParallaxBackground.h"
#include "SpawnManager.h"

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

    // PARALLAX
    ParallaxBackground background;
    background.addLayer(20.f, 800.f, 320.f, sf::Color(15, 15, 15));
    background.addLayer(40.f, 800.f, 320.f, sf::Color(35, 35, 35));
    background.addLayer(80.f, 800.f, 320.f, sf::Color(60, 60, 60));

    // OBSTACLES & PLATFORMS
    std::vector<Obstacle> obstacles;
    std::vector<Platform> platforms;

    SpawnManager spawnManager(300.f, 300.f);

    // SPEED
    float gameSpeed = 1.5f;
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

    //AK::SoundEngine::PostEvent(AKTEXT("Loop"), playerID);
    AK::SoundEngine::PostEvent(AKTEXT("PlayLayered"), playerID);


    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        // SCORE
        score += deltaTime * (scoreIncreaseAmount * gameSpeed);
        scoreText.setString("Score: " + std::to_string((int)score));

        // SPEED
        gameSpeed += deltaTime * speedIncreaseRate;
        if (gameSpeed > maxSpeed)
            gameSpeed = maxSpeed;

        speedText.setString("Speed: " + std::to_string(gameSpeed).substr(0, 4) + "x");
        speedText.setPosition({ 650,10 });


        AK::SoundEngine::SetRTPCValue("GameSpeed", gameSpeed, playerID);

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

        // SPAWN MANAGER
        spawnManager.update(deltaTime, gameSpeed, obstacles, platforms);

        // UPDATE OBSTACLES
        for (auto& obstacle : obstacles)
            obstacle.update(deltaTime, gameSpeed, 300.f);

        // UPDATE PLATFORMS
        for (auto& platform : platforms)
            platform.update(deltaTime, gameSpeed, 300.f);

        // UPDATE BACKGROUND
        background.update(deltaTime, gameSpeed);

        // PLATFORM COLLISION
        for (auto& platform : platforms)
        {
            auto playerBounds = player.getGlobalBounds();
            auto platformBounds = platform.getBounds();

            if (playerBounds.findIntersection(platformBounds))
            {
                float playerBottom = playerBounds.position.y + playerBounds.size.y;
                float playerTop = playerBounds.position.y;
                float platTop = platformBounds.position.y;
                float platBottom = platformBounds.position.y + platformBounds.size.y;

                float prevBottom = playerBottom - velocityY * deltaTime;
                float prevTop = playerTop - velocityY * deltaTime;

                if (velocityY >= 0 && prevBottom <= platTop + 2.0f)
                {
                    // TOP — land on platform
                    player.setPosition({ playerBounds.position.x, platTop - playerBounds.size.y });
                    velocityY = 0;
                    onGround = true;
                }
                else if (velocityY < 0 && prevTop >= platBottom - 2.0f)
                {
                    // BOTTOM — bonk head, push back down
                    player.setPosition({ playerBounds.position.x, platBottom });
                    velocityY = 0;
                }
                else
                {
                    // SIDE — game over
                    std::cout << "Game Over\n";
                    AK::SoundEngine::PostEvent(AKTEXT("Hit"), playerID);
                    window.close();
                }
            }
        }

        // OBSTACLE COLLISION
        for (auto& obstacle : obstacles)
        {
            if (player.getGlobalBounds().findIntersection(obstacle.getBounds()))
            {
                std::cout << "Game Over\n";
                AK::SoundEngine::PostEvent(AKTEXT("Hit"), playerID);
                window.close();
            }
        }

        // REMOVE OFFSCREEN OBSTACLES
        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(),
                [](Obstacle& o) { return o.isOffScreen(); }),
            obstacles.end()
        );

        // REMOVE OFFSCREEN PLATFORMS
        platforms.erase(
            std::remove_if(platforms.begin(), platforms.end(),
                [](Platform& p) { return p.isOffScreen(); }),
            platforms.end()
        );

        // AUDIO
        AK::SoundEngine::RenderAudio();

        // DRAW
        window.clear(sf::Color::Black);

        background.draw(window);
        window.draw(floor);

        

        window.draw(player);

        for (auto& obstacle : obstacles)
            obstacle.draw(window);

        for (auto& platform : platforms)
            platform.draw(window);
        window.draw(speedText);
        window.draw(scoreText);

        window.display();
    }

    wwise.terminateSoundEngine();

    return 0;
}