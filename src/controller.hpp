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

class ThemeController : public QObject {
public:
    explicit ThemeController(QWidget *main, QObject *parent = nullptr);
    bool enabled = true;
    bool motion = true;
    QString mode = "follow";
    QMap<QString, bool> panels;
    QSet<QString> found;
    int lastFound = 0;
    static QStringList dockIds();
    void loadSettings(QSettings &settings);
    void saveSettings(QSettings &settings) const;
    void resetPreferences();
    void apply();
    void remove();
    QJsonObject palette() const;
    static QString script(bool enabled, const QJsonObject &palette, bool motion = true);
private:
    QPointer<QWidget> main;
    QTimer timer;
    int dispatch(bool active);
};
