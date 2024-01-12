
#include "quartz/defines.h"
#include "quartz/platform/defines.h"
#include "quartz/platform/platform.h"
#include "quartz/platform/filesystem/filesystem.h"

#include <stdlib.h>
#include <stdio.h>

namespace Quartz
{

uint32_t PlatformLoadFile(void** buffer, const char* path)
{
  FILE* inFile;
  if (fopen_s(&inFile, path, "rb"))
  {
    QTZ_ERROR("Failed to load the file '{}'", path);
    return 0;
  }

  fseek(inFile, 0, SEEK_END);
  uint64_t inSize = ftell(inFile);
  char* inBuffer = (char*)malloc(inSize);
  rewind(inFile);

  uint64_t readSize = fread(inBuffer, 1, inSize, inFile);
  if (readSize != inSize)
  {
    QTZ_WARNING("Unexpected end of file in '{}'", path);
    inBuffer = (char*)realloc(inBuffer, readSize);
    inSize = readSize;
  }
  fclose(inFile);

  *buffer = inBuffer;
  return inSize;
}

} // namespace Quartz
