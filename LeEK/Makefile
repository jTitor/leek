PROJ := LeEK
all : $(PROJ)


CFLAGS:=-Os -std=c++0x -IMath -I. -IStrings -ILibraries -ILibraries/Bullet_2_81/src \
	-I/usr/include/freetype2
CXXFLAGS:=$(CFLAGS)
LDFLAGS:=-lGL -lz -lxml2 -lfreetype

LIBS:= \
	Libraries/GL_Loaders/GL/gl_core_4_3.o \
	Libraries/TinyXML2/tinyxml2.o \
	Libraries/MurmurHash3/MurmurHash3.o \

LIBSA:= \
	/usr/lib/libboost_system.so.1.46.1 \
	/usr/lib/libboost_filesystem.so.1.46.1 \
	Libraries/LibPNG/x86_64libpng16.a

PRELIBS:= \
	Libraries/Bullet_2_81/lib/x86_64/libBulletDynamics.a \
	Libraries/Bullet_2_81/lib/x86_64/libBulletSoftBody.a \
	Libraries/Bullet_2_81/lib/x86_64/libBulletCollision.a \
	Libraries/Bullet_2_81/lib/x86_64/libLinearMath.a \

PHYSICS:=  \
	Physics/Physics.o\

OBJS:= $(LIBS) $(PHYSICS)\
	LeEK.o \
	Config/Config.o\
	Constants/ControllerConstants.o\
	Constants/GraphicsConstants.o\
	DataStructures/BinaryTree.o\
	DataStructures/IntrusiveList.o\
	DataStructures/RedBlackTreeBase.o\
	FileManagement/ArchiveTypes.o\
	FileManagement/Filesystem.o\
	FileManagement/ModelFile.o\
	FileManagement/path.o\
	FileManagement/StdLibDataStream.o\
	GraphicsWrappers/IGraphicsWrapper.o\
	GraphicsWrappers/OGLGrpWrapper.o\
	Hashing/Hash.o\
	Hashing/HashedString.o\
	Hashing/HashTable.o\
	Input/DeadZone.o\
	Input/Input.o\
	Logging/Log.o\
	Math/Matrix3x3.o\
	Math/Quaternion.o\
	Math/Frustrum.o\
	Math/Matrix4x4.o\
	Math/Vector2.o\
	Math/Transform.o\
	Math/MathFunctions.o\
	Math/Rectangle.o\
	Math/Vector4.o\
	Math/Vector3.o\
	Memory/Allocator.o\
	Memory/HeapLayers.o\
	MultiThreading/StdThreading.o\
	Platforms/IPlatform.o\
	Platforms/LinuxHelpers.o\
	Platforms/LinuxPlatform.o\
	Random/Random.o\
	Rendering/Camera/Camera.o\
	Rendering/Color.o\
	Rendering/Mesh.o\
	Rendering/Renderer.o\
	Rendering/Text.o\
	Rendering/Texture.o\
	Rendering/Geometry.o\
	Rendering/Model.o\
	Rendering/Shader.o\
	Rendering/Font.o\
	ResourceManagement/ResourceManager.o\
	ResourceManagement/ResourceLoaders.o\
	ResourceManagement/Resource.o\
	ResourceManagement/ResourceArchive.o\
	Stats/FPSCounter.o\
	Stats/StatMonitor.o\
	Stats/Profiling.o\
	Stats/AllocStats.o\
	Strings/String.o\
	Strings/StringUtils.o\
	Structures/Handle.o\
	Structures/Actor.o\
	Structures/IDManager.o\
	Structures/Transform.o\
	Structures/Game.o\
	Structures/SceneGraph.o\
	Testing/TestGame.o\
	Testing/PerformanceTests.o\
	Time/Duration.o\
	Time/DateTime.o\
	Time/GameTime.o



LeEK : $(OBJS)
	g++ -o $@ $^  $(PRELIBS)   $(LDFLAGS)  $(LIBSA) 

clean :
	rm -rf *.o *~ $(PROJ) $(OBJS)

