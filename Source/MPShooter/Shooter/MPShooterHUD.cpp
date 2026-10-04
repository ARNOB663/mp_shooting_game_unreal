// Copyright Epic Games, Inc. All Rights Reserved.

#include "MPShooterHUD.h"
#include "MPShooterCharacter.h"
#include "MPShooterPlayerController.h"
#include "MPShooterPlayerState.h"
#include "MPWeapon.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "Engine/Font.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"

void AMPShooterHUD::DrawHUD()
{
	Super::DrawHUD();

	if (!Canvas || !GEngine)
	{
		return;
	}

	// everything is laid out for 1080p and scaled to the real screen
	const float UIScale = Canvas->ClipY / 1080.f;

	const AMPShooterPlayerController* PC = Cast<AMPShooterPlayerController>(PlayerOwner);
	const AMPShooterCharacter* Character = PC ? Cast<AMPShooterCharacter>(PC->GetPawn()) : nullptr;

	if (Character && !Character->IsDead())
	{
		DrawCrosshair(Character, UIScale);
		DrawHitMarker(PC, UIScale);
		DrawVitals(Character, UIScale);
	}
	else if (Character)
	{
		DrawDeathScreen(PC, UIScale);
	}

	if (PC)
	{
		DrawKillFeed(PC, UIScale);
	}

	DrawScoreboard(UIScale);

	if (bShowControlsHelp)
	{
		DrawControlsHelp(UIScale);
	}
}

void AMPShooterHUD::DrawShadowedText(const FString& Text, FLinearColor Color, float X, float Y, UFont* Font, float Scale)
{
	DrawText(Text, FLinearColor(0.f, 0.f, 0.f, 0.8f), X + 2.f, Y + 2.f, Font, Scale);
	DrawText(Text, Color, X, Y, Font, Scale);
}

void AMPShooterHUD::DrawCrosshair(const AMPShooterCharacter* Character, float UIScale)
{
	const float CenterX = Canvas->ClipX * 0.5f;
	const float CenterY = Canvas->ClipY * 0.5f;

	// the lines move apart with the weapon's spread
	const AMPWeapon* Weapon = Character->GetEquippedWeapon();
	const float Spread = Weapon ? Weapon->GetCurrentSpread() : 1.f;

	const float Gap = (4.f + Spread * 6.f) * UIScale;
	const float Length = 10.f * UIScale;
	const float Thickness = FMath::Max(1.f, 2.f * UIScale);

	DrawLine(CenterX - Gap - Length, CenterY, CenterX - Gap, CenterY, CrosshairColor, Thickness);
	DrawLine(CenterX + Gap, CenterY, CenterX + Gap + Length, CenterY, CrosshairColor, Thickness);
	DrawLine(CenterX, CenterY - Gap - Length, CenterX, CenterY - Gap, CrosshairColor, Thickness);
	DrawLine(CenterX, CenterY + Gap, CenterX, CenterY + Gap + Length, CrosshairColor, Thickness);
	DrawRect(CrosshairColor, CenterX - Thickness * 0.5f, CenterY - Thickness * 0.5f, Thickness, Thickness);
}

void AMPShooterHUD::DrawHitMarker(const AMPShooterPlayerController* PC, float UIScale)
{
	const double Age = GetWorld()->GetTimeSeconds() - PC->GetLastHitConfirmTime();
	const double Duration = 0.25;

	if (Age > Duration)
	{
		return;
	}

	FLinearColor Color = PC->WasLastHitKill() || PC->WasLastHitHeadshot() ? KillMarkerColor : HitMarkerColor;
	Color.A = static_cast<float>(1.0 - Age / Duration);

	const float CenterX = Canvas->ClipX * 0.5f;
	const float CenterY = Canvas->ClipY * 0.5f;
	const float Inner = 6.f * UIScale;
	const float Outer = (PC->WasLastHitKill() ? 18.f : 13.f) * UIScale;
	const float Thickness = FMath::Max(1.f, 2.5f * UIScale);

	DrawLine(CenterX - Outer, CenterY - Outer, CenterX - Inner, CenterY - Inner, Color, Thickness);
	DrawLine(CenterX + Outer, CenterY - Outer, CenterX + Inner, CenterY - Inner, Color, Thickness);
	DrawLine(CenterX - Outer, CenterY + Outer, CenterX - Inner, CenterY + Inner, Color, Thickness);
	DrawLine(CenterX + Outer, CenterY + Outer, CenterX + Inner, CenterY + Inner, Color, Thickness);
}

void AMPShooterHUD::DrawVitals(const AMPShooterCharacter* Character, float UIScale)
{
	UFont* LargeFont = GEngine->GetLargeFont();
	UFont* MediumFont = GEngine->GetMediumFont();

	// health bar, bottom left
	const float Margin = 40.f * UIScale;
	const float BarWidth = 320.f * UIScale;
	const float BarHeight = 22.f * UIScale;
	const float BarX = Margin;
	const float BarY = Canvas->ClipY - Margin - BarHeight;

	const float HealthPercent = FMath::Clamp(Character->GetHealth() / Character->GetMaxHealth(), 0.f, 1.f);
	const FLinearColor HealthColor = FLinearColor::LerpUsingHSV(FLinearColor(0.9f, 0.1f, 0.1f), FLinearColor(0.2f, 0.85f, 0.3f), HealthPercent);

	DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.5f), BarX - 3.f * UIScale, BarY - 3.f * UIScale, BarWidth + 6.f * UIScale, BarHeight + 6.f * UIScale);
	DrawRect(HealthColor, BarX, BarY, BarWidth * HealthPercent, BarHeight);
	DrawShadowedText(FString::Printf(TEXT("HP %d"), FMath::CeilToInt(Character->GetHealth())), FLinearColor::White, BarX, BarY - 36.f * UIScale, LargeFont, UIScale * 1.2f);

	// ammo, bottom right
	const AMPWeapon* Weapon = Character->GetEquippedWeapon();

	if (!Weapon)
	{
		return;
	}

	const FString AmmoText = FString::Printf(TEXT("%d / %d"), Weapon->GetAmmoInMagazine(), Weapon->GetReserveAmmo());
	const FString WeaponText = Weapon->IsReloading() ? FString::Printf(TEXT("%s - RELOADING"), *Weapon->GetWeaponName()) : Weapon->GetWeaponName();

	float AmmoWidth, AmmoHeight;
	GetTextSize(AmmoText, AmmoWidth, AmmoHeight, LargeFont, UIScale * 1.6f);

	float NameWidth, NameHeight;
	GetTextSize(WeaponText, NameWidth, NameHeight, MediumFont, UIScale);

	const FLinearColor AmmoColor = Weapon->GetAmmoInMagazine() == 0 ? FLinearColor(1.f, 0.25f, 0.2f) : FLinearColor::White;

	DrawShadowedText(AmmoText, AmmoColor, Canvas->ClipX - Margin - AmmoWidth, Canvas->ClipY - Margin - AmmoHeight, LargeFont, UIScale * 1.6f);
	DrawShadowedText(WeaponText, FLinearColor(0.9f, 0.9f, 0.9f), Canvas->ClipX - Margin - NameWidth, Canvas->ClipY - Margin - AmmoHeight - NameHeight - 4.f * UIScale, MediumFont, UIScale);
}

void AMPShooterHUD::DrawDeathScreen(const AMPShooterPlayerController* PC, float UIScale)
{
	UFont* LargeFont = GEngine->GetLargeFont();

	DrawRect(FLinearColor(0.3f, 0.f, 0.f, 0.25f), 0.f, 0.f, Canvas->ClipX, Canvas->ClipY);

	const FString Title = TEXT("ELIMINATED");
	const FString KillerName = PC ? PC->GetLastKillerName() : FString();
	const FString Subtitle = KillerName.IsEmpty() ? FString(TEXT("Respawning...")) : FString::Printf(TEXT("by %s - respawning..."), *KillerName);

	float TitleWidth, TitleHeight;
	GetTextSize(Title, TitleWidth, TitleHeight, LargeFont, UIScale * 3.f);

	float SubWidth, SubHeight;
	GetTextSize(Subtitle, SubWidth, SubHeight, LargeFont, UIScale * 1.3f);

	const float CenterY = Canvas->ClipY * 0.4f;

	DrawShadowedText(Title, FLinearColor(1.f, 0.2f, 0.15f), (Canvas->ClipX - TitleWidth) * 0.5f, CenterY - TitleHeight, LargeFont, UIScale * 3.f);
	DrawShadowedText(Subtitle, FLinearColor::White, (Canvas->ClipX - SubWidth) * 0.5f, CenterY + 10.f * UIScale, LargeFont, UIScale * 1.3f);
}

void AMPShooterHUD::DrawKillFeed(const AMPShooterPlayerController* PC, float UIScale)
{
	UFont* Font = GEngine->GetMediumFont();
	const double Now = GetWorld()->GetTimeSeconds();
	const double ShowTime = 6.0;

	float Y = 40.f * UIScale;

	for (int32 Index = PC->GetKillFeed().Num() - 1; Index >= 0; --Index)
	{
		const FMPKillFeedEntry& Entry = PC->GetKillFeed()[Index];

		if (Now - Entry.Time > ShowTime)
		{
			continue;
		}

		float Width, Height;
		GetTextSize(Entry.Text, Width, Height, Font, UIScale * 1.1f);

		const float X = Canvas->ClipX - 40.f * UIScale - Width;

		DrawRect(FLinearColor(0.f, 0.f, 0.f, 0.45f), X - 8.f * UIScale, Y - 3.f * UIScale, Width + 16.f * UIScale, Height + 6.f * UIScale);
		DrawText(Entry.Text, FLinearColor::White, X, Y, Font, UIScale * 1.1f);

		Y += Height + 10.f * UIScale;
	}
}

void AMPShooterHUD::DrawScoreboard(float UIScale)
{
	const AGameStateBase* GameState = GetWorld()->GetGameState();

	if (!GameState)
	{
		return;
	}

	TArray<const AMPShooterPlayerState*> Players;

	for (const APlayerState* PlayerState : GameState->PlayerArray)
	{
		if (const AMPShooterPlayerState* ShooterState = Cast<AMPShooterPlayerState>(PlayerState))
		{
			Players.Add(ShooterState);
		}
	}

	if (Players.Num() == 0)
	{
		return;
	}

	Players.Sort([](const AMPShooterPlayerState& A, const AMPShooterPlayerState& B)
	{
		return A.GetKills() > B.GetKills();
	});

	UFont* Font = GEngine->GetSmallFont();
	const float Scale = UIScale * 1.2f;
	float Y = 40.f * UIScale;
	const float X = 40.f * UIScale;

	const APlayerState* LocalState = PlayerOwner ? PlayerOwner->PlayerState : nullptr;

	DrawShadowedText(TEXT("PLAYER                K   D"), FLinearColor(1.f, 0.85f, 0.3f), X, Y, Font, Scale);
	Y += 22.f * UIScale;

	for (const AMPShooterPlayerState* Player : Players)
	{
		const FString Name = Player->GetPlayerName().Left(18);
		const FString Line = FString::Printf(TEXT("%-20s %3d %3d"), *Name, Player->GetKills(), Player->GetDeaths());
		const FLinearColor Color = Player == LocalState ? FLinearColor(0.4f, 1.f, 0.5f) : FLinearColor::White;

		DrawShadowedText(Line, Color, X, Y, Font, Scale);
		Y += 20.f * UIScale;
	}
}

void AMPShooterHUD::DrawControlsHelp(float UIScale)
{
	UFont* Font = GEngine->GetSmallFont();
	const FString Help = TEXT("LMB fire   RMB aim   R reload   V first/third person   1 / 2 / 3 or wheel switch weapon");

	float Width, Height;
	GetTextSize(Help, Width, Height, Font, UIScale);

	DrawShadowedText(Help, FLinearColor(1.f, 1.f, 1.f, 0.7f), (Canvas->ClipX - Width) * 0.5f, Canvas->ClipY - Height - 12.f * UIScale, Font, UIScale);
}
