#include "controller.hpp"
#include "browser-panel.hpp"
#include "bridge-generated.hpp"
#include <QApplication>
#include <QPalette>
#include <QJsonDocument>
#include <QSet>
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

void ThemeController::resetPreferences()
{
    enabled = true;
    motion = true;
    mode = "follow";
    for (const auto &id : dockIds()) panels[id] = true;
}

void ThemeController::loadSettings(QSettings &s)
{
    resetPreferences();
    enabled = s.value("enabled", true).toBool();
    motion = s.value("motion", true).toBool();
    mode = s.value("mode", "follow").toString();
    if (mode != "obsidian" && mode != "graphite" && mode != "follow") mode = "follow";
    const bool legacy = !s.contains("schema") && s.contains("secondary");
    for (const auto &id : dockIds()) {
        const bool defaultValue = legacy ? (id == "twitchChat" || s.value("secondary").toBool()) : true;
        panels[id] = s.value("panels/" + id, defaultValue).toBool();
    }
}

void ThemeController::saveSettings(QSettings &s) const
{
    s.setValue("schema", 2);
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

QJsonObject ThemeController::palette() const
{
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
    const auto p = QApplication::palette();
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

QString ThemeController::script(bool active, const QJsonObject &colors, bool motion)
{
    QString result = QString::fromUtf8(bridgeScript);
    const QJsonObject config{{"enabled", active}, {"palette", colors}, {"motion", motion}};
    result.replace("__CONFIG__", QString::fromUtf8(QJsonDocument(config).toJson(QJsonDocument::Compact)));
    return result;
}

int ThemeController::dispatch(bool active)
{
    found.clear();
    if (!main) return 0;
    const std::string applyScript = script(active, palette(), motion).toUtf8().toStdString();
    const std::string removeScript = script(false, {}).toUtf8().toStdString();
    int count = 0;
    // Run on Qt's UI thread. Only named built-in Twitch docks are considered.
    for (auto *dock : main->findChildren<QDockWidget *>()) {
        if (!panels.contains(dock->objectName())) continue;
        const bool selected = panels.value(dock->objectName(), false);
        for (auto *widget : dock->findChildren<QWidget *>()) {
            if (!widget->inherits("QCefWidget")) continue;
            // ABI is pinned to OBS 32.2.2 / browser panel API 3.
            auto *browser = static_cast<QCefWidget *>(widget);
            browser->executeJavaScript(active && selected ? applyScript : removeScript);
            found.insert(dock->objectName());
            if (selected) ++count;
        }
    }
    return count;
}

void ThemeController::apply() { lastFound = dispatch(enabled); }
void ThemeController::remove() { lastFound = dispatch(false); }
