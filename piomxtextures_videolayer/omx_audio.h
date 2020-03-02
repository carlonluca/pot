#ifndef OMX_AUDIO_H
#define OMX_AUDIO_H

#include <QQuickItem>

#include "omx_video.h"

class OMX_Audio : public OMX_Video
{
    Q_OBJECT
public:
    OMX_Audio(QQuickItem* parent = nullptr);

protected:
    QSGNode* updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData*) override;
};

#endif // OMX_AUDIO_H
