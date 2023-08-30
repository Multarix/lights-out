#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <vector>
#include <tuple>
#include <math.h>
#include <queue>
#include <time.h>
#include <algorithm>
#include <mutex>
#include <chrono>
#include <thread>
#include <windows.h>
#include <bitset>

using namespace std;


typedef int MovesTillSolved;
typedef int OptimalMove;
typedef unsigned int State;

typedef tuple<State, OptimalMove> Permutation;
typedef vector<vector<bool>> Matrix;
typedef vector<vector<int>> LookUpMatrix;
typedef vector<State> Rotations;

typedef tuple<State, MovesTillSolved, OptimalMove, State> QueueItem; // state, movesTillSolved, optimalMove, parent
typedef tuple<MovesTillSolved, OptimalMove, State> MemoryItem; // movesTillSolved, optimalMove, parent


// Config settings


const int boardSize = 5;
const bool rotationEnabled = true;
bool isModded = false;

const bool threadingEnabled = false;
const int maxThreads = 5;


//const State stateToSolveFrom2 = 16;
//const State stateToSolveFrom3 = 461;
//const State stateToSolveFrom4 = 25397;
//const State stateToSolveFrom5 = 20051570;


// End Config


// Global Variables
const int totalTiles = boardSize * boardSize;
double maxStates = pow(2, totalTiles);

// Mutex
mutex queueLock;
mutex memoryLock;
mutex outputLock;

State memSize = 0;


unordered_set<State> inQueue;
queue<QueueItem> theQueue; // state, move, optimalMove, parent
unordered_map<State, MemoryItem> memory; // state | move, optimalMove, parent




/* Reverses the binary of a number */
State reverseBinary(State& state) {
	auto bits = std::bitset<totalTiles>(state);

	string debugString = "";

	State reversed = 0;
	int j = 0;
	for (int i = totalTiles - 1; i >= 0; i--) {
		reversed += (bits.test(i)) ? pow(2, j) : 0;

		debugString += (bits.test(i)) ? "1" : "0";
		j++;
	}

	//std::cout << "Original: " << state << " Reversed: " << reversed << endl;

	return reversed;
}


/* Shrinks a 2D Boolean Vector into number */
State shrinkToBinary(Matrix& matrix) {
	State state = 0;

	int index = 0;
	for (vector<bool> row : matrix) {
		for (bool bit : row) {
			state += (bit) ? pow(2, index) : 0;
			index++;
		}
	}

	return state;
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

	return shrinkToBinary(flipMatrix);
}

/* Because reinventing the wheel is stupid, the code below is taken from: https://www.prodevelopertutorial.com/given-an-n-x-n-2d-matrix-rotate-it-by-90-degrees-clockwise-in-c-in-place/ */
void rotateLookup(LookUpMatrix& flipMatrix) {
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
}


Matrix expandToMatrix(State& state) {
	auto bits = std::bitset<totalTiles>(state);
	Matrix matrix;

	for (int i = 0; i < boardSize; i++) {
		vector<bool> row;

		for (int j = 0; j < boardSize; j++) {
			row.push_back(bits.test(i * boardSize + j));
		};

		matrix.push_back(row);
	}

	return matrix;
}


LookUpMatrix createRotationMatrix(int& numberOfRotations) {
	LookUpMatrix rotationMatrix;

	for (int i = 0; i < boardSize; i++) {
		vector<int> row;

		for (int j = 0; j < boardSize; j++) row.push_back(i * boardSize + j);

		rotationMatrix.push_back(row);
	}


	for (int i = 0; i < numberOfRotations; i++) rotateLookup(rotationMatrix);

	return rotationMatrix;
}



Rotations getRotations(State& state) {
	Rotations rotations;
	rotations.push_back(state);

	Matrix matrix = expandToMatrix(state);
	State rotatedState = rotateState(matrix);

	if (rotatedState == state) return rotations;
	rotations.push_back(rotatedState);

	State reverseState = reverseBinary(state);
	if (reverseState == state) return rotations;

	rotations.push_back(reverseState);
	rotations.push_back(reverseBinary(rotatedState));

	return rotations;
}


State getModdedMask(int& tile) {
	string singleOne = "1";
	string doubleOne = singleOne + singleOne;
	string tripleOne = doubleOne + singleOne;

	int topLeft = 0;
	int topRight = boardSize - 1;
	int bottomLeft = totalTiles - (boardSize);
	int bottomRight = totalTiles - 1;

	string boardSizeMinus2Zeros = std::string(boardSize - 2, '0');
	string boardSizeMinus3Zeros = std::string(boardSize - 3, '0');


	// Handle Corners
	if (tile == topLeft || tile == topRight || tile == bottomLeft || tile == bottomRight) {
		State corner = stoi(doubleOne + boardSizeMinus2Zeros + doubleOne, 0, 2);

		if (tile == topLeft) return (corner << ((boardSize - 2) + ((boardSize - 2) * boardSize)));	// Top left
		if (tile == topRight) return (corner << (totalTiles - (boardSize * 2)));					// Top right
		if (tile == bottomLeft) return (corner << (boardSize - 2));									// Bottom Left
		if (tile == bottomRight) return corner;													// Bottom Right
	}

	State topBottom = stoi(tripleOne + boardSizeMinus3Zeros + tripleOne, 0, 2);
	State leftRight = stoi(doubleOne + boardSizeMinus2Zeros + doubleOne + boardSizeMinus2Zeros + doubleOne, 0, 2);

	// Handle top side clicks
	if (topLeft < tile && topRight > tile) return (topBottom << ((totalTiles - 1) - (tile + boardSize + 1)));

	// Handle left side clicks
	if (tile % boardSize == 0) return (leftRight << ((totalTiles - 1) - (tile + boardSize + 1)));

	// Handle right side clicks
	if (tile % boardSize == boardSize - 1) return (leftRight << ((totalTiles - 1) - (tile + boardSize)));

	// Handle bottom side clicks
	if (bottomLeft < tile && bottomRight > tile) return (topBottom << ((totalTiles - 1) - (tile + 1)));

	// Everything else
	return (stoi(tripleOne + boardSizeMinus3Zeros + tripleOne + boardSizeMinus3Zeros + tripleOne, 0, 2) << ((totalTiles - 1) - (tile + boardSize + 1)));
}


State getNormalMask(int& tile) {
	string singleOne = "1";
	string doubleOne = singleOne + singleOne;
	string tripleOne = doubleOne + singleOne;

	int topLeft = 0;
	int topRight = boardSize - 1;
	int bottomLeft = totalTiles - (boardSize);
	int bottomRight = totalTiles - 1;

	string boardSizeMinus1Zeros = std::string(boardSize - 1, '0');
	string boardSizeMinus2Zeros = std::string(boardSize - 2, '0');


	// Handle Corners
	if (tile == topLeft || tile == topRight || tile == bottomLeft || tile == bottomRight) {
		if (tile == topLeft) return (stoi(doubleOne + boardSizeMinus2Zeros + singleOne, 0, 2) << ((totalTiles - (boardSize + 1))));						// Top left
		if (tile == topRight) return (stoi(doubleOne + boardSizeMinus1Zeros + singleOne, 0, 2) << (totalTiles - (boardSize * 2)));						// Top right
		if (tile == bottomLeft) return stoi(singleOne + boardSizeMinus1Zeros + doubleOne, 0, 2) << (boardSize - 2);										// Bottom Left
		if (tile == bottomRight) return stoi(singleOne + boardSizeMinus2Zeros + doubleOne, 0, 2);														// Bottom Right
	}

	// Handle top side clicks
	if (topLeft < tile && topRight > tile) return (stoi(tripleOne + boardSizeMinus2Zeros + singleOne, 0, 2) << (((boardSize - tile) - 1) + ((boardSize - 2) * boardSize)));

	// Handle left side clicks
	if (tile % boardSize == 0) return (stoi(singleOne + boardSizeMinus1Zeros + doubleOne + boardSizeMinus2Zeros + singleOne, 0, 2) << ((boardSize - 1) + (((boardSize - (tile / boardSize)) - 2) * boardSize)));

	// Handle right side clicks
	if (tile % boardSize == boardSize - 1) return (stoi(singleOne + boardSizeMinus2Zeros + doubleOne + boardSizeMinus1Zeros + singleOne, 0, 2) << (((boardSize - ((tile + 1) / boardSize)) - 1) * boardSize));

	// Handle bottom side clicks
	if (bottomLeft < tile && bottomRight > tile) return (stoi(singleOne + boardSizeMinus2Zeros + tripleOne, 0, 2) << ((totalTiles - 2) - tile));

	// Everything else
	return (stoi(singleOne + boardSizeMinus2Zeros + tripleOne + boardSizeMinus2Zeros + singleOne, 0, 2) << ((totalTiles - 1) - (tile + boardSize)));
}


vector<Permutation> getPermutations(State& state) {
	vector<Permutation> permutations;

	for (int i = 0; i < totalTiles; i++) {
		State mask = (isModded) ? getModdedMask(i) : getNormalMask(i);
		permutations.push_back(make_tuple(state ^ mask, i));
	}

	return permutations;
}


bool checkRotations(State& state) {
	bool rotated = false;
	Rotations rotations = getRotations(state);

	for (State rotation : rotations) { // We can skip the first rotation because it's guarenteed to be the same as the original state

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
	State newState = get<0>(perm);
	OptimalMove optimalMove = get<1>(perm);

	queueLock.lock();
	bool queueFound = inQueue.find(newState) != inQueue.end();
	queueLock.unlock();

	if (queueFound) return;
	if (rotationEnabled) {
		if (checkRotations(newState)) return;
	}

	QueueItem queueItem = make_tuple(newState, move + 1, optimalMove, state);
	queueLock.lock();
	theQueue.emplace(queueItem);
	inQueue.insert(newState);
	queueLock.unlock();

	return;
}




bool keepGoing(double& memoryMax) {

	//queueLock.lock();
	//bool queueEmpty = (theQueue.size() == 0);
	//queueLock.unlock();

	//if (queueEmpty) return false;

	memoryLock.lock();
	bool answer = (memory.size() < memoryMax);
	memoryLock.unlock();

	return answer;
}


void processQueue(double maxMemorySize, int threadID) {
	int processed = 0;

	while (keepGoing(maxMemorySize)) {
		queueLock.lock();

		//cout << theQueue.size() << endl;
		//cout << memory.size() << endl;

		if (theQueue.size() == 0) {
			queueLock.unlock();
			continue;
		}

		QueueItem item = theQueue.front();
		theQueue.pop();

		//inQueue.erase(get<0>(item));
		queueLock.unlock();

		State state = get<0>(item);
		MovesTillSolved move = get<1>(item);
		OptimalMove optimalMove = get<2>(item);
		State parent = get<3>(item);

		memoryLock.lock();
		bool foundItem = memory.find(state) != memory.end();
		memoryLock.unlock();

		if (foundItem) continue;

		MemoryItem memoryItem = make_tuple(move, optimalMove, parent);

		memoryLock.lock();
		memory.insert({ state, memoryItem });
		memoryLock.unlock();

		vector<Permutation> permutations = getPermutations(state);
		for (Permutation perm : permutations) {
			permutationChecker(perm, move, state);
		}

		processed++;
	}

	outputLock.lock();
	std::cout << threadID << " | Thread Finished. Processed: " << processed << "\n";
	outputLock.unlock();
}





int main() {

	//State solveState = setup();
	State startPoint = maxStates - 1;

	string boardSizeString = "Board Size: " + to_string(boardSize);
	boardSizeString += to_string(boardSize);

	string rotationString = " | Rotation: ";
	rotationString += (rotationEnabled) ? "Enabled" : "Disabled";

	string moddedString = " | Modded: ";
	moddedString += (isModded) ? "True" : "False";

	string threadString = "\nThreading: ";
	threadString += (threadingEnabled) ? "Enabled" : "Disabled";

	string maxThreadsString = " | Max Threads: ";
	maxThreadsString += to_string(maxThreads);


	std::cout << "Finding Solution...\nSettings:\n" << boardSizeString << rotationString << moddedString << threadString << maxThreadsString << endl;

	clock_t start = clock();
	theQueue.emplace(make_tuple(startPoint, 0, -1, 0));


	double stoppingPoint = maxStates;

	if (rotationEnabled) {
		if (boardSize == 2) stoppingPoint = 6;
		if (boardSize == 3) stoppingPoint = 140;
		if (boardSize == 4) stoppingPoint = 16456;
		if (boardSize == 5) stoppingPoint = 2099264;
	}

	//cout << maxStates << endl;

	if (threadingEnabled) {
		thread threadList[maxThreads];

		for (int i = 0; i < maxThreads; i++) {
			threadList[i] = thread(processQueue, stoppingPoint, i);
		}

		for (int i = 0; i < maxThreads; i++) {
			threadList[i].join();
		}
	}
	else {
		processQueue(stoppingPoint, 0);
	}


	clock_t end = clock();
	double elapsed = double(end - start);
	double inSeconds = elapsed / 1000;

	std::cout << "Took: " << elapsed << "ms\n";
	std::cout << "In seconds: " << inSeconds << "s\n\n";

	system("pause");

	return 0;
}