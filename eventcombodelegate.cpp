#include "eventcombodelegate.h"
#include <QComboBox>

EventComboDelegate::EventComboDelegate(const QStringList &options, QObject *parent)
    : QStyledItemDelegate(parent)
    , m_options(options)
{
}

QWidget *EventComboDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &,
                                          const QModelIndex &) const
{
    QComboBox *combo = new QComboBox(parent);
    combo->addItems(m_options);
    combo->setEditable(false);
    return combo;
}

void EventComboDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (!combo) {
        return;
    }
    QString value = index.data(Qt::EditRole).toString();
    int idx = combo->findText(value);
    if (idx < 0) {
        idx = 0;
    }
    combo->setCurrentIndex(idx);
}

void EventComboDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                      const QModelIndex &index) const
{
    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (!combo || !model) {
        return;
    }
    model->setData(index, combo->currentText(), Qt::EditRole);
}

void EventComboDelegate::updateEditorGeometry(QWidget *editor,
                                              const QStyleOptionViewItem &option,
                                              const QModelIndex &) const
{
    if (editor) {
        editor->setGeometry(option.rect);
    }
}
