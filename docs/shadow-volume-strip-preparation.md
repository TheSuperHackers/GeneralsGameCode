# GFX 12a static shadow-volume strip preparation

Generals and Zero Hour now use a shared CPU routine to order silhouette edges into strips and calculate the vertex and triangle counts for static volume shadows. The existing prepass now has explicit inputs and outputs and can be verified independently of D3D8. It prepares the shadow silhouette before either video buffer is allocated or locked.

`Core/GameEngineDevice/Include/W3DDevice/GameClient/W3DShadowVolumeStrips.h` contains the strip preparation. Both games call it from `W3DVolumetricShadow::constructVolumeVB`. The vertex and index writes, stencil drawing, and dynamic volume path use their existing code. The pass is serial. Game INI, assets, saves, replays, and network formats are unchanged.

## Test in Zero Hour

1. Build and run the Win32 Release target `z_generals` with volume shadows enabled. Use a skirmish or replay with both static buildings and moving vehicles. Compare the building shadows with the previous build at the same game time and camera angle.
2. Pan and rotate around the structures, zoom out to overview and back, and watch for missing, flipped, or flickering silhouettes. Keep an animated or airborne unit in view to cover the existing dynamic path.
3. Switch away from the game and return in the same match, then start another match. Check that shadows appear as the camera returns to the scene.

## Roll back

Restore the previous executable, or revert the GFX 12a commit and rebuild.
