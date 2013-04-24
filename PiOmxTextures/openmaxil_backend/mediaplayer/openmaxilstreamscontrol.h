/*
 * Author:  Luca Carlon
 * Company: -
 * Date:    04.14.2013
 * Project: OpenMAXIL QtMultimedia Plugin
 */

#ifndef QGSTREAMERSTREAMSCONTROL_H
#define QGSTREAMERSTREAMSCONTROL_H

#include <qmediastreamscontrol.h>

QT_BEGIN_NAMESPACE

class QGstreamerPlayerSession;

class QGstreamerStreamsControl : public QMediaStreamsControl
{
    Q_OBJECT
public:
    QGstreamerStreamsControl(QObject* parent);
    virtual ~QGstreamerStreamsControl();

    virtual int streamCount();
    virtual StreamType streamType(int streamNumber);

    virtual QVariant metaData(int streamNumber, const QString &key);

    virtual bool isActive(int streamNumber);
    virtual void setActive(int streamNumber, bool state);
};

QT_END_NAMESPACE

#endif // QGSTREAMERSTREAMSCONTROL_H

