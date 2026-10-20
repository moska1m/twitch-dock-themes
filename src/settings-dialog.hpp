#pragma once
#include <QDialog>
#include <functional>

class SettingsDialog : public QDialog {
public:
    using QDialog::QDialog;
    std::function<bool()> canDiscard;
    void reject() override
    {
        if (!canDiscard || canDiscard()) QDialog::reject();
    }
};
