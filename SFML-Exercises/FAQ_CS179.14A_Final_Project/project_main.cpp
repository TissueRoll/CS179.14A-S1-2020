#include <iostream>
#include <math.h>
#include <fstream>
#include <random>
#include <string>
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
const sf::Vector2f zero_vector{0.f,0.f};

// default values
namespace default_vals {
    constexpr unsigned int window_w{1500};
    constexpr unsigned int window_h{900};
    const std::string fontFileName{"arial.ttf"};
    constexpr unsigned int fontSize{25};
    namespace user {
        constexpr float width{100.f};
        constexpr float height{30.f};
        constexpr float speed{300.f};
        constexpr int health_points{10};
        const sf::Color color = sf::Color::White;
    }
    namespace balls {
        constexpr float radius{30.f};
        const sf::Time ballGenerationPeriod = sf::seconds(0.25f);
        const sf::Color color = sf::Color::Red;
    }
}

std::random_device rd;
std::string fontFileName{default_vals::fontFileName};
unsigned int fontSize{default_vals::fontSize};

unsigned int score{0};

struct BallEntity {
    sf::CircleShape ball;
    float radius;
    sf::Vector2f velocity;
    sf::Color color;

    BallEntity() = default;

    void setColor(const sf::Color& c) {
        color = c;
    }

    void initializeEntity(float x, float y) {
        ball.setOrigin(radius,radius);
        ball.setRadius(radius);
        ball.setPosition(x, y);
        ball.setFillColor(color);
    }

    void setVelocity(const sf::Vector2f& v) {
        velocity = v;
    }

    void moveEntity(const sf::Vector2f& acceleration, float delta) {
        sf::Vector2f nVelocity = velocity;
        sf::Vector2f pos = ball.getPosition();
        pos += acceleration * 0.5f * delta * delta + nVelocity * delta;
        nVelocity += acceleration * delta;
        ball.setPosition(pos);

        float nVMag = std::hypot(nVelocity.x, nVelocity.y);

        if (nVMag > epsilon) {
            velocity = nVelocity;
        } else {
            velocity = zero_vector;
        }
    }

    bool isInBounds(float x_bound, float y_bound) {
        sf::Vector2f tempPosition = ball.getPosition();
        if (tempPosition.x - radius < 0 ||
            tempPosition.x + radius > x_bound ||
            tempPosition.y - radius < 0 ||
            tempPosition.y + radius > y_bound) {
            return false;
        } else {
            return true;
        }
    }

    bool collidesWith(const sf::RectangleShape& rect) {
        sf::Vector2f ballPos = ball.getPosition();
        sf::Vector2f rectPos = rect.getPosition();
        sf::Vector2f rectSize = rect.getSize();
        
        sf::Vector2f tmp = ballPos;

        // pick a corner or edge
        if (ballPos.x < rectPos.x) tmp.x = rectPos.x;
        else if (ballPos.x > rectPos.x + rectSize.x) tmp.x = rectPos.x + rectSize.x;
        if (ballPos.y < rectPos.y) tmp.y = rectPos.y;
        else if (ballPos.y > rectPos.y + rectSize.y) tmp.y = rectPos.y + rectSize.y;

        tmp = ballPos - tmp;
        float dist = std::hypot(tmp.x, tmp.y);

        if (dist <= radius) {
            return true;
        } else {
            return false;
        }
    }
};

class BallPool {
public:
    float x_bound;
    float y_bound;
    BallPool() = default;
    bool initializeBallPool(float xb, float yb, float radius, sf::Color c, sf::Vector2f std_v) {
        if (2*radius > xb || 2*radius > yb) {
            std::cout << "radius is too large for the screen\n";
            return false;
        }
        x_bound = xb;
        y_bound = yb;
        for (unsigned int i = 0; i < _pool_size; ++i) {
            _ballPool[i].setColor(c);
            _ballPool[i].radius = radius;
            _ballPool[i].initializeEntity(-radius, -radius);
            _ballPool[i].setVelocity(std_v);
        }
        return true;
    }
    void updateAllBallsInBound(float delta, unsigned int& s) {
        for (unsigned int i = 0; i < _pool_size; ++i) {
            if (_ballPool[i].isInBounds(x_bound, y_bound)) {
                _ballPool[i].moveEntity(zero_vector, delta);
                if (!_ballPool[i].isInBounds(x_bound, y_bound)) {
                    s += 10;
                }
            } 
        }
    }
    // puts all balls out of bounds
    void resetAllBalls() {
        for (unsigned int i = 0; i < _pool_size; ++i) {
            _ballPool[i].ball.setPosition(-_ballPool[i].radius,-_ballPool[i].radius);
        }
    }
    void generateFallingBall() {
        std::mt19937 gen(rand()); // change to rd() if using MinGW10
        std::uniform_int_distribution<int> distrib_w(0, x_bound);
        std::uniform_real_distribution<float> distrib_vel(100.f, 1000.f);
        for (unsigned int i = 0; i < _pool_size; ++i) {
            if (!_ballPool[i].isInBounds(x_bound, y_bound)) {
                int gen_x = distrib_w(gen);
                sf::Vector2f gen_vel{0.f, distrib_vel(gen)};
                float ballRadius = _ballPool[i].radius;
                if (gen_x - ballRadius < 0)
                    gen_x = ballRadius;
                if (gen_x + ballRadius > x_bound)
                    gen_x = x_bound - ballRadius;
                _ballPool[i].setVelocity(gen_vel);
                _ballPool[i].ball.setPosition(gen_x, ballRadius);
                break;
            }
        }
    }

    int ballsCollidingWith(const sf::RectangleShape& paddle) {
        int ans = 0;
        for (unsigned int i = 0; i < _pool_size; ++i) {
            if (_ballPool[i].isInBounds(x_bound, y_bound) && _ballPool[i].collidesWith(paddle)) {
                ans++;
            }
        }
        return ans;
    }

    void resetBallsOnCollision(const sf::RectangleShape& paddle) {
        for (unsigned int i = 0; i < _pool_size; ++i) {
            if (_ballPool[i].isInBounds(x_bound, y_bound) && _ballPool[i].collidesWith(paddle)) {
                _ballPool[i].ball.setPosition(-_ballPool[i].radius, -_ballPool[i].radius);
            }
        }
    }

    void drawVisibleBalls(sf::RenderWindow& window) {
        for (unsigned int i = 0; i < _pool_size; ++i) {
            if (_ballPool[i].isInBounds(x_bound, y_bound)) {
                window.draw(_ballPool[i].ball);
            }
        }
    }

private:
    static const unsigned int _pool_size{200};
    BallEntity _ballPool[_pool_size];
};

// enumerations
enum Direction {up, down, left, right};

// globals
unsigned int window_w{default_vals::window_w};
unsigned int window_h{default_vals::window_h};

bool directionFlags[4] = {false, false, false, false};
bool leftMouseButtonFlag = false;

struct PaddleProperties {
    float width{default_vals::user::width};
    float height{default_vals::user::height};
    float speed{default_vals::user::speed};
    int health_points{default_vals::user::health_points};
    sf::Color color = default_vals::user::color;
};

struct BallProperties {
    float radius{default_vals::balls::radius};
    sf::Time ballGenerationPeriod = default_vals::balls::ballGenerationPeriod;
    sf::Color color = default_vals::balls::color;
};

PaddleProperties paddleProps;
sf::RectangleShape user_paddle;
BallProperties ballProps;
BallPool ballPool;

struct TextObject {
    sf::Font font;
    sf::Text text;
};

TextObject textObject;

bool readFromAvailableText() {
    std::string input;
    std::ifstream settings("project_settings.txt");
    if (settings.is_open()) {
        int r, g, b;
        settings >> window_w >> window_h;
        settings >> paddleProps.width >> paddleProps.height >> paddleProps.speed >> paddleProps.health_points;
        settings >> r >> g >> b;
        r %= 256; g %= 256; b %= 256;
        paddleProps.color = sf::Color(r,g,b);
        settings >> ballProps.radius;
        settings >> r >> g >> b;
        r %= 256; g %= 256; b %= 256;
        ballProps.color = sf::Color(r,g,b);
        settings >> fontFileName >> fontSize;
        settings.close();
        return true;
    } else {
        return false;
    }
}

bool initializeSettings() {
    if (readFromAvailableText()) {
        std::cout << "project_settings.txt successfully loaded.\n";
    } else {
        std::cout << "project_settings.txt not loaded. Using default values.\n";    
    }

    if (!textObject.font.loadFromFile(fontFileName)) {
        return false;
    }
    textObject.text.setString(
        "HP: " + std::to_string(paddleProps.health_points) + 
        "\nScore: " + std::to_string(score) +
        "\nAvoid the falling balls!"
        );
    textObject.text.setFont(textObject.font);
    textObject.text.setCharacterSize(fontSize);

    sf::Vector2f rectSize{paddleProps.width, paddleProps.height};
    user_paddle.setSize(rectSize);
    user_paddle.setPosition(window_w / 2.f - paddleProps.width / 2.f, window_h - paddleProps.height - 15.f);
    user_paddle.setFillColor(sf::Color::White);
    bool initSuccess = ballPool.initializeBallPool(window_w, window_h, ballProps.radius, sf::Color::Red, zero_vector);
    if (!initSuccess) {
        return false;
    }
    ballPool.resetAllBalls();

    return true;
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

    // move paddle
    sf::Vector2f paddle_v;
    if (directionFlags[static_cast<unsigned int>(Direction::left)]) paddle_v.x -= paddleProps.speed;
    if (directionFlags[static_cast<unsigned int>(Direction::right)]) paddle_v.x += paddleProps.speed;
    user_paddle.move(paddle_v * delta);

    // move existing balls
    ballPool.updateAllBallsInBound(delta, score);

    // count collisions
    int collisions = ballPool.ballsCollidingWith(user_paddle);
    ballPool.resetBallsOnCollision(user_paddle);
    paddleProps.health_points -= collisions;
    paddleProps.health_points = std::max(0, paddleProps.health_points);

    textObject.text.setString(
        "HP: " + std::to_string(paddleProps.health_points) + 
        "\nScore: " + std::to_string(score) +
        "\nAvoid the falling balls!"
        );
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    window.draw(user_paddle);
    ballPool.drawVisibleBalls(window);
    window.draw(textObject.text);
    window.display();
}

int main () {
    srand(time(NULL));
    sf::RenderWindow window(sf::VideoMode(window_w, window_h), "CS179.14A Final Project");
	window.setFramerateLimit(fps_limit);

    if (!initializeSettings()) {
        std::cout << "Initialization unsuccessful.\n";
        return 0;
    }
    
    sf::Clock clock;
    sf::Time timeSinceLastUpdate;
    sf::Time timeSinceLastGeneration;
    while(window.isOpen()) {
        sf::Time elapsed = clock.restart();
        timeSinceLastUpdate += elapsed;
        timeSinceLastGeneration += elapsed;

        if (timeSinceLastGeneration >= ballProps.ballGenerationPeriod) {
            if (paddleProps.health_points > 0)
                ballPool.generateFallingBall();
            timeSinceLastGeneration -= ballProps.ballGenerationPeriod;
        }

        handleInput(window);
        while (timeSinceLastUpdate >= fixed_update_time) {
            if (paddleProps.health_points > 0)
                update(fixed_update_time);
            timeSinceLastUpdate -= fixed_update_time;
        }
        render(window);
    }
    return 0;
}