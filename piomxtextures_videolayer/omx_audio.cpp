#include "omx_audio.h"

OMX_Audio::OMX_Audio(QQuickItem* parent) : OMX_Video(parent)
{

}

QSGNode* OMX_Audio::updatePaintNode(QSGNode* oldNode, UpdatePaintNodeData* data)
{
    return QQuickItem::updatePaintNode(oldNode, data);
}
