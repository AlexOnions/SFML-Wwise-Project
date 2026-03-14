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

enum class GameState {
    Playing,
    GameOver
};

enum CurrentEvent {
    None,
    Squish,
    SquishStay,
    SquishReverse
};

float lerp(float a, float b, float t)
{
    return a + (b - a) * t;
}

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

    sf::RenderWindow window(sf::VideoMode({ 800,400 }), "Endless Runner Prototype");
    window.setFramerateLimit(60);

    GameState gameState = GameState::Playing;

    // PLAYER
    sf::RectangleShape player({ 40,40 });
    player.setFillColor(sf::Color::Cyan);
    player.setPosition({ 100,300 });

    float velocityY = 0;
    const float gravity = 2000.f;
    const float jumpForce = -700.f;
    bool onGround = true;

    // FLOOR
    sf::RectangleShape floor({ 900,150 });
    floor.setPosition({ -50,340 });
    floor.setFillColor(sf::Color(40, 40, 40));
    floor.setOutlineThickness(-5);
    floor.setOutlineColor(sf::Color(100, 100, 100));

    sf::RectangleShape ceiling({ 900,150 });
    ceiling.setPosition({ -50,-110 });
    ceiling.setFillColor(sf::Color(40, 40, 40));
    ceiling.setOutlineThickness(-5);
    ceiling.setOutlineColor(sf::Color(100, 100, 100)); 
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
    float gameSpeed = 1.2f;
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

    // GAME OVER TEXT
    sf::Text gameOverText(font);
    gameOverText.setCharacterSize(40);
    gameOverText.setFillColor(sf::Color::White);
    gameOverText.setString("GAME OVER\nPress R to Restart\nPress ESC to Quit");
    gameOverText.setPosition(sf::Vector2f(200, 120));

    sf::Clock clock;


    AK::SoundEngine::SetDefaultListeners(&playerID, 1);
    AK::SoundEngine::SetListeners(playerID, &playerID, 1);


    AK::SoundEngine::PostEvent(AKTEXT("PlayLayered"), playerID);

    CurrentEvent currentEvent = CurrentEvent::None;
    float originalEventTimer = 5;
    float randomEventTimer = originalEventTimer + rand() % 15;
    float squishAmount = 70;
    float squishSpeed = 2.0f;
    float squishStayOriginalTime = 10.0f;
    float squishStayTime = 10.0f;

    float floorOriginalY = floor.getPosition().y;
    float ceilingOriginalY = ceiling.getPosition().y;
    float floorOffset = 0.f;
    float lastFloorOffset = 0.f;


    float floorSquishY = floorOriginalY - squishAmount;
    float ceilingSquishY = ceilingOriginalY + squishAmount;

    bool onPlatform = false;


    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();
        onPlatform = false;
        // EVENTS
        while (const std::optional event = window.pollEvent())
        {
            if (event->is<sf::Event::Closed>())
                window.close();

            else if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>())
            {
                if (gameState == GameState::GameOver)
                {
                    if (keyPressed->scancode == sf::Keyboard::Scancode::Escape)
                        window.close();

                    if (keyPressed->scancode == sf::Keyboard::Scancode::R)
                    {
                        // RESET GAME
                        player.setPosition({ 100, 300 });
                        velocityY = 0;
                        onGround = true;

                        obstacles.clear();
                        platforms.clear();
                        score = 0;
                        gameSpeed = 1.0f;

                        gameState = GameState::Playing;
                    }
                }
                else if (gameState == GameState::Playing)
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
        }

        // GAMEPLAY ONLY UPDATES WHEN PLAYING
        if (gameState == GameState::Playing)
        {
            //EVENTS
            randomEventTimer -= deltaTime;

            if (currentEvent == CurrentEvent::None && randomEventTimer <= 0.f)
            {
                currentEvent = CurrentEvent::Squish;
            }

            if (currentEvent == CurrentEvent::Squish)
            {


                // Lerp toward squish positions
                float newFloorY = lerp(floor.getPosition().y, floorSquishY, deltaTime * squishSpeed);
                float newCeilingY = lerp(ceiling.getPosition().y, ceilingSquishY, deltaTime * squishSpeed);

                floor.setPosition(sf::Vector2f(floor.getPosition().x, newFloorY));
                ceiling.setPosition(sf::Vector2f(ceiling.getPosition().x, newCeilingY));

                // Check if close enough to target
                if (std::abs(newFloorY - floorSquishY) < 0.5f &&
                    std::abs(newCeilingY - ceilingSquishY) < 0.5f)
                {
                    currentEvent = CurrentEvent::SquishStay;
                }

                floorOffset = newFloorY - floorOriginalY;

            }
            else if (currentEvent == CurrentEvent::SquishStay)
            {
                squishStayTime -= deltaTime;

                if (squishStayTime <= 0.f)
                {
                    currentEvent = CurrentEvent::SquishReverse;

                    squishStayTime = squishStayOriginalTime;
                }
            }

            else if (currentEvent == CurrentEvent::SquishReverse)
            {
                // Lerp back to original positions
                float newFloorY = lerp(floor.getPosition().y, floorOriginalY, deltaTime * squishSpeed);
                float newCeilingY = lerp(ceiling.getPosition().y, ceilingOriginalY, deltaTime * squishSpeed);

                floor.setPosition(sf::Vector2f(floor.getPosition().x, newFloorY));
                ceiling.setPosition(sf::Vector2f(ceiling.getPosition().x, newCeilingY));

                // Check if close enough to original
                if (std::abs(newFloorY - floorOriginalY) < 0.5f &&
                    std::abs(newCeilingY - ceilingOriginalY) < 0.5f)
                {
                    currentEvent = CurrentEvent::None;
                    randomEventTimer = originalEventTimer + rand() % 15; // random 5–10 seconds
                }

                floorOffset = newFloorY - floorOriginalY;
            }
            // --- SQUISH EVENT ---
            float oldFloorY = floor.getPosition().y;
            float oldCeilingY = ceiling.getPosition().y;

            if (currentEvent == CurrentEvent::Squish)
            {
                float newFloorY = lerp(oldFloorY, floorSquishY, deltaTime * squishSpeed);
                float newCeilingY = lerp(oldCeilingY, ceilingSquishY, deltaTime * squishSpeed);

                floor.setPosition({ floor.getPosition().x, newFloorY });
                ceiling.setPosition({ ceiling.getPosition().x, newCeilingY });

                if (std::abs(newFloorY - floorSquishY) < 0.5f &&
                    std::abs(newCeilingY - ceilingSquishY) < 0.5f)
                {
                    currentEvent = CurrentEvent::SquishStay;
                }
            }
            else if (currentEvent == CurrentEvent::SquishStay)
            {
                squishStayTime -= deltaTime;

                if (squishStayTime <= 0.f)
                {
                    currentEvent = CurrentEvent::SquishReverse;
                    squishStayTime = squishStayOriginalTime;
                }
            }
            else if (currentEvent == CurrentEvent::SquishReverse)
            {
                float newFloorY = lerp(oldFloorY, floorOriginalY, deltaTime * squishSpeed);
                float newCeilingY = lerp(oldCeilingY, ceilingOriginalY, deltaTime * squishSpeed);

                floor.setPosition({ floor.getPosition().x, newFloorY });
                ceiling.setPosition({ ceiling.getPosition().x, newCeilingY });

                if (std::abs(newFloorY - floorOriginalY) < 0.5f &&
                    std::abs(newCeilingY - ceilingOriginalY) < 0.5f)
                {
                    currentEvent = CurrentEvent::None;
                    randomEventTimer = originalEventTimer + rand() % 15;
                }
            }

            // --- COMPUTE OFFSET ---
            floorOffset = floor.getPosition().y - floorOriginalY;


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

            // PLAYER PHYSICS
            velocityY += gravity * deltaTime;
            player.move({ 0, velocityY * deltaTime });

            //Floor Collision
            if (!onPlatform)
            {
                float floorY = floor.getPosition().y;
                float playerBottom = player.getPosition().y + player.getSize().y;

                if (playerBottom >= floorY)
                {
                    player.setPosition({
                        player.getPosition().x,
                        floorY - player.getSize().y
                        });

                    velocityY = 0;
                    onGround = true;
                }

                float ceilingBottom = ceiling.getPosition().y + ceiling.getSize().y;
                float playerTop = player.getPosition().y;

                if (playerTop <= ceilingBottom)
                {
                    // Snap player just below the ceiling
                    player.setPosition({
                        player.getPosition().x,
                        ceilingBottom
                        });

                    velocityY = 0;
                }

            }


            // SPAWN MANAGER
            spawnManager.update(deltaTime, gameSpeed, obstacles, platforms, floor.getPosition().y);


            // UPDATE OBSTACLES
            for (auto& obstacle : obstacles)
            {
                obstacle.update(deltaTime, gameSpeed, 300.f);

                // Move with floor squish offset
                obstacle.shape.move({ 0, floorOffset - lastFloorOffset });

                sf::FloatRect ob = obstacle.getBounds();

                float obLeft = ob.position.x;
                float obRight = ob.position.x + ob.size.x;

                for (auto& platform : platforms)
                {
                    sf::FloatRect pb = platform.getBounds();

                    float platLeft = pb.position.x;
                    float platRight = pb.position.x + pb.size.x;
                    float platTop = pb.position.y;

                    bool horizontallyAligned =
                        obRight > platLeft &&
                        obLeft < platRight;

                    if (horizontallyAligned)
                    {
                        obstacle.shape.setPosition({
                            ob.position.x,
                            platTop - ob.size.y
                            });

                        break;
                    }
                }
            }



            // UPDATE PLATFORMS
            for (auto& platform : platforms) {
                platform.update(deltaTime, gameSpeed, 300.f);
                platform.shape.move(sf::Vector2f(0, floorOffset - lastFloorOffset));

            }
            lastFloorOffset = floorOffset;

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
                        player.setPosition({ playerBounds.position.x, platTop - playerBounds.size.y });
                        velocityY = 0;
                        onGround = true;
                        onPlatform = true;
                    }
                    else if (velocityY < 0 && prevTop >= platBottom - 2.0f)
                    {
                        player.setPosition({ playerBounds.position.x, platBottom });
                        velocityY = 0;
                    }
                    else
                    {
                        AK::SoundEngine::PostEvent(AKTEXT("Play_Hit"), playerID);
                        gameState = GameState::GameOver;
                    }
                }
            }

            // OBSTACLE COLLISION
            for (auto& obstacle : obstacles)
            {
                if (player.getGlobalBounds().findIntersection(obstacle.getBounds()))
                {
                    AK::SoundEngine::PostEvent(AKTEXT("Play_Hit"), playerID);
                    gameState = GameState::GameOver;
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


        }

        // AUDIO
        AK::SoundEngine::RenderAudio();

        // DRAW
        window.clear(sf::Color::Black);

        background.draw(window);
        window.draw(floor);
        window.draw(ceiling);
        window.draw(player);

        for (auto& obstacle : obstacles)
            obstacle.draw(window);

        for (auto& platform : platforms)
            platform.draw(window);

        window.draw(speedText);
        window.draw(scoreText);

        if (gameState == GameState::GameOver)
            window.draw(gameOverText);

        window.display();
    }

    wwise.terminateSoundEngine();

    return 0;
}