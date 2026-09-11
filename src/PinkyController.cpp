/*#include "PinkyController.h"


PinkyController::PinkyController(std::shared_ptr<Character> character):
	Controller(character){
}

PinkyController::~PinkyController() {

}

Move
PinkyController::getMove(const GameState& game){
	return PASS;	

	
}*/
//Laboratorio 5
#include "PinkyController.h"
#include "Ghost.h"

namespace {
// Devuelve el mejor movimiento para ir a nodo destino
Move bestMoveTowards(const GameState& gs, const std::shared_ptr<Character>& character,
		std::pair<int,int> target) {
	int pos = character->getPos();
	std::vector<Move> moves;
	if (character->getDirection() == PASS) {// si no se mueve, puede ir a cualquier dirección
		moves = gs.getMaze().getPossibleMoves(pos);
	} else { 
		moves = gs.getMaze().getGhostLegalMoves(pos, character->getDirection()); 
	}

	Move best = PASS;
	float minDist = -1;
	for (auto m : moves) {// para cada movimiento posible, calcula la distancia al objetivo
		if (m == PASS) continue;
		int neighbour = gs.getMaze().getNeighbour(pos, m);
		if (neighbour < 0) continue;
		float dist = euclid2(gs.getMaze().getNodePos(neighbour), target);
		if (minDist == -1 || dist < minDist) {
			minDist = dist;
			best = m;
		}
	}
	return best;
}
}

PinkyController::PinkyController(std::shared_ptr<Character> character):
		Controller(character),
		bb(std::make_shared<PinkyBlackboard>()),
		root(std::make_shared<Selector>()) {

	bb->character = character;

	//si está asustado, huye.
	auto frightenedFilter = std::make_shared<Filter>();
	frightenedFilter->addCondition(std::make_shared<PinkyIsFrightened>(bb));
	frightenedFilter->addAction(std::make_shared<PinkyFrightened>(bb));

	//si toca ventana de Scatter, vuelve a su esquina.
	auto scatterFilter = std::make_shared<Filter>();
	scatterFilter->addCondition(std::make_shared<PinkyScatterWindow>());
	scatterFilter->addAction(std::make_shared<PinkyScatter>(bb));

	//persigue a Pacman (emboscada).
	root->addChild(frightenedFilter);
	root->addChild(scatterFilter);
	root->addChild(std::make_shared<PinkyChase>(bb));
}

PinkyController::~PinkyController() {
}

Move PinkyController::getMove(const GameState& game) {
	bb->gs = &game;
	root->tick();
	return bb->outMove;
}

PinkyIsFrightened::PinkyIsFrightened(std::shared_ptr<PinkyBlackboard> bb): bb(bb) {
}

Status PinkyIsFrightened::update() {
	auto ghost = std::dynamic_pointer_cast<Ghost>(bb->character);
	if (ghost != nullptr && ghost->isEdible()) {
		return BH_SUCCESS;
	}
	return BH_FAILURE;
}

PinkyFrightened::PinkyFrightened(std::shared_ptr<PinkyBlackboard> bb):
		bb(bb), e(std::random_device{}()) {
}

Status PinkyFrightened::update() {
	int pos = bb->character->getPos();
	std::vector<Move> moves;
	if (bb->character->getDirection() == PASS) {
		moves = bb->gs->getMaze().getPossibleMoves(pos);
	} else {
		moves = bb->gs->getMaze().getGhostLegalMoves(pos, bb->character->getDirection());
	}
	bb->outMove = moves[e() % moves.size()];
	return BH_SUCCESS;
}

PinkyScatterWindow::PinkyScatterWindow():
		startTime(std::chrono::high_resolution_clock::now()) {
}

Status PinkyScatterWindow::update() {
	// Ciclo de 27s: 20s en Chase + 7s en Scatter (igual que el FSM de Blinky del lab 4).
	std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - startTime;
	if ((int) elapsed.count() % 27 < 7) {
		return BH_SUCCESS;
	}
	return BH_FAILURE;
}

PinkyScatter::PinkyScatter(std::shared_ptr<PinkyBlackboard> bb):
		bb(bb), homeNode(bb->character->getPos()) {
}

Status PinkyScatter::update() {
	auto target = bb->gs->getMaze().getNodePos(homeNode);
	bb->outMove = bestMoveTowards(*bb->gs, bb->character, target);
	return BH_SUCCESS;
}

PinkyChase::PinkyChase(std::shared_ptr<PinkyBlackboard> bb): bb(bb) {
}

Status PinkyChase::update() {
	const Maze& maze = bb->gs->getMaze();
	int pacmanNode = bb->gs->getPacmanPos();
	Move pacmanDir = static_cast<Move>(bb->gs->getPacmanDir());

	// Emboscada 4 nodos por delante de Pacman en su dirección actual.
	int targetNode = pacmanNode;
	if (pacmanDir != PASS) {
		for (int i = 0; i < 4; i++) {
			int next = maze.getNeighbour(targetNode, pacmanDir);
			if (next < 0) break;
			targetNode = next;
		}
	}

	auto target = maze.getNodePos(targetNode);
	bb->outMove = bestMoveTowards(*bb->gs, bb->character, target);
	return BH_SUCCESS;
}
