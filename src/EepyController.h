#pragma once

#include "Controller.h"
#include "FSM.h"
#include "FSMGhostController.h"

class EepyStateMachine;

class EepyController: public Controller {
	std::shared_ptr<EepyStateMachine> fsm;
public:
	EepyController(std::shared_ptr<Character> character);
	virtual ~EepyController();
	virtual Move getMove(const GameState& game) override;
};


class TiredState: public FSMState {
	int frozenTarget;
public:
	TiredState(std::shared_ptr<Character> _character);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	~TiredState();
};


class TiredCycleFSM: public FiniteStateMachine {
public:
	TiredCycleFSM(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~TiredCycleFSM();
};


class TiredNonfrightenedState: public FSMState {
	std::shared_ptr<TiredCycleFSM> subFsm;
public:
	TiredNonfrightenedState(std::shared_ptr<Character> _character);
	void onEnter(const GameState& gs) override;
	Move onUpdate(const GameState& gs) override;
	~TiredNonfrightenedState();
};


class EepyStateMachine: public FiniteStateMachine {
public:
	EepyStateMachine(std::shared_ptr<Character> _character);
	Move update(const GameState& gs) override;
	~EepyStateMachine();
};