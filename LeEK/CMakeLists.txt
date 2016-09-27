cmake_minimum_required(VERSION 2.8)

project(leek)

#need to specify processor feature flags.
#SSE in particular is necessary - HP Pavilion g71 under Ubuntu 12.04 LTE requires that you specify SSE1-2
if(CMAKE_COMPILER_IS_GNUCXX)
    add_definitions(-O3 -msse -msse2 -mfpmath=sse -std=c++0x `freetype-config --cflags`)
endif()

#list required libraries
find_package(Boost 1.54 COMPONENTS filesystem system REQUIRED)
find_package(OpenGL REQUIRED)

#setup bullet vars
#...should really fix these to search for these files instead
set(Bullet_LIBRARIES BulletDynamics BulletSoftBody BulletCollision LinearMath)
set(Bullet_INCLUDE_DIRS /usr/include/bullet/ /usr/include/bullet/BulletCollision /usr/include/bullet/BulletDynamics /usr/include/bullet/BulletSoftBody /usr/include/bullet/LinearMath /usr/include/bullet/vectormath)

#specify header locations
include_directories(.)
include_directories(/usr/include)
include_directories(${Bullet_INCLUDE_DIRS})
include_directories(${Boost_INCLUDE_DIR})
include_directories(${OpenGL_INCLUDE_DIRS})

#specify where library files are
link_directories(/usr/lib)
link_directories(${OpenGL_LIBRARY_DIRS})
add_definitions(${OpenGL_DEFINITIONS})
IF(NOT OPENGL_FOUND)
    message(ERROR " OPENGL not found!")
ENDIF(NOT OPENGL_FOUND)

#now actually add the project's source
file(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

FOREACH(item ${sources})
    #remove Windows objects
    IF(${item} MATCHES "Libraries/GL_Loaders/WGL/*")
        LIST(REMOVE_ITEM sources ${item})
    ENDIF(${item} MATCHES "Libraries/GL_Loaders/WGL/*")
    IF(${item} MATCHES "Libraries/Bullet_2_81/*")
        LIST(REMOVE_ITEM sources ${item})
    ENDIF(${item} MATCHES "Libraries/Bullet_2_81/*")
    #and if the cmake files are generated 
    #inside the project directory,
    #ignore those too
    IF(${item} MATCHES "CMakeFiles/*")
        LIST(REMOVE_ITEM sources ${item})
    ENDIF(${item} MATCHES "CMakeFiles/*")
ENDFOREACH(item)

#currently outputting an executable,
#could be a library for editors
add_executable(leek ${sources})

#specify libraries the project needs;
#the order does in fact matter
target_link_libraries(	leek
			${OPENGL_LIBRARIES}
			${Boost_LIBRARIES}
			png
			z
			freetype
			${Bullet_LIBRARIES}
			)
