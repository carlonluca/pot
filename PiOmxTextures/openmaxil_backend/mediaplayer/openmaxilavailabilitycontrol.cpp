/*
 * Author:  Luca Carlon
 * Company: -
 * Date:    04.14.2013
 * Project: OpenMAXIL QtMultimedia Plugin
 */

#include "openmaxilavailabilitycontrol.h"

QT_BEGIN_NAMESPACE

OpenMAXILAvailabilityControl::OpenMAXILAvailabilityControl(QObject* parent)
   : QMediaAvailabilityControl(parent)
{
}

QMultimedia::AvailabilityStatus OpenMAXILAvailabilityControl::availability() const
{
   // TODO: Implement so that only one media can be played at a time.
   return QMultimedia::Available;
}

QT_END_NAMESPACE
