/*
 * Project: PiOmxTextures
 * Author:  Luca Carlon
 * Date:    12.28.2012
 *
 * Copyright (c) 2012 Luca Carlon. All rights reserved.
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

#ifndef OMX_AUDIOPROCESSOR_H
#define OMX_AUDIOPROCESSOR_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QObject>

#include <stdint.h>
#include <IL/OMX_Core.h>

#include "omx_qthread.h"


/*------------------------------------------------------------------------------
|    OMX_AudioProcessor class
+-----------------------------------------------------------------------------*/
class OMX_AudioProcessor : public QObject
{
    Q_OBJECT
public:
    explicit OMX_AudioProcessor(QObject* parent = 0);
    void play();

public:
    QString m_source;

private slots:
    void audioDecoding();

private:
    uint32_t getLatency(OMX_HANDLETYPE renderer);

    OMX_QThread m_thread;
};

#endif // OMX_AUDIOPROCESSOR_H
