#ifndef PARAMETERCODEC_H
#define PARAMETERCODEC_H

#include <QList>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

enum class ParameterValueType {
    String = 0,
    Number,
    Boolean,
    Json,
    Path
};

struct ParameterRow {
    bool enabled = true;
    QString key;
    ParameterValueType type = ParameterValueType::String;
    QString value;
    QString description;
};

namespace ParameterCodec {
QString valueTypeToString(ParameterValueType type);
ParameterValueType valueTypeFromString(const QString &text);
QStringList valueTypeNames();

QList<ParameterRow> rowsFromMap(const QVariantMap &map);
QVariantMap mapFromRows(const QList<ParameterRow> &rows, QString *errorMessage);

QString jsonFromMap(const QVariantMap &map);
bool mapFromJson(const QString &text, QVariantMap *map, QString *errorMessage);
}

#endif // PARAMETERCODEC_H
