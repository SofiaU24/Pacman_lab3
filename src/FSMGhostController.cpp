#include "FSMGhostController.h"


FSMGhostController::FSMGhostController(std::shared_ptr<Character> character):
	Controller(character),
	fsm(std::make_shared<GhostStateMachine>(character)) {
}

FSMGhostController::~FSMGhostController() {
}

Move FSMGhostController::getMove(const GameState& game){
	return fsm->update(game);
}


TimedTransition::TimedTransition(std::shared_ptr<FSMState> _next, int seconds):
	start(std::chrono::steady_clock::now()),
	duration(seconds),
	next(_next){
}

void TimedTransition::reset(){
	start = std::chrono::steady_clock::now();
}

bool TimedTransition::isValid(const GameState&){
	return (std::chrono::steady_clock::now() - start) >= duration;
}

std::shared_ptr<FSMState> TimedTransition::getNextState(){
	return next;
}


GhostChaseState::GhostChaseState(std::shared_ptr<Character> _character):
	FSMState(_character){
}

GhostChaseState::~GhostChaseState(){
}

void GhostChaseState::onEnter(const GameState&){
	// cada vez que vuelva a Chase reinicia el temporizador de 20s
	for(auto& t: transitions){
		auto timed = std::dynamic_pointer_cast<TimedTransition>(t);
		if(timed) timed->reset();
	}
}

Move GhostChaseState::onUpdate(const GameState& gs){
	int pos = character->getPos();
	auto target = gs.getMaze().getNodePos(gs.getPacmanPos());

	std::vector<Move> moves;
	if(character->getDirection()==PASS){
		moves = gs.getMaze().getPossibleMoves(pos);
	}else{
		moves = gs.getMaze().getGhostLegalMoves(pos, character->getDirection());
	}

	Move best = moves[0];
	float minDist = -1;
	for(Move m: moves){
		if(m==PASS) continue;
		int vecino = gs.getMaze().getNeighbour(pos, m);
		if(vecino<0) continue;
		float dist = euclid2(gs.getMaze().getNodePos(vecino), target);
		if(minDist==-1 || dist<minDist){
			minDist = dist;
			best = m;
		}
	}
	return best;
}


GhostScatterState::GhostScatterState(std::shared_ptr<Character> _character):
	FSMState(_character),
	homeNode(_character->getPos()){ // nodo de inicio es lugar de scatter
}

GhostScatterState::~GhostScatterState(){
}

void GhostScatterState::onEnter(const GameState&){
	// cada vez que vuelva a Scatter reinicia el temporizador de 7s
	for(auto& t: transitions){
		auto timed = std::dynamic_pointer_cast<TimedTransition>(t);
		if(timed) timed->reset();
	}
}

Move GhostScatterState::onUpdate(const GameState& gs){
	int pos = character->getPos();
	auto target = gs.getMaze().getNodePos(homeNode);

	std::vector<Move> moves;
	if(character->getDirection()==PASS){
		moves = gs.getMaze().getPossibleMoves(pos);
	}else{
		moves = gs.getMaze().getGhostLegalMoves(pos, character->getDirection());
	}

	Move best = moves[0];
	float minDist = -1;
	for(Move m: moves){
		if(m==PASS) continue;
		int vecino = gs.getMaze().getNeighbour(pos, m);
		if(vecino<0) continue;
		float dist = euclid2(gs.getMaze().getNodePos(vecino), target);
		if(minDist==-1 || dist<minDist){
			minDist = dist;
			best = m;
		}
	}
	return best;
}


ChaseScatterFSM::ChaseScatterFSM(std::shared_ptr<Character> _character):
	FiniteStateMachine(_character){

	auto chase = std::make_shared<GhostChaseState>(character);
	auto scatter = std::make_shared<GhostScatterState>(character);

	chase->addTransition(std::make_shared<TimedTransition>(scatter, 20));   // 20s en Chase
	scatter->addTransition(std::make_shared<TimedTransition>(chase, 7));    //  7s en Scatter

	states.push_back(chase);
	states.push_back(scatter);

	initialState = chase;
	activeState = initialState;
}

ChaseScatterFSM::~ChaseScatterFSM(){
}

Move ChaseScatterFSM::update(const GameState& gs){
	auto t = activeState->getActiveTransition(gs);
	if(t!=nullptr){
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}


NonfrightenedState::NonfrightenedState(std::shared_ptr<Character> _character):
	FSMState(_character),
	subFsm(std::make_shared<ChaseScatterFSM>(_character)){
}

NonfrightenedState::~NonfrightenedState(){
}

void NonfrightenedState::onEnter(const GameState&){
	// invierte direccion cuando vuelve a frightned
	std::dynamic_pointer_cast<Ghost>(character)->revert();
}

Move NonfrightenedState::onUpdate(const GameState& gs){
	return subFsm->update(gs);
}


FrightenedState::FrightenedState(std::shared_ptr<Character> _character):
	FSMState(_character),
	e(std::random_device{}()){
}

FrightenedState::~FrightenedState(){
}

Move FrightenedState::onUpdate(const GameState& gs){
	int pos = character->getPos();
	std::vector<Move> moves;
	if(character->getDirection()==PASS){
		moves = gs.getMaze().getPossibleMoves(pos);
	}else{
		moves = gs.getMaze().getGhostLegalMoves(pos, character->getDirection());
	}
	return moves[e() % moves.size()];
}


EdibleTransition::EdibleTransition(std::shared_ptr<Character> _character, std::shared_ptr<FSMState> _next):
	character(_character), next(_next){
}

bool EdibleTransition::isValid(const GameState&){
	auto ghost = std::dynamic_pointer_cast<Ghost>(character);
	return ghost && ghost->isEdible();
}

std::shared_ptr<FSMState> EdibleTransition::getNextState(){
	return next;
}


NotEdibleTransition::NotEdibleTransition(std::shared_ptr<Character> _character, std::shared_ptr<FSMState> _next):
	character(_character), next(_next){
}

bool NotEdibleTransition::isValid(const GameState&){
	auto ghost = std::dynamic_pointer_cast<Ghost>(character);
	return ghost && !ghost->isEdible();
}

std::shared_ptr<FSMState> NotEdibleTransition::getNextState(){
	return next;
}


GhostStateMachine::GhostStateMachine(std::shared_ptr<Character> _character):
	FiniteStateMachine(_character){

	auto nonfrightened = std::make_shared<NonfrightenedState>(character);
	auto frightened = std::make_shared<FrightenedState>(character);

	nonfrightened->addTransition(std::make_shared<EdibleTransition>(character, frightened));
	frightened->addTransition(std::make_shared<NotEdibleTransition>(character, nonfrightened));

	states.push_back(nonfrightened);
	states.push_back(frightened);

	initialState = nonfrightened;
	activeState = initialState;
}

GhostStateMachine::~GhostStateMachine(){
}

Move GhostStateMachine::update(const GameState& gs){
	auto t = activeState->getActiveTransition(gs);
	if(t!=nullptr){
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}