module;

// The legacy GameClient headers remain source-compatible ABI boundaries for
// the game and renderer targets. New WND code consumes the module surface
// below instead of reaching through those headers directly.

export module Engine.UI.WND.Runtime;

export import Engine.UI.WND;
export import Engine.UI.WND.Controls;
export import Engine.UI.WND.Document;
export import Engine.UI.WND.Layout;
