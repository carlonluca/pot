/*
 * Author:  Luca Carlon
 * Company: -
 * Date:    04.14.2013
 * Project: OpenMAXIL QtMultimedia Plugin
 */

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include "QtMultimedia/qabstractvideosurface.h"
#include "QtMultimedia/qvideosurfaceformat.h"
#include "QtCore/qtimer.h"
#include "QtCore/qmutex.h"
#include "openmaxilvideorenderercontrol.h"
#include "lgl_logging.h"

static QMutex m_mutex;


/*------------------------------------------------------------------------------
|    OpenMAXILVideoBuffer class
+-----------------------------------------------------------------------------*/
class OpenMAXILVideoBuffer : public QAbstractVideoBuffer
{
public:
   OpenMAXILVideoBuffer(HandleType handleType, GLuint textureId) :
      QAbstractVideoBuffer(handleType)
   {
      m_handleType = handleType;
      m_textureId  = textureId;
   }

   ~OpenMAXILVideoBuffer() {
      LOG_VERBOSE(LOG_TAG, "VideoBuffer freed.");
   }

   QVariant handle() const {
      return QVariant::fromValue<unsigned int>(m_textureId);
   }

   uchar* map(MapMode mode, int* numBytes, int* bytesPerLine) {
      return NULL;
   }

   MapMode mapMode() const {
      return ReadOnly;
   }

   void unmap() {}

private:
   HandleType m_handleType;
   uint m_textureId;
};

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::OpenMAXILVideoRendererControl
+-----------------------------------------------------------------------------*/
OpenMAXILVideoRendererControl::OpenMAXILVideoRendererControl(QObject *parent) :
   QVideoRendererControl(parent),
   m_surface(NULL),
   m_surfaceFormat(NULL),
   m_frame(NULL),
   m_buffer(NULL),
   m_textureId(0),
   m_updateTimer(new QTimer(this))
{
   m_updateTimer->setInterval(30);
   m_updateTimer->setSingleShot(false);
   connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(onUpdateTriggered()));
   m_updateTimer->start();
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::~OpenMAXILVideoRendererControl
+-----------------------------------------------------------------------------*/
OpenMAXILVideoRendererControl::~OpenMAXILVideoRendererControl()
{
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   delete m_frame;
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::setSurface
+-----------------------------------------------------------------------------*/
void OpenMAXILVideoRendererControl::setSurface(QAbstractVideoSurface* surface)
{
   QMutexLocker locker(&m_mutex);
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   if (m_surface && m_surface->isActive())
       m_surface->stop();
   m_surface = surface;
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::surface
+-----------------------------------------------------------------------------*/
QAbstractVideoSurface* OpenMAXILVideoRendererControl::surface() const
{
   QMutexLocker locker(&m_mutex);
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   return m_surface;
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::onTextureReady
+-----------------------------------------------------------------------------*/
void OpenMAXILVideoRendererControl::onTextureReady(const OMX_TextureData* textureData)
{
   QMutexLocker locker(&m_mutex);
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   m_buffer = new OpenMAXILVideoBuffer(
            QAbstractVideoBuffer::GLTextureHandle,
            textureData->m_textureId
            );
   m_frame = new QVideoFrame(
            m_buffer,
            textureData->m_textureSize,
            QVideoFrame::Format_RGB565
            );
   m_surfaceFormat = new QVideoSurfaceFormat(
            textureData->m_textureSize,
            QVideoFrame::Format_RGB565,
            QAbstractVideoBuffer::GLTextureHandle
            );
   m_textureId = textureData->m_textureId;
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::onTextureInvalidated
+-----------------------------------------------------------------------------*/
void OpenMAXILVideoRendererControl::onTextureInvalidated()
{
   QMutexLocker locker(&m_mutex);
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);

   // Stop the surface first.
   if (m_surface)
      m_surface->stop();

   delete m_surfaceFormat;
   delete m_frame;
#if 0
   // NOTE: Do not free the buffer. It is already freed (I guess by
   // deleting the frame somehow).
   delete m_buffer;
#endif
   m_surfaceFormat = NULL;
   m_frame         = NULL;
   m_buffer        = NULL;

   m_textureId = 0;
}

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl::onUpdateTriggered
+-----------------------------------------------------------------------------*/
void OpenMAXILVideoRendererControl::onUpdateTriggered()
{
   QMutexLocker locker(&m_mutex);
#ifdef UNACCEPTABLY_VERBOSE_LOGS
   LOG_DEBUG(LOG_TAG, "%s", Q_FUNC_INFO);
#endif

   if (m_surface && m_frame && m_surfaceFormat) {
      if (!m_surface->isActive() && !m_surface->start(*m_surfaceFormat)) {
         LOG_WARNING(LOG_TAG, "Failed to start surface.");
      }
      m_surface->present(*m_frame);
   }
}
