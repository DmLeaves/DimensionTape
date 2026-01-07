#include "eventdetailpanel.h"
#include <QFormLayout>
#include <QLayout>
#include <QGroupBox>
#include <QLabel>
#include <QVBoxLayout>

EventDetailPanel::EventDetailPanel(QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    m_titleLabel = new QLabel(this);
    m_titleLabel->setStyleSheet("font-weight: bold; font-size: 13px;");
    layout->addWidget(m_titleLabel);

    m_descriptionLabel = new QLabel(this);
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setStyleSheet("color: #555555;");
    layout->addWidget(m_descriptionLabel);

    QGroupBox *inputGroup = new QGroupBox("输入说明", this);
    m_fieldsLayout = new QFormLayout(inputGroup);

    layout->addWidget(inputGroup);

    m_exampleLabel = new QLabel(this);
    m_exampleLabel->setWordWrap(true);
    m_exampleLabel->setStyleSheet("color: #444444;");
    layout->addWidget(m_exampleLabel);

    layout->addStretch();
    clear();
}

namespace {
void clearFormRows(QFormLayout *layout)
{
    if (!layout) {
        return;
    }
    while (layout->count() > 0) {
        QLayoutItem *item = layout->takeAt(0);
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
}
}

void EventDetailPanel::setSpec(const EventTypeSpec &spec)
{
    m_titleLabel->setText(spec.displayName);
    m_descriptionLabel->setText(spec.description);

    clearFormRows(m_fieldsLayout);

    if (spec.fields.isEmpty()) {
        QLabel *emptyLabel = new QLabel("无输入项", this);
        QLabel *emptyHint = new QLabel(" ", this);
        emptyHint->setStyleSheet("color: #666666;");
        m_fieldsLayout->addRow(emptyLabel, emptyHint);
    } else {
        for (const EventFieldSpec &field : spec.fields) {
            QString title = field.label;
            if (field.optional) {
                title += "（可选）";
            }
            QLabel *fieldTitle = new QLabel(title, this);
            QLabel *fieldHint = new QLabel(field.hint.isEmpty() ? " " : field.hint, this);
            fieldHint->setWordWrap(true);
            fieldHint->setStyleSheet("color: #666666;");
            m_fieldsLayout->addRow(fieldTitle, fieldHint);
        }
    }

    if (spec.example.isEmpty()) {
        m_exampleLabel->setVisible(false);
    } else {
        m_exampleLabel->setVisible(true);
        m_exampleLabel->setText(spec.example);
    }
}

void EventDetailPanel::clear()
{
    m_titleLabel->setText("事件说明");
    m_descriptionLabel->setText("请选择一个事件查看输入说明。");
    clearFormRows(m_fieldsLayout);
    QLabel *emptyLabel = new QLabel(" ", this);
    QLabel *emptyHint = new QLabel(" ", this);
    emptyHint->setStyleSheet("color: #666666;");
    if (m_fieldsLayout) {
        m_fieldsLayout->addRow(emptyLabel, emptyHint);
    }
    m_exampleLabel->setVisible(false);
}
