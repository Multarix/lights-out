"use strict";


/*
Brute force 'Solver'
1x1, 2x2, 3x3 and 4x4 are all possible to find a solution for, no matter what the initial state is "quickly".

HOWEVER:
I cannot recommend trying to solve a 5x5 or higher board.
Technically, there are 33.5million possible states for a 5x5 board
Even if you take into account certain states are just rotations of others (which this does), it doesn't help much.

However, one thing that I can say with reasonable confidence, is that because I'm using a breadth first search
when a solution does get found, it will be a solution with the least amount of moves possible.
*/



// Board Dimensions
const width = 3;
const height = 3;



// Initial setup
const totalTiles = width * height;
const solvedState = "T".repeat(totalTiles);
const splitRegex = new RegExp(`.{${width},${width}}`, "g");

const memory = new Map();
const queue = [];
queue.push({ state: solvedState, parent: null, moves: 0, optimal: null });
const nextInQueue = () => queue.shift();



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
	const state = _state;
	const rows = state.match(splitRegex) ?? [];
	// console.log(rows)
	const expandedState = [];
	for(const row of rows){
		const expandedRow = [];
		for(const char of row){
			const bool = (char === "T");
			expandedRow.push(!!bool);
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
	if(top >= 0){
		expanded[top][col] = !expanded[top][col];
	}
	if(bottom < height){
		expanded[bottom][col] = !expanded[bottom][col];
	}
	if(left >= 0){
		expanded[row][left] = !expanded[row][left];
	}
	if(right < width){
		expanded[row][right] = !expanded[row][right];
	}
	return shrinkState(expanded);
}


/** Produces a single permutation of a given state */
function permutate(state, tile) {
	const col = tile % width;
	const row = Math.floor(tile / width);
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
	while(queue.length > 0){
		const item = nextInQueue();
		if(!item){
			break;
		}
		const { state, moves, parent, optimal } = item;
		const rotations = getRotations(expandState(state));
		let isRotated = false;
		for(const rotation of rotations){
			if(memory.has(rotation)){
				isRotated = true;
				break;
			}
		}
		if(isRotated){
			continue;
		}
		memory.set(state, { movesToSolve: moves, optimalMove: optimal, parentState: parent });
		const permutations = getPermutations(state);
		for(const perm of permutations){
			const { permutation, tile } = perm;
			queue.push({ state: permutation, parent: state, optimal: tile, moves: moves + 1 });
		}
	}
}


// doQueue();