#include "omx_reader.h"

AVDictionary* OMX_Reader::getMetadata()
{
   if (!m_pFormatContext)
      return NULL;
   return m_pFormatContext->metadata;
}
