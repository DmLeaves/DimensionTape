#ifndef EVENTCOMBODELEGATE_H
#define EVENTCOMBODELEGATE_H

#include <QStyledItemDelegate>
#include <QStringList>

class EventComboDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit EventComboDelegate(const QStringList &options, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                          const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                      const QModelIndex &index) const override;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                              const QModelIndex &index) const override;

private:
    QStringList m_options;
};

#endif // EVENTCOMBODELEGATE_H
