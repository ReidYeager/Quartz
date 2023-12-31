#pragma once

#include "quartz/defines.h"
#include "quartz/platform/defines.h"

namespace Quartz
{

uint32_t PlatformLoadFile(void** buffer, const char* path);

} // namespace Quartz