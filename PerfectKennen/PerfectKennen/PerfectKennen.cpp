#include "PluginSDK.h"

PluginSetup("PerfectKennen")

// Menus
IMenu* MainMenu;

IMenu* ComboSettings;
IMenu* ComboQSettings;
IMenuOption* ComboQ;
IMenu* ComboWSettings;
IMenuOption* ComboW;


IMenu* MixedSettings;
IMenu* MixedQSettings;
IMenuOption* MixedQ;
IMenu* MixedWSettings;
IMenuOption* MixedW;
IMenuOption* SupportGenerateStacks;


IMenu* LaneClearSettings;
IMenu* LaneClearQSettings;
IMenuOption* LaneClearQ;

IMenu* MiscKennen;
IMenuOption* EOnTargeted;



IMenu* DrawSettings;
IMenuOption* DrawQRange;
IMenuOption* DrawWRange;


ISpell2* Q;
ISpell2* W;
ISpell2* E;


// Useful units
IUnit* myHero;
std::vector<IUnit*> minions;
std::vector<IUnit*> myTeam;
std::vector<IUnit*> enemyTeam;

// Useful pointers
IGame* game;

float timeSinceLastAuto;


void Menu()
{
	//Main Menu Setup
	MainMenu = GPluginSDK->AddMenu("Perfect Kennen");
	ComboSettings = MainMenu->AddMenu("Combo Settings");
	MixedSettings = MainMenu->AddMenu("Mixed Settings");
	MiscKennen = MainMenu->AddMenu("MiscKennen");
	LaneClearSettings = MainMenu->AddMenu("Lane Clear Settings");
	DrawSettings = MainMenu->AddMenu("Drawings");

	//Combo Menu Setup
	ComboQSettings = ComboSettings->AddMenu("Combo Q");
	ComboWSettings = ComboSettings->AddMenu("Combo W");

	//Combo Q Option Setup
	ComboQ = ComboQSettings->CheckBox("Enabled", true);
	//Combo W Option Setup
	ComboW = ComboWSettings->CheckBox("Enabled", true);
	SupportGenerateStacks = ComboWSettings->CheckBox("Support Mode / Auto Get Stacks", false);



	//Mixed Menu Setup
	MixedQSettings = MixedSettings->AddMenu("Mixed Q");
	MixedWSettings = MixedSettings->AddMenu("Mixed W");

	//Mixed Q Option Setup
	MixedQ = MixedQSettings->CheckBox("Enabled", true);
	//Mixed W Option Setup
	MixedW = MixedWSettings->CheckBox("Enabled", true);


	//Lane Clear Menu Setup
	LaneClearQSettings = LaneClearSettings->AddMenu("Lane Clear Q");

	//Lane Clear Q Option Setup
	LaneClearQ = LaneClearQSettings->CheckBox("Use Q", true);

	//Draw Menu Setup
	DrawQRange = DrawSettings->CheckBox("Draw Q Range", true);
	DrawWRange = DrawSettings->CheckBox("Draw W Range", true);
	EOnTargeted = MiscKennen->CheckBox("Use E when Targeted by a spell", true);

}


/**
Stolen from another script
returns the amount of enemies in range from source
used to determine wether kennen should auto attack minions or champions.
*/
int EnemiesInRange(IUnit* Source, float range)
{
	auto Targets = GEntityList->GetAllHeros(false, true);
	auto enemiesInRange = 0;

	for (auto target : Targets)
	{
		if (target != nullptr)
		{
			auto flDistance = (target->GetPosition() - Source->GetPosition()).Length();
			if (flDistance < range)
			{
				enemiesInRange++;
			}
		}
	}
	return enemiesInRange;
}

void CastQ() {
	if (Q->IsReady()) {

		auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

		if (myHero->IsValidTarget(target, Q->Range())) {
			Q->CastOnTarget(target, kHitChanceMedium);
		}
	}
}

void CastW() {
	auto target = GTargetSelector->GetFocusedTarget() != nullptr ? GTargetSelector->GetFocusedTarget() :
		GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range());

	if (target != nullptr && target->IsHero() && myHero->IsValidTarget(target, 749)) {

		//std::vector<void*> buffs;
		//target->GetAllBuffsData(buffs);

		//for (auto buff : buffs) {
		//	GGame->PrintChat(GBuffData->GetBuffName(buff));
		//}

		if (target->HasBuff("kennenmarkofstorm")) {

			if (W->IsReady()) {
				//GGame->PrintChat("w ready");
				W->CastOnPlayer();
			}
		}
	}
}
/*
Basic combo logic.

Will try to Q enemy if in range.
Will Try to W Enemy if they have the debuff

If enabled will automatically try to get stacks for W./ 
**/
void Combo()
{
	// Is Combo Q Enabled?
	if (ComboQ->Enabled())
	{
		CastQ();
	}

	// Is Combo W Enabled?
	if (ComboW->Enabled())
	{
		
		CastW();


		//	Next is the logic for getting up stacks for support Kennen
		//	Basically it checks and sees if kennen next auto attack will proc his damage, if not it 
		//	will attack minions with >50% health untill he gets it.

		if (SupportGenerateStacks->Enabled()) {
			bool nextAuto = myHero->HasBuff("kennendoublestrikelive");
			auto target = GTargetSelector->FindTarget(QuickestKill, PhysicalDamage, 550);

			if (EnemiesInRange(myHero, 549) <= 0) {
				if (!GOrbwalking->CanMove()) {
					GOrbwalking->SetMovementAllowed(true);
				}
				if (!nextAuto) {
					if (GGame->Time() > (timeSinceLastAuto + .30)) {
						for (auto minion : GEntityList->GetAllMinions(false, true, false)) {
							if (minion != nullptr && myHero->IsValidTarget(minion, myHero->GetRealAutoAttackRange(minion))) {

								auto healthPercent = (minion->GetHealth() / minion->GetMaxHealth());
								if (healthPercent > .5) {

									timeSinceLastAuto = GGame->Time();

									GOrbwalking->DisableNextAttack();
									GOrbwalking->SetMovementAllowed(false);
									GGame->IssueOrder(myHero, kAttackUnit, minion);
									GGame->IssueOrder(myHero, kStop, myHero);
									break;
								}
							}
						}
					}
				}
			}
			else {
				if (!GOrbwalking->CanMove()) {
					GOrbwalking->SetMovementAllowed(true);
				}
			}
		}
	}

}




void Mixed()
{
	if (MixedQ->Enabled())
	{
		CastQ();
		Q->AttackMinions(1);
	}
	if (MixedW->Enabled())
	{
		CastW();
	}

}

void LaneClear()
{
	if (LaneClearQ->Enabled())
	{
		Q->AttackMinions(1);
	}
}

void Auto()
{

}

PLUGIN_EVENT(void) OnGameUpdate()
{
	myHero = GEntityList->Player();
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
		Combo();
	else if (GOrbwalking->GetOrbwalkingMode() == kModeMixed)
	{
		Mixed();
	}
	else if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear)
	{
		LaneClear();
	}
	Auto();
}

void DrawRanges()
{
	if (DrawQRange->Enabled())
	{
		GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range());
	}
	if (DrawWRange->Enabled())
	{
		GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), W->Range());
	}
}

PLUGIN_EVENT(void) OnRender()
{
	DrawRanges();
}

void InitSpells()
{
	//Q
	//	Initialize the Q, set the q to ignore minions and yasou wall
	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, false, static_cast<eCollisionFlags>(kCollidesWithMinions | kCollidesWithYasuoWall));
	//	Set the range to 1050
	Q->SetOverrideRange(1050);
	Q->SetOverrideSpeed(1650);
	Q->SetOverrideRadius(60);
	Q->SetOverrideDelay(0.25);



	//W
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, true, true, kCollidesWithNothing);
	W->SetOverrideRange(750);


	E = GPluginSDK->CreateSpell2(kSlotE, kCircleCast, true, true, kCollidesWithNothing);

}


PLUGIN_EVENT(void) OnSpellCast(CastedSpell const& spell)
{
	if (EOnTargeted->Enabled()) {

		//GGame->PrintChat("Spell Casted");
		if (spell.Target_ == myHero && spell.Caster_->IsHero() && spell.Caster_->IsEnemy(myHero)) {

			auto data = spell.Data_;
			auto target = GSpellData->GetTarget(data);
			auto number = GSpellData->NumberOfTargets(data);
			std::string spellName = spell.Name_;

			if (spellName.find("BasicAttack") != std::string::npos) {
				return;
			}

			char array[10];
			sprintf_s(array, "%d", number);

			GGame->PrintChat("NUMBER OF TARGETS");
			GGame->PrintChat(array);

			GGame->PrintChat(target->ChampionName());
			GGame->PrintChat("Was targeted by ");
			GGame->PrintChat(spell.Name_);


			E->CastOnPlayer();
		}
	}
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();
	timeSinceLastAuto = 0;

	InitSpells();
	myHero = GEntityList->Player();
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);
	GEventManager->AddEventHandler(kEventOnSpellCast, OnSpellCast);
}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
	GEventManager->RemoveEventHandler(kEventOnSpellCast, OnSpellCast);

}