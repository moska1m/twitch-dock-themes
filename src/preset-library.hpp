#pragma once
#include "controller.hpp"
#include <QFile>
#include <QSaveFile>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QDir>
#include <QFileInfo>

class PresetLibrary {
public:
    explicit PresetLibrary(QString file) : path(std::move(file)) {}
    bool read(QJsonObject &items, QString &error) const
    {
        items = {};
        QFile file(path);
        if (!file.exists()) return true;
        if (!file.open(QIODevice::ReadOnly) || file.size() > 2 * 1024 * 1024) {
            error = "Não foi possível ler a biblioteca de presets."; return false;
        }
        QJsonParseError parse;
        const auto doc = QJsonDocument::fromJson(file.readAll(), &parse);
        const auto root = doc.object();
        if (parse.error != QJsonParseError::NoError || !doc.isObject() ||
            root["format"] != "twitch-dock-themes-library" || root["version"] != 1 || !root["presets"].isObject()) {
            error = "Biblioteca inválida. O arquivo foi preservado."; return false;
        }
        const auto stored = root["presets"].toObject();
        if (stored.size() > 100) { error = "A biblioteca excede 100 presets."; return false; }
        ThemeController validator(nullptr);
        for (auto it = stored.begin(); it != stored.end(); ++it) {
            if (!validName(it.key()) || !it.value().isObject() || !validator.setPreset(it.value().toObject())) {
                error = "A biblioteca contém um preset inválido. O arquivo foi preservado."; return false;
            }
            items[it.key()] = validator.preset();
        }
        return true;
    }
    bool put(const QString &name, const QJsonObject &preset, bool replace, QString &error) const
    {
        if (!validName(name)) { error = "Use um nome de 1 a 64 caracteres, sem quebras de linha."; return false; }
        ThemeController validator(nullptr);
        if (!validator.setPreset(preset, &error)) return false;
        QJsonObject items;
        if (!read(items, error)) return false;
        if (items.contains(name) && !replace) { error = "Já existe um preset com esse nome."; return false; }
        if (!items.contains(name) && items.size() >= 100) { error = "Limite de 100 presets atingido."; return false; }
        items[name] = validator.preset();
        return write(items, error);
    }
    bool erase(const QString &name, QString &error) const
    {
        QJsonObject items;
        if (!read(items, error)) return false;
        if (!items.contains(name)) { error = "Preset não encontrado."; return false; }
        items.remove(name);
        return write(items, error);
    }
private:
    QString path;
    static bool validName(const QString &name)
    {
        if (name.isEmpty() || name.size() > 64 || name != name.trimmed()) return false;
        for (const auto c : name) if (c.category() == QChar::Other_Control) return false;
        return true;
    }
    bool write(const QJsonObject &items, QString &error) const
    {
        if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
            error = "Não foi possível criar a pasta da biblioteca."; return false;
        }
        QSaveFile file(path);
        const auto bytes = QJsonDocument(QJsonObject{{"format", "twitch-dock-themes-library"},
            {"version", 1}, {"presets", items}}).toJson();
        if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit()) {
            error = "Não foi possível salvar a biblioteca."; return false;
        }
        return true;
    }
};
