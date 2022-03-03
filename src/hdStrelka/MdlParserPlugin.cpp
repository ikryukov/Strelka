#include "MdlParserPlugin.h"

#include <pxr/base/tf/staticTokens.h>

#include <pxr/usd/sdr/shaderNode.h>
#include <pxr/usd/ar/resolver.h>
#include "pxr/usd/ar/resolvedPath.h"
#include "pxr/usd/ar/asset.h"
#include <pxr/usd/ar/ar.h>

//#include "Tokens.h"

PXR_NAMESPACE_OPEN_SCOPE

NDR_REGISTER_PARSER_PLUGIN(HdStrelkaMdlParserPlugin);

// clang-format off
TF_DEFINE_PRIVATE_TOKENS(_tokens,
    (mdl)
    (subIdentifier));
// clang-format on

NdrNodeUniquePtr HdStrelkaMdlParserPlugin::Parse(const NdrNodeDiscoveryResult& discoveryResult)
{
    NdrTokenMap metadata = discoveryResult.metadata;
    metadata[_tokens->subIdentifier] = discoveryResult.subIdentifier;

    return std::make_unique<SdrShaderNode>(discoveryResult.identifier, discoveryResult.version, discoveryResult.name,
                                           discoveryResult.family, _tokens->mdl, discoveryResult.sourceType,
                                           discoveryResult.uri, discoveryResult.resolvedUri, NdrPropertyUniquePtrVec{},
                                           metadata);
}

const NdrTokenVec& HdStrelkaMdlParserPlugin::GetDiscoveryTypes() const
{
    static NdrTokenVec s_discoveryTypes{ _tokens->mdl };
    return s_discoveryTypes;
}

const TfToken& HdStrelkaMdlParserPlugin::GetSourceType() const
{
    return _tokens->mdl;
}

PXR_NAMESPACE_CLOSE_SCOPE
