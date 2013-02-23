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

/*------------------------------------------------------------------------------
|    includes
+-----------------------------------------------------------------------------*/
#include <QFile>

#include <IL/OMX_Audio.h>
#include <IL/OMX_Broadcom.h>

extern "C" {
#include <libavformat/avformat.h>
}

#include <memory>
#include <stdint.h>
#include <unistd.h>
#include <vector>

#include "omx_audioprocessor.h"
#include "OMX_Core.h"
#include "lgl_logging.h"

using namespace std;

#define TIMEOUT_MS          10000
#define BUFFER_SIZE_SAMPLES 1024
#define N_WAVE              1024    /* dimension of Sinewave[] */
#define PI                  (1 << 16 >> 1)
#define SIN(x)              Sinewave[((x)>>6) & (N_WAVE-1)]
#define COS(x)              SIN((x) + (PI >> 1))
#define CTTW_SLEEP_TIME     10
#define MIN_LATENCY_TIME    20
#define AUDIO_INBUF_SIZE    20480
#define AUDIO_REFILL_THRESH 4096

short Sinewave[] = {
    0,    201,    402,    603,    804,   1005,   1206,   1406,
    1607,   1808,   2009,   2209,   2410,   2610,   2811,   3011,
    3211,   3411,   3611,   3811,   4011,   4210,   4409,   4608,
    4807,   5006,   5205,   5403,   5601,   5799,   5997,   6195,
    6392,   6589,   6786,   6982,   7179,   7375,   7571,   7766,
    7961,   8156,   8351,   8545,   8739,   8932,   9126,   9319,
    9511,   9703,   9895,  10087,  10278,  10469,  10659,  10849,
    11038,  11227,  11416,  11604,  11792,  11980,  12166,  12353,
    12539,  12724,  12909,  13094,  13278,  13462,  13645,  13827,
    14009,  14191,  14372,  14552,  14732,  14911,  15090,  15268,
    15446,  15623,  15799,  15975,  16150,  16325,  16499,  16672,
    16845,  17017,  17189,  17360,  17530,  17699,  17868,  18036,
    18204,  18371,  18537,  18702,  18867,  19031,  19194,  19357,
    19519,  19680,  19840,  20000,  20159,  20317,  20474,  20631,
    20787,  20942,  21096,  21249,  21402,  21554,  21705,  21855,
    22004,  22153,  22301,  22448,  22594,  22739,  22883,  23027,
    23169,  23311,  23452,  23592,  23731,  23869,  24006,  24143,
    24278,  24413,  24546,  24679,  24811,  24942,  25072,  25201,
    25329,  25456,  25582,  25707,  25831,  25954,  26077,  26198,
    26318,  26437,  26556,  26673,  26789,  26905,  27019,  27132,
    27244,  27355,  27466,  27575,  27683,  27790,  27896,  28001,
    28105,  28208,  28309,  28410,  28510,  28608,  28706,  28802,
    28897,  28992,  29085,  29177,  29268,  29358,  29446,  29534,
    29621,  29706,  29790,  29873,  29955,  30036,  30116,  30195,
    30272,  30349,  30424,  30498,  30571,  30643,  30713,  30783,
    30851,  30918,  30984,  31049,
    31113,  31175,  31236,  31297,
    31356,  31413,  31470,  31525,  31580,  31633,  31684,  31735,
    31785,  31833,  31880,  31926,  31970,  32014,  32056,  32097,
    32137,  32176,  32213,  32249,  32284,  32318,  32350,  32382,
    32412,  32441,  32468,  32495,  32520,  32544,  32567,  32588,
    32609,  32628,  32646,  32662,  32678,  32692,  32705,  32717,
    32727,  32736,  32744,  32751,  32757,  32761,  32764,  32766,
    32767,  32766,  32764,  32761,  32757,  32751,  32744,  32736,
    32727,  32717,  32705,  32692,  32678,  32662,  32646,  32628,
    32609,  32588,  32567,  32544,  32520,  32495,  32468,  32441,
    32412,  32382,  32350,  32318,  32284,  32249,  32213,  32176,
    32137,  32097,  32056,  32014,  31970,  31926,  31880,  31833,
    31785,  31735,  31684,  31633,  31580,  31525,  31470,  31413,
    31356,  31297,  31236,  31175,  31113,  31049,  30984,  30918,
    30851,  30783,  30713,  30643,  30571,  30498,  30424,  30349,
    30272,  30195,  30116,  30036,  29955,  29873,  29790,  29706,
    29621,  29534,  29446,  29358,  29268,  29177,  29085,  28992,
    28897,  28802,  28706,  28608,  28510,  28410,  28309,  28208,
    28105,  28001,  27896,  27790,  27683,  27575,  27466,  27355,
    27244,  27132,  27019,  26905,  26789,  26673,  26556,  26437,
    26318,  26198,  26077,  25954,  25831,  25707,  25582,  25456,
    25329,  25201,  25072,  24942,  24811,  24679,  24546,  24413,
    24278,  24143,  24006,  23869,  23731,  23592,  23452,  23311,
    23169,  23027,  22883,  22739,  22594,  22448,  22301,  22153,
    22004,  21855,  21705,  21554,  21402,  21249,  21096,  20942,
    20787,  20631,  20474,  20317,  20159,  20000,  19840,  19680,
    19519,  19357,  19194,  19031,  18867,  18702,  18537,  18371,
    18204,  18036,  17868,  17699,  17530,  17360,  17189,  17017,
    16845,  16672,  16499,  16325,  16150,  15975,  15799,  15623,
    15446,  15268,  15090,  14911,  14732,  14552,  14372,  14191,
    14009,  13827,  13645,  13462,  13278,  13094,  12909,  12724,
    12539,  12353,  12166,  11980,  11792,  11604,  11416,  11227,
    11038,  10849,  10659,  10469,  10278,  10087,   9895,   9703,
    9511,   9319,   9126,   8932,   8739,   8545,   8351,   8156,
    7961,   7766,   7571,   7375,   7179,   6982,   6786,   6589,
    6392,   6195,   5997,   5799,   5601,   5403,   5205,   5006,
    4807,   4608,   4409,   4210,   4011,   3811,   3611,   3411,
    3211,   3011,   2811,   2610,   2410,   2209,   2009,   1808,
    1607,   1406,   1206,   1005,    804,    603,    402,    201,
    0,   -201,   -402,   -603,   -804,  -1005,  -1206,  -1406,
    -1607,  -1808,  -2009,  -2209,  -2410,  -2610,  -2811,  -3011,
    -3211,  -3411,  -3611,  -3811,  -4011,  -4210,  -4409,  -4608,
    -4807,  -5006,  -5205,  -5403,  -5601,  -5799,  -5997,  -6195,
    -6392,  -6589,  -6786,  -6982,  -7179,  -7375,  -7571,  -7766,
    -7961,  -8156,  -8351,  -8545,  -8739,  -8932,  -9126,  -9319,
    -9511,  -9703,  -9895, -10087, -10278, -10469, -10659, -10849,
    -11038, -11227, -11416, -11604, -11792, -11980, -12166, -12353,
    -12539, -12724, -12909, -13094, -13278, -13462, -13645, -13827,
    -14009, -14191, -14372, -14552, -14732, -14911, -15090, -15268,
    -15446, -15623, -15799, -15975, -16150, -16325, -16499, -16672,
    -16845, -17017, -17189, -17360, -17530, -17699, -17868, -18036,
    -18204, -18371, -18537, -18702, -18867, -19031, -19194, -19357,
    -19519, -19680, -19840, -20000, -20159, -20317, -20474, -20631,
    -20787, -20942, -21096, -21249, -21402, -21554, -21705, -21855,
    -22004, -22153, -22301, -22448, -22594, -22739, -22883, -23027,
    -23169, -23311, -23452, -23592, -23731, -23869, -24006, -24143,
    -24278, -24413, -24546, -24679, -24811, -24942, -25072, -25201,
    -25329, -25456, -25582, -25707, -25831, -25954, -26077, -26198,
    -26318, -26437, -26556, -26673, -26789, -26905, -27019, -27132,
    -27244, -27355, -27466, -27575, -27683, -27790, -27896, -28001,
    -28105, -28208, -28309, -28410, -28510, -28608, -28706, -28802,
    -28897, -28992, -29085, -29177, -29268, -29358, -29446, -29534,
    -29621, -29706, -29790, -29873, -29955, -30036, -30116, -30195,
    -30272, -30349, -30424, -30498, -30571, -30643, -30713, -30783,
    -30851, -30918, -30984, -31049, -31113, -31175, -31236, -31297,
    -31356, -31413, -31470, -31525, -31580, -31633, -31684, -31735,
    -31785, -31833, -31880, -31926, -31970, -32014, -32056, -32097,
    -32137, -32176, -32213, -32249, -32284, -32318, -32350, -32382,
    -32412, -32441, -32468, -32495, -32520, -32544, -32567, -32588,
    -32609, -32628, -32646, -32662, -32678, -32692, -32705, -32717,
    -32727, -32736, -32744, -32751, -32757, -32761, -32764, -32766,
    -32767, -32766, -32764, -32761, -32757, -32751, -32744, -32736,
    -32727, -32717, -32705, -32692, -32678, -32662, -32646, -32628,
    -32609, -32588, -32567, -32544, -32520, -32495, -32468, -32441,
    -32412, -32382, -32350, -32318, -32284, -32249, -32213, -32176,
    -32137, -32097, -32056, -32014, -31970, -31926, -31880, -31833,
    -31785, -31735, -31684, -31633, -31580, -31525, -31470, -31413,
    -31356, -31297, -31236, -31175, -31113, -31049, -30984, -30918,
    -30851, -30783, -30713, -30643, -30571, -30498, -30424, -30349,
    -30272, -30195, -30116, -30036, -29955, -29873, -29790, -29706,
    -29621, -29534, -29446, -29358, -29268, -29177, -29085, -28992,
    -28897, -28802, -28706, -28608, -28510, -28410, -28309, -28208,
    -28105, -28001, -27896, -27790, -27683, -27575, -27466, -27355,
    -27244, -27132, -27019, -26905, -26789, -26673, -26556, -26437,
    -26318, -26198, -26077, -25954, -25831, -25707, -25582, -25456,
    -25329, -25201, -25072, -24942, -24811, -24679, -24546, -24413,
    -24278, -24143, -24006, -23869, -23731, -23592, -23452, -23311,
    -23169, -23027, -22883, -22739, -22594, -22448, -22301, -22153,
    -22004, -21855, -21705, -21554, -21402, -21249, -21096, -20942,
    -20787, -20631, -20474, -20317, -20159, -20000, -19840, -19680,
    -19519, -19357, -19194, -19031, -18867, -18702, -18537, -18371,
    -18204, -18036, -17868, -17699, -17530, -17360, -17189, -17017,
    -16845, -16672, -16499, -16325, -16150, -15975, -15799, -15623,
    -15446, -15268, -15090, -14911, -14732, -14552, -14372, -14191,
    -14009, -13827, -13645, -13462, -13278, -13094, -12909, -12724,
    -12539, -12353, -12166, -11980, -11792, -11604, -11416, -11227,
    -11038, -10849, -10659, -10469, -10278, -10087,  -9895,  -9703,
    -9511,  -9319,  -9126,  -8932,  -8739,  -8545,  -8351,  -8156,
    -7961,  -7766,  -7571,  -7375,  -7179,  -6982,  -6786,  -6589,
    -6392,  -6195,  -5997,  -5799,  -5601,  -5403,  -5205,  -5006,
    -4807,  -4608,  -4409,  -4210,  -4011,  -3811,  -3611,  -3411,
    -3211,  -3011,  -2811,  -2610,  -2410,  -2209,  -2009,  -1808,
    -1607,  -1406,  -1206,  -1005,   -804,   -603,   -402,   -201,
};


/*------------------------------------------------------------------------------
|    OMX_AudioProcessor::OMX_AudioProcessor
+-----------------------------------------------------------------------------*/
OMX_AudioProcessor::OMX_AudioProcessor(QObject* parent) :
    QObject(parent),
    m_source("")
{
    moveToThread(&m_thread);
    m_thread.start();
}

void OMX_AudioProcessor::play()
{
    QMetaObject::invokeMethod(this, "audioDecoding");
}

/*------------------------------------------------------------------------------
|    OMX_AudioProcessor::play
+-----------------------------------------------------------------------------*/
void OMX_AudioProcessor::audioDecoding()
{
    // 0 = headphones, 1 = hdmi
    int audioDest  = 1;
    int sampleRate = 48000;
    int channels   = 1;
    int bitDepths  = sizeof(float)*8;
    int bufferSize = (sampleRate*bitDepths*channels) >> 3;
    int phase      = 0;
    int inc        = 256 << 16;
    int dinc       = 0;
    LOG_DEBUG(LOG_TAG, "Size: %d.", sizeof(float));
    //uint8_t inbuf[AUDIO_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE];
    uint8_t* inbuf;
    assert(posix_memalign((void**)&inbuf, 16, AUDIO_INBUF_SIZE + FF_INPUT_BUFFER_PADDING_SIZE) >= 0);
    int len;

    // Open source.
    QFile audioFile("/home/pi/usb/big_buck_bunny_1080p_h264.aac");
    if (!audioFile.exists()) {
        LOG_WARNING(LOG_TAG, "Can't find source.");
        return;
    }
    if (!audioFile.open(QIODevice::ReadOnly)) {
        LOG_WARNING(LOG_TAG, "Can't open source.");
        return;
    }

    // Init.
    OMX_Core* core = (OMX_Core::instance());

#ifdef ENABLE_ENCODINGS
    OMXComponentShared compDecoder = OMXComponentFactory<OMXComponent>::getInstance(
                "OMX.broadcom.audio_decode"
                );
    for (int i = 0; true; i++) {
        OMX_AUDIO_PARAM_PORTFORMATTYPE portFormat;
        portFormat.nSize = sizeof(OMX_AUDIO_PARAM_PORTFORMATTYPE);
        portFormat.nVersion.nVersion = OMX_VERSION;
        portFormat.nPortIndex = 120;
        portFormat.nIndex = i;
        OMX_ERRORTYPE err = compDecoder->GetParameter(OMX_IndexParamAudioPortFormat, &portFormat);
        assert(err != OMX_ErrorNoMore);
        LOG_DEBUG(LOG_TAG, "Encoding: %d.", portFormat.eEncoding);
    }
#endif

    OMXComponentShared compRenderer = OMXComponentFactory<OMXComponent>::getInstance(
                "OMX.broadcom.audio_render"
                );
    LOG_VERBOSE(LOG_TAG, "Disabling input buffers...");
    compRenderer->sendCommand(OMX_CommandPortDisable, 100, NULL);
    compRenderer->sendCommand(OMX_CommandPortDisable, 101, NULL);
    compRenderer->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, 100, TIMEOUT_MS);
    compRenderer->waitForEvent(OMX_EventCmdComplete, OMX_CommandPortDisable, 101, TIMEOUT_MS);

    // Set up number and size of the buffers.
    OMX_PARAM_PORTDEFINITIONTYPE portdef;
    memset(&portdef, 0, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
    portdef.nSize             = sizeof(OMX_PARAM_PORTDEFINITIONTYPE);
    portdef.nVersion.nVersion = OMX_VERSION;
    portdef.nPortIndex        = 100;
    compRenderer->GetParameter(OMX_IndexParamPortDefinition, &portdef);

    portdef.nBufferSize        = bufferSize >> 2;
    portdef.nBufferCountActual = 300;
    compRenderer->SetParameter(OMX_IndexParamPortDefinition, &portdef);
    LOG_DEBUG(LOG_TAG, "OMX buffer size set to: %d.", bufferSize >> 2);

    // set the pcm parameters
    OMX_AUDIO_PARAM_PCMMODETYPE pcm;
    memset(&pcm, 0, sizeof(OMX_AUDIO_PARAM_PCMMODETYPE));
    pcm.nSize             = sizeof(OMX_AUDIO_PARAM_PCMMODETYPE);
    pcm.nVersion.nVersion = OMX_VERSION;
    pcm.nPortIndex        = 100;
    pcm.nChannels         = channels == 6 ? 8 : channels;
    pcm.eNumData          = OMX_NumericalDataSigned;
    pcm.eEndian           = OMX_EndianLittle;
    pcm.nSamplingRate     = sampleRate;
    pcm.bInterleaved      = OMX_TRUE;
    pcm.nBitPerSample     = bitDepths;
    pcm.ePCMMode          = OMX_AUDIO_PCMModeLinear;

    switch (channels) {
    case 6:
        pcm.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
        pcm.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
        pcm.eChannelMapping[2] = OMX_AUDIO_ChannelCF;
        pcm.eChannelMapping[3] = OMX_AUDIO_ChannelLFE;
        pcm.eChannelMapping[4] = OMX_AUDIO_ChannelLR;
        pcm.eChannelMapping[5] = OMX_AUDIO_ChannelRR;
        pcm.eChannelMapping[6] = OMX_AUDIO_ChannelNone;
        pcm.eChannelMapping[7] = OMX_AUDIO_ChannelNone;
        break;
    case 1:
        pcm.eChannelMapping[0] = OMX_AUDIO_ChannelCF;
        break;
    case 8:
        pcm.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
        pcm.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
        pcm.eChannelMapping[2] = OMX_AUDIO_ChannelCF;
        pcm.eChannelMapping[3] = OMX_AUDIO_ChannelLFE;
        pcm.eChannelMapping[4] = OMX_AUDIO_ChannelLR;
        pcm.eChannelMapping[5] = OMX_AUDIO_ChannelRR;
        pcm.eChannelMapping[6] = OMX_AUDIO_ChannelLS;
        pcm.eChannelMapping[7] = OMX_AUDIO_ChannelRS;
        break;
    case 4:
        pcm.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
        pcm.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
        pcm.eChannelMapping[2] = OMX_AUDIO_ChannelLR;
        pcm.eChannelMapping[3] = OMX_AUDIO_ChannelRR;
        break;
    case 2:
        pcm.eChannelMapping[0] = OMX_AUDIO_ChannelLF;
        pcm.eChannelMapping[1] = OMX_AUDIO_ChannelRF;
        break;
    }

    LOG_VERBOSE(LOG_TAG, "Setting PCM parameters...");
    compRenderer->SetParameter(OMX_IndexParamAudioPcm, &pcm);

    LOG_VERBOSE(LOG_TAG, "Changing state to IDLE...");
    compRenderer->sendCommand(OMX_CommandStateSet, OMX_StateIdle, NULL);
    compRenderer->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateIdle, TIMEOUT_MS);

    LOG_VERBOSE(LOG_TAG, "Providing buffers...");
    compRenderer->enablePortBuffers(100);

    LOG_VERBOSE(LOG_TAG, "Change state to EXECUTING...");
    compRenderer->sendCommand(OMX_CommandStateSet, OMX_StateExecuting, NULL);
    compRenderer->waitForEvent(OMX_EventCmdComplete, OMX_CommandStateSet, OMX_StateExecuting, TIMEOUT_MS);

    LOG_VERBOSE(LOG_TAG, "Setting audio output...");
    OMX_CONFIG_BRCMAUDIODESTINATIONTYPE dest;
    memset(&dest, 0 , sizeof(OMX_CONFIG_BRCMAUDIODESTINATIONTYPE));
    dest.nSize = sizeof(OMX_CONFIG_BRCMAUDIODESTINATIONTYPE);
    dest.nVersion.nVersion = OMX_VERSION;
    strcpy((char*)dest.sName, "hdmi");
    if (OMX_SetConfig(compRenderer->GetHandle(), OMX_IndexConfigBrcmAudioDestination, &dest) != OMX_ErrorNone)
        throw runtime_error("Failed to set config.");

    av_register_all();
    AVFormatContext* formatContext = avformat_alloc_context(); // TODO: free this.
    if (avformat_open_input(&formatContext, "/home/pi/usb/big_buck_bunny_1080p_h264.aac", NULL, NULL) != 0)
        throw std::runtime_error("Error while calling avformat_open_input (probably invalid file format)");

    if (avformat_find_stream_info(formatContext, NULL) < 0)
        throw std::runtime_error("Error while calling avformat_find_stream_info");

    LOG_INFORMATION(LOG_TAG, "Found %u streams.", formatContext->nb_streams);
    for (unsigned int i = 0; i < formatContext->nb_streams; ++i) {
        LOG_INFORMATION(LOG_TAG, "Stream %u: duration is %lld.", i, formatContext->streams[i]->duration);
        LOG_INFORMATION(LOG_TAG, "Codec type: %d.", formatContext->streams[i]->codec->codec_type);
        LOG_INFORMATION(LOG_TAG, "Codec ID: %d.", formatContext->streams[i]->codec->codec_id);
#if 0
        auto stream = avFormat->streams[i];		// pointer to a structure describing the stream
        auto codecType = stream->codec->codec_type;	// the type of data in this stream, notable values are AVMEDIA_TYPE_VIDEO and AVMEDIA_TYPE_AUDIO
        auto codecID = stream->codec->codec_id;		// identifier for the codec
#endif
    }


    // getting the required codec structure
    AVStream* stream = formatContext->streams[0];
    AVCodec* codec = avcodec_find_decoder(formatContext->streams[0]->codec->codec_id);
    if (codec == NULL)
        throw std::runtime_error("Codec required by file not available");

    // allocating a structure
    AVCodecContext* codecContext = avcodec_alloc_context3(codec);

    // we need to make a copy of videoStream->codec->extradata and give it to the context
    // make sure that this vector exists as long as the avVideoCodec exists
    //vector<uint8_t*> codecContextExtraData(stream->codec->extradata, stream->codec->extradata + stream->codec->extradata_size);
    //codecContext->extradata = codecContextExtraData.data();
    //codecContext->extradata_size = codecContextExtraData.size();

    // initializing the structure by opening the codec
    if (avcodec_open2(codecContext, codec, NULL) < 0)
        throw std::runtime_error("Could not open codec");

    AVPacket packet;
    av_init_packet(&packet);
    packet.data = inbuf;
    packet.size = audioFile.read((char*)inbuf, AUDIO_INBUF_SIZE);
    //AVCodecContext* codec = formatContext->streams[0]->codec; // TODO: Choose stream somehow.
    AVFrame* decoded_frame = NULL;

    // Start decoding.
    while (packet.size > 0) {
        int gotFrame = 0;

        // Alloc decoded frame or set default values.
        if (!decoded_frame) {
            LOG_VERBOSE(LOG_TAG, "Allocating frame...");
            if (!(decoded_frame = avcodec_alloc_frame())) {
                LOG_ERROR(LOG_TAG, "Could not allocate audio frame.");
                exit(1);
                // TODO: Send error event from component.
            }
        }
        else
            avcodec_get_frame_defaults(decoded_frame);

        // Actually decode the frame.
        LOG_VERBOSE(LOG_TAG, "Decoding...");
        len = avcodec_decode_audio4(codecContext, decoded_frame, &gotFrame, &packet);
        if (len < 0) {
            LOG_ERROR(LOG_TAG, "Error while decoding.");
            exit(1);
            // TODO: Error signal.
        }
        if (gotFrame) {
            // if a frame has been decoded, output it.
            LOG_VERBOSE(LOG_TAG, "Got frame!");
            int bufferSize = av_samples_get_buffer_size(
                        NULL, codecContext->channels,
                        decoded_frame->nb_samples,
                        codecContext->sample_fmt, 1
                        );
#if 0
            LOG_INFORMATION(LOG_TAG, "Channels: %d.", codecContext->channels);
            LOG_INFORMATION(LOG_TAG, "Samples: %d.", decoded_frame->nb_samples);
            LOG_INFORMATION(LOG_TAG, "Format: %d.", codecContext->sample_fmt);
            LOG_INFORMATION(LOG_TAG, "Sample rate: %d.", codecContext->sample_rate);
            LOG_INFORMATION(LOG_TAG, "Buffer size: %d.", bufferSize);
#endif
#if 0
            FILE* outfile = fopen("/home/pi/output.pcm", "a");
            fwrite(decoded_frame->data[0], 1, bufferSize/codecContext->channels, outfile);
            fclose(outfile);
#endif
            // TODO: Send to OpenMAX component.
            LOG_VERBOSE(LOG_TAG, "Trying to acquire buffer...");
            OMX_BUFFERHEADERTYPE* buffer;
            if (!compRenderer->waitForInputBufferReady(100, buffer)) {
                LOG_WARNING(LOG_TAG, "Failed to acquire buffer.");
                return;
            }

            // TODO: Can I avoid the copy here?
            memcpy(buffer->pBuffer, decoded_frame->data[0], bufferSize/6.0);
            //buffer->pBuffer     = (OMX_U8*)decoded_frame->data;
            buffer->pAppPrivate = NULL;
            buffer->nOffset     = 0;
            buffer->nFilledLen  = bufferSize/6.0;
            compRenderer->EmptyThisBuffer(buffer);
            LOG_VERBOSE(LOG_TAG, "EmptyThisBuffer called!");
        }
        packet.size -= len;
        packet.data += len;
        packet.dts = packet.pts = AV_NOPTS_VALUE;
        if (packet.size < AUDIO_REFILL_THRESH) {
            /* Refill the input buffer, to avoid trying to decode
             * incomplete frames. Instead of this, one could also use
             * a parser, or use a proper container format through
             * libavformat. */
            memmove(inbuf, packet.data, packet.size);
            packet.data = inbuf;
            len = audioFile.read((char*)(packet.data + packet.size), AUDIO_INBUF_SIZE - packet.size);
            if (len > 0)
                packet.size += len;
        }
    }

#if 0
    // iterate for 5 seconds worth of packets
    for (int n = 0; n < ((sampleRate*5)/ BUFFER_SIZE_SAMPLES); n++) {
        uint8_t* buf;
        int16_t* p;
#if 0
        uint32_t latency;
#endif

        LOG_VERBOSE(LOG_TAG, "Trying to acquire buffer...");
        OMX_BUFFERHEADERTYPE* buffer;
        if (!compRenderer->waitForInputBufferReady(100, buffer)) {
            LOG_WARNING(LOG_TAG, "Failed to acquire buffer.");
            return;
        }

        buf = (uint8_t*)buffer->pBuffer;
        p   = (int16_t*)buf;

        // fill the buffer
        for (int i = 0; i < BUFFER_SIZE_SAMPLES; i++) {
            int16_t val = SIN(phase);
            phase += inc >> 16;
            inc   += dinc;
            if (inc >> 16 < 512)
                dinc++;
            else
                dinc--;

            for (int j = 0; j < channels; j++) {
                if (bitDepths == 32)
                    *p++ = 0;
                *p++ = val;
            }
        }

        // Try and wait for a minimum latency time (in ms) before
        // sending the next packet
#if 0
        LOG_DEBUG(LOG_TAG, "Waiting for latency...");
        latency = getLatency(compRenderer->GetHandle());
        while (latency > (sampleRate*(MIN_LATENCY_TIME + CTTW_SLEEP_TIME)/1000))
            usleep(CTTW_SLEEP_TIME*1000);
#endif

        //ret = audioplay_play_buffer(st, buf, buffer_size);
        //assert(ret == 0);
        buffer->pAppPrivate = NULL;
        buffer->nOffset = 0;
        buffer->nFilledLen = bufferSize;
        compRenderer->EmptyThisBuffer(buffer);
        LOG_VERBOSE(LOG_TAG, "EmptyThisBuffer called!");
    }
#endif

    // TODO: Should be destroyed after.
    core->destroyInstance();
}

/*------------------------------------------------------------------------------
|    OMX_AudioProcessor::getLatency
+-----------------------------------------------------------------------------*/
uint32_t OMX_AudioProcessor::getLatency(OMX_HANDLETYPE renderer)
{
    OMX_PARAM_U32TYPE param;
    memset(&param, 0, sizeof(OMX_PARAM_U32TYPE));
    param.nSize = sizeof(OMX_PARAM_U32TYPE);
    param.nVersion.nVersion = OMX_VERSION;
    param.nPortIndex = 100;

    // TODO: instance member getConfig.
    assert(OMX_GetConfig(renderer, OMX_IndexConfigAudioRenderingLatency, &param) == OMX_ErrorNone);
    return param.nU32;
}
