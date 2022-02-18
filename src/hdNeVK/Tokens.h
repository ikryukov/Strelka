#pragma once

#include <pxr/base/tf/staticTokens.h>

PXR_NAMESPACE_OPEN_SCOPE

#define HD_NEVK_SETTINGS_TOKENS                  \
  ((spp, "spp"))                                    \
  ((max_bounces, "max-bounces"))                    

// mtlx node identifier is given by usdMtlx.
#define HD_NEVK_NODE_IDENTIFIER_TOKENS           \
  (mtlx)                                         \
  (mdl)

#define HD_NEVK_SOURCE_TYPE_TOKENS               \
  (mtlx)                                         \
  (mdl)

#define HD_NEVK_DISCOVERY_TYPE_TOKENS            \
  (mtlx)                                         \
  (mdl)

#define HD_NEVK_RENDER_CONTEXT_TOKENS            \
  (mtlx)                                         \
  (mdl)

#define HD_NEVK_NODE_CONTEXT_TOKENS              \
  (mtlx)                                         \
  (mdl)

#define HD_NEVK_NODE_METADATA_TOKENS             \
  (subIdentifier)

TF_DECLARE_PUBLIC_TOKENS(HdNeVKSettingsTokens,  HD_NEVK_SETTINGS_TOKENS);
TF_DECLARE_PUBLIC_TOKENS(HdNeVKNodeIdentifiers, HD_NEVK_NODE_IDENTIFIER_TOKENS);
TF_DECLARE_PUBLIC_TOKENS(HdNeVKSourceTypes,     HD_NEVK_SOURCE_TYPE_TOKENS);
TF_DECLARE_PUBLIC_TOKENS(HdNeVKDiscoveryTypes,  HD_NEVK_DISCOVERY_TYPE_TOKENS);
TF_DECLARE_PUBLIC_TOKENS(HdNeVKRenderContexts,  HD_NEVK_RENDER_CONTEXT_TOKENS);
TF_DECLARE_PUBLIC_TOKENS(HdNeVKNodeContexts,    HD_NEVK_NODE_CONTEXT_TOKENS);
TF_DECLARE_PUBLIC_TOKENS(HdNeVKNodeMetadata,    HD_NEVK_NODE_METADATA_TOKENS);

PXR_NAMESPACE_CLOSE_SCOPE
