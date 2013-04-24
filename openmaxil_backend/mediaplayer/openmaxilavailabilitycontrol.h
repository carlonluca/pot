/*
 * Author:  Luca Carlon
 * Company: -
 * Date:    04.14.2013
 * Project: OpenMAXIL QtMultimedia Plugin
 */

#ifndef QOPENMAXILAVAILABILITYCONTROL_H
#define QOPENMAXILAVAILABILITYCONTROL_H

#include <QObject>
#include <qmediaavailabilitycontrol.h>

QT_BEGIN_NAMESPACE

class OpenMAXILAvailabilityControl : public QMediaAvailabilityControl
{
    Q_OBJECT
public:
    OpenMAXILAvailabilityControl(QObject* parent = 0);
    QMultimedia::AvailabilityStatus availability() const;
};

QT_END_NAMESPACE

#endif // QOPENMAXILAVAILABILITYCONTROL_H
