#pragma once

#ifdef QTZ_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <windowsx.h>
#undef CreateWindow
#undef max
#undef min
#else
#endif // QTZ_PLATFORM_WIN32

namespace Quartz
{

} // namespace Quartz
