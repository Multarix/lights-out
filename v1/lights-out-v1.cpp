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


using namespace std;

typedef int MovesTillSolved;
typedef int OptimalMove;
typedef string State;

typedef tuple<State, OptimalMove> Permutation;
typedef vector<vector<bool>> Matrix;
typedef vector<vector<int>> LookUpMatrix;
typedef vector<State> Rotations;

typedef tuple<State, MovesTillSolved, OptimalMove, State> QueueItem;
typedef tuple<MovesTillSolved, OptimalMove, State> MemoryItem;



// Config settings


const int boardSize = 5;
const int maxThreads = 5;
const bool rotationEnabled = true;
bool modified = true;


const State stateToSolveFrom2 = "TT TF";
const State stateToSolveFrom3 = "TTT FFT TFT";
const State stateToSolveFrom4 = "TFTF FFTT TTTF FFFT";
const State stateToSolveFrom5 = "TFFTT FFFTT TTTFT TFFTT TFFTF";


// End Config



// Global Variables
int memSize = 0;
const int totalTiles = boardSize * boardSize;

unordered_map<int, tuple<double, double>> expectedStateSize;

State solution = "";

mutex queueLock;
queue<QueueItem> memQueue; // state, move, optimalMove, parent
unordered_set<State> beenQueued; // State, true

mutex memoryLock;
unordered_map<State, MemoryItem> memory; // state | move, optimalMove, parent

mutex outputLock;


State shrinkState(Matrix& matrix) {
	State state = "";

	for (int i = 0; i < boardSize; i++) {
		for (int j = 0; j < boardSize; j++) state = state + (matrix[i][j] ? "T" : "F");
	}

	return state;
}


Matrix expandState(State& state) {
	Matrix matrix;

	for (int i = 0; i < boardSize; i++) {
		vector<bool> row;

		for (int j = 0; j < boardSize; j++) row.push_back(state[i * boardSize + j] == 'T');

		matrix.push_back(row);
	}

	return matrix;
}



/**
 * Because reinventing the wheel is stupid, the code below is taken from:
 * https://www.prodevelopertutorial.com/given-an-n-x-n-2d-matrix-rotate-it-by-90-degrees-clockwise-in-c-in-place/
*/
void flip(Matrix& flipMatrix) {
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


void matrixflip(LookUpMatrix& flipMatrix) {
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


LookUpMatrix createRotationMatrix(int& numberOfRotations) {
	LookUpMatrix rotationMatrix;

	for (int i = 0; i < boardSize; i++) {
		vector<int> row;

		for (int j = 0; j < boardSize; j++) row.push_back(i * boardSize + j);

		rotationMatrix.push_back(row);
	}


	for (int i = 0; i < numberOfRotations; i++) matrixflip(rotationMatrix);

	return rotationMatrix;
}


State rotateMatrix(State& state) {
	Matrix matrix = expandState(state);

	flip(matrix);

	State rotatedState = "";
	rotatedState = shrinkState(matrix);

	return rotatedState;
}


Rotations getRotations(State& state) {
	Rotations rotations;

	State rotatedState = rotateMatrix(state);;

	while (rotatedState != state) {
		rotations.push_back(rotatedState);
		rotatedState = rotateMatrix(rotatedState);

		if (rotations.size() == 3) break;
	}

	return rotations;
}


State toggleTiles(State& state, int& row, int& col) {
	Matrix expanded = expandState(state);

	expanded[row][col] = !expanded[row][col];

	int top = row - 1;
	int bottom = row + 1;
	int left = col - 1;
	int right = col + 1;

	if (top >= 0) expanded[top][col] = !expanded[top][col];
	if (bottom < boardSize) expanded[bottom][col] = !expanded[bottom][col];
	if (left >= 0) expanded[row][left] = !expanded[row][left];
	if (right < boardSize) expanded[row][right] = !expanded[row][right];


	if (modified) {
		bool topRight = top >= 0 && right < boardSize;
		bool topLeft = top >= 0 && left >= 0;
		bool bottomRight = bottom < boardSize && right < boardSize;
		bool bottomLeft = bottom < boardSize && left >= 0;

		if (topRight) expanded[top][right] = !expanded[top][right];
		if (topLeft) expanded[top][left] = !expanded[top][left];
		if (bottomRight) expanded[bottom][right] = !expanded[bottom][right];
		if (bottomLeft) expanded[bottom][left] = !expanded[bottom][left];
	}

	return shrinkState(expanded);
}


State permutate(State& state, int tile) {
	int row = floor(tile) / boardSize;
	int col = tile % boardSize;

	return toggleTiles(state, row, col);
}


vector<Permutation> getPermutations(State& state, int& optimalMove) {
	vector<Permutation> permutations;

	for (int i = 0; i < totalTiles; i++) {
		if (i == optimalMove) continue; // We know that this has already been checked
		State permutation = permutate(state, i);

		tuple<State, OptimalMove> perm = make_tuple(permutation, i);
		permutations.push_back(perm);
	}

	return permutations;
}

// string makeDiagram(int& tile) {
// 	string rowDelimiter = string((boardSize * 4) + 1, '-');
// 	string eTile = "|   ";
// 	string fTile = "| x ";

// 	string diagram = "";
// 	for (int i = 0; i < boardSize; i++) {
// 		diagram += rowDelimiter + "\n";

// 		for (int j = 0; j < boardSize; j++) diagram += (i * boardSize + j == tile) ? fTile : eTile;

// 		diagram += "|\n";
// 	}

// 	diagram += rowDelimiter;

// 	/* Example:
// 	"-------------"
// 	"|   | x |   |"
// 	"-------------"
// 	"|   |   |   |"
// 	"-------------"
// 	"|   |   |   |"
// 	"-------------"
// 	*/

// 	return diagram;
// }


//int getRotationFactor(State& state) {
//	int rotationFactor = -1;
//	Rotations rotations = getRotations(state);
//
//	for (int i = 0; i < rotations.size(); i++) {
//		State rotation = rotations[i];
//
//		if (memory.find(rotation) != memory.end()) {
//			rotationFactor = i;
//			break;
//		}
//	}
//
//	return rotationFactor;
//}

//bool findSolution(State& state) {
//	Rotations rotations = getRotations(state);
//
//	int rotationFactor = getRotationFactor(state);
//	if (rotationFactor == -1) {
//		std::cout << "No solution exists!" << endl;
//		return false;
//	}
//	std::cout << "Rotation Factor: " << rotationFactor << endl;
//
//	MemoryItem item = memory.at(rotations[rotationFactor]);
//	std::cout << "Moves Till Solved: " << get<0>(item) << endl;
//
//
//	LookUpMatrix rotationMatrix = createRotationMatrix(rotationFactor);
//
//
//	return true;
//}

bool checkRotations(State& state) {
	bool rotated = false;
	Rotations rotations = getRotations(state);
	for (int i = 0; i < rotations.size(); i++) {
		State rotation = rotations[i];

		queueLock.lock();
		bool foundItem = beenQueued.find(rotation) != beenQueued.end();
		queueLock.unlock();

		if (foundItem) return true;
	}

	return false;
}


void permutationChecker(Permutation& perm, int& move, State& state) {
	State newState = get<0>(perm);
	OptimalMove optimalMove = get<1>(perm);

	queueLock.lock();
	bool queueFound = beenQueued.find(newState) != beenQueued.end();
	queueLock.unlock();

	if (queueFound) return;
	if (rotationEnabled) {
		if (checkRotations(newState)) return;
	}

	QueueItem queueItem = make_tuple(newState, move + 1, optimalMove, state);
	queueLock.lock();
		memQueue.emplace(queueItem);
		beenQueued.insert({ newState, true });
	queueLock.unlock();

	return;
}


void processQueue(int maxMemorySize, int id) {
	int processed = 0;


	while (memory.size() < maxMemorySize) {

		queueLock.lock();
		bool sizeIsZero = memQueue.size() == 0;
		queueLock.unlock();

		if (sizeIsZero) continue;

		queueLock.lock();
		QueueItem item = memQueue.front();
		memQueue.pop();
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
		// cout << state << " | " << "States: " << memory.size() << ", Queue: " << memQueue.size() << "\n";

		vector<Permutation> permutations = getPermutations(state, optimalMove);
		for (int i = 0; i < permutations.size(); i++) {
			permutationChecker(permutations[i], move, state);
		}

		processed++;
	}
	outputLock.lock();
	std::cout << id << " | Thread Finished. Processed: " << processed << "\n";
	outputLock.unlock();
}



void setup() {
	expectedStateSize.insert({ 0, make_tuple(0, 0) });
	expectedStateSize.insert({ 1, make_tuple(1, 1) });
	expectedStateSize.insert({ 2, make_tuple(16, 6) });
	expectedStateSize.insert({ 3, make_tuple(512, 140) });
	expectedStateSize.insert({ 4, make_tuple(65536, 16456) });
	expectedStateSize.insert({ 5, make_tuple(33554431, 2099264) });

	for (int i = 0; i < totalTiles; i++) solution = solution + "T";

	double maxStates = pow(2, pow(boardSize, 2));

	State stateToSolveFrom = "";
	if (boardSize == 2) {
		modified = false;
		stateToSolveFrom = stateToSolveFrom2;
	}

	if (boardSize == 3) stateToSolveFrom = stateToSolveFrom3;

	if (boardSize == 4) {
		modified = true;
		stateToSolveFrom = stateToSolveFrom4;
	}

	if (boardSize == 5) {
		modified = false;
		stateToSolveFrom = stateToSolveFrom5;
	}

	if (stateToSolveFrom == "") {
		std::cout << "No state to solve from! Exiting..." << endl;
		exit(1);
	}


	stateToSolveFrom.erase(remove_if(stateToSolveFrom.begin(), stateToSolveFrom.end(), [](char c) { return isspace(c); }), stateToSolveFrom.end());
	if (stateToSolveFrom.size() != totalTiles) {
		std::cout << "State is incorrect size! Exiting..." << endl;
		exit(1);
	}

	memSize = (rotationEnabled) ? get<1>(expectedStateSize[boardSize]) : get<0>(expectedStateSize[boardSize]);
}


int main() {

	setup();

	std::cout << "Finding Solution...\n";

	clock_t start = clock();
	memQueue.emplace(make_tuple(solution, 0, -1, ""));


	thread threadList[maxThreads];

	for (int i = 0; i < maxThreads; i++) {
		threadList[i] = thread(processQueue, memSize, i);
	}


	for (int i = 0; i < maxThreads; i++) {
		threadList[i].join();
	}


	clock_t end = clock();
	double elapsed = double(end - start);
	double inSeconds = elapsed / 1000;

	std::cout << "Took: " << elapsed << "ms\n";
	std::cout << "In seconds: " << inSeconds << "s\n\n";


	std::cout << "Memory Size: " << memory.size() << "\n";
	std::cout << "Expected Size: " << memSize << "\n";
	// findSolution(stateToSolveFrom);


	return 0;
}