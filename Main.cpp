#include <algorithm>
#include <cctype>
#include <format>
#include "Game.hpp"
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

using namespace ExperisBowling;

// creates a string to present the bowling game in a user-friendly format, like a console
static std::string FormatScore(Game const& game) {
	std::stringstream ss;
	unsigned currentFrameIndex = game.GetCurrentRoundIndex();

	for (unsigned i = 0u; i < game.FinalFrame; i++) {
		if (i == currentFrameIndex) {
			ss << std::string(48, 'v') << "\n";
		}

		ss << "Round " << std::setw(2) << i + 1u << " - " << "[";

		if (game.GetFrame(i).isStrike) {
			ss << " X" << ", " << " _";
		}
		else {
			ss << std::setw(2) << game.GetFrame(i).pinsOnFirstRoll.value_or(0u) << ", " << std::setw(2);

			if (game.GetFrame(i).isSpare) {
				ss << "/";
			}
			else if (game.GetFrame(i).isStrike) {
				ss << "X";
			}
			else {
				ss << game.GetFrame(i).pinsOnSecondRoll.value_or(0u);
			}
		}


		if (i == game.FinalFrame - 1u) {
			ss << ", ";

			unsigned pins = game.GetFrame(i + 1u).pinsOnFirstRoll.value_or(0u);
			if (pins == Game::NumPins) {
				ss << "X";
			}
			else {
				ss << pins;
			}

			ss << "] ";
		}
		else {
			ss << "] " << std::string(3, ' ');
		}

		ss << "Current: " << std::setw(3) << game.GetFrame(i).currentScore;
		ss << ", Total: " << std::setw(3) << game.GetFrame(i).totalScore << "\n";
		if (i == currentFrameIndex) {
			ss << std::string(48, '^') << "\n";
		}
	}

	return ss.str();
}

// executes the given bowling game example - strike and spare calls can be replaced with 10s and appropriate numbers, respectively
consteval Game RunExampleGame() {
	Game ex;
	ex.Roll(8); ex.RollSpare();
	ex.Roll(5); ex.Roll(4);
	ex.Roll(9); ex.Roll(0);
	ex.RollStrike();
	ex.RollStrike();
	ex.Roll(5); ex.RollSpare();
	ex.Roll(5); ex.Roll(3);
	ex.Roll(6); ex.Roll(3);
	ex.Roll(9); ex.RollSpare();
	ex.Roll(9); ex.RollSpare(); ex.RollStrike();

	return ex;
}

int main() {
	std::cout << "=== Example game ===\n";
	constexpr Game ex = RunExampleGame();
	std::cout << FormatScore(ex) << "\n";

	std::cout << "=== Main game ===\n";
	std::cout << "Type 'q' to quit the game.\n";
	std::cout << "Type 'r' to reset the game.\n";
	std::cout << "Type a number 0-9 to bowl. x for strike, / for spare.\n";

	Game game;
	while (true) {
		std::cout << "\n" << FormatScore(game) << "\n";
		if (game.IsGameComplete()) {
			std::cout << "\n=== Game complete. Starting a new one. ===\n";
			game = Game();
			continue;
		}

		std::string s;
		std::cin >> s;

		std::transform(s.begin(), s.end(), s.begin(), [](char c) { return static_cast<char>(std::tolower(c)); });

		if (s.empty() || s == "q" || s == "quit" || s == "exit" || s == "stop") {
			break;
		}

		if (s == "r" || s == "reset" || s == "restart") {
			game = Game();
			continue;
		}

		if (s == "x") {
			std::cout << game.RollStrike().value_or("") << "\n";
			continue;
		}

		if (s == "/") {
			std::cout << game.RollSpare().value_or("") << "\n";
			continue;
		}

		if (std::isdigit(s[0])) {
			std::cout << game.Roll(std::stoul(s)).value_or("") << "\n";
			continue;
		}

		std::cout << "Invalid input\n";
	}

	std::cout << "\n" << FormatScore(game) << "\n";

	return 0;
}