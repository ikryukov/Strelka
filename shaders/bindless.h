#pragma once

#ifdef __APPLE__
const uint32_t BINDLESS_TEXTURE_COUNT = 8;
const uint32_t BINDLESS_SAMPLER_COUNT = 2;
#else
const uint32_t BINDLESS_TEXTURE_COUNT = 2048;
const uint32_t BINDLESS_SAMPLER_COUNT = 36;
#endif
