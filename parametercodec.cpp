#include "parametercodec.h"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonValue>

namespace {
bool parseBoolText(const QString &text, bool *ok)
{
    QString normalized = text.trimmed().toLower();
    if (normalized == "true" || normalized == "1" || normalized == "yes" || normalized == "on") {
        if (ok) *ok = true;
        return true;
    }
    if (normalized == "false" || normalized == "0" || normalized == "no" || normalized == "off") {
        if (ok) *ok = true;
        return false;
    }
    if (ok) *ok = false;
    return false;
}

QString toJsonString(const QJsonValue &value)
{
    QJsonDocument doc;
    if (value.isObject()) {
        doc = QJsonDocument(value.toObject());
    } else if (value.isArray()) {
        doc = QJsonDocument(value.toArray());
    } else {
        QJsonArray wrap;
        wrap.append(value);
        doc = QJsonDocument(wrap);
        QByteArray raw = doc.toJson(QJsonDocument::Compact);
        if (raw.startsWith('[') && raw.endsWith(']')) {
            raw = raw.mid(1, raw.size() - 2);
        }
        return QString::fromUtf8(raw);
    }
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}
}

QString ParameterCodec::valueTypeToString(ParameterValueType type)
{
    switch (type) {
    case ParameterValueType::Number:
        return "数字";
    case ParameterValueType::Boolean:
        return "布尔";
    case ParameterValueType::Json:
        return "JSON";
    case ParameterValueType::Path:
        return "路径";
    case ParameterValueType::String:
    default:
        return "文本";
    }
}

ParameterValueType ParameterCodec::valueTypeFromString(const QString &text)
{
    QString normalized = text.trimmed().toLower();
    if (normalized == "数字" || normalized == "number") {
        return ParameterValueType::Number;
    }
    if (normalized == "布尔" || normalized == "boolean" || normalized == "bool") {
        return ParameterValueType::Boolean;
    }
    if (normalized == "json") {
        return ParameterValueType::Json;
    }
    if (normalized == "路径" || normalized == "path") {
        return ParameterValueType::Path;
    }
    return ParameterValueType::String;
}

QStringList ParameterCodec::valueTypeNames()
{
    return QStringList() << "文本" << "数字" << "布尔" << "JSON" << "路径";
}

QList<ParameterRow> ParameterCodec::rowsFromMap(const QVariantMap &map)
{
    QList<ParameterRow> rows;
    for (auto it = map.constBegin(); it != map.constEnd(); ++it) {
        ParameterRow row;
        row.key = it.key();
        row.enabled = true;
        QVariant value = it.value();
        int typeId = value.type();
        if (typeId == QVariant::Bool) {
            row.type = ParameterValueType::Boolean;
            row.value = value.toBool() ? "true" : "false";
        } else if (typeId == QVariant::Int || typeId == QVariant::UInt
                   || typeId == QVariant::LongLong || typeId == QVariant::ULongLong
                   || typeId == QVariant::Double) {
            row.type = ParameterValueType::Number;
            row.value = QString::number(value.toDouble());
        } else if (typeId == QVariant::Map || typeId == QVariant::List) {
            row.type = ParameterValueType::Json;
            row.value = toJsonString(QJsonValue::fromVariant(value));
        } else {
            row.type = ParameterValueType::String;
            row.value = value.toString();
        }
        rows.append(row);
    }
    return rows;
}

QVariantMap ParameterCodec::mapFromRows(const QList<ParameterRow> &rows, QString *errorMessage)
{
    QVariantMap map;
    QString error;
    for (const ParameterRow &row : rows) {
        if (!row.enabled) {
            continue;
        }
        QString key = row.key.trimmed();
        if (key.isEmpty()) {
            continue;
        }
        QString valueText = row.value;
        if (valueText.trimmed().isEmpty()) {
            continue;
        }

        switch (row.type) {
        case ParameterValueType::Number: {
            bool ok = false;
            double number = valueText.toDouble(&ok);
            if (!ok) {
                error = QString("参数 %1 的数字格式无效").arg(key);
                map.insert(key, valueText);
            } else {
                map.insert(key, number);
            }
            break;
        }
        case ParameterValueType::Boolean: {
            bool ok = false;
            bool value = parseBoolText(valueText, &ok);
            if (!ok) {
                error = QString("参数 %1 的布尔格式无效").arg(key);
                map.insert(key, valueText);
            } else {
                map.insert(key, value);
            }
            break;
        }
        case ParameterValueType::Json: {
            QJsonParseError parseError;
            QJsonDocument doc = QJsonDocument::fromJson(valueText.toUtf8(), &parseError);
            if (parseError.error != QJsonParseError::NoError) {
                error = QString("参数 %1 的 JSON 无效").arg(key);
                map.insert(key, valueText);
            } else {
                QJsonValue value;
                if (doc.isObject()) {
                    value = doc.object();
                } else if (doc.isArray()) {
                    value = doc.array();
                } else {
                    value = QJsonValue();
                }
                map.insert(key, value.toVariant());
            }
            break;
        }
        case ParameterValueType::Path:
        case ParameterValueType::String:
        default:
            map.insert(key, valueText);
            break;
        }
    }
    if (errorMessage) {
        *errorMessage = error;
    }
    return map;
}

QString ParameterCodec::jsonFromMap(const QVariantMap &map)
{
    QJsonDocument doc = QJsonDocument::fromVariant(map);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Indented));
}

bool ParameterCodec::mapFromJson(const QString &text, QVariantMap *map, QString *errorMessage)
{
    if (text.trimmed().isEmpty()) {
        if (map) {
            map->clear();
        }
        if (errorMessage) {
            errorMessage->clear();
        }
        return true;
    }

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(text.toUtf8(), &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        if (errorMessage) {
            *errorMessage = QString("JSON 解析失败: %1").arg(parseError.errorString());
        }
        return false;
    }
    if (!doc.isObject()) {
        if (errorMessage) {
            *errorMessage = "JSON 需要是对象（{}）";
        }
        return false;
    }
    if (map) {
        *map = doc.object().toVariantMap();
    }
    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}
