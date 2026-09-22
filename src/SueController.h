/*#pragma once

#include "Controller.h"


class SueController: public Controller {

public:
	SueController(std::shared_ptr<Character> character);
	virtual ~SueController();
	virtual Move getMove(const GameState& game)override;
};*/
//Tarea 1
#pragma once

#include "Controller.h"          
#include "FSM.h"                
#include "Ghost.h"                
#include "FSMGhostController.h"  


class SueStateMachine;

class SueController: public Controller {
	std::shared_ptr<SueStateMachine> fsm;
public:
	SueController(std::shared_ptr<Character> character);
	virtual ~SueController();
	virtual Move getMove(const GameState& game) override;
};


// Transicion por distancia
class DistanceTransition: public FSMTransition {
	std::shared_ptr<Character> character;
	std::shared_ptr<FSMState> next;
	float thresholdSq;
	bool whenCloser;
public:
	DistanceTransition(std::shared_ptr<Character> _character, std::shared_ptr<FSMState> _next, float tilesThreshold, bool _whenCloser);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};


class SueBehaviorFSM: public FiniteStateMachine {
public:
	SueBehaviorFSM(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~SueBehaviorFSM();
};


class SueNonfrightenedState: public FSMState {
	std::shared_ptr<SueBehaviorFSM> subFsm;
public:
	SueNonfrightenedState(std::shared_ptr<Character> _character);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	~SueNonfrightenedState();
};


//Sue se queda quieto hasta que Pacman come 1/3 de las pastillas 
class SueHouseState: public FSMState {
public:
	SueHouseState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override; //siempre retorna PASS
	~SueHouseState();
};


class SueStateMachine: public FiniteStateMachine {
public:
	SueStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~SueStateMachine();
};

