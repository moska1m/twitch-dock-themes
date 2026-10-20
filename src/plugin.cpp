#include <obs-module.h>
#include <obs-frontend-api.h>
#include "browser-panel.hpp"
#include "controller.hpp"
#include "preset-library.hpp"
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
#include <QPlainTextEdit>
#include <QClipboard>
#include <QInputDialog>
#include <QLineEdit>
#include <QScrollArea>
#include <QScreen>

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
    d.setWindowTitle("Twitch Dock Themes · experimental");
    d.setMinimumWidth(480);
    auto *outer = new QVBoxLayout(&d);
    auto *scroll = new QScrollArea; scroll->setWidgetResizable(true);
    scroll->setFrameShape(QFrame::NoFrame);
    auto *content = new QWidget;
    auto *layout = new QVBoxLayout(content);
    scroll->setWidget(content); outer->addWidget(scroll);
    d.resize(860, qMin(780, d.screen()->availableGeometry().height() - 80));
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
    QMap<QString, QComboBox *> panelModes, panelDensities;
    int row = 1;
    grid->addWidget(new QLabel("Painel"), 0, 0);
    grid->addWidget(new QLabel("Aparência"), 0, 1);
    grid->addWidget(new QLabel("Densidade (chat e atividades)"), 0, 2);
    for (const auto &id : ThemeController::dockIds()) {
        checks[id] = new QCheckBox(names.value(id)); states[id] = new QLabel;
        auto *pm = new QComboBox;
        auto *pd = ThemeController::supportsDensity(id) ? new QComboBox : nullptr;
        pm->addItem("Usar geral", ""); pm->addItem("Paleta do OBS", "follow");
        pm->addItem("Obsidian", "obsidian"); pm->addItem("Obsidian Graphite", "graphite");
        pm->addItem("Cores personalizadas", "custom");
        if (pd) { pd->addItem("Usar geral", ""); pd->addItem("Compacta", "compact");
        pd->addItem("Padrão da Twitch", "standard"); pd->addItem("Confortável", "comfortable"); }
        pm->setToolTip("Cores personalizadas usa as cores escolhidas acima.");
        panelModes[id] = pm; if (pd) panelDensities[id] = pd;
        grid->addWidget(checks[id], row, 0); grid->addWidget(pm, row, 1);
        if (pd) grid->addWidget(pd, row, 2); else grid->addWidget(new QLabel("—"), row, 2); grid->addWidget(states[id], row++, 3);
        QObject::connect(checks[id], &QCheckBox::toggled, &d, [pm, pd](bool on) { pm->setEnabled(on); if (pd) pd->setEnabled(on); });
    }
    auto *syncPanels = new QPushButton("Sincronizar todos com a configuração geral");
    grid->addWidget(syncPanels, row, 0, 1, 4);
    QObject::connect(syncPanels, &QPushButton::clicked, &d, [&]() {
        for (const auto &id : ThemeController::dockIds()) {
            panelModes[id]->setCurrentIndex(0); if (panelDensities.contains(id)) panelDensities[id]->setCurrentIndex(0);
        }
    });
    auto *motion = new QCheckBox("Transição suave ao passar o mouse");
    motion->setToolTip("Respeita a preferência de movimento reduzido do sistema.");
    auto *status = new QLabel("Use Prévia para experimentar. Aplicar salva; Fechar desfaz a prévia.");
    status->setWordWrap(true);
    auto *diagnostic = new QPushButton("Diagnóstico dos painéis");
    auto *preview = new QPushButton("Prévia nos painéis");
    auto *importButton = new QPushButton("Importar preset");
    auto *exportButton = new QPushButton("Exportar preset");
    PresetLibrary library(QFileInfo(settingsPath).dir().filePath("presets.json"));
    auto *libraryBox = new QGroupBox("Meus presets");
    auto *libraryLayout = new QGridLayout(libraryBox);
    auto *savedPresets = new QComboBox;
    savedPresets->setAccessibleName("Presets salvos");
    auto *loadPreset = new QPushButton("Carregar");
    auto *savePreset = new QPushButton("Salvar como");
    auto *deletePreset = new QPushButton("Excluir");
    libraryLayout->addWidget(savedPresets, 0, 0, 1, 3);
    libraryLayout->addWidget(loadPreset, 1, 0); libraryLayout->addWidget(savePreset, 1, 1);
    libraryLayout->addWidget(deletePreset, 1, 2);
    auto *presetRow = new QHBoxLayout;
    presetRow->addWidget(importButton); presetRow->addWidget(exportButton);
    auto *buttons = new QDialogButtonBox(QDialogButtonBox::Apply | QDialogButtonBox::Close);
    auto *apply = buttons->button(QDialogButtonBox::Apply); apply->setText("Aplicar e salvar");
    buttons->button(QDialogButtonBox::Close)->setText("Fechar");
    auto *reset = new QPushButton("Desativar e restaurar Twitch");
    layout->addWidget(enabled); layout->addWidget(new QLabel("Aparência")); layout->addWidget(mode);
    layout->addWidget(colors); layout->addWidget(new QLabel("Densidade (chat e atividades)")); layout->addWidget(density);
    layout->addWidget(panelBox); layout->addWidget(motion); layout->addWidget(libraryBox); layout->addLayout(presetRow);
    layout->addWidget(diagnostic);
    layout->addWidget(status); layout->addWidget(preview); layout->addWidget(reset); outer->addWidget(buttons);
    auto updateColors = [&]() {
        bool needed = mode->currentData() == "custom";
        for (auto *pm : panelModes) needed = needed || pm->currentData() == "custom";
        colors->setEnabled(needed);
    };
    auto populate = [&]() {
        enabled->setChecked(draft.enabled); motion->setChecked(draft.motion);
        mode->setCurrentIndex(mode->findData(draft.mode)); density->setCurrentIndex(density->findData(draft.density));
        for (const auto &id : ThemeController::dockIds()) {
            checks[id]->setChecked(draft.panels.value(id));
            const auto item = draft.overrides.value(id).toObject();
            panelModes[id]->setCurrentIndex(qMax(0, panelModes[id]->findData(item["mode"].toString())));
            if (panelDensities.contains(id)) panelDensities[id]->setCurrentIndex(qMax(0, panelDensities[id]->findData(item["density"].toString())));
            panelModes[id]->setEnabled(checks[id]->isChecked()); if (panelDensities.contains(id)) panelDensities[id]->setEnabled(checks[id]->isChecked());
        }
        const auto c = draft.preset()["colors"].toObject();
        for (auto it = colorButtons.begin(); it != colorButtons.end(); ++it) {
            it.value()->setProperty("color", c[it.key()].toString());
            it.value()->setText((it.key() == "background" ? "Fundo " : it.key() == "accent" ? "Destaque " : "Bordas ") + c[it.key()].toString());
        }
        updateColors();
    };
    auto collect = [&]() {
        draft.enabled = enabled->isChecked(); draft.motion = motion->isChecked();
        draft.mode = mode->currentData().toString(); draft.density = density->currentData().toString();
        draft.background = colorButtons["background"]->property("color").toString();
        draft.accent = colorButtons["accent"]->property("color").toString();
        draft.border = colorButtons["border"]->property("color").toString();
        draft.overrides = {};
        for (const auto &id : ThemeController::dockIds()) {
            draft.panels[id] = checks[id]->isChecked();
            QJsonObject item;
            if (!panelModes[id]->currentData().toString().isEmpty()) item["mode"] = panelModes[id]->currentData().toString();
            if (panelDensities.contains(id) && !panelDensities[id]->currentData().toString().isEmpty()) item["density"] = panelDensities[id]->currentData().toString();
            if (!item.isEmpty()) draft.overrides[id] = item;
        }
    };
    auto refreshLibrary = [&](const QString &selection) {
        QJsonObject items; QString error;
        const bool ok = library.read(items, error);
        savedPresets->clear();
        if (ok) {
            auto names = items.keys(); names.sort(Qt::CaseInsensitive);
            for (const auto &name : names) savedPresets->addItem(name, name);
        } else status->setText(error);
        if (!selection.isEmpty()) savedPresets->setCurrentIndex(savedPresets->findData(selection));
        loadPreset->setEnabled(ok && savedPresets->currentIndex() >= 0);
        deletePreset->setEnabled(ok && savedPresets->currentIndex() >= 0);
        savePreset->setEnabled(ok);
        savedPresets->setPlaceholderText(ok ? "Nenhum preset salvo" : "Biblioteca indisponível");
    };
    refreshLibrary({});
    QObject::connect(loadPreset, &QPushButton::clicked, &d, [&]() {
        QJsonObject items; QString error;
        if (!library.read(items, error) || !draft.setPreset(items.value(savedPresets->currentData().toString()).toObject(), &error)) {
            QMessageBox::warning(&d, "Carregar preset", error); return;
        }
        populate(); status->setText("Preset carregado nos controles. Use Prévia ou Aplicar e salvar.");
    });
    QObject::connect(savePreset, &QPushButton::clicked, &d, [&]() {
        bool accepted = false;
        const auto name = QInputDialog::getText(&d, "Salvar preset", "Nome (até 64 caracteres)", QLineEdit::Normal,
            {}, &accepted).trimmed();
        if (!accepted) return;
        QJsonObject items; QString error;
        if (!library.read(items, error)) { QMessageBox::warning(&d, "Salvar preset", error); return; }
        const bool exists = items.contains(name);
        if (exists && QMessageBox::question(&d, "Substituir preset", "Substituir o preset “" + name + "”?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
        collect();
        if (!library.put(name, draft.preset(), exists, error)) { QMessageBox::warning(&d, "Salvar preset", error); return; }
        refreshLibrary(name); status->setText("Preset salvo na biblioteca. Use Prévia ou Aplicar para alterar os painéis.");
    });
    QObject::connect(deletePreset, &QPushButton::clicked, &d, [&]() {
        const auto name = savedPresets->currentData().toString();
        if (QMessageBox::question(&d, "Excluir preset", "Excluir “" + name + "” da biblioteca?",
            QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes) return;
        QString error;
        if (!library.erase(name, error)) { QMessageBox::warning(&d, "Excluir preset", error); return; }
        refreshLibrary({}); status->setText("Preset excluído da biblioteca. A configuração dos painéis foi mantida.");
    });
    QObject::connect(mode, &QComboBox::currentIndexChanged, &d, updateColors);
    for (auto *pm : panelModes) QObject::connect(pm, &QComboBox::currentIndexChanged, &d, updateColors);
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
    QObject::connect(diagnostic, &QPushButton::clicked, &d, [&]() {
        if (!controller) return;
        QDialog report(&d);
        report.setWindowTitle("Diagnóstico dos painéis"); report.resize(700, 520);
        auto *box = new QVBoxLayout(&report);
        auto *explanation = new QLabel("O relatório mostra a configuração ativa, inclusive uma prévia. Encontrar o navegador e enviar CSS não confirma que a página aplicou o visual. O horário está em UTC.");
        explanation->setWordWrap(true);
        auto *text = new QPlainTextEdit; text->setReadOnly(true);
        auto *actions = new QDialogButtonBox(QDialogButtonBox::Close);
        auto *refresh = actions->addButton("Atualizar", QDialogButtonBox::ActionRole);
        auto *copy = actions->addButton("Copiar relatório", QDialogButtonBox::ActionRole);
        auto refreshReport = [&]() {
            if (!controller) { report.reject(); return; }
            controller->apply();
            auto data = controller->diagnostics();
            data["pluginBuild"] = "0.4.0";
            data["qtRuntime"] = QString::fromLatin1(qVersion());
            data["obsApiVersion"] = static_cast<int>(obs_get_version());
            text->setPlainText(QString::fromUtf8(QJsonDocument(data).toJson()));
            copy->setText("Copiar relatório");
        };
        box->addWidget(explanation); box->addWidget(text); box->addWidget(actions);
        QObject::connect(refresh, &QPushButton::clicked, &report, refreshReport);
        QObject::connect(copy, &QPushButton::clicked, &report, [&]() {
            QApplication::clipboard()->setText(text->toPlainText()); copy->setText("Copiado");
        });
        QObject::connect(actions, &QDialogButtonBox::rejected, &report, &QDialog::reject);
        refreshReport(); report.exec();
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
        blog(LOG_WARNING, "[twitch-dock-themes] Build 0.4.0 requires OBS 32.2.2.");
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
    blog(LOG_INFO, "[twitch-dock-themes] 0.4.0 experimental preset-library loaded; mode=%s.", controller->mode.toUtf8().constData());
}

void obs_module_unload(void)
{
    obs_frontend_remove_event_callback(frontendEvent, nullptr);
    if (controller) { controller->remove(); delete controller.data(); }
    if (menuAction) delete menuAction.data();
}
