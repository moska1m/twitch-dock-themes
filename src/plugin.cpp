#include <obs-module.h>
#include <obs-frontend-api.h>
#include "browser-panel.hpp"
#include "controller.hpp"
#include <QApplication>
#include <QAction>
#include <QDialog>
#include <QVBoxLayout>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QSettings>
#include <QDir>
#include <QFileInfo>
#include <QPointer>
#include <QGroupBox>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QMessageBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QSaveFile>
#include <QFile>
#include <QJsonDocument>
#include <QJsonParseError>

OBS_DECLARE_MODULE()
MODULE_EXPORT const char *obs_module_name(void) { return "Twitch Dock Themes"; }
MODULE_EXPORT const char *obs_module_description(void) { return "Local appearance for OBS Twitch browser docks."; }
MODULE_EXPORT const char *obs_module_author(void) { return "Launcher Styles"; }

static QPointer<ThemeController> controller;
static QPointer<QAction> menuAction;
static QString settingsPath;

static bool save()
{
    if (!controller) return false;
    QSettings s(settingsPath, QSettings::IniFormat);
    controller->saveSettings(s);
    return s.status() == QSettings::NoError;
}

static void openSettings()
{
    if (!controller) return;
    QJsonObject committed = controller->preset();
    ThemeController draft(nullptr);
    draft.setPreset(committed);
    QDialog d(static_cast<QWidget *>(obs_frontend_get_main_window()));
    d.setWindowTitle("Twitch Dock Themes · 0.3.0");
    d.setMinimumWidth(480);
    auto *layout = new QVBoxLayout(&d);
    auto *enabled = new QCheckBox("Aplicar aparência aos painéis da Twitch");
    auto *mode = new QComboBox;
    mode->addItem("Seguir a paleta do OBS", "follow");
    mode->addItem("Obsidian", "obsidian");
    mode->addItem("Obsidian Graphite", "graphite");
    mode->addItem("Cores personalizadas", "custom");
    auto *density = new QComboBox;
    density->addItem("Compacta", "compact");
    density->addItem("Padrão da Twitch", "standard");
    density->addItem("Confortável", "comfortable");
    auto *colors = new QGroupBox("Cores personalizadas");
    auto *colorLayout = new QHBoxLayout(colors);
    QMap<QString, QPushButton *> colorButtons;
    for (const auto &pair : {qMakePair(QString("background"), QString("Fundo")),
         qMakePair(QString("accent"), QString("Destaque")), qMakePair(QString("border"), QString("Bordas"))}) {
        auto *button = new QPushButton(pair.second);
        colorButtons[pair.first] = button; colorLayout->addWidget(button);
        QObject::connect(button, &QPushButton::clicked, &d, [&, key = pair.first, button]() {
            const auto c = QColorDialog::getColor(QColor(button->property("color").toString()), &d, "Escolher cor");
            if (c.isValid()) { button->setProperty("color", c.name()); button->setText((key == "background" ? "Fundo " : key == "accent" ? "Destaque " : "Bordas ") + c.name()); }
        });
    }
    auto *panelBox = new QGroupBox("Painéis");
    auto *grid = new QGridLayout(panelBox);
    const QMap<QString, QString> names = {{"twitchChat", "Chat"}, {"twitchInfo", "Informações da transmissão"},
        {"twitchFeed", "Atividades"}, {"twitchStats", "Estatísticas"}};
    QMap<QString, QCheckBox *> checks;
    QMap<QString, QLabel *> states;
    int row = 0;
    for (const auto &id : ThemeController::dockIds()) {
        checks[id] = new QCheckBox(names.value(id)); states[id] = new QLabel;
        grid->addWidget(checks[id], row, 0); grid->addWidget(states[id], row++, 1);
    }
    auto *motion = new QCheckBox("Transição suave ao passar o mouse");
    motion->setToolTip("Respeita a preferência de movimento reduzido do sistema.");
    auto *status = new QLabel("Use Prévia para experimentar. Aplicar salva; Fechar desfaz a prévia.");
    status->setWordWrap(true);
    auto *preview = new QPushButton("Prévia nos painéis");
    auto *importButton = new QPushButton("Importar preset");
    auto *exportButton = new QPushButton("Exportar preset");
    auto *presetRow = new QHBoxLayout;
    presetRow->addWidget(importButton); presetRow->addWidget(exportButton);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Close);
    auto *apply = buttons->button(QDialogButtonBox::Apply); apply->setText("Aplicar e salvar");
    buttons->button(QDialogButtonBox::Close)->setText("Fechar");
    auto *reset = new QPushButton("Desativar e restaurar Twitch");
    layout->addWidget(enabled); layout->addWidget(new QLabel("Aparência")); layout->addWidget(mode);
    layout->addWidget(colors); layout->addWidget(new QLabel("Densidade")); layout->addWidget(density);
    layout->addWidget(panelBox); layout->addWidget(motion); layout->addLayout(presetRow);
    layout->addWidget(status); layout->addWidget(preview); layout->addWidget(reset); layout->addWidget(buttons);
    auto populate = [&]() {
        enabled->setChecked(draft.enabled); motion->setChecked(draft.motion);
        mode->setCurrentIndex(mode->findData(draft.mode)); density->setCurrentIndex(density->findData(draft.density));
        for (const auto &id : ThemeController::dockIds()) checks[id]->setChecked(draft.panels.value(id));
        const auto c = draft.preset()["colors"].toObject();
        for (auto it = colorButtons.begin(); it != colorButtons.end(); ++it) {
            it.value()->setProperty("color", c[it.key()].toString());
            it.value()->setText((it.key() == "background" ? "Fundo " : it.key() == "accent" ? "Destaque " : "Bordas ") + c[it.key()].toString());
        }
        colors->setEnabled(draft.mode == "custom");
    };
    auto collect = [&]() {
        draft.enabled = enabled->isChecked(); draft.motion = motion->isChecked();
        draft.mode = mode->currentData().toString(); draft.density = density->currentData().toString();
        draft.background = colorButtons["background"]->property("color").toString();
        draft.accent = colorButtons["accent"]->property("color").toString();
        draft.border = colorButtons["border"]->property("color").toString();
        for (const auto &id : ThemeController::dockIds()) draft.panels[id] = checks[id]->isChecked();
    };
    QObject::connect(mode, &QComboBox::currentIndexChanged, &d, [&]() { colors->setEnabled(mode->currentData() == "custom"); });
    populate();
    auto update = [&]() {
        if (!controller) { d.reject(); return; }
        for (const auto &id : ThemeController::dockIds()) states[id]->setText(controller->found.contains(id) ? "Encontrado" : "Não aberto");
    };
    controller->apply(); update();
    QTimer statusTimer(&d);
    QObject::connect(&statusTimer, &QTimer::timeout, &d, update); statusTimer.start(1000);
    auto persist = [&]() {
        if (save()) { committed = controller->preset(); status->setText("Preferências salvas."); }
        else QMessageBox::warning(&d, "Twitch Dock Themes", "Não foi possível salvar. A prévia será desfeita ao fechar.");
    };
    QObject::connect(preview, &QPushButton::clicked, &d, [&]() {
        if (!controller) return;
        collect(); controller->setPreset(draft.preset()); controller->apply(); update();
        status->setText("Prévia ativa nos painéis. Aplicar salva; Fechar restaura a configuração salva.");
    });
    QObject::connect(apply, &QPushButton::clicked, &d, [&]() {
        if (!controller) return;
        collect(); controller->setPreset(draft.preset()); controller->apply(); persist(); update();
    });
    QObject::connect(reset, &QPushButton::clicked, &d, [&]() {
        if (!controller) return;
        controller->setPreset(committed); controller->enabled = false; controller->remove(); persist();
        draft.setPreset(controller->preset()); populate(); update();
    });
    QObject::connect(importButton, &QPushButton::clicked, &d, [&]() {
        const auto path = QFileDialog::getOpenFileName(&d, "Importar preset", {}, "Preset JSON (*.json)");
        if (path.isEmpty()) return;
        QFile file(path); QString error; QJsonParseError parse;
        if (!file.open(QIODevice::ReadOnly) || file.size() > 65536) {
            QMessageBox::warning(&d, "Importar preset", "Arquivo ilegível ou maior que 64 KB."); return;
        }
        const auto doc = QJsonDocument::fromJson(file.readAll(), &parse);
        if (parse.error != QJsonParseError::NoError || !doc.isObject() || !draft.setPreset(doc.object(), &error)) {
            QMessageBox::warning(&d, "Importar preset", "Preset inválido ou incompatível. Nenhuma alteração aplicada."); return;
        }
        populate(); status->setText("Preset carregado. Use Prévia ou Aplicar para testar nos painéis.");
    });
    QObject::connect(exportButton, &QPushButton::clicked, &d, [&]() {
        const auto path = QFileDialog::getSaveFileName(&d, "Exportar preset", "twitch-dock-preset.json", "Preset JSON (*.json)");
        if (path.isEmpty()) return;
        collect(); QSaveFile file(path);
        const auto bytes = QJsonDocument(draft.preset()).toJson();
        if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
            QMessageBox::warning(&d, "Exportar preset", "Não foi possível salvar o arquivo.");
        else status->setText("Preset exportado. A exportação não altera os painéis.");
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    d.exec();
    if (controller) { controller->setPreset(committed); controller->apply(); }
}

static void frontendEvent(enum obs_frontend_event event, void *)
{
    if (!controller) return;
    if (event == OBS_FRONTEND_EVENT_EXIT) {
        delete controller.data();
    } else if (event == OBS_FRONTEND_EVENT_THEME_CHANGED || event == OBS_FRONTEND_EVENT_FINISHED_LOADING) {
        QTimer::singleShot(0, controller.data(), []() { if (controller) controller->apply(); });
    }
}

bool obs_module_load(void)
{
    if (obs_get_version() != LIBOBS_API_VER) {
        blog(LOG_WARNING, "[twitch-dock-themes] Build 0.3.0 requires OBS 32.2.2.");
        return false;
    }
    return true;
}

void obs_module_post_load(void)
{
    if (obs_browser_qcef_version() != 3) {
        blog(LOG_WARNING, "[twitch-dock-themes] Unsupported browser panel API; disabled.");
        return;
    }
    auto *main = static_cast<QWidget *>(obs_frontend_get_main_window());
    if (!main) return;
    char *path = obs_module_config_path("settings.ini");
    if (!path) return;
    settingsPath = QString::fromUtf8(path); bfree(path);
    QDir().mkpath(QFileInfo(settingsPath).absolutePath());
    controller = new ThemeController(main, main);
    QSettings s(settingsPath, QSettings::IniFormat);
    controller->loadSettings(s);
    menuAction = static_cast<QAction *>(obs_frontend_add_tools_menu_qaction("Twitch Dock Themes"));
    if (menuAction) QObject::connect(menuAction, &QAction::triggered, controller, []() { openSettings(); });
    obs_frontend_add_event_callback(frontendEvent, nullptr);
    blog(LOG_INFO, "[twitch-dock-themes] 0.3.0 loaded; mode=%s.", controller->mode.toUtf8().constData());
}

void obs_module_unload(void)
{
    obs_frontend_remove_event_callback(frontendEvent, nullptr);
    if (controller) { controller->remove(); delete controller.data(); }
    if (menuAction) delete menuAction.data();
}
