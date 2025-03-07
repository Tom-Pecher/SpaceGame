
#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <SFML/System.hpp>
#include <cmath>
#include <iostream>
#include <sstream>

class Asteroid {
public:
    sf::CircleShape shape;
    int material;

    Asteroid(float x, float y, int m) : material(m) {
        shape.setRadius(40.f);
        shape.setFillColor(sf::Color::Red);
        shape.setPosition(x, y);
    }

    void mine(int amount) {
        material += amount;
    }
};

class Debris {
public:
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float lifetime;

    Debris(float x, float y, sf::Vector2f vel) {
        shape.setRadius(2.f);
        shape.setFillColor(sf::Color::Yellow);
        shape.setPosition(x, y);
        velocity = vel;
        lifetime = 5.0f; // Seconds the debris will exist
    }

    void update(float deltaTime) {
        shape.move(velocity * deltaTime);
        lifetime -= deltaTime;
    }

    bool isDead() {
        return lifetime <= 0;
    }
};

class Ship {
public:
    sf::RectangleShape shape;
    float speed = 1.f;
    sf::RectangleShape laser;
    bool isShooting = false;
    sf::Vector2f targetPos;

    Ship(float x, float y) {
        shape.setSize({40.f, 40.f});
        shape.setFillColor(sf::Color::Blue);
        shape.setPosition(x, y);
        
        laser.setSize({1.f, 1.f});
        laser.setFillColor(sf::Color::Green);
    }

    void move(float dx, float dy) {
        shape.move(dx * speed, dy * speed);
        // Update laser position if shooting
        if (isShooting) {
            updateLaser();
        }
    }

    void shoot(sf::Vector2f target) {
        isShooting = true;
        targetPos = target;
        updateLaser();
    }

private:
    void updateLaser() {
        // Get ship's center position
        sf::Vector2f shipCenter = shape.getPosition() + sf::Vector2f(shape.getSize().x / 2, shape.getSize().y / 2);
        
        // Calculate laser angle and length
        float dx = targetPos.x - shipCenter.x;
        float dy = targetPos.y - shipCenter.y;
        float rotation = std::atan2(dy, dx) * 180 / M_PI;
        float length = std::sqrt(dx*dx + dy*dy);
        
        // Set laser properties
        laser.setPosition(shipCenter);
        laser.setSize({length, 2.f});
        laser.setRotation(rotation);
    }
};

bool laserIntersectsAsteroid(const Ship& ship, const Asteroid& asteroid) {
    if (!ship.isShooting) return false;
    
    sf::Vector2f shipCenter = ship.shape.getPosition() + sf::Vector2f(ship.shape.getSize().x / 2, ship.shape.getSize().y / 2);
    sf::Vector2f asteroidCenter = asteroid.shape.getPosition() + sf::Vector2f(asteroid.shape.getRadius(), asteroid.shape.getRadius());
    
    // Get distance from asteroid center to laser line
    sf::Vector2f laserDir = ship.targetPos - shipCenter;
    float laserLength = std::sqrt(laserDir.x * laserDir.x + laserDir.y * laserDir.y);
    laserDir /= laserLength; // Normalize
    
    sf::Vector2f asteroidToShip = asteroidCenter - shipCenter;
    float dot = asteroidToShip.x * laserDir.x + asteroidToShip.y * laserDir.y;
    sf::Vector2f projection = shipCenter + laserDir * dot;
    
    sf::Vector2f asteroidToProj = projection - asteroidCenter;
    float distance = std::sqrt(asteroidToProj.x * asteroidToProj.x + asteroidToProj.y * asteroidToProj.y);
    
    return distance < asteroid.shape.getRadius() && dot > 0 && dot < laserLength;
}


int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Asteroid Mining Game");
    Ship ship(100, 300);
    Asteroid asteroid(500, 250, 0);
    std::vector<Debris> debris;
    sf::Clock clock;

    sf::Font font;
    if (!font.loadFromFile("/usr/share/fonts/truetype/msttcorefonts/arial.ttf")) {
        std::cerr << "Error loading font!" << std::endl;
        return -1;
    }

    sf::Text materialText;
    materialText.setFont(font);
    materialText.setCharacterSize(20);
    materialText.setFillColor(sf::Color::White);
    materialText.setPosition(20, 20);

    while (window.isOpen()) {
        float deltaTime = clock.restart().asSeconds();
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            
            // Handle mouse click for shooting
            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                ship.shoot(sf::Vector2f(mousePos));
            } else {
                ship.isShooting = false;
            }
            
            // Reset laser when mouse button is released
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    ship.isShooting = false;
                }
            }
        }

        // Ship Movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || sf::Keyboard::isKeyPressed(sf::Keyboard::A))  ship.move(-1, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || sf::Keyboard::isKeyPressed(sf::Keyboard::D)) ship.move(1, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || sf::Keyboard::isKeyPressed(sf::Keyboard::W))    ship.move(0, -1);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || sf::Keyboard::isKeyPressed(sf::Keyboard::S))  ship.move(0, 1);

        for (auto it = debris.begin(); it != debris.end();) {
            it->update(deltaTime);
            if (it->isDead()) {
                it = debris.erase(it);
            } else {
                ++it;
            }
        }

        // Create new debris if laser hits asteroid
        if (laserIntersectsAsteroid(ship, asteroid)) {
            for (int i = 0; i < 2; i++) {
                float angle = static_cast<float>(rand()) / RAND_MAX * 2 * M_PI;
                sf::Vector2f vel(std::cos(angle) * 100.f, std::sin(angle) * 100.f);
                sf::Vector2f asteroidCenter = asteroid.shape.getPosition() + 
                    sf::Vector2f(asteroid.shape.getRadius(), asteroid.shape.getRadius());
                debris.emplace_back(asteroidCenter.x, asteroidCenter.y, vel);
            }
        }

        // Mining
        // if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
        //     sf::Vector2f shipPos = ship.shape.getPosition();
        //     sf::Vector2f asteroidPos = asteroid.shape.getPosition();

        //     float distance = std::sqrt(std::pow(shipPos.x - asteroidPos.x, 2) + std::pow(shipPos.y - asteroidPos.y, 2));
        //     if (distance < 60) {  // Mining range
        //         asteroid.mine(10);
        //     }
        // }

        // Update UI
        std::stringstream ss;
        ss << "Asteroid Material: " << asteroid.material;
        materialText.setString(ss.str());

        // Draw
        window.clear();
        window.draw(asteroid.shape);
        if (ship.isShooting) {
            window.draw(ship.laser);
        }
        window.draw(ship.shape);
        for (const auto& d : debris) {
            window.draw(d.shape);
        }
        window.draw(materialText);
        window.display();
    }

    return 0;
}