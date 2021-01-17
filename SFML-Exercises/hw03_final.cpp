#include <iostream>
#include <math.h>
#include <vector>
#include <fstream>
#include <random>
#include <SFML/Graphics.hpp>

// constants
constexpr unsigned int fps_limit{60};
constexpr unsigned int color_amount{6};
constexpr float epsilon{1e-6f};
const sf::Time fixed_update_time = sf::seconds(1.f/144.f);

// default values
namespace default_vals {
    constexpr unsigned int window_w{850};
    constexpr unsigned int window_h{500};
    constexpr unsigned int circ_amount{60};
    constexpr unsigned int rect_amount{40};
    constexpr float circle_radius{30.f};
    constexpr float square_size{50.f};
    constexpr float speed{20.f};
    constexpr float fcircle_speed{200.f};
    constexpr float frectangle_speed{200.f};
}

const sf::Color color[color_amount] = {sf::Color::Red, sf::Color::Green, sf::Color::Blue, sf::Color::Yellow, sf::Color::Cyan, sf::Color::White};

// enumerations
enum Direction {up, down, left, right};

// globals
unsigned int window_w{default_vals::window_w};
unsigned int window_h{default_vals::window_h};
unsigned int circ_amount{default_vals::circ_amount};
unsigned int rect_amount{default_vals::rect_amount};
float circle_radius{default_vals::circle_radius};
float square_size{default_vals::square_size};
float speed{default_vals::speed};
float fcircle_speed{default_vals::fcircle_speed};
float frectangle_speed{default_vals::frectangle_speed};

bool directionFlags[4] = {false, false, false, false};
bool leftMouseButtonFlag = false;
std::vector<sf::RectangleShape> rectangles;
std::vector<sf::CircleShape> circles;

bool readFromAvailableText() {
    std::string input;
    std::ifstream settings("hw03_settings.txt");
    if (settings.is_open()) {
        settings >> window_w >> window_h;
        settings >> circ_amount >> rect_amount;
        settings >> circle_radius;
        settings >> square_size;
        settings >> speed;
        settings >> fcircle_speed;
        settings >> frectangle_speed;
        settings.close();
        return true;
    } else {
        return false;
    }
}

void generateShapes() {
    std::random_device rd; // this is deterministic in earlier versions of MinGW, so use rand() or some other random seed generation
    std::mt19937 gen(rand()); // change to rd() if using MinGW 10
    std::uniform_int_distribution<int> distrib_w(0, window_w-1); 
    std::uniform_int_distribution<int> distrib_h(0, window_h-1);
    for (unsigned int i = 0; i < rect_amount; ++i) {
        rectangles[i].setSize(sf::Vector2f(square_size, square_size));
        rectangles[i].setFillColor(color[i % color_amount]);
        rectangles[i].setPosition(distrib_w(gen), distrib_h(gen));
    }

    for (unsigned int i = 0; i < circ_amount; ++i) {
        circles[i].setRadius(circle_radius);
        circles[i].setFillColor(color[i % color_amount]);
        circles[i].setPosition(distrib_w(gen), distrib_h(gen));
    }
}

void initializeSettings() {
    if (readFromAvailableText()) {
        std::cout << "hw03_settings.txt successfully loaded.\n";
    } else {
        std::cout << "hw03_settings.txt not loaded. Using default values.\n";
    }
    rectangles.resize(rect_amount);
    circles.resize(circ_amount);
    generateShapes();
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

void update(sf::RenderWindow& window, const sf::Time& elapsed) {
    float delta = elapsed.asSeconds();
    // update all the non-controllable objects
    for (unsigned int i = 1; i < rect_amount; ++i) {
        rectangles[i].move(delta * speed, 0.f);
    }
    for (unsigned int i = 1; i < circ_amount; ++i) {
        circles[i].move(0.f, delta * speed);
    }

    // first circle control
    sf::Vector2f fcircle_dir;
    if (directionFlags[static_cast<unsigned int>(Direction::up)]) fcircle_dir.y -= 69.f;
    if (directionFlags[static_cast<unsigned int>(Direction::left)]) fcircle_dir.x -= 69.f;
    if (directionFlags[static_cast<unsigned int>(Direction::down)]) fcircle_dir.y += 69.f;
    if (directionFlags[static_cast<unsigned int>(Direction::right)]) fcircle_dir.x += 69.f;
    float fcircle_dir_mag = std::hypot(fcircle_dir.x, fcircle_dir.y);
    if (fcircle_dir_mag > epsilon) {
        sf::Vector2f fcircle_v = (fcircle_dir / fcircle_dir_mag) * fcircle_speed * delta;
        circles[0].move(fcircle_v);    
    }

    // first rectangle control
    if (leftMouseButtonFlag) {
        sf::Vector2f diff = static_cast<sf::Vector2f>(sf::Mouse::getPosition(window)) - rectangles[0].getPosition();
        float actual_length = std::hypot(diff.x, diff.y); // overflow protection
        float decided_length = std::min(actual_length, frectangle_speed * delta); // actual vs trajectory
        if (decided_length > epsilon) {
            sf::Vector2f frectangle_v = (diff / actual_length) * decided_length;
            rectangles[0].move(frectangle_v);
        }
    }
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    for (unsigned int i = 0; i < rect_amount; ++i) window.draw(rectangles[i]);
    for (unsigned int i = 0; i < circ_amount; ++i) window.draw(circles[i]);
    window.display();
}


int main() {
    srand(time(NULL)); // random_device is deterministic on older MinGW versions, kept as failsafe
    sf::RenderWindow window(sf::VideoMode(window_w, window_h), "HW03 - Game of Squares and Circles");
    window.setFramerateLimit(fps_limit);

    initializeSettings();

    sf::Clock clock;
    sf::Time timeSinceLastUpdate;
    while (window.isOpen()) {
        sf::Time elapsed = clock.restart();
        timeSinceLastUpdate += elapsed;

        handleInput(window);
        while (timeSinceLastUpdate >= fixed_update_time) {
            update(window, fixed_update_time);    
            timeSinceLastUpdate -= fixed_update_time;
        }
        render(window);
    }
    return 0;
}


