#include "PluginSDK.h"

PluginSetup("antiAFk")

// Menus
IMenu* MainMenu;

IMenu* JumpSettings;
IMenuOption* JumpKey;

ISpell2* jump;


boolean ButtonDown = false;
boolean doJump = false;
Vec3 location;

IInventoryItem *sight_stone;
IInventoryItem *red_stone;
IInventoryItem *pink_ward;
IInventoryItem *yellow_trinket;
IInventoryItem *t_knife;
IInventoryItem *t_knifeA;
IInventoryItem *t_knifeB;
IInventoryItem *t_knifeC;
IInventoryItem *t_knifeD;
float time;
float last_time = 0;


void Menu()
{
	//Main Menu Setup
	MainMenu = GPluginSDK->AddMenu("Perfect afki");
	JumpSettings = MainMenu->AddMenu("Ward Jump Settings");
	JumpKey = JumpSettings->AddKey("Tap to ward jump", 71);
}

//stolen from zFederal
static bool IsKeyDown(IMenuOption *menuOption)
{
	return GetAsyncKeyState(menuOption->GetInteger()) & 0x8000;
}


/*
Simple helper function to get the best item.
*/
IInventoryItem* GetAvaliableItem() {

	if (sight_stone->IsOwned() && sight_stone->IsReady()) {
		return sight_stone;
	}
	else if (t_knife->IsOwned() && t_knife->IsReady()) {
		return t_knife;
	}
	else if (t_knifeA->IsOwned() && t_knifeA->IsReady()) {
		return t_knifeA;
	}
	else if (t_knifeB->IsOwned() && t_knifeB->IsReady()) {
		return t_knifeB;
	}
	else if (t_knifeC->IsOwned() && t_knifeC->IsReady()) {
		return t_knifeC;
	}
	else if (t_knifeD->IsOwned() && t_knifeD->IsReady()) {
		return t_knifeD;
	}
	else if (red_stone->IsOwned() && red_stone->IsReady()) {
		return red_stone;
	}
	else if (yellow_trinket->IsOwned() && yellow_trinket->IsReady()) {
		return yellow_trinket;
	}

	else if (pink_ward->IsOwned() && pink_ward->IsReady()) {
		return pink_ward;
	}



}


/*
Checks and see if jump is ready, grabs the best item and then triggers the jump*/
void DoWardJump() {

	if (!jump->IsReady()) {
		return;
	}

	auto item = GetAvaliableItem();
	if (item == nullptr) {
		//GGame->PrintChat("No item()");
		return;
	}

	location = GGame->CursorPosition();
	item->CastOnPosition(location);
	doJump = true;
}


/*
Checks for updates on the key press.
Three cases here.
1. It's the first time the program has recognized a keypress
2. It's the second...n time the program has regonized the same keypress(so we do nothing)
3. The button has been raised, so set the boolean to reflect as such.
*/
void KeyPressEvent() {

	if (IsKeyDown(JumpKey) && ButtonDown == false) {
		//first key press
		ButtonDown = true;
		DoWardJump();

	}

	else if (IsKeyDown(JumpKey)) {
		// don't want to spam spells
		return;
	}
	else {
		ButtonDown = false;
	}

}


/*
Checks the object created
1. To see if we should do the jump.
2. To see if the source is even valid
3. To see if the object is near where we requested to ward jump
4. To see if the object is a ward.*/
PLUGIN_EVENT(void) OnCreateObject(IUnit* Source)
{


	if (!doJump) {
		return;
	}

	if (Source == nullptr) {
		return;
	}

	//GGame->PrintChat("Created Object1");
	float flDistance = (Source->GetPosition() - location).Length();
	//char array[30];
	//sprintf_s(array, "%f", flDistance);
	//GGame->PrintChat(array);

	if (flDistance < 100) {

		std::string itemName = Source->GetObjectName();
		if (itemName.find("Ward") != std::string::npos || itemName.find("ward") != std::string::npos) {
			jump->CastOnUnit(Source);
			doJump = false;
		}

	}

}

/*
This just calls the KeyPressEvent function, which checks to see if there is an update to the keypress.
*/
PLUGIN_EVENT(void) OnGameUpdate()
{
	KeyPressEvent();
	time = GGame->Time();

	if (time > last_time + 30) {
		last_time = time;
		GGame->SendPing(kPingDanger, GEntityList->Player()->GetPosition());
		GGame->Say("/laugh");
		GGame->IssueOrder(GEntityList->Player(), kMoveTo, GEntityList->Player()->GetPosition());
		GGame->PrintChat("anti afk");

	}
	
	
}

/*
This function loads the current champion aswell as the warding items.

*/
boolean InitSpells()
{

	auto champion = GEntityList->Player();

	if (champion == nullptr) {
		return false;
	}

	auto name = champion->ChampionName();
	//GGame->PrintChat(name);


	if (strcmp(name, "LeeSin") == 0) {
		GGame->PrintChat("Loaded Lee Jump!");

		jump = GPluginSDK->CreateSpell2(kSlotW, kTargetCast, false, false, kCollidesWithNothing);

	}
	else if ((strcmp(name, "Jax")) == 0) {
		GGame->PrintChat("Loaded Jax Jump!");
		jump = GPluginSDK->CreateSpell2(kSlotQ, kTargetCast, false, false, kCollidesWithNothing);
	}

	sight_stone = GPluginSDK->CreateItemForId(2049, 625);
	red_stone = GPluginSDK->CreateItemForId(2045, 625);
	yellow_trinket = GPluginSDK->CreateItemForId(3340, 625);
	pink_ward = GPluginSDK->CreateItemForId(2055, 625);
	t_knife = GPluginSDK->CreateItemForId(3711, 625);
	t_knifeA = GPluginSDK->CreateItemForId(1408, 625);
	t_knifeB = GPluginSDK->CreateItemForId(1409, 625);
	t_knifeC = GPluginSDK->CreateItemForId(1410, 625);
	t_knifeD = GPluginSDK->CreateItemForId(1418, 625);


	return true;



}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();
	if (!InitSpells()) {
		return;
	}
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnCreateObject, OnCreateObject);

}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnCreateObject, OnCreateObject);

}