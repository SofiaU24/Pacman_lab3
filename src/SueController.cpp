/*#include "SueController.h"


SueController::SueController(std::shared_ptr<Character> character):
	Controller(character){
}

SueController::~SueController() {

}

Move
SueController::getMove(const GameState& game){
	return PASS;	

	
}*/
//Tarea 1

#include "SueController.h"


SueController::SueController(std::shared_ptr<Character> character):
	Controller(character),
	fsm(std::make_shared<SueStateMachine>(character)) {
}

SueController::~SueController() {
}

Move SueController::getMove(const GameState& game){
	return fsm->update(game);
}



DistanceTransition::DistanceTransition(std::shared_ptr<Character> _character, std::shared_ptr<FSMState> _next, float tilesThreshold, bool _whenCloser):
	character(_character), next(_next), thresholdSq(tilesThreshold*tilesThreshold), whenCloser(_whenCloser){
}

bool DistanceTransition::isValid(const GameState& gs){
	auto myCoords = gs.getMaze().getNodePos(character->getPos());
	auto pacCoords = gs.getMaze().getNodePos(gs.getPacmanPos());
	float dist = euclid2(myCoords, pacCoords);
	return whenCloser ? (dist < thresholdSq) : (dist >= thresholdSq);
}

std::shared_ptr<FSMState> DistanceTransition::getNextState(){
	return next;
}


SueBehaviorFSM::SueBehaviorFSM(std::shared_ptr<Character> _character):
	FiniteStateMachine(_character){

	auto chase = std::make_shared<GhostChaseState>(character);     //reutilizado de Blinky
	auto scatter = std::make_shared<GhostScatterState>(character); //reutilizado de Blinky

	chase->addTransition(std::make_shared<DistanceTransition>(character, scatter, 8, true));   //distancia < 8 huye
	scatter->addTransition(std::make_shared<DistanceTransition>(character, chase, 8, false));   // distancia >= 8 persigue

	states.push_back(chase);
	states.push_back(scatter);

	initialState = chase;
	activeState = initialState;
}

SueBehaviorFSM::~SueBehaviorFSM(){
}

Move SueBehaviorFSM::update(const GameState& gs){
	auto t = activeState->getActiveTransition(gs);
	if(t!=nullptr){
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}


SueNonfrightenedState::SueNonfrightenedState(std::shared_ptr<Character> _character):
	FSMState(_character),
	subFsm(std::make_shared<SueBehaviorFSM>(_character)){
}

SueNonfrightenedState::~SueNonfrightenedState(){
}

void SueNonfrightenedState::onEnter(const GameState&){
	std::dynamic_pointer_cast<Ghost>(character)->revert(); 
}

Move SueNonfrightenedState::onUpdate(const GameState& gs){
	return subFsm->update(gs);
}

SueHouseState::SueHouseState(std::shared_ptr<Character> _character):
	FSMState(_character){
}

SueHouseState::~SueHouseState(){
}

//Mientras esta en casa se queda quieto
Move SueHouseState::onUpdate(const GameState&){
	return PASS;
}


SueStateMachine::SueStateMachine(std::shared_ptr<Character> _character):
	FiniteStateMachine(_character){

	auto house = std::make_shared<SueHouseState>(character);                
	auto nonfrightened = std::make_shared<SueNonfrightenedState>(character); 
	auto frightened = std::make_shared<FrightenedState>(character);            

	//Sale de casa cuando pacman come 1/3 de las pastillas del nivel 
	house->addTransition(std::make_shared<PillsEatenTransition>(nonfrightened, 1.0f/3.0f));

	nonfrightened->addTransition(std::make_shared<EdibleTransition>(character, frightened));
	frightened->addTransition(std::make_shared<NotEdibleTransition>(character, nonfrightened));

	states.push_back(house);
	states.push_back(nonfrightened);
	states.push_back(frightened);

	initialState = house; //empieza en casa
	activeState = initialState;
}

SueStateMachine::~SueStateMachine(){
}

Move SueStateMachine::update(const GameState& gs){
	auto t = activeState->getActiveTransition(gs);
	if(t!=nullptr){
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}
