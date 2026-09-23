#include "EepyController.h"
#include "Ghost.h"

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

	Move best = moves[0];
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
}


EepyController::EepyController(std::shared_ptr<Character> character):
		Controller(character),
		fsm(std::make_shared<EepyStateMachine>(character)) {
}

EepyController::~EepyController() {
}

Move EepyController::getMove(const GameState& game) {
	return fsm->update(game);
}


TiredState::TiredState(std::shared_ptr<Character> _character):
		FSMState(_character), frozenTarget(-1) {
}

TiredState::~TiredState() {
}

void TiredState::onEnter(const GameState& gs) {
	frozenTarget = gs.getPacmanPos();

	for (auto& t : transitions) {
		auto timed = std::dynamic_pointer_cast<TimedTransition>(t);
		if (timed) timed->reset();
	}
}

Move TiredState::onUpdate(const GameState& gs) {
	auto target = gs.getMaze().getNodePos(frozenTarget);
	return bestMoveTowards(gs, character, target);
}


TiredCycleFSM::TiredCycleFSM(std::shared_ptr<Character> _character):
		FiniteStateMachine(_character) {

	auto chase = std::make_shared<GhostChaseState>(character);
	auto tired = std::make_shared<TiredState>(character);
	auto scatter = std::make_shared<GhostScatterState>(character);

	chase->addTransition(std::make_shared<TimedTransition>(tired, 10));
	tired->addTransition(std::make_shared<TimedTransition>(scatter, 10));
	scatter->addTransition(std::make_shared<TimedTransition>(chase, 7));

	states.push_back(chase);
	states.push_back(tired);
	states.push_back(scatter);

	initialState = chase;
	activeState = initialState;
}

TiredCycleFSM::~TiredCycleFSM() {
}

Move TiredCycleFSM::update(const GameState& gs) {
	auto t = activeState->getActiveTransition(gs);
	if (t != nullptr) {
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}


TiredNonfrightenedState::TiredNonfrightenedState(std::shared_ptr<Character> _character):
		FSMState(_character),
		subFsm(std::make_shared<TiredCycleFSM>(_character)) {
}

TiredNonfrightenedState::~TiredNonfrightenedState() {
}

void TiredNonfrightenedState::onEnter(const GameState&) {
	std::dynamic_pointer_cast<Ghost>(character)->revert();
}

Move TiredNonfrightenedState::onUpdate(const GameState& gs) {
	return subFsm->update(gs);
}


EepyStateMachine::EepyStateMachine(std::shared_ptr<Character> _character):
		FiniteStateMachine(_character) {

	auto nonfrightened = std::make_shared<TiredNonfrightenedState>(character);
	auto frightened = std::make_shared<FrightenedState>(character);

	nonfrightened->addTransition(std::make_shared<EdibleTransition>(character, frightened));
	frightened->addTransition(std::make_shared<NotEdibleTransition>(character, nonfrightened));

	states.push_back(nonfrightened);
	states.push_back(frightened);

	initialState = nonfrightened;
	activeState = initialState;
}

EepyStateMachine::~EepyStateMachine() {
}

Move EepyStateMachine::update(const GameState& gs) {
	auto t = activeState->getActiveTransition(gs);
	if (t != nullptr) {
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}