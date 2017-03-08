#include "PluginSDK.h"
#include <limits>

PluginSetup("Shulepin's Brand");

IMenu *Config, *Combo, *Harass, *Drawings;
IMenuOption *ComboQ1, *ComboQ2, *ComboW1, *ComboW2, *ComboE, *ComboR, *BlockAA;
IMenuOption *Damage, *DrawQ, *DrawW, *DrawE, *DrawR, *DrawOff;
ISpell2 *Q, *W, *E, *R;
int xOffset = 10, yOffset = 15, Width = 103, Height = 9;;
Vec4 Color = Vec4(105, 198, 5, 255), FillColor = Vec4(198, 176, 5, 255);

void Menu()
{
	Config = GPluginSDK->AddMenu("Brand");

	Combo = Config->AddMenu("Combo");
	{
		ComboQ1 = Combo->CheckBox("Use Q", true);
		ComboQ2 = Combo->CheckBox("Use Q Only If Ablazed", true);
		ComboW1 = Combo->CheckBox("Use W", true);
		ComboW2 = Combo->CheckBox("Use W Only After E+Q", true);
		ComboE = Combo->CheckBox("Use E", true);
		ComboR = Combo->CheckBox("Use R", true);
		BlockAA = Combo->CheckBox("Block AA", false);
	}
	Drawings = Config->AddMenu("Drawings");
	{
		Damage = Drawings->CheckBox("Draw Damage Indicator", true);
		DrawQ = Drawings->CheckBox("Draw Q Range", true);
		DrawW = Drawings->CheckBox("Draw W Range", true);
		DrawE = Drawings->CheckBox("Draw E Range", true);
		DrawR = Drawings->CheckBox("Draw R Range", true);
		DrawOff = Drawings->CheckBox("Disable All Drawings", false);
	}
}

void Spells()
{
	Q = GPluginSDK->CreateSpell2(kSlotQ, kLineCast, true, false, static_cast<eCollisionFlags>(kCollidesWithYasuoWall | kCollidesWithMinions));
	W = GPluginSDK->CreateSpell2(kSlotW, kCircleCast, false, true, static_cast<eCollisionFlags>(kCollidesWithNothing));
	E = GPluginSDK->CreateSpell2(kSlotE, kTargetCast, false, false, static_cast<eCollisionFlags>(kCollidesWithNothing));
	R = GPluginSDK->CreateSpell2(kSlotR, kTargetCast, true, true, static_cast<eCollisionFlags>(kCollidesWithYasuoWall));

	Q->SetSkillshot(0.25f, 60.f, 1600.f, 1100.f);
	W->SetSkillshot(0.85f, 250.f, std::numeric_limits<float>::infinity(), 900);
	E->SetOverrideRange(630.f);
	R->SetOverrideRange(750.f);
}

double PassiveDamage(IUnit *target)
{
	return GDamage->CalcMagicDamage(GEntityList->Player(), target, (target->GetMaxHealth() * 0.08) - (target->HPRegenRate() * 5));
}

double ComboDamage(IUnit *target)
{
	double Damage = 0;
	if (Q->IsReady())
	{
		Damage += GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotQ) + PassiveDamage(target);
	}
	if (W->IsReady())
	{
		Damage += GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotW) + PassiveDamage(target);
	}
	if (E->IsReady())
	{
		Damage += GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotE) + PassiveDamage(target);
	}
	if (R->IsReady())
	{
		Damage += GDamage->GetSpellDamage(GEntityList->Player(), target, kSlotR) + PassiveDamage(target);
	}
	return Damage;
}

void ComboMode()
{
	auto target = GTargetSelector->FindTarget(QuickestKill, SpellDamage, Q->Range());

	if (target == nullptr || !target->IsValidTarget()) { return; }

	if (BlockAA->Enabled() && target->HealthPercent() > 20)
	{
		GOrbwalking->SetAttacksAllowed(false);
		if (!Q->IsReady() && !E->IsReady() && !W->IsReady())
		{
			GOrbwalking->SetAttacksAllowed(true);
		}
	}
	else
	{
		GOrbwalking->SetAttacksAllowed(true);
	}

	if (E->IsReady() && target->IsValidTarget(target, E->Range()) && ComboE->Enabled())
	{
		E->CastOnUnit(target);
	}

	if (Q->IsReady() && target->IsValidTarget(target, Q->Range()) && ComboQ1->Enabled())
	{
		if (!ComboQ2->Enabled())
		{
			Q->CastOnTarget(target, 5);
		}
		else
		{
			if (target->HasBuff("brandablaze"))
			{
				Q->CastOnTarget(target, 5);
			}
		}
	}

	if (W->IsReady() && target->IsValidTarget(target, W->Range()) && ComboW1->Enabled())
	{
		if (!ComboW2->Enabled() || GEntityList->Player()->GetLevel() < 4)
		{
			W->CastOnTarget(target, 5);
		}
		else
		{
			if (!E->IsReady() && !Q->IsReady())
			{
				W->CastOnTarget(target, 5); 
			}
		}
	}

	if (R->IsReady() && target->IsValidTarget(target, R->Range()))
	{
		if (ComboR->Enabled() && ComboDamage(target) >= target->GetHealth())
		{
			R->CastOnUnit(target);
		}
	}
}

PLUGIN_EVENT(void) OnGameUpdate()
{
	switch (GOrbwalking->GetOrbwalkingMode())
	{
	    case kModeCombo:
		ComboMode();
		break;
	    default:
		break;
	}
}

PLUGIN_EVENT(void) OnRender()
{
	if (DrawOff->Enabled()) return;

	//Credits: MotionCuber | https://www.leagueplusplus.net/forums/topic/266-how-much-hurts/
	if (Damage->Enabled())
	{
		for (auto hero : GEntityList->GetAllHeros(false, true))
		{
			if (!hero->IsDead() && hero->IsVisible())
			{
				Vec2 barPos = Vec2();
				if (hero->GetHPBarPosition(barPos))
				{
					float damage = ComboDamage(hero);
					float percentHealthAfterDamage = max(0, hero->GetHealth() - damage) / hero->GetMaxHealth();
					float yPos = barPos.y + yOffset;
					float xPosDamage = barPos.x + xOffset + Width * percentHealthAfterDamage;
					float xPosCurrentHp = barPos.x + xOffset + Width * hero->GetHealth() / hero->GetMaxHealth();

					if (damage > hero->GetHealth())
					{
						GRender->DrawTextW(Vec2(barPos.x + xOffset, barPos.y + yOffset - 13), Color, "Killable");
					}

					GRender->DrawLine(Vec2(xPosDamage, yPos), Vec2(xPosDamage, yPos + Height), Color);

					float differenceInHP = xPosCurrentHp - xPosDamage;
					float pos1 = barPos.x + 9 + (107 * percentHealthAfterDamage);

					for (int i = 0; i < differenceInHP; i++)
					{
						GRender->DrawLine(Vec2(pos1 + i, yPos), Vec2(pos1 + i, yPos + Height), FillColor);
					}
				}
			}
		}
	}
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
		GRender->DrawOutlinedCircle(GEntityList->Player()->GetPosition(), Vec4(255, 255, 0, 255), R->Range());
	} 
}

PLUGIN_API void OnLoad(IPluginSDK* PluginSDK)
{
	PluginSDKSetup(PluginSDK);
	Menu();
	Spells();
	GRender->Notification(Vec4(255, 255, 0, 255), 5, "Shulepin's Brand Loaded!");

	GEventManager->AddEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->AddEventHandler(kEventOnRender, OnRender);
}

PLUGIN_API void OnUnload()
{
	Config->Remove();

	GEventManager->RemoveEventHandler(kEventOnGameUpdate, OnGameUpdate);
	GEventManager->RemoveEventHandler(kEventOnRender, OnRender);
}