#pragma once
#pragma once
#include "PluginSDK.h"
//	Really shit of way of printing but it works
void printDouble(double num, char* abc) {

	char array[30];
	sprintf_s(array, "%f", num);
	GGame->PrintChat(abc);
	GGame->PrintChat(array);
}


void printInt(int num, char* text) {
	char array[30];
	sprintf_s(array, "%d", num);
	GGame->PrintChat(text);
	GGame->PrintChat(array);
}


inline bool CheckTarget(IUnit* target)
{
	if (target != nullptr && !target->IsDead() && !target->IsInvulnerable())
	{
		return true;
	}
	return false;
}