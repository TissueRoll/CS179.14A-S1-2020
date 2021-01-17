#include <iostream>
#include <cstdlib>
#include <ctime>
#include <math.h>
#include <cmath>
#include <SFML/Graphics.hpp>

constexpr unsigned int FPS{60};
constexpr unsigned int CSIZE{5};

using namespace std;

int main( )
{
    sf::RenderWindow window( sf::VideoMode( 200, 200 ), "SFML works!" );
    sf::CircleShape shape( 100.f );
    sf::Time change = sf::seconds(1.f);

	sf::Color color[CSIZE];
	color[0] = sf::Color::Green;
	color[1] = sf::Color::Blue;
	color[2] = sf::Color::Red;
	color[3] = sf::Color::Cyan;
    color[4] = sf::Color::Magenta;
    
	srand(time(NULL));
	unsigned int curr = rand() % CSIZE;
    shape.setFillColor( color[curr] );
	window.setFramerateLimit( FPS );
	
	sf::Clock clock;
	sf::Time total;
	sf::Time timeSinceLastUpdate;

	double integerPart, fractionalPart;

    while( window.isOpen() )
    {
		sf::Time elapsed = clock.restart();
		total += elapsed;
		timeSinceLastUpdate += elapsed;

		fractionalPart = modf(elapsed.asSeconds(), &integerPart);
		printf("Time of last iteration: %g seconds and %.3g milliseconds\n", ceil(integerPart), ceil(fractionalPart * 1000));

		fractionalPart = modf(total.asSeconds(), &integerPart);
		printf("Total elapsed time: %g seconds and %.3g milliseconds\n", ceil(integerPart), ceil(fractionalPart * 1000));

		// input
        sf::Event event;
        while( window.pollEvent( event ) )
        {
            switch( event.type )
			{
			case sf::Event::Closed:
                window.close();
				break;
			// case sf::Event::MouseButtonPressed:
			// 	if( event.mouseButton.button == sf::Mouse::Left )
			// 	{
			// 		curr = (curr + 1) % CSIZE;
			// 		shape.setFillColor( color[curr] );
			// 	}
			// 	break;
			case sf::Event::KeyPressed:
				if( event.key.code == sf::Keyboard::Escape )
				{
					window.close();
				}
				break;
			}
        }

        // update
        while (timeSinceLastUpdate >= change) {
        	curr = rand() % CSIZE;
        	shape.setFillColor( color[curr] );
        	timeSinceLastUpdate -= change; // in case of lag spikes for some reason, this will attempt to catch up
        }

        // render
        window.clear();
        window.draw(shape);
        window.display();
    }

    return 0;
}