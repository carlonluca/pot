/*
 * Author:  Luca Carlon
 * Company: -
 * Date:    04.14.2013
 * Project: OpenMAXIL QtMultimedia Plugin
 */

#ifndef QGSTREAMERMETADATAPROVIDER_H
#define QGSTREAMERMETADATAPROVIDER_H

#include <qmetadatareadercontrol.h>

QT_BEGIN_NAMESPACE

class QGstreamerPlayerSession;

class QGstreamerMetaDataProvider : public QMetaDataReaderControl
{
    Q_OBJECT
public:
    QGstreamerMetaDataProvider(QObject* parent);
    virtual ~QGstreamerMetaDataProvider();

    bool isMetaDataAvailable() const;
    bool isWritable() const;

    QVariant metaData(const QString &key) const;
    QStringList availableMetaData() const;

private slots:
    void updateTags();

private:
    QVariantMap m_tags;
    QMap<QByteArray, QString> m_keysMap;
};

QT_END_NAMESPACE

#endif // QGSTREAMERMETADATAPROVIDER_H
