#pragma once
#include "GraphicsWrappers/IGraphicsWrapper.h"
#include "Strings/String.h"
#include "Math/Matrix4x4.h"
#include "FileManagement/Filesystem.h"
#include "Hashing/HashMap.h"
#include "Rendering/Shader.h"
#include <vector>

namespace LeEK
{
	/**
	* Dummy implementation of the IGraphicsWrapper class.
	*/
	class NullGrpWrapper :	public IGraphicsWrapper
	{
	private:
	public:
		NullGrpWrapper(void) {}
		~NullGrpWrapper(void) {}

		#pragma region Interface Implementation
		inline const RendererType Type() const { return INVALID; }
		F32 ScreenAspect()  const { return -1.0f; }
		const Vector2& GetScreenResolution() const { return Vector2::Zero; }

		void PrintConsole() {}
		//must be able to start up, but nothing is actually functional
		bool Startup() { return true; }
		bool InitGeometry(Geometry& mesh) { return false; }
		bool InitText(Text& text) { return false; }
		bool InitTexture(Texture2D& tex, TextureMeta::MapType type) { return false; }
		void Shutdown() {}
		void ShutdownMesh(const Geometry& mesh) {}
		void ShutdownText(const Text& text) {}
		void ShutdownTexture(Texture2D& tex) {}
		void Clear(Color c) {}
		inline void Clear() {}
		void Draw(const Geometry& mesh) {}
		void Draw(Text& text) {}
		#pragma region Debug Drawing Commands
		void DebugDrawLine(const Vector3& start, const Vector3& end, const Color& color) {}
		void DebugDrawPlane(const Vector3& origin, const Vector3& normal, const Color& color) {}
		void DebugDrawBox(const Vector3& center, Quaternion rotation, F32 halfHeight, F32 halfWidth, F32 halfDepth, const Color& color) {}
		void DebugDrawSphere(const Vector3& center, F32 radius, const Color& color) {}
		void DebugDrawFrustum(const Matrix4x4& invProjMat, const Matrix4x4& viewMat, const Color& color) {}
		void DebugDrawFrustum(const CameraBase* cam, const Color& color) {}
		void DebugDrawFrustum(const Frustum& frust, const Color& color) {}
		void DebugDrawMesh(const Geometry& meshGeom, const Vector3& center, Quaternion rotation, const Color& color) {}
		void DebugDrawModel(const Model& model, const Vector3& center, Quaternion rotation, const Color& color) {}
		#pragma endregion
		TypedHandle<Shader> MakeShader(String shaderName, U32 shaderFileCount, Vector< std::pair< ShaderType,Path > > FilePaths) { return 0;}
		bool SetShader(String shaderName) { return false; }
		bool SetShader(TypedHandle<Shader> shader) { return false; }
		#pragma endregion

		bool LoadFunctions() { return true; }
		bool LoadShader(GLenum type, Path filePath) { return false; }
		//varargs should consist first of the number of attributes, followed pairs of attribute positions and attribute names in char* form
		bool CreateShaderProgram(String progName, U32 attribCount, ...) { return false; }
		bool CreateShaderProgram(String progName, U32 attribCount, va_list attribs) { return false; }
		void ShutdownShaderProgram() {}
		//helper function
		//varargs should consist first of the number of attributes, followed pairs of attribute positions and attribute names in char* form
		bool ProgramFromShaderPair(String progName, Path vertexShaderSource, Path fragShaderSource, U32 attribCount, ...);
		void PrintShaderStatus() {}

		bool SetMatrixUniform(String name, const Matrix4x4& value) { return false; }
		bool SetVec3Uniform(String name, const Vector3& value) { return false; }
		bool SetVec4Uniform(String name, const Vector4& value) { return false; }
		bool SetIntUniform(String name, const U32& value) { return false; }
		bool SetFloatUniform(String name, const F32& value) { return false; }
		bool SetTexture(const Texture2D& tex, TextureMeta::MapType type) { return false; }
		bool SetTexture(U32 texHandle, TextureMeta::MapType type) { return false; }

		bool SetWorld(const Matrix4x4& world) { return false; }
		bool SetView(const Matrix4x4& view) { return false; }
		bool SetProjection(const Matrix4x4& projection) { return false; }

		Texture2D GenerateBlankTexture(	U32 width, U32 height, 
												Texture2D::PixelType pixType,
												Texture2D::CompressionType compType) { return Texture2D(); }
		bool FillTextureSection(const Texture2D& tex, U32 x, U32 y, U32 width, U32 height, char* data) { return false; }
	};
}
