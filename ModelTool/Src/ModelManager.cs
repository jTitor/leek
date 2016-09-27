using ModelTool.UI;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

using ModelTool.Model;
using ModelTool.Statics;
using ModelTool.Log;
using LogVerbosity = ModelTool.Log.LogElement.LogVerbosity;
using System.Runtime.InteropServices;
using System.Windows.Threading;

namespace ModelTool.Src
{
	public class ModelManager : PropertyNotifier
	{
		#region Enum Definitions
		public enum OpenFSElemResult
		{
			FileOpened,
			DirectoryChanged,
			NoAction,
			Failure
		};
		#endregion

		private static uint currVer = 200;

		//Are the address bar and directory using normal colors?
		bool usingNormalBgs;

		//filesystem fields
        private DirectoryInfo cwd;
		private Progress<float> fileProgress;

		private string[] filters;
		private string openedFileName;
		private bool isModelLoaded;

		private List<LogElement> logElems;

		private ModelInfo modelData;

		#region Properties
		public ModelInfo CurrentModel
		{
			get { return modelData; }
			set
			{
				if (value != null)
				{
					modelData = value;
				}
			}
		}

		public Progress<float> FileProgressWatcher
		{
			get { return fileProgress; }
			set 
			{ 
				fileProgress = value;
				NotifyPropertyChanged();
			}
		}

		public bool IsModelLoaded
		{
			get { return isModelLoaded; }
		}

		public IEnumerable<LogElement> LogElements
		{
			get { return logElems; }
		}

		public DirectoryInfo CWD
		{
			get { return cwd; }
		}
		#endregion

		private void updateLog()
		{
			uint numNewLogs = NativeFunctions.GetLogBufferLen();
			//IntPtr temp = Marshal.AllocCoTaskMem(2048);
			for (uint i = 0; i < numNewLogs; ++i)
			{
				logElems.Add(new LogElement(NativeFunctions.GetLogMsg(i), (LogVerbosity)NativeFunctions.GetLogMsgVerbosity(i)));
			}
			//Marshal.FreeCoTaskMem(temp);
			NativeFunctions.ClearLogBuffer();
			//LogList.View.
			NotifyPropertyChanged("LogElements");
		}

		private void buildModelInfo(int mdlIdx, uint verNum)
		{
			//clear the current model if it exists!
			if (modelData.NumMeshes > 0)
			{
				modelData = new ModelInfo();
			}

			//use the marshaller to copy over the float vals
			int colorSize = 4*sizeof(float);
			int vec3Size = 3*sizeof(float);
			IntPtr tempBuf = Marshal.AllocHGlobal(Math.Max(colorSize, vec3Size));
			NativeFunctions.GetModelCenter(mdlIdx, tempBuf);
			Vector3 bndCenter = Vector3.FromNativeArray(tempBuf);
			NativeFunctions.GetModelAABBBounds(mdlIdx, tempBuf);
			Vector3 aabbBnd = Vector3.FromNativeArray(tempBuf);

			modelData.FileVersion	= verNum;
			modelData.BoundsCenter	= bndCenter;
			modelData.AABBBounds	= aabbBnd;
			modelData.BoundsRadius	= NativeFunctions.GetModelRadius(mdlIdx);
			
			//now add meshes
			for(int i = 0; i < NativeFunctions.GetMeshCount(mdlIdx); ++i)
			{
				MeshInfo inf = new MeshInfo();
				inf.NumVerts		= (uint)NativeFunctions.GetMeshVertexCount(mdlIdx, i);
				inf.NumInds			= (uint)NativeFunctions.GetMeshIndexCount(mdlIdx, i);
				inf.Diffuse			= LColor.FromNativeArray(NativeFunctions.GetMeshMaterialDiffuseColor(mdlIdx, i));
				inf.Specular		= LColor.FromNativeArray(NativeFunctions.GetMeshMaterialSpecularColor(mdlIdx, i));
				inf.Emissive		= LColor.FromNativeArray(NativeFunctions.GetMeshMaterialEmissiveColor(mdlIdx, i));
				inf.DiffuseTexGUID	= NativeFunctions.GetMeshMaterialDiffuseMapGUID(mdlIdx, i);
				inf.SpecularTexGUID	= NativeFunctions.GetMeshMaterialSpecularMapGUID(mdlIdx, i);
				inf.NormalTexGUID	= NativeFunctions.GetMeshMaterialNormalMapGUID(mdlIdx, i);
				inf.GlowTexGUID		= NativeFunctions.GetMeshMaterialEmissiveMapGUID(mdlIdx, i);

				modelData.AddMesh(inf);
			}

			NativeFunctions.ListModelData(mdlIdx);
			updateLog();
		}		

		public ModelManager()
		{
			/*
			//init UI fields
			usingNormalBgs = true;
			LogList.Background = ThemeBrushes.LogBg;
			logElems = new List<Tuple<string, LogVerbosity>>();
			LogList.ItemsSource = logElems;
			
			selectorElems = new List<int>();
			MeshSelector.ItemsSource = selectorElems;
			//ExplorerElement.IconSize = 24;

			//init thread fields
			uiDispatcher = Dispatcher.CurrentDispatcher;
			timer = new System.Timers.Timer();
			timer.Interval = uiUpdateDelay;
			timer.Elapsed += onUpdateTimerElapsed;
			timer.Start();
			*/
			
			//fileProgress = new Progress<int>();
			//fileProgress.ProgressChanged += fileProgress_ProgressChanged;
			isModelLoaded = false;
			logElems = new List<LogElement>();
			modelData = new ModelInfo();
			//init the filesystem fields
			cwd = new DirectoryInfo(Directory.GetCurrentDirectory());
			//initWatcher();

			openedFileName = "";

			filters = new string[] { FileFilters.ShowAll };

			//AddressBar.Text = cwd.FullName;
			//refreshWindow();
		}

		/**
		 * true iff directory successfully changed
		 */
        public bool ChangeCWD(string newPath)
        {
			//reset if the address is obviously invalid
			if (!FileSystemOps.PathIsValid(newPath))
			{
				return false;
			}
			var newCwd = new DirectoryInfo(newPath);
			if (!newCwd.Exists)
			{
				return false;
			}
			//otherwise go to where the address bar is pointing
            cwd = new DirectoryInfo(newPath);
			return true;
        }
		/**
		 * true iff cwd is now the cwd's parent folder
		 */
		public bool CWDMoveUp()
		{
			if (cwd.Parent == null)
			{
				return false;
			}

			cwd = cwd.Parent;
			return true;
		}
		/*
		private void readMeshFields(int meshIdx)
		{
			if (meshIdx < 0)
			{
				return;
			}
			DiffuseColorBox.SetValues(LColor.FromNativeArray(NativeFunctions.GetMeshMaterialDiffuseColor(1, meshIdx)));
			SpecularColorBox.SetValues(LColor.FromNativeArray(NativeFunctions.GetMeshMaterialSpecularColor(1, meshIdx)));
			EmissiveColorBox.SetValues(LColor.FromNativeArray(NativeFunctions.GetMeshMaterialEmissiveColor(1, meshIdx)));

			DiffuseTexGUIDBox.Text = NativeFunctions.GetMeshMaterialDiffuseMapGUID(1, meshIdx);
			SpecularTexGUIDBox.Text = NativeFunctions.GetMeshMaterialSpecularMapGUID(1, meshIdx);
			NormalMapGUIDBox.Text = NativeFunctions.GetMeshMaterialDiffuseMapGUID(1, meshIdx);
			GlowMapGUIDBox.Text = NativeFunctions.GetMeshMaterialEmissiveMapGUID(1, meshIdx);
		}

		private void populateMeshSubsec(int numMeshes)
		{
			selectorElems.Clear();
			for (int i = 0; i < numMeshes; ++i)
			{
				selectorElems.Add(i);
			}
			MeshSelector.SelectedIndex = 0;
			readMeshFields(MeshSelector.SelectedIndex);
		}

		private void populateEditorFields()
		{
			if (NativeFunctions.GetNumModelsLoaded() > 0)
			{
				Vector3 aabbBnd = Vector3.FromNativeArray(NativeFunctions.GetModelAABBBounds(1));
				float bndRad = NativeFunctions.GetModelRadius(1);
				int numMeshes = NativeFunctions.GetMeshCount(1);

				AABBBoundsBox.SetValues(aabbBnd);
				BoundingRadiusBox.Text = bndRad.ToString();

				populateMeshSubsec(numMeshes);
			}
		}
		*/

		/**
		 * Return value is an enum; its values are self explanatory
		 */
		public async Task<OpenFSElemResult> OpenFSElement(FileSystemInfo elem)
		{
			if (elem != null)
			{
				if (FileSystemOps.IsDirectory(elem))
				{
					//move into the directory
					cwd = new DirectoryInfo(elem.FullName);
					//refreshWindow();
					updateLog();
					return OpenFSElemResult.DirectoryChanged;
				}
				//else it must be some kind of file.
				//may need to reset the loaded model,
				//or (in the future) ask the user
				//if they want to export the current model.
				//check file:
				//is it a .lmdl?
				openedFileName = elem.Name;
				if (elem.Extension.Equals(FileFilters.EngineModel))
				{
					//...have to quit, since there's no exposed function for
					//LOADING a .lmdl.
					updateLog();
					isModelLoaded = true;
					return OpenFSElemResult.FileOpened;
				}
				else
				{
					//try importing it?
					int modelIdx = await ImporterOps.ImportModelAsync(elem.FullName, fileProgress);
					if (modelIdx < 0)
					{
						//import failed!
						openedFileName = "";
						updateLog();
						return OpenFSElemResult.Failure;
					}
					buildModelInfo(modelIdx, currVer);
					updateLog();
					isModelLoaded = true;
					return OpenFSElemResult.FileOpened;
				}
			}
			return OpenFSElemResult.NoAction;
		}

		/**
		 * true iff model successfully written
		 */
		public bool SaveModel(string path)
		{
			if(!FileSystemOps.PathIsValid(path))
			{
				return false;
			}
			ImporterOps.ExportModel(path, 1);
			updateLog();
			return true;
		}

		public void CloseModel()
		{
			isModelLoaded = false;
		}

		public void ClearLog()
		{
			logElems.Clear();
		}
	}
}
