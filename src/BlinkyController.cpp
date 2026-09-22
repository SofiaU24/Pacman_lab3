/*#include "BlinkyController.h"


BlinkyController::BlinkyController(std::shared_ptr<Character> character):
	Controller(character){
}

BlinkyController::~BlinkyController() {

}

Move
BlinkyController::getMove(const GameState& game){
	return PASS;	

	
}*/
//Tarea 1
#include "BlinkyController.h"

static const int ELROY_PILL_THRESHOLD = 30; // pildoras restantes -> activa Cruise Elroy
static const int CYCLE_SECONDS = 27;        // 20s Chase + 7s Scatter
static const int SCATTER_SECONDS = 7;

//moverse hacia el objeto
namespace {
std::vector<Move> legalMoves(const GameState& gs, const std::shared_ptr<Character>& c){
	int pos = c->getPos();
	if(c->getDirection()==PASS) return gs.getMaze().getPossibleMoves(pos);
	return gs.getMaze().getGhostLegalMoves(pos, c->getDirection());
}

Move bestMoveTowards(const GameState& gs, const std::shared_ptr<Character>& c, std::pair<int,int> target){
	int pos = c->getPos();
	Move best = PASS; float minDist = -1;
	for(Move m: legalMoves(gs, c)){
		if(m==PASS) continue;
		int vecino = gs.getMaze().getNeighbour(pos, m);
		if(vecino<0) continue;
		float dist = euclid2(gs.getMaze().getNodePos(vecino), target);
		if(minDist==-1 || dist<minDist){ minDist=dist; best=m; }
	}
	return best;
}
}

BlinkyController::BlinkyController(std::shared_ptr<Character> character):
	Controller(character), fsm(std::make_shared<BlinkyStateMachine>(character)){
}
BlinkyController::~BlinkyController(){}

Move BlinkyController::getMove(const GameState& game){
	return fsm->update(game);
}

BlinkyCycleTransition::BlinkyCycleTransition(std::chrono::steady_clock::time_point* start,
		int cycleSeconds, int scatterSeconds, bool wantsScatter, std::shared_ptr<FSMState> _next):
	cycleStart(start), cycleSeconds(cycleSeconds), scatterSeconds(scatterSeconds),
	wantsScatter(wantsScatter), next(_next){
}

bool BlinkyCycleTransition::isValid(const GameState&){
	auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(
			std::chrono::steady_clock::now() - *cycleStart).count();
	bool inScatterWindow = (elapsed % cycleSeconds) < scatterSeconds;
	return wantsScatter ? inScatterWindow : !inScatterWindow;
}

std::shared_ptr<FSMState> BlinkyCycleTransition::getNextState(){ return next; }

BlinkyEdibleTransition::BlinkyEdibleTransition(std::shared_ptr<Character> _character, bool wantsEdible, std::shared_ptr<FSMState> _next):
	character(_character), wantsEdible(wantsEdible), next(_next){
}

bool BlinkyEdibleTransition::isValid(const GameState&){
	auto ghost = std::dynamic_pointer_cast<Ghost>(character);
	bool edible = ghost && ghost->isEdible();
	return wantsEdible ? edible : !edible; // segun el sentido pedido en el constructor
}

std::shared_ptr<FSMState> BlinkyEdibleTransition::getNextState(){ return next; }

BlinkyChaseState::BlinkyChaseState(std::shared_ptr<Character> _character): FSMState(_character){}

void BlinkyChaseState::onEnter(const GameState&){
	std::dynamic_pointer_cast<Ghost>(character)->revert(); // cambio de direccion (diagrama)
}

Move BlinkyChaseState::onUpdate(const GameState& gs){
	// persigue la posicion directa de pacman
	return bestMoveTowards(gs, character, gs.getMaze().getNodePos(gs.getPacmanPos()));
}

BlinkyScatterState::BlinkyScatterState(std::shared_ptr<Character> _character):
	FSMState(_character), homeNode(_character->getPos()){
}

void BlinkyScatterState::onEnter(const GameState&){
	std::dynamic_pointer_cast<Ghost>(character)->revert();
}

Move BlinkyScatterState::onUpdate(const GameState& gs){
	// Cruise Elroy: va directo por Pac-Man en vez de a su esquina
	int pillsLeft = (int)gs.getMaze().getPillPositions().size() + (int)gs.getMaze().getPowerPillPositions().size();
	auto target = (pillsLeft <= ELROY_PILL_THRESHOLD)
		? gs.getMaze().getNodePos(gs.getPacmanPos())
		: gs.getMaze().getNodePos(homeNode);//si no cumple cantidad de pildoras, va a su esquina
	return bestMoveTowards(gs, character, target);
}

BlinkyFrightenedState::BlinkyFrightenedState(std::shared_ptr<Character> _character):
	FSMState(_character), randNumber(std::random_device{}()){
}

Move BlinkyFrightenedState::onUpdate(const GameState& gs){
	auto moves = legalMoves(gs, character);
	return moves[randNumber() % moves.size()]; //huye al azar
}

BlinkyStateMachine::BlinkyStateMachine(std::shared_ptr<Character> _character):
	FiniteStateMachine(_character), cycleStart(std::chrono::steady_clock::now()){

	auto chase      = std::make_shared<BlinkyChaseState>(character);
	auto scatter    = std::make_shared<BlinkyScatterState>(character);
	auto frightened = std::make_shared<BlinkyFrightenedState>(character);

	chase->addTransition(std::make_shared<BlinkyEdibleTransition>(character, true, frightened));
	chase->addTransition(std::make_shared<BlinkyCycleTransition>(&cycleStart, CYCLE_SECONDS, SCATTER_SECONDS, true, scatter));

	scatter->addTransition(std::make_shared<BlinkyEdibleTransition>(character, true, frightened));
	scatter->addTransition(std::make_shared<BlinkyCycleTransition>(&cycleStart, CYCLE_SECONDS, SCATTER_SECONDS, false, chase));

	frightened->addTransition(std::make_shared<BlinkyEdibleTransition>(character, false, chase));

	states.push_back(chase);
	states.push_back(scatter);
	states.push_back(frightened);

	initialState = scatter;
	activeState = initialState;
}

Move BlinkyStateMachine::update(const GameState& gs){
	auto t = activeState->getActiveTransition(gs);
	if(t != nullptr){
		activeState->onExit(gs);
		t->onTransition(gs);
		activeState = t->getNextState();
		activeState->onEnter(gs);
	}
	return activeState->onUpdate(gs);
}
