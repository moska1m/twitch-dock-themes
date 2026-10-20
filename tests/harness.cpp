#include "controller.hpp"
#include "browser-panel.hpp"
#include <QApplication>
#include <QDockWidget>
#include <QJsonDocument>
#include <QPalette>
#include <QTemporaryDir>
#include <QFile>
#include <QJsonArray>
#include <iostream>
#include <stdexcept>

class FakeBrowser : public QCefWidget {
public:
    explicit FakeBrowser(QWidget *parent) : QCefWidget(parent) {}
    std::string last;
    int calls = 0;
    void setURL(const std::string &) override { throw std::runtime_error("Unexpected navigation"); }
    void setStartupScript(const std::string &) override { throw std::runtime_error("Startup script overwritten"); }
    void allowAllPopups(bool) override {}
    void closeBrowser() override {}
    void reloadPage() override { throw std::runtime_error("Unexpected reload"); }
    bool zoomPage(int) override { return false; }
    void executeJavaScript(const std::string &s) override { last = s; ++calls; }
};
static void check(bool value, const char *message) { if (!value) throw std::runtime_error(message); }
int main(int argc, char **argv) try
{
    QApplication app(argc, argv);
    QWidget main;
    auto *chat = new QDockWidget(&main); chat->setObjectName("twitchChat");
    auto *chatBrowser = new FakeBrowser(chat); chat->setWidget(chatBrowser);
    auto *info = new QDockWidget(&main); info->setObjectName("twitchInfo");
    auto *infoBrowser = new FakeBrowser(info); info->setWidget(infoBrowser);
    auto *feed = new QDockWidget(&main); feed->setObjectName("twitchFeed");
    auto *feedBrowser = new FakeBrowser(feed); feed->setWidget(feedBrowser);
    auto *stats = new QDockWidget(&main); stats->setObjectName("twitchStats");
    auto *statsBrowser = new FakeBrowser(stats); stats->setWidget(statsBrowser);
    auto *other = new QDockWidget(&main); other->setObjectName("customBrowser");
    auto *otherBrowser = new FakeBrowser(other); other->setWidget(otherBrowser);
    ThemeController controller(&main);
    controller.apply();
    check(controller.lastFound == 4, "Expected all four docks on new install");
    check(chatBrowser->last.find("\"enabled\":true") != std::string::npos, "Chat not enabled");
    check(infoBrowser->last.find("\"enabled\":true") != std::string::npos, "Info dock not enabled");
    check(feedBrowser->calls == 1 && statsBrowser->calls == 1, "Activity/stat docks not reached");
    check(otherBrowser->calls == 0, "Unrelated browser touched");
    controller.panels["twitchInfo"] = false;
    controller.panels["twitchChat"] = false;
    controller.apply();
    check(controller.lastFound == 2, "Independent toggles not respected");
    check(infoBrowser->last.find("\"enabled\":false") != std::string::npos, "Info style not removed");
    check(chatBrowser->last.find("\"enabled\":false") != std::string::npos, "Chat style not removed");
    check(controller.found.size() == 4, "Disabled but open docks missing from status");
    controller.resetPreferences();
    controller.mode = "graphite";
    controller.apply();
    check(chatBrowser->last.find("#26262b") != std::string::npos, "Graphite palette missing");
    controller.mode = "follow";
    QPalette p = QApplication::palette();
    p.setColor(QPalette::Window, QColor("#fafafa"));
    p.setColor(QPalette::Highlight, QColor("#ffdd00"));
    QApplication::setPalette(p);
    check(controller.palette()["window"] == "#fafafa", "Did not follow window palette");
    check(controller.palette()["onAccent"] == "#000000", "Incorrect bright accent contrast");
    p.setColor(QPalette::Highlight, QColor("#182050"));
    QApplication::setPalette(p);
    check(controller.palette()["onAccent"] == "#ffffff", "Incorrect dark accent contrast");
    controller.enabled = false;
    controller.apply();
    check(chatBrowser->last.find("\"enabled\":false") != std::string::npos, "Did not disable chat");
    delete info;
    controller.apply();
    check(controller.lastFound == 3, "Destroyed dock was retained");
    auto *newInfo = new QDockWidget(&main); newInfo->setObjectName("twitchInfo");
    auto *newBrowser = new FakeBrowser(newInfo); newInfo->setWidget(newBrowser);
    controller.enabled = true;
    controller.apply();
    check(newBrowser->calls == 1, "New dock not discovered");
    controller.motion = false;
    controller.apply();
    check(chatBrowser->last.find("\"motion\":false") != std::string::npos, "Motion preference lost");
    QTemporaryDir temp;
    check(temp.isValid(), "No temp dir");
    QSettings settings(temp.filePath("settings.ini"), QSettings::IniFormat);
    settings.setValue("secondary", false);
    settings.setValue("mode", "obsidian");
    controller.loadSettings(settings);
    check(controller.panels["twitchChat"] && !controller.panels["twitchInfo"], "Legacy chat-only migration failed");
    settings.setValue("secondary", true);
    controller.loadSettings(settings);
    check(controller.panels["twitchInfo"], "Legacy secondary migration failed");
    controller.panels["twitchChat"] = false;
    controller.motion = false;
    controller.enabled = false;
    controller.saveSettings(settings);
    controller.resetPreferences();
    controller.loadSettings(settings);
    check(!controller.enabled && !controller.motion && !controller.panels["twitchChat"], "Roundtrip lost preferences");
    check(!settings.contains("secondary"), "Legacy key retained");
    settings.setValue("mode", "invalid");
    controller.loadSettings(settings);
    check(controller.mode == "follow", "Invalid mode not handled");

    const auto saved = controller.preset();
    auto custom = saved;
    custom["mode"] = "custom"; custom["density"] = "compact";
    custom["colors"] = QJsonObject{{"background", "#000000"}, {"accent", "#ffdd00"}, {"border", "#333333"}};
    check(controller.setPreset(custom), "Valid preset rejected");
    check(controller.palette()["base"] == "#000000", "Custom background lost");
    check(controller.palette()["text"] == "#ffffff", "Dark background text contrast failed");
    check(controller.palette()["onAccent"] == "#000000", "Custom accent contrast failed");
    check(controller.palette()["border"] == "#333333", "Custom border lost");
    controller.saveSettings(settings); controller.resetPreferences(); controller.loadSettings(settings);
    check(controller.preset() == custom, "Custom settings roundtrip failed");
    auto bad = custom; bad["density"] = "arbitrary css";
    check(!controller.setPreset(bad) && controller.preset() == custom, "Invalid density mutated state");
    bad = custom; bad["colors"] = QJsonObject{{"background", "red;"}};
    check(!controller.setPreset(bad) && controller.preset() == custom, "Invalid colors mutated state");
    bad = custom; bad["panels"] = QJsonObject{};
    check(!controller.setPreset(bad), "Incomplete panel selection accepted");
    bad = custom; bad["version"] = 3;
    check(!controller.setPreset(bad), "Future preset accepted");
    const auto encoded = QJsonDocument(custom).toJson();
    check(controller.setPreset(QJsonDocument::fromJson(encoded).object()), "Export/import roundtrip failed");
    controller.enabled = true; controller.panels["twitchChat"] = true;
    controller.apply();
    check(chatBrowser->last.find("\"density\":\"compact\"") != std::string::npos, "Density missing from bridge");
    check(controller.setPreset(saved) && controller.preset() == saved, "Preview snapshot restoration failed");

    auto separate = saved;
    separate["enabled"] = true; separate["mode"] = "obsidian";
    separate["panels"] = QJsonObject{{"twitchChat", true}, {"twitchInfo", true}, {"twitchFeed", true}, {"twitchStats", true}};
    separate["overrides"] = QJsonObject{{"twitchChat", QJsonObject{{"mode", "graphite"}, {"density", "compact"}}},
        {"twitchInfo", QJsonObject{{"density", "comfortable"}}}};
    check(controller.setPreset(separate), "Per-panel preset rejected");
    controller.apply();
    check(chatBrowser->last.find("#26262b") != std::string::npos, "Chat appearance override missing");
    check(chatBrowser->last.find("\"density\":\"compact\"") != std::string::npos, "Chat density override missing");
    check(newBrowser->last.find("#18181b") != std::string::npos, "Info did not inherit global palette");
    check(newBrowser->last.find("\"density\":\"comfortable\"") != std::string::npos, "Info density override missing");
    check(feedBrowser->last.find("#18181b") != std::string::npos, "Override leaked to unrelated panel");
    controller.saveSettings(settings); controller.resetPreferences(); controller.loadSettings(settings);
    check(controller.preset() == separate, "Override persistence failed");
    auto invalidOverrides = separate;
    invalidOverrides["overrides"] = QJsonObject{{"customBrowser", QJsonObject{{"mode", "graphite"}}}};
    check(!controller.setPreset(invalidOverrides) && controller.preset() == separate, "Unknown panel override accepted");
    invalidOverrides["overrides"] = QJsonObject{{"twitchChat", QJsonObject{{"density", "invalid"}}}};
    check(!controller.setPreset(invalidOverrides), "Invalid override density accepted");
    auto legacyPreset = saved; legacyPreset["version"] = 1; legacyPreset.remove("overrides");
    check(controller.setPreset(legacyPreset) && controller.overrides.isEmpty(), "Legacy preset migration failed");
    check(controller.setPreset(saved), "Failed to restore global settings");

    QFile fixture(PALETTE_FIXTURE);
    check(fixture.open(QIODevice::ReadOnly), "Missing palette fixtures");
    const auto fixtures = QJsonDocument::fromJson(fixture.readAll()).object();
    const QMap<QString, QPalette::ColorRole> roles{
        {"window", QPalette::Window}, {"base", QPalette::Base}, {"text", QPalette::Text},
        {"button", QPalette::Button}, {"highlight", QPalette::Highlight},
        {"link", QPalette::Link}, {"light", QPalette::Light}
    };
    for (const auto &key : fixtures.keys()) {
        const auto colors = fixtures[key].toObject();
        QPalette sample;
        for (auto it = roles.begin(); it != roles.end(); ++it)
            sample.setColor(it.value(), QColor(colors[it.key()].toString()));
        sample.setColor(QPalette::Disabled, QPalette::Text, QColor(colors["muted"].toString()));
        QApplication::setPalette(sample);
        const auto result = controller.palette();
        check(result["window"] == colors["window"], "Theme window color not followed");
        check(result["base"] == colors["base"], "Theme base color not followed");
        for (const auto &value : result)
            check(QColor(value.toString()).isValid(), "Invalid output color");
    }
    std::cout << "PASS: four docks, per-dock toggles, cleanup, motion, preference migration, "
              << fixtures.size() << " OBS palettes, dock lifecycle, no unrelated browser access\n";
    return 0;
}

catch (const std::exception &error) { std::cerr << "FAIL: " << error.what() << std::endl; return 1; }
