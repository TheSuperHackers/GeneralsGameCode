# GFX 11 parallel billboard depth preparation

The sorting renderer can use worker threads to calculate depth keys for transparent billboard quads during large batches. It uses the existing sort and draw order and preserves the mixed quad and triangle entry count fixed in PR #26. This is an adaptation of the render fork/join approach in [CnCGeneralsZH-Reforged](https://github.com/olcayseygan/CnCGeneralsZH-Reforged), by Olcay Seygan and contributors.

The change is in `Core/Libraries/Source/WWVegas/WW3D2/sortingrenderer.cpp` and applies to the shared Generals and Zero Hour renderer. Workers read CPU vertex data and write to disjoint entries. The render thread releases the GPU buffer lock before dispatch and waits for workers before sorting or recycling buffers. Renderer shutdown joins the workers. Game INI, assets, saves, replays, network protocol, and simulation formats are unchanged.

## Enable and reproduce

Set the process environment variable `GENERALS_RENDER_WORKERS` before starting the game. It accepts 1–8 additional workers, capped at 8. An unset value or zero selects the default serial path. A batch needs at least five billboard jobs and 4096 quads to qualify for parallel execution. Smaller batches and triangle depth calculations run on the render thread.

Build the Win32 Release target `z_generals`. Run the same particle-heavy replay twice at the same game time and camera angle, once with `GENERALS_RENDER_WORKERS=0` and once with `GENERALS_RENDER_WORKERS=4`. Compare transparency ordering while panning, rotating, and switching between normal zoom and overview. Return to the menu, replay the scene again, and verify clean game exit. For multiplayer, run the same build on both PCs and set different worker counts before launch.

## Validation

- Win32 Release `z_generals` compiled and was installed for the Contra Invasion test.
- A stand-alone Win32 check compared serial and parallel depth entries bit for bit at 1, 2, and 8 workers using two floating-point control modes.
- The tester reported 10 minutes of multiplayer with two PCs and two bots, configured for 8 and 4 render workers respectively, with no observed desynchronization. Worker participation during the match was not measured.
- The test did not benchmark frame time. The original Generals target was outside this build.

## Roll back

Unset `GENERALS_RENDER_WORKERS` or set it to `0` and restart the game to use serial depth preparation. To restore the previous build entirely, restore its executable or revert the integration of PR #18 and rebuild.
