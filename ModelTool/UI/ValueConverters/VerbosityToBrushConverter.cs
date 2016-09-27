using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;
using System.Windows.Media;
using LogVerbosity = ModelTool.Log.LogElement.LogVerbosity;

namespace ModelTool.UI.ValueConverters
{
	class VerbosityToBrushConverter : IValueConverter
	{
		public Object Convert(Object value, Type targetType, Object parameter, CultureInfo culture)
		{
			LogVerbosity verb = (LogVerbosity)value;
			Brush result = Brushes.Black;
			switch (verb)
			{
				case LogVerbosity.Error:
					result = ThemeBrushes.LogErr;
					break;
				case LogVerbosity.Warning:
					result = ThemeBrushes.LogWarn;
					break;
				case LogVerbosity.Debug:
					result = ThemeBrushes.LogDebug;
					break;
				case LogVerbosity.Info:
					result = ThemeBrushes.LogInfo;
					break;
				case LogVerbosity.Verbose:
					result = ThemeBrushes.LogVerb;
					break;
			}
			return result;
		}

		public Object ConvertBack(Object value, Type targetType, Object parameter, CultureInfo culture)
		{
			Brush b = (Brush)value;
			LogVerbosity result = LogVerbosity.Error;
			if(b == ThemeBrushes.LogErr)
			{
				result = LogVerbosity.Error;
			}
			else if(b == ThemeBrushes.LogWarn)
			{
				result = LogVerbosity.Warning;
			}
			else if(b == ThemeBrushes.LogDebug)
			{
				result = LogVerbosity.Debug;
			}
			else if(b == ThemeBrushes.LogInfo)
			{
				result = LogVerbosity.Info;
			}
			else if(b == ThemeBrushes.LogVerb)
			{
				result = LogVerbosity.Verbose;
			}
			return result;
		}
	}
}
