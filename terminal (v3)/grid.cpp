#include <stdexcept>
#include <mutex>
#include <chrono>
#include <thread>
#include <tuple>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <bitset>
#include <iostream>
#include <string>


typedef int MovesTillSolved;
typedef int OptimalMove;
typedef unsigned int State;
typedef std::vector<bool> Binary;

typedef std::tuple<State, OptimalMove> Permutation;
typedef std::vector<std::vector<bool>> Matrix;
typedef std::vector<std::vector<int>> LookUpMatrix;
typedef std::vector<State> Rotations;

typedef std::tuple<State, MovesTillSolved, OptimalMove, State> QueueItem; // state, movesTillSolved, optimalMove, parent
typedef std::tuple<MovesTillSolved, OptimalMove, State> MemoryItem; // movesTillSolved, optimalMove, parent


// This entire class is just an excuse to move the functions into a separate file
class LightsOutGrid {
private:
	/* The tiles on each side of the grid*/
	int gridSize;
	int threadAllocation;
	int totalTiles;

	double maxStates;
	double stoppingPoint;

	bool isModded;
	bool solutionsFound = false;
	bool threadEnabled = false;

	std::vector<std::thread> threadList;

	std::unordered_set<State> inQueue;
	std::queue<QueueItem> theQueue; // state, move, optimalMove, parent
	std::unordered_map<State, MemoryItem> memory; // state | move, optimalMove, parent


	std::mutex queueLock;
	std::mutex memoryLock;

	/* Creates a vector of bools that represent binary */
	Binary convertToBinary(State& state) {
		std::vector<bool> binary;

		State newState = state;

		while (newState > 0) {
			int bit = newState % 2;
			binary.push_back(bit == 1);

			newState -= bit;
			newState /= 2;
		}

		while (binary.size() < LightsOutGrid::totalTiles) {
			binary.push_back(false);
		}

		return binary;
	}


	/* Reverses the binary of a number */
	State reverseBinary(State& state) {
		Binary bits = LightsOutGrid::convertToBinary(state);

		State reversed = 0;

		int i = bits.size() - 1;
		for (bool bit : bits) {
			reversed += (bit) ? std::pow(2, i) : 0;
			i--;
		}

		//std::cout << "Original: " << state << " Reversed: " << reversed << std::endl;

		return reversed;
	}


	/* Shrinks a 2D Boolean Vector into number */
	State shrinkToBinary(Matrix& matrix) {
		State state = 0;

		int index = 0;
		for (std::vector<bool> row : matrix) {
			for (bool bit : row) {
				state += (bit) ? std::pow(2, index) : 0;
				index++;
			}
		}

		return state;
	}

	/* Flattens a 2D int matrix */
	std::vector<int> flattenMatrix(LookUpMatrix& matrix) {
		std::vector<int> flattened;

		int index = 0;
		for (std::vector<int> row : matrix) {
			for (int col : row) {
				flattened.push_back(col);
			}
		}

		return flattened;
	}

	std::string makeDiagram(int& tile) {
		std::string rowDelimiter = std::string((LightsOutGrid::gridSize * 4) + 1, '-');
		std::string eTile = "|   ";
		std::string fTile = "| x ";

		std::string diagram = "";
		for (int i = 0; i < LightsOutGrid::gridSize; i++) {
			diagram += rowDelimiter + "\n";

			for (int j = 0; j < LightsOutGrid::gridSize; j++) diagram += (i * LightsOutGrid::gridSize + j == tile) ? fTile : eTile;

			diagram += "|\n";
		}

		diagram += rowDelimiter;

		/* Example:
		"-------------"
		"|   | x |   |"
		"-------------"
		"|   |   |   |"
		"-------------"
		"|   |   |   |"
		"-------------"
		*/

		return diagram;
	}


	/* Because reinventing the wheel is stupid, the code below is taken from: https://www.prodevelopertutorial.com/given-an-n-x-n-2d-matrix-rotate-it-by-90-degrees-clockwise-in-c-in-place/ */
	State rotateState(Matrix& flipMatrix) {
		size_t size = flipMatrix.size();

		/*Flip Diagonally*/
		for (size_t i = 0; i < size; i++) {
			for (size_t j = i; j < size; j++) {
				swap(flipMatrix[i][j], flipMatrix[j][i]);
			}
		}

		/* flip horizontally  */
		for (size_t i = 0; i < size; i++) {
			reverse(flipMatrix[i].begin(), flipMatrix[i].end());
		}

		return LightsOutGrid::shrinkToBinary(flipMatrix);
	}


	/* Because reinventing the wheel is stupid, the code below is taken from: https://www.prodevelopertutorial.com/given-an-n-x-n-2d-matrix-rotate-it-by-90-degrees-clockwise-in-c-in-place/ */
	void rotateLookup(LookUpMatrix& flipMatrix) {
		size_t size = flipMatrix.size();

		/*Flip Diagonally*/
		for (size_t i = 0; i < size; i++) {
			for (size_t j = i; j < size; j++) {
				std::swap(flipMatrix[i][j], flipMatrix[j][i]);
			}
		}

		/* flip horizontally  */
		for (size_t i = 0; i < size; i++) {
			reverse(flipMatrix[i].begin(), flipMatrix[i].end());
		}
	}


	Matrix expandToMatrix(State& state) {
		Binary bits = LightsOutGrid::convertToBinary(state);

		Matrix matrix;

		for (int i = LightsOutGrid::gridSize - 1; i >= 0; i--) {
			std::vector<bool> row;

			for (int j = LightsOutGrid::gridSize - 1; j >= 0; j--) {
				row.push_back(bits[i * LightsOutGrid::gridSize + j]);
			};

			matrix.push_back(row);
		}

		return matrix;
	}


	LookUpMatrix createRotationMatrix(int& numberOfRotations) {
		LookUpMatrix rotationMatrix;

		for (int i = 0; i < LightsOutGrid::gridSize; i++) {
			std::vector<int> row;

			for (int j = 0; j < LightsOutGrid::gridSize; j++) row.push_back(i * LightsOutGrid::gridSize + j);

			rotationMatrix.push_back(row);
		}


		for (int i = 0; i < numberOfRotations; i++) LightsOutGrid::rotateLookup(rotationMatrix);

		return rotationMatrix;
	}


	int getRotationFactor(State& state, Rotations& rotations) {
		int rotationFactor = -1;

		for (int i = 0; i < 4; i++) {
			State rotation = rotations[i];

			if (LightsOutGrid::memory.find(rotation) != LightsOutGrid::memory.end()) {
				rotationFactor = i;
				break;
			}
		}

		return rotationFactor;
	}


	Rotations getRotations(State& state) {
		Rotations rotations;
		rotations.push_back(state);

		Matrix matrix = LightsOutGrid::expandToMatrix(state);

		State rotatedState = LightsOutGrid::rotateState(matrix);
		if (rotatedState == state) return rotations; // Length of 1;
		rotations.push_back(rotatedState);

		State reverseState = LightsOutGrid::reverseBinary(state);
		if (reverseState == state) return rotations; // Length of 2;

		rotations.push_back(reverseState);
		rotations.push_back(LightsOutGrid::reverseBinary(rotatedState));

		return rotations; // Length of 4;
	}


	State getModdedMask(int& tile) {
		std::string singleOne = "1";
		std::string doubleOne = singleOne + singleOne;
		std::string tripleOne = doubleOne + singleOne;

		int topLeft = 0;
		int topRight = LightsOutGrid::gridSize - 1;
		int bottomLeft = LightsOutGrid::totalTiles - (LightsOutGrid::gridSize);
		int bottomRight = LightsOutGrid::totalTiles - 1;

		std::string gridSizeMinus2Zeros = std::string(LightsOutGrid::gridSize - 2, '0');
		std::string gridSizeMinus3Zeros = std::string(LightsOutGrid::gridSize - 3, '0');


		// Handle Corners
		if (tile == topLeft || tile == topRight || tile == bottomLeft || tile == bottomRight) {
			State corner = std::stoi(doubleOne + gridSizeMinus2Zeros + doubleOne, 0, 2);

			if (tile == topLeft) return (corner << ((LightsOutGrid::gridSize - 2) + ((LightsOutGrid::gridSize - 2) * LightsOutGrid::gridSize)));	// Top left
			if (tile == topRight) return (corner << (LightsOutGrid::totalTiles - (LightsOutGrid::gridSize * 2)));					// Top right
			if (tile == bottomLeft) return (corner << (LightsOutGrid::gridSize - 2));									// Bottom Left
			if (tile == bottomRight) return corner;													// Bottom Right
		}

		State topBottom = std::stoi(tripleOne + gridSizeMinus3Zeros + tripleOne, 0, 2);
		State leftRight = std::stoi(doubleOne + gridSizeMinus2Zeros + doubleOne + gridSizeMinus2Zeros + doubleOne, 0, 2);

		// Handle top side clicks
		if (topLeft < tile && topRight > tile) return (topBottom << ((LightsOutGrid::totalTiles - 1) - (tile + LightsOutGrid::gridSize + 1)));

		// Handle left side clicks
		if (tile % LightsOutGrid::gridSize == 0) return (leftRight << ((LightsOutGrid::totalTiles - 1) - (tile + LightsOutGrid::gridSize + 1)));

		// Handle right side clicks
		if (tile % LightsOutGrid::gridSize == LightsOutGrid::gridSize - 1) return (leftRight << ((LightsOutGrid::totalTiles - 1) - (tile + LightsOutGrid::gridSize)));

		// Handle bottom side clicks
		if (bottomLeft < tile && bottomRight > tile) return (topBottom << ((LightsOutGrid::totalTiles - 1) - (tile + 1)));

		// Everything else
		return (std::stoi(tripleOne + gridSizeMinus3Zeros + tripleOne + gridSizeMinus3Zeros + tripleOne, 0, 2) << ((LightsOutGrid::totalTiles - 1) - (tile + LightsOutGrid::gridSize + 1)));
	}


	State getNormalMask(int& tile) {
		std::string singleOne = "1";
		std::string doubleOne = singleOne + singleOne;
		std::string tripleOne = doubleOne + singleOne;

		int topLeft = 0;
		int topRight = LightsOutGrid::gridSize - 1;
		int bottomLeft = LightsOutGrid::totalTiles - (LightsOutGrid::gridSize);
		int bottomRight = LightsOutGrid::totalTiles - 1;

		std::string gridSizeMinus1Zeros = std::string(LightsOutGrid::gridSize - 1, '0');
		std::string gridSizeMinus2Zeros = std::string(LightsOutGrid::gridSize - 2, '0');


		// Handle Corners
		if (tile == topLeft || tile == topRight || tile == bottomLeft || tile == bottomRight) {
			if (tile == topLeft) return (std::stoi(doubleOne + gridSizeMinus2Zeros + singleOne, 0, 2) << ((LightsOutGrid::totalTiles - (LightsOutGrid::gridSize + 1))));	// Top left
			if (tile == topRight) return (std::stoi(doubleOne + gridSizeMinus1Zeros + singleOne, 0, 2) << (LightsOutGrid::totalTiles - (LightsOutGrid::gridSize * 2)));		// Top right
			if (tile == bottomLeft) return std::stoi(singleOne + gridSizeMinus1Zeros + doubleOne, 0, 2) << (LightsOutGrid::gridSize - 2);									// Bottom Left
			if (tile == bottomRight) return std::stoi(singleOne + gridSizeMinus2Zeros + doubleOne, 0, 2);																	// Bottom Right
		}

		// Handle top side clicks
		if (topLeft < tile && topRight > tile) return (std::stoi(tripleOne + gridSizeMinus2Zeros + singleOne, 0, 2) << (((LightsOutGrid::gridSize - tile) - 1) + ((LightsOutGrid::gridSize - 2) * LightsOutGrid::gridSize)));

		// Handle left side clicks
		if (tile % LightsOutGrid::gridSize == 0) return (std::stoi(singleOne + gridSizeMinus1Zeros + doubleOne + gridSizeMinus2Zeros + singleOne, 0, 2) << ((LightsOutGrid::gridSize - 1) + (((LightsOutGrid::gridSize - (tile / LightsOutGrid::gridSize)) - 2) * LightsOutGrid::gridSize)));

		// Handle right side clicks
		if (tile % LightsOutGrid::gridSize == LightsOutGrid::gridSize - 1) return (std::stoi(singleOne + gridSizeMinus2Zeros + doubleOne + gridSizeMinus1Zeros + singleOne, 0, 2) << (((LightsOutGrid::gridSize - ((tile + 1) / LightsOutGrid::gridSize)) - 1) * LightsOutGrid::gridSize));

		// Handle bottom side clicks
		if (bottomLeft < tile && bottomRight > tile) return (std::stoi(singleOne + gridSizeMinus2Zeros + tripleOne, 0, 2) << ((LightsOutGrid::totalTiles - 2) - tile));

		// Everything else
		return (std::stoi(singleOne + gridSizeMinus2Zeros + tripleOne + gridSizeMinus2Zeros + singleOne, 0, 2) << ((LightsOutGrid::totalTiles - 1) - (tile + LightsOutGrid::gridSize)));
	}


	std::vector<Permutation> getPermutations(State& state) {
		std::vector<Permutation> permutations;

		for (int i = 0; i < LightsOutGrid::totalTiles; i++) {
			State mask = (isModded) ? LightsOutGrid::getModdedMask(i) : LightsOutGrid::getNormalMask(i);
			permutations.push_back(std::make_tuple(state ^ mask, i));
		}

		return permutations;
	}


	bool checkRotations(State& state) {
		bool rotated = false;
		Rotations rotations = LightsOutGrid::getRotations(state);

		//std::cout << "Checking rotations for " << state << "(" << rotations.size() << ")" << std::endl;
		//for (State rot : rotations) {
		//	std:: cout << state << " rotated -> " << rot << std::endl;
		//}

		for (State rotation : rotations) {
			queueLock.lock();
			bool queueCheck = (inQueue.find(rotation) != inQueue.end());
			queueLock.unlock();
			if (queueCheck) return true;

			memoryLock.lock();
			bool memoryCheck = (memory.find(rotation) != memory.end());
			memoryLock.unlock();
			if (memoryCheck) return true;
		}

		return false;
	}


	void permutationChecker(Permutation& perm, int& move, State& state) {
		State newState = std::get<0>(perm);
		OptimalMove optimalMove = std::get<1>(perm);

		memoryLock.lock();
		bool memoryFound = memory.find(newState) != memory.end();
		memoryLock.unlock();
		if (memoryFound) return;

		queueLock.lock();
		bool queueCheck = (inQueue.find(newState) != inQueue.end());
		queueLock.unlock();
		if (queueCheck) return;

		if (LightsOutGrid::checkRotations(newState)) return;

		QueueItem queueItem = std::make_tuple(newState, move + 1, optimalMove, state);
		queueLock.lock();
		theQueue.emplace(queueItem);
		inQueue.insert(newState);
		queueLock.unlock();

		return;
	}


	bool keepGoing() {

		/*queueLock.lock();
		bool queueEmpty = (theQueue.size() == 0);
		queueLock.unlock();

		if (queueEmpty) return false;
		return true;*/

		memoryLock.lock();
		bool answer = (memory.size() < LightsOutGrid::stoppingPoint);
		memoryLock.unlock();

		return answer;
	}


	void processQueue() {

		while (keepGoing()) {
			queueLock.lock();

			//std::cout << theQueue.size() << std::endl;
			//std::cout << memory.size() << " Max: " << LightsOutGrid::stoppingPoint << std::endl;

			bool shouldContinue = (theQueue.size() == 0);
			queueLock.unlock();

			if (shouldContinue) continue;

			queueLock.lock();
			QueueItem item = theQueue.front();
			theQueue.pop();

			//inQueue.erase(get<0>(item));
			queueLock.unlock();

			State state = std::get<0>(item);
			MovesTillSolved move = std::get<1>(item);
			OptimalMove optimalMove = std::get<2>(item);
			State parent = std::get<3>(item);

			memoryLock.lock();
			bool foundItem = memory.find(state) != memory.end();
			memoryLock.unlock();

			if (foundItem) continue;

			MemoryItem memoryItem = std::make_tuple(move, optimalMove, parent);

			memoryLock.lock();
			memory.insert({ state, memoryItem });
			memoryLock.unlock();

			std::vector<Permutation> permutations = getPermutations(state);
			for (Permutation perm : permutations) {
				permutationChecker(perm, move, state);
			}
		}

		//std::cout << "exiting" << std::endl;
	}


public:
	LightsOutGrid(int gridSize, bool isModded, int threadAllocation) {

		// If it's not between 2 and 5, throw an error
		if (gridSize < 2 || gridSize > 5) throw std::out_of_range("Grid size must be: '>= 2 && <= 5' !");

		// Not all modded vs unmodded combos work, so we fix em up if that's the case
		LightsOutGrid::isModded = isModded; // Assume it's a 3x3 to start with.
		if (gridSize == 4) LightsOutGrid::isModded = true;
		if (gridSize == 2 || gridSize == 5) LightsOutGrid::isModded = false;

		LightsOutGrid::gridSize = gridSize;
		LightsOutGrid::threadAllocation = threadAllocation;
		LightsOutGrid::totalTiles = gridSize * gridSize;
		LightsOutGrid::maxStates = std::pow(2, LightsOutGrid::totalTiles);


		LightsOutGrid::stoppingPoint = 140; // Assume it's a 3x3 to start with.
		if (gridSize == 2) LightsOutGrid::stoppingPoint = 6;
		if (gridSize == 4) LightsOutGrid::stoppingPoint = 16456;
		if (gridSize == 5) LightsOutGrid::stoppingPoint = 2099264;

		State startPoint = maxStates - 1;
		theQueue.emplace(std::make_tuple(startPoint, 0, -1, 0)); // Push the starting state onto the queue
	}


	/* Gets the length of one side of the grid */
	int getGridSize() {
		return LightsOutGrid::gridSize;
	}


	/* Returns true if the game is a modded varient, false otherwise */
	bool getIsModded() {
		return LightsOutGrid::isModded;
	}


	/* Gets the amount of threads that are allowed to be used to brute force the solution */
	int getThreadAllocation() {
		return LightsOutGrid::threadAllocation;
	}


	bool hasSolutions() {
		return LightsOutGrid::solutionsFound;
	}


	void bruteForceSolutions() {
		if (LightsOutGrid::threadEnabled && !LightsOutGrid::solutionsFound) return;

		for (int i = 0; i < LightsOutGrid::threadAllocation; i++) {
			threadList.push_back(std::thread(&LightsOutGrid::processQueue, this));
		}

		LightsOutGrid::threadEnabled = true;
	}


	void waitForThreads() {
		if (!LightsOutGrid::threadEnabled) return;
		for (int i = 0; i < LightsOutGrid::threadAllocation; i++) {
			threadList[i].join();
		}

		threadEnabled = false;
		LightsOutGrid::solutionsFound = true;
	}


	bool getSolution(State& state) {
		Rotations rotations = getRotations(state);
		int rotationFactor = getRotationFactor(state, rotations);

		int correctRotation[4] = { 0, 3, 2, 1 };

		std::cout << std::endl;
		std::cout << state << std::endl;
		std::cout << "Rotation Factor: '" << std::to_string(rotationFactor) << "'" << std::endl;
		std::cout << "Fixed Rotation: '" << std::to_string(correctRotation[rotationFactor]) << "'" << std::endl;
		std::cout << std::endl;

		if (rotationFactor == -1) {
			std::cout << "No solution exists!" << std::endl;
			return false;
		}

		MemoryItem item = LightsOutGrid::memory.at(rotations[rotationFactor]);
		std::cout << "Solution Found! " << "Moves Till Solved: " << std::get<0>(item) << std::endl;

		std::vector<int> moves;

		OptimalMove bestMove = std::get<1>(item);
		State parent = std::get<2>(item);

		while (true) {
			moves.push_back(bestMove);

			MemoryItem newItem = LightsOutGrid::memory.at(parent);
			OptimalMove newMove = std::get<1>(newItem);
			State newParent = std::get<2>(newItem);

			if (newMove == -1) break;

			bestMove = newMove;
			parent = newParent;
		}


		LookUpMatrix rotationMatrix = createRotationMatrix(correctRotation[rotationFactor]);
		std::vector<int> lookupVector = flattenMatrix(rotationMatrix);

		std::vector<int> rotatedNumbers;

		for (int i = 0; i < moves.size(); i++) {

			int optimalMove = moves[i];
			int rotatedMove = lookupVector[optimalMove];

			rotatedNumbers.push_back(rotatedMove + 1);

			std::string moveNumber = "\nMove " + std::to_string(i + 1) + ":\n";

			std::cout << moveNumber << makeDiagram(rotatedMove) << std::endl;
		}

		std::cout << "\nOr in number format: ";

		for (int i = 0; i < rotatedNumbers.size(); i++) {
			std::cout << rotatedNumbers[i] << " ";
		}

		std::cout << "\n" << std::endl;


		return true;
	}
};