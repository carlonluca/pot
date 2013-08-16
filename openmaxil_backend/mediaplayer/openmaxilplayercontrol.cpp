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

#include "openmaxilplayercontrol.h"

#include <private/qmediaplaylistnavigator_p.h>
#include <private/qmediaresourcepolicy_p.h>
#include <private/qmediaresourceset_p.h>

#include <QtCore/qdir.h>
#include <QtCore/qsocketnotifier.h>
#include <QtCore/qurl.h>
#include <QtCore/qdebug.h>
#include <QtCore/qfiledevice.h>
#include <QtCore/qtimer.h>
#include <QtMultimedia/qmediacontent.h>
#include <QtQuick/qquickview.h>
#include <QtQuick/qquickitem.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <omx_mediaprocessor.h>
#include <omx_textureprovider.h>

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
//#define DEBUG_PLAYBIN
#define DEBUG_PLAYER_CONTROL

#ifndef DEBUG_PLAYER_CONTROL
#ifdef LOG_DEBUG
#undef LOG_DEBUG
#define LOG_DEBUG(...) {(void)0}
#endif // LOG_DEBUG
#else
#include <lgl_logging.h>
#endif // DEBUG_PLAYER_CONTROL


QT_BEGIN_NAMESPACE

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::OpenMAXILPlayerControl
+-----------------------------------------------------------------------------*/
OpenMAXILPlayerControl::OpenMAXILPlayerControl(QObject *parent)
   : QMediaPlayerControl(parent)
   , m_ownStream(false)
   , m_seekToStartPending(false)
   , m_pendingSeekPosition(-1)
   , m_mediaProcessor(new OMX_MediaProcessor(this))
   , m_textureData(NULL)
   , m_sceneGraphInitialized(false)
   , m_quickItem(NULL)
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   connect(m_mediaProcessor, SIGNAL(textureReady(const OMX_TextureData*)),
           this, SIGNAL(textureReady(const OMX_TextureData*)));
   connect(m_mediaProcessor, SIGNAL(textureInvalidated()),
           this, SIGNAL(textureInvalidated()));
   connect(m_mediaProcessor, SIGNAL(stateChanged(OMX_MediaProcessor::OMX_MediaProcessorState)),
           this, SLOT(onStateChanged(OMX_MediaProcessor::OMX_MediaProcessorState)));
   connect(m_mediaProcessor, SIGNAL(metadataChanged(QVariantMap)),
           this, SIGNAL(metaDataChanged(QVariantMap)));
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::~OpenMAXILPlayerControl
+-----------------------------------------------------------------------------*/
OpenMAXILPlayerControl::~OpenMAXILPlayerControl()
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   delete m_mediaProcessor;
   m_mediaProcessor = NULL;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setMediaPlayer
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setMediaPlayer(QMediaPlayer* mediaPlayer)
{
   LOG_DEBUG(LOG_TAG, "Setting QMediaPlayer...");
   m_quickItem = dynamic_cast<QQuickItem*>(mediaPlayer->parent());
   if (!m_quickItem) {
      LOG_ERROR(LOG_TAG, "Failed to get declarative media player.");
      return;
   }

   connect(mediaPlayer, SIGNAL(itemSceneChanged()), this, SLOT(onItemSceneChanged()));

   // Immediately set if already available.
   if (m_quickItem->window())
      onItemSceneChanged();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::play
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::play()
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   LOG_VERBOSE(LOG_TAG, "Deferring play() command...");
   PlayerCommandPlay* play = new PlayerCommandPlay;
   play->m_playerCommandType = PLAYER_COMMAND_TYPE_PLAY;
   appendCommand(play);
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::playInt
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::playInt()
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // Can be done in any thread.
   assert(m_mediaProcessor);
   m_mediaProcessor->play();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::pause
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::pause()
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   PlayerCommandPause* pause = new PlayerCommandPause;
   pause->m_playerCommandType = PLAYER_COMMAND_TYPE_PAUSE;
   appendCommand(pause);
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::pauseInt
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::pauseInt()
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   assert(m_mediaProcessor);
   m_mediaProcessor->pause();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::stop
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::stop()
{
   LOG_DEBUG("%s", Q_FUNC_INFO);

   assert(m_mediaProcessor);
   m_mediaProcessor->stop();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::stopInt
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::stopInt()
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // Can be done in any thread.
   assert(m_mediaProcessor); 
   m_mediaProcessor->stop();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::onSceneGraphInitialized
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::onSceneGraphInitialized()
{
   LOG_DEBUG(LOG_TAG, "Renderer thread is: 0x%x.", (unsigned int)QThread::currentThread());

   m_sceneGraphInitialized = true;
   processCommands();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::onBeforeRendering
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::onAfterRendering()
{
   processCommands();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::onStateChanged
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::onStateChanged(OMX_MediaProcessor::OMX_MediaProcessorState state)
{
   LOG_DEBUG(LOG_TAG, "State changed...");
   emit stateChanged(convertState(state));
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::onItemSceneChanged
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::onItemSceneChanged()
{
   LOG_DEBUG(LOG_TAG, "Getting window...");
   QQuickWindow* window = m_quickItem->window();
   if (!window) {
      LOG_ERROR(LOG_TAG, "Failed to get QQuickWindow.");
      return;
   }

   connect(window, SIGNAL(sceneGraphInitialized()),
           this, SLOT(onSceneGraphInitialized()), Qt::DirectConnection);
   connect(window, SIGNAL(afterRendering()),
           this, SLOT(onAfterRendering()), Qt::DirectConnection);
   window->update();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::appendCommand
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::appendCommand(PlayerCommand* command)
{
   QMutexLocker locker(&m_pendingCommandsMutex);
   m_pendingCommands.append(command);
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::pendingCommands
+-----------------------------------------------------------------------------*/
inline
void OpenMAXILPlayerControl::processCommands()
{
   QMutexLocker locker(&m_pendingCommandsMutex);

   // Go through all the commands and execute.
   for (int i = 0; i < m_pendingCommands.size(); i++) {
      PlayerCommand* command = m_pendingCommands.at(i);

      if (command->m_playerCommandType == PLAYER_COMMAND_TYPE_SET_MEDIA) {
         LOG_VERBOSE(LOG_TAG, "Processing post setMedia()...");
         PlayerCommandSetMedia* setMediaCommand = dynamic_cast<PlayerCommandSetMedia*>(command);
         this->setMediaInt(setMediaCommand->m_mediaContent);
      }

      if (command->m_playerCommandType == PLAYER_COMMAND_TYPE_PLAY) {
         LOG_VERBOSE(LOG_TAG, "Processing post play()...");
         this->playInt();
      }

      if (command->m_playerCommandType == PLAYER_COMMAND_TYPE_PAUSE) {
         LOG_VERBOSE(LOG_TAG, "Processing post pause()...");
         this->pauseInt();
      }

      if (command->m_playerCommandType == PLAYER_COMMAND_TYPE_STOP) {
         LOG_VERBOSE(LOG_TAG, "Processing post stop()...");
         this->stopInt();
      }

      if (command->m_playerCommandType == PLAYER_COMMAND_TYPE_FREE_TEXTURE_DATA) {
         LOG_VERBOSE(LOG_TAG, "Processing post freeTexture()...");
         PlayerCommandFreeTextureData* freeTextureCommand =
               dynamic_cast<PlayerCommandFreeTextureData*>(command);
         this->freeTextureInt(freeTextureCommand->m_textureData);
      }
   }

   qDeleteAll(m_pendingCommands);
   m_pendingCommands.clear();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::instantiateTexture
+-----------------------------------------------------------------------------*/
/**
 * @brief OpenMAXILPlayerControl::instantiateTexture is invoked by the OMX_MediaProcessor
 * when a texture is requested. The thread the code is run on is the same as the thread
 * running the setFilename method: in this case the renderer thread. This makes it possible
 * to make it work correctly.
 * @param size The size of the texture to instantiate.
 * @return The OMX_TextureData containing the data.
 */
OMX_TextureData* OpenMAXILPlayerControl::instantiateTexture(QSize size)
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   OMX_TextureProviderQQuickItem provider(NULL);
   m_textureData = provider.instantiateTexture(size);
   return m_textureData;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::freeTexture
+-----------------------------------------------------------------------------*/
/**
 * @brief OpenMAXILPlayerControl::freeTexture must free textureData content. To do this
 * it is necessary to be in the renderer thread.
 * @param textureData
 */
void OpenMAXILPlayerControl::freeTexture(OMX_TextureData* textureData)
{
   LOG_DEBUG("%s", Q_FUNC_INFO);

#if 0
   PlayerCommandFreeTextureData* command = new PlayerCommandFreeTextureData;
   command->m_playerCommandType = PLAYER_COMMAND_TYPE_FREE_TEXTURE_DATA;
   command->m_textureData       = textureData;
   appendCommand(command);
#endif

   // Guaranteed to be invoked in the renderer thread. No need to queue a message.
   OMX_TextureProviderQQuickItem provider(NULL);
   provider.freeTexture(m_textureData);
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::freeTextureInt
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::freeTextureInt(OMX_TextureData* textureData)
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   OMX_TextureProviderQQuickItem provider(NULL);
   provider.freeTexture(textureData);
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setMedia
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setMedia(const QMediaContent& content, QIODevice* stream)
{
   Q_UNUSED(stream);

   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);
   LOG_DEBUG(LOG_TAG; "Media: %s.", qPrintable(content.canonicalUrl().path()));
   LOG_DEBUG(LOG_TAG, "setMedia thread is: 0x%x.", (unsigned int)QThread::currentThread());

   LOG_VERBOSE(LOG_TAG, "Deferring setMedia()...");
   QUrl url = content.canonicalUrl();
   if (url.isLocalFile() && !QFile(url.path()).exists()) {
      LOG_DEBUG(LOG_TAG, "Does not exist!");
      return;
   }

   PlayerCommandSetMedia* setMedia = new PlayerCommandSetMedia;
   setMedia->m_playerCommandType = PLAYER_COMMAND_TYPE_SET_MEDIA;
   setMedia->m_mediaContent = content;
   appendCommand(setMedia);
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setMediaInt
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setMediaInt(const QMediaContent& mediaContent)
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   m_mediaProcessor->stop();

   m_textureData = NULL;
   if (!m_mediaProcessor->setFilename(mediaContent.canonicalUrl().toString(), m_textureData))
      return;
   m_currentResource = mediaContent;
   emit mediaChanged(mediaContent);
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::getMetaData
+-----------------------------------------------------------------------------*/
QVariantMap OpenMAXILPlayerControl::getMetaData()
{
   if (!m_mediaProcessor)
      return QVariantMap();
   return m_mediaProcessor->getMetaData();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::media
+-----------------------------------------------------------------------------*/
QMediaContent OpenMAXILPlayerControl::media() const
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   return m_currentResource;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::mediaStream
+-----------------------------------------------------------------------------*/
const QIODevice* OpenMAXILPlayerControl::mediaStream() const
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // TODO: Implement.
   return NULL;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::isAudioAvailable
+-----------------------------------------------------------------------------*/
bool OpenMAXILPlayerControl::isAudioAvailable() const
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   return m_mediaProcessor->hasAudio();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::isVideoAvailable
+-----------------------------------------------------------------------------*/
bool OpenMAXILPlayerControl::isVideoAvailable() const
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   return m_mediaProcessor->hasVideo();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::availablePlaybackRanges
+-----------------------------------------------------------------------------*/
QMediaTimeRange OpenMAXILPlayerControl::availablePlaybackRanges() const
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // TODO: Implement.
   return QMediaTimeRange();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::bufferStatus
+-----------------------------------------------------------------------------*/
int OpenMAXILPlayerControl::bufferStatus() const
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // TODO: Implement.
   return 0;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::duration
+-----------------------------------------------------------------------------*/
qint64 OpenMAXILPlayerControl::duration() const
{
   //LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   return m_mediaProcessor->streamLength();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::isMuted
+-----------------------------------------------------------------------------*/
bool OpenMAXILPlayerControl::isMuted() const
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // TODO: Implement mute.
   return false;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::isSeekable
+-----------------------------------------------------------------------------*/
bool OpenMAXILPlayerControl::isSeekable() const
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // TODO: Implement.
   return false;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::mediaStatus
+-----------------------------------------------------------------------------*/
QMediaPlayer::MediaStatus OpenMAXILPlayerControl::mediaStatus() const
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // TODO: Implement.
   return QMediaPlayer::UnknownMediaStatus;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::playbackRate
+-----------------------------------------------------------------------------*/
qreal OpenMAXILPlayerControl::playbackRate() const
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   return 1.0;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setPlaybackRate
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setPlaybackRate(qreal rate)
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // TODO: Implement.
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setPosition
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setPosition(qint64 position)
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // TODO: Implement.
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setVolume
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setVolume(int volume)
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // TODO: Implement.
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::state
+-----------------------------------------------------------------------------*/
QMediaPlayer::State OpenMAXILPlayerControl::state() const
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   assert(m_mediaProcessor);
   return convertState(m_mediaProcessor->state());
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::state
+-----------------------------------------------------------------------------*/
int OpenMAXILPlayerControl::volume() const
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // TODO: Implement.
   return 0;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::position
+-----------------------------------------------------------------------------*/
qint64 OpenMAXILPlayerControl::position() const
{
   //LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   return m_mediaProcessor->streamPosition();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setMuted
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setMuted(bool muted)
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // TODO: Implement.
}

QT_END_NAMESPACE
