using System;
using System.Collections.Generic;
using System.Collections;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows.Media.Imaging;
using System.Windows;
using System.Windows.Media;

namespace ModelTool.Statics
{
	//encapsulates info retrieved from the shell.
	[StructLayout(LayoutKind.Sequential)]
	public struct SHFILEINFO
	{
		public IntPtr hIcon;
		public int iIcon;
		public uint dwAttributes;
		//The struct uses fixed-size strings; you can indicate this
		//by marshalling, and pass the length w/ SizeConst
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 260)]
		public string szDisplayName;
		[MarshalAs(UnmanagedType.ByValTStr, SizeConst = 80)]
		public string szTypeName;
	};

    static class FileSystemOps
    {
        /**
         * Lists all elements of a directory that fit
         * a given set of wildcard filters.
         */
        public static IEnumerable<FileSystemInfo> ListFilter(String dir, String[] filters, bool showSubDirs)
        {
            try
            {
                String[] uniqueFilters = filters;//(String[])filters.Distinct();
                /*
                 * can throw:
                    * ArgumentNullException	
                        path is null.
                    * SecurityException	
                        The caller does not have the required permission.
                    * ArgumentException	
                        path contains invalid characters such as ", <, >, or |.
                    * PathTooLongException
                 */
                DirectoryInfo dirInf = new DirectoryInfo(dir);
                var subDirs = dirInf.EnumerateDirectories();
                IEnumerable<FileSystemInfo> files = null;
                if (uniqueFilters[0].Equals("*.*"))
                {
                    files = dirInf.EnumerateFiles();
                }
                else
                {
                    files = dirInf.EnumerateFiles(uniqueFilters[0]);
                    foreach (String filter in uniqueFilters)
                    {
                        files.Concat(dirInf.EnumerateFiles(filter));
                    }
                }
                IList<FileSystemInfo> result = new List<FileSystemInfo>();
                if (showSubDirs)
                {
                    //result.Concat(subDirs);
                    foreach (var d in subDirs)
                    {
                        result.Add(d);
                    }
                }
                //result.Concat(files);
                foreach (var f in files)
                {
                    result.Add(f);
                }
                return result;
            }
            catch
            {
                return null;
            }
        }

        /**
         * Lists the contents of a directory.
         */
        public static IEnumerable<FileSystemInfo> ListContents(String dir, bool showSubDirs)
        {
            return ListFilter(dir, new String[] { "*.*" }, showSubDirs);
        }

		/**
		 * Gets the icon for a given path, if available.
		 */
		public static Icon GetIcon(string path)
		{
			SHFILEINFO info = new SHFILEINFO();
			//will need to use an API call, SHGetFileInfo.
			IntPtr largeIconHnd;
			largeIconHnd = Win32API.SHGetFileInfo(	path,
													0,
													ref info,
													(uint)Marshal.SizeOf(info),
													Win32API.SHGFI_ICON | Win32API.SHGFI_LARGEICON
													);
			//now we can pull the icon from that icon handle
			return Icon.FromHandle(info.hIcon);
		}

		public static ImageSource GetImageFromIcon(string path, int imageSize)
		{
			try
			{
				System.Drawing.Icon iconRes = FileSystemOps.GetIcon(path);
				//now convert that into a WPF source
				return System.Windows.Interop.Imaging.CreateBitmapSourceFromHIcon(iconRes.Handle,
					Int32Rect.Empty,
					BitmapSizeOptions.FromWidthAndHeight(imageSize, imageSize));
			}
			catch
			{
				//set to no source, and default icon size
				return null;
			}
		}

		public static bool PathIsValid(string path)
		{
			if (path == null || path.Length < 1)
			{
				return false;
			}

			//first, check that it has no forbidden characters besides ':'
			char[] forbiddenChars = { '*', '?', '\"', '|', '<', '>', '\0' };
			foreach (char c in forbiddenChars)
			{
				if (path.Contains(c))
				{
					return false;
				}
			}
			// if it passes, convert it to a URI. That means:
			//put file:/// at the start
			//replace all \\ with /
			string finalConverted = "file:///";
			string tempPath = String.Copy(path);
			finalConverted += tempPath.Replace('\\', '/');
			return Uri.IsWellFormedUriString(finalConverted, UriKind.Absolute);
		}

		public static bool IsDirectory(FileSystemInfo fs)
		{
			if ((fs.Attributes & FileAttributes.Directory) == 
				FileAttributes.Directory)
			{
				return true;
			}
			return false;
		}
    }
}
