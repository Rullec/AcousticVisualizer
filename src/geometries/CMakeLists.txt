
add_library(
    geo_lib
    ObjExport.cpp
    Primitives.cpp
    Triangulator.cpp
    ObjUtil.cpp
    # Raycaster.cpp
    BezierCurve.cpp
    Tetrahedron.cpp
    TriangulatorDelaunay.cpp
    ObjectBVH.cpp
    AABB.cpp
)

target_link_libraries(geo_lib utils_lib)