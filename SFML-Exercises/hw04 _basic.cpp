#include <iostream>
#include <algorithm>
#include <fstream>
#include <SFML/Audio.hpp>
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
const std::string help_text{
R"(List of Commands:
H: Help -- displays this help screen
Q: Play/Pause Music
R: Reset Music -- stops music and plays from start
W/S: Increase/Decrease Music Volume
A/D: Increase/Decrease Music Pitch
E: Play Sound
I/K: Increase/Decrease SFX Volume
J/L: Increase/Decrease SFX Pitch)"
};

// default values
namespace default_vals {
    constexpr unsigned int window_w{800};
    constexpr unsigned int window_h{600};
    constexpr float music_volume{50.f};
    constexpr float music_pitch{1.f};
    constexpr float sfx_volume{50.f};
    constexpr float sfx_pitch{1.f};
    const std::string fontFileName{"arial.ttf"};
    constexpr unsigned int fontSize{25};
    const std::string musicFileName{"Music.wav"};
    const std::string sfxFileName{"Sound.wav"};
}

// enumerations
enum Direction {up, down, left, right};

// globals
unsigned int window_w{default_vals::window_w};
unsigned int window_h{default_vals::window_h};

// NOTE: up and down control volume, left and right control pitch
bool musicFlags[4] = {false, false, false, false};
bool sfxFlags[4] = {false, false, false, false};
bool helpFlag{true};

float music_volume{default_vals::music_volume};
float music_pitch{default_vals::music_pitch};
float sfx_volume{default_vals::sfx_volume};
float sfx_pitch{default_vals::sfx_pitch};

std::string fontFileName{default_vals::fontFileName};
unsigned int fontSize{default_vals::fontSize};
std::string musicFileName{default_vals::musicFileName};
std::string sfxFileName{default_vals::sfxFileName};

struct TextObject {
    sf::Font font;
    sf::Text text;
};

struct AudioObject {
    TextObject textObject;
    sf::Music music;
    sf::SoundBuffer soundBuffer;
    sf::Sound sound;
};

AudioObject audioObject;

bool readFromAvailableText() {
    std::string input;
    std::ifstream settings("hw04_settings.txt");
    if (settings.is_open()) {
        settings >> window_w >> window_h;
        settings >> fontFileName >> fontSize;
        settings >> musicFileName;
        settings >> music_volume >> music_pitch;
        settings >> sfxFileName;
        settings >> sfx_volume >> sfx_pitch;
        settings.close();
        return true;
    } else {
        return false;
    }
}

bool initializeSettings() {
    if (readFromAvailableText()) {
        std::cout << "hw04_settings.txt successfully loaded.\n";
    } else {
        std::cout << "hw04_settings.txt not loaded. Using default values.\n";
    }
    if (!audioObject.textObject.font.loadFromFile(fontFileName)) {
        return false;
    }
    audioObject.textObject.text.setString(help_text);
    audioObject.textObject.text.setFont(audioObject.textObject.font);
    audioObject.textObject.text.setCharacterSize(fontSize);
    if (!audioObject.music.openFromFile(musicFileName)) {
        return false;
    }

    if (!audioObject.soundBuffer.loadFromFile(sfxFileName)) {
        return false;
    }
    audioObject.sound.setBuffer(audioObject.soundBuffer);
    return true;
}

void helperFunction() {
    sf::SoundSource::Status musicState = audioObject.music.getStatus();
    if (musicState == sf::SoundSource::Status::Paused || musicState == sf::SoundSource::Status::Stopped) {
        audioObject.music.play();
    } else if (musicState == sf::SoundSource::Status::Playing) {
        audioObject.music.pause();
    } else {
        std::cout << "UNKNOWN MUSIC STATE--ERROR\n";
    }
}

void pressEvents(sf::RenderWindow& window, const sf::Event& event) {
    switch (event.key.code) {
        case sf::Keyboard::Escape:
            audioObject.music.stop();
            audioObject.sound.stop();
            window.close();
            break;
        // unique commands
        case sf::Keyboard::H:
            helpFlag = !helpFlag;
            break;
        case sf::Keyboard::R:
            audioObject.music.stop();
            audioObject.music.play();
            break;
        case sf::Keyboard::Q:
            helperFunction();
            break;
        case sf::Keyboard::E:
            audioObject.sound.play();
            break;
        // common commands
        case sf::Keyboard::W:
            musicFlags[static_cast<unsigned int>(Direction::up)] = true;
            break;
        case sf::Keyboard::A:
            musicFlags[static_cast<unsigned int>(Direction::left)] = true;
            break;
        case sf::Keyboard::S:
            musicFlags[static_cast<unsigned int>(Direction::down)] = true;
            break;
        case sf::Keyboard::D:
            musicFlags[static_cast<unsigned int>(Direction::right)] = true;
            break;
        case sf::Keyboard::I:
            sfxFlags[static_cast<unsigned int>(Direction::up)] = true;
            break;
        case sf::Keyboard::J:
            sfxFlags[static_cast<unsigned int>(Direction::left)] = true;
            break;
        case sf::Keyboard::K:
            sfxFlags[static_cast<unsigned int>(Direction::down)] = true;
            break;
        case sf::Keyboard::L:
            sfxFlags[static_cast<unsigned int>(Direction::right)] = true;
            break;
        default:
            // nothing
            break;
    }
}

void releaseEvents(sf::RenderWindow& window, const sf::Event& event) {
    switch (event.key.code) {
        case sf::Keyboard::W:
            musicFlags[static_cast<unsigned int>(Direction::up)] = false;
            break;
        case sf::Keyboard::A:
            musicFlags[static_cast<unsigned int>(Direction::left)] = false;
            break;
        case sf::Keyboard::S:
            musicFlags[static_cast<unsigned int>(Direction::down)] = false;
            break;
        case sf::Keyboard::D:
            musicFlags[static_cast<unsigned int>(Direction::right)] = false;
            break;
        case sf::Keyboard::I:
            sfxFlags[static_cast<unsigned int>(Direction::up)] = false;
            break;
        case sf::Keyboard::J:
            sfxFlags[static_cast<unsigned int>(Direction::left)] = false;
            break;
        case sf::Keyboard::K:
            sfxFlags[static_cast<unsigned int>(Direction::down)] = false;
            break;
        case sf::Keyboard::L:
            sfxFlags[static_cast<unsigned int>(Direction::right)] = false;
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
            case sf::Event::KeyReleased:
                releaseEvents(window, event);
                break;
            default:
                // nothing
                break;
        }
    }
}

void update(const sf::Time& elapsed) {
    float delta = elapsed.asSeconds();

    if (helpFlag) {
        audioObject.textObject.text.setFillColor(sf::Color::White);
    } else {
        audioObject.textObject.text.setFillColor(sf::Color::Black);
    }

    if (musicFlags[static_cast<unsigned int>(Direction::up)]) music_volume += 2.5f;
    if (musicFlags[static_cast<unsigned int>(Direction::left)]) music_pitch -= 0.1f;
    if (musicFlags[static_cast<unsigned int>(Direction::down)]) music_volume -= 2.5f;
    if (musicFlags[static_cast<unsigned int>(Direction::right)]) music_pitch += 0.1f;

    if (sfxFlags[static_cast<unsigned int>(Direction::up)]) sfx_volume += 2.5f;
    if (sfxFlags[static_cast<unsigned int>(Direction::left)]) sfx_pitch -= 0.1f;
    if (sfxFlags[static_cast<unsigned int>(Direction::down)]) sfx_volume -= 2.5f;
    if (sfxFlags[static_cast<unsigned int>(Direction::right)]) sfx_pitch += 0.1f;

    // if your compiler doesn't have clamp, use the utility namespace
    music_volume = std::clamp(music_volume, 0.f, 100.f);
    music_pitch = std::max(music_pitch, 0.1f);
    sfx_volume = std::clamp(sfx_volume, 0.f, 100.f);
    sfx_pitch = std::max(sfx_pitch, 0.1f);

    audioObject.music.setVolume(music_volume);
    audioObject.music.setPitch(music_pitch);
    audioObject.sound.setVolume(sfx_volume);
    audioObject.sound.setPitch(sfx_pitch);
}

void render(sf::RenderWindow& window) {
    window.clear(sf::Color::Black);
    window.draw(audioObject.textObject.text);
    window.display();
}

int main() {
    if (!initializeSettings()) {
        std::cout << "Information provided not complete. Exiting.";
        return 0;
    }
    audioObject.music.play();

    sf::RenderWindow window(sf::VideoMode(window_w, window_h), "HW04 - Audio");

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
