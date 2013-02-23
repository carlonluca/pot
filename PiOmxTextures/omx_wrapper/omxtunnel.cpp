#include <Qt>

#include <exception>
#include <stdexcept>

#include "omxtunnel.h"
#include "lgl_logging.h"

using namespace std;

OMXTunnel::OMXTunnel(OMXComponent* compSource, OMX_U32 portSource, OMXComponent* compDest, OMX_U32 portDest) :
    compSource(compSource),
    portSource(portSource),
    compDest(compDest),
    portDest(portDest)
{
    // Do nothing.
}

OMXTunnel::~OMXTunnel()
{
    LOG_VERBOSE(LOG_TAG, "Tearing down tunnel...");
    OMX_ERRORTYPE error;
    error = OMX_SetupTunnel(compSource->GetHandle(), portSource, NULL, 0);
    assert(error == OMX_ErrorNone);
    error = OMX_SetupTunnel(compDest->GetHandle(), portDest, NULL, 0);
    assert(error == OMX_ErrorNone);
}

void OMXTunnel::setupTunnel(unsigned int portStream, unsigned int timeout, bool waitForPortSettingsChanged)
{
    // Source component must at least be idle, not loaded.
    OMX_STATETYPE state;
    compSource->getState(state);
    if (state == OMX_StateLoaded) {
        // TODO: The timeout should only be used for the portsettingschanged.
        LOG_VERBOSE(LOG_TAG, "Changing component %x state to IDLE.", (unsigned int)compSource->GetHandle());
        compSource->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);
        compSource->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, timeout);
    }

    // Wait for the port parameter changed from the source port.
    if (waitForPortSettingsChanged) {
        LOG_VERBOSE(LOG_TAG, "Waiting for PortSettingsChanged event on %x.", (unsigned int)compSource->GetHandle());
        compSource->waitForEvent(OMX_EventPortSettingsChanged, portSource, 0, timeout);
    }

    // TODO: Do I really want to use this timeout?
#if 0 // ilclient is doing this it seems.
    LOG_VERBOSE(LOG_TAG, "Disabling tunnel.");
    disable(timeout);
    LOG_VERBOSE(LOG_TAG, "Tunnel disabled.");
#endif

    // if this source port uses port streams, we need to select one of them before proceeding
    // if getparameter causes an error that's fine, nothing needs selecting.
    OMX_PARAM_U32TYPE param;
    param.nSize = sizeof(OMX_PARAM_U32TYPE);
    param.nVersion.nVersion = OMX_VERSION;
    param.nPortIndex = portSource;
    try {
        compSource->GetParameter(OMX_IndexParamNumAvailableStreams, &param);
        if (param.nU32 == 0) {
            // No streams available.
            // Leave the source port disabled and return failure.
            LOG_ERROR(LOG_TAG, "No stream available.");
            throw runtime_error("No stream port available.");
        }
        if (param.nU32 <= portStream) {
            // Requested stream not available.
            // No streams available.
            // Leave the source port disabled, and return a failure.
            LOG_ERROR(LOG_TAG, "Requested stream not available.");
            throw runtime_error("Requested stream not available.");
        }
        param.nU32 = portStream;
        compSource->SetParameter(OMX_IndexParamActiveStream, &param);
    }
    catch (const runtime_error& e) {
        Q_UNUSED(e);
        // Do nothing. It only means port streams are not supported.
    }

    // Create the tunnel.
    LOG_VERBOSE(LOG_TAG, "Creating the tunnel.");
    compSource->SetupTunnel(portSource, compDest, portDest);

    // Enable the tunnel.
    LOG_VERBOSE(LOG_TAG, "Enabling tunnel...");
    enable(timeout);
    LOG_VERBOSE(LOG_TAG, "Tunner enabled.");
#if 0

    // now create the tunnel
    error = OMX_SetupTunnel(tunnel->source->comp, tunnel->source_port, tunnel->sink->comp, tunnel->sink_port);

    enable_error = 0;

    if (error != OMX_ErrorNone || (enable_error=ilclient_enable_tunnel(tunnel)) < 0)
    {
       // probably format not compatible
       error = OMX_SetupTunnel(tunnel->source->comp, tunnel->source_port, NULL, 0);
       vc_assert(error == OMX_ErrorNone);
       error = OMX_SetupTunnel(tunnel->sink->comp, tunnel->sink_port, NULL, 0);
       vc_assert(error == OMX_ErrorNone);

       if(enable_error)
       {
          //Clean up the errors. This does risk removing an error that was nothing to do with this tunnel :-/
          ilclient_remove_event(tunnel->sink, OMX_EventError, 0, 1, 0, 1);
          ilclient_remove_event(tunnel->source, OMX_EventError, 0, 1, 0, 1);
       }

       ilclient_debug_output("ilclient: could not setup/enable tunnel (setup=0x%x,enable=%d)",
                              error, enable_error);
       return -5;
    }

    return 0;
#endif
}

void OMXTunnel::flush(unsigned int timeout)
{
    compSource->sendCommand(OMX_CommandFlush, portSource, NULL);
    compDest->sendCommand(OMX_CommandFlush, portDest, NULL);

    compSource->waitForEvent(OMX_EventCmdComplete, OMX_CommandFlush, portSource, timeout);
    compDest->waitForEvent(OMX_EventCmdComplete, OMX_CommandFlush, portDest, timeout);
}

void OMXTunnel::disable(unsigned int timeout)
{
    // Both ends must exist.
    if (!compSource || !compDest)
        return;

    // Disable the ports.
    LOG_VERBOSE(LOG_TAG, "Sending disable command.");
    compSource->sendCommand(OMX_CommandPortDisable, portSource, NULL);
    compDest->sendCommand(OMX_CommandPortDisable, portDest, NULL);
    LOG_VERBOSE(LOG_TAG, "Sent disable command.");

    // Wait for command to be executed.
    // TODO: This timeout is used incorrectly.
    compSource->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, portSource, timeout);
    compDest->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, portDest, timeout);
}

void OMXTunnel::enable(unsigned int timeout)
{
    compSource->sendCommand(OMX_CommandPortEnable, portSource, NULL);
    compDest->sendCommand(OMX_CommandPortEnable, portDest, NULL);

    OMX_STATETYPE state;
    compDest->getState(state);
    if (state == OMX_StateLoaded) {
        // Wait for port enabled, then switch state to IDLE and wait for
        // state to change.
        compDest->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, portDest, timeout);
        compDest->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);
        compDest->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, timeout);
    }
    else
        compDest->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, portDest, timeout);

    compSource->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortEnable, portSource, timeout);

#if 0
    OMX_STATETYPE state;
    OMX_ERRORTYPE error;

    ilclient_debug_output("ilclient: enable tunnel from %x/%d to %x/%d",
                          tunnel->source, tunnel->source_port,
                          tunnel->sink, tunnel->sink_port);

    error = OMX_SendCommand(tunnel->source->comp, OMX_CommandPortEnable, tunnel->source_port, NULL);
    vc_assert(error == OMX_ErrorNone);

    error = OMX_SendCommand(tunnel->sink->comp, OMX_CommandPortEnable, tunnel->sink_port, NULL);
    vc_assert(error == OMX_ErrorNone);

    // to complete, the sink component can't be in loaded state
    error = OMX_GetState(tunnel->sink->comp, &state);
    vc_assert(error == OMX_ErrorNone);
    if (state == OMX_StateLoaded)
    {
       int ret = 0;

       if(ilclient_wait_for_command_complete(tunnel->sink, OMX_CommandPortEnable, tunnel->sink_port) != 0 ||
          OMX_SendCommand(tunnel->sink->comp, OMX_CommandStateSet, OMX_StateIdle, NULL) != OMX_ErrorNone ||
          (ret = ilclient_wait_for_command_complete_dual(tunnel->sink, OMX_CommandStateSet, OMX_StateIdle, tunnel->source)) < 0)
       {
          if(ret == -2)
          {
             // the error was reported fom the source component: clear this error and disable the sink component
             ilclient_wait_for_command_complete(tunnel->source, OMX_CommandPortEnable, tunnel->source_port);
             ilclient_disable_port(tunnel->sink, tunnel->sink_port);
          }

          ilclient_debug_output("ilclient: could not change component state to IDLE");
          ilclient_disable_port(tunnel->source, tunnel->source_port);
          return -1;
       }
    }
    else
    {
       if (ilclient_wait_for_command_complete(tunnel->sink, OMX_CommandPortEnable, tunnel->sink_port) != 0)
       {
          ilclient_debug_output("ilclient: could not change sink port %d to enabled", tunnel->sink_port);

          //Oops failed to enable the sink port
          ilclient_disable_port(tunnel->source, tunnel->source_port);
          //Clean up the port enable event from the source port.
          ilclient_wait_for_event(tunnel->source, OMX_EventCmdComplete,
                                  OMX_CommandPortEnable, 0, tunnel->source_port, 0,
                                  ILCLIENT_PORT_ENABLED | ILCLIENT_EVENT_ERROR, VCOS_EVENT_FLAGS_SUSPEND);
          return -1;
       }
    }

    if(ilclient_wait_for_command_complete(tunnel->source, OMX_CommandPortEnable, tunnel->source_port) != 0)
    {
       ilclient_debug_output("ilclient: could not change source port %d to enabled", tunnel->source_port);

       //Failed to enable the source port
       ilclient_disable_port(tunnel->sink, tunnel->sink_port);
       return -1;
    }

    return 0;
#endif
}
