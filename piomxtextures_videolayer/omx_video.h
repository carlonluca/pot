#ifndef OMX_VIDEO_H
#define OMX_VIDEO_H

#include <QObject>
#include <QQuickItem>

#include "omx_videolayer.h"
#include "omx_omxplayercontroller.h"

class MediaPlayer : public QObject
{
    Q_OBJECT
    Q_ENUMS(Status)
    Q_ENUMS(PlaybackState)
    Q_ENUMS(Error)

public:
    enum Status
    {
        UnknownStatus = QMediaPlayer::UnknownMediaStatus,
        NoMedia       = QMediaPlayer::NoMedia,
        Loading       = QMediaPlayer::LoadingMedia,
        Loaded        = QMediaPlayer::LoadedMedia,
        Stalled       = QMediaPlayer::StalledMedia,
        Buffering     = QMediaPlayer::BufferingMedia,
        Buffered      = QMediaPlayer::BufferedMedia,
        EndOfMedia    = QMediaPlayer::EndOfMedia,
        InvalidMedia  = QMediaPlayer::InvalidMedia
    };

    enum PlaybackState
    {
        PlayingState = QMediaPlayer::PlayingState,
        PausedState  = QMediaPlayer::PausedState,
        StoppedState = QMediaPlayer::StoppedState
    };

    enum Error
    {
        NoError        = QMediaPlayer::NoError,
        ResourceError  = QMediaPlayer::ResourceError,
        FormatError    = QMediaPlayer::FormatError,
        NetworkError   = QMediaPlayer::NetworkError,
        AccessDenied   = QMediaPlayer::AccessDeniedError,
        ServiceMissing = QMediaPlayer::ServiceMissingError
    };
};

class OMX_Video : public OMX_VideoLayer
{
    Q_OBJECT
    Q_PROPERTY(MediaPlayer::PlaybackState playbackState READ playbackState NOTIFY playbackStateChanged)
    Q_PROPERTY(MediaPlayer::Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(MediaPlayer::Error error READ error NOTIFY errorChanged)

public:
    OMX_Video(QQuickItem* parent = nullptr);

public slots:
    MediaPlayer::PlaybackState playbackState() const { return m_playbackState; }
    MediaPlayer::Status status() const { return m_status; }
    MediaPlayer::Error error() const { return MediaPlayer::NoError; }
    void setSource(QUrl url) override;
    void seek(qint64 millis);

signals:
    void playbackStateChanged(MediaPlayer::PlaybackState playbackState);
    void statusChanged(MediaPlayer::Status status);
    void playing();
    void errorChanged(MediaPlayer::Error error);

private:
    void setPlaybackState(QMediaPlayer::State state);
    void setStatus(QMediaPlayer::MediaStatus status);

private:
    MediaPlayer::PlaybackState m_playbackState;
    MediaPlayer::Status m_status;
};

#endif // OMX_VIDEO_H
