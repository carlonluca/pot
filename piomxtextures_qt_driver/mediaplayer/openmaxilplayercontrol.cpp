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

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
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

#include "lc_logging.h"

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
#include <lc_logging.h>
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
   , m_mediaProcessor(new OMX_MediaProcessor(make_shared<OMX_EGLBufferProvider>()))
   , m_textureData(NULL)
   , m_sceneGraphInitialized(false)
   , m_quickItem(NULL)
{
   log_debug_func;

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
   log_debug_func;

   delete m_mediaProcessor;
   m_mediaProcessor = NULL;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setMediaPlayer
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setMediaPlayer(QMediaPlayer* mediaPlayer)
{
   log_debug_func;
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
   log_debug_func;

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
   log_debug_func;

   // Can be done in any thread.
   assert(m_mediaProcessor);
   m_mediaProcessor->play();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::pause
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::pause()
{
   log_debug_func;

   PlayerCommandPause* pause = new PlayerCommandPause;
   pause->m_playerCommandType = PLAYER_COMMAND_TYPE_PAUSE;
   appendCommand(pause);
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::pauseInt
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::pauseInt()
{
   log_debug_func;

   assert(m_mediaProcessor);
   m_mediaProcessor->pause();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::stop
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::stop()
{
   log_debug_func;
   m_mediaProcessor->stop();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::stopInt
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::stopInt()
{
	log_debug_func;
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
   QQuickWindow* window = m_quickItem->window();
   if (!window)
      return;

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

   QQuickWindow* window = m_quickItem->window();
   if (window)
      window->update();
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
         m_texProvider->freeTexture(freeTextureCommand->m_textureData);
      }
   }

   qDeleteAll(m_pendingCommands);
   m_pendingCommands.clear();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setMedia
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setMedia(const QMediaContent& content, QIODevice* stream)
{
   Q_UNUSED(stream);

   log_debug_func;
   log_debug("Media: %s.", qPrintable(content.canonicalUrl().toString()));
   log_debug("setMedia thread is: 0x%x.", ((unsigned int)QThread::currentThread()));

   log_verbose("Deferring setMedia()...");
   /*QUrl url = content.canonicalUrl();
   if (url.isLocalFile() && !QFile(url.path()).exists()) {
      log_warn("Does not exist!");
      return;
   }*/

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
   log_debug_func;

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
   log_debug_func;

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
   log_debug_func;

   return m_mediaProcessor->hasAudio();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::isVideoAvailable
+-----------------------------------------------------------------------------*/
bool OpenMAXILPlayerControl::isVideoAvailable() const
{
   log_debug_func;

   return m_mediaProcessor->hasVideo();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::availablePlaybackRanges
+-----------------------------------------------------------------------------*/
QMediaTimeRange OpenMAXILPlayerControl::availablePlaybackRanges() const
{
   log_debug_func;

   // TODO: Implement.
   return QMediaTimeRange();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::bufferStatus
+-----------------------------------------------------------------------------*/
int OpenMAXILPlayerControl::bufferStatus() const
{
   log_debug_func;

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
   log_debug_func;

   return m_mediaProcessor->muted();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::isSeekable
+-----------------------------------------------------------------------------*/
bool OpenMAXILPlayerControl::isSeekable() const
{
   log_debug_func;

   // TODO: Implement.
   return false;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::mediaStatus
+-----------------------------------------------------------------------------*/
QMediaPlayer::MediaStatus OpenMAXILPlayerControl::mediaStatus() const
{
   log_debug_func;

   // TODO: Implement.
   return QMediaPlayer::UnknownMediaStatus;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::playbackRate
+-----------------------------------------------------------------------------*/
qreal OpenMAXILPlayerControl::playbackRate() const
{
   log_debug_func;

   return 1.0;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setPlaybackRate
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setPlaybackRate(qreal rate)
{
   log_debug_func;

   // TODO: Implement.
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setPosition
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setPosition(qint64 position)
{
   log_debug_func;

   m_mediaProcessor->seek(position);
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setVolume
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setVolume(int volume)
{
	log_debug("Setting volume to: %d.", volume);
	m_mediaProcessor->setVolume(volume, true);
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::state
+-----------------------------------------------------------------------------*/
QMediaPlayer::State OpenMAXILPlayerControl::state() const
{
   log_debug_func;

   assert(m_mediaProcessor);
   return convertState(m_mediaProcessor->state());
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::state
+-----------------------------------------------------------------------------*/
int OpenMAXILPlayerControl::volume() const
{
   log_debug_func;

	return m_mediaProcessor->volume(true);
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
   log_debug_func;

   m_mediaProcessor->setMute(muted);
}

QT_END_NAMESPACE
