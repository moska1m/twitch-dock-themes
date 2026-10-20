#include "../src/plugin.cpp"
#include <QTemporaryDir>
#include <iostream>
#include <stdexcept>

static void checkUi(bool value, const char *message)
{
    if (!value) throw std::runtime_error(message);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);
    QTemporaryDir temp;
    if (!temp.isValid()) return 1;
    QWidget host;
    controller = new ThemeController(&host, &host);
    settingsPath = temp.filePath("settings.ini");
    QString failure;
    auto exercise = [&](bool reopen) {
        auto *dialog = dynamic_cast<SettingsDialog *>(QApplication::activeModalWidget());
        try {
            checkUi(dialog, "Settings dialog not opened");
            QComboBox *appearance = nullptr;
            for (auto *combo : dialog->findChildren<QComboBox *>())
                if (combo->findData("follow") == 0 && combo->findData("custom") >= 0) appearance = combo;
            checkUi(appearance, "Global appearance control missing");
            auto *toggle = dialog->findChild<QToolButton *>();
            checkUi(toggle && !toggle->isChecked(), "Advanced options did not start collapsed");
            QGroupBox *colors = nullptr;
            for (auto *group : dialog->findChildren<QGroupBox *>())
                if (group->title() == "Cores personalizadas") colors = group;
            checkUi(colors, "Custom colors group missing");
            const auto capture = qEnvironmentVariable("TDT_UI_CAPTURE_PATH");
            if (!reopen && !capture.isEmpty())
                checkUi(dialog->grab().save(capture), "Could not save UI capture");
            if (reopen) {
                checkUi(appearance->currentData() == "custom", "Reopened controls lost saved choice");
                checkUi(!colors->isHidden(), "Saved custom colors hidden");
                dialog->reject(); return;
            }
            checkUi(colors->isHidden(), "Unused colors should be hidden");
            const auto beforeToggle = controller->preset();
            toggle->click(); QCoreApplication::processEvents();
            checkUi(controller->preset() == beforeToggle, "Disclosure changed preferences");
            appearance->setCurrentIndex(appearance->findData("custom"));
            QCoreApplication::processEvents();
            checkUi(controller->mode == "custom" && !colors->isHidden(), "Live custom appearance failed");
            QPushButton *saveButton = nullptr;
            for (auto *button : dialog->findChildren<QPushButton *>())
                if (button->text() == "Salvar") saveButton = button;
            checkUi(saveButton, "Save button missing");
            saveButton->click();
            QSettings saved(settingsPath, QSettings::IniFormat);
            checkUi(saved.value("mode") == "custom", "Save did not persist live choice");
            appearance->setCurrentIndex(appearance->findData("graphite"));
            QCoreApplication::processEvents();
            QTimer::singleShot(0, dialog, []() {
                auto *notice = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
                if (notice && notice->defaultButton()) notice->defaultButton()->click();
            });
            dialog->reject();
            checkUi(dialog->isVisible() && controller->mode == "graphite", "Continue editing discarded live changes");
            QTimer::singleShot(0, dialog, []() {
                auto *notice = qobject_cast<QMessageBox *>(QApplication::activeModalWidget());
                if (notice) for (auto *button : notice->buttons())
                    if (notice->buttonRole(button) == QMessageBox::DestructiveRole) button->click();
            });
            dialog->reject();
            checkUi(!dialog->isVisible(), "Confirmed discard did not close");
        } catch (const std::exception &error) {
            failure = error.what();
            if (dialog) { dialog->canDiscard = {}; dialog->QDialog::reject(); }
        }
    };
    QTimer::singleShot(0, &app, [&]() { exercise(false); });
    openSettings();
    if (failure.isEmpty() && controller->mode != "custom") failure = "Closing did not restore saved mode";
    if (failure.isEmpty()) {
        QTimer::singleShot(0, &app, [&]() { exercise(true); });
        openSettings();
    }
    delete controller.data();
    if (!failure.isEmpty()) { std::cerr << failure.toStdString() << '\n'; return 1; }
    std::cout << "PASS: real settings UI, live edits, save, discard/cancel, re-open and progressive disclosure\n";
}
