#include "EchoController.h"
#include "Ghost.h"
#include <queue>
#include <unordered_set>

namespace {
Move bestMoveTowards(const GameState& gs, const std::shared_ptr<Character>& character,
		std::pair<int,int> target) {
	int pos = character->getPos();
	std::vector<Move> moves;
	if (character->getDirection() == PASS) {
		moves = gs.getMaze().getPossibleMoves(pos);
	} else {
		moves = gs.getMaze().getGhostLegalMoves(pos, character->getDirection());
	}

	Move best = PASS;
	float minDist = -1;
	for (auto m : moves) {
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

bool withinNodes(const Maze& maze, int from, int to, int limit) {
	if (from == to) return true;
	std::unordered_set<int> visited;
	visited.insert(from);
	std::queue<std::pair<int,int>> pending;
	pending.push(std::make_pair(from, 0));
	while (!pending.empty()) {
		std::pair<int,int> cur = pending.front();
		pending.pop();
		if (cur.second + 1 >= limit) continue;
		for (Move m : {UP, RIGHT, DOWN, LEFT}) {
			int next = maze.getNeighbour(cur.first, m);
			if (next < 0 || visited.count(next)) continue;
			if (next == to) return true;
			visited.insert(next);
			pending.push(std::make_pair(next, cur.second + 1));
		}
	}
	return false;
}
}

EchoController::EchoController(std::shared_ptr<Character> character):
		Controller(character),
		bb(std::make_shared<EchoBlackboard>()),
		root(std::make_shared<Selector>()),
		lastPills(0) {

	bb->character = character;

	auto frightenedFilter = std::make_shared<Filter>();
	frightenedFilter->addCondition(std::make_shared<EchoIsFrightened>(bb));
	frightenedFilter->addAction(std::make_shared<EchoFrightened>(bb));

	auto scatterFilter = std::make_shared<Filter>();
	scatterFilter->addCondition(std::make_shared<EchoPacmanNear>(bb, true));
	scatterFilter->addAction(std::make_shared<EchoScatter>(bb));

	auto mimicFilter = std::make_shared<Filter>();
	mimicFilter->addCondition(std::make_shared<EchoPacmanNear>(bb, false));
	mimicFilter->addAction(std::make_shared<EchoMimic>(bb));

	root->addChild(frightenedFilter);
	root->addChild(scatterFilter);
	root->addChild(mimicFilter);
}

EchoController::~EchoController() {
}

Move EchoController::getMove(const GameState& game) {
	bb->gs = &game;

	size_t pills = game.getMaze().getPillPositions().size();
	if (pills > lastPills) bb->history.clear();
	lastPills = pills;

	bb->history.push_back(game.getPacmanPos());
	if (bb->history.size() > ECHO_DELAY) bb->history.pop_front();

	root->tick();
	return bb->outMove;
}


EchoIsFrightened::EchoIsFrightened(std::shared_ptr<EchoBlackboard> bb): bb(bb) {
}

Status EchoIsFrightened::update() {
	auto ghost = std::dynamic_pointer_cast<Ghost>(bb->character);
	if (ghost != nullptr && ghost->isEdible()) {
		return BH_SUCCESS;
	}
	return BH_FAILURE;
}


EchoFrightened::EchoFrightened(std::shared_ptr<EchoBlackboard> bb):
		bb(bb), e(std::random_device{}()) {
}

Status EchoFrightened::update() {
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


EchoPacmanNear::EchoPacmanNear(std::shared_ptr<EchoBlackboard> bb, bool wantNear):
		bb(bb), wantNear(wantNear) {
}

Status EchoPacmanNear::update() {
	bool near = withinNodes(bb->gs->getMaze(), bb->character->getPos(),
			bb->gs->getPacmanPos(), ECHO_NEAR);
	return (near == wantNear) ? BH_SUCCESS : BH_FAILURE;
}


EchoScatter::EchoScatter(std::shared_ptr<EchoBlackboard> bb): bb(bb) {
}

Status EchoScatter::update() {
	const Maze& maze = bb->gs->getMaze();
	auto home = maze.getNodePos(maze.getGhostStart()[ECHO_START_SLOT]);
	bb->outMove = bestMoveTowards(*bb->gs, bb->character, home);
	return BH_SUCCESS;
}


EchoMimic::EchoMimic(std::shared_ptr<EchoBlackboard> bb): bb(bb) {
}

Status EchoMimic::update() {
	const Maze& maze = bb->gs->getMaze();
	int pacmanNode = bb->gs->getPacmanPos();

	int target = pacmanNode;
	if (bb->history.size() >= ECHO_DELAY) target = bb->history.front();
	if (target == bb->character->getPos()) target = pacmanNode;

	bb->outMove = bestMoveTowards(*bb->gs, bb->character, maze.getNodePos(target));
	return BH_SUCCESS;
}