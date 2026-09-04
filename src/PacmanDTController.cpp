#include "PacmanDTController.h"
#include <limits>

PacmanDTController::PacmanDTController(std::shared_ptr<Character> character):
	Controller(character){
}

PacmanDTController::~PacmanDTController(){}

Move
PacmanDTController::getMove(const GameState& game){

	if(character->getDirection()==PASS && game.getMaze().getPacmanStart()){
		return RIGHT;
    }
	int pacmanNode = game.getPacmanPos();
	auto pacmanCoords = game.getMaze().getNodePos(pacmanNode);

    //movimientos posibles
    std::vector<Move> moves;
    for(Move m : {UP, RIGHT, DOWN, LEFT})
    {
		if(game.getMaze().isLegal(pacmanNode, m)) moves.push_back(m);
    }
	
    if(moves.empty()) moves.push_back(PASS);
	
    bool fantasmaComestible = false;
	int ghostIndex = -1;

    for(int i = 0; i < 4; i++)
    {
		if(game.isGhostEdible(i)){
			fantasmaComestible = true;
			ghostIndex = i;
            break;
        }else{
		  ghostIndex = i;
		}
    }

	
    if(fantasmaComestible){
		int minDist=10000000;
		int ghostNode = game.getGhostsPos(ghostIndex);
		auto ghostCoords = game.getMaze().getNodePos(ghostNode);
        Move minMove=character->getDirection();
        std::vector<Move> moves=game.getMaze().getPossibleMoves(character->getPos());
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
    }else{//escapar
        int maxDist=-1;
		Move maxMove=character->getDirection();
		std::vector<Move> moves=game.getMaze().getPossibleMoves(character->getPos());
		for(Move m:moves){
			int vecino = game.getMaze().getNeighbour(pacmanNode,m);
			if(vecino<0)continue;
			//buscar distancia mas cercana de este enemigo
			int minDistGhost = 10000000;
			for(int i = 0;i < 4; i++){
				//calcular coordenadas de fantasma mas cercano para escapar
				int ghostNode = game.getGhostsPos(i);
				auto ghostCoords = game.getMaze().getNodePos(ghostNode);
				auto vecinoCoords = game.getMaze().getNodePos(vecino);
				vecinoCoords.first-=ghostCoords.first;
				vecinoCoords.second-=ghostCoords.second;
				int sqDist=vecinoCoords.first*vecinoCoords.first+vecinoCoords.second*vecinoCoords.second;

				if(sqDist>maxDist){
					maxDist=sqDist;
					maxMove=m;
				}
			}
		}
		return maxMove;
    
    }
	
}

