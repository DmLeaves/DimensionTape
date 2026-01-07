#include "parametertypedelegate.h"
#include "parametercodec.h"
#include <QComboBox>

ParameterTypeDelegate::ParameterTypeDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *ParameterTypeDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &,
                                             const QModelIndex &) const
{
    QComboBox *combo = new QComboBox(parent);
    combo->addItems(ParameterCodec::valueTypeNames());
    return combo;
}

void ParameterTypeDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (!combo) {
        return;
    }
    QString current = index.data(Qt::EditRole).toString();
    int idx = combo->findText(current);
    if (idx < 0) {
        idx = 0;
    }
    combo->setCurrentIndex(idx);
}

void ParameterTypeDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                         const QModelIndex &index) const
{
    QComboBox *combo = qobject_cast<QComboBox *>(editor);
    if (!combo) {
        return;
    }
    model->setData(index, combo->currentText(), Qt::EditRole);
}
