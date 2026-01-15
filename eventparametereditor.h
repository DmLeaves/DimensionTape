#ifndef EVENTPARAMETEREDITOR_H
#define EVENTPARAMETEREDITOR_H

#include <QWidget>
#include <QVariantMap>
#include "eventtyperegistry.h"

class QFormLayout;
class QGroupBox;
class QLabel;
class QPlainTextEdit;
class QTableView;
class QPushButton;
class ParameterTableModel;
class ParameterTypeDelegate;

class EventParameterEditor : public QWidget
{
    Q_OBJECT

public:
    explicit EventParameterEditor(QWidget *parent = nullptr);

    void setSpec(const EventTypeSpec &spec);
    void setValues(const QString &target, const QVariantMap &parameters);
    QString target() const;
    QVariantMap parameters() const;

    bool validate(QString *errorMessage) const;

signals:
    void valuesChanged(const QString &target, const QVariantMap &parameters);
    void parseErrorChanged(const QString &message);

private slots:
    void onFormValueChanged();
    void onKvRowsChanged();
    void onRawTextChanged();
    void onAddKvRow();
    void onRemoveKvRow();
    void onRawToggled(bool checked);

private:
    struct FormField {
        EventFieldSpec spec;
        QWidget *editor = nullptr;
    };

    void rebuildForm();
    void clearForm();
    void updateFormFromValues();
    void updateKvFromValues();
    void updateRawFromValues();
    QString buildTargetFromForm() const;
    QVariantMap buildParametersFromForm() const;
    bool readFormValue(const FormField &field, QVariant &value) const;
    void setFormValue(const FormField &field, const QVariant &value);
    bool isValueEmpty(const QVariant &value, EventFieldValueType type) const;
    void setParseError(const QString &message);

    EventTypeSpec m_spec;
    QString m_target;
    QVariantMap m_parameters;
    QString m_parseError;
    bool m_updating = false;

    QGroupBox *m_formGroup = nullptr;
    QFormLayout *m_formLayout = nullptr;
    QList<FormField> m_formFields;

    QGroupBox *m_kvGroup = nullptr;
    QTableView *m_kvTable = nullptr;
    ParameterTableModel *m_kvModel = nullptr;
    ParameterTypeDelegate *m_typeDelegate = nullptr;
    QPushButton *m_addRowButton = nullptr;
    QPushButton *m_removeRowButton = nullptr;
    QLabel *m_kvErrorLabel = nullptr;

    QGroupBox *m_rawGroup = nullptr;
    QPlainTextEdit *m_rawEdit = nullptr;
    QLabel *m_rawErrorLabel = nullptr;
};

#endif // EVENTPARAMETEREDITOR_H
