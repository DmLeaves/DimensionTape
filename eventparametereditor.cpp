#include "eventparametereditor.h"
#include "parametercodec.h"
#include "parametertablemodel.h"
#include "parametertypedelegate.h"
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTableView>
#include <QVBoxLayout>

namespace {
ParameterValueType toParameterValueType(EventFieldValueType type)
{
    switch (type) {
    case EventFieldValueType::Number:
        return ParameterValueType::Number;
    case EventFieldValueType::Boolean:
        return ParameterValueType::Boolean;
    case EventFieldValueType::Multiline:
    case EventFieldValueType::Text:
    default:
        return ParameterValueType::String;
    }
}
}

EventParameterEditor::EventParameterEditor(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    m_formGroup = new QGroupBox("表单", this);
    QVBoxLayout *formGroupLayout = new QVBoxLayout(m_formGroup);
    QWidget *formContainer = new QWidget(m_formGroup);
    m_formLayout = new QFormLayout(formContainer);
    m_formLayout->setContentsMargins(0, 0, 0, 0);
    formGroupLayout->addWidget(formContainer);
    formGroupLayout->addStretch();
    mainLayout->addWidget(m_formGroup);

    m_kvGroup = new QGroupBox("参数 (KV)", this);
    QVBoxLayout *kvLayout = new QVBoxLayout(m_kvGroup);
    m_kvTable = new QTableView(m_kvGroup);
    m_kvModel = new ParameterTableModel(this);
    m_typeDelegate = new ParameterTypeDelegate(this);
    m_kvTable->setModel(m_kvModel);
    m_kvTable->setItemDelegateForColumn(ParameterTableModel::TypeColumn, m_typeDelegate);
    m_kvTable->horizontalHeader()->setStretchLastSection(true);
    kvLayout->addWidget(m_kvTable);

    QHBoxLayout *kvButtons = new QHBoxLayout;
    m_addRowButton = new QPushButton("添加", m_kvGroup);
    m_removeRowButton = new QPushButton("删除", m_kvGroup);
    kvButtons->addWidget(m_addRowButton);
    kvButtons->addWidget(m_removeRowButton);
    kvButtons->addStretch();
    kvLayout->addLayout(kvButtons);

    m_kvErrorLabel = new QLabel(m_kvGroup);
    m_kvErrorLabel->setStyleSheet("color: #c0392b;");
    m_kvErrorLabel->setText(" ");
    kvLayout->addWidget(m_kvErrorLabel);
    mainLayout->addWidget(m_kvGroup);

    m_rawGroup = new QGroupBox("Raw JSON", this);
    m_rawGroup->setCheckable(true);
    m_rawGroup->setChecked(false);
    QVBoxLayout *rawLayout = new QVBoxLayout(m_rawGroup);
    m_rawEdit = new QPlainTextEdit(m_rawGroup);
    m_rawEdit->setPlaceholderText("{\n  \"key\": \"value\"\n}");
    rawLayout->addWidget(m_rawEdit);
    m_rawErrorLabel = new QLabel(m_rawGroup);
    m_rawErrorLabel->setStyleSheet("color: #c0392b;");
    m_rawErrorLabel->setText(" ");
    rawLayout->addWidget(m_rawErrorLabel);
    mainLayout->addWidget(m_rawGroup);
    m_rawEdit->setVisible(false);
    m_rawErrorLabel->setVisible(false);

    connect(m_addRowButton, &QPushButton::clicked, this, &EventParameterEditor::onAddKvRow);
    connect(m_removeRowButton, &QPushButton::clicked, this, &EventParameterEditor::onRemoveKvRow);
    connect(m_kvModel, &ParameterTableModel::rowsChanged, this, &EventParameterEditor::onKvRowsChanged);
    connect(m_rawEdit, &QPlainTextEdit::textChanged, this, &EventParameterEditor::onRawTextChanged);
    connect(m_rawGroup, &QGroupBox::toggled, this, &EventParameterEditor::onRawToggled);

    setSpec(EventTypeSpec());
    setValues(QString(), QVariantMap());
}

void EventParameterEditor::setSpec(const EventTypeSpec &spec)
{
    m_spec = spec;
    rebuildForm();
    setValues(QString(), QVariantMap());
}

void EventParameterEditor::setValues(const QString &target, const QVariantMap &parameters)
{
    m_updating = true;
    m_target = target;
    m_parameters = parameters;
    updateFormFromValues();
    updateKvFromValues();
    updateRawFromValues();
    setParseError(QString());
    m_updating = false;
}

QString EventParameterEditor::target() const
{
    return m_target;
}

QVariantMap EventParameterEditor::parameters() const
{
    return m_parameters;
}

bool EventParameterEditor::validate(QString *errorMessage) const
{
    if (!m_parseError.isEmpty()) {
        if (errorMessage) {
            *errorMessage = m_parseError;
        }
        return false;
    }

    for (const FormField &field : m_formFields) {
        QVariant value;
        if (!readFormValue(field, value)) {
            continue;
        }
        if (!field.spec.optional && isValueEmpty(value, field.spec.valueType)) {
            if (errorMessage) {
                *errorMessage = QString("请输入 %1").arg(field.spec.label);
            }
            return false;
        }
    }
    if (errorMessage) {
        errorMessage->clear();
    }
    return true;
}

void EventParameterEditor::onFormValueChanged()
{
    if (m_updating) {
        return;
    }
    m_updating = true;
    m_target = buildTargetFromForm();
    m_parameters = buildParametersFromForm();
    updateKvFromValues();
    updateRawFromValues();
    setParseError(QString());
    m_updating = false;
    emit valuesChanged(m_target, m_parameters);
}

void EventParameterEditor::onKvRowsChanged()
{
    if (m_updating) {
        return;
    }
    m_updating = true;
    QString error;
    QVariantMap map = ParameterCodec::mapFromRows(m_kvModel->rows(), &error);
    m_parameters = map;
    updateFormFromValues();
    updateRawFromValues();
    setParseError(error);
    m_updating = false;
    emit valuesChanged(m_target, m_parameters);
}

void EventParameterEditor::onRawTextChanged()
{
    if (m_updating) {
        return;
    }
    m_updating = true;
    QString error;
    QVariantMap map;
    if (ParameterCodec::mapFromJson(m_rawEdit->toPlainText(), &map, &error)) {
        m_parameters = map;
        updateFormFromValues();
        updateKvFromValues();
        setParseError(QString());
        emit valuesChanged(m_target, m_parameters);
    } else {
        setParseError(error);
    }
    m_updating = false;
}

void EventParameterEditor::onAddKvRow()
{
    ParameterRow row;
    row.enabled = true;
    row.type = ParameterValueType::String;
    m_kvModel->addRow(row);
}

void EventParameterEditor::onRemoveKvRow()
{
    int row = m_kvTable ? m_kvTable->currentIndex().row() : -1;
    if (row >= 0) {
        m_kvModel->removeRow(row);
    }
}

void EventParameterEditor::onRawToggled(bool checked)
{
    if (m_rawEdit) {
        m_rawEdit->setVisible(checked);
    }
    if (m_rawErrorLabel) {
        m_rawErrorLabel->setVisible(checked);
    }
    if (checked) {
        m_updating = true;
        updateRawFromValues();
        m_updating = false;
    }
}

void EventParameterEditor::rebuildForm()
{
    clearForm();
    if (!m_formLayout) {
        return;
    }

    for (const EventFieldSpec &field : m_spec.fields) {
        QWidget *editor = nullptr;
        switch (field.valueType) {
        case EventFieldValueType::Multiline: {
            QPlainTextEdit *edit = new QPlainTextEdit(m_formGroup);
            edit->setPlaceholderText(field.placeholder);
            edit->setFixedHeight(70);
            editor = edit;
            connect(edit, &QPlainTextEdit::textChanged, this, &EventParameterEditor::onFormValueChanged);
            break;
        }
        case EventFieldValueType::Number: {
            QDoubleSpinBox *spin = new QDoubleSpinBox(m_formGroup);
            spin->setRange(-999999.0, 999999.0);
            spin->setDecimals(3);
            editor = spin;
            connect(spin, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
                    this, &EventParameterEditor::onFormValueChanged);
            break;
        }
        case EventFieldValueType::Boolean: {
            QCheckBox *check = new QCheckBox(m_formGroup);
            editor = check;
            connect(check, &QCheckBox::toggled, this, &EventParameterEditor::onFormValueChanged);
            break;
        }
        case EventFieldValueType::Text:
        default: {
            QLineEdit *edit = new QLineEdit(m_formGroup);
            edit->setPlaceholderText(field.placeholder);
            editor = edit;
            connect(edit, &QLineEdit::textChanged, this, &EventParameterEditor::onFormValueChanged);
            break;
        }
        }
        if (!editor) {
            continue;
        }
        editor->setToolTip(field.hint);
        QString labelText = field.label;
        if (field.optional) {
            labelText += "（可选）";
        }
        m_formLayout->addRow(labelText + ":", editor);
        FormField entry;
        entry.spec = field;
        entry.editor = editor;
        m_formFields.append(entry);
    }
}

void EventParameterEditor::clearForm()
{
    if (!m_formLayout) {
        return;
    }
    while (m_formLayout->count() > 0) {
        QLayoutItem *item = m_formLayout->takeAt(0);
        if (!item) {
            break;
        }
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        } else if (QLayout *childLayout = item->layout()) {
            delete childLayout;
        }
        delete item;
    }
    m_formFields.clear();
}

void EventParameterEditor::updateFormFromValues()
{
    for (const FormField &field : m_formFields) {
        if (field.spec.binding == EventFieldBinding::Target) {
            setFormValue(field, m_target);
        } else {
            setFormValue(field, m_parameters.value(field.spec.key));
        }
    }
}

void EventParameterEditor::updateKvFromValues()
{
    if (!m_kvModel) {
        return;
    }
    QList<ParameterRow> rows = ParameterCodec::rowsFromMap(m_parameters);
    QHash<QString, int> keyIndex;
    for (int i = 0; i < rows.size(); ++i) {
        keyIndex.insert(rows[i].key, i);
    }
    for (const EventFieldSpec &field : m_spec.fields) {
        if (field.binding != EventFieldBinding::Parameter) {
            continue;
        }
        int index = keyIndex.value(field.key, -1);
        if (index < 0) {
            ParameterRow row;
            row.enabled = true;
            row.key = field.key;
            row.type = toParameterValueType(field.valueType);
            row.value = QString();
            row.description = field.hint;
            rows.append(row);
        } else {
            rows[index].description = field.hint;
        }
    }
    m_kvModel->setRows(rows);
}

void EventParameterEditor::updateRawFromValues()
{
    if (!m_rawEdit) {
        return;
    }
    m_rawEdit->setPlainText(ParameterCodec::jsonFromMap(m_parameters));
}

QString EventParameterEditor::buildTargetFromForm() const
{
    QStringList values;
    for (const FormField &field : m_formFields) {
        if (field.spec.binding != EventFieldBinding::Target) {
            continue;
        }
        QVariant value;
        if (!readFormValue(field, value)) {
            continue;
        }
        QString text = value.toString().trimmed();
        if (!text.isEmpty()) {
            values << text;
        }
    }
    return values.join(" ");
}

QVariantMap EventParameterEditor::buildParametersFromForm() const
{
    QVariantMap map;
    for (const FormField &field : m_formFields) {
        if (field.spec.binding != EventFieldBinding::Parameter) {
            continue;
        }
        QVariant value;
        if (!readFormValue(field, value)) {
            continue;
        }
        if (field.spec.optional && isValueEmpty(value, field.spec.valueType)) {
            continue;
        }
        map.insert(field.spec.key, value);
    }
    return map;
}

bool EventParameterEditor::readFormValue(const FormField &field, QVariant &value) const
{
    if (!field.editor) {
        return false;
    }
    switch (field.spec.valueType) {
    case EventFieldValueType::Multiline: {
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit *>(field.editor);
        value = edit ? edit->toPlainText() : QString();
        return true;
    }
    case EventFieldValueType::Number: {
        QDoubleSpinBox *spin = qobject_cast<QDoubleSpinBox *>(field.editor);
        value = spin ? spin->value() : 0.0;
        return true;
    }
    case EventFieldValueType::Boolean: {
        QCheckBox *check = qobject_cast<QCheckBox *>(field.editor);
        value = check ? check->isChecked() : false;
        return true;
    }
    case EventFieldValueType::Text:
    default: {
        QLineEdit *edit = qobject_cast<QLineEdit *>(field.editor);
        value = edit ? edit->text() : QString();
        return true;
    }
    }
}

void EventParameterEditor::setFormValue(const FormField &field, const QVariant &value)
{
    if (!field.editor) {
        return;
    }
    switch (field.spec.valueType) {
    case EventFieldValueType::Multiline: {
        QPlainTextEdit *edit = qobject_cast<QPlainTextEdit *>(field.editor);
        if (edit) {
            edit->setPlainText(value.toString());
        }
        break;
    }
    case EventFieldValueType::Number: {
        QDoubleSpinBox *spin = qobject_cast<QDoubleSpinBox *>(field.editor);
        if (spin) {
            spin->setValue(value.toDouble());
        }
        break;
    }
    case EventFieldValueType::Boolean: {
        QCheckBox *check = qobject_cast<QCheckBox *>(field.editor);
        if (check) {
            check->setChecked(value.toBool());
        }
        break;
    }
    case EventFieldValueType::Text:
    default: {
        QLineEdit *edit = qobject_cast<QLineEdit *>(field.editor);
        if (edit) {
            edit->setText(value.toString());
        }
        break;
    }
    }
}

bool EventParameterEditor::isValueEmpty(const QVariant &value, EventFieldValueType type) const
{
    switch (type) {
    case EventFieldValueType::Text:
    case EventFieldValueType::Multiline:
        return value.toString().trimmed().isEmpty();
    case EventFieldValueType::Number:
    case EventFieldValueType::Boolean:
    default:
        return false;
    }
}

void EventParameterEditor::setParseError(const QString &message)
{
    if (m_parseError == message) {
        return;
    }
    m_parseError = message;
    QString display = message.isEmpty() ? " " : message;
    if (m_kvErrorLabel) {
        m_kvErrorLabel->setText(display);
    }
    if (m_rawErrorLabel) {
        m_rawErrorLabel->setText(display);
    }
    emit parseErrorChanged(message);
}
