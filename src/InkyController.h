/*#pragma once

#include "Controller.h"


class InkyController: public Controller {

public:
	InkyController(std::shared_ptr<Character> character);
	virtual ~InkyController();
	virtual Move getMove(const GameState& game)override;
};*/
//Tarea 1
#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include <chrono>
#include <random>

// estructura de datos compartidos entre los nodos del arbol de Inky
struct InkyBlackboard {
	const GameState* gamestate = nullptr;
	std::shared_ptr<Character> character;
	Move outMove = PASS;
};

class InkyController: public Controller {
	std::shared_ptr<InkyBlackboard> blackboard;
	std::shared_ptr<Composite> root;
public:
	InkyController(std::shared_ptr<Character> character);
	virtual ~InkyController();
	virtual Move getMove(const GameState& game) override;
};

// ¿debe seguir esperando en casa? (true mientras no se hayan comido mas de 30 pastillas)
class InkyInHouse : public Behavior {
	std::shared_ptr<InkyBlackboard> blackboard;
	int totalPills; // se captura en la primera llamada a update
public:
	InkyInHouse(std::shared_ptr<InkyBlackboard> blackboard);
	virtual Status update() override;
};

//se queda quieto mientras espera en casa
class InkyWaitInHouse : public Behavior {
	std::shared_ptr<InkyBlackboard> blackboard;
public:
	InkyWaitInHouse(std::shared_ptr<InkyBlackboard> blackboard);
	virtual Status update() override;
};
//frightened?
class InkyIsFrightened : public Behavior {
	std::shared_ptr<InkyBlackboard> blackboard;
public:
	InkyIsFrightened(std::shared_ptr<InkyBlackboard> blackboard);
	virtual Status update() override;
};

//huir en direccion valida aleatoria
class InkyFrightened : public Behavior {
	std::shared_ptr<InkyBlackboard> blackboard;
	std::mt19937 e;
public:
	InkyFrightened(std::shared_ptr<InkyBlackboard> blackboard);
	virtual Status update() override;
};

//dentro de la ventana de Scatter del ciclo 20s chase / 7s scatter?
class InkyScatterWindow : public Behavior {
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
public:
	InkyScatterWindow();
	virtual Status update() override;
};

class InkyScatter : public Behavior {
	std::shared_ptr<InkyBlackboard> blackboard;
	int homeNode;
public:
	InkyScatter(std::shared_ptr<InkyBlackboard> blackboard);
	virtual Status update() override;
};

// perseguir usando el vector desde Blinky (comportamiento propio de Inky)
class InkyChase : public Behavior {
	std::shared_ptr<InkyBlackboard> blackboard;
public:
	InkyChase(std::shared_ptr<InkyBlackboard> blackboard);
	virtual Status update() override;
};
