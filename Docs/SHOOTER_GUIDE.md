# MPShooter: multiplayer shooter guide

This guide covers the C++ shooter code in `Source/MPShooter` and `Source/MPShooter/Shooter`:

- **Camera:** third-person over-the-shoulder view, plus first-person view with arms (**V** switches).
- **Weapons:** M4 (slot 1), 9mm pistol (slot 2) and AK-47 (slot 3), with hitscan shooting, spread, recoil, ammo and reload.
- **Effects:** muzzle flash, shell ejection, impact effects and sounds.
- **Multiplayer:** the server checks every shot and decides hits, damage, headshots (×2), death (ragdoll) and respawn.
- **HUD:** crosshair, hit marker, health, ammo, kill feed and scoreboard. No widgets are needed.

## 1. Build and run

1. Close the Unreal Editor.
2. Right-click `MPShooter.uproject` → **Generate Visual Studio project files**.
3. Open `MPShooter.sln` and build **Development Editor / Win64** (or press F5).
   - Alternatively, open the editor and let it rebuild the modules when it asks.
4. Open `Lvl_ThirdPerson` and press **Play**.

If the build fails, copy the **first** error from the Output window (it looks like `error C2039: ...`) and send it to Claude.

## 2. Test multiplayer in the editor

1. Click the **⋮** menu next to the Play button.
2. Set **Number of Players** to `2`.
3. Set **Net Mode** to **Play As Listen Server**.
4. Press **Play**. Two windows open, and each one is a player.
5. To get several spawn points, drag extra **Player Start** actors into the level (Quickly add → Basic → Player Start). Players respawn at the start farthest from the other players.

## 3. Controls

| Action | Keyboard / mouse | Gamepad |
|---|---|---|
| Fire | Left mouse | Right trigger |
| Aim (zoom) | Right mouse | Left trigger |
| Reload | R | X / Square |
| First / third person | V | Right stick click |
| Weapons | 1 / 2 / 3, mouse wheel | Y / Triangle |

The input actions are created in code (`AMPShooterPlayerController::EnsureShooterInput`). If you want your own Input Action assets, assign them on `BP_ThirdPersonPlayerController` → **Input | Shooter** and map their keys in an Input Mapping Context.

## 4. Line up the guns in the hands (important)

The Fab guns are Static Meshes, and each one has its own size, rotation and pivot, so they need a one-time alignment.

1. Press **Play**, then press **`** (the key under Esc) to open the console.
2. Third-person gun in the body's hand:
   ```
   MPGunTP  X Y Z  Pitch Yaw Roll  Scale
   MPGunTP  0 0 0  0 90 0  1
   ```
3. First-person gun in the arms' hand (press **V** first):
   ```
   MPGunFP  0 0 0  0 90 0  1
   ```
4. The first-person arms relative to the camera (default `-30 0 -150 0 0 0`):
   ```
   MPArms  -30 0 -150  0 0 0
   ```
5. Change the numbers until the gun looks right. Each command prints the values on screen.
6. Make the values permanent with either of these:
   - **Blueprint:** Content Browser → **C++ Classes/MPShooter** → right-click `MPWeapon_M4` → **Create Blueprint Class**. Set **Weapon | Mesh → Third Person Offset / First Person Offset**. Then open `BP_ThirdPersonCharacter` → **Shooter | Weapons → Default Weapons** and replace `MPWeapon_M4` with your Blueprint.
   - **C++:** ask Claude to put the numbers in `Source/MPShooter/Shooter/MPWeaponPresets.cpp`.

Tip: if a gun is huge or tiny, the last number (`Scale`) fixes it, for example `0.01` or `100`.

## 5. Where things are

| File | What it does |
|---|---|
| `MPShooterCharacter` | Cameras, first-person arms, inventory, health, death, animations |
| `Shooter/MPWeapon` | Base weapon: firing, server checks, damage, ammo, reload, effects |
| `Shooter/MPWeaponPresets` | M4, pistol and AK-47: mesh, damage, fire rate, sounds, effects |
| `MPShooterPlayerController` | Input actions and keys, kill feed and hit marker data, gun tuning console commands |
| `MPShooterGameMode` | Kills and deaths, kill feed, respawn, spawn point choice |
| `Shooter/MPShooterPlayerState` | Replicated kills and deaths |
| `Shooter/MPShooterHUD` | Crosshair, hit marker, health, ammo, kill feed, scoreboard |
| `Shooter/MPShooterAnimInstance` | Optional parent class for your own Anim Blueprints |

### Assets used by default

- **Guns:** `Fab/Classic_M4`, `Fab/Pistol_9mm_Custom`, `Fab/Soviet_Assault_Rifle`
- **Body animations:** `Characters/Mannequins/Anims/Rifle` and `.../Pistol` (idle, 8-direction jog, fall, fire, reload, equip, hit reacts)
- **First-person arms and animations:** `MuzzleFlash/Demo/FirstPersonArms` (`SK_Mannequin_Arms`, `FP_Rifle_*`)
- **Muzzle flash, shell and smoke:** `NW_MuzzleFX/Particle_FX`
- **Sounds:** `NW_MuzzleFX/Sound/Sfx`, `Free_Sounds_Pack/cue`

If you move or rename these assets in the editor, update the paths in `MPWeapon.cpp`, `MPWeaponPresets.cpp` and `MPShooterCharacter.cpp`, or set the assets in a Blueprint child.

## 6. How a shot works over the network

1. **The shooter's PC** traces from the camera at once, so the crosshair is accurate, and plays the flash, sound and impact without waiting. It then calls `ServerFire`.
2. **The server** checks the shot:
   - the shooter is alive
   - it is the weapon in hand and there is ammo
   - the fire rate is not exceeded
   - the shot starts near the shooter

   It then traces again and applies the damage. A hit on the `head` bone does ×2 damage.
3. **Everyone else** gets `MulticastFireEffects` and plays the flash, sound and impact on the shooter's third-person gun.
4. **The shooter** gets `ClientConfirmHit`, which shows the hit marker. The marker is red for a kill or a headshot.

Health, death, ammo and equipped weapon all live on the server and replicate to clients.

## 7. Animation modes

- **Code driven (default, works out of the box).** The C++ code picks the animation from the weapon:
  - idle, jog in 8 directions, or fall
  - fire, reload, equip and hit animations when standing still

  There is no blending between animations, so changes are instant.
- **Anim Blueprint (smoother, optional).** Set this up as follows:
  1. Create an Anim Blueprint for `SK_Mannequin` and set its **Class Settings → Parent Class** to `MPShooterAnimInstance`.
  2. Use `Speed`, `Direction`, `AimPitch`, `bIsInAir`, `WeaponType` and the other variables in its graph, for example in a blend space for the rifle jogs plus `AO_Rifle` for aiming up and down.
  3. Make sure the graph has a `DefaultSlot` slot node.
  4. Assign it to the character's Mesh, then untick **Shooter | Animation → Use Code Driven Animation** on `BP_ThirdPersonCharacter`.

  With this setup, the C++ code only plays fire, reload, equip and hit animations, in `DefaultSlot`.

## 8. Known limits and next steps

- **No lag compensation:** against high-ping players, fast targets can be hit or missed slightly differently than they looked.
- **No first-person pistol animations:** the pistol reuses the first-person rifle animations.
- **No ammo pickups yet:** you respawn with full ammo.
- **Possible next features:** team deathmatch, a main menu with Steam/LAN sessions, grenades, ammo pickups, crouch and sprint.
