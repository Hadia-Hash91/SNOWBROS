#include<iostream>
#include"Game.h"
#include"Player.h"
int main()
{
	Game game;
	game.run();
	Player land;
	land.land(500.f);
	return 0;
}