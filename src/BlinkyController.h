/*#pragma once

#include "Controller.h"


class BlinkyController: public Controller {

public:
	BlinkyController(std::shared_ptr<Character> character);
	virtual ~BlinkyController();
	virtual Move getMove(const GameState& game)override;
};*/
//Tarea 1
#pragma once

#include "Controller.h"
#include "FSM.h"
#include "Ghost.h"
#include <chrono>
#include <random>

class BlinkyStateMachine;

class BlinkyController: public Controller {
	std::shared_ptr<BlinkyStateMachine> fsm;
public:
	BlinkyController(std::shared_ptr<Character> character);
	virtual ~BlinkyController();
	virtual Move getMove(const GameState& game) override;
};

// Chase <-> Scatter con UN solo reloj fijo + modulo (igual que Pinky).
class BlinkyCycleTransition: public FSMTransition {
	std::chrono::steady_clock::time_point* cycleStart;
	int cycleSeconds, scatterSeconds;
	bool wantsScatter;
	std::shared_ptr<FSMState> next;
public:
	BlinkyCycleTransition(std::chrono::steady_clock::time_point* start, int cycleSeconds,
	                      int scatterSeconds, bool wantsScatter, std::shared_ptr<FSMState> _next);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

// Una sola clase sirve para ir HACIA Frightened y para volver: "wantsEdible" marca el sentido.
class BlinkyEdibleTransition: public FSMTransition {
	std::shared_ptr<Character> character;
	bool wantsEdible; // true: se activa si es comestible; false: si NO lo es
	std::shared_ptr<FSMState> next;
public:
	BlinkyEdibleTransition(std::shared_ptr<Character> _character, bool wantsEdible, std::shared_ptr<FSMState> _next);
	bool isValid(const GameState& gs) override;
	std::shared_ptr<FSMState> getNextState() override;
};

class BlinkyChaseState: public FSMState {
public:
	BlinkyChaseState(std::shared_ptr<Character> _character);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
};

class BlinkyScatterState: public FSMState {
	int homeNode;
public:
	BlinkyScatterState(std::shared_ptr<Character> _character);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
};

class BlinkyFrightenedState: public FSMState {
	std::mt19937 randNumber;
public:
	BlinkyFrightenedState(std::shared_ptr<Character> _character);
	Move onUpdate(const GameState& gs) override;
};

class BlinkyStateMachine: public FiniteStateMachine {
	std::chrono::steady_clock::time_point cycleStart; // se fija una sola vez
public:
	BlinkyStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
};


