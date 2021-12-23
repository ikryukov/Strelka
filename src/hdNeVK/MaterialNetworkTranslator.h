#pragma once

#include <pxr/usd/sdf/path.h>

#include <memory>

namespace MaterialX
{
  using DocumentPtr = std::shared_ptr<class Document>;
}

PXR_NAMESPACE_OPEN_SCOPE

struct HdMaterialNetwork2;

class MaterialNetworkTranslator
{
public:
  MaterialNetworkTranslator(const std::string& mtlxLibPath);

  std::string ParseNetwork(const SdfPath& id,
                            const HdMaterialNetwork2& network) const;

private:
  MaterialX::DocumentPtr CreateMaterialXDocumentFromNetwork(const SdfPath& id,
                                                            const HdMaterialNetwork2& network) const;

private:
  MaterialX::DocumentPtr m_nodeLib;
};

PXR_NAMESPACE_CLOSE_SCOPE
