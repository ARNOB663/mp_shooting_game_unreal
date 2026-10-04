// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MPWeapon.h"
#include "MPWeaponPresets.generated.h"

/**
 *  Ready-to-use weapons built from the assets already in the project.
 *  To tweak one without touching C++: right-click the class in the Content Browser (C++ Classes > MPShooter)
 *  > Create Blueprint Class, change the values, then put the Blueprint in the character's Default Weapons list.
 */

/** Slot 1: automatic rifle (Fab "Classic M4") */
UCLASS()
class AMPWeapon_M4 : public AMPWeapon
{
	GENERATED_BODY()

public:
	AMPWeapon_M4();
};

/** Slot 2: semi-automatic pistol (Fab "Pistol 9mm Custom") */
UCLASS()
class AMPWeapon_Pistol : public AMPWeapon
{
	GENERATED_BODY()

public:
	AMPWeapon_Pistol();
};

/** Slot 3: hard hitting automatic rifle (Fab "Soviet Assault Rifle") */
UCLASS()
class AMPWeapon_AK47 : public AMPWeapon
{
	GENERATED_BODY()

public:
	AMPWeapon_AK47();
};
