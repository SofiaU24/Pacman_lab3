#include "PacmanDTController.h"
#include <limits>

PacmanDTController::PacmanDTController(std::shared_ptr<Character> character):
	Controller(character){
}

PacmanDTController::~PacmanDTController(){}

Move
PacmanDTController::getMove(const GameState& game){

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

	
    if(fantasmaComestible){ //ir al fantasma comestible
		int minDist=10000000;
		int ghostNode = game.getGhostsPos(ghostIndex);
		auto ghostCoords = game.getMaze().getNodePos(ghostNode);
        Move minMove=character->getDirection();
        std::vector<Move> moves;
		if(character->getDirection()==PASS){
			moves = game.getMaze().getPossibleMoves(character->getPos());
		}else{
			moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
		}
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
		std::vector<Move> moves;
		if(character->getDirection()==PASS){
			moves = game.getMaze().getPossibleMoves(character->getPos());
		}else{
			moves = game.getMaze().getGhostLegalMoves(character->getPos(), character->getDirection());
		}
		for(Move m:moves){
			int vecino = game.getMaze().getNeighbour(pacmanNode,m);
			if(vecino<0)continue;
			//calcular coordenadas de vecino fuera de for fantasmas para evitar usar un valor modificado en la siguiente iteracion
			auto vecinoCoords = game.getMaze().getNodePos(vecino);
			//buscar fantasma más cercano para este movimiento
			int minDist= std::numeric_limits<int>::max();
			for(int i = 0;i < 4; i++){
				//calcular coordenadas de fantasma mas cercano para escapar
				int ghostNode = game.getGhostsPos(i);
				auto ghostCoords = game.getMaze().getNodePos(ghostNode);
				vecinoCoords.first-=ghostCoords.first;
				vecinoCoords.second-=ghostCoords.second;
				int sqDist=vecinoCoords.first*vecinoCoords.first+vecinoCoords.second*vecinoCoords.second;

				if(sqDist<minDist){
					minDist=sqDist;
				}
			}
			//elegir el movimiento que aumente la distancia al fantasma más cercano
			if(minDist>maxDist){
				maxDist=minDist;
				maxMove=m;
			}
		}
		return maxMove;
    
    }
	
}

