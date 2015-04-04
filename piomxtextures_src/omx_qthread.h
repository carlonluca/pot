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
 * along with PiOmxTextures.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OMX_QTHREAD_H
#define OMX_QTHREAD_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QThread>


/*------------------------------------------------------------------------------
|    OMX_VideoProcessorThread class
+-----------------------------------------------------------------------------*/
class OMX_QThread : public QThread
{
public:
    OMX_QThread(QObject* parent = 0) : QThread(parent) {
        // Do nothing.
    }

    inline Qt::HANDLE getThreadId() {
        return m_threadId;
    }

protected:
    void run() {
        m_threadId = QThread::currentThreadId();
        QThread::run();
    }

private:
    Qt::HANDLE m_threadId;
};

#endif // OMX_QTHREAD_H
