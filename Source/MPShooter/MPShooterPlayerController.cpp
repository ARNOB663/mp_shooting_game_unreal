// Copyright Epic Games, Inc. All Rights Reserved.


#include "MPShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "InputCoreTypes.h"
#include "Components/SkeletalMeshComponent.h"
#include "Blueprint/UserWidget.h"
#include "MPShooter.h"
#include "MPShooterCharacter.h"
#include "MPWeapon.h"
#include "Widgets/Input/SVirtualJoystick.h"

void AMPShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController() && ShouldUseTouchControls())
	{
		// spawn the mobile controls widget
		MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

		if (MobileControlsWidget)
		{
			// add the controls to the player screen
			MobileControlsWidget->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogMPShooter, Error, TEXT("Could not spawn mobile controls widget."));

		}

	}
}

void AMPShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}

		EnsureShooterInput();
	}
}

bool AMPShooterPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AMPShooterPlayerController::EnsureShooterInput()
{
	if (bShooterInputReady || !IsLocalPlayerController())
	{
		return;
	}

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

	if (!Subsystem)
	{
		return;
	}

	bShooterInputReady = true;

	GeneratedShooterContext = NewObject<UInputMappingContext>(this, TEXT("IMC_MPShooter_Generated"));

	// creates a button action with default keys, unless one was assigned in the Blueprint
	auto MakeAction = [this](TObjectPtr<UInputAction>& Action, const TCHAR* Name, const TArray<FKey>& Keys)
	{
		if (Action)
		{
			return;
		}

		Action = NewObject<UInputAction>(this, Name);

		for (const FKey& Key : Keys)
		{
			GeneratedShooterContext->MapKey(Action, Key);
		}
	};

	MakeAction(FireAction, TEXT("IA_MP_Fire"), { EKeys::LeftMouseButton, EKeys::Gamepad_RightTrigger });
	MakeAction(AimAction, TEXT("IA_MP_Aim"), { EKeys::RightMouseButton, EKeys::Gamepad_LeftTrigger });
	MakeAction(ReloadAction, TEXT("IA_MP_Reload"), { EKeys::R, EKeys::Gamepad_FaceButton_Left });
	MakeAction(ToggleViewAction, TEXT("IA_MP_ToggleView"), { EKeys::V, EKeys::Gamepad_RightThumbstick });
	MakeAction(NextWeaponAction, TEXT("IA_MP_NextWeapon"), { EKeys::MouseScrollDown, EKeys::Gamepad_FaceButton_Top });

	if (WeaponSlotActions.Num() < 3)
	{
		WeaponSlotActions.SetNum(3);
	}

	MakeAction(WeaponSlotActions[0], TEXT("IA_MP_Weapon1"), { EKeys::One });
	MakeAction(WeaponSlotActions[1], TEXT("IA_MP_Weapon2"), { EKeys::Two });
	MakeAction(WeaponSlotActions[2], TEXT("IA_MP_Weapon3"), { EKeys::Three });

	// priority 1 so these keys win over the template's default context
	Subsystem->AddMappingContext(GeneratedShooterContext, 1);
}

void AMPShooterPlayerController::NotifyHitConfirmed(bool bHeadshot, bool bKilled)
{
	LastHitConfirmTime = GetWorld()->GetTimeSeconds();
	bLastHitWasHeadshot = bHeadshot;
	bLastHitWasKill = bKilled;
}

void AMPShooterPlayerController::ClientAddKillFeedMessage_Implementation(const FString& Message)
{
	FMPKillFeedEntry Entry;
	Entry.Text = Message;
	Entry.Time = GetWorld()->GetTimeSeconds();
	KillFeed.Add(Entry);

	// keep the last few lines only
	while (KillFeed.Num() > 5)
	{
		KillFeed.RemoveAt(0);
	}
}

void AMPShooterPlayerController::ClientNotifyKilledBy_Implementation(const FString& KillerName)
{
	LastKillerName = KillerName;
}

namespace
{
	void PrintTuning(const FString& Line)
	{
		UE_LOG(LogMPShooter, Display, TEXT("%s"), *Line);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.f, FColor::Yellow, Line);
		}
	}
}

void AMPShooterPlayerController::MPGunTP(float X, float Y, float Z, float Pitch, float Yaw, float Roll, float Scale)
{
	const AMPShooterCharacter* ShooterCharacter = Cast<AMPShooterCharacter>(GetPawn());
	AMPWeapon* Weapon = ShooterCharacter ? ShooterCharacter->GetEquippedWeapon() : nullptr;

	if (!Weapon)
	{
		PrintTuning(TEXT("MPGunTP: no weapon in hand"));
		return;
	}

	Weapon->SetThirdPersonOffset(FTransform(FRotator(Pitch, Yaw, Roll), FVector(X, Y, Z), FVector(Scale)));

	PrintTuning(FString::Printf(TEXT("%s Third Person Offset -> Location (%g, %g, %g) Rotation (Pitch %g, Yaw %g, Roll %g) Scale %g  [only on this PC: copy into the weapon's Blueprint / C++ defaults]"),
		*Weapon->GetWeaponName(), X, Y, Z, Pitch, Yaw, Roll, Scale));
}

void AMPShooterPlayerController::MPGunFP(float X, float Y, float Z, float Pitch, float Yaw, float Roll, float Scale)
{
	const AMPShooterCharacter* ShooterCharacter = Cast<AMPShooterCharacter>(GetPawn());
	AMPWeapon* Weapon = ShooterCharacter ? ShooterCharacter->GetEquippedWeapon() : nullptr;

	if (!Weapon)
	{
		PrintTuning(TEXT("MPGunFP: no weapon in hand"));
		return;
	}

	Weapon->SetFirstPersonOffset(FTransform(FRotator(Pitch, Yaw, Roll), FVector(X, Y, Z), FVector(Scale)));

	PrintTuning(FString::Printf(TEXT("%s First Person Offset -> Location (%g, %g, %g) Rotation (Pitch %g, Yaw %g, Roll %g) Scale %g  [only on this PC: copy into the weapon's Blueprint / C++ defaults]"),
		*Weapon->GetWeaponName(), X, Y, Z, Pitch, Yaw, Roll, Scale));
}

void AMPShooterPlayerController::MPArms(float X, float Y, float Z, float Pitch, float Yaw, float Roll)
{
	const AMPShooterCharacter* ShooterCharacter = Cast<AMPShooterCharacter>(GetPawn());

	if (!ShooterCharacter || !ShooterCharacter->GetFirstPersonArms())
	{
		PrintTuning(TEXT("MPArms: no character"));
		return;
	}

	ShooterCharacter->GetFirstPersonArms()->SetRelativeLocationAndRotation(FVector(X, Y, Z), FRotator(Pitch, Yaw, Roll));

	PrintTuning(FString::Printf(TEXT("First Person Arms -> Location (%g, %g, %g) Rotation (Pitch %g, Yaw %g, Roll %g)  [copy into BP_ThirdPersonCharacter > FirstPersonArms]"),
		X, Y, Z, Pitch, Yaw, Roll));
}
