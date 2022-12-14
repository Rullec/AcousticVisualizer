# - Find Intel MKL
find_path(MKL_INCLUDE_DIR mkl.h
    PATHS ENV INCLUDE
    PATHS ${SYSTEM_INC_PATH}
    PATHS $ENV{MKLROOT}/include
)
message("MKL_INCLUDE_DIR = " ${MKL_INCLUDE_DIR})
message("LIBRARY_PATH = " $ENV{LIBRARY_PATH} )
find_library(MKL_LIBRARY libmkl_core.a mkl_core
    PATHS ENV LIBRARY_PATH
    PATHS ENV LD_LIBRARY_PATH
    PATHS ${SYSTEM_LIB_PATH}
    PATHS $ENV{MKLROOT}/lib
    PATHS $ENV{MKLROOT}/lib/intel64
)
message("MKL_LIBRARY = " ${MKL_LIBRARY})

set(_IOMP5_LIB iomp5)
if (WIN32)
  if (MKL_USE_STATIC_LIBS)
      list(APPEND _IOMP5_LIB libiomp5mt.lib)
  else()
      list(APPEND _IOMP5_LIB libiomp5md.lib)
  endif()
endif()

find_library(IOMP5_LIBRARY
    NAMES ${_IOMP5_LIB}
    PATHS ENV LD_LIBRARY_PATH
    PATHS ${SYSTEM_LIB_PATH}
    PATHS ENV LIBRARY_PATH
    DOC "Path to OpenMP runtime library"
)

if (MKL_INCLUDE_DIR AND MKL_LIBRARY)
    if (NOT MKL_FIND_QUIETLY)
        MESSAGE(STATUS "Found MKL: ${MKL_LIBRARY}")
    endif (NOT MKL_FIND_QUIETLY)
else (MKL_INCLUDE_DIR AND MKL_LIBRARY)
    if (MKL_FIND_REQUIRED)
        MESSAGE(FATAL_ERROR "Could not find MKL: $ENV{INCLUDE} $ENV{LD_LIBRARY_PATH}")
    else (MKL_FIND_REQUIRED)
        if ( NOT MKL_LIBRARY )
            MESSAGE(STATUS "WARNING: Could not find MKL library: $ENV{LD_LIBRARY_PATH}")
        else ( NOT MKL_LIBRARY )
            MESSAGE(STATUS "WARNING: Could not find MKL include: $ENV{INCLUDE}")
        endif ( NOT MKL_LIBRARY )
    endif (MKL_FIND_REQUIRED)
endif (MKL_INCLUDE_DIR AND MKL_LIBRARY)

mark_as_advanced(
    MKL_INCLUDE_DIR
    MKL_LIBRARY
    IOMP5_LIBRARY
)
