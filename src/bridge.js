(() => {
  "use strict";
  const config = __CONFIG__;
  const id = "obs-twitch-dock-themes-style";
  const host = location.hostname.toLowerCase();
  const allowed = location.protocol === "https:" &&
    (host === "www.twitch.tv" || host === "twitch.tv" || host === "dashboard.twitch.tv") &&
    /^\/popout\//.test(location.pathname);
  const existing = document.getElementById(id);
  if (!allowed) {
    if (existing) existing.remove();
    return "outside-scope";
  }
  if (!config.enabled) {
    if (existing) existing.remove();
    return "removed";
  }
  const p = config.palette;
  const required = ["base", "window", "button", "hover", "input", "selection",
    "accent", "accentHover", "text", "muted", "onAccent", "border", "link"];
  if (!p || !required.every(k => typeof p[k] === "string" && /^#[0-9a-f]{6}$/i.test(p[k])))
    return "invalid-palette";
  const spacing = new Map([["compact", ["2px", "4px"]], ["comfortable", ["8px", "10px"]]]).get(config.density);
  const densityCss = spacing ? `
.chat-line__message { padding-top: ${spacing[0]} !important; padding-bottom: ${spacing[0]} !important; }
.chat-input { padding: ${spacing[1]} !important; }
textarea:not([aria-invalid="true"]), input:is([type="text"], [type="search"], [type="number"], [type="url"]):not([aria-invalid="true"]) {
  padding-top: ${spacing[1]} !important; padding-bottom: ${spacing[1]} !important;
}` : "";
  const css = `
${densityCss}
:root, .tw-root--theme-dark, .tw-root--theme-light {
  --color-background-base: ${p.base} !important;
  --color-background-body: ${p.window} !important;
  --color-background-alt: ${p.button} !important;
  --color-background-alt-2: ${p.hover} !important;
  --color-background-float: ${p.button} !important;
  --color-background-input: ${p.input} !important;
  --color-background-input-focus: ${p.base} !important;
  --color-background-interactable-hover: ${p.hover} !important;
  --color-background-interactable-active: ${p.selection} !important;
  --color-background-interactable-selected: ${p.selection} !important;
  --color-background-button-primary-default: ${p.accent} !important;
  --color-background-button-primary-hover: ${p.accentHover} !important;
  --color-background-button-primary-active: ${p.accent} !important;
  --color-background-button-secondary-default: ${p.button} !important;
  --color-background-button-secondary-hover: ${p.hover} !important;
  --color-background-button-secondary-active: ${p.selection} !important;
  --color-background-button-text-hover: ${p.hover} !important;
  --color-background-button-text-active: ${p.selection} !important;
  --color-text-base: ${p.text} !important;
  --color-text-alt: ${p.muted} !important;
  --color-text-alt-2: ${p.muted} !important;
  --color-text-input: ${p.text} !important;
  --color-text-input-placeholder: ${p.muted} !important;
  --color-text-button-primary: ${p.onAccent} !important;
  --color-text-button-secondary: ${p.text} !important;
  --color-text-link: ${p.link} !important;
  --color-text-link-hover: ${p.link} !important;
  --color-text-link-active: ${p.link} !important;
  --color-text-button-text: ${p.text} !important;
  --color-text-button-disabled: ${p.muted} !important;
  --color-border-base: ${p.border} !important;
  --color-border-input: ${p.border} !important;
  --color-border-input-focus: ${p.accent} !important;
  --color-border-interactable-focus: ${p.accent} !important;
  --color-border-interactable-selected: ${p.accent} !important;
  --border-radius-small: 4px;
  --border-radius-medium: 8px;
  --border-radius-large: 10px;
}
html, body { background-color: ${p.window} !important; }
/* Structural containers only; no blanket foreground rule on chat content. */
.chat-room, .stream-chat, .chat-shell, .chat-input {
  background-color: ${p.base} !important;
}
textarea:not([aria-invalid="true"]),
input:is([type="text"], [type="search"], [type="number"], [type="url"]):not([aria-invalid="true"]) {
  background-color: ${p.input} !important;
  color: ${p.text} !important;
  border-color: ${p.border} !important;
  border-radius: 8px !important;
}
textarea:not([aria-invalid="true"]):focus-visible,
input:is([type="text"], [type="search"], [type="number"], [type="url"]):not([aria-invalid="true"]):focus-visible {
  outline: 2px solid ${p.accent};
  outline-offset: -2px;
}
/* The chat container owns the border and focus ring. Its editor is a transparent layer. */
.chat-input textarea:not([aria-invalid="true"]),
.chat-input [contenteditable="true"]:not([aria-invalid="true"]),
[data-a-target="chat-input"]:is(textarea, [contenteditable="true"]):not([aria-invalid="true"]) {
  background: transparent !important;
  border: 0 !important;
  border-radius: 0 !important;
  box-shadow: none !important;
  outline: none !important;
  color: ${p.text} !important;
}
[data-a-target="chat-send-button"] {
  background-color: ${p.accent} !important;
  color: ${p.onAccent} !important;
  border-radius: 8px !important;
}
[data-a-target="chat-send-button"]:hover:not(:disabled) {
  background-color: ${p.accentHover} !important;
}
[data-a-target="chat-send-button"]:disabled {
  background-color: ${p.button} !important;
  color: ${p.muted} !important;
}
.chat-line__message:hover { background-color: ${p.hover}; }
${config.motion ? `
@media (prefers-reduced-motion: no-preference) {
  .chat-line__message, [data-a-target="chat-send-button"] {
    transition: background-color 110ms ease-out;
  }
}
` : ""}
`;
  if (existing && existing.textContent === css) return "unchanged";
  const target = document.head || document.documentElement;
  if (!target) return "waiting-for-document";
  const style = existing || document.createElement("style");
  style.id = id;
  style.textContent = css;
  if (!existing) target.appendChild(style);
  return "applied";
})();
