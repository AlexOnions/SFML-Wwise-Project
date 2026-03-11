#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <optional>
#include <algorithm>
#include <cstdlib>

#include "WwiseWrapper.h"

int main()
{
    //Git Test
    WwiseWrapper wwise;

    // ---------------- WWISE INIT ----------------
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

    // ---------------- WINDOW ----------------
    sf::RenderWindow window(sf::VideoMode({ 800, 400 }), "Geometry Dash Prototype");
    window.setFramerateLimit(60);

    // ---------------- PLAYER ----------------
    sf::RectangleShape player({ 40, 40 });
    player.setFillColor(sf::Color::Cyan);
    player.setPosition({ 100, 300 });

    float velocityY = 0;
    const float gravity = 2000.f;
    const float jumpForce = -700.f;
    bool onGround = true;

    // ---------------- FLOOR ----------------
    sf::RectangleShape floor({ 800, 40 });
    floor.setPosition({ 0, 340 });
    floor.setFillColor(sf::Color(100, 100, 100));

    // ---------------- OBSTACLES ----------------
    std::vector<sf::RectangleShape> obstacles;

    float obstacleTimer = -1.5f;
    float obstacleSpawnTime = 2.0f;
    float obstacleSpeed = 300.f;

    // ---------------- GAME SPEED ----------------
    float gameSpeed = 1.0f;
    const float maxSpeed = 5.0f;
    const float speedIncreaseRate = 0.01f;

    // ---------------- FONT ----------------
    sf::Font font;

    if (!font.openFromFile("C:/Users/Alexo/source/repos/AlexOnions/SFML-Wwise-Project/SFML Wwise Project/Assets/arial.ttf"))
    {
        std::cout << "Failed to load font\n";
    }

    // ---------------- SPEED TEXT ----------------
    sf::Text speedText(font);
    speedText.setCharacterSize(20);
    speedText.setFillColor(sf::Color::White);

    // ---------------- SCORE TEXT ----------------
    float score = 0.f;
    float scoreIncreaseAmount = 5;

    sf::Text scoreText(font);
    scoreText.setCharacterSize(20);
    scoreText.setFillColor(sf::Color::White);
    scoreText.setPosition({ 10, 10 });

    // ---------------- CLOCK ----------------
    sf::Clock clock;

    // ---------------- GAME LOOP ----------------
    while (window.isOpen())
    {
        float deltaTime = clock.restart().asSeconds();

        // -------- SCORE UPDATE --------
        score += deltaTime * scoreIncreaseAmount; 
        scoreText.setString("Score: " + std::to_string((int)score));

        // -------- SPEED UPDATE --------
        gameSpeed += deltaTime * speedIncreaseRate;
        if (gameSpeed > maxSpeed)
            gameSpeed = maxSpeed;

        speedText.setString("Speed: " + std::to_string(gameSpeed).substr(0, 4) + "x");
        speedText.setPosition({ 650, 10 });

        // -------- EVENTS --------
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
                    velocityY = jumpForce;
                    onGround = false;

                    AK::SoundEngine::PostEvent(AKTEXT("Jump"), playerID);
                }
            }
        }

        // -------- PHYSICS --------
        velocityY += gravity * deltaTime;
        player.move({ 0, velocityY * deltaTime });

        if (player.getPosition().y >= 300)
        {
            player.setPosition({ 100, 300 });
            velocityY = 0;
            onGround = true;
        }

        // -------- SPAWN OBSTACLES --------
        obstacleTimer += deltaTime * gameSpeed;

        if (obstacleTimer > obstacleSpawnTime)
        {
            obstacleTimer = 0;

            sf::RectangleShape obstacle;

            int type = rand() % 3;

            if (type == 0)
            {
                obstacle.setSize({ 20, 70 });
                obstacle.setFillColor(sf::Color::Yellow);
            }
            else if (type == 1)
            {
                obstacle.setSize({ 50, 50 });
                obstacle.setFillColor(sf::Color::Red);
            }
            else
            {
                obstacle.setSize({ 80, 30 });
                obstacle.setFillColor(sf::Color::Magenta);
            }

            obstacle.setPosition({ 900, 340 - obstacle.getSize().y });

            obstacles.push_back(obstacle);
        }

        // -------- MOVE OBSTACLES --------
        for (auto& obstacle : obstacles)
        {
            obstacle.move({ -obstacleSpeed * deltaTime * gameSpeed, 0 });
        }

        // -------- COLLISION --------
        for (auto& obstacle : obstacles)
        {
            if (player.getGlobalBounds().findIntersection(obstacle.getGlobalBounds()))
            {
                std::cout << "Game Over\n";

                AK::SoundEngine::PostEvent(AKTEXT("Hit"), playerID);

                window.close();
            }
        }

        // -------- REMOVE OFFSCREEN OBSTACLES --------
        obstacles.erase(
            std::remove_if(obstacles.begin(), obstacles.end(),
                [](sf::RectangleShape& o)
                {
                    return o.getPosition().x < -100;
                }),
            obstacles.end());

        // -------- AUDIO --------
        AK::SoundEngine::RenderAudio();

        // -------- DRAW --------
        window.clear(sf::Color::Black);

        window.draw(floor);
        window.draw(player);

        for (auto& obstacle : obstacles)
            window.draw(obstacle);

        window.draw(speedText);
        window.draw(scoreText);

        window.display();
    }

    // ---------------- CLEANUP ----------------
    wwise.terminateSoundEngine();

    return 0;
}