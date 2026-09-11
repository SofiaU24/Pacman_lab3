/*#pragma once

#include "Controller.h"


class PinkyController: public Controller {

public:
	PinkyController(std::shared_ptr<Character> character);
	virtual ~PinkyController();
	virtual Move getMove(const GameState& game)override;
};*/
//Laboratorio 5
#pragma once

#include "Controller.h"
#include "BehaviorTree.h"
#include <chrono>
#include <random>

// estructura de datos compartidos entre los nodos del árbol de Pinky
struct PinkyBlackboard {
	const GameState* gs = nullptr; 
	std::shared_ptr<Character> character;
	Move outMove = PASS;
};

class PinkyController: public Controller {
	std::shared_ptr<PinkyBlackboard> bb;
	std::shared_ptr<Composite> root; //selector
public:
	PinkyController(std::shared_ptr<Character> character);
	virtual ~PinkyController();
	virtual Move getMove(const GameState& game) override;
};

//condicion: frigthened? 
class PinkyIsFrightened : public Behavior {
	std::shared_ptr<PinkyBlackboard> bb;
public:
	PinkyIsFrightened(std::shared_ptr<PinkyBlackboard> bb);
	virtual Status update() override;
};

//accion: huir en direccion valida aleatoria
class PinkyFrightened : public Behavior {
	std::shared_ptr<PinkyBlackboard> bb;
	std::mt19937 e; //objeto generador de numeros aleatorios
public:
	PinkyFrightened(std::shared_ptr<PinkyBlackboard> bb);
	virtual Status update() override;
};

// condicoin: dentro de la ventana de Scatter del ciclo 20s chase/ 7s scatter?
class PinkyScatterWindow : public Behavior {
	std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
public:
	PinkyScatterWindow();
	virtual Status update() override;
};

//accio: volver a su esquina de origen
class PinkyScatter : public Behavior {
	std::shared_ptr<PinkyBlackboard> bb;
	int homeNode; //para volver a su origen "nodo casa"
public:
	PinkyScatter(std::shared_ptr<PinkyBlackboard> bb);
	virtual Status update() override;
};

//accion: perseguir (4 nodos por delante de Pacman en la direccion que se mueve)
class PinkyChase : public Behavior {
	std::shared_ptr<PinkyBlackboard> bb;
public:
	PinkyChase(std::shared_ptr<PinkyBlackboard> bb);
	virtual Status update() override;
};


