
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

class Ship {
public:
    sf::RectangleShape shape;
    float speed = 1.f;
    sf::RectangleShape laser;  // Add laser member
    bool isShooting = false;   // Track if laser is being shown
    sf::Vector2f targetPos;    // Store target position for laser

    Ship(float x, float y) {
        shape.setSize({40.f, 40.f});
        shape.setFillColor(sf::Color::Blue);
        shape.setPosition(x, y);
        
        // Initialize laser
        laser.setSize({1.f, 1.f});  // Width will be adjusted when shooting
        laser.setFillColor(sf::Color::Green);
    }

    void move(float dx, float dy) {
        shape.move(dx * speed, dy * speed);
    }

    void shoot(sf::Vector2f target) {
        isShooting = true;
        targetPos = target;
        
        // Get ship's center position
        sf::Vector2f shipCenter = shape.getPosition() + sf::Vector2f(shape.getSize().x / 2, shape.getSize().y / 2);
        
        // Calculate laser angle and length
        float dx = target.x - shipCenter.x;
        float dy = target.y - shipCenter.y;
        float rotation = std::atan2(dy, dx) * 180 / M_PI;
        float length = std::sqrt(dx*dx + dy*dy);
        
        // Set laser properties
        laser.setPosition(shipCenter);
        laser.setSize({length, 2.f});  // Make laser 2 pixels thick
        laser.setRotation(rotation);
    }
};

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Asteroid Mining Game");
    Ship ship(100, 300);
    Asteroid asteroid(500, 250, 0);

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
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            
            // Handle mouse click for shooting
            if (event.type == sf::Event::MouseButtonPressed) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    sf::Vector2i mousePos = sf::Mouse::getPosition(window);
                    ship.shoot(sf::Vector2f(mousePos));
                }
            }
            
            // Reset laser when mouse button is released
            if (event.type == sf::Event::MouseButtonReleased) {
                if (event.mouseButton.button == sf::Mouse::Left) {
                    ship.isShooting = false;
                }
            }
        }

        // Ship Movement
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))  ship.move(-1, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) ship.move(1, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))    ship.move(0, -1);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))  ship.move(0, 1);

        // Mining
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space)) {
            sf::Vector2f shipPos = ship.shape.getPosition();
            sf::Vector2f asteroidPos = asteroid.shape.getPosition();

            float distance = std::sqrt(std::pow(shipPos.x - asteroidPos.x, 2) + std::pow(shipPos.y - asteroidPos.y, 2));
            if (distance < 60) {  // Mining range
                asteroid.mine(10);
            }
        }

        // Update UI
        std::stringstream ss;
        ss << "Asteroid Material: " << asteroid.material;
        materialText.setString(ss.str());

        // Draw
        window.clear();
        window.draw(asteroid.shape);
        window.draw(ship.shape);
        if (ship.isShooting) {
            window.draw(ship.laser);
        }
        window.draw(materialText);
        window.display();
    }

    return 0;
}