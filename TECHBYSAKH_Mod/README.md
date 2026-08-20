# TECHBYSAKH Modified Mod Package

**TECHBYSAKH Modified - Community Edition** is the branded expansion surface for this Generals/Zero Hour code fork. The executable-side changes improve timing, LAN responsiveness, GPU-aware detail selection, and visible identity. This folder carries the data-driven content plan that can be paired with a compatible game-patch/assets repository.

> The original game installation and the original game data remain required. This package does not redistribute EA assets.

## Included

| Component | Purpose |
| --- | --- |
| `Branding/techbysakh_emblem.png` | Original TECHBYSAKH shield-and-circuit emblem for menus, launchers, documentation, or a future texture conversion. |
| `Modes/TECHBYSAKH_Modes.ini` | Stable names and rules for the new lobby/mode concepts. |
| `Maps/MAP_AUTHORING.md` | Authoring rules for maps that remain compatible with synchronized lockstep play. |
| `Maps/TECHBYSAKH_FRONTLINE.md` | Competitive team-assault map brief. |
| `Maps/TECHBYSAKH_COOPERATIVE_CANYON.md` | Cooperative mission map brief with shared objectives. |
| `Docs/LAN_PLAY.md` | Local-network host/join instructions and troubleshooting. |

## Play concepts

**Co-op Mission** gives two to four players a shared objective chain, staged enemy reinforcements, limited starting resources, and a clear extraction or survival condition. **Team Assault** uses two allied teams with shared victory conditions, strategic chokepoints, and a central resource objective. **Free-for-All Blitz** is a shorter match with accelerated income, a compact map footprint, and an explicit time limit so it is easy to replay with a group of friends.

These are intentionally expressed as data and authoring contracts first. Generals/Zero Hour map logic is driven by map scripts and INI data, so the final playable missions belong in the compatible data/assets repository rather than being embedded into the C++ executable as an unsafe parallel rules engine.

## Build and use

Build the executable with the branded profile enabled, install the resulting binaries beside a supported Generals/Zero Hour installation, and copy the compatible data pack into the game’s normal mod/data location. Start the game, select **LAN**, and use the same map/data version on every machine. For a host, allow the game through the local firewall and create the game from the LAN lobby. For clients, use LAN discovery or the host’s local IP through direct connect.

## Branding usage

The TECHBYSAKH name is part of the executable product identity and credits in branded builds. The emblem is an original project asset and may be used in a launcher, readme, mod splash, or converted texture where the asset pipeline requires a different format. Keep the branding visible but do not replace required original game legal notices.
