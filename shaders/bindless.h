#pragma once

#ifdef __cplusplus
#include <cstdint>
#endif

#ifdef __APPLE__
static const uint32_t BINDLESS_TEXTURE_COUNT = 8;
static const uint32_t BINDLESS_SAMPLER_COUNT = 2;
#else
static const uint32_t BINDLESS_TEXTURE_COUNT = 2048;
static const uint32_t BINDLESS_SAMPLER_COUNT = 36;
#endif
