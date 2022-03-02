#pragma once

#include <pxr/base/tf/staticTokens.h>

PXR_NAMESPACE_OPEN_SCOPE

#define HD_OKA_SETTINGS_TOKENS \
    ((spp, "spp"))                                    \
    ((max_bounces, "max-bounces"))

// mtlx node identifier is given by usdMtlx.
#define HD_OKA_NODE_IDENTIFIER_TOKENS \
    (mtlx)                                         \
    (mdl)

#define HD_OKA_SOURCE_TYPE_TOKENS \
    (mtlx)                                         \
    (mdl)

#define HD_OKA_DISCOVERY_TYPE_TOKENS \
    (mtlx)                                         \
    (mdl)

#define HD_OKA_RENDER_CONTEXT_TOKENS \
    (mtlx)                                         \
    (mdl)

#define HD_OKA_NODE_CONTEXT_TOKENS \
    (mtlx)                                         \
    (mdl)

#define HD_OKA_NODE_METADATA_TOKENS \
    (subIdentifier)

TF_DECLARE_PUBLIC_TOKENS(HdOkaSettingsTokens, HD_OKA_SETTINGS_TOKENS);
TF_DECLARE_PUBLIC_TOKENS(HdOkaNodeIdentifiers, HD_OKA_NODE_IDENTIFIER_TOKENS);
TF_DECLARE_PUBLIC_TOKENS(HdOkaSourceTypes, HD_OKA_SOURCE_TYPE_TOKENS);
TF_DECLARE_PUBLIC_TOKENS(HdOkaDiscoveryTypes, HD_OKA_DISCOVERY_TYPE_TOKENS);
TF_DECLARE_PUBLIC_TOKENS(HdOkaRenderContexts, HD_OKA_RENDER_CONTEXT_TOKENS);
TF_DECLARE_PUBLIC_TOKENS(HdOkaNodeContexts, HD_OKA_NODE_CONTEXT_TOKENS);
TF_DECLARE_PUBLIC_TOKENS(HdOkaNodeMetadata, HD_OKA_NODE_METADATA_TOKENS);

PXR_NAMESPACE_CLOSE_SCOPE
