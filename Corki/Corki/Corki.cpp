#include "PluginSDK.h"

PluginSetup("Corki");

IMenu* MainMenu;
IMenu* ComboMenu;
IMenu* HarassMenu;
IMenu* LaneClearMenu;
IMenu* KSMenu;
IMenu* DrawMenu;

IMenuOption* ComboQ;
IMenuOption* ComboE;
IMenuOption* ComboR;

IMenuOption* HarassMana;
IMenuOption* HarassQ;
IMenuOption* HarassE;
IMenuOption* HarassR;

IMenuOption* LaneClearMana;
IMenuOption* LaneClearQ;
IMenuOption* LaneClearQHit;

IMenuOption* KSQ;
IMenuOption* KSR;

IMenuOption* DrawQ;
IMenuOption* DrawW;
IMenuOption* DrawE;
IMenuOption* DrawR;
IMenuOption* DrawReady;
IMenuOption* DrawOff;

ISpell2* Q;
ISpell2* W;
ISpell2* E;
ISpell2* R;

bool BigR = false;

void Menu()
{
	MainMenu = GPluginSDK->AddMenu("Corki | The Daring Bombardier");

	ComboMenu = MainMenu->AddMenu("Combo");
	ComboQ = ComboMenu->CheckBox("Use Q", true);
	ComboE = ComboMenu->CheckBox("Use E", true);
	ComboR = ComboMenu->CheckBox("Use R", true);

	HarassMenu = MainMenu->AddMenu("Harass");
	HarassQ = HarassMenu->CheckBox("Use Q", true);
	HarassE = HarassMenu->CheckBox("Use E", true);
	HarassR = HarassMenu->CheckBox("Use R", true);
	HarassMana = HarassMenu->AddInteger("Mana Manager(%)", 0, 100, 45);

	LaneClearMenu = MainMenu->AddMenu("LaneClear");
	LaneClearQ = LaneClearMenu->CheckBox("Use Q", true);
	LaneClearQHit = LaneClearMenu->AddInteger("Q Hit Count", 0, 10, 3);
	LaneClearMana = LaneClearMenu->AddInteger("Mana Manager(%)", 0, 100, 45);

	KSMenu = MainMenu->AddMenu("KillSteal");
	KSQ = KSMenu->CheckBox("Use Q", true);
	KSR = KSMenu->CheckBox("Use R", true);

	DrawMenu = MainMenu->AddMenu("Drawings");
	DrawQ = DrawMenu->CheckBox("Use Q", true);
	DrawW = DrawMenu->CheckBox("Use W", true);
	DrawE = DrawMenu->CheckBox("Use E", true);
	DrawR = DrawMenu->CheckBox("Use R", true);
	DrawReady = DrawMenu->CheckBox("Draw Only Ready Spells", true);
	DrawOff = DrawMenu->CheckBox("Disable All Drawings", false);
}

void Spells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kCircleCast, true, true, static_cast<eCollisionFlags>(kCollidesWithNothing));
	W = GPluginSDK->CreateSpell2(kSlotW, kLineCast, false, true, static_cast<eCollisionFlags>(kCollidesWithNothing));
	E = GPluginSDK->CreateSpell2(kSlotE, kLineCast, false, true, static_cast<eCollisionFlags>(kCollidesWithNothing));
	R = GPluginSDK->CreateSpell2(kSlotR, kLineCast, true, true, static_cast<eCollisionFlags>(kCollidesWithYasuoWall | kCollidesWithMinions));

	Q->SetOverrideRange(825);
	Q->SetOverrideRadius(125);
	Q->SetOverrideDelay(0.35);
	Q->SetOverrideSpeed(1000);

	W->SetOverrideRange(525);

	E->SetOverrideRange(600);
	
	R->SetOverrideRange(1300);
	R->SetOverrideRadius(20);
	R->SetOverrideDelay(0.2);
	R->SetOverrideSpeed(2000);
}

void CastQ(IUnit* target)
{
	if (Q->IsReady() && target->IsValidTarget(target, Q->Range()))
	{
		Q->CastOnTarget(target, kHitChanceHigh);
	}
}

void CastE(IUnit* target)
{
	if (E->IsReady() && target->IsValidTarget(target, E->Range()))
	{
		E->CastOnTarget(target);
	}
}

void CastR(IUnit* target)
{
	if (R->IsReady())
	{
		if (BigR)
		{
			if (target->IsValidTarget(target, R->Range() + 200))
			{
				R->CastOnTarget(target, kHitChanceHigh);
			}
		}
		else
		{
			if (target->IsValidTarget(target, R->Range()))
			{
				R->CastOnTarget(target, kHitChanceHigh);
			}
		}	
	}
}

void Combo(IUnit* target)
{
	if (GOrbwalking->GetOrbwalkingMode() == kModeCombo)
	{
		if (ComboQ->Enabled())
		{
			CastQ(target);
		}
		if (ComboE->Enabled())
		{
			CastE(target);
		}
		if (ComboR->Enabled())
		{
			CastR(target);
		}
	}
}

void Harass(IUnit* target)
{
	if (GOrbwalking->GetOrbwalkingMode() == kModeMixed && GEntityList->Player()->ManaPercent() >= HarassMana->GetInteger())
	{
		if (ComboQ->Enabled())
		{
			CastQ(target);
		}
		if (ComboE->Enabled())
		{
			CastE(target);
		}
		if (ComboR->Enabled())
		{
			CastR(target);
		}
	}
}

void LaneClear()
{
	if (GOrbwalking->GetOrbwalkingMode() == kModeLaneClear && GEntityList->Player()->ManaPercent() >= LaneClearMana->GetInteger())
	{
		if (LaneClearQ->Enabled())
		{
			int count = 0;
			Vec3 pos = Vec3();
			Q->FindBestCastPosition(true, false, pos, count);
			if (count >= LaneClearQHit->GetInteger())
			{
				Q->CastOnPosition(pos);
			}
		}
	}
}

void KS()
{
	for (auto Enemy : GEntityList->GetAllHeros(false, true))
	{
		if (Enemy != nullptr && !Enemy->IsDead())
		{
			if (KSQ->Enabled())
			{
				auto dmg = GHealthPrediction->GetKSDamage(Enemy, kSlotQ, Q->GetDelay(), true);
				if (Enemy->GetHealth() <= dmg)
				{
					CastQ(Enemy);
				}
			}
			if (KSR->Enabled())
			{
				auto dmg = GHealthPrediction->GetKSDamage(Enemy, kSlotR, R->GetDelay(), true);
				if (Enemy->GetHealth() <= dmg)
				{
					CastR(Enemy);
				}
			}
		}
	}
}

void BigRUpdate()
{
	if (GEntityList->Player()->HasBuff("mbcheck2"))
	{
		BigR = true;
	}
	else
	{
		BigR = false;
	}
}


PLUGIN_EVENT(void) OnGameUpdate()
{
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, R->Range()+200);

	if (target)
	{
		Combo(target);
		Harass(target);
	}

	LaneClear();
	KS();
	BigRUpdate();
}

PLUGIN_EVENT(void) OnRender()
{
	if (DrawOff->Enabled()) return;

	if (DrawReady->Enabled())
	{
		if (DrawQ->Enabled() && Q->IsReady())
		{
			GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range());
		}
		if (DrawW->Enabled() && W->IsReady())
		{
			GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), W->Range());
		}
		if (DrawE->Enabled() && E->IsReady())
		{
			GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range());
		}
		if (DrawR->Enabled() && R->IsReady())
		{
			if (BigR)
			{
				GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range() + 200);
			}
			else
			{
				GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range());
			}
		}
	}
	else
	{
		if (DrawQ->Enabled())
		{
			GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), Q->Range());
		}
		if (DrawW->Enabled())
		{
			GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), W->Range());
		}
		if (DrawE->Enabled())
		{
			GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), E->Range());
		}
		if (DrawR->Enabled())
		{
			if (BigR)
			{
				GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range() + 200);
			}
			else
			{
				GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range());
			}
		}
	}
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();
	Spells();

	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);
}

PLUGIN_API void OnUnload()
{
	MainMenu->Remove();

	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
}