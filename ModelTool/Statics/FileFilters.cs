using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ModelTool.Statics
{
	//Contains various file extensions that can filter a list of FileSystemInfo.
    static class FileFilters
    {
		public const string ShowAll = ".*";

		//Model formats
        public const string EngineModel = ".lmdl";
		public const string FBX = ".fbx";
		public const string Collada = ".dae";
		public const string Blender = ".blend";
		public const string ThreeDSMax = ".3ds";
		public const string ThreeDSMaxASE = ".ase";
		public const string OBJ = ".obj";
		public const string IFCStep = ".ifc";
		public const string XGL = ".xgl";
		public const string ZGL = ".zgl";
		public const string Stanford = ".ply";
		public const string AutoCADDXF = ".dxf";
		public const string LightWave = ".lwo";
		public const string LightWaveScene = ".lws";
		public const string Modo = ".lxo";
		public const string StereoLithography = ".stl";
		public const string DirectX = ".x";
		public const string AC3D = ".ac";
		public const string MilkShape3D = ".ms3d";

		private static string[] assimpCompatible = null;
		public static string[] AssimpCompatible
		{
			get
			{
				if (assimpCompatible == null)
				{
					assimpCompatible = new string[] {	Collada,
														Blender,
														ThreeDSMax,
														ThreeDSMaxASE,
														OBJ,
														IFCStep,
														XGL,
														ZGL,
														Stanford,
														AutoCADDXF,
														LightWave,
														LightWaveScene,
														Modo,
														StereoLithography,
														DirectX,
														AC3D,
														MilkShape3D
													 };
				}
				return assimpCompatible;
			}
		}
    }
}
