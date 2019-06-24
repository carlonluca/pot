/*
 * Project: PiOmxTexturesVideoLayer
 * Author:  Luca Carlon
 * Date:    01.02.2016
 *
 * Copyright (c) 2016 Luca Carlon. All rights reserved.
 *
 * This file is part of PiOmxTexturesVideoLayer.
 *
 * PiOmxTexturesVideoLayer is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * PiOmxTexturesVideoLayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with PiOmxTexturesVideoLayer. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef OMX_PIOMXTEXTURESPLUGIN_H
#define OMX_PIOMXTEXTURESPLUGIN_H

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QQmlExtensionPlugin>
#include <QtQml>
#include <QObject>
#include <QProcess>

#include "omx_videolayer.h"
#include "omx_logging.h"

/*------------------------------------------------------------------------------
|    OMX_PiOmxTexturesPlugin class
+-----------------------------------------------------------------------------*/
class OMX_PiOmxTexturesPlugin : public QQmlExtensionPlugin
{
   Q_OBJECT
   Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QQmlExtensionInterface")

public:
   OMX_PiOmxTexturesPlugin(QObject* parent = nullptr);

	void registerTypes(const char* uri) {
		log_info("PiOmxTexturesVideoLayer version %s built %s, %s.",
					VERSION, __DATE__, __TIME__);

      log_verbose("Registering OMX_VideoLayer QML type...");
      Q_ASSERT(uri == QLatin1String("PiOmxTexturesVideoLayer"));
      qmlRegisterType<OMX_VideoLayer>(uri, 0, 1, "POT_VideoLayer");
	}
};

#endif // OMX_PIOMXTEXTURESPLUGIN_H
