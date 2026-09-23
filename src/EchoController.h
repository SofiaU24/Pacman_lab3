#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include <deque>
#include <random>

constexpr size_t ECHO_DELAY = 8;
constexpr int ECHO_NEAR = 6;
constexpr int ECHO_START_SLOT = 0;

struct EchoBlackboard {
	const GameState* gs = nullptr;
	std::shared_ptr<Character> character;
	Move outMove = PASS;
	std::deque<int> history;
};

class EchoController: public Controller {
	std::shared_ptr<EchoBlackboard> bb;
	std::shared_ptr<Composite> root;
	size_t lastPills;
public:
	EchoController(std::shared_ptr<Character> character);
	virtual ~EchoController();
	virtual Move getMove(const GameState& game) override;
};

class EchoIsFrightened : public Behavior {
	std::shared_ptr<EchoBlackboard> bb;
public:
	EchoIsFrightened(std::shared_ptr<EchoBlackboard> bb);
	virtual Status update() override;
};

class EchoFrightened : public Behavior {
	std::shared_ptr<EchoBlackboard> bb;
	std::mt19937 e;
public:
	EchoFrightened(std::shared_ptr<EchoBlackboard> bb);
	virtual Status update() override;
};

class EchoPacmanNear : public Behavior {
	std::shared_ptr<EchoBlackboard> bb;
	bool wantNear;
public:
	EchoPacmanNear(std::shared_ptr<EchoBlackboard> bb, bool wantNear);
	virtual Status update() override;
};

class EchoScatter : public Behavior {
	std::shared_ptr<EchoBlackboard> bb;
public:
	EchoScatter(std::shared_ptr<EchoBlackboard> bb);
	virtual Status update() override;
};

class EchoMimic : public Behavior {
	std::shared_ptr<EchoBlackboard> bb;
public:
	EchoMimic(std::shared_ptr<EchoBlackboard> bb);
	virtual Status update() override;
};