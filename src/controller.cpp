#include "controller.hpp"
#include "browser-panel.hpp"
#include "bridge-generated.hpp"
#include <QApplication>
#include <QPalette>
#include <QJsonDocument>
#include <QJsonArray>
#include <QSet>
#include <QRegularExpression>
#include <QDockWidget>
#include <algorithm>
#include <cmath>

static double luminance(const QColor &c)
{
    auto linear = [](double x) { return x <= .04045 ? x / 12.92 : std::pow((x + .055) / 1.055, 2.4); };
    return .2126 * linear(c.redF()) + .7152 * linear(c.greenF()) + .0722 * linear(c.blueF());
}

ThemeController::ThemeController(QWidget *window, QObject *parent) : QObject(parent), main(window)
{
    resetPreferences();
    timer.setInterval(2500);
    connect(&timer, &QTimer::timeout, this, [this]() { if (enabled) apply(); });
    timer.start();
}

QStringList ThemeController::dockIds()
{
    return {"twitchChat", "twitchInfo", "twitchFeed", "twitchStats"};
}

bool ThemeController::supportsDensity(const QString &id)
{
    return id == "twitchChat" || id == "twitchFeed";
}

void ThemeController::resetPreferences()
{
    overrides = {};
    enabled = true;
    motion = true;
    mode = "follow";
    density = "standard";
    background = "#101014"; accent = "#54ccf5"; border = "#35353b";
    for (const auto &id : dockIds()) panels[id] = true;
}

void ThemeController::loadSettings(QSettings &s)
{
    resetPreferences();
    enabled = s.value("enabled", true).toBool();
    motion = s.value("motion", true).toBool();
    mode = s.value("mode", "follow").toString();
    if (mode != "obsidian" && mode != "graphite" && mode != "follow" && mode != "custom") mode = "follow";
    density = s.value("density", "standard").toString();
    if (density != "compact" && density != "comfortable") density = "standard";
    const QRegularExpression hex("^#[0-9a-fA-F]{6}$");
    for (auto entry : {qMakePair(QString("background"), &background), qMakePair(QString("accent"), &accent), qMakePair(QString("border"), &border)}) {
        const auto value = s.value("colors/" + entry.first, *entry.second).toString();
        if (hex.match(value).hasMatch()) *entry.second = value;
    }
    const auto stored = QJsonDocument::fromJson(s.value("overrides").toByteArray()).object();
    // Invalid stored overrides fall back to the global settings.
    for (const auto &id : dockIds()) {
        const auto item = stored[id].toObject();
        QJsonObject valid;
        if (QStringList{"follow", "obsidian", "graphite", "custom"}.contains(item["mode"].toString())) valid["mode"] = item["mode"];
        if (supportsDensity(id) && QStringList{"compact", "standard", "comfortable"}.contains(item["density"].toString())) valid["density"] = item["density"];
        if (!valid.isEmpty()) overrides[id] = valid;
    }
    const bool legacy = !s.contains("schema") && s.contains("secondary");
    for (const auto &id : dockIds()) {
        const bool defaultValue = legacy ? (id == "twitchChat" || s.value("secondary").toBool()) : true;
        panels[id] = s.value("panels/" + id, defaultValue).toBool();
    }
}

void ThemeController::saveSettings(QSettings &s) const
{
    s.setValue("schema", 4);
    s.setValue("overrides", QJsonDocument(overrides).toJson(QJsonDocument::Compact));
    s.setValue("density", density);
    s.setValue("colors/background", background);
    s.setValue("colors/accent", accent);
    s.setValue("colors/border", border);
    s.setValue("enabled", enabled);
    s.setValue("motion", motion);
    s.setValue("mode", mode);
    for (const auto &id : dockIds()) s.setValue("panels/" + id, panels.value(id, false));
    s.remove("secondary");
    s.sync();
}

static double contrast(const QColor &a, const QColor &b)
{
    const auto x = luminance(a), y = luminance(b);
    return (std::max(x, y) + .05) / (std::min(x, y) + .05);
}

static QColor readableLink(const QColor &original, const QColor &base, const QColor &window)
{
    const auto score = [&](const QColor &c) { return std::min(contrast(c, base), contrast(c, window)); };
    if (score(original) >= 4.5) return original;
    const QColor target = score(Qt::white) > score(Qt::black) ? Qt::white : Qt::black;
    for (int step = 1; step <= 20; ++step) {
        const double t = step / 20.;
        const QColor candidate = QColor::fromRgbF(
            original.redF() * (1-t) + target.redF() * t,
            original.greenF() * (1-t) + target.greenF() * t,
            original.blueF() * (1-t) + target.blueF() * t);
        if (score(candidate) >= 4.5) return candidate;
    }
    return target;
}

QJsonObject ThemeController::palette(const QString &modeOverride) const
{
    const auto mode = modeOverride.isEmpty() ? this->mode : modeOverride;
    if (mode == "obsidian" || mode == "graphite") {
        const bool graphite = mode == "graphite";
        return {
            {"window", graphite ? "#26262b" : "#18181b"}, {"base", graphite ? "#202024" : "#101014"},
            {"button", graphite ? "#303035" : "#202024"}, {"hover", graphite ? "#414148" : "#232329"},
            {"input", graphite ? "#2b2b30" : "#202024"}, {"selection", graphite ? "#404045" : "#153c4d"},
            {"text", "#f1f1f3"}, {"muted", "#aaaab3"}, {"border", graphite ? "#4b4b52" : "#35353b"},
            {"accent", "#54ccf5"}, {"accentHover", "#8edfff"}, {"onAccent", "#101014"},
            {"link", "#54ccf5"}
        };
    }
    auto p = QApplication::palette();
    if (mode == "custom") {
        const QColor bg(background);
        const bool bright = luminance(bg) > .45;
        const QColor surface = bright ? bg.darker(108) : QColor::fromRgb(
            std::min(255, bg.red()+16), std::min(255, bg.green()+16), std::min(255, bg.blue()+16));
        p.setColor(QPalette::Window, bg); p.setColor(QPalette::Base, bg);
        p.setColor(QPalette::Button, surface);
        const QColor text = contrast(Qt::white, bg) > contrast(Qt::black, bg) ? Qt::white : Qt::black;
        p.setColor(QPalette::Text, text);
        p.setColor(QPalette::Disabled, QPalette::Text, readableLink(bright ? QColor("#555555") : QColor("#aaaaaa"), bg, surface));
        p.setColor(QPalette::Highlight, QColor(this->accent));
        p.setColor(QPalette::Link, QColor(this->accent));
        p.setColor(QPalette::Light, QColor(border));
    }
    const QColor accent = p.color(QPalette::Highlight);
    const bool light = luminance(p.color(QPalette::Window)) > .45;
    const QColor hover = light ? p.color(QPalette::Button).darker(108) : p.color(QPalette::Button).lighter(125);
    const QColor accentHover = light ? accent.darker(110) : accent.lighter(115);
    const double whiteScore = std::min(contrast(Qt::white, accent), contrast(Qt::white, accentHover));
    const double blackScore = std::min(contrast(Qt::black, accent), contrast(Qt::black, accentHover));
    const QColor onAccent = blackScore > whiteScore ? Qt::black : Qt::white;
    const QColor link = readableLink(p.color(QPalette::Link), p.color(QPalette::Base), p.color(QPalette::Window));
    return {
        {"window", p.color(QPalette::Window).name()}, {"base", p.color(QPalette::Base).name()},
        {"button", p.color(QPalette::Button).name()}, {"hover", hover.name()},
        {"input", p.color(QPalette::Button).name()}, {"selection", accent.name()},
        {"text", p.color(QPalette::Text).name()}, {"muted", p.color(QPalette::Disabled, QPalette::Text).name()},
        {"border", p.color(QPalette::Light).name()}, {"accent", accent.name()},
        {"accentHover", accentHover.name()}, {"onAccent", onAccent.name()}, {"link", link.name()}
    };
}

QString ThemeController::script(bool active, const QJsonObject &colors, bool motion, const QString &density, const QString &panel)
{
    QString result = QString::fromUtf8(bridgeScript);
    const QJsonObject config{{"enabled", active}, {"palette", colors}, {"motion", motion}, {"density", density}, {"panel", panel}};
    result.replace("__CONFIG__", QString::fromUtf8(QJsonDocument(config).toJson(QJsonDocument::Compact)));
    return result;
}

int ThemeController::dispatch(bool active)
{
    found.clear();
    requests.clear();
    lastScan = QDateTime::currentDateTimeUtc();
    if (!main) return 0;
    const std::string removeScript = script(false, {}).toUtf8().toStdString();
    int count = 0;
    // Run on Qt's UI thread. Only named built-in Twitch docks are considered.
    for (auto *dock : main->findChildren<QDockWidget *>()) {
        if (!panels.contains(dock->objectName())) continue;
        const bool selected = panels.value(dock->objectName(), false);
        const auto settings = overrides.value(dock->objectName()).toObject();
        const auto panelDensity = supportsDensity(dock->objectName()) ? settings["density"].toString(density) : QString("standard");
        const auto applyScript = script(active, palette(settings["mode"].toString()), motion, panelDensity, dock->objectName()).toUtf8().toStdString();
        for (auto *widget : dock->findChildren<QWidget *>()) {
            if (!widget->inherits("QCefWidget")) continue;
            // QCefWidget layout audited in OBS 30.0.0 through 32.2.2; browser API 3 checked before use.
            auto *browser = static_cast<QCefWidget *>(widget);
            browser->executeJavaScript(active && selected ? applyScript : removeScript);
            found.insert(dock->objectName());
            requests[dock->objectName()] = active && selected ? "style-requested" : "removal-requested";
            if (selected) ++count;
        }
    }
    return count;
}

void ThemeController::apply() { lastFound = dispatch(enabled); }
void ThemeController::remove() { lastFound = dispatch(false); }

QJsonObject ThemeController::preset() const
{
    QJsonObject selected;
    for (const auto &id : dockIds()) selected[id] = panels.value(id, false);
    return {{"format", "twitch-dock-themes"}, {"version", 2}, {"overrides", overrides}, {"enabled", enabled},
        {"mode", mode}, {"motion", motion}, {"density", density}, {"panels", selected},
        {"colors", QJsonObject{{"background", background}, {"accent", accent}, {"border", border}}}};
}

bool ThemeController::setPreset(const QJsonObject &v, QString *error)
{
    auto fail = [&]() { if (error) *error = "Preset inválido ou de uma versão incompatível."; return false; };
    if (v["format"] != "twitch-dock-themes" || (v["version"] != 1 && v["version"] != 2) ||
        !v["enabled"].isBool() || !v["motion"].isBool()) return fail();
    const auto m = v["mode"].toString(), d = v["density"].toString();
    if (!QStringList{"follow", "obsidian", "graphite", "custom"}.contains(m) ||
        !QStringList{"compact", "standard", "comfortable"}.contains(d)) return fail();
    const auto colors = v["colors"].toObject(), selected = v["panels"].toObject();
    const QRegularExpression hex("^#[0-9a-fA-F]{6}$");
    for (const auto &key : {"background", "accent", "border"})
        if (!hex.match(colors[key].toString()).hasMatch()) return fail();
    for (const auto &id : dockIds()) if (!selected[id].isBool()) return fail();
    QJsonObject incoming;
    if (v["version"] == 2) {
        if (!v["overrides"].isObject()) return fail();
        const auto raw = v["overrides"].toObject();
        for (auto it = raw.begin(); it != raw.end(); ++it) {
            if (!dockIds().contains(it.key())) return fail();
            if (it.value().isNull()) continue; // Recover presets affected by the old mutating lookup.
            if (!it.value().isObject()) return fail();
            auto item = it.value().toObject();
            for (auto field = item.begin(); field != item.end(); ++field) {
                if (field.key() == "mode") {
                    if (!QStringList{"follow", "obsidian", "graphite", "custom"}.contains(field.value().toString())) return fail();
                } else if (field.key() == "density") {
                    if (!QStringList{"compact", "standard", "comfortable"}.contains(field.value().toString())) return fail();
                } else return fail();
            }
            if (!supportsDensity(it.key())) item.remove("density");
            if (!item.isEmpty()) incoming[it.key()] = item;
        }
    }
    overrides = incoming;
    enabled = v["enabled"].toBool(); motion = v["motion"].toBool(); mode = m; density = d;
    background = colors["background"].toString(); accent = colors["accent"].toString(); border = colors["border"].toString();
    for (const auto &id : dockIds()) panels[id] = selected[id].toBool();
    return true;
}

QJsonObject ThemeController::diagnostics() const
{
    QJsonArray items;
    for (const auto &id : dockIds()) {
        const auto item = overrides.value(id).toObject();
        const QString effectiveMode = item["mode"].toString(mode);
        items.append(QJsonObject{{"panel", id}, {"browserFound", found.contains(id)},
            {"selected", panels.value(id, false)}, {"mode", effectiveMode},
            {"density", supportsDensity(id) ? item["density"].toString(density) : QString("standard")},
            {"densitySupported", supportsDensity(id)},
            {"request", requests.value(id, "none")},
            {"colors", palette(effectiveMode)}});
    }
    return {{"format", "twitch-dock-themes-diagnostics"}, {"version", 1},
        {"enabled", enabled}, {"motion", motion},
        {"lastScanUtc", lastScan.isValid() ? lastScan.toString(Qt::ISODateWithMs) : QString()},
        {"visualApplicationConfirmed", false},
        {"scope", "Built-in Twitch docks only. Script execution has no result callback; page styling is not confirmed."},
        {"panels", items}};
}
