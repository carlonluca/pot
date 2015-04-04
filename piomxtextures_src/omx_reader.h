#ifndef OMX_READER_H
#define OMX_READER_H

#include <QString>

#include "OMXReader.h"

/**
 * @brief The OMX_Reader class Class used to extend the functionalities of
 * the OMXReader class from omxplayer. This is to avoid modifications to the
 * omxplayer code.
 */
class OMX_Reader : public OMXReader
{
public:
   OMX_Reader() : OMXReader() {}
   ~OMX_Reader() {}

   AVDictionary* getMetadata();
};

#endif // OMX_READER_H
