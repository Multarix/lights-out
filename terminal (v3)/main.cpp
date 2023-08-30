#include <iostream>
#include <algorithm>
#include <regex>
#include <string>
#include "grid.cpp"



void validateInput(std::string& input, int& expectedSize) {
	input.erase(std::remove(input.begin(), input.end(), ' '), input.end()); // Remove whitespace from string

	// Use regex to check if the string contains anything other than 1's or 0's
	std::regex binaryValidation("^[01]+$");
	if (!std::regex_match(input, binaryValidation)) throw "The input contains invalid characters, please try again.";
	if (input.size() != expectedSize) throw "The input does not appear to have the correct amount of tiles, please try again.";

	return;
}


State getStateToSolveFrom(int expectedSize) {
	State startingState;

	std::cout << "Please enter the state of the board, left to right, top to bottom.\n1 Means correct state, 0 means incorrect. eg: 010 111 010" << std::endl;
	while (true) {
		try {
			std::cout << "Input: ";
			std::cin.clear(); // Make sure the input is clear
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			// Get input from user
			std::string input;
			std::getline(std::cin, input);


			validateInput(input, expectedSize);

			startingState = std::stoi(input, 0, 2);
			break;

		} catch (const char* error) {
			std::cout << error << "\n" << std::endl; // If invalid, throw an error, which we catch
		}
	}

	// Return input if valid
	return startingState;
}


bool getYesNo() {
	bool input;
	while (true) {
		std::cout << "Input (Y/N): ";
		std::cin.clear(); // Make sure the input is clear
		std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

		std::string response;
		std::cin >> response;

		char letter = std::tolower(response[0]);
		if (letter == 'y') {
			input = true;
			break;
		}

		if (letter == 'n') {
			input = false;
			break;
		}

		std::cout << "Invalid Input!" << std::endl;
	}

	return input;
}


void doThreeByThree(LightsOutGrid& normal, LightsOutGrid& modded) {
	std::cout << "Does the game use the modified ruleset? " << std::endl;
	bool answer = getYesNo();
	LightsOutGrid& board = (answer) ? modded : normal;

	State stateToSolve = getStateToSolveFrom(9);

	bool solution = board.getSolution(stateToSolve);

	return;
}


void doFourByFour(LightsOutGrid& board) {
	State stateToSolve = getStateToSolveFrom(16);
	bool solution = board.getSolution(stateToSolve);

	return;
}

void doFiveByFive(LightsOutGrid& board) {
	if (!board.hasSolutions()) {
		std::cout << "Running the 5x5 calculation for the first time can take up to a minute.\nDo you wish to proceed?" << std::endl;

		bool answer = getYesNo();

		if (!answer) return;
		std::cout << "Calculating all states please wait..." << std::endl;
		board.bruteForceSolutions();
		board.waitForThreads();
	}

	State stateToSolve = getStateToSolveFrom(25);
	bool solution = board.getSolution(stateToSolve);

	return;
}


void waitForInitialThreads(LightsOutGrid& board1, LightsOutGrid& board2, LightsOutGrid& board3) {
	if (!board1.hasSolutions() && board2.hasSolutions() && board3.hasSolutions()) return;

	board1.waitForThreads();
	board2.waitForThreads();
	board3.waitForThreads();

	return;
}


int main() {
	LightsOutGrid Board3Normal(3, false, 1);
	LightsOutGrid Board3Modified(3, true,  1);
	LightsOutGrid Board4(4, true,  2);
	LightsOutGrid Board5(5, false, 4);

	Board3Normal.bruteForceSolutions();
	Board3Modified.bruteForceSolutions();
	Board4.bruteForceSolutions();

	while (true) {
		std::cout << "What size board would you like to solve? (3, 4, 5)" << std::endl;

		while (true) {
			std::cout << "Input: ";

			std::string sizeOfBoard;
			std::cin >> sizeOfBoard;

			waitForInitialThreads(Board3Normal, Board3Modified, Board4);

			if (sizeOfBoard == "3") {
				std::cout << "3x3 game selected!" << std::endl;
				doThreeByThree(Board3Normal, Board3Modified);
				break;
			}

			if (sizeOfBoard == "4") {
				std::cout << "4x4 game selected!" << std::endl;
				doFourByFour(Board4);
				break;
			}

			if(sizeOfBoard == "5") {
				std::cout << "5x5 game selected!" << std::endl;
				doFiveByFive(Board5);
				break;
			}


			std::cout << "Invalid response, please try again!" << std::endl;

			std::cin.clear(); // Make sure the input is clear
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
		}
	}

	return 0;
}