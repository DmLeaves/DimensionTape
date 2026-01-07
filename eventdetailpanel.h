#ifndef EVENTDETAILPANEL_H
#define EVENTDETAILPANEL_H

#include <QWidget>
#include "eventtyperegistry.h"

class QLabel;
class QFormLayout;

class EventDetailPanel : public QWidget
{
    Q_OBJECT

public:
    explicit EventDetailPanel(QWidget *parent = nullptr);

    void setSpec(const EventTypeSpec &spec);
    void clear();

private:
    QLabel *m_titleLabel = nullptr;
    QLabel *m_descriptionLabel = nullptr;
    QFormLayout *m_fieldsLayout = nullptr;
    QLabel *m_exampleLabel = nullptr;
};

#endif // EVENTDETAILPANEL_H
