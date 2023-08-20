type State = string;
type Matrix = any[][];
interface Permutation {
    permutation: State;
    tile: number;
}
interface MemoryState {
    movesToSolve: number;
    optimalMove: number | null;
    parentState: State | null;
}
interface QueueState {
    moves: number;
    optimal: number | null;
    state: State;
    parent: State | null;
}
declare const width = 5;
declare const height = 5;
declare const totalTiles: number;
declare const solvedState: string;
declare const splitRegex: RegExp;
/** Rotates a 2D matrix 90 degrees clockwise
 * @remarks Code transpiled to JS from a comment on stackoverflow - https://stackoverflow.com/a/35438327 */
declare function rotate(matrix: Matrix): void;
/** Produces a list of all possible rotations of a given state */
declare function getRotations(_matrix: Matrix): State[];
/** Expands a state into a 2D array of booleans */
declare function expandState(_state: State): Matrix;
/** Shrinks an expanded state back down to a string */
declare function shrinkState(_expandedState: Matrix): State;
/** Toggles the tile and appropriate adjacent tiles in a state */
declare function toggleTiles(state: State, row: number, col: number): State;
/** Produces a single permutation of a given state */
declare function permutate(state: State, tile: number): State;
/** Lists all the possible permutations of a state */
declare function getPermutations(_state: State): Permutation[];
declare const memory: Map<State, MemoryState>;
declare const queue: QueueState[];
declare const nextInQueue: () => QueueState | undefined;
declare function doQueue(_state?: State): void;
