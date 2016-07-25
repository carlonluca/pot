/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <lc_logging.h>

#include "poc_bridge.h"

/*------------------------------------------------------------------------------
|    POC_Bridge::POC_Bridge
+-----------------------------------------------------------------------------*/
POC_Bridge::POC_Bridge(const QString& videoId, QObject* parent) :
	QObject(parent),
	m_videoId(videoId)
{
	// Do nothing.
}

/*------------------------------------------------------------------------------
|    POC_Bridge::onPlayerReady
+-----------------------------------------------------------------------------*/
void POC_Bridge::onPlayerReady()
{
	log_info("YouTube player ready.");
	emit playVideoId(m_videoId);
}
