// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MPShooterHUD.generated.h"

class AMPShooterCharacter;
class AMPShooterPlayerController;

/**
 *  Simple shooter HUD drawn with the canvas (no widgets needed):
 *  crosshair, hit marker, health, ammo, kill feed, scoreboard and death screen
 */
UCLASS()
class AMPShooterHUD : public AHUD
{
	GENERATED_BODY()

protected:

	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor CrosshairColor = FLinearColor(1.f, 1.f, 1.f, 0.85f);

	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor HitMarkerColor = FLinearColor(1.f, 1.f, 1.f, 1.f);

	UPROPERTY(EditAnywhere, Category="HUD")
	FLinearColor KillMarkerColor = FLinearColor(1.f, 0.15f, 0.1f, 1.f);

	/** Show the controls help line at the bottom of the screen */
	UPROPERTY(EditAnywhere, Category="HUD")
	bool bShowControlsHelp = true;

public:

	virtual void DrawHUD() override;

protected:

	void DrawCrosshair(const AMPShooterCharacter* Character, float UIScale);
	void DrawHitMarker(const AMPShooterPlayerController* PC, float UIScale);
	void DrawVitals(const AMPShooterCharacter* Character, float UIScale);
	void DrawDeathScreen(const AMPShooterPlayerController* PC, float UIScale);
	void DrawKillFeed(const AMPShooterPlayerController* PC, float UIScale);
	void DrawScoreboard(float UIScale);
	void DrawControlsHelp(float UIScale);

	/** Draws text with a dark shadow so it reads on any background */
	void DrawShadowedText(const FString& Text, FLinearColor Color, float X, float Y, class UFont* Font, float Scale);
};
