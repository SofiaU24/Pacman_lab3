#include "Controller.h"
#include "FSM.h"
#include "Ghost.h"
#include <chrono>
#include <random>

class GhostStateMachine;

class FSMGhostController: public Controller {
	std::shared_ptr<GhostStateMachine> fsm;
public:
	FSMGhostController(std::shared_ptr<Character> character);
	virtual ~FSMGhostController();
	virtual Move getMove(const GameState& game) override;
};


class TimedTransition: public FSMTransition {
	std::chrono::steady_clock::time_point start;
	std::chrono::seconds duration;
	std::shared_ptr<FSMState> next;
public:
	TimedTransition(std::shared_ptr<FSMState> _next, int seconds);
	void reset();
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class GhostChaseState: public FSMState {
public:
	GhostChaseState(std::shared_ptr<Character> _character);
	void onEnter(const GameState& gs) override; // reinicia el reloj 
	Move onUpdate(const GameState& gs) override;
	~GhostChaseState();
};

class GhostScatterState: public FSMState {
	int homeNode; // nodo origen de stcatter
public:
	GhostScatterState(std::shared_ptr<Character> _character);
	void onEnter(const GameState& gs) override; // reinicia el reloj 
	Move onUpdate(const GameState& gs) override;
	~GhostScatterState();
};

class ChaseScatterFSM: public FiniteStateMachine {
public:
	ChaseScatterFSM(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~ChaseScatterFSM();
};


class NonfrightenedState: public FSMState {
	std::shared_ptr<ChaseScatterFSM> subFsm;
public:
	NonfrightenedState(std::shared_ptr<Character> _character);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	~NonfrightenedState();
};


class FrightenedState: public FSMState {
	std::mt19937 e;
public:
	FrightenedState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
	~FrightenedState();
};


class EdibleTransition: public FSMTransition {
	std::shared_ptr<Character> character;
	std::shared_ptr<FSMState> next;
public:
	EdibleTransition(std::shared_ptr<Character> _character, std::shared_ptr<FSMState> _next);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class NotEdibleTransition: public FSMTransition {
	std::shared_ptr<Character> character;
	std::shared_ptr<FSMState> next;
public:
	NotEdibleTransition(std::shared_ptr<Character> _character, std::shared_ptr<FSMState> _next);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};


class GhostStateMachine: public FiniteStateMachine {
public:
	GhostStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~GhostStateMachine();
};
