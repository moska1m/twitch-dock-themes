#pragma once
#include <QObject>
#include <QPointer>
#include <QTimer>
#include <QJsonObject>
#include <QWidget>
#include <QMap>
#include <QSet>
#include <QSettings>
#include <QStringList>
#include <QDateTime>
#include "visual-options.hpp"

class ThemeController : public QObject {
public:
    explicit ThemeController(QWidget *main, QObject *parent = nullptr);
    bool enabled = true;
    bool motion = true;
    VisualOptions visual;
    void testBorder();
    QString mode = "follow";
    QString density = "standard";
    QString background = "#101014", accent = "#54ccf5", border = "#35353b";
    QJsonObject diagnostics() const;
    QJsonObject preset() const;
    bool setPreset(const QJsonObject &value, QString *error = nullptr);
    QMap<QString, bool> panels;
    QJsonObject overrides;
    QSet<QString> found;
    int lastFound = 0;
    static QStringList dockIds();
    static bool supportsDensity(const QString &id);
    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings) const;
    void resetPreferences();
    void apply();
    void remove();
    QJsonObject palette(const QString &modeOverride = {}) const;
    static QString script(bool enabled, const QJsonObject &palette, bool motion = true, const QString &density = "standard", const QString &panel = "twitchChat", const QJsonObject &visual = {}, const QString &testPulse = {});
private:
    QPointer<QWidget> main;
    QTimer timer;
    QDateTime lastScan;
    QString testPulse;
    QMap<QString, QString> requests;
    int dispatch(bool active);
};
