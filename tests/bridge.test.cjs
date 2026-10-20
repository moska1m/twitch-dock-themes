const fs = require("node:fs");
const vm = require("node:vm");
const assert = require("node:assert/strict");
const source = fs.readFileSync(require("node:path").join(__dirname, "../src/bridge.js"), "utf8");
const palette = Object.fromEntries(["base", "window", "button", "hover", "input", "selection", "accent",
  "accentHover", "text", "muted", "onAccent", "border", "link"].map(k => [k, "#202024"]));
let elements = new Map(), writes = 0;
const document = {
  getElementById: id => elements.get(id),
  createElement: () => {
    let text = "";
    return { id: "", get textContent() { return text; }, set textContent(v) { text = v; writes++; },
      remove() { elements.delete(this.id); } };
  },
  head: { appendChild: el => elements.set(el.id, el) },
};
function run(enabled, hostname = "www.twitch.tv", pathname = "/popout/example/chat", colors = palette, protocol = "https:", motion = true, density = "standard", panel = "twitchChat") {
  return vm.runInNewContext(source.replace("__CONFIG__", JSON.stringify({ enabled, palette: colors, motion, density, panel })),
    { document, location: { hostname, pathname, protocol } });
}
assert.equal(run(true), "applied");
assert.equal(elements.size, 1);
assert.equal(run(true), "unchanged");
assert.equal(writes, 1);
assert.equal(run(false), "removed");
assert.equal(elements.size, 0);
for (const host of ["twitch.tv.evil.test", "id.twitch.tv", "example.com"])
  assert.equal(run(true, host), "outside-scope");
assert.equal(run(true, "www.twitch.tv", "/login"), "outside-scope");
assert.equal(run(true, "www.twitch.tv", "/popout/example/chat", palette, "http:"), "outside-scope");
assert.equal(run(true, "www.twitch.tv", "/popout/example/chat", { ...palette, accent: "red; } body { display:none" }), "invalid-palette");
assert.equal(run(true, "www.twitch.tv", "/popout/example/chat", {}), "invalid-palette");
assert.equal(run(true, "dashboard.twitch.tv", "/popout/u/example/stream-manager/edit-stream-info"), "applied");
elements.clear(); // Simulate a navigation with a new document.
assert.equal(run(true), "applied");
assert.equal(run(true, "www.twitch.tv", "/login"), "outside-scope");
assert.equal(elements.size, 0, "SPA navigation to a login page must remove the style");
run(true, "www.twitch.tv", "/popout/example/chat", palette, "https:", false);
assert.ok(!elements.values().next().value.textContent.includes("transition:"));
run(true);
assert.ok(elements.values().next().value.textContent.includes("prefers-reduced-motion"));
assert.ok(!/\.chat-author__display-name|\.chat-badge|\.chat-image/.test(source));
assert.ok(source.includes("prefers-reduced-motion"));
assert.ok(!/\b(fetch|XMLHttpRequest|localStorage|cookie)\b/.test(source));
console.log("PASS: host/path scope, idempotency, removal, navigation, palette validation; no network/storage access");

for (const [density, padding] of [["compact", "2px"], ["comfortable", "8px"]]) {
  run(true, "www.twitch.tv", "/popout/example/chat", palette, "https:", true, density);
  assert.ok(elements.values().next().value.textContent.includes(`padding-top: ${padding} !important`));
}
for (const density of ["standard", "__proto__", "red; } body { display:none"]) {
  run(true, "www.twitch.tv", "/popout/example/chat", palette, "https:", true, density);
  assert.ok(!elements.values().next().value.textContent.includes("padding-top:"));
}
console.log("PASS: compact/comfortable spacing, standard reset, unknown density rejection");

run(true);
const chatCss = elements.values().next().value.textContent;
assert.ok(chatCss.includes('[data-a-target="chat-input"]:is(textarea, [contenteditable="true"])'));
assert.ok(chatCss.includes("background: transparent !important;"));
assert.ok(chatCss.includes("box-shadow: none !important;"));
console.log("PASS: chat editor is transparent; container retains native focus styling");

const feedPath = "/popout/u/example/stream-manager/activity-feed";
run(true, "dashboard.twitch.tv", feedPath, palette, "https:", true, "compact", "twitchFeed");
let feedCss = elements.values().next().value.textContent;
assert.ok(feedCss.includes(".activity-list-layout .activity-base-list-item"));
assert.ok(feedCss.includes("font-size: 12px !important"));
assert.ok(!feedCss.includes(".chat-input { padding:"));
run(true, "dashboard.twitch.tv", feedPath, palette, "https:", true, "comfortable", "twitchFeed");
assert.ok(elements.values().next().value.textContent.includes("font-size: 14px !important"));
for (const panel of ["twitchInfo", "twitchStats"]) {
  run(true, "dashboard.twitch.tv", feedPath, palette, "https:", true, "compact", panel);
  assert.ok(!elements.values().next().value.textContent.includes("padding-top:"));
}
run(true, "dashboard.twitch.tv", feedPath, palette, "https:", true, "standard", "twitchFeed");
assert.ok(!elements.values().next().value.textContent.includes("font-size:"));
console.log("PASS: activity feed density, font size, standard reset, info/stats exclusion");
