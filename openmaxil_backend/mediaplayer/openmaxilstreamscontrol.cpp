/*
 * Author:  Luca Carlon
 * Company: -
 * Date:    04.14.2013
 * Project: OpenMAXIL QtMultimedia Plugin
 */

#include "openmaxilstreamscontrol.h"

QGstreamerStreamsControl::QGstreamerStreamsControl(QObject *parent)
   :QMediaStreamsControl(parent)
{
}

QGstreamerStreamsControl::~QGstreamerStreamsControl()
{
}

int QGstreamerStreamsControl::streamCount()
{
   // TODO: Implement.
   return 0;
}

QMediaStreamsControl::StreamType QGstreamerStreamsControl::streamType(int streamNumber)
{
   // TODO: Implement.
   return UnknownStream;
}

QVariant QGstreamerStreamsControl::metaData(int streamNumber, const QString &key)
{
   // TODO: Implement.
   return 0;
}

bool QGstreamerStreamsControl::isActive(int streamNumber)
{
   // TODO: Implement.
   return true;
}

void QGstreamerStreamsControl::setActive(int streamNumber, bool state)
{
   // TODO: Implement.
}

