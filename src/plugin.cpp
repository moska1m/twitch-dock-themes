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
    QDialog d(static_cast<QWidget *>(obs_frontend_get_main_window()));
    d.setWindowTitle("Twitch Dock Themes · 0.2.0");
    d.setMinimumWidth(460);
    auto *layout = new QVBoxLayout(&d);
    auto *enabled = new QCheckBox("Aplicar aparência aos painéis da Twitch");
    enabled->setChecked(controller->enabled);
    auto *mode = new QComboBox;
    mode->addItem("Seguir a paleta do OBS", "follow");
    mode->addItem("Obsidian", "obsidian");
    mode->addItem("Obsidian Graphite", "graphite");
    mode->setCurrentIndex(qMax(0, mode->findData(controller->mode)));
    auto *panelBox = new QGroupBox("Painéis");
    auto *grid = new QGridLayout(panelBox);
    const QMap<QString, QString> names = {
        {"twitchChat", "Chat"}, {"twitchInfo", "Informações da transmissão"},
        {"twitchFeed", "Atividades"}, {"twitchStats", "Estatísticas"}};
    QMap<QString, QCheckBox *> panelChecks;
    QMap<QString, QLabel *> panelStates;
    int row = 0;
    for (const auto &id : ThemeController::dockIds()) {
        auto *check = new QCheckBox(names.value(id));
        check->setChecked(controller->panels.value(id));
        auto *state = new QLabel;
        grid->addWidget(check, row, 0); grid->addWidget(state, row++, 1);
        panelChecks[id] = check; panelStates[id] = state;
    }
    auto *motion = new QCheckBox("Transição suave ao passar o mouse");
    motion->setChecked(controller->motion);
    motion->setToolTip("Respeita a preferência de movimento reduzido do sistema.");
    auto *note = new QLabel("O modo automático acompanha as cores do estilo escolhido no OBS.\n"
                           "Abra os painéis desejados pelo menu Painéis do OBS.");
    note->setWordWrap(true);
    auto *status = new QLabel;
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Close);
    auto *apply = buttons->button(QDialogButtonBox::Apply);
    apply->setText("Aplicar agora");
    buttons->button(QDialogButtonBox::Close)->setText("Fechar");
    auto *reset = new QPushButton("Desativar e restaurar Twitch");
    layout->addWidget(enabled); layout->addWidget(new QLabel("Aparência")); layout->addWidget(mode);
    layout->addWidget(panelBox); layout->addWidget(motion); layout->addWidget(note);
    layout->addWidget(status); layout->addWidget(reset); layout->addWidget(buttons);
    auto update = [&]() {
        if (!controller) { d.reject(); return; }
        for (const auto &id : ThemeController::dockIds())
            panelStates[id]->setText(controller->found.contains(id) ? "Encontrado" : "Não aberto");
        status->setText(controller->enabled ? "Personalização ativada." : "Aparência original da Twitch.");
    };
    controller->apply();
    update();
    QTimer statusTimer(&d);
    QObject::connect(&statusTimer, &QTimer::timeout, &d, update);
    statusTimer.start(1000);
    auto persist = [&]() {
        if (!save()) QMessageBox::warning(&d, "Twitch Dock Themes",
            "A aparência foi aplicada, mas não foi possível salvar as preferências.");
    };
    QObject::connect(apply, &QPushButton::clicked, &d, [&]() {
        if (!controller) return;
        controller->enabled = enabled->isChecked();
        controller->motion = motion->isChecked();
        for (const auto &id : ThemeController::dockIds()) controller->panels[id] = panelChecks[id]->isChecked();
        controller->mode = mode->currentData().toString();
        controller->apply(); persist(); update();
    });
    QObject::connect(reset, &QPushButton::clicked, &d, [&]() {
        if (!controller) return;
        controller->enabled = false; enabled->setChecked(false);
        controller->remove(); persist(); update();
    });
    QObject::connect(buttons, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    d.exec();
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
        blog(LOG_WARNING, "[twitch-dock-themes] Build 0.2.0 requires OBS 32.2.2.");
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
    blog(LOG_INFO, "[twitch-dock-themes] 0.2.0 loaded; mode=%s.", controller->mode.toUtf8().constData());
}

void obs_module_unload(void)
{
    obs_frontend_remove_event_callback(frontendEvent, nullptr);
    if (controller) { controller->remove(); delete controller.data(); }
    if (menuAction) delete menuAction.data();
}
