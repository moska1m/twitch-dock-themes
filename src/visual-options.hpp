#pragma once
#include <QJsonObject>
#include <QStringList>

struct VisualOptions {
    QString icons = "native", feed = "native";
    bool focus = false, alerts = false;
    bool raids = true, subscriptions = true, mentions = true;
    int duration = 3;
    QJsonObject json() const
    {
        return {{"icons", icons}, {"feed", feed}, {"focus", focus}, {"alerts", alerts},
            {"raids", raids}, {"subscriptions", subscriptions}, {"mentions", mentions}, {"duration", duration}};
    }
    static bool parse(const QJsonObject &value, VisualOptions &out)
    {
        if (!QStringList{"native", "thin", "rounded"}.contains(value["icons"].toString()) ||
            !QStringList{"native", "compact", "cards"}.contains(value["feed"].toString())) return false;
        for (const auto &key : {"focus", "alerts", "raids", "subscriptions", "mentions"})
            if (!value[key].isBool()) return false;
        if (!value["duration"].isDouble() || value["duration"].toDouble() != value["duration"].toInt() ||
            value["duration"].toInt() < 1 || value["duration"].toInt() > 8) return false;
        VisualOptions result;
        result.icons = value["icons"].toString(); result.feed = value["feed"].toString();
        result.focus = value["focus"].toBool(); result.alerts = value["alerts"].toBool();
        result.raids = value["raids"].toBool(); result.subscriptions = value["subscriptions"].toBool();
        result.mentions = value["mentions"].toBool(); result.duration = value["duration"].toInt();
        out = result; return true;
    }
};
