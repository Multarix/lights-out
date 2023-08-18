// Times for each of the timers.
const allGameVars = {
	"game-1": {
		"timer": null,
		"interval": null,
		"started": null,
		"locked": false,
		"bogoInterval": null
	},
	"game-2": {
		"timer": null,
		"interval": null,
		"started": null,
		"locked": false,
		"bogoInterval": null
	},
	"game-3": {
		"timer": null,
		"interval": null,
		"started": null,
		"locked": false,
		"bogoInterval": null
	},
	"game-4": {
		"timer": null,
		"interval": null,
		"started": null,
		"locked": false,
		"bogoInterval": null
	}
};

/**
 * Starts the timer for a game.
 * @param {String | Number} game
 */
function startTime(game) {

	const gameID = `game-${game}`;
	const value = 0;

	allGameVars[gameID]['started'] = Date.now();
	const timeElement = document.getElementById(`${gameID}-time`);

	allGameVars[gameID]['timer'] = value;
	timeElement.innerText = allGameVars[gameID]['timer'];

	allGameVars[gameID]['interval'] = setInterval(() => {
		allGameVars[gameID]['timer'] += 1;
		timeElement.innerText = allGameVars[gameID]['timer'];
	}, 1000);

}

/**
 * Resets a board to its original state.
 * @param {String | Number} game
 */
function reset(game) {

	const gameID = `game-${game}`;

	stopBogo(game);
	const bogoButton = document.getElementById(`${gameID}-bogo`);
	bogoButton.disabled = false;

	allGameVars[gameID]['locked'] = false;
	allGameVars[gameID]['started'] = null;
	allGameVars[gameID]['timer'] = null;

	clearInterval(allGameVars[gameID]['interval']);

	document.getElementById(gameID).classList.remove('light-win');
	document.getElementById(`${gameID}-time-taken`).innerHTML = `<span id="${gameID}-time">0</span> seconds`;

	document.querySelectorAll(`#${gameID} .light-box`).forEach(element => {

		element.classList.remove('light-clicked');

		// Randomly remove elements.
		if(Math.random() > 0.65) element.classList.add('light-clicked');

	});

}


function stopBogo(game) {
	const gameID = `game-${game}`;
	clearInterval(allGameVars[gameID]['bogoInterval']);

	const bogoButton = document.getElementById(`${gameID}-bogo`);

	bogoButton.innerText = "Bogo Solve";
	bogoButton.onclick = bogoSolve.bind(null, game);
}


function bogoSolve(game) {
	const gameID = `game-${game}`;
	const allBoxes = [...document.querySelectorAll(`#${gameID} .light-box`)];

	try {
		clearInterval(allGameVars[gameID]['bogoInterval']);
	} catch (e){
		null;
	}

	const bogoButton = document.getElementById(`${gameID}-bogo`);
	bogoButton.innerText = "Stop";
	bogoButton.onclick = stopBogo.bind(null, game);

	allGameVars[gameID]['bogoInterval'] = setInterval(() => {

		if(allGameVars[gameID]['locked']) clearInterval(allGameVars[gameID]['bogoInterval']);

		const randomBox = allBoxes[Math.floor(Math.random() * allBoxes.length)];
		randomBox.click();

	}, 10);

}


/**
 * Checks if a game has been won.
 * @param {String | Number} game
 */
function checkWin(game) {

	const gameID = `game-${game}`;
	const allBoxes = document.querySelectorAll(`#${gameID} .light-box`);
	let allClicked = true;

	for(const box of allBoxes){
		if(!box.classList.contains('light-clicked')){
			allClicked = false;
			break;
		}
	}

	if(allClicked){
		stopBogo(game);
		const bogoButton = document.getElementById(`${gameID}-bogo`);
		bogoButton.disabled = true;

		allGameVars[gameID]['locked'] = true;

		const timeTaken = (Date.now() - allGameVars[gameID]["started"]) / 1000;

		document.getElementById(gameID).classList.add('light-win');
		document.getElementById(`${gameID}-time-taken`).innerHTML = `Took ${timeTaken.toFixed(2)} seconds.`;
		clearInterval(allGameVars[gameID]['interval']);
	}

}

/**
 * Toggles an element's class.
 * @param {HTMLBaseElement} element
 */
function toggle(element) {
	if(element.classList.contains('light-clicked')){
		element.classList.remove('light-clicked');
	} else {
		element.classList.add('light-clicked');
	}
}

/**
 * The standard toggle function for the lights out game.
 * @param {HTMLDivElement} element
 * @param {String | Number} game
 * @returns
 */
function toggleElementsStandard(element, game) {

	if(allGameVars[`game-${game}`]['locked']) return;
	if(!allGameVars[`game-${game}`]['started']) startTime(game);

	const clickedIDArray = element.id.split('-');

	const clickedRow = clickedIDArray[0];
	const clickedCol = clickedIDArray[1];
	const clickedGame = clickedIDArray[2];

	const topID = `${parseInt(clickedRow) - 1}-${clickedCol}-${clickedGame}`;
	const top = document.getElementById(topID);

	const botID = `${parseInt(clickedRow) + 1}-${clickedCol}-${clickedGame}`;
	const bot = document.getElementById(botID);

	const leftID = `${clickedRow}-${parseInt(clickedCol) - 1}-${clickedGame}`;
	const left = document.getElementById(leftID);

	const rightID = `${clickedRow}-${parseInt(clickedCol) + 1}-${clickedGame}`;
	const right = document.getElementById(rightID);

	const validElements = [element];

	if(top) validElements.push(top);
	if(bot) validElements.push(bot);
	if(left) validElements.push(left);
	if(right) validElements.push(right);

	for(const element of validElements){
		toggle(element);
	}

	checkWin(game);
}

/**
 * The standard toggle function for the lights out game.
 * @param {HTMLDivElement} element
 * @param {String | Number} game
 * @returns
 */
function toggleElementsModified(element, game) {

	if(allGameVars[`game-${game}`]['locked']) return;
	if(!allGameVars[`game-${game}`]['started']) startTime(game);

	const clickedIDArray = element.id.split('-');

	const clickedRow = clickedIDArray[0];
	const clickedCol = clickedIDArray[1];
	const clickedGame = clickedIDArray[2];

	const topID = `${parseInt(clickedRow) - 1}-${clickedCol}-${clickedGame}`;
	const top = document.getElementById(topID);

	const botID = `${parseInt(clickedRow) + 1}-${clickedCol}-${clickedGame}`;
	const bot = document.getElementById(botID);

	const leftID = `${clickedRow}-${parseInt(clickedCol) - 1}-${clickedGame}`;
	const left = document.getElementById(leftID);

	const rightID = `${clickedRow}-${parseInt(clickedCol) + 1}-${clickedGame}`;
	const right = document.getElementById(rightID);

	const topLeftID = `${parseInt(clickedRow) - 1}-${parseInt(clickedCol) - 1}-${clickedGame}`;
	const topLeft = document.getElementById(topLeftID);

	const topRightID = `${parseInt(clickedRow) - 1}-${parseInt(clickedCol) + 1}-${clickedGame}`;
	const topRight = document.getElementById(topRightID);

	const botLeftID = `${parseInt(clickedRow) + 1}-${parseInt(clickedCol) - 1}-${clickedGame}`;
	const botLeft = document.getElementById(botLeftID);

	const botRightID = `${parseInt(clickedRow) + 1}-${parseInt(clickedCol) + 1}-${clickedGame}`;
	const botRight = document.getElementById(botRightID);

	const validElements = [element];

	if(top) validElements.push(top);
	if(bot) validElements.push(bot);
	if(left) validElements.push(left);
	if(right) validElements.push(right);
	if(topLeft) validElements.push(topLeft);
	if(topRight) validElements.push(topRight);
	if(botLeft) validElements.push(botLeft);
	if(botRight) validElements.push(botRight);

	for(const element of validElements){
		toggle(element);
	}

	checkWin(game);
}



document.querySelectorAll('.light-box').forEach(element => {

	// Ensure it's a random start each time.
	if(Math.random() > 0.65) element.classList.add('light-clicked');

	const game = element.id.split("-")[2];
	if(["1", "2"].includes(game)){
		element.addEventListener('click', toggleElementsStandard.bind(this, element, game));
	}

	if(["3", "4"].includes(game)){
		element.addEventListener('click', toggleElementsModified.bind(this, element, game));
	}

});