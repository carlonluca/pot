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

#include <QDataStream>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <openmaxilvideorenderercontrol.h>

#include <omx_mediaprocessor.h>
#include <omx_textureprovider.h>
#include <omx_logging.h>

/*------------------------------------------------------------------------------
|    definitions
+-----------------------------------------------------------------------------*/
//#define DEBUG_PLAYBIN
//#define DEBUG_PLAYER_CONTROL

#ifndef DEBUG_PLAYER_CONTROL
#ifdef LOG_DEBUG
#undef LOG_DEBUG
#define LOG_DEBUG(...) {(void)0;}
#endif // LOG_DEBUG
#else
#include <omx_logging.h>
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
	, m_texProvider(make_shared<OMX_EGLBufferProvider>())
	, m_mediaProcessor(new OMX_MediaProcessor(m_texProvider))
   , m_renderer(NULL)
{
	logi_debug_func;

   connect(m_mediaProcessor, SIGNAL(stateChanged(OMX_MediaProcessor::OMX_MediaProcessorState)),
           this, SLOT(onStateChanged(OMX_MediaProcessor::OMX_MediaProcessorState)));
	connect(m_mediaProcessor, SIGNAL(mediaStatusChanged(OMX_MediaProcessor::OMX_MediaStatus)),
			  this, SLOT(onMediaStatusChanged(OMX_MediaProcessor::OMX_MediaStatus)));
   connect(m_mediaProcessor, SIGNAL(metadataChanged(QVariantMap)),
			  this, SIGNAL(metaDataChanged(QVariantMap)));
	connect(m_mediaProcessor, SIGNAL(streamLengthChanged(qint64)),
			  this, SIGNAL(durationChanged(qint64)));
	connect(m_mediaProcessor, SIGNAL(bufferStatusChanged(int)),
			  this, SIGNAL(bufferStatusChanged(int)));
	connect(m_mediaProcessor, SIGNAL(availablePlaybackRangesChanged(QMediaTimeRange)),
			  this, SIGNAL(availablePlaybackRangesChanged(QMediaTimeRange)));
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::~OpenMAXILPlayerControl
+-----------------------------------------------------------------------------*/
OpenMAXILPlayerControl::~OpenMAXILPlayerControl()
{
	log_dtor_func;

	m_mediaProcessor->deleteLater();
   m_mediaProcessor = NULL;
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::play
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::play()
{
   logi_debug_func;
   m_mediaProcessor->play();

	// FIXME: This should be removed.
	emit availablePlaybackRangesChanged(QMediaTimeRange(0, duration()));
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::pause
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::pause()
{
   logi_debug_func;
   m_mediaProcessor->pause();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::stop
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::stop()
{
   logi_debug_func;
   m_mediaProcessor->stop();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::onStateChanged
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::onStateChanged(OMX_MediaProcessor::OMX_MediaProcessorState state)
{
   emit stateChanged(convertState(state));
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::onMediaStatusChanged
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::onMediaStatusChanged(OMX_MediaProcessor::OMX_MediaStatus status)
{
	emit mediaStatusChanged(convertMediaStatus(status));
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::setMedia
+-----------------------------------------------------------------------------*/
void OpenMAXILPlayerControl::setMedia(const QMediaContent& content, QIODevice* stream)
{
   Q_UNUSED(stream);

   log_debug_func;

   logi_debug("Media: %s.", qPrintable(content.canonicalUrl().toString()));
   logi_debug("setMedia thread is: %p.", ((unsigned int)QThread::currentThread()));

   if (!m_mediaProcessor->setFilename(content.canonicalUrl().toString()))
      return;
   m_currentResource = content;
   emit mediaChanged(content);
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
   logi_debug_func;

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
	return QMediaTimeRange(0, duration());
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::bufferStatus
+-----------------------------------------------------------------------------*/
int OpenMAXILPlayerControl::bufferStatus() const
{
   log_debug_func;

   // TODO: Implement.
	return 100;
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

	return m_mediaProcessor->isSeekable();
}

/*------------------------------------------------------------------------------
|    OpenMAXILPlayerControl::mediaStatus
+-----------------------------------------------------------------------------*/
QMediaPlayer::MediaStatus OpenMAXILPlayerControl::mediaStatus() const
{
	return convertMediaStatus(m_mediaProcessor->mediaStatus());
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
	log_debug("Playback rate: %f.", rate);

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
	// volume is in [0, 100] here.
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
