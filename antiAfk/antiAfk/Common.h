#pragma once
#include "PluginSDK.h"


ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;



void printDouble(double num, char* abc) {

	char array[30];
	sprintf_s(array, "%f", num);
	GGame->PrintChat(abc);
	GGame->PrintChat(array);

}



inline double GetComboDamage(IUnit* target)
{
	if (target == nullptr || !target->IsHero() || !target->HasBuff("willrevive"))
		return 0;

	float BaseDamage = 0;

	if (Q->IsReady() && Q->ManaCost() < GEntityList->Player()->GetMana())
	{
		BaseDamage += GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotQ);
	}

	if (W->IsReady() && W->ManaCost() < GEntityList->Player()->GetMana())
	{
		BaseDamage += GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotW);
	}

	if (E->IsReady() && E->ManaCost() < GEntityList->Player()->GetMana())
	{
		BaseDamage += GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotE);
	}

	if (R->IsReady() && R->ManaCost() < GEntityList->Player()->GetMana())
	{
		BaseDamage += GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotR);
	}

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

	/*if (target->ChampionName() == "Moredkaiser")
	BaseDamage *= target->ManaPercent;

	if (target->HasBuff("BlitzcrankManaBarrierCD") && target->HasBuff("ManaBarrier"))
	BaseDamage *= (target->ManaPercent/2);*/

	return (float)BaseDamage;
}

inline bool CheckTarget(IUnit* target)
{
	if (target != nullptr && !target->IsDead() && !target->IsInvulnerable())
	{
		return true;
	}
	return false;
}


inline int CountEnemy(Vec3 Location, int range)
{
	int Count = 0;

	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		if ((Enemy->GetPosition() - Location).Length() < range && Enemy->IsValidTarget() && !Enemy->IsDead())
		{
			Count++;
		}
	}
	return (Count);
}

inline int CountAlly(Vec3 Location, int range)
{
	int Count = 0;

	for (auto Ally : GEntityList->GetAllHeros(true, false))
	{
		if ((Ally->GetPosition() - Location).Length() < range && Ally->IsValidTarget() && !Ally->IsDead() && Ally != GEntityList->Player())
		{
			Count++;
		}
	}
	return (Count);
}

inline int GetEnemiesInRange(float range)
{
	int enemies = 0;
	for (auto enemy : GEntityList->GetAllHeros(false, true))
	{
		if (enemy != nullptr && GEntityList->Player()->IsValidTarget(enemy, range))
			enemies++;
	}
	return enemies;
}


inline float GetDistance(IUnit* source, IUnit* target)
{
	auto x1 = source->GetPosition().x;
	auto x2 = target->GetPosition().x;
	auto y1 = source->GetPosition().y;
	auto y2 = target->GetPosition().y;
	auto z1 = source->GetPosition().z;
	auto z2 = target->GetPosition().z;
	return static_cast<float>(sqrt(pow((x2 - x1), 2.0) + pow((y2 - y1), 2.0) + pow((z2 - z1), 2.0)));
}

inline float GetDistanceVectors(Vec3 from, Vec3 to)
{
	float x1 = from.x;
	float x2 = to.x;
	float y1 = from.y;
	float y2 = to.y;
	float z1 = from.z;
	float z2 = to.z;
	return static_cast<float>(sqrt(pow((x2 - x1), 2.0) + pow((y2 - y1), 2.0) + pow((z2 - z1), 2.0)));
}




static bool IsKeyDown(IMenuOption *menuOption)
{
	return GetAsyncKeyState(menuOption->GetInteger()) & 0x8000;
}

inline bool ValidUlt(IUnit* target)
{
	if (target->HasBuffOfType(BUFF_PhysicalImmunity) || target->HasBuffOfType(BUFF_SpellImmunity)

		|| target->IsInvulnerable() || target->HasBuffOfType(BUFF_Invulnerability) || target->HasBuff("kindredrnodeathbuff")

		|| target->HasBuffOfType(BUFF_SpellShield))
	{
		return false;
	}
	else
	{
		return true;
	}
}

static bool CheckShielded(IUnit* target)
{
	if (!target->HasBuff("BlackShield") && !target->HasBuff("bansheesveil") && !target->HasBuff("itemmagekillerveil"))
	{
		return true;
	}

	return false;
}