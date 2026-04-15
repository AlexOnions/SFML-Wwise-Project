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
    //Initialise soundbanl
    WwiseWrapper wwise;
    if (!wwise.initSoundEngine(AKTEXT("SFML Wwise Project/GeneratedSoundBanks/Windows")))
    {
        std::cout << "Could not initialise Wwise. Exiting." << std::endl;
        return 1;
    }

    //Load soundbank
    AkBankID mainBankId;
    if (AK::SoundEngine::LoadBank(AKTEXT("MainSoundbank"), mainBankId) != AK_Success)
    {
        std::cout << "Could not load soundbank." << std::endl;
        return 1;
    }

    const uint64_t playerID = 0;
    AK::SoundEngine::RegisterGameObj(playerID);

    sf::RenderWindow window(sf::VideoMode({ 800,400 }), "Endless Runner Prototype");
    window.setFramerateLimit(60);

    GameState gameState = GameState::Playing;

    // Player 
    sf::RectangleShape player({ 40,40 });
    player.setFillColor(sf::Color::Cyan);
    player.setPosition({ 100,300 });

    float velocityY = 0;
    const float gravity = 2000.f;
    const float jumpForce = -700.f;
    bool onGround = true;

    // Floor 
    sf::RectangleShape floor({ 900,150 });
    floor.setPosition({ -50,340 });
    floor.setFillColor(sf::Color(40, 40, 40));
    floor.setOutlineThickness(-5);
    floor.setOutlineColor(sf::Color(100, 100, 100));

    //Ceiling 
    sf::RectangleShape ceiling({ 900,150 });
    ceiling.setPosition({ -50,-110 });
    ceiling.setFillColor(sf::Color(40, 40, 40));
    ceiling.setOutlineThickness(-5);
    ceiling.setOutlineColor(sf::Color(100, 100, 100));

    // Parralax Background
    ParallaxBackground background;
    background.addLayer(20.f, 800.f, 320.f, sf::Color(15, 15, 15));
    background.addLayer(40.f, 800.f, 320.f, sf::Color(35, 35, 35));
    background.addLayer(80.f, 800.f, 320.f, sf::Color(60, 60, 60));

    // Obstable piil - 6 obstacles that are registered once then re used (More efficient that creating them each time thye are needed)
    const int POOL_SIZE = 6;
    std::vector<Obstacle> obstaclePool(POOL_SIZE);
    for (int i = 0; i < POOL_SIZE; i++)
        obstaclePool[i].init(100 + i, playerID);

    // Platform 
    std::vector<Platform> platforms;
    platforms.reserve(16);

    SpawnManager spawnManager(300.f, 300.f, playerID);



    // Font
    sf::Font font;
    font.openFromFile("C:/Users/Alexo/source/repos/AlexOnions/SFML-Wwise-Project/SFML Wwise Project/Assets/arial.ttf");

    // Speed 
    float gameSpeed = 1.2f;
    const float maxSpeed = 5.0f;
    const float speedIncreaseRate = 0.01f;

    sf::Text speedText(font);
    speedText.setCharacterSize(20);
    speedText.setFillColor(sf::Color::White);

    // Score
    float score = 0.f;
    float scoreIncreaseAmount = 5;

    sf::Text scoreText(font);
    scoreText.setCharacterSize(20);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({ 10,10 });

    // Game Over
    sf::Text gameOverText(font);
    gameOverText.setCharacterSize(40);
    gameOverText.setFillColor(sf::Color::White);
    gameOverText.setString("GAME OVER\nPress R to Restart\nPress ESC to Quit");
    gameOverText.setPosition(sf::Vector2f(200, 120));

    sf::Clock clock;

    AK::SoundEngine::SetDefaultListeners(&playerID, 1);
    AK::SoundEngine::SetListeners(playerID, &playerID, 1);

    AK::SoundEngine::PostEvent(AKTEXT("PlayLayeredMusic"), playerID);

    //Squish Event
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

        // Input event updates
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
                        // Reset (deactivate all pool slots)
                        for (auto& o : obstaclePool)
                            if (o.active) o.deactivate();

                        platforms.clear();
                        player.setPosition({ 100, 300 });
                        velocityY = 0;
                        onGround = true;
                        score = 0;
                        gameSpeed = 1.2f;
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

        // Gameplay updates
        if (gameState == GameState::Playing)
        {
            // RANDOM EVENT TIMER
            randomEventTimer -= deltaTime;
            if (currentEvent == CurrentEvent::None && randomEventTimer <= 0.f)
            {
                currentEvent = CurrentEvent::Squish;
                AK::SoundEngine::PostTrigger(AKTEXT("Rumble_Stinger"), playerID);
            }

            // Squish event updates
            float oldFloorY = floor.getPosition().y;
            float oldCeilingY = ceiling.getPosition().y;

            switch (currentEvent)
            {
            case CurrentEvent::Squish:
            {
                float newFloorY = lerp(oldFloorY, floorSquishY, deltaTime * squishSpeed);
                float newCeilingY = lerp(oldCeilingY, ceilingSquishY, deltaTime * squishSpeed);
                floor.setPosition({ floor.getPosition().x, newFloorY });
                ceiling.setPosition({ ceiling.getPosition().x, newCeilingY });
                if (std::abs(newFloorY - floorSquishY) < 0.5f &&
                    std::abs(newCeilingY - ceilingSquishY) < 0.5f)
                    currentEvent = CurrentEvent::SquishStay;
                break;
            }
            case CurrentEvent::SquishStay:
            {
                squishStayTime -= deltaTime;
                if (squishStayTime <= 0.f)
                {
                    AK::SoundEngine::PostTrigger(AKTEXT("Rumble_Stinger"), playerID);
                    currentEvent = CurrentEvent::SquishReverse;
                    squishStayTime = squishStayOriginalTime;
                }
                break;
            }
            case CurrentEvent::SquishReverse:
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
                break;
            }
            default: break;
            }

            floorOffset = floor.getPosition().y - floorOriginalY;

            // Score
            score += deltaTime * (scoreIncreaseAmount * gameSpeed);
            scoreText.setString("Score: " + std::to_string((int)score));

            // Speed
            gameSpeed += deltaTime * speedIncreaseRate;
            if (gameSpeed > maxSpeed) gameSpeed = maxSpeed;
            speedText.setString("Speed: " + std::to_string(gameSpeed).substr(0, 4) + "x");
            speedText.setPosition({ 650,10 });
            AK::SoundEngine::SetRTPCValue("GameSpeed", gameSpeed, playerID);

            // Player physics
            velocityY += gravity * deltaTime;
            player.move({ 0, velocityY * deltaTime });

            AkSoundPosition playerPos;
            playerPos.SetPosition(player.getPosition().x, 0, 0);
            AK::SoundEngine::SetPosition(playerID, playerPos);

            // Floor & ceiling collision
            if (!onPlatform)
            {
                float floorY = floor.getPosition().y;
                float playerBottom = player.getPosition().y + player.getSize().y;
                if (playerBottom >= floorY)
                {
                    player.setPosition({ player.getPosition().x, floorY - player.getSize().y });
                    velocityY = 0;
                    onGround = true;
                }
                float ceilingBottom = ceiling.getPosition().y + ceiling.getSize().y;
                if (player.getPosition().y <= ceilingBottom)
                {
                    player.setPosition({ player.getPosition().x, ceilingBottom });
                    velocityY = 0;
                }
            }

            // Spawning
            spawnManager.update(deltaTime, gameSpeed, obstaclePool, platforms, floor.getPosition().y);

            // Update obtacles
            for (auto& obstacle : obstaclePool)
            {
                if (!obstacle.active) continue;

                obstacle.update(deltaTime, gameSpeed, 300.f, player.getPosition().x);
                obstacle.shape.move({ 0, floorOffset - lastFloorOffset });

                // Snap to platform top if aligned
                sf::FloatRect ob = obstacle.getBounds();
                for (auto& platform : platforms)
                {
                    sf::FloatRect pb = platform.getBounds();
                    bool aligned = (ob.position.x + ob.size.x > pb.position.x &&
                        ob.position.x < pb.position.x + pb.size.x);
                    if (aligned)
                    {
                        obstacle.shape.setPosition({ ob.position.x, pb.position.y - ob.size.y });
                        break;
                    }
                }
            }

            // Update platforms
            for (auto& platform : platforms)
            {
                platform.update(deltaTime, gameSpeed, 300.f);
                platform.shape.move({ 0, floorOffset - lastFloorOffset });
            }

            lastFloorOffset = floorOffset;
            background.update(deltaTime, gameSpeed);

            // Platform collisions
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
                        AK::SoundEngine::PostTrigger(AKTEXT("Game_Over_Stinger"), playerID);
                        gameState = GameState::GameOver;
                    }
                }
            }

            // Obstacle collision
            for (auto& obstacle : obstaclePool)
            {
                if (!obstacle.active) continue;
                if (player.getGlobalBounds().findIntersection(obstacle.getBounds()))
                {
                    AK::SoundEngine::PostTrigger(AKTEXT("Game_Over_Stinger"), playerID);
                    gameState = GameState::GameOver;
                }
            }

            // Remove offscreen platforms 
            platforms.erase(
                std::remove_if(platforms.begin(), platforms.end(),
                    [](Platform& p) { return p.isOffScreen(); }),
                platforms.end()
            );
        }

        // Audio
        AK::SoundEngine::RenderAudio();

        // Draw
        window.clear(sf::Color::Black);
        background.draw(window);
        window.draw(floor);
        window.draw(ceiling);
        window.draw(player);

        for (auto& obstacle : obstaclePool)
            obstacle.draw(window);

        for (auto& platform : platforms)
            platform.draw(window);

        window.draw(speedText);
        window.draw(scoreText);

        if (gameState == GameState::GameOver)
            window.draw(gameOverText);

        window.display();
    }

    // Unregister pool on exit
    for (auto& o : obstaclePool)
        AK::SoundEngine::UnregisterGameObj(o.m_audioID);

    wwise.terminateSoundEngine();
    return 0;
}
