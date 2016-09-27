using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Data;
using System.Windows.Media;
using EditorFileState = ModelTool.UI.Presenter.EditorFileState;

using ModelTool.Properties;
using System.Windows;

namespace ModelTool.UI.ValueConverters
{
	class EditorFileStateToStringConverter : IMultiValueConverter
	{
		public Object Convert(Object[] values, Type targetType, Object parameter, CultureInfo culture)
		{
			EditorFileState fs = (EditorFileState)values[0];
			String fileName = values[1] == DependencyProperty.UnsetValue ? 
								"" : (String)values[1];
			String result = Resources.FileStateNothingLoaded;
			switch (fs)
			{
				case EditorFileState.NothingLoaded:
					result = Resources.FileStateNothingLoaded;
					break;
				case EditorFileState.Ready:
					result = Resources.FileStateReady;
					break;
				case EditorFileState.Reading:
					result = Resources.FileStateReading;
					break;
				case EditorFileState.ReadComplete:
					result = Resources.FileStateReadComplete;
					break;
				case EditorFileState.Importing:
					result = Resources.FileStateImporting;
					break;
				case EditorFileState.ImportComplete:
					result = Resources.FileStateImportComplete;
					break;
				case EditorFileState.Writing:
					result = Resources.FileStateWriting;
					break;
				case EditorFileState.WriteComplete:
					result = Resources.FileStateWriteComplete;
					break;
				case EditorFileState.ChangingDir:
					result = Resources.FileStateChangingDir;
					break;
			}
			return String.Format(result, fileName);
		}

		public Object[] ConvertBack(Object value, Type[] targets, Object parameter, CultureInfo culture)
		{
			return null;
		}
	}
}
