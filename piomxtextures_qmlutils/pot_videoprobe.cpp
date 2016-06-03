/*
 * Project: PiOmxTexturesQmlUtils
 * Author:  Luca Carlon
 * Date:    03.12.2016
 *
 * Copyright (c) 2016 Luca Carlon. All rights reserved.
 *
 * This file is part of PiOmxTexturesQmlUtils.
 *
 * PiOmxTexturesQmlUtils is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PiOmxTexturesQmlUtils is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PiOmxTexturesQmlUtils. If not, see <http://www.gnu.org/licenses/>.
 */

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QMediaObject>
#include <QMediaPlayer>

#include "pot_videoprobe.h"

/*------------------------------------------------------------------------------
|    POT_VideoProbe::POT_VideoProbe
+-----------------------------------------------------------------------------*/
POT_VideoProbe::POT_VideoProbe(QObject* parent) :
	QVideoProbe(parent)
{
	// Do nothing.

    connect(this, SIGNAL(videoFrameProbed(QVideoFrame)),
            this, SLOT(onVideoFrameProbed(QVideoFrame)));
}

/*------------------------------------------------------------------------------
|    POT_VideoProbe::source
+-----------------------------------------------------------------------------*/
QObject* POT_VideoProbe::source()
{
	return m_source;
}

/*------------------------------------------------------------------------------
|    POT_VideoProbe::setSource
+-----------------------------------------------------------------------------*/
void POT_VideoProbe::setSource(QObject* source)
{
	if (m_source == source)
		return;
	m_source = source;

	QMediaPlayer* player = NULL;
	if (m_source)
		player = qvariant_cast<QMediaPlayer*>(m_source->property("mediaObject"));

	if (!QVideoProbe::setSource((QMediaObject*)player)) {
			qWarning("Failed to set probe.");
			m_source = NULL;
			return;
	}

	emit sourceChanged();
}

/*------------------------------------------------------------------------------
|    POT_VideoProbe::onVideoFrameProbed
+-----------------------------------------------------------------------------*/
void POT_VideoProbe::onVideoFrameProbed(const QVideoFrame &frame)
{
    emit videoFrameDecoded();
}
