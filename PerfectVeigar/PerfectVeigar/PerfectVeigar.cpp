#include "PluginSDK.h"
#include "Common.h"

PluginSetup("Perfect Veigar")

// Menus
IMenu* MainMenu;

IMenu* ComboSettings;
IMenu* ComboQSettings;
IMenuOption* ComboQ;
IMenu* ComboWSettings;
IMenuOption* ComboW;
IMenu* ComboESettings;
IMenuOption* ComboE;
IMenu* ComboRSettings;
IMenuOption* ComboR;

IMenu* MixedSettings;
IMenu* MixedQSettings;
IMenuOption* MixedQ;
IMenu* MixedWSettings;
IMenuOption* MixedW;
IMenu* MixedESettings;
IMenuOption* MixedE;

IMenu* LaneClearSettings;
IMenu* LaneClearQSettings;
IMenuOption* LaneClearQ;
IMenuOption* LaneClearQMinionsRequired;
IMenu* LaneClearWSettings;
IMenuOption* LaneClearW;
IMenuOption* LaneClearWMinionsRequired;


IMenu* DrawSettings;
IMenuOption* DrawQRange;
IMenuOption* DrawWRange;
IMenuOption* DrawERange;
IMenuOption* DrawRRange;



// Useful units
IUnit* myHero;
std::vector<IUnit*> minions;
std::vector<IUnit*> myTeam;
std::vector<IUnit*> enemyTeam;
bool require_stun = true;

// Useful pointers
IGame* game;


int q_hit_change = 5;

void Menu()
{
	//Main Menu Setup
	MainMenu = GPluginSDK->AddMenu("Perfect Veigar");
	ComboSettings = MainMenu->AddMenu("Combo Settings");
	MixedSettings = MainMenu->AddMenu("Mixed Settings");
	LaneClearSettings = MainMenu->AddMenu("Lane Clear Settings");
	DrawSettings = MainMenu->AddMenu("Drawings");

	//Combo Menu Setup
	ComboQSettings = ComboSettings->AddMenu("Combo Q");
	ComboWSettings = ComboSettings->AddMenu("Combo W");
	ComboESettings = ComboSettings->AddMenu("Combo E");
	ComboRSettings = ComboSettings->AddMenu("Combo R");
	//Combo Q Option Setup
	ComboQ = ComboQSettings->CheckBox("Enabled", true);
	//Combo W Option Setup
	ComboW = ComboWSettings->CheckBox("Enabled", true);
	//Combo E Option Setup
	ComboE = ComboESettings->CheckBox("Enabled", true);
	//Combo R Option Setup
	ComboR = ComboRSettings->CheckBox("Enabled", true);

	//Mixed Menu Setup
	MixedQSettings = MixedSettings->AddMenu("Mixed Q");
	MixedWSettings = MixedSettings->AddMenu("Mixed W");
	MixedESettings = MixedSettings->AddMenu("Mixed E");
	//Mixed Q Option Setup
	MixedQ = MixedQSettings->CheckBox("Enabled", true);
	//Mixed W Option Setup
	MixedW = MixedWSettings->CheckBox("Enabled", true);


	//Lane Clear Menu Setup
	LaneClearQSettings = LaneClearSettings->AddMenu("Lane Clear Q");
	LaneClearWSettings = LaneClearSettings->AddMenu("Lane Clear W");

	//Lane Clear Q Option Setup
	LaneClearQ = LaneClearQSettings->CheckBox("Use Q", true);
	LaneClearQMinionsRequired = LaneClearQSettings->AddInteger("Min # of Minions", 1, 2, 1);
	//Lane Clear W Option Setup
	LaneClearW = LaneClearWSettings->CheckBox("Use W", true);
	LaneClearWMinionsRequired = LaneClearWSettings->AddInteger("Min # of Minions", 1, 6, 3);

	//Draw Menu Setup
	DrawQRange = DrawSettings->CheckBox("Draw Q Range", true);
	DrawWRange = DrawSettings->CheckBox("Draw W Range", true);
	DrawERange = DrawSettings->CheckBox("Draw E Range", true);
	DrawRRange = DrawSettings->CheckBox("Draw R Range", true);
}

Vec3 cagePosition(IUnit* target) {

	Vec3 target_prediction;
	GPrediction->GetFutureUnitPosition(target, .5f, true, target_prediction);
	target_prediction = target_prediction.Extend(myHero->GetPosition(), E->Radius() - 15);
	return target_prediction;
}



float GetRDamage(IUnit* target)
{

	if (target == nullptr || !target->IsHero() || target->HasBuff("willrevive"))
		return 0;

	float BaseDamage = 0;


	/*	Veigar ult pretty interesting.
		These are just the calculations to predict the damages
		It's pretty accurate within about 10 damage or so, probably has to do
		with masteries and stuff
	*/
	double base_damage = std::vector<double>({ 175, 250, 325 }).at(myHero->GetSpellBook()->GetLevel(kSlotR)-1);
	double current_percent_health = (target->GetHealth() / target->GetMaxHealth()) * 100;
	double health_missing_perecent = 100 - current_percent_health;
	double damage_health_multiplier = health_missing_perecent * 1.5;
	if (damage_health_multiplier > 100) {
		damage_health_multiplier = 100;
	}
	double current_ap = myHero->TotalMagicDamage();
	double damage_before_mult = (.75 * current_ap);
	double damage_after_mult = ((damage_health_multiplier/100) * damage_before_mult) + damage_before_mult;
	double base_damage_a = ((damage_health_multiplier / 100) * base_damage) + base_damage;
	double total_damage = damage_after_mult + base_damage_a;
	double final_damage = GDamage->CalcMagicDamage(myHero, target, total_damage);
	//printDouble(final_damage, "final damage");

	BaseDamage += final_damage;


	if (GEntityList->Player()->HasBuff("SummonerExhaust"))
	BaseDamage *= 0.6;

	if (GEntityList->Player()->HasBuff("urgotentropypassive"))
	BaseDamage *= 0.85;

	auto bondofstoneBuffCount = target->GetBuffCount("MasteryWardenOfTheDawn");
	if (bondofstoneBuffCount > 0)
	BaseDamage *= 1 - 0.06 * bondofstoneBuffCount;

	if (target->HasBuff("FerociousHowl"))
	BaseDamage *= 0.6 - std::vector<double>({ 0.1, 0.2, 0.3 }).at(target->GetSpellBook()->GetLevel(kSlotR) - 1);

	if (target->HasBuff("Tantrum"))
	BaseDamage *= std::vector<double>({ 2, 4, 6, 8, 10 }).at(target->GetSpellBook()->GetLevel(kSlotE) - 1);

	if (target->HasBuff("BraumShieldRaise"))
	BaseDamage *= std::vector<double>({ 0.3, 0.325, 0.35, 0.375, 0.4 }).at(target->GetSpellBook()->GetLevel(kSlotE) - 1);

	if (target->HasBuff("GragasWSelf"))
	BaseDamage *= std::vector<double>({ 0.1, 0.12, 0.14, 0.16, 0.18 }).at(target->GetSpellBook()->GetLevel(kSlotW) - 1);

	auto phantomdancerBuff = target->GetBuffDataByName("itemphantomdancerdebuff");
	if (phantomdancerBuff != nullptr && GBuffData->GetCaster(phantomdancerBuff) == target)
	BaseDamage *= 0.88;

	if (target->HasBuff("GalioIdolOfDurand"))
	BaseDamage *= 0.5;

	if (target->HasBuff("GarenW"))
	BaseDamage *= 0.7;

	if (target->HasBuff("KatarinaEReduction"))
	BaseDamage *= 0.85;
	
	return BaseDamage;
}

void castQ(IUnit* target) {

	if (!CheckTarget(target)) { return; };

	if (!Q->IsReady()) { return; };

	if (!target->IsValidTarget(myHero, Q->Range() - 25)) { return; };

	//Q->SetRangeCheckFrom(myHero->ServerPosition());
	//Q->SetFrom(myHero->ServerPosition());
	AdvPredictionOutput outputfam;
	Q->RunPrediction(target, true, static_cast<eCollisionFlags>(kCollidesWithYasuoWall), &outputfam);
	Q->SetFrom(Vec3(0, 0, 0));
	
	printInt(outputfam.AoETargetsHit.size(), "Targets-Hit");
	printInt(outputfam.HitChance, "Hit Chance");

}

void castE(IUnit* target) {

	if (!CheckTarget(target)) { return; };

	if (!E->IsReady()) { return; };

	if (!target->IsValidTarget(myHero, E->Range() - 25)) { return; };

	auto position = cagePosition(target);

	E->CastOnPosition(position);

}

void castW(IUnit* target) {

	if (!CheckTarget(target)) { return; };
	if (!W->IsReady()) { return; };

	if (!target->IsValidTarget(myHero, W->Range() - 25)) { return; };

	if (!require_stun || target->HasBuffOfType(BUFF_Stun) || target->HasBuffOfType(BUFF_Suppression) || target->HasBuffOfType(BUFF_Snare)) {
		W->CastOnTargetAoE(target, 1, 4);
	}

}

void castR(IUnit* target) {

	if (!CheckTarget(target)) { return; };
	if (!R->IsReady()) { return; };
	if (!target->IsValidTarget(myHero, R->Range())) { return; };
	if (GetRDamage(target) < target->GetHealth()) { return; }

	R->CastOnTarget(target, kHitChanceLow);


}

void Combo()
{


	// Is Combo W Enabled?
	if (ComboW->Enabled())
	{
		auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range());
		//castW(target);
	}

	// Is Combo R Enabled?
	if (ComboR->Enabled())
	{
		auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, R->Range());
		//castR(target);
	}
	// Is Combo Q Enabled?
	if (ComboQ->Enabled())
	{
		auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());
		castQ(target);
	}

	// Is Combo E Enabled?
	if (ComboE->Enabled())
	{
		auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());
		//castE(target);
	}
}

void Mixed()
{
	if (MixedQ->Enabled())
	{
		auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, W->Range());
		castW(target);
	}
	if (MixedW->Enabled())
	{
		auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, R->Range());
		castR(target);
	}
	if (MixedE->Enabled())
	{
		auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, E->Range());
		castE(target);
	}
}

//void LastHitQ() {
//
//	for (auto minion : GEntityList->GetAllMinions(false, true, false))
//	{
//
//		if (minion == nullptr) { continue; };
//		if (!myHero )
//		if (minions != nullptr && myHero->IsValidTarget(minions, Q->Range()))
//		{
//			auto dmg = GDamage->GetSpellDamage(myHero, minions, kSlotQ);
//			if (lasthitQ->Enabled() && Q->IsReady())
//			{
//				if (GetDistance(myHero, minions) > myHero->GetRealAutoAttackRange(minions) && minions->GetHealth() <= dmg)
//				{
//					Q->CastOnUnit(minions);
//				}
//			}
//		}
//	}
//}

void LaneClear()
{
	if (LaneClearQ->Enabled())
	{
		Q->AttackMinions(1);
	}
	if (LaneClearW->Enabled())
	{
		W->AttackMinions(3);
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

	}
	if (DrawWRange->Enabled())
	{

	}
	if (DrawERange->Enabled())
	{

		GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range());


	}
	if (DrawRRange->Enabled())
	{

	}
}

PLUGIN_EVENT(void) OnRender()
{
	DrawRanges();
}

void InitSpells()
{

	auto me = GEntityList->Player();
	if (me == nullptr) {
		return;
	}
	//Q

	float range = me->GetSpellBook()->GetRange(kSlotQ);
	float speed = me->GetSpellBook()->GetSpeed(kSlotQ);
	float radius = me->GetSpellBook()->GetRadius(kSlotQ);

	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, true, kCollidesWithMinions);
	Q->SetOverrideRange(range);
	Q->SetOverrideSpeed(speed);
	Q->SetOverrideRadius(radius);
	//Delay?

	//W

	range = me->GetSpellBook()->GetRange(kSlotW);
	speed = me->GetSpellBook()->GetSpeed(kSlotW);
	radius = me->GetSpellBook()->GetRadius(kSlotW);

	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, true, kCollidesWithNothing);
	W->SetOverrideRange(range);
	W->SetOverrideSpeed(speed);
	W->SetOverrideRadius(radius);
	W->SetOverrideDelay(1.25f);
	//Delay?

	//E

	range = me->GetSpellBook()->GetRange(kSlotE);
	speed = me->GetSpellBook()->GetSpeed(kSlotE);
	radius = me->GetSpellBook()->GetRadius(kSlotE);

	E = GPluginSDK->CreateSpell2(kSlotE, kCircleCast, false, true, kCollidesWithNothing);
	E->SetOverrideRange(range + radius);
	E->SetOverrideSpeed(speed);
	E->SetOverrideRadius(radius);
	E->SetOverrideDelay(.5f);

	//R

	range = me->GetSpellBook()->GetRange(kSlotR);
	speed = me->GetSpellBook()->GetSpeed(kSlotR);
	radius = me->GetSpellBook()->GetRadius(kSlotR);

	R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, false, true, kCollidesWithNothing);
	R->SetOverrideRange(range);
	R->SetOverrideSpeed(speed);
	R->SetOverrideRadius(radius);
	//R->SetOverrideDelay(.5f);

}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();
	InitSpells();
	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);
}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();
	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
}