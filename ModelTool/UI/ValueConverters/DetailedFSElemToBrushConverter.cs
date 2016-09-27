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
	class DetailedFSElemToBrushConverter : IValueConverter
	{
		public Object Convert(Object value, Type targetType, Object parameter, CultureInfo culture)
		{
			DetailedFSElement elem = (DetailedFSElement)value;
			if (elem.IsDirectory)
			{
				return ThemeBrushes.Clear;
			}
			if (elem.IsEngineModelFile)
			{
				return ThemeBrushes.FileIsInEngineFormatBg;
			}
			if(elem.IsImportable)
			{
				return ThemeBrushes.FileCanBeConvertedBg;
			}
			return ThemeBrushes.Clear;
		}

		public Object ConvertBack(Object value, Type targetType, Object parameter, CultureInfo culture)
		{
			//No damn way to do this!
			return null;
		}
	}
}
