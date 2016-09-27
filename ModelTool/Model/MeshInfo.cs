using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ModelTool.Src;

namespace ModelTool.Model
{
	public class MeshInfo : PropertyNotifier
	{
		static string noGUID = "(none)";
		private LColor diff, spec, emit;
		private string diffTex, specTex, normTex, emitTex;
		private uint numVerts, numInds;

		public MeshInfo()
		{
			diff = new LColor();
			spec = new LColor();
			emit = new LColor();

			diffTex = "";
			specTex = "";
			normTex = "";
			emitTex = "";

			numVerts = 0;
			numInds = 0;
		}

		public LColor Diffuse
		{
			get { return diff; }
			set
			{
				diff = value;
				NotifyPropertyChanged();
			}
		}

		public LColor Specular
		{
			get { return spec; }
			set
			{
				spec = value;
				NotifyPropertyChanged();
			}
		}

		public LColor Emissive
		{
			get { return emit; }
			set
			{
				emit = value;
				NotifyPropertyChanged();
			}
		}

		public string DiffuseTexGUID
		{
			get { return diffTex == "" ? noGUID : diffTex; }
			set
			{
				diffTex = value;
				NotifyPropertyChanged();
			}
		}

		public string SpecularTexGUID
		{
			get { return specTex == "" ? noGUID : specTex; }
			set
			{
				specTex = value;
				NotifyPropertyChanged();
			}
		}

		public string NormalTexGUID
		{
			get { return normTex == "" ? noGUID : normTex; }
			set
			{
				normTex = value;
				NotifyPropertyChanged();
			}
		}

		public string GlowTexGUID
		{
			get { return emitTex == "" ? noGUID : emitTex; }
			set
			{
				emitTex = value;
				NotifyPropertyChanged();
			}
		}

		public uint NumVerts
		{
			get { return numVerts; }
			set
			{
				numVerts = value;
				NotifyPropertyChanged();
			}
		}

		public uint NumInds
		{
			get { return numInds; }
			set
			{
				numInds = value;
				NotifyPropertyChanged();
			}
		}
	}
}
