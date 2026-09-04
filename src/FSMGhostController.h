#include "Controller.h"
#include "FSM.h"
#include <random>
#include <chrono>
auto start = std::chrono::high_resolution_clock::now();
class GhostStateMachine;

class FSMGhostController: public Controller{
    std::shared_ptr<GhostStateMachine> fsm;
    public:

}

auto end = std::chrono::high_resolution_clock::now();