find_path( GLEW_INCLUDE_DIR NAMES GL/glew.h
	HINTS
		${GLEW_ROOT}
		${GLEW_ROOT}/include )
find_library(GLEW_LIBRARY NAMES glew32 glew32sd
	HINTS
		${OPENGL_LIBRARY_DIR}
		${GLEW_ROOT}/Release
		${GLEW_ROOT}/lib
	)
find_library(GLEW_LIBRARY_DEBUG NAMES glew32d glew32sd
	HINTS
		${OPENGL_LIBRARY_DIR}
		${GLEW_ROOT}/Debug
		${GLEW_ROOT}/lib
	)

set( GLEW_FOUND "NO" )
if(GLEW_INCLUDE_DIR AND GLEW_LIBRARY)
	if(GLEW_GLEW_LIBRARY_DEBUG)
		set(GLEW_LIBRARIES debug ${GLEW_LIBRARY_DEBUG} optimized ${GLEW_LIBRARY})
	else()
		set(GLEW_LIBRARIES ${GLEW_LIBRARY})
	endif()
	SET(GLEW_INCLUDE_DIRS ${GLEW_INCLUDE_DIR} ${GLEW_INCLUDE_DIR}/GL)

	get_filename_component(GLEW_LIBRARY_DIRS "${GLEW_GLEW_LIBRARY}" PATH)
	set( GLEW_FOUND "YES" )
endif()

MARK_AS_ADVANCED(
	GLEW_LIBRARIES
	GLEW_LIBRARY_DIRS
	GLEW_INCLUDE_DIR
	GLEW_INCLUDE_DIRS
	GLEW_LIBRARY
)