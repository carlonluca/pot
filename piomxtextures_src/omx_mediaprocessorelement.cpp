/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    11.01.2012
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
 * along with PiOmxTextures.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <memory>

#include "omx_mediaprocessorelement.h"
#include "omx_textureprovider.h"
#include "lc_logging.h"

using namespace std;

#define CHECK_MEDIA_PROCESSOR                                            \
   if (!m_mediaProc) {                                                   \
      LOG_WARNING(LOG_TAG, "The media processor is not available yet."); \
      return false;                                                      \
   }

OMX_MediaProcessorElement::OMX_MediaProcessorElement(QQuickItem *parent) :
   QQuickItem(parent),
   m_buffProvider(NULL),
   m_mediaProc(NULL),
   m_pendingOpen(false),
   m_pendingStop(false),
   m_pendingPlay(false),
   m_textureData(NULL)
{
   // I need to set this as a "has-content" item because I need the updatePaintNode
   // to be invoked.
   setFlag(QQuickItem::ItemHasContents, true);
}

OMX_MediaProcessorElement::~OMX_MediaProcessorElement()
{
   delete m_mediaProc;
   delete m_textureData;
}

QString OMX_MediaProcessorElement::source()
{
   return m_source;
}

void OMX_MediaProcessorElement::setSource(QString source)
{
   LOG_VERBOSE(LOG_TAG, "Setting source.");

   // TODO: Handle errors.
   m_source = source;
   m_pendingOpen = true;
   update();
}

bool OMX_MediaProcessorElement::autoPlay()
{
   return m_autoPlay;
}

void OMX_MediaProcessorElement::setAutoPlay(bool autoPlay)
{
   LOG_VERBOSE(LOG_TAG, "Setting autoPlay.");

   m_autoPlay = autoPlay;
}

QSGNode* OMX_MediaProcessorElement::updatePaintNode(QSGNode*, UpdatePaintNodeData*)
{
   if (m_pendingOpen) {
      openMedia(m_source);
      m_pendingOpen = false;
   }

   if (m_pendingStop) {
      m_mediaProc->stop();
      m_pendingStop = false;
   }

   if (m_pendingPlay) {
      m_mediaProc->play();
      m_pendingPlay = false;
   }

   return NULL;
}

bool OMX_MediaProcessorElement::play()
{
   CHECK_MEDIA_PROCESSOR;
   //return m_mediaProc->play();
   m_pendingPlay = true;
   update();

   return true;
}

bool OMX_MediaProcessorElement::stop()
{
   CHECK_MEDIA_PROCESSOR;
   //return m_mediaProc->stop();
   m_pendingStop = true;
   update();

   return true;
}

bool OMX_MediaProcessorElement::pause()
{
   CHECK_MEDIA_PROCESSOR;
   return m_mediaProc->pause();
}

bool OMX_MediaProcessorElement::seek(long millis)
{
   CHECK_MEDIA_PROCESSOR;
   return m_mediaProc->seek(millis);
}

void OMX_MediaProcessorElement::instantiateMediaProcessor()
{
   if (!m_buffProvider)
      m_buffProvider = make_shared<OMX_EGLBufferProvider>();
   if (!m_mediaProc) {
      m_mediaProc = new OMX_MediaProcessor(m_buffProvider);
      connect(m_mediaProc, SIGNAL(playbackCompleted()), this, SIGNAL(playbackCompleted()));
      connect(m_mediaProc, SIGNAL(playbackStarted()), this, SIGNAL(playbackStarted()));
      connect(m_mediaProc, SIGNAL(stateChanged(OMX_MediaProcessor::OMX_MediaProcessorState)),
              this, SIGNAL(playbackStateChanged(OMX_MediaProcessor::OMX_MediaProcessorState)));
   }
}

bool OMX_MediaProcessorElement::openMedia(QString filepath)
{
   instantiateMediaProcessor();

   // I need to do this in the rendering thread.
   if (m_textureData) {
      m_textureData->freeData();
      m_textureData = NULL;
   }

   assert(false);

   // TODO: This needs refactoring.
   //if (!m_mediaProc->setFilename(filepath, m_textureData))
   //   return false;

   // Check to see if we should be auto-playing the media
   if (m_autoPlay) {
      if (!play())
         return false;
   }

   return true;
}
