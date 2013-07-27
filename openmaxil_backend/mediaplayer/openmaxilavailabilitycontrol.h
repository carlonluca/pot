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

#ifndef QOPENMAXILAVAILABILITYCONTROL_H
#define QOPENMAXILAVAILABILITYCONTROL_H

#include <QObject>
#include <qmediaavailabilitycontrol.h>

QT_BEGIN_NAMESPACE

class OpenMAXILAvailabilityControl : public QMediaAvailabilityControl
{
    Q_OBJECT
public:
    OpenMAXILAvailabilityControl(QObject* parent = 0);
    QMultimedia::AvailabilityStatus availability() const;
};

QT_END_NAMESPACE

#endif // QOPENMAXILAVAILABILITYCONTROL_H
