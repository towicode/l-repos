#include "PluginSDK.h"
#include <algorithm>    // std::sort
#include "Header.h"

PluginSetup("PerfectFeeder");


//	Global for my hero
IUnit* myHero;
boolean wasAlive = false;
float gameTime = 0;

enum Lane { Top, Mid, Bot };
const Lane lastLane = Bot;
Lane current = Lane(0);


bool reachedLane = false;

Vec3 mid = Vec3(7232.f, 53.f, 7378.f);
Vec3 top = Vec3(2082.f, 52.f, 12450.f);
Vec3 bot = Vec3(12434.f, 51.f, 2380.f);
Vec3 lanes[] = { top,mid,bot };
//Vec3 nexus = GEntityList->GetEnemyNexus()->GetPosition();



//	returns wether or not my champion is under turret
bool myChampUnderTurret() {
	return GUtility->IsPositionUnderTurret(myHero->GetPosition(), false, true);
}

void Menu() {

}

//	defines what we should do on death, in this case switch lanes
void OnDeath() {

	reachedLane = false;
	//	On death switch lanes
	if (current != lastLane)
		current = Lane(current + 1);
	else
		current = Lane(0);

}


//	walks our champ to the vec 3 location
void walkTo(Vec3 location) {
	GGame->IssueOrder(myHero, kMoveTo, location);

}


//	returns wether or not the champ is currently walk
//	also does a check to see if we need to stop walking
//	this is placed here since it's called the most.
bool isWalking() {
	if (myChampUnderTurret()) {
		GGame->IssueOrder(myHero, kStop, myHero);
	}
	return myHero->GetWaypointList().size() > 1;
}


//	feeds
void Feed() {


	if (isWalking()) { return; };

	if (reachedLane || (myHero->GetPosition() - lanes[current]).Length() < 1500) {
		reachedLane = true;

		auto nexus = GEntityList->GetEnemyNexus();
		auto point = nexus->GetPosition();

		walkTo(point);
	}
	else {
		walkTo(lanes[current]);
	}

}

void Auto() {

	//Only want to do actions every 3 seconds
	if (GGame->Time() <= (gameTime + 3)) { return; };

	//Update game time
	gameTime = GGame->Time();

	// avoid unloads by allways null checking
	auto temp = GEntityList->Player();
	if (temp == nullptr) { return; };

	myHero = temp;

	//	check if we need to trigger on dead
	if (myHero->IsDead() && wasAlive) {
		wasAlive = false;
		OnDeath();
		return;
	}

	//	check if we need to continue feeding.
	if (!myHero->IsDead()) {
		wasAlive = true;
		Feed();
	}

}


PLUGIN_EVENT(void) OnGameUpdate()
{
	Auto();

}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();

	myHero = GEntityList->Player();
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
}

PLUGIN_API void OnUnload()
{
	//MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
}