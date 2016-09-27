using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace ModelTool.Statics
{
	static class Win32API
	{
		public const uint SHGFI_ICON = 0x100;
		public const uint SHGFI_LARGEICON = 0x0;
		public const uint SHGFI_SMALLICON = 0x1;

		//Retrieves information about a file.
		[DllImport("shell32.dll")]
		public static extern IntPtr SHGetFileInfo(	string pszPath,
													uint dwFileAttributes, 
													ref SHFILEINFO psfi,
													uint cbSizeFileInfo,
													uint uFlags);
	}
}
