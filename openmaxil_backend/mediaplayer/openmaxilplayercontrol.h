/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    04.14.2013
 *
 * Copyright (c) 2012, 2013 Luca Carlon. All rights reserved.
 *
 * This file is part of PiOmxTextures.
 *
 * PiOmxTextures is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PiOmxTextures is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with PiOmxTextures. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QOPENMAXILPLAYERCONTROL_H
#define QOPENMAXILPLAYERCONTROL_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QtCore/qobject.h>
#include <QtCore/qstack.h>
#include <QtCore/qsemaphore.h>
#include <QtCore/qmutex.h>

#include <qmediacontent.h>
#include <qmediaplayer.h>
#include <qmediaplayercontrol.h>

#include <limits.h>

#include <omx_mediaprocessor.h>
#include <omx_textureprovider.h>

QT_BEGIN_NAMESPACE

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
class QMediaPlaylist;
class QMediaPlaylistNavigator;
class QSocketNotifier;

enum PlayerCommandType {
   PLAYER_COMMAND_TYPE_SET_MEDIA,
   PLAYER_COMMAND_TYPE_PLAY,
   PLAYER_COMMAND_TYPE_PAUSE,
   PLAYER_COMMAND_TYPE_STOP,
   PLAYER_COMMAND_TYPE_FREE_TEXTURE_DATA
};


/*------------------------------------------------------------------------------
|    PlayerCommand class
+-----------------------------------------------------------------------------*/
struct PlayerCommand
{
   virtual ~PlayerCommand() {}

   PlayerCommandType m_playerCommandType;
};

/*------------------------------------------------------------------------------
|    PlayerCommandSetMedia class
+-----------------------------------------------------------------------------*/
struct PlayerCommandSetMedia : public PlayerCommand
{
   virtual ~PlayerCommandSetMedia() {}

   QMediaContent m_mediaContent;
};

/*------------------------------------------------------------------------------
|    PlayerCommandPlay class
+-----------------------------------------------------------------------------*/
struct PlayerCommandPlay : public PlayerCommand
{
   virtual ~PlayerCommandPlay() {}

   QSize m_mediaSize;
};

/*------------------------------------------------------------------------------
|    PlayerCommandPause class
+-----------------------------------------------------------------------------*/
struct PlayerCommandPause : public PlayerCommand
{
   virtual ~PlayerCommandPause() {}
};

/*------------------------------------------------------------------------------
|    PlayerCommandStop class
+-----------------------------------------------------------------------------*/
struct PlayerCommandStop : public PlayerCommand
{
   virtual ~PlayerCommandStop() {}
};

/*------------------------------------------------------------------------------
|    PlayerCommandStop class
+-----------------------------------------------------------------------------*/
struct PlayerCommandFreeTextureData : public PlayerCommand
{
   virtual ~PlayerCommandFreeTextureData() {}

   OMX_TextureData* m_textureData;
};

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl class
+-----------------------------------------------------------------------------*/
class OpenMAXILPlayerControl : public QMediaPlayerControl
{
    Q_OBJECT
public:
    OpenMAXILPlayerControl(QObject* parent = 0);
    ~OpenMAXILPlayerControl();

    void setMediaPlayer(QMediaPlayer* mediaPlayer);
    QMediaPlayer::State state() const;
    QMediaPlayer::MediaStatus mediaStatus() const;

    qint64 position() const;
    qint64 duration() const;

    int bufferStatus() const;

    int volume() const;
    bool isMuted() const;

    bool isAudioAvailable() const;
    bool isVideoAvailable() const;

    bool isSeekable() const;
    QMediaTimeRange availablePlaybackRanges() const;

    qreal playbackRate() const;
    void setPlaybackRate(qreal rate);

    QMediaContent media() const;
    const QIODevice* mediaStream() const;

    void setMedia(const QMediaContent&, QIODevice*);
    void setMediaInt(const QMediaContent& mediaContent);

    QVariantMap getMetaData();

public Q_SLOTS:
    void setPosition(qint64 pos);

    void play();
    void pause();
    void stop();

    void setVolume(int volume);
    void setMuted(bool muted);

    void onSceneGraphInitialized();
    void onAfterRendering();

signals:
    void metaDataChanged(const QVariantMap metaData);
    void textureReady(const OMX_TextureData* textureData);
    void textureInvalidated();

private slots:
    void onStateChanged(OMX_MediaProcessor::OMX_MediaProcessorState state);
    void onItemSceneChanged();
    void playInt();
    void pauseInt();
    void stopInt();

private:
    void appendCommand(PlayerCommand* command);
    void processCommands();
    QMediaPlayer::State convertState(OMX_MediaProcessor::OMX_MediaProcessorState state) const;

    bool m_ownStream;
    QMediaPlayer::MediaStatus m_mediaStatus;
    QStack<QMediaPlayer::State> m_stateStack;
    QStack<QMediaPlayer::MediaStatus> m_mediaStatusStack;

    bool m_seekToStartPending;
    qint64 m_pendingSeekPosition;
    QMediaContent m_currentResource;

    OMX_TextureProvider* m_texProvider;
    OMX_MediaProcessor*  m_mediaProcessor;
    OMX_TextureData*     m_textureData;

    QList<PlayerCommand*> m_pendingCommands;
    QMutex                m_pendingCommandsMutex;
    bool                  m_sceneGraphInitialized;

    QMediaPlayer* m_mediaPlayer;
    QQuickItem*   m_quickItem;
};

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::convertState
+-----------------------------------------------------------------------------*/
inline
QMediaPlayer::State OpenMAXILPlayerControl::convertState(OMX_MediaProcessor::OMX_MediaProcessorState state) const
{
   switch (state) {
   case OMX_MediaProcessor::STATE_STOPPED:
      return QMediaPlayer::StoppedState;
   case OMX_MediaProcessor::STATE_INACTIVE:
      return QMediaPlayer::StoppedState;
   case OMX_MediaProcessor::STATE_PAUSED:
      return QMediaPlayer::PausedState;
   case OMX_MediaProcessor::STATE_PLAYING:
      return QMediaPlayer::PlayingState;
   default:
      break;
   }

   return QMediaPlayer::StoppedState;
}

QT_END_NAMESPACE

#endif // QOPENMAXILPLAYERCONTROL_H
