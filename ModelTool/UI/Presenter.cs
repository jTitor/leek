using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Collections.ObjectModel;
using System.Windows.Input;

using ModelTool.Src;
using ModelTool.Log;
using ModelTool.Model;
using System.IO;
using ModelTool.Statics;
using System.Windows;
using System.Windows.Threading;
using System.Windows.Forms;
using System.Windows.Data;
using System.ComponentModel;

namespace ModelTool.UI
{
	/**
	 * Allows automatically updated access and bindings to the application's
	 * internal model framework.
	 */
	public class Presenter : PropertyNotifier
	{
		#region UI Properties
		private static double generalPadding = 4;
		private static double subPaddingFactor = 0.5;
		private static double splitterPaddingFactor = 4;
		private static double listViewPaddingReduceFactor = -0.5;

		public double GeneralPadding
		{
			get { return generalPadding; }
		}

		public double SubPadding
		{
			get { return generalPadding * subPaddingFactor; }
		}

		public double SplitterPadding
		{
			get { return generalPadding * splitterPaddingFactor; }
		}

		public Thickness LogColumnPadding
		{
			get { return new Thickness(0, 0, SubPadding, 0); }
		}

		public Thickness ListMarginReduction
		{
			get { return new Thickness(0, listViewPaddingReduceFactor * generalPadding, 0, 0); }
		}

		public double MinTextBoxLen
		{
			get { return 96; }
		}
		#endregion

		//update rate, in milliseconds.
		int uiUpdateDelay = 250;
		Dispatcher uiDispatcher;

		DispatcherTimer progressUpdateTimer;

		private readonly ModelManager modelMgr;

		private static int iconSize = 16;

		private static string noGUID = "(none)";

		int currentMesh;
		private ObservableCollection<int> meshSelectorElements;
        private RangeObservableCollection<DetailedFSElement> dirListing;
		FileSystemWatcher cwdWatcher;

		private Progress<float> fileProgress;
		private float fileProgressVal;
		private string addressBarText;
		private bool explorerActive;
		private DetailedFSElement selectedFSElem;
		private string currMdlPath;
		private bool showOnlyImportable;

		private LogElement.LogVerbosity verbLevel;
		private ICollectionView logView;
		private ICollectionView dirListView;
		private ICollectionView importableFileView;

		public enum EditorFileState
		{
			NothingLoaded,	//no file loaded
			Ready,
			Reading,
			ReadComplete,
			Writing,
			WriteComplete,
			Importing,		//separate from a read;
							//large files may take significant time
							//to convert to .lmdl
			ImportComplete,
			ChangingDir
		};
		EditorFileState currentFileState;

		public static event EventHandler CWDInvalid;
		public static event EventHandler ModelFileLoaded;
		public static event EventHandler MeshChanged;

		public Presenter()
		{
			uiDispatcher = Dispatcher.CurrentDispatcher;
			progressUpdateTimer = new DispatcherTimer();
			progressUpdateTimer.Interval = new TimeSpan(0, 0, 0, 0, uiUpdateDelay);
			progressUpdateTimer.Tick += progressUpdateTimer_Tick;
			explorerActive = true;
			fileProgress = new Progress<float>();
			fileProgress.ProgressChanged += fileProgress_ProgressChanged;
			modelMgr = new ModelManager();
			modelMgr.FileProgressWatcher = fileProgress;
			dirListing = new RangeObservableCollection<DetailedFSElement>();
			dirListView = CollectionViewSource.GetDefaultView(dirListing);
			dirListView.Filter = fileVisibleFilter;
			importableFileView = new ListCollectionView(dirListing);
			importableFileView.Filter = fileImportableFilter;

			meshSelectorElements = new ObservableCollection<int>();
			currentMesh = 0;
			currMdlPath = "";
			addressBarText = CWD.FullName;
			currentFileState = EditorFileState.NothingLoaded;
			showOnlyImportable = false;

			//activate fs watcher
			cwdWatcher = new FileSystemWatcher(CWD.FullName);
			cwdWatcher.Changed += cwdWatcherUpdateCWD;
			cwdWatcher.Deleted += cwdWatcherUpdateCWD;
			cwdWatcher.Created += cwdWatcherUpdateCWD;
			cwdWatcher.Renamed += cwdWatcherRenamed;

			//and start watching!
			cwdWatcher.EnableRaisingEvents = true;

			verbLevel = LogElement.LogVerbosity.Debug;
			//init log filter
			logView = CollectionViewSource.GetDefaultView(modelMgr.LogElements);
			logView.Filter = verbosityFilter;

			NotifyPropertyChanged(null);
			updateFSData();
		}

		#region Properties
		public string NoGUIDText
		{
			get { return noGUID; }
		}

		public bool ExplorerActive
		{
			get { return explorerActive; }
		}

		public string AddressBarText
		{
			get { return addressBarText; }
			set { 
					addressBarText = value;
					NotifyPropertyChanged("AddressBarText");
				}
		}

		public EditorFileState FileState
		{
			get { return currentFileState; }
		}

		public DetailedFSElement SelectedItem
		{
			get
			{
				return selectedFSElem;
			}
		}

		public DirectoryInfo CWD
		{
			get { return modelMgr.CWD; }
		}

		public ICollectionView LogElements
		{
			get { return logView; }
		}

		public IEnumerable<int> MeshSelectorValues
		{
			get { return meshSelectorElements; }
		}

		public ICollectionView DirListView
		{
			get { return dirListView; }
		}

		public IEnumerable<DetailedFSElement> DirListing
		{
			get { return dirListing; }
		}

		public bool ShowOnlyImportable
		{
			get { return showOnlyImportable; }
			set
			{
				showOnlyImportable = value;
				DirListView.Refresh();
				NotifyPropertyChanged();
				NotifyPropertyChanged("DirListView");
			}
		}

		public ModelInfo LoadedModel
		{
			get { return modelMgr.CurrentModel; }
			set
			{
				modelMgr.CurrentModel = value;
				NotifyPropertyChanged();
			}
		}

		public int SelectedMeshIndex
		{
			get { return currentMesh; }
			set
			{
				currentMesh = value;
				onMeshSelected();
				NotifyPropertyChanged("CurrentMesh");
				NotifyPropertyChanged();
			}
		}

		public MeshInfo CurrentMesh
		{
			get 
			{ 
				return LoadedModel == null ? 
					default(MeshInfo) : 
					LoadedModel.GetMesh(currentMesh);
			}
		}

		public int FileProgress
		{
			get
			{
				return (int)fileProgressVal;
			}
		}

		public LogElement.LogVerbosity VerbosityLevel
		{
			get { return verbLevel; }
			set
			{
				verbLevel = value;
				logView.Refresh();
				NotifyPropertyChanged();
				NotifyPropertyChanged("LogElements");
			}
		}
		#endregion

		private void setCurrentFileState(EditorFileState newState)
		{
			currentFileState = newState;
			NotifyPropertyChanged("FileState");
		}

		/**
		 */
		public void SetRendererTarget(IntPtr hwnd)
		{
			NativeFunctions.SetViewPort(hwnd);
		}

		#region Commands
		public ICommand CmdOpenFSElement
		{
			get
			{ 
				return new AsyncDelegateCommand(() => Task.Run(() => { openFSElement(); }));
			}
		}

		public ICommand CmdWriteModel
		{
			get { return new DelegateCommand(writeModel); }
		}

		public ICommand CmdConvertAll
		{
			get { return new DelegateCommand(convertAllInCWD); }
		}

		public ICommand CmdSelectFSElement
		{
			get { return new DelegateCommand<DetailedFSElement>(selectFSElement); }
		}

		public ICommand CmdSelectMesh
		{
			get { return new DelegateCommand<int>(selectMesh); }
		}

		public ICommand CmdGo
		{
			get { return new DelegateCommand(setCWD); }
		}

		public ICommand CmdUp
		{
			get { return new DelegateCommand(moveCwdUp); }
		}

		public ICommand CmdClearLog
		{
			get { return new DelegateCommand(clearLog); }
		}
		#endregion

		#region Command Callbacks
		private void resetAddressBar()
		{
			AddressBarText = modelMgr.CWD.FullName;
		}

		private void resetFileProgress()
		{
			fileProgressVal = 0;
			NotifyPropertyChanged("FileProgress");
		}

		private void setCWD(string path)
		{
			DirectoryInfo cwd = modelMgr.CWD;

			//attempt to change directory; if it failed, reset.
			if (!modelMgr.ChangeCWD(path))
			{
				resetAddressBar();
				return;
			}
			//if successful, check if the folder's invalid
			if (!cwd.Exists)
			{
				//let view know that folder's unavailable
				onCWDInvalid();
				//reset address bar text
				resetAddressBar();
				return;
			}

			//otherwise the directory changed.
			//refresh directory listing and notify.
			updateFSData();
		}

		private void setCWD()
		{
			setCWD(addressBarText);
		}

		private void moveCwdUp()
		{
			modelMgr.CWDMoveUp();
			updateFSData();
		}

		private void updateFSData()
		{
			//first, the address displayed
			resetAddressBar();
			dirListing.Clear();
			var newList = FileSystemOps.ListContents(CWD.FullName, true);
            var detailedList = new List<DetailedFSElement>();
			foreach(var elem in newList)
			{
				detailedList.Add(new DetailedFSElement(elem));
			}
            dirListing.AddRange(detailedList);
			//NotifyPropertyChanged("DirListing");
			//dirListView.Refresh();
			//NotifyPropertyChanged("DirListView");
			cwdWatcher.Path = CWD.FullName;
		}

		private void updateMeshSelector()
		{
			meshSelectorElements.Clear();
			for (int i = 0; i < LoadedModel.NumMeshes; ++i)
			{
				meshSelectorElements.Add(i);
			}
			if (meshSelectorElements.Count > 0)
			{
				SelectedMeshIndex = meshSelectorElements[0];
			}
			NotifyPropertyChanged("MeshSelectorValues");
		}

		//actual commands issued to the ModelManager
		private async void openFSElement()
		{
			var elem = selectedFSElem;
			if (elem == null)
			{
				return;
			}
			//this can do a LOT of different things;
			//the status message may thus vary depending
			//on what the selected element actually is.
			var fileOpenResult = modelMgr.OpenFSElement(elem.Info);
			//may need to swap the current file state
			var lastFileState = currentFileState;
			if(elem.IsDirectory)
			{
				setCurrentFileState(EditorFileState.ChangingDir);
			}
			else
			{
				setCurrentFileState(EditorFileState.Reading);
			}
			//progressUpdateTimer.Start();
			//let view know you can't mess with the directory view
			setExplorerActive(false);
			ModelManager.OpenFSElemResult res = await fileOpenResult;
			updateLog();
			resetFileProgress();
			if (elem.IsDirectory)
			{
				setCurrentFileState(lastFileState);
			}
			else
			{
				setCurrentFileState(EditorFileState.ReadComplete);
			}
			//progressUpdateTimer.Stop();
			//let view know the directory view is active
			setExplorerActive(true);
			//this all may be running on another thread;
			//ensure the UI knows what's happening
			uiDispatcher.Invoke(new Action(delegate
				{
					switch (res)
					{
						case ModelManager.OpenFSElemResult.FileOpened:
							//update the mesh selector
							updateMeshSelector();
							currMdlPath = elem.Info.FullName;
							//and note that a lot of the info's changed
							onModelFileLoaded();
							break;
						case ModelManager.OpenFSElemResult.DirectoryChanged:
							//CWD is different now, refresh it
							updateFSData();
							break;
						case ModelManager.OpenFSElemResult.Failure:
							//notify failure
							break;
						case ModelManager.OpenFSElemResult.NoAction:
						default:
							//do nothing!
							break;
					}
				}));
		}

		private void selectFSElement(DetailedFSElement elem)
		{
			selectedFSElem = elem;
			NotifyPropertyChanged("SelectedItem");
		}

		private void writeModel()
		{
			if (!modelMgr.IsModelLoaded)
			{
				return;
			}
			var elem = modelMgr.CurrentModel;
			if (elem == null)
			{
				return;
			}
			setExplorerActive(false);
			setCurrentFileState(EditorFileState.Writing);
			//change extension
			var finalPath = currMdlPath.Remove(currMdlPath.LastIndexOf('.'));
			finalPath += FileFilters.EngineModel;
			modelMgr.SaveModel(finalPath);
			NotifyPropertyChanged("LogElements");
			setCurrentFileState(EditorFileState.WriteComplete);
			setExplorerActive(true);
		}

		private async void convertAllInCWD()
		{
			//pause the watcher; a lot of files are going to be modified
			cwdWatcher.EnableRaisingEvents = false;
			importableFileView.Refresh();
			//these should all be importable files now.
			foreach (var file in importableFileView)
			{
				var castFS = (DetailedFSElement)file;
				uiDispatcher.Invoke(() => { selectFSElement(castFS); });
				currMdlPath = castFS.Info.FullName;
				setExplorerActive(false);
				await Task.Run(async () =>
				{
					var fileOpenResult = modelMgr.OpenFSElement(castFS.Info);
					setCurrentFileState(EditorFileState.Reading);
					ModelManager.OpenFSElemResult res = await fileOpenResult;
					updateLog();
				});
				writeModel();
				//clear logs?
			}
			resetFileProgress();
			setExplorerActive(true);
			setCurrentFileState(EditorFileState.Ready);
			//now re-enable the watcher
			cwdWatcher.EnableRaisingEvents = true;
			updateFSData();
		}

		private void selectMesh(int newMeshVal)
		{
			//sanitize value
			newMeshVal = MathHelpers.Clamp(newMeshVal, 0, LoadedModel.NumMeshes);
			currentMesh = newMeshVal;
			NotifyPropertyChanged("CurrentMesh");
			onMeshSelected();
		}

		private void updateLog()
		{
			uiDispatcher.BeginInvoke(new Action(delegate
			{
				logView.Refresh();
			}));
			NotifyPropertyChanged("LogElements");
		}

		private void setExplorerActive(bool value)
		{
			explorerActive = value;
			NotifyPropertyChanged("ExplorerActive");
		}

		private void clearLog()
		{
			modelMgr.ClearLog();
			updateLog();
		}
		#endregion

		#region Event Callers
		private void onCWDInvalid()
		{
			if (CWDInvalid != null)
			{
				CWDInvalid(this, EventArgs.Empty);
			}
		}

		private void onModelFileLoaded()
		{
			if (ModelFileLoaded != null)
			{
				ModelFileLoaded(this, EventArgs.Empty);
			}
			NotifyPropertyChanged("LoadedModel");
			NotifyPropertyChanged("LogElements");
			currentFileState = EditorFileState.ImportComplete;
			onMeshSelected();
		}

		private void onMeshSelected()
		{
			if (MeshChanged != null)
			{
				MeshChanged(this, EventArgs.Empty);
			}
		}
		#endregion

		#region Event Callbacks
		private void progressUpdateTimer_Tick(object sender, EventArgs e)
		{
			NotifyPropertyChanged("FileProgress");
		}

		private void fileProgress_ProgressChanged(object sender, float e)
		{
			fileProgressVal += e;
			NotifyPropertyChanged("FileProgress");
			if (currentFileState != EditorFileState.Importing)
			{
				setCurrentFileState(EditorFileState.Importing);
			}
		}

		private void cwdWatcherRenamed(object sender, RenamedEventArgs e)
		{
			//remember to move *us* if it was this directory that was renamed
			if (e.OldFullPath == CWD.FullName)
			{
				setCWD(e.FullPath);
			}
			uiDispatcher.Invoke(updateFSData);
		}

		private void cwdWatcherUpdateCWD(object sender, FileSystemEventArgs e)
		{
			uiDispatcher.Invoke(updateFSData);
		}
		#endregion

		#region Filters
		private bool verbosityFilter(object obj)
		{
			LogElement elem = (LogElement)obj;
			return elem.Verbosity <= verbLevel;
		}

		private bool fileVisibleFilter(object obj)
		{
			DetailedFSElement elem = (DetailedFSElement)obj;
			return elem.IsImportable || elem.IsDirectory || !showOnlyImportable;
		}

		private bool fileImportableFilter(object obj)
		{
			DetailedFSElement elem = (DetailedFSElement)obj;
			return elem.IsImportable;
		}
		#endregion
	}
}
