# Find common dependencies such as DLT and GLIB
FIND_PACKAGE(PkgConfig)

IF(WITH_DLT)    
    pkg_check_modules(DLT REQUIRED automotive-dlt>=2.2.0)
    add_definitions(${DLT_CFLAGS_OTHER})
    include_directories(${DLT_INCLUDE_DIRS})
    link_directories(${DLT_LIBRARY_DIRS})
ENDIF(WITH_DLT)

pkg_check_modules(GLIB REQUIRED glib-2.0)
add_definitions(${GLIB_CFLAGS_OTHER})
include_directories(${GLIB_INCLUDE_DIRS})

set(COMMON_LIBRARIES 
	${GLIB_LIBRARIES}
	${DLT_LIBRARIES}
)
