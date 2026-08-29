
#include "Controller.h"


class PacmanDTController: public Controller {

public:
	PacmanDTController(std::shared_ptr<Character> character);
	virtual ~PacmanDTController();
	virtual Move getMove(const GameState& game)override;
};

