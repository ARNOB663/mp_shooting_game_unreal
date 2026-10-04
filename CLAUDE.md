# Push guide for AI assistants

Unreal Engine 5.8 C++ project (`MPShooter`, Third Person template). Read this before committing or pushing.

## Never commit
- `Binaries/`, `Intermediate/`, `Saved/`, `DerivedDataCache/`, `.vs/`, `*.sln` (already in `.gitignore`)
- Raw downloads (`.glb`, `.fbx`, `.zip`) — keep them outside the project; only the imported `.uasset` files go in `Content/`

## Before pushing
1. Make sure Git LFS is active: `git lfs install` (once per machine).
2. Check that binaries are LFS pointers, not full files: `git lfs ls-files` should list every new `.uasset` / `.umap`.
3. Stage only what changed and belongs together, e.g. `git add Content/Weapons/L85` or `git add Source/`.
4. Never `git add -A` after an editor session without reviewing `git status` first.
5. GitHub rejects any non-LFS file over 100 MB. Free LFS has ~1 GB storage/bandwidth — keep textures at 2048 max.

## Commit and push
```
git status
git add <paths>
git commit -m "Short summary of the change"
git push -u origin <branch>
```
- One topic per commit (e.g. "Add L85 rifle", "Add FP/TP camera toggle").
- Don't push to `main` directly from an AI session; use a feature branch and let the owner merge.
- Never force-push `main` or rewrite pushed history.
- If a push fails with a network error, retry with backoff (2s, 4s, 8s, 16s).

## Folder layout
- C++: `Source/MPShooter/`
- Weapons: `Content/Weapons/<GunName>/`
- Third-party packs stay in the folder Fab created (e.g. `Content/Free_Sounds_Pack/`); move assets only inside the Unreal editor, never in Explorer.

## Can't build here?
Cloud AI sessions can't run Unreal. Say so, keep C++ changes small, and ask the owner to compile and report errors.

## Shooter code
See `Docs/SHOOTER_GUIDE.md` for how the weapons, networking, animation and HUD code fit together, and the asset paths it uses.
