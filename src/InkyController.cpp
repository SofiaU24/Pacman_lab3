/*#include "InkyController.h"


InkyController::InkyController(std::shared_ptr<Character> character):
	Controller(character){
}

InkyController::~InkyController() {

}

Move
InkyController::getMove(const GameState& game){
	return PASS;	

	
}*/
//Tarea 1
#include "InkyController.h"
#include "Ghost.h"

namespace {
Move bestMoveTowards(const GameState& gamestate, const std::shared_ptr<Character>& character,
		std::pair<int,int> target) {
	int pos = character->getPos();
	std::vector<Move> moves;
	if (character->getDirection() == PASS) {
		moves = gamestate.getMaze().getPossibleMoves(pos);
	} else {
		moves = gamestate.getMaze().getGhostLegalMoves(pos, character->getDirection());
	}

	Move best = PASS;
	float minDist = -1;
	for (auto m : moves) {
		if (m == PASS) continue;
		int neighbour = gamestate.getMaze().getNeighbour(pos, m);
		if (neighbour < 0) continue;
		float dist = euclid2(gamestate.getMaze().getNodePos(neighbour), target);
		if (minDist == -1 || dist < minDist) {
			minDist = dist;
			best = m;
		}
	}
	return best;
}
}

InkyController::InkyController(std::shared_ptr<Character> character):
		Controller(character),
		blackboard(std::make_shared<InkyBlackboard>()),
		root(std::make_shared<Selector>()) {

	blackboard->character = character;

	//mientras siga en casa, se queda quieto
	auto houseSequence = std::make_shared<Sequence>();
	houseSequence->addChild(std::make_shared<InkyInHouse>(blackboard));//condicion primero
	houseSequence->addChild(std::make_shared<InkyWaitInHouse>(blackboard));//accion despues

	auto frightenedSequence = std::make_shared<Sequence>();
	frightenedSequence->addChild(std::make_shared<InkyIsFrightened>(blackboard));
	frightenedSequence->addChild(std::make_shared<InkyFrightened>(blackboard));

	auto scatterSequence = std::make_shared<Sequence>();
	scatterSequence->addChild(std::make_shared<InkyScatterWindow>());
	scatterSequence->addChild(std::make_shared<InkyScatter>(blackboard));

	//el selector prueba cada rama en orden hasta que una tenga exito,en cualquier otro caso persigue
	root->addChild(houseSequence);
	root->addChild(frightenedSequence);
	root->addChild(scatterSequence);
	root->addChild(std::make_shared<InkyChase>(blackboard));
}

InkyController::~InkyController() {
}

Move InkyController::getMove(const GameState& game) {
	blackboard->gamestate = &game;
	root->tick();
	return blackboard->outMove;
}


InkyInHouse::InkyInHouse(std::shared_ptr<InkyBlackboard> blackboard): blackboard(blackboard), totalPills(-1) {
}

Status InkyInHouse::update() {
	int remaining = static_cast<int>(blackboard->gamestate->getMaze().getPillPositions().size());
	if (totalPills < 0) totalPills = remaining;
	int eaten = totalPills - remaining;
	if (eaten <= 30) {
		return BH_SUCCESS;
	}
	return BH_FAILURE;
}

InkyWaitInHouse::InkyWaitInHouse(std::shared_ptr<InkyBlackboard> blackboard): blackboard(blackboard) {
}

Status InkyWaitInHouse::update() {
	blackboard->outMove = PASS;
	return BH_SUCCESS;
}


InkyIsFrightened::InkyIsFrightened(std::shared_ptr<InkyBlackboard> blackboard): blackboard(blackboard) {
}

Status InkyIsFrightened::update() {
	auto ghost = std::dynamic_pointer_cast<Ghost>(blackboard->character);
	if (ghost != nullptr && ghost->isEdible()) {
		return BH_SUCCESS;
	}
	return BH_FAILURE;
}

InkyFrightened::InkyFrightened(std::shared_ptr<InkyBlackboard> blackboard):
		blackboard(blackboard), e(std::random_device{}()) {
}

Status InkyFrightened::update() {
	int pos = blackboard->character->getPos();
	std::vector<Move> moves;
	if (blackboard->character->getDirection() == PASS) {
		moves = blackboard->gamestate->getMaze().getPossibleMoves(pos);
	} else {
		moves = blackboard->gamestate->getMaze().getGhostLegalMoves(pos, blackboard->character->getDirection());
	}
	blackboard->outMove = moves[e() % moves.size()];
	return BH_SUCCESS;
}


InkyScatterWindow::InkyScatterWindow():
		startTime(std::chrono::high_resolution_clock::now()) {
}

Status InkyScatterWindow::update() {
	std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - startTime;
	if ((int) elapsed.count() % 27 < 7) {
		return BH_SUCCESS;
	}
	return BH_FAILURE;
}

InkyScatter::InkyScatter(std::shared_ptr<InkyBlackboard> blackboard):
		blackboard(blackboard), homeNode(blackboard->character->getPos()) {
}

Status InkyScatter::update() {
	auto target = blackboard->gamestate->getMaze().getNodePos(homeNode);
	blackboard->outMove = bestMoveTowards(*blackboard->gamestate, blackboard->character, target);
	return BH_SUCCESS;
}


InkyChase::InkyChase(std::shared_ptr<InkyBlackboard> blackboard): blackboard(blackboard) {
}

Status InkyChase::update() {
	const Maze& maze = blackboard->gamestate->getMaze();
	int pacmanNode = blackboard->gamestate->getPacmanPos();
	Move pacmanDir = static_cast<Move>(blackboard->gamestate->getPacmanDir());

	int aheadNode = pacmanNode;
	if (pacmanDir != PASS) {
		for (int i = 0; i < 2; i++) {
			int next = maze.getNeighbour(aheadNode, pacmanDir);
			if (next < 0) break;
			aheadNode = next;
		}
	}
	auto aheadCoords = maze.getNodePos(aheadNode);

	int blinkyNode = blackboard->gamestate->getGhostsPos(0);
	auto blinkyCoords = maze.getNodePos(blinkyNode);

	std::pair<int,int> target = {
		2 * aheadCoords.first  - blinkyCoords.first,
		2 * aheadCoords.second - blinkyCoords.second
	};

	blackboard->outMove = bestMoveTowards(*blackboard->gamestate, blackboard->character, target);
	return BH_SUCCESS;
}
