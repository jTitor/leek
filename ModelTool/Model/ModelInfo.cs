using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ModelTool.Src;

namespace ModelTool.Model
{
	public class ModelInfo : PropertyNotifier
	{
		uint fileVer;
		//uint numMeshes;
		uint numUVChannels;
		uint totalVerts;
		uint totalInds;

		Vector3 aabbBnd;
		Vector3 bndCenter;
		float bndRadius;

		private List<MeshInfo> meshes;

		public ModelInfo()
		{
			fileVer = 0;
			//numMeshes = 0;
			numUVChannels = 0;
			aabbBnd = new Vector3();
			bndCenter = new Vector3();
			bndRadius = 0;
			meshes = new List<MeshInfo>();
			totalVerts = 0;
			totalInds = 0;
		}

		public uint FileVersion
		{
			get { return fileVer; }
			set
			{
				fileVer = value;
				NotifyPropertyChanged();
			}
		}

		public int NumMeshes
		{
			get { return meshes.Count; }
		}

		public uint NumUVChannels
		{
			get { return numUVChannels; }
			set
			{
				numUVChannels = value;
				NotifyPropertyChanged();
			}
		}

		public float BoundsRadius
		{
			get { return bndRadius; }
			set
			{
				bndRadius = value;
				NotifyPropertyChanged();
			}
		}

		public Vector3 AABBBounds
		{
			get { return aabbBnd; }
			set
			{
				aabbBnd = value;
				NotifyPropertyChanged();
			}
		}

		public Vector3 BoundsCenter
		{
			get { return bndCenter; }
			set
			{
				bndCenter = value;
				NotifyPropertyChanged();
			}
		}

		public uint TotalVerts
		{
			get { return totalVerts; }
		}

		public uint TotalInds
		{
			get { return totalInds; }
		}

		public MeshInfo GetMesh(int idx)
		{
			if (idx < 0 || idx >= NumMeshes)
			{
				return default(MeshInfo);
			}
			return meshes[idx];
		}

		public void AddMesh(MeshInfo m)
		{
			if (m == null)
			{
				return;
			}
			meshes.Add(m);
			totalVerts += m.NumVerts;
			totalInds += m.NumInds;
		}

		public void RemoveMesh(int idx)
		{
			if (idx < 0 || idx <= NumMeshes)
			{
				return;
			}
			MeshInfo m = meshes[idx];
			totalVerts -= m.NumVerts;
			totalInds -= m.NumInds;
			meshes.RemoveAt(idx);
		}
	}
}
