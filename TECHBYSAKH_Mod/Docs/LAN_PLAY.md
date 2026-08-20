# TECHBYSAKH LAN Play

## Host

Install the same executable, data pack, map files, and INI files on every machine. Start the host game, open the LAN lobby, create a named game, select a TECHBYSAKH mode-compatible map, and wait for the other players to appear. Allow the game executable through the operating system firewall on the private network profile. If discovery does not show the host, use direct connect with the host’s private IPv4 address.

## Join

Join from the LAN list or connect directly to the host’s private IPv4 address. Every player must have matching map and data files. If the map is missing, the host can use the built-in transfer path for the supported extensions, but distributing a large map pack before the session is more reliable.

## Shared session rules

For a fair synchronized match, use one shared game version and one shared mode manifest. Do not mix stock and modified INI files. Keep the machine that hosts the lobby online until the match finishes, and avoid changing the network adapter while the game is running.

## Troubleshooting

| Symptom | Action |
| --- | --- |
| Host is not visible | Confirm all machines are on the same private subnet, allow the executable through the firewall, then try direct connect. |
| Join is denied for a map | Compare map/data files and their CRCs; install the same companion pack on all clients. |
| Chat or ready state feels delayed | Use the TECHBYSAKH branded build, which pumps the LAN lobby more frequently than the upstream fallback. |
| Match stutters | Set a reasonable render FPS cap, close GPU-heavy overlays, and keep the game on a local SSD. The new render limiter does not alter the synchronized logic rate. |
| A player drops | Rejoin only after the current game is closed; the underlying lockstep connection manager still owns disconnect and router fallback behavior. |
