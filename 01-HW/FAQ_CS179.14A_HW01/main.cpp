#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cctype>
#include <map>
#include <functional>

unsigned int MAX_ROWS = 10;
unsigned int MAX_COLUMNS = 10;

struct Position {
	int _r, _c;
	Position(int r = -1, int c = -1): _r(r), _c(c) {}
	bool operator==(const Position& rhs) const {
		return _r == rhs._r && _c == rhs._c;
	}
	friend std::istream& operator>>(std::istream& i, Position& p) {
		return i >> p._r >> p._c;
	}
	bool inBounds(int rows, int columns) {
		return 0 <= _r && _r < rows && 0 <= _c && _c < columns;
	}
};

std::vector<std::vector<char>> minimap;
Position xPosition, oPosition;
std::map<std::string, std::function<void()>> commands;

void toLowercase(std::string& s) {
	for (int i = 0; i < s.size(); ++i) {
		s[i] = tolower(s[i]);
	}
}

bool readFromAvailableText() {
	std::string input;
	std::ifstream settings("settings.txt");
	if (settings.is_open()) {
		settings >> MAX_ROWS >> MAX_COLUMNS >> oPosition >> xPosition;
		settings.close();
		return true;
	} else {
		return false;
	}
}

void processUserInput() {
	bool validRows = false;
	while (!validRows) {
		std::cout << "Input the maximum amount of rows the board has: ";
		std::cin >> MAX_ROWS;
		if (MAX_ROWS > 0) {
			validRows = true;
		} else {
			std::cout << "Please enter a non-negative, non-zero number.\n";
		}
	}
	
	bool validColumns = false;
	while (!validColumns) {
		std::cout << "Input the maximum amount of columns the board has: ";
		std::cin >> MAX_COLUMNS;
		if (MAX_COLUMNS > 0) {
			validColumns = true;
		} else {
			std::cout << "Please enter a non-negative, non-zero number.\n";
		}
	}
	

	bool validPlayerPosition = false;
	while (!validPlayerPosition) {
		std::cout << "Input the player's position (input as \"<row> <column>\" without quotes): ";
		std::cin >> oPosition;
		if (oPosition.inBounds(MAX_ROWS, MAX_COLUMNS)) {
			validPlayerPosition = true;
		} else {
			std::cout << "Player position is invalid.\n";
		}
	}

	bool validTargetPosition = false;
	while (!validTargetPosition) {
		std::cout << "Input the target's position (input as \"<row> <column>\" without quotes): ";
		std::cin >> xPosition;
		if (xPosition.inBounds(MAX_ROWS, MAX_COLUMNS)) {
			validTargetPosition = true;
		} else {
			std::cout << "Target position is invalid.\n";
		}
	}
}

void initializeSettings() {
	char input;
	bool validInput = false;
	while (!validInput) {
		std::cout << "Select input option (1/2/3):\n";
		std::cout << "1. settings.txt\n";
		std::cout << "2. User input\n";
		std::cout << "3. Default settings\n";
		std::cin >> input;
		switch (input) {
			case '1':
				if (readFromAvailableText()) {
					std::cout << "settings.txt found.\n";
					validInput = true;
				} else {
					std::cout << "settings.txt not found.\n";
				}
				break;
			case '2':
				processUserInput();
				validInput = true;
				break;
			case '3':
				MAX_ROWS = MAX_COLUMNS = 3;
				xPosition = {MAX_ROWS / 2, MAX_COLUMNS / 2};
				oPosition = {MAX_ROWS - 1, MAX_COLUMNS - 1};
				std::cout << "Using default settings.\n";
				validInput = true;
				break;
			default:
				std::cout << "Invalid input.\n";
				break;
		}
	}
}



void showMinimap(const Position& xPosition, const Position& oPosition) {
	// setting the x and o Position on the board
	minimap[xPosition._r * 3 + 1][xPosition._c * 3 + 1] = 'X';
	minimap[oPosition._r * 3 + 2][oPosition._c * 3 + 2] = 'O';
	
	for (int i = 0; i < MAX_ROWS * 3 + 1; ++i) {
		for (int j = 0; j < MAX_COLUMNS * 3 + 1; ++j) {
			std::cout << minimap[i][j];
		}
		std::cout << '\n';
	}

	// resetting the x and o Position on the board
	minimap[xPosition._r * 3 + 1][xPosition._c * 3 + 1] = ' ';
	minimap[oPosition._r * 3 + 2][oPosition._c * 3 + 2] = ' ';
}

void clearMinimap() {
	for (int i = 0; i < MAX_ROWS * 3 + 1; ++i) {
		for (int j = 0; j < MAX_COLUMNS * 3 + 1; ++j) {
			minimap[i][j] = ' ';
		}
	}
}

void prepareBoard() {
	for (int i = 0; i < MAX_ROWS * 3 + 1; ++i) {
		minimap[i][0] = minimap[i][MAX_COLUMNS * 3] = (i % 3 == 0 ? '+' : '|');
	}
	for (int i = 0; i < MAX_COLUMNS * 3 + 1; ++i) {
		minimap[0][i] = minimap[MAX_ROWS * 3][i] = (i % 3 == 0 ? '+' : '-');
	}

	for (int i = 0; i < MAX_ROWS+1; ++i) {
		for (int j = 0; j < MAX_COLUMNS+1; ++j) {
			minimap[i * 3][j * 3] = '+';
		}
	}
}

void initializeCommands(bool &exitGame) {
	commands["north"] = [](){
		oPosition._r = (oPosition._r - 1 + MAX_ROWS) % MAX_ROWS;
	};
	commands["south"] = [](){
		oPosition._r = (oPosition._r + 1 + MAX_ROWS) % MAX_ROWS;
	};
	commands["east"] = [](){
		oPosition._c = (oPosition._c + 1 + MAX_COLUMNS) % MAX_COLUMNS;
	};
	commands["west"] = [](){
		oPosition._c = (oPosition._c - 1 + MAX_COLUMNS) % MAX_COLUMNS;
	};
	commands["attack"] = [&exitGame](){
		if (oPosition == xPosition) {
			std::cout << "Target attacked!\n";
			exitGame = true;
		} else {
			std::cout << "There's nothing to attack here!\n";
		}
	};

	for (const auto& command : commands) {
		commands[command.first.substr(0,1)] = command.second;
	}

	commands["exit"] = [&exitGame](){
		std::cout << "Goodbye!\n";
		exitGame = true;
	};
	commands["x"] = commands["exit"];
}

int main() {
	// initialize
	initializeSettings();
	std::cout << "Grid size is " << MAX_ROWS << " x " << MAX_COLUMNS << ".\n";
	minimap.resize(3 * MAX_ROWS + 1);
	for (int i = 0; i < 3 * MAX_ROWS + 1; ++i) {
		minimap[i].resize(3 * MAX_COLUMNS + 1);
	}
	clearMinimap();
	prepareBoard();
	std::string input;
	bool exitGame = false;
	initializeCommands(exitGame);

	// game loop
	while (true) {
		// show state
		showMinimap(xPosition, oPosition);
		std::cout << "You are in room " << oPosition._r * MAX_COLUMNS + oPosition._c << ".\n";
		if (oPosition == xPosition) {
			std::cout << "Your target is here!\n";
		}

		// get input
		std::cout << "> ";
		std::getline(std::cin >> std::ws, input);
		toLowercase(input);

		// process input
		auto command = commands.find(input);
		if (command != commands.end()) {
			command->second();
		} else {
			std::cout << "Invalid command.\n";
		}

		if (exitGame) {
			break;
		}
	}

	std::cout << "Thanks for playing!\n";
	return 0;
}