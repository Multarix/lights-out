"use strict";
import chalk from "chalk";


/*
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



// Config
const boardSize = 2;
const modded = false;
const rotationEnabled = true;



// Initial setup
const totalTiles = boardSize * boardSize;
const maxMemory = Math.pow(2, Math.pow(boardSize, 2));
const solvedState = maxMemory - 1 >>> 0;

/** @type {number[]} */
const memory = new Array();

const queue = new Map();
// Functions

const nextInQueue = () => { // Use in CPP
	const next = queue.entries().next().value;
	queue.delete(next[0]);

	return next[1];
};


function makeBinaryArray(state) {
	const bits = [];

	let i = state;
	while(i > 0){
		bits.push(i % 2);
		i = Math.floor(i / 2);
	}

	while(bits.length < totalTiles){ // Ensure that it will be the correct length
		bits.push(0);
	}

	// console.log(bits);
	return bits;
}


function reverseBinary(state) {
	const bits = makeBinaryArray(state);
	bits.reverse();

	let decimal = 0;

	// we can use `i` as the power
	for(let i = 0; i < bits.length; i++){
		decimal += bits[i] * Math.pow(2, i);
	}

	return decimal;
}


function shrinkToBinary(matrix) {
	let binaryString = "";

	for(const row of matrix){
		for(const bit of row){
			binaryString += (bit) ? "1" : "0";
		}
	}

	return parseInt(binaryString, 2);
}


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

	return shrinkToBinary(matrix);
}


/** Expand to a 2D matrix */
function expandToMatrix(state) {
	const binary = state.toString(2).padStart(totalTiles, "0").split("");

	const matrix = [];
	for(let i = 0; i < boardSize; i++){
		const row = [];
		for(let j = 0; j < boardSize; j++){
			const index = i * boardSize + j;
			const bit = (binary[index] === "1") ? true : false;
			row.push(bit);
		}

		matrix.push(row);
	}

	return matrix;
}



// /** Produces a list of all possible rotations of a given state */
function getRotations(state) {

	/** @type {number[]} */
	const rotations = [state];


	const rotatedState = rotate(expandToMatrix(state));
	if(rotatedState === state) return rotations; // This shouldn't happen?
	rotations.push(rotatedState);

	rotations.push(reverseBinary(state), reverseBinary(rotatedState));

	return rotations;
}



function getModdedMask(tile) {
	const singleOne = "1";
	const doubleOne = singleOne + singleOne;
	const tripleOne = doubleOne + singleOne;

	const topLeft = 0;
	const topRight = boardSize - 1;
	const bottomLeft = totalTiles - (boardSize);
	const bottomRight = totalTiles - 1;

	const boardSizeMinus2Zeros = "0".repeat(boardSize - 2);
	const boardSizeMinus3Zeros = "0".repeat(boardSize - 3);


	// Handle Corners
	if(tile === topLeft || tile === topRight || tile === bottomLeft || tile === bottomRight){
		const corner = parseInt(doubleOne + boardSizeMinus2Zeros + doubleOne, 2);

		if(tile === topLeft) return corner << (boardSize - 2) + ((boardSize - 2) * boardSize);	// Top left
		if(tile === topRight) return corner << totalTiles - (boardSize * 2);					// Top right
		if(tile === bottomLeft) return corner << boardSize - 2;									// Bottom Left
		if(tile === bottomRight) return corner;													// Bottom Right
	}

	const topBottom = parseInt(tripleOne + boardSizeMinus3Zeros + tripleOne, 2);
	const leftRight = parseInt(doubleOne + boardSizeMinus2Zeros + doubleOne + boardSizeMinus2Zeros + doubleOne, 2);

	// Handle top side clicks
	if(topLeft < tile && topRight > tile) return topBottom << (totalTiles - 1) - (tile + boardSize + 1);

	// Handle left side clicks
	if(tile % boardSize === 0) return leftRight << (totalTiles - 1) - (tile + boardSize + 1);

	// Handle right side clicks
	if(tile % boardSize === boardSize - 1) return leftRight << (totalTiles - 1) - (tile + boardSize);

	// Handle bottom side clicks
	if(bottomLeft < tile && bottomRight > tile) return topBottom << (totalTiles - 1) - (tile + 1);

	// Everything else
	return parseInt(tripleOne + boardSizeMinus3Zeros + tripleOne + boardSizeMinus3Zeros + tripleOne, 2) << (totalTiles - 1) - (tile + boardSize + 1);
}



function getNormalMask(tile) {
	const singleOne = "1";
	const doubleOne = singleOne + singleOne;
	const tripleOne = doubleOne + singleOne;

	const topLeft = 0;
	const topRight = boardSize - 1;
	const bottomLeft = totalTiles - (boardSize);
	const bottomRight = totalTiles - 1;

	const boardSizeMinus1Zeros = "0".repeat(boardSize - 1);
	const boardSizeMinus2Zeros = "0".repeat(boardSize - 2);


	// Handle Corners
	if(tile === topLeft || tile === topRight || tile === bottomLeft || tile === bottomRight){
		if(tile === topLeft) return parseInt(doubleOne + boardSizeMinus2Zeros + singleOne, 2) << totalTiles - (boardSize + 1);							// Top left
		if(tile === topRight) return parseInt(doubleOne + boardSizeMinus1Zeros + singleOne, 2) << totalTiles - (boardSize * 2);	// Top right
		if(tile === bottomLeft) return parseInt(singleOne + boardSizeMinus1Zeros + doubleOne, 2) << boardSize - 2;										// Bottom Left
		if(tile === bottomRight) return parseInt(singleOne + boardSizeMinus2Zeros + doubleOne, 2);														// Bottom Right
	}

	// Handle top side clicks
	if(topLeft < tile && topRight > tile) return parseInt(tripleOne + boardSizeMinus2Zeros + singleOne, 2) << ((boardSize - tile) - 1) + ((boardSize - 2) * boardSize);

	// Handle left side clicks
	if(tile % boardSize === 0) return parseInt(singleOne + boardSizeMinus1Zeros + doubleOne + boardSizeMinus2Zeros + singleOne, 2) << (boardSize - 1) + (((boardSize - (tile / boardSize)) - 2) * boardSize);

	// Handle right side clicks
	if(tile % boardSize === boardSize - 1) return parseInt(singleOne + boardSizeMinus2Zeros + doubleOne + boardSizeMinus1Zeros + singleOne, 2) << ((boardSize - ((tile + 1) / boardSize)) - 1) * boardSize;

	// Handle bottom side clicks
	if(bottomLeft < tile && bottomRight > tile) return parseInt(singleOne + boardSizeMinus2Zeros + tripleOne, 2) << (totalTiles - 2) - tile;

	// Everything else
	return parseInt(singleOne + boardSizeMinus2Zeros + tripleOne + boardSizeMinus2Zeros + singleOne, 2) << (totalTiles - 1) - (tile + boardSize);
}


/** Lists all the possible permutations of a state */
function getPermutations(state) {
	const permutations = [];

	for(let i = 0; i < totalTiles; i++){
		const mask = (modded) ? getModdedMask(i) : getNormalMask(i);
		permutations.push({ perm: state ^ mask, tile: i });
	}

	return permutations;
}

const keepGoing = () => maxMemory > memory.length && queue.size > 0; // Use in CPP
function doQueue(_state = solvedState) {

	/**
	 * Parent +1 = Impossible to permutate to, so we know it's a dead end.
	 * totalTiles + 1 = A tile that obviously cannot be toggled, so we know it's a dead end.
	 * moves: 0 = There are no moves left to make, so we know it's a dead end.
	 *
	 * All 3 of these combined means it'll be easy to indentify later when we solve the puzzle, meaning we know we have reached the end
	**/
	queue.set(solvedState, { state: solvedState, parent: solvedState + 1, movesToSolve: 0, optimalMove: totalTiles + 1 });

	if(rotationEnabled){
		while(queue.size > 0){
			const { state, movesToSolve, parent, optimalMove } = nextInQueue();


			if(memory[state] !== undefined) continue; // This shouldn't happen, but just in case
			memory[state] = { movesToSolve, optimalMove, parent };


			const permutations = getPermutations(state);


			for(const permutation of permutations){
				const { perm, tile } = permutation;
				if(memory[perm] !== undefined || queue.has(perm)) continue;


				const rotations = getRotations(perm);

				let isRotated = false;
				for(let i = 1; i < rotations.length; i++){
					if(memory[rotations[i]] !== undefined || queue.has(rotations[i])){
						isRotated = true;
						break;
					}
				}

				if(isRotated) continue;


				queue.set(perm, { state: perm, movesToSolve: movesToSolve + 1, parent: state, optimalMove: tile });
			}
		}
	} else {
		while(queue.size > 0){
			const { state, movesToSolve, parent, optimalMove } = nextInQueue();


			if(memory[state] !== undefined) continue; // This shouldn't happen, but just in case
			memory[state] = { movesToSolve, optimalMove, parent };


			const permutations = getPermutations(state);


			for(const permutation of permutations){
				const { perm, tile } = permutation;
				if(memory[perm] !== undefined || queue.has(perm)) continue;

				queue.set(perm, { state: perm, movesToSolve: movesToSolve + 1, parent: state, optimalMove: tile });
			}
		}
	}

	console.log("Done!\n\n");
}
console.log("Version 2.0");
console.log("\nFinding solution...");

console.time("Took");
doQueue();
console.timeEnd("Took");

console.log(`\nMemory Size: ${chalk.yellow(memory.filter(i => i !== undefined).length)} | Max Memory: ${chalk.magenta(maxMemory)}`);

const rotationsAreEnabled = (rotationEnabled) ? chalk.green("Enabled") : chalk.red("Disabled");
const isModdedGame = (modded) ? chalk.green("True") : chalk.red("False");

console.log(`Rotation: ${rotationsAreEnabled} | Modded: ${isModdedGame} | Board Size: ${chalk.blue(boardSize)}`);


/*
Version 2.0 Times

3x3 Normal:		4ms (No Rotation)
3x3 Normal:		6ms (With Rotation)

3x3 Modded:		4ms (No Rotation)
3x3 Modded:		6ms (With Rotation)



4x4 Normal:		N/A (Many combinations are impossible to solve)
4x4 Normal: 	N/A (Many combinations are impossible to solve)

4x4 Modded:		595ms (No Rotation)
4x4 Modded:		345ms (With Rotation)



5x5 Normal:		I refuse to test (No Rotation)
5x5 Normal:		7m, 40s, 108ms (With Rotation)

5x5 Modded:		N/A (Many combinations are impossible to solve)
5x5 Modded:		N/A (Many combinations are impossible to solve)

*/