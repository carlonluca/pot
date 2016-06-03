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

#ifndef POT_VIDEOPROBE_H
#define POT_VIDEOPROBE_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QQuickItem>
#include <QVideoProbe>

/*------------------------------------------------------------------------------
|    POT_VideoProbe class
+-----------------------------------------------------------------------------*/
class POT_VideoProbe : public QVideoProbe
{
	Q_OBJECT

	Q_PROPERTY(QObject* source READ source WRITE setSource NOTIFY sourceChanged)

public:
	POT_VideoProbe(QObject* parent = 0);

	QObject* source();
	void setSource(QObject* source);

signals:
	void sourceChanged();
    void videoFrameDecoded();

private slots:
    void onVideoFrameProbed(const QVideoFrame& frame);

private:
	QObject* m_source;
};

#endif // POT_VIDEOPROBE_H
