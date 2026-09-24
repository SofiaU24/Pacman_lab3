#pragma once

#include "Controller.h"
#include "BehaviorTree.h"

constexpr int PACMAN_DANGER_NODES = 8;

struct PacmanBlackboard {
	const GameState* gs = nullptr;
	std::shared_ptr<Character> character;
	Move outMove = PASS;
};

class PacmanBTController: public Controller {
	std::shared_ptr<PacmanBlackboard> bb;
	std::shared_ptr<Composite> root;
public:
	PacmanBTController(std::shared_ptr<Character> character);
	virtual ~PacmanBTController();
	virtual Move getMove(const GameState& game) override;
};

class PacmanDanger : public Behavior {
	std::shared_ptr<PacmanBlackboard> bb;
public:
	PacmanDanger(std::shared_ptr<PacmanBlackboard> bb);
	virtual Status update() override;
};

class PacmanFlee : public Behavior {
	std::shared_ptr<PacmanBlackboard> bb;
public:
	PacmanFlee(std::shared_ptr<PacmanBlackboard> bb);
	virtual Status update() override;
};

class PacmanHasEdible : public Behavior {
	std::shared_ptr<PacmanBlackboard> bb;
public:
	PacmanHasEdible(std::shared_ptr<PacmanBlackboard> bb);
	virtual Status update() override;
};

class PacmanChase : public Behavior {
	std::shared_ptr<PacmanBlackboard> bb;
public:
	PacmanChase(std::shared_ptr<PacmanBlackboard> bb);
	virtual Status update() override;
};

class PacmanEatPills : public Behavior {
	std::shared_ptr<PacmanBlackboard> bb;
public:
	PacmanEatPills(std::shared_ptr<PacmanBlackboard> bb);
	virtual Status update() override;
};