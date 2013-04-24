/*
 * Author:  Luca Carlon
 * Company: -
 * Date:    04.14.2013
 * Project: OpenMAXIL QtMultimedia Plugin
 */

#ifndef QGSTREAMERPLAYERSERVICEPLUGIN_H
#define QGSTREAMERPLAYERSERVICEPLUGIN_H

#include <qmediaserviceproviderplugin.h>
#include <QtCore/qset.h>
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE


class QGstreamerPlayerServicePlugin
    : public QMediaServiceProviderPlugin
    , public QMediaServiceFeaturesInterface
    , public QMediaServiceSupportedFormatsInterface
{
    Q_OBJECT
    Q_INTERFACES(QMediaServiceFeaturesInterface)
    Q_INTERFACES(QMediaServiceSupportedFormatsInterface)
    Q_PLUGIN_METADATA(IID "org.qt-project.qt.mediaserviceproviderfactory/5.0" FILE "mediaplayer.json")
public:
    QMediaService* create(QString const& key);
    void release(QMediaService *service);

    QMediaServiceProviderHint::Features supportedFeatures(const QByteArray &service) const;

    QMultimedia::SupportEstimate hasSupport(const QString &mimeType, const QStringList& codecs) const;
    QStringList supportedMimeTypes() const;

private:
    void updateSupportedMimeTypes() const;

    mutable QSet<QString> m_supportedMimeTypeSet; //for fast access
};

QT_END_NAMESPACE

#endif // QGSTREAMERPLAYERSERVICEPLUGIN_H

