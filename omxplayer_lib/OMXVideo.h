#pragma once
/*
 *      Copyright (C) 2010 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#if defined(HAVE_OMXLIB)

#include <QObject>

#include <memory>

#include "OMXCore.h"
#include "OMXStreamInfo.h"

#include <IL/OMX_Video.h>

#include "OMXClock.h"
#include "OMXReader.h"

#include "guilib/Geometry.h"
#include "utils/SingleLock.h"

using namespace std;

class OMX_VideoSurfaceElement;
class OMX_TextureProvider;
class OMX_TextureData;

typedef shared_ptr<OMX_TextureProvider> OMX_TextureProviderSh;


#define VIDEO_BUFFERS 60

enum EDEINTERLACEMODE
{
  VS_DEINTERLACEMODE_OFF=0,
  VS_DEINTERLACEMODE_AUTO=1,
  VS_DEINTERLACEMODE_FORCE=2
};

#define CLASSNAME "COMXVideo"

class DllAvUtil;
class DllAvFormat;
class COMXVideo : public QObject
{
    Q_OBJECT
public:
  COMXVideo(OMX_TextureProviderSh provider);
  ~COMXVideo();

  // Required overrides
  bool SendDecoderConfig();
  bool NaluFormatStartCodes(enum AVCodecID codec, uint8_t *in_extradata, int in_extrasize);
  bool Open(COMXStreamInfo &hints,
          OMXClock *clock,
          float display_aspect = 0.0f,
          EDEINTERLACEMODE deinterlace = VS_DEINTERLACEMODE_OFF,
          bool hdmi_clock_sync = false,
          float fifo_size = 0.0f,
          OMX_TextureData* textureData = NULL
          );
  bool PortSettingsChanged();
  void Close(void);
  unsigned int GetFreeSpace();
  unsigned int GetSize();
  OMXPacket *GetText();
  int  DecodeText(uint8_t *pData, int iSize, double dts, double pts);
  int  Decode(uint8_t *pData, int iSize, double pts);
  void Reset(void);
  void SetDropState(bool bDrop);
  std::string GetDecoderName() { return m_video_codec_name; };
  void SetVideoRect(const CRect& SrcRect, const CRect& DestRect);
  bool SetVideoEGL();
  bool SetVideoEGLOutputPort();
  int GetInputBufferSize();
  void SubmitEOS();
  bool IsEOS();
  bool SubmittedEOS() { return m_submitted_eos; }
  bool BadState() { return m_omx_decoder.BadState(); };

signals:
  void textureDataReady(const OMX_TextureData* textureData);

protected:
  // Video format
  bool              m_drop_state;
  unsigned int      m_decoded_width;
  unsigned int      m_decoded_height;

  OMX_VIDEO_CODINGTYPE m_codingType;

  COMXCoreComponent m_omx_text;
  COMXCoreComponent m_omx_decoder;
  COMXCoreComponent m_omx_render;
  COMXCoreComponent m_omx_sched;
  COMXCoreComponent m_omx_image_fx;
  COMXCoreComponent *m_omx_clock;
  OMXClock           *m_av_clock;

  COMXCoreTunel     m_omx_tunnel_text;
  COMXCoreTunel     m_omx_tunnel_decoder;
  COMXCoreTunel     m_omx_tunnel_clock;
  COMXCoreTunel     m_omx_tunnel_sched;
  COMXCoreTunel     m_omx_tunnel_image_fx;
  bool              m_is_open;

  bool              m_setStartTime;
  bool              m_setStartTimeText;

  uint8_t           *m_extradata;
  int               m_extrasize;

  std::string       m_video_codec_name;

  bool              m_deinterlace;
  EDEINTERLACEMODE  m_deinterlace_request;
  bool              m_hdmi_clock_sync;
  bool              m_first_text;
  // lcarlon: modified members.
  OMX_TextureProviderSh m_provider;
  OMX_BUFFERHEADERTYPE* m_eglBuffer;
  OMX_TextureData*  m_textureData; // Only used in case of re-use of the texture.
  // ====
  float             m_pixel_aspect;
  bool              m_submitted_eos;
  bool              m_failed_eos;
  OMX_DISPLAYTRANSFORMTYPE m_transform;
  bool              m_settings_changed;
  CCriticalSection  m_critSection;
};

#endif
