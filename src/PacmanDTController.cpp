#include "PacmanDTController.h"
#include "Ghost.h"
#include <limits>

PacmanDTController::PacmanDTController(std::shared_ptr<Character> character):
	Controller(character){
}

PacmanDTController::~PacmanDTController(){}

Move
PacmanDTController::getMove(const GameState& game){

	if(character->getDirection()==PASS && game.getMaze().getPacmanStart()[0]){
		return RIGHT;
    }
	int pacmanNode = game.getPacmanPos();
	auto pacmanCoords = game.getMaze().getNodePos(pacmanNode);
    
    int ghostNode = Ghost->getPos();
    
    Ghost *ghost = dynamic_cast<Ghost*>(character.get());
    //movimientos posibles
    std::vector<Move> moves;
    for(Move m : {UP, RIGHT, DOWN, LEFT})
    {
        if(game.getMaze().isLegal(pacmanNode, m)) move.push_back(m);
    }

    if(moves.empty) move.push_back(PASS);

    bool fantasmaComestible = false;
    for(int i = 0; i < 4; i++)
    {
        if(game.isGhostEdible(i)){
            fantasmaComestible = true;
            break;
        }
    }

    if(fantasmaComestible){
        int minDist=10000000;
        Move minMove=character->getDirection();
        std::vector<Move> moves=game.getMaze().isLegal(character->getPos(),character->getDirection());
		for(Move m:moves){
			int vecino = game.getMaze().getNeighbour(pacmanNode,m);
			
			if(vecino<0)
				continue;

			auto vecinoCoords = game.getMaze().getNodePos(vecino);
			vecinoCoords.first-=ghostCoords.first;
			vecinoCoords.second-=ghostCoords.second;
			int sqDist=vecinoCoords.first*vecinoCoords.first+vecinoCoords.second*vecinoCoords.second;
			if(sqDist<minDist){
				minDist=sqDist;
				minMove=m;
			}
		}
		return minMove;
    }else{
        int maxDist=-1;
		Move maxMove=character->getDirection();
		std::vector<Move> moves=game.getMaze().isLegal(character->getPos(),character->getDirection());
		for(Move m:moves){
			int vecino = game.getMaze().getNeighbour(pacmanNode,m);
			if(vecino<0)continue;
			auto vecinoCoords = game.getMaze().getNodePos(vecino);
			vecinoCoords.first-=ghostCoords.first;
			vecinoCoords.second-=ghostCoords.second;
			int sqDist=vecinoCoords.first*vecinoCoords.first+vecinoCoords.second*vecinoCoords.second;
			if(sqDist>maxDist){
				maxDist=sqDist;
				maxMove=m;
			}
		}
		return maxMove;
    
    }
	
}

