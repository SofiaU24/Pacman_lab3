#include "PacmanBTController.h"
#include <limits>
#include <queue>
#include <unordered_set>

namespace {
Move bestMoveTowards(const Maze& maze, int pos, std::pair<int,int> target) {
	Move best = PASS;
	float minDist = std::numeric_limits<float>::max();
	for (Move m : maze.getPossibleMoves(pos)) {
		if (m == PASS) continue;
		float dist = euclid2(maze.getNodePos(maze.getNeighbour(pos, m)), target);
		if (dist < minDist) {
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

Move firstMoveToNearestPill(const Maze& maze, int start) {
	std::unordered_set<int> visited;
	visited.insert(start);
	std::queue<std::pair<int,Move>> pending;
	for (Move m : {UP, RIGHT, DOWN, LEFT}) {
		int next = maze.getNeighbour(start, m);
		if (next < 0) continue;
		visited.insert(next);
		pending.push(std::make_pair(next, m));
	}
	while (!pending.empty()) {
		std::pair<int,Move> cur = pending.front();
		pending.pop();
		if (maze.hasPill(cur.first) || maze.hasPowerPill(cur.first)) return cur.second;
		for (Move m : {UP, RIGHT, DOWN, LEFT}) {
			int next = maze.getNeighbour(cur.first, m);
			if (next < 0 || visited.count(next)) continue;
			visited.insert(next);
			pending.push(std::make_pair(next, cur.second));
		}
	}
	return PASS;
}
}

PacmanBTController::PacmanBTController(std::shared_ptr<Character> character):
		Controller(character),
		bb(std::make_shared<PacmanBlackboard>()),
		root(std::make_shared<Selector>()) {

	bb->character = character;

	auto fleeFilter = std::make_shared<Filter>();
	fleeFilter->addCondition(std::make_shared<PacmanDanger>(bb));
	fleeFilter->addAction(std::make_shared<PacmanFlee>(bb));

	auto chaseFilter = std::make_shared<Filter>();
	chaseFilter->addCondition(std::make_shared<PacmanHasEdible>(bb));
	chaseFilter->addAction(std::make_shared<PacmanChase>(bb));

	root->addChild(fleeFilter);
	root->addChild(chaseFilter);
	root->addChild(std::make_shared<PacmanEatPills>(bb));
}

PacmanBTController::~PacmanBTController() {
}

Move PacmanBTController::getMove(const GameState& game) {
	bb->gs = &game;
	root->tick();
	return bb->outMove;
}


PacmanDanger::PacmanDanger(std::shared_ptr<PacmanBlackboard> bb): bb(bb) {
}

Status PacmanDanger::update() {
	const GameState& gs = *bb->gs;
	for (int i = 0; i < gs.getNumGhosts(); i++) {
		if (gs.isGhostEdible(i)) continue;
		if (withinNodes(gs.getMaze(), bb->character->getPos(), gs.getGhostsPos(i), PACMAN_DANGER_NODES)) {
			return BH_SUCCESS;
		}
	}
	return BH_FAILURE;
}


PacmanFlee::PacmanFlee(std::shared_ptr<PacmanBlackboard> bb): bb(bb) {
}

Status PacmanFlee::update() {
	const GameState& gs = *bb->gs;
	const Maze& maze = gs.getMaze();
	int pos = bb->character->getPos();

	Move best = PASS;
	float maxDist = -1;
	for (Move m : maze.getPossibleMoves(pos)) {
		if (m == PASS) continue;
		auto next = maze.getNodePos(maze.getNeighbour(pos, m));
		float nearest = std::numeric_limits<float>::max();
		for (int i = 0; i < gs.getNumGhosts(); i++) {
			if (gs.isGhostEdible(i)) continue;
			float dist = euclid2(next, maze.getNodePos(gs.getGhostsPos(i)));
			if (dist < nearest) nearest = dist;
		}
		if (nearest > maxDist) {
			maxDist = nearest;
			best = m;
		}
	}
	bb->outMove = best;
	return BH_SUCCESS;
}


PacmanHasEdible::PacmanHasEdible(std::shared_ptr<PacmanBlackboard> bb): bb(bb) {
}

Status PacmanHasEdible::update() {
	for (int i = 0; i < bb->gs->getNumGhosts(); i++) {
		if (bb->gs->isGhostEdible(i)) return BH_SUCCESS;
	}
	return BH_FAILURE;
}


PacmanChase::PacmanChase(std::shared_ptr<PacmanBlackboard> bb): bb(bb) {
}

Status PacmanChase::update() {
	const GameState& gs = *bb->gs;
	const Maze& maze = gs.getMaze();
	int pos = bb->character->getPos();
	auto here = maze.getNodePos(pos);

	auto target = here;
	float minDist = std::numeric_limits<float>::max();
	for (int i = 0; i < gs.getNumGhosts(); i++) {
		if (!gs.isGhostEdible(i)) continue;
		auto ghostPos = maze.getNodePos(gs.getGhostsPos(i));
		float dist = euclid2(here, ghostPos);
		if (dist < minDist) {
			minDist = dist;
			target = ghostPos;
		}
	}
	bb->outMove = bestMoveTowards(maze, pos, target);
	return BH_SUCCESS;
}


PacmanEatPills::PacmanEatPills(std::shared_ptr<PacmanBlackboard> bb): bb(bb) {
}

Status PacmanEatPills::update() {
	bb->outMove = firstMoveToNearestPill(bb->gs->getMaze(), bb->character->getPos());
	return BH_SUCCESS;
}