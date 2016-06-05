#pragma once

#if WIN32
#   define SR_PLATFORM_WIN
#   define SR_DATA_EXPORT
#else
#   define SR_PLATFORM_LINUX
#endif
