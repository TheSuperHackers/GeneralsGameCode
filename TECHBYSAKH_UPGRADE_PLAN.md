# TECHBYSAKH Upgrade Plan

## Product direction

This fork will present itself as **TECHBYSAKH Modified — Community Edition** while retaining the original Generals/Zero Hour compatibility model. The branding will be visible in version/product strings, build metadata, documentation, and the in-game credits path where the existing engine exposes one.

## Priority order

| Priority | Upgrade | Implementation boundary | Verification |
| --- | --- | --- | --- |
| 1 | Build identity and TECHBYSAKH touch | Shared version/product helpers and CMake metadata; no replacement of EA assets | Product-string unit checks and source audit |
| 2 | Smoother frame pacing | Improve the existing high-resolution limiter to avoid accumulated drift and oversleep jitter; keep deterministic network logic independent | Static compile check plus timing helper tests |
| 3 | GPU-ready graphics profile | Add explicit graphics-profile/build switches and an extensible runtime capability description around the existing W3D/D3D8 path; do not falsely claim a D3D11 renderer without the missing renderer implementation | CMake/config audit and safe default behavior |
| 4 | LAN play surface | Add a compatibility-preserving LAN session profile and clearer host/join metadata, using the existing broadcast/direct-connect APIs rather than replacing the lockstep transport | Header/source checks and LAN option documentation |
| 5 | New modes and maps | Add a data/mod integration pack with cooperative objective concepts, team-versus-team presets, and new-map authoring templates. This is intentionally data-driven so it can be used with the separate game-patch/assets repository | Schema/data validation and install instructions |
| 6 | Build and release workflow | Add CI/static checks, release metadata, and packaging documentation for the 32-bit Windows binary path | CMake/configure when toolchain is available; otherwise deterministic checks |

## Explicit technical boundary

The repository already uses a GPU-backed Direct3D 8-compatible renderer through the W3D layer, but the active source tree does not contain a complete modern D3D11/Vulkan backend. Therefore this change will improve the existing GPU path, rendering configuration, and frame pacing without claiming that a full renderer replacement has been completed. A true modern backend remains a separate high-risk project requiring a renderer rewrite, asset/material conversion, and extensive compatibility testing.

Likewise, the current LAN transport already supports broadcast discovery, direct connect, map transfer, chat, and synchronized lockstep gameplay. The safe first improvement is to make that session flow more discoverable and configurable, not to introduce a second unsynchronized netcode path that could invalidate replays or retail compatibility.

## Data pack concept

The repository will include a `TECHBYSAKH_Mod` documentation/data skeleton defining three modes: **Co-op Mission**, **Team Assault**, and **Free-for-All Blitz**. It will provide map naming conventions, lobby option fields, objective vocabulary, and a map authoring checklist so actual `.map`/`.ini` assets can be added without coupling them to the executable.
