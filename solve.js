"use strict";
import chalk from "chalk";


/*

Web Version

Brute force 'Solver'
1x1, 2x2, 3x3 and 4x4 are all possible to find a solution for, no matter what the initial state is "quickly".

HOWEVER:
I cannot recommend trying to solve a 5x5 or higher board.
Technically, there are 33.5million possible states for a 5x5 board
Even if you take into account certain states are just rotations of others (which this does), it doesn't help much.
A 5x5 board would still only be brought down to around 2.1million states.

However, one thing that I can say with reasonable confidence, is that because I'm using a breadth first search
when a solution does get found, it will be a solution with the least amount of moves possible.

*/



// Board Dimensions
const boardSize = 4;
const modded = true;
const rotationEnabled = true;


// Initial setup
const maxMemory = Math.pow(2, Math.pow(boardSize, 2));
const totalTiles = boardSize * boardSize;
const solvedState = "T".repeat(totalTiles);

const memory = new Map();
const queue = new Map();

const nextInQueue = () => {
	const next = queue.entries().next().value;
	queue.delete(next[0]);

	return next[1];
};



// Functions
/** Rotates a 2D matrix 90 degrees clockwise
 * @remarks Code transpiled to JS from a comment on stackoverflow - https://stackoverflow.com/a/35438327 */
function rotate(matrix) {
	const size = matrix.length;
	const layerCount = Math.floor(size / 2);
	for(let layer = 0; layer < layerCount; layer++){
		const first = layer;
		const last = size - first - 1;
		for(let element = first; element < last; element++){
			const offset = element - first;
			const top = matrix[first][element];
			const rightSide = matrix[element][last];
			const bottom = matrix[last][last - offset];
			const leftSide = matrix[last - offset][first];
			matrix[first][element] = leftSide;
			matrix[element][last] = top;
			matrix[last][last - offset] = rightSide;
			matrix[last - offset][first] = bottom;
		}
	}
}


/** Produces a list of all possible rotations of a given state */
function getRotations(_matrix) {
	const matrix = _matrix;
	const rotations = [];
	const base = shrinkState(matrix);
	rotations.push(base);
	for(let i = 0; i < 3; i++){
		rotate(matrix);
		rotations.push(shrinkState(matrix));
	}
	return rotations;
}


/** Expands a state into a 2D array of booleans */
function expandState(_state) {
	const state = _state.split("");
	const expandedState = [];

	for(let i = 0; i < boardSize; i++){
		const expandedRow = [];

		for(let j = 0; j < boardSize; j++){
			const index = i * boardSize + j;
			const bool = state[index] === "T";
			expandedRow.push(bool);
		}

		expandedState.push(expandedRow);
	}
	return expandedState;
}


/** Shrinks an expanded state back down to a string */
function shrinkState(_expandedState) {
	const expandedState = _expandedState;
	const state = [];
	for(const row of expandedState){
		for(const bool of row){
			state.push(bool ? "T" : "F");
		}
	}
	return state.join("");
}


/** Toggles the tile and appropriate adjacent tiles in a state */
function toggleTiles(state, row, col) {
	const expanded = expandState(state);
	expanded[row][col] = !expanded[row][col];
	const top = row - 1;
	const bottom = row + 1;
	const left = col - 1;
	const right = col + 1;

	if(top >= 0) expanded[top][col] = !expanded[top][col];
	if(bottom < boardSize) expanded[bottom][col] = !expanded[bottom][col];
	if(left >= 0) expanded[row][left] = !expanded[row][left];
	if(right < boardSize) expanded[row][right] = !expanded[row][right];

	if(modded){
		const topRight = top >= 0 && right < boardSize;
		const topLeft = top >= 0 && left >= 0;
		const bottomRight = bottom < boardSize && right < boardSize;
		const bottomLeft = bottom < boardSize && left >= 0;

		if(topRight) expanded[top][right] = !expanded[top][right];
		if(topLeft) expanded[top][left] = !expanded[top][left];
		if(bottomRight) expanded[bottom][right] = !expanded[bottom][right];
		if(bottomLeft) expanded[bottom][left] = !expanded[bottom][left];
	}

	return shrinkState(expanded);
}


/** Produces a single permutation of a given state */
function permutate(state, tile) {
	const col = tile % boardSize;
	const row = Math.floor(tile / boardSize);
	return toggleTiles(state, row, col);
}


/** Lists all the possible permutations of a state */
function getPermutations(_state) {
	const state = _state;
	const permutations = [];
	for(let i = 0; i < totalTiles; i++){
		const permutation = permutate(state, i);
		permutations.push({ permutation, tile: i });
	}
	return permutations;
}


function doQueue(_state = solvedState) {

	queue.set(solvedState, { state: solvedState, parent: null, moves: 0, optimal: null });

	if(rotationEnabled){
		while(queue.size > 0){
			const { state, moves, parent, optimal } = nextInQueue();

			if(memory.has(state)) continue; // This shouldn't happen, but just in case
			memory.set(state, { movesToSolve: moves, optimalMove: optimal, parentState: parent });

			// console.log(`${state} | States: ${memory.size}, Queue: ${queue.size}`);

			const permutations = getPermutations(state);
			for(const perm of permutations){

				const { permutation, tile } = perm;
				if(memory.has(permutation) || queue.has(permutation)) continue;


				const rotations = getRotations(expandState(permutation));

				let rotatedPerm = false;
				for(const rotation of rotations){
					if(memory.has(rotation) || queue.has(rotation)){
						rotatedPerm = true;
						break;
					}
				}

				if(rotatedPerm) continue;



				queue.set(permutation, { state: permutation, parent: state, optimal: tile, moves: moves + 1 });
			}
		}
	} else {
		while(queue.size > 0){
			const { state, moves, parent, optimal } = nextInQueue();

			if(memory.has(state)) continue; // This shouldn't happen, but just in case
			memory.set(state, { movesToSolve: moves, optimalMove: optimal, parentState: parent });

			const permutations = getPermutations(state);


			for(const perm of permutations){
				const { permutation, tile } = perm;
				if(memory.has(permutation) || queue.has(permutation)) continue;

				queue.set(permutation, { state: permutation, parent: state, optimal: tile, moves: moves + 1 });
			}
		}
	}

	console.log("Done!");
}

console.log("Version 1.5");
console.log("\nFinding solution...");

console.time("Took");
doQueue();
console.timeEnd("Took");
console.log(`\n\nMemory Size: ${chalk.yellow(memory.size)} | Max Memory: ${chalk.magenta(maxMemory)}`);
const rotationsAreEnabled = (rotationEnabled) ? chalk.green("Enabled") : chalk.red("Disabled");
const isModdedGame = (modded) ? chalk.green("True") : chalk.red("False");

console.log(`Rotation: ${rotationsAreEnabled} | Modded: ${isModdedGame} | Board Size: ${chalk.blue(boardSize)}`);


/*
Version 1.5 Times

3x3 Normal:		10ms (No Rotation)
3x3 Normal:		9ms (With Rotation)

3x3 Modded:		10ms (No Rotation)
3x3 Modded:		8ms (With Rotation)



4x4 Normal:		N/A (Many combinations are impossible to solve)
4x4 Normal: 	N/A (Many combinations are impossible to solve)

4x4 Modded:		1340ms (No Rotation)
4x4 Modded:		490ms (With Rotation)



5x5 Normal:		I refuse to test (No Rotation)
5x5 Normal:		8m, 11s, 391ms (With Rotation)

5x5 Modded:		N/A (Many combinations are impossible to solve)
5x5 Modded:		N/A (Many combinations are impossible to solve)

*/