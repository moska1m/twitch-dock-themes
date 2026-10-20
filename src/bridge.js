(() => {
  "use strict";
  const config = __CONFIG__;
  const id = "obs-twitch-dock-themes-style";
  const host = location.hostname.toLowerCase();
  const allowed = location.protocol === "https:" &&
    (host === "www.twitch.tv" || host === "twitch.tv" || host === "dashboard.twitch.tv") &&
    /^\/popout\//.test(location.pathname);
  const existing = document.getElementById(id);
  const effectsKey = "__obsTwitchDockThemeEffects";
  const clearEffects = () => {
    if (typeof window !== "undefined" && window[effectsKey]?.version === 1) {
      window[effectsKey].dispose(); delete window[effectsKey];
    }
  };
  if (!allowed) {
    clearEffects();
    if (existing) existing.remove();
    return "outside-scope";
  }
  if (!config.enabled) {
    clearEffects();
    if (existing) existing.remove();
    return "removed";
  }
  const p = config.palette;
  const required = ["base", "window", "button", "hover", "input", "selection",
    "accent", "accentHover", "text", "muted", "onAccent", "border", "link"];
  if (!p || !required.every(k => typeof p[k] === "string" && /^#[0-9a-f]{6}$/i.test(p[k])))
    return "invalid-palette";
  const spacing = new Map([["compact", ["2px", "4px"]], ["comfortable", ["8px", "10px"]]]).get(config.density);
  const isChat = config.panel === "twitchChat" && /\/chat\/?$/.test(location.pathname);
  const isFeed = config.panel === "twitchFeed" && /\/activity-feed\/?$/.test(location.pathname);
  const chatDensity = spacing && isChat ? `
.chat-line__message { padding-top: ${spacing[0]} !important; padding-bottom: ${spacing[0]} !important; }
.chat-input { padding: ${spacing[1]} !important; }
.chat-input textarea { padding-top: ${spacing[1]} !important; padding-bottom: ${spacing[1]} !important; }
` : "";
  // Stable activity classes verified in Twitch's public activity-feed bundle.
  const compact = config.density === "compact";
  const feedDensity = spacing && isFeed ? `
.activity-list-layout .activity-base-list-item { padding-left: ${compact ? "6px" : "16px"} !important; padding-right: ${compact ? "6px" : "16px"} !important; }
.activity-list-layout .activity-base-list-item > div { padding-top: ${compact ? "4px" : "12px"} !important; padding-bottom: ${compact ? "4px" : "10px"} !important; }
.activity-list-layout .activity-base-list-item :is(p, span) { font-size: ${compact ? "12px" : "14px"} !important; line-height: ${compact ? "1.3" : "1.5"} !important; }
.activity-list-layout .activity-base-list-item__title { min-width: 0 !important; }
.activity-list-layout .activity-base-list-item__title p { word-break: normal !important; overflow-wrap: anywhere !important; }
.activity-list-layout .activity-base-list-item__subtitle { padding-left: ${compact ? "0" : "24px"} !important; }
.activity-list-layout .activity-base-list-item__icon { margin-right: ${compact ? "4px" : "10px"} !important; }
` : "";
  const densityCss = chatDensity + feedDensity;
  const raw = config.visual || {};
  const visual = {
    icons: ["native", "thin", "rounded"].includes(raw.icons) ? raw.icons : "native",
    feed: ["native", "compact", "cards"].includes(raw.feed) ? raw.feed : "native",
    focus: raw.focus === true, alerts: raw.alerts === true,
    raids: raw.raids !== false, subscriptions: raw.subscriptions !== false, mentions: raw.mentions !== false,
    duration: Number.isInteger(raw.duration) && raw.duration >= 1 && raw.duration <= 8 ? raw.duration : 3
  };
  const focusCss = isChat && visual.focus ? `
.stream-chat-header { max-height: 10px; min-height: 10px !important; overflow: hidden; opacity: .25; }
.stream-chat-header:hover, .stream-chat-header:focus-within { max-height: 100px; min-height: 40px !important; overflow: visible; opacity: 1; }
` : "";
  // Only control buttons: never badges, emotes, or content icons.
  const iconSvg = path => `url("data:image/svg+xml,${encodeURIComponent(`<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 24 24" fill="none" stroke="black" stroke-width="${visual.icons === "thin" ? 1.5 : 2.2}" stroke-linecap="round" stroke-linejoin="round"><path d="${path}"/></svg>`)}")`;
  const controlIcons = [
    ['button:is([data-a-target="chat-settings"], [data-a-target="chat-settings-button"])', visual.icons === "thin" ? "M4 7h16M4 17h16M8 4v6M16 14v6" : "M12 3a9 9 0 1 0 0 18 9 9 0 0 0 0-18M8 12h8M12 8v8"],
    ['button[data-a-target="emote-picker-button"]', "M12 3a9 9 0 1 0 0 18 9 9 0 0 0 0-18M8 9h.01M16 9h.01M8 14q4 5 8 0"],
    ['button:is([data-a-target="chat-viewers-list"], [data-a-target="chat-viewer-list"] )', "M9 4a3 3 0 1 0 0 6 3 3 0 0 0 0-6M3 20v-3a6 6 0 0 1 12 0v3M16 4a3 3 0 0 1 0 6M18 14a5 5 0 0 1 3 5v1"]
  ];
  const iconsCss = isChat && visual.icons !== "native" ? controlIcons.map(([selector,path]) => `
${selector} { position: relative; }
${selector} svg { visibility: hidden !important; }
${selector}::after { content: ""; position: absolute; width: 18px; height: 18px; left: 50%; top: 50%; transform: translate(-50%,-50%); background: currentColor; mask: ${iconSvg(path)} center / contain no-repeat; -webkit-mask: ${iconSvg(path)} center / contain no-repeat; pointer-events: none; }
`).join("\n") : "";
  const feedLayoutCss = isFeed && visual.feed !== "native" ? `
.activity-list-layout .activity-base-list-item { margin: ${visual.feed === "cards" ? "6px" : "0"} !important; border-radius: ${visual.feed === "cards" ? "8px" : "0"} !important; background-color: ${p.base} !important; ${visual.feed === "cards" ? `border: 1px solid ${p.border} !important; border-left: 3px solid ${p.accent} !important;` : "padding-left: 6px !important; padding-right: 6px !important;"} }
.activity-list-layout .activity-base-list-item__title { min-width: 0 !important; }
.activity-list-layout .activity-base-list-item__title p { overflow-wrap: anywhere !important; word-break: normal !important; }
${visual.feed === "compact" ? ".activity-list-layout .activity-base-list-item > div { padding-top: 4px !important; padding-bottom: 4px !important; }" : ""}
` : "";
  const edgeId = "obs-twitch-dock-themes-edge";
  const edgeCss = isChat && visual.alerts ? `
#${edgeId} { position: fixed !important; inset: 0 !important; box-sizing: border-box !important; pointer-events: none !important; z-index: 2147483000 !important; border: 2px solid transparent; opacity: 0; }
#${edgeId}[data-event] { opacity: 1; border-color: var(--tdt-edge-color); box-shadow: inset 0 0 13px var(--tdt-edge-color); }
@media (prefers-reduced-motion: no-preference) {
 #${edgeId}[data-event][data-motion="true"] { animation: tdt-border-pulse 1500ms ease-in-out infinite; }
}
@keyframes tdt-border-pulse { 0%,100% { opacity: .45; } 50% { opacity: 1; } }
` : "";
  function syncEffects() {
    if (typeof window === "undefined" || typeof MutationObserver === "undefined" || !document.body) return;
    if (!isChat || !visual.alerts) { clearEffects(); return; }
    let state = window[effectsKey];
    if (state?.version === 1 && state.body !== document.body) { clearEffects(); state = null; }
    if (!state || state.version !== 1) {
      const seen = new WeakSet(); const knownIds = new Set();
      const selector = '[data-msg-id], [data-notice-type], [data-a-target="raid-notice"], [data-a-target="subscription-notice"], .chat-line__message--mentioned, .mention-fragment--highlighted, [data-a-target="chat-message-mention"][data-is-self="true"]';
      const edge = document.createElement("div"); edge.id = edgeId;
      edge.setAttribute("aria-hidden", "true"); document.body.appendChild(edge);
      let timer; let alive = true;
      state = {
        version: 1, body: document.body, visual, accent: p.accent, motion: config.motion === true, lastTest: typeof config.testPulse === "string" ? config.testPulse : "", readyAt: Date.now() + 1500,
        pulse(kind) {
          if (!alive) return;
          const allowed = kind === "test" || (kind === "raid" && this.visual.raids) || (kind === "subscription" && this.visual.subscriptions) || (kind === "mention" && this.visual.mentions);
          if (!allowed) return;
          clearTimeout(timer);
          edge.style.setProperty("--tdt-edge-color", {raid:"#b7a0ff",subscription:"#6eddb9",mention:"#ffd275",test:this.accent}[kind]);
          edge.removeAttribute("data-event"); void edge.offsetWidth;
          edge.setAttribute("data-event", kind); edge.setAttribute("data-motion", String(this.motion));
          timer = setTimeout(() => edge.removeAttribute("data-event"), this.visual.duration * 1000);
        },
        dispose() { alive = false; clearTimeout(timer); observer.disconnect(); edge.remove(); }
      };
      function inspect(element, initial = false) {
        if (!(element instanceof Element)) return;
        const candidates = [ ...(element.matches(selector) ? [element] : []), ...element.querySelectorAll(selector) ];
        for (const candidate of candidates) {
          const item = candidate.closest('.user-notice-line, .chat-line__message, [data-msg-id], [data-notice-type]') || candidate;
          if (seen.has(item)) continue;
          const msg = item.getAttribute("data-msg-id") || candidate.getAttribute("data-msg-id") || item.getAttribute("data-notice-type") || candidate.getAttribute("data-notice-type") || "";
          const target = candidate.getAttribute("data-a-target");
          let kind = "";
          if (msg === "raid" || target === "raid-notice") kind = "raid";
          if (["sub","resub","subgift","anonsubgift","submysterygift","giftpaidupgrade"].includes(msg) || target === "subscription-notice") kind = "subscription";
          if (candidate.matches('.chat-line__message--mentioned, .mention-fragment--highlighted, [data-a-target="chat-message-mention"][data-is-self="true"]')) kind = "mention";
          if (!kind) continue;
          seen.add(item);
          const uid = item.getAttribute("data-id");
          const key = uid ? `${kind}:${uid}` : "";
          if (key && knownIds.has(key)) continue;
          if (key) { knownIds.add(key); if (knownIds.size > 500) knownIds.delete(knownIds.values().next().value); }
          if (!initial && Date.now() >= state.readyAt) state.pulse(kind);
        }
      }
      const observer = new MutationObserver(records => {
        for (const record of records) {
          if (record.type === "attributes") inspect(record.target);
          else for (const node of record.addedNodes) inspect(node);
        }
      });
      inspect(document.body, true);
      observer.observe(document.body, {childList:true,subtree:true,attributes:true,attributeFilter:["data-msg-id","data-notice-type","data-a-target","data-is-self","class"]});
      window[effectsKey] = state;
    }
    state.visual = visual; state.motion = config.motion === true; state.accent = p.accent;
    const activeEdge = document.getElementById(edgeId);
    if (activeEdge) {
      activeEdge.setAttribute("data-motion", String(state.motion));
      const kind = activeEdge.getAttribute("data-event");
      if ((kind === "raid" && !visual.raids) || (kind === "subscription" && !visual.subscriptions) || (kind === "mention" && !visual.mentions)) activeEdge.removeAttribute("data-event");
    }
    // Test requests have a transient nonce, so the periodic reinjection does not replay them.
    if (typeof config.testPulse === "string" && config.testPulse && state.lastTest !== config.testPulse) {
      state.lastTest = config.testPulse; state.pulse("test");
    }
  }

  const css = `
${densityCss}
${focusCss}
${iconsCss}
${feedLayoutCss}
${edgeCss}
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
  syncEffects();
  if (existing && existing.textContent === css) return "unchanged";
  const target = document.head || document.documentElement;
  if (!target) return "waiting-for-document";
  const style = existing || document.createElement("style");
  style.id = id;
  style.textContent = css;
  if (!existing) target.appendChild(style);
  return "applied";
})();
