export module lysa.math;
#include <cstdint>
using int32 = int32_t;
using uint32 = uint32_t;

#define HLSLPP_FEATURE_TRANSFORM
#define HLSLPP_MODULE_DECLARATION

#include "hlsl++/vector_float.h"
#include "hlsl++/vector_float8.h"

#include "hlsl++/vector_int.h"
#include "hlsl++/vector_uint.h"
#include "hlsl++/vector_double.h"

#include "hlsl++/matrix_float.h"

#include "hlsl++/quaternion.h"
#include "hlsl++/dependent.h"

#include "hlsl++/data_packing.h"