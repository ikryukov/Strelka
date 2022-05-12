// generated code; do not edit
#include "../source/core/slang-basic.h"
Slang::String get_slang_hlsl_prelude()
{
Slang::StringBuilder sb;
sb << 
"#ifdef SLANG_HLSL_ENABLE_NVAPI\n"
"#include \"nvHLSLExtns.h\"\n"
"#endif\n"
"\n"
;
return sb.ProduceString();
}
