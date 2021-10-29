#include "omx_video.h"

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
#define ASYNC(...) \
    QMetaObject::invokeMethod(__VA_ARGS__)

OMX_Video::OMX_Video(QQuickItem* parent) :
    OMX_VideoLayer(parent)
  , m_playbackState(static_cast<MediaPlayer::PlaybackState>(m_controller->playbackState()))
  , m_status(static_cast<MediaPlayer::Status>(m_controller->mediaStatus()))
{
    connect(m_controller, &OMX_OmxplayerController::playbackStateChanged, this, [this] (OMX_MediaPlayerState state) {
        setPlaybackState(state);
        if (state == QMediaPlayer::PlayingState)
            emit playing();
    });
    connect(m_controller, &OMX_OmxplayerController::mediaStatusChanged, this, [this] (QMediaPlayer::MediaStatus status) {
        setStatus(status);
    });
}

/*------------------------------------------------------------------------------
|    OMX_Video::setSource
+-----------------------------------------------------------------------------*/
void OMX_Video::setSource(QUrl source)
{
    if (m_source == source)
        return;

    log_debug_func;
    m_source = source;
    emit sourceChanged();

    m_controller->setSource(source);
    if (autoPlay())
        m_controller->play();
}

/*------------------------------------------------------------------------------
|    OMX_Video::seek
+-----------------------------------------------------------------------------*/
void OMX_Video::seek(qint64 millis)
{
    log_debug_func;
    m_controller->setPosition(millis*1000);
}

/*------------------------------------------------------------------------------
|    OMX_Video::setPlaybackState
+-----------------------------------------------------------------------------*/
void OMX_Video::setPlaybackState(OMX_MediaPlayerState state)
{
    if (state == static_cast<OMX_MediaPlayerState>(m_playbackState))
        return;
    m_playbackState = static_cast<MediaPlayer::PlaybackState>(state);
    emit playbackStateChanged(m_playbackState);
}

/*------------------------------------------------------------------------------
|    OMX_Video::setStatus
+-----------------------------------------------------------------------------*/
void OMX_Video::setStatus(QMediaPlayer::MediaStatus status)
{
    if (status == static_cast<QMediaPlayer::MediaStatus>(m_status))
        return;
    m_status = static_cast<MediaPlayer::Status>(status);
    emit statusChanged(m_status);
}
