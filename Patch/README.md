# Custom Hotkey Remapping - Installation

This patch adds in-game hotkey remapping for unit/building production keys per faction (USA, China, GLA).

## Installation

After building the game from source, copy the `.wnd` files to your Zero Hour installation:

```
Patch/Window/Menus/KeyboardOptionsMenu.wnd  →  <ZH Install>/Window/Menus/KeyboardOptionsMenu.wnd
Patch/Window/Menus/OptionsMenu.wnd          →  <ZH Install>/Window/Menus/OptionsMenu.wnd
```

Replace `<ZH Install>` with your Zero Hour directory, for example:
```
C:\Program Files (x86)\EA Games\Command and Conquer Generals Zero Hour\
```

The loose `.wnd` files will override the ones inside `WindowZH.big` automatically.

## What it does

- Adds a **Keyboard Options** button to the Options menu
- Opens a new menu with:
  - **Category dropdown** (Control, Information, Interface, ... + Unit Keys: USA/China/GLA/Other)
  - **Command list** showing all hotkeys for the selected category
  - **Description and current key** display
  - **Text input** to type a new key
  - **Assign** and **Reset All** buttons
- Hotkey overrides are saved per CommandButton name and persist across sessions
