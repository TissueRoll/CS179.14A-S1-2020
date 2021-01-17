#include <iostream>
#include <math.h>
#include <fstream>
#include <SFML/Graphics.hpp>

namespace utility {
    // in case the person compiling this does not have C++17 installed
    // https://en.cppreference.com/w/cpp/algorithm/clamp
    // follows the first version of possible implementations
    template <class T>
    constexpr const T& clamp (const T& v, const T& lo, const T& hi) {
        assert(!(hi < lo));
        return (v < lo) ? lo : (hi < v) ? hi : v;
    }

}

// constants
constexpr unsigned int fps_limit{60};
constexpr float epsilon{1e-6f};
const sf::Time fixed_update_time = sf::seconds(1.f/144.f);

// default values
namespace default_vals {
    constexpr unsigned int window_w{1500};
    constexpr unsigned int window_h{900};
    constexpr float force{10000.f};
    namespace user {
        constexpr float radius{30.f};
        constexpr float mass{1000.f};
        constexpr float elasticity{0.f};
        constexpr float friction{0.05f};    
    }
}

struct Material {
    float mass{100.f};
    float elasticity{0.f};
    float friction{0.01f};
};

struct BallEntity {
    sf::CircleShape ball;
    Material material;
    float radius;
    sf::Vector2f velocity;
    sf::Color colorNoFriction{sf::Color::Green};
    sf::Color colorFriction{sf::Color::Red};

    BallEntity() = default;

    void setFrictionColors(const sf::Color& cNF, const sf::Color& cF) {
        colorNoFriction = cNF;
        colorFriction = cF;
    }

    void initializeEntity(float x, float y, bool frictionEnabled = false) {
        ball.setOrigin(radius,radius);
        ball.setRadius(radius);
        ball.setPosition(x, y);
        ball.setFillColor(frictionEnabled ? colorFriction : colorNoFriction);
    }

    void moveEntity(const sf::Vector2f& acceleration, float delta, bool frictionEnabled = false) {
        sf::Vector2f nVelocity = velocity;
        sf::Vector2f pos = ball.getPosition();
        pos += acceleration * 0.5f * delta * delta + nVelocity * delta;
        nVelocity += acceleration * delta;
        ball.setPosition(pos);

        float nVMag = std::hypot(nVelocity.x, nVelocity.y);
        if (frictionEnabled) {
            ball.setFillColor(colorFriction);
            if (nVMag > epsilon) {
                sf::Vector2f nVNorm = nVelocity / nVMag;
                nVMag = std::max(0.f, nVMag - material.friction * delta);
                nVelocity = nVNorm * nVMag;
            }
        } else {
            ball.setFillColor(colorNoFriction);
        }

        if (nVMag > epsilon) {
            velocity = nVelocity;
        } else {
            velocity = {0,0};
        }
    }

    // snapping; can't think of a better way
    void wallBounce(float x_bound, float y_bound) {
        sf::Vector2f tempPosition = ball.getPosition();
        if (tempPosition.x - radius < 0) {
            ball.setPosition(radius, tempPosition.y);
            tempPosition.x = radius;
            velocity.x *= -material.elasticity;
        }

        if (tempPosition.y - radius < 0) {
            ball.setPosition(tempPosition.x, radius);
            tempPosition.y = radius;
            velocity.y *= -material.elasticity;
        }

        if (tempPosition.x + radius > x_bound) {
            ball.setPosition(x_bound - radius, tempPosition.y);
            tempPosition.x = x_bound - radius;
            velocity.x *= -material.elasticity;
        }

        if (tempPosition.y + radius > y_bound) {
            ball.setPosition(tempPosition.x, y_bound - radius);
            tempPosition.y = y_bound - radius;
            velocity.y *= -material.elasticity;
        }
    }
};

// enumerations
enum Direction {up, down, left, right};

// globals
unsigned int window_w{default_vals::window_w};
unsigned int window_h{default_vals::window_h};
float force{default_vals::force};

bool directionFlags[4] = {false, false, false, false};
bool leftMouseButtonFlag = false;
bool gfrictionEnabled = false;

BallEntity userBallEntity;

bool readFromAvailableText() {
    std::string input;
    std::ifstream settings("hw05_settings.txt");
    if (settings.is_open()) {
        settings >> window_w >> window_h;
        settings >> force;
        settings >> userBallEntity.material.mass >> userBallEntity.material.elasticity >> userBallEntity.material.friction;
        settings >> userBallEntity.radius;
        settings.close();
        return true;
    } else {
        return false;
    }
}

void initializeSettings() {
    if (readFromAvailableText()) {
        std::cout << "hw05_settings.txt successfully loaded.\n";
    } else {
        std::cout << "hw05_settings.txt not loaded. Using default values.\n";
        userBallEntity.material = {default_vals::user::mass, default_vals::user::elasticity, default_vals::user::friction};
        userBallEntity.radius = default_vals::user::radius;
        userBallEntity.setFrictionColors(sf::Color::Green, sf::Color::Red);
        force = default_vals::force;
    }

    userBallEntity.initializeEntity(window_w / 2.f, window_h / 2.f);
}

void pressEvents(sf::RenderWindow& window, const sf::Event& event) {
    switch (event.key.code) {
        case sf::Keyboard::Escape:
            window.close();
            break;
        case sf::Keyboard::W:
            directionFlags[static_cast<unsigned int>(Direction::up)] = true;
            break;
        case sf::Keyboard::A:
            directionFlags[static_cast<unsigned int>(Direction::left)] = true;
            break;
        case sf::Keyboard::S:
            directionFlags[static_cast<unsigned int>(Direction::down)] = true;
            break;
        case sf::Keyboard::D:
            directionFlags[static_cast<unsigned int>(Direction::right)] = true;
            break;
        case sf::Keyboard::F:
            gfrictionEnabled = !gfrictionEnabled;
            break;
        default:
            // nothing
            break;
    }
}

void releaseEvents(sf::RenderWindow& window, const sf::Event& event) {
    switch (event.key.code) {
        case sf::Keyboard::W:
            directionFlags[static_cast<unsigned int>(Direction::up)] = false;
            break;
        case sf::Keyboard::A:
            directionFlags[static_cast<unsigned int>(Direction::left)] = false;
            break;
        case sf::Keyboard::S:
            directionFlags[static_cast<unsigned int>(Direction::down)] = false;
            break;
        case sf::Keyboard::D:
            directionFlags[static_cast<unsigned int>(Direction::right)] = false;
            break;
        default:
            // nothing
            break;
    }
}

void handleInput(sf::RenderWindow& window) {
    sf::Event event;
    while (window.pollEvent(event)) {
        switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;
            case sf::Event::KeyPressed:
                pressEvents(window, event);
                break;
            case sf::Event::MouseButtonPressed:
                if (event.mouseButton.button == sf::Mouse::Left) {leftMouseButtonFlag = true;}
                break;
            case sf::Event::KeyReleased:
                releaseEvents(window, event);
                break;
            case sf::Event::MouseButtonReleased:
                if (event.mouseButton.button == sf::Mouse::Left) {leftMouseButtonFlag = false;}
                break;
            default:
                // nothing
                break;
        }
    }
}

// note: if it's instantaneous acceleration, use a local variable instead
void update(const sf::Time& elapsed) {
    float delta = elapsed.asSeconds();

    sf::Vector2f dir;
    sf::Vector2f acceleration;
    if (directionFlags[static_cast<unsigned int>(Direction::up)]) dir.y -= 69.f;
    if (directionFlags[static_cast<unsigned int>(Direction::left)]) dir.x -= 69.f;
    if (directionFlags[static_cast<unsigned int>(Direction::down)]) dir.y += 69.f;
    if (directionFlags[static_cast<unsigned int>(Direction::right)]) dir.x += 69.f;
    float dir_mag = std::hypot(dir.x, dir.y);
    if (dir_mag > epsilon) {
        acceleration = (dir / dir_mag) * force / userBallEntity.material.mass;
    }

    userBallEntity.moveEntity(acceleration, delta, gfrictionEnabled);
    userBallEntity.wallBounce(window_w, window_h);
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    window.draw(userBallEntity.ball);
    window.display();
}

int main () {
    srand(time(NULL));
    sf::RenderWindow window(sf::VideoMode(window_w, window_h), "HW05");
	window.setFramerateLimit(fps_limit);

    initializeSettings();
    
    sf::Clock clock;
    sf::Time timeSinceLastUpdate;
    while(window.isOpen()) {
        sf::Time elapsed = clock.restart();
        timeSinceLastUpdate += elapsed;

        handleInput(window);
        while (timeSinceLastUpdate >= fixed_update_time) {
            update(fixed_update_time);
            timeSinceLastUpdate -= fixed_update_time;
        }
        render(window);
    }
    return 0;
}