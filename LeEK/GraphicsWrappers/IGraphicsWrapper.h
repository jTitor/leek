#pragma once
#include "Datatypes.h"
#include "Rendering/Geometry.h"
#include "Rendering/Texture.h"
#include "Math/Quaternion.h"
#include "Rendering/Shader.h"
#include "Rendering/Color.h"
//#include "EngineLogic/IEngineModule.h"
#include "Memory/Handle.h"
#include "Rendering/Text.h"
#include "Rendering/Model.h"
#include "Rendering/Bounds/AABBBounds.h"
#include "Rendering/Bounds/SphereBounds.h"
#include "Rendering/Bounds/Frustum.h"
#include "Math/Plane.h"
#include "FileManagement/path.h"

/**
* \file	IGraphicsWrapper.h
*		Header for IGraphicsWrapper interface.
*		This class now has implementation, so it should be refactored as an
*		abstract class.
*/

namespace LeEK
{
	//forward decs
	class IGraphicsWrapper;
	class CameraBase;
	class IPlatform;

	typedef TypedHandle<IGraphicsWrapper> GfxWrapperHandle;
	/**
	* Enumerates the wrapper's implementation.
	*/
	enum RendererType { OPEN_GL, DIRECTX, INVALID };

	/**
	* Enumerates the position of an attribute, if the wrapper is using OpenGL.
	*/
	enum RendererAttribute { POSITION, COLOR, NORMAL, UV0, UV1, UV2, UV3, ENUM_COUNT };

	/**
	* Enumerates the type of shader program contained in a shader file.
	*/
	enum ShaderType { VERTEX, FRAGMENT, GEOMETRY, TESS_CONTROL, TESS_EVAL };

	typedef std::pair< ShaderType,Path > ShaderFilePair;

	/**
	* Basic subsystem to handle drawing graphics to the client's drawing surface.
	* Uses shaders to render graphics; the shader language used depends on the wrapper's RenderType.
	*/
	class IGraphicsWrapper// : public IEngineModule
	{	
	public:
		
		virtual ~IGraphicsWrapper(void) {}
		/**
		* Gets the wrapper's implementation type.
		*/
		virtual const RendererType Type() const = 0;
		virtual F32 ScreenAspect() const = 0;
		virtual const Vector2& GetScreenResolution() const = 0;
		virtual void PrintShaderStatus() {}
		/**
		* Opens any conections to rendering libraries and initializes any internal resources.
		*/
		virtual bool Startup() = 0;
		/**
		* Builds wrapper-specific data for the specified mesh.
		* @param mesh the mesh to be processed by the wrapper.
		*/
		virtual bool InitGeometry(Geometry& mesh) = 0;
		/**
		* Builds wrapper-specific data for the specified text element.
		* @param text the text to be processed by the wrapper.
		*/
		virtual bool InitText(Text& text) = 0;
		/**
		* Builds wrapper-specific data for the texture.
		* @param tex the texture to be processed by the wrapper.
		*/
		virtual bool InitTexture(Texture2D& tex, TextureMeta::MapType type) = 0;
		/**
		* Closes any conections to rendering libraries and deletes any internal resources.
		*/
		virtual void Shutdown() = 0;
		/**
		* Deletes any data in the wrapper pertaining to a specified mesh.
		* @param mesh the mesh to be removed from the wrapper.
		*/
		virtual void ShutdownMesh(const Geometry& mesh) = 0;
		/**
		* Deletes any data in the wrapper pertaining to a specified text element.
		* @param text the text to be removed from the wrapper.
		*/
		virtual void ShutdownText(const Text& text) = 0;
		/**
		* Deletes any data in the wrapper pertaining to a specified texture.
		* @param tex the tex to be removed from the wrapper.
		*/
		virtual void ShutdownTexture(Texture2D& tex) = 0;
		/**
		* Sets all pixels in the drawing surface to the supplied color.
		* @ param r the red component of the color, in the range [0.0f-1.0f].
		* @ param g the green component of the color, in the range [0.0f-1.0f].
		* @ param b the blue component of the color, in the range [0.0f-1.0f].
		* @ param a the alpha component of the color, in the range [0.0f-1.0f].
		*/
		virtual void Clear(Color c) = 0;
		virtual void Clear() = 0;

		/**
		* Draws the specified mesh to the drawing surface.
		* @param mesh the mesh to be drawn. Must have been initialized via InitGeometry().
		*/
		virtual void Draw(const Geometry& mesh) = 0;
		/**
		* Draws the specified text element to the drawing surface.
		* @param text the text to be drawn. Must have been initialized via InitText().
		*/
		virtual void Draw(Text& text) = 0;

		/**
		* Draws a line independent of the current shading program.
		*/
		virtual void DebugDrawLine(const Vector3& start, const Vector3& end, const Color& color) = 0;
		virtual void DebugDrawPlane(const Vector3& origin, const Vector3& normal, const Color& color) = 0;
		void DebugDrawPlane(const Plane& plane, const Color& color)
		{
			DebugDrawPlane(plane.GetOrigin(), plane.GetNormal(), color);
		}
		/**
		* Draws a wireframe box independent of the current shading program.
		*/
		virtual void DebugDrawBox(const Vector3& center, Quaternion rotation, F32 halfHeight, F32 halfWidth, F32 halfDepth, const Color& color) = 0;
		void DebugDrawBox(const Vector3& center, Quaternion rotation, const Vector3& halfDims, const Color& color)
		{
			DebugDrawBox(center, rotation, halfDims.X(), halfDims.Y(), halfDims.Z(), color);
		}

		void DebugDrawAABB(const Vector3& center, F32 halfHeight, F32 halfWidth, F32 halfDepth, const Color& color)
		{
			DebugDrawBox(center, Quaternion::Identity, halfHeight, halfWidth, halfDepth, color);
		}
		void DebugDrawAABB(const Vector3& center, F32 halfExtent, const Color& color)
		{
			DebugDrawBox(center, Quaternion::Identity, halfExtent, halfExtent, halfExtent, color);
		}
		void DebugDrawAABB(const Vector3& center, const Vector3& halfDims, const Color& color)
		{
			DebugDrawBox(center, Quaternion::Identity, halfDims, color);
		}
		void DebugDrawAABB(const AABBBounds& aabb, const Color& color)
		{
			DebugDrawAABB(aabb.Center(), aabb.GetHalfDimensions(), color);
		}
		/**
		* Draws a wireframe sphere independent of the current shading program.
		*/
		virtual void DebugDrawSphere(const Vector3& center, F32 radius, const Color& color) = 0;
		void DebugDrawSphere(const SphereBounds& sphere, const Color& color)
		{
			DebugDrawSphere(sphere.Center(), sphere.Radius(), color);
		}
		//virtual void DebugDrawCone(const Vector3& base, const Vector3& point, F32 baseRadius) = 0;
		//virtual void DebugDrawCube() = 0;
		//virtual void DebugDrawSet()?
		/**
		* Draws a wireframe frustum independent of the current shading program.
		* @param invProjMat an inverted projection matrix.
		*/
		virtual void DebugDrawFrustum(const Matrix4x4& invProjMat, const Matrix4x4& viewMat, const Color& color) = 0;
		void DebugDrawFrustum(const Matrix4x4& invProjMat, const Color& color)
		{
			DebugDrawFrustum(invProjMat, Matrix4x4::Identity, color);
		}
		virtual void DebugDrawFrustum(const CameraBase* cam, const Color& color) = 0;
		virtual void DebugDrawFrustum(const Frustum& frust, const Color& color) = 0;


		virtual void DebugDrawMesh(const Geometry& meshGeom, const Vector3& center, Quaternion rotation, const Color& color) = 0;
		virtual void DebugDrawModel(const Model& model, const Vector3& center, Quaternion rotation, const Color& color) = 0;

		/**
		* Generates a shader program from the specified shader files.
		* At least 1 fragment shader and 1 vertex shader must be passed, or no shader will be built.
		* @param shaderName the name of the shader program.
		* @param shaderFileCount the number of shader file pairs.
		* @param ... the shader file pairs. Each pair consists of a ShaderType, followed by a Path to the shader file.
		* @return true if the shader was successfully built, false otherwise.
		*/
		virtual TypedHandle<Shader> MakeShader(String shaderName, U32 shaderFileCount, Vector< ShaderFilePair > FilePaths) = 0;

		/**
		* Sets the specified shader as the wrapper's current shader program, if it exists.
		* @param shaderName the name of the shader program.
		* @return true if the shader was successfully set as the current program, false otherwise.
		*/
		virtual bool SetShader(String shaderName) = 0;
		virtual bool SetShader(TypedHandle<Shader> shader) = 0;

		//Uniform setters.
		virtual bool SetMatrixUniform(String name, const Matrix4x4& value) = 0;
		virtual bool SetVec3Uniform(String name, const Vector3& value) = 0;
		virtual bool SetVec4Uniform(String name, const Vector4& value) = 0;
		virtual bool SetIntUniform(String name, const U32& value) = 0;
		virtual bool SetFloatUniform(String name, const F32& value) = 0;
		/**
		* Sets the specified texture as the wrapper's current texture of its type, if it exists.
		* @param tex the texture to be assigned.
		* @param type the type to assign the texture to.
		* @return true if the texture was successfully assigned to the given type, false otherwise.
		*/
		virtual bool SetTexture(const Texture2D& tex, TextureMeta::MapType type) = 0;
		virtual bool SetTexture(U32 texHandle, TextureMeta::MapType type) = 0;

		virtual bool SetWorld(const Matrix4x4& world) = 0;
		virtual bool SetView(const Matrix4x4& view) = 0;
		virtual bool SetProjection(const Matrix4x4& projection) = 0;
		bool SetWorldViewProjection(const Matrix4x4& world, const Matrix4x4& view, const Matrix4x4& projection) { return SetWorld(world) && SetView(view) && SetProjection(projection); }

		//Special render-to-texture systems.

		/**
		* Generates a blank texture that can be written to.
		* @param width the desired width in pixels. Will be raised to the nearest power of two if it is not itself a power of two.
		* @param height the desired height in pixels. Will be raised to the nearest power of two if it is not itself a power of two.
		* @param pixType the pixel type of the texture to be generated.
		* @param compType the compression type of the texture to be generated.
		* @return a valid Texture2D object with a texture buffer handle if the texture could be created, or a Texture2D object with all fields set to null otherwise.
		*/
		virtual Texture2D GenerateBlankTexture(	U32 width, U32 height, 
												Texture2D::PixelType pixType = Texture2D::RGBA8,
												Texture2D::CompressionType compType = Texture2D::NONE) = 0;

		/**
		* Overwrites the specified area of a texture with the given data.
		* @param tex the texture object to be modified.
		* @param x the left coordinate of the area to be overwritten.
		* @param y the lower coordinate of the area to be overwritten.
		* @param width the width of the area to be overwritten.
		* @param height the height of the area to be overwritten.
		* @param data the data to overwrite the texture with.
		* @return true if the texture was properly overwritten, false otherwise. If false, the texture should be unmodified.
		*/
		virtual bool FillTextureSection(const Texture2D& tex, U32 x, U32 y, U32 width, U32 height, char* data) = 0;

		/**
		* Overwrites the entire texture with the given data.
		* @param tex the texture object to be modified.
		* @param data the data to overwrite the texture with.
		* @return true if the texture was properly overwritten, false otherwise. If false, the texture should be unmodified.
		*/
		bool FillTexture(const Texture2D& tex, char* data) { return FillTextureSection(tex, 0, 0, tex.Width, tex.Height, data); }
	};
}