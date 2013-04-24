/*
 * Author:  Luca Carlon
 * Company: -
 * Date:    04.14.2013
 * Project: OpenMAXIL QtMultimedia Plugin
 */

#ifndef OPENMAXILVIDEORENDERERCONTROL_H
#define OPENMAXILVIDEORENDERERCONTROL_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include "QtMultimedia/qvideorenderercontrol.h"
#include "QtMultimedia/qvideoframe.h"
#include "QtMultimedia/qvideosurfaceformat.h"

#include <omx_textureprovider.h>

/*------------------------------------------------------------------------------
|    defintions
+-----------------------------------------------------------------------------*/
class OpenMAXILVideoBuffer;

/*------------------------------------------------------------------------------
|    OpenMAXILVideoRendererControl class
+-----------------------------------------------------------------------------*/
class OpenMAXILVideoRendererControl : public QVideoRendererControl
{
   Q_OBJECT
public:
   explicit OpenMAXILVideoRendererControl(QObject* parent = 0);
   ~OpenMAXILVideoRendererControl();
   void setSurface(QAbstractVideoSurface* surface);
   QAbstractVideoSurface* surface() const;

public slots:
   void onTextureReady(const OMX_TextureData* textureData);
   void onTextureInvalidated();
   void onUpdateTriggered();

private:
   QAbstractVideoSurface* m_surface;
   QVideoSurfaceFormat* m_surfaceFormat;
   QVideoFrame* m_frame;
   OpenMAXILVideoBuffer* m_buffer;
   uint m_textureId;
   QTimer* m_updateTimer;
};

#endif // OPENMAXILVIDEORENDERERCONTROL_H
