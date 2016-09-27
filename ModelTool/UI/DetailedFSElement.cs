using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Media;

using ModelTool.Statics;

namespace ModelTool.UI
{
	public class DetailedFSElement
	{
		private bool isDirectory, isImportable, isEngineModelFile;
		private FileSystemInfo fsInf;
		private ImageSource fsIconSource;
		private static int iconSize = 16;

		public DetailedFSElement(FileSystemInfo pFSInf)
		{
			fsInf = pFSInf;
			isDirectory = FileSystemOps.IsDirectory(fsInf);
			foreach (string filter in FileFilters.AssimpCompatible)
			{
				if (fsInf.Extension == filter)
				{
					isImportable = true;
					break;
				}
			}
			if (fsInf.Extension == FileFilters.EngineModel)
			{
				isEngineModelFile = true;
			}

			//try to load an icon if possible!
			fsIconSource = FileSystemOps.GetImageFromIcon(fsInf.FullName, iconSize);
			//TODO: set a default
		}

		public bool IsDirectory
		{
			get { return isDirectory; }
		}

		public bool IsImportable
		{
			get { return isImportable; }
		}

		public bool IsEngineModelFile
		{
			get { return isEngineModelFile; }
		}

		public FileSystemInfo Info
		{
			get { return fsInf; }
		}

		public ImageSource IconSource
		{
			get { return fsIconSource; }
		}

        public String Name
        {
            get { return fsInf.Name; }
        }
	}
}
