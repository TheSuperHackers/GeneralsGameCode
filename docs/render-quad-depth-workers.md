# GFX 11 parallel billboard depth preparation

The sorting renderer can use worker threads to calculate depth keys for transparent billboard quads during large batches. It uses the existing sort and draw order and preserves the mixed quad and triangle entry count fixed in PR #26. This is an adaptation of the render fork/join approach in [CnCGeneralsZH-Reforged](https://github.com/olcayseygan/CnCGeneralsZH-Reforged), by Olcay Seygan and contributors.

The change is in `Core/Libraries/Source/WWVegas/WW3D2/sortingrenderer.cpp` and applies to the shared Generals and Zero Hour renderer. Workers read CPU vertex data and write to disjoint entries. The render thread releases the GPU buffer lock before dispatch and waits for workers before sorting or recycling buffers. Renderer shutdown joins the workers. Game INI, assets, saves, replays, network protocol, and simulation formats are unchanged.

## Enable and reproduce

The game defaults to eight additional render workers. To change the count for one PC, set `RenderWorkers` in its user-data `Options.ini` and restart the game. For Zero Hour, this is normally `Documents/Command and Conquer Generals Zero Hour Data/Options.ini`. The setting is read when the game's graphical display initializes.

```ini
RenderWorkers = 8
```

Use `0` for serial preparation or `1`–`8` for additional workers. Numeric values are clamped to that range. A missing or invalid setting uses eight. The process variable `GENERALS_RENDER_WORKERS` overrides the INI for A/B testing when it contains a valid integer, including zero. The render thread also performs work. A batch needs at least five billboard jobs and 4096 quads before workers are started or used. Smaller batches and triangle depth calculations run on the render thread.

`Options.ini` is a local preference file. Game data INI, assets, saves, replays, and network formats are unchanged.

Build the Win32 Release target `z_generals`. Run the same particle-heavy replay twice at the same game time and camera angle, once with `GENERALS_RENDER_WORKERS=0` and once with `GENERALS_RENDER_WORKERS=4`. Compare transparency ordering while panning, rotating, and switching between normal zoom and overview. Return to the menu, replay the scene again, and verify clean game exit. For multiplayer, run the same build on both PCs and set different worker counts before launch.

## Profile worker participation (GFX 11b)

Build the Win32 Release target with `RTS_BUILD_OPTION_PROFILE_TRACY=ON` and `TRACY_NO_CALLSTACK=ON`. The second option keeps the Win32 build compatible with the legacy `dbghelp.dll` shipped with the game, which lacks `SymLoadModuleEx`. The capture still includes zone timings, frame markers, and the worker plot. For a local capture, use `TRACY_ON_DEMAND=ON`, `TRACY_ONLY_LOCALHOST=ON` and `TRACY_NO_BROADCAST=ON`. Use [Tracy 0.13.1](https://github.com/wolfpld/tracy/releases/tag/v0.13.1), the version pinned by the project's CMake configuration. Connect to the local game process before the replay segment you want to measure.

Run the same particle-heavy replay at the same speed, resolution, camera position and zoom. Restart the game between runs with `GENERALS_RENDER_WORKERS` set to `0`, `4` and `8`. Compare the same interval after the scene has warmed up.

- `SortingRenderer::PrepareDepthEntries` measures render-thread elapsed time for preparing each sorting batch, including the vertex buffer work and any wait for workers.
- `SortingRenderer::FillQuadDepth` measures depth computation by billboard job on the render thread and on workers. Worker zones can overlap, so their durations do not add up to elapsed time.
- `SortingRenderer::Flush` measures the complete sorting renderer flush.
- `SortingRenderer/Worker-assisted quad batches` counts batches since game launch in which a worker finished at least one nonempty quad-depth job. It stays at zero in serial mode. A flat plot with workers configured means they did no work during that interval. A dense scene is needed to test the parallel path.

For GFX 11c captures, the additional `SortingRenderer/Quads per batch` and `SortingRenderer/Nonempty quad jobs per batch` plots include serial batches, even when the initial dispatch gate is not met. Compare these distributions with the existing worker-assisted count. For controlled experiments only, set `GENERALS_RENDER_MIN_QUADS` to an integer from 256 to 65536 before launch (default 4096; missing or invalid values retain the default). The five-job gate and worker-count configuration are unchanged. Compare 4096, 2048, 1024 and 512 with the same replay segment and worker counts; use 0 workers as the serial reference. Do not lower the shipping default until matched measurements show a benefit.

Compare frame time in the same replay window as well as time spent in the renderer. Keep the profiler connected in every run. The game's existing Tracy frame images can introduce periodic capture overhead. This profiling setup does not measure standalone GPU execution or change the worker defaults.

Set `RTS_BUILD_OPTION_PROFILE_TRACY=OFF` and rebuild to remove the instrumentation from the executable. The diagnostics introduce no game-data or network format changes.

## Validation

- Win32 Release `z_generals` compiled and was installed for the Contra Invasion test.
- A stand-alone Win32 check compared serial and parallel depth entries bit for bit at 1, 2, and 8 workers using two floating-point control modes.
- The tester reported 10 minutes of multiplayer with two PCs and two bots, configured for 8 and 4 render workers respectively, with no observed desynchronization. Worker participation during the match was not measured.
- The test did not benchmark frame time. The original Generals target was outside this build.

## Roll back

Set `RenderWorkers = 0`, clear any environment override, and restart the game to use serial depth preparation. You can also launch once with `GENERALS_RENDER_WORKERS=0`. To restore the previous build entirely, restore its executable or revert the applicable changes and rebuild.
