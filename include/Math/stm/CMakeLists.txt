
add_library(stm INTERFACE)

target_sources(stm PUBLIC
                algorithm.h
                common.h
                comparison.h
                complex.h
                concept.h
                constant.h
                conversion.h
                error.h
                fraction.h
                geometry.h
                literals.h
                math_internal.h
                "math.h"
                matrix.h
                numeric.h
                polar_complex.h
                quaternion.h
                spatial_transform.h
                units.h
                utilities.h
                vector.h
                vector2.h
                vector3.h
                vector4.h)

target_compile_features(stm INTERFACE cxx_std_20)
target_include_directories(stm INTERFACE ..)