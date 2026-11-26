# Dynamic radius cursors (first stage)

This change adds optional, named **terrain radius decals** for General Powers and Super Weapons in Generals and Zero Hour. It does not change the fixed mouse-pointer enum or gameplay simulation.

## INI usage

Create `Data/INI/RadiusCursor.ini` (or INI files in `Data/INI/RadiusCursor/`). The files are optional and loaded before CommandButton definitions, including in the World Builder ControlBar initialization path.

```ini
RadiusCursor CUSTOM_ION_CANNON
    Texture = SCCFuelAirBomb_USA
    Style = SHADOW_ALPHA_DECAL
    OpacityMin = 50%
    OpacityMax = 100%
    OpacityThrobTime = 1000
    Color = R:100 G:180 B:255 A:255
    OnlyVisibleToOwningPlayer = Yes
End
```

Replace the example texture with an available decal texture. In an existing CommandButton for a targeted special power, use:

```ini
RadiusCursorType = CUSTOM_ION_CANNON
```

The associated SpecialPower definition continues to supply the radius through its existing `RadiusCursorRadius` field.

### Compatibility rules

- No new INI file is required for old mods.
- Original cursor names, numeric values and InGameUI.ini fields are unchanged.
- Custom names are case-insensitive; a repeated custom definition overrides its existing entry without allocating another ID.
- Built-in names are reserved and must still be configured through the existing InGameUI fields.
- An unknown reference and a new definition with no texture name are INI errors.
- Custom IDs are UI-local and assigned after the original enum range. They must not be used as serialized network or savegame identifiers.
- This stage supports custom radius decals for special powers. Custom weapon-derived radius sources and dynamically registered mouse pointers are out of scope.

## Validation checklist (not yet executed)

1. Build both game targets and World Builder.
2. Start an unmodified game without RadiusCursor.ini; compare all legacy cursor styles and radii with baseline.
3. Define two custom decals with different textures/colors, attach them to two existing targeted abilities, and test both shortcut and unit command buttons.
4. Exercise a large definition set (e.g. 256 and 1024 entries), including the last ID and case-insensitive references.
5. Test repeated definitions, unknown names, missing texture names, and attempts to redefine built-ins.
6. Test switching abilities, cancelling targeting, replay/spectator mode, map reload, control bar recreation, and shutdown; check for stale template pointers.
7. Load existing mod INIs and maps in World Builder, including a project using a new custom radius cursor.
8. Perform multiplayer and replay comparison with identical gameplay actions; confirm no additional simulation RNG calls or divergent simulation CRC.
9. Verify actual texture presence and rendering; the parser currently validates the texture name is nonempty, not the underlying asset.
10. Review cursor references across the full repository and check same-cursor/different-special-power radius transitions (existing cursor caching behavior remains).
