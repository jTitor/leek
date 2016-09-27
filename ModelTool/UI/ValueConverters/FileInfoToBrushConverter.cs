using System;
using System.Collections.Generic;
using System.Globalization;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Data;
using System.Windows.Media;

using ModelTool.Statics;

namespace ModelTool.UI.ValueConverters
{
	class FileInfoToBrushConverter : IValueConverter
	{
		public Object Convert(Object value, Type targetType, Object parameter, CultureInfo culture)
		{
			FileSystemInfo elem = (FileSystemInfo)value;
			Brush result = ThemeBrushes.Clear;
			if ((elem.Attributes & FileAttributes.Directory) == FileAttributes.Directory)
			{
				return result;
			}
			if (elem.Extension == FileFilters.EngineModel)
			{
				result = ThemeBrushes.FileIsInEngineFormatBg;
			}
			else
			{
				foreach (string ext in FileFilters.AssimpCompatible)
				{
					if (elem.Extension == ext)
					{
						result = ThemeBrushes.FileCanBeConvertedBg;
					}
				}
			}
			return result;
		}

		public Object ConvertBack(Object value, Type targetType, Object parameter, CultureInfo culture)
		{
			//No damn way to do this!
			return DependencyProperty.UnsetValue;
		}
	}
}
