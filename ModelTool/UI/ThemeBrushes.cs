using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Media;

namespace ModelTool.UI
{
	public static class ThemeBrushes
	{
		public static Brush Clear = new SolidColorBrush(Color.FromArgb(0,0,0,0));

		//General font colors.
		public static Brush FontNormal = SystemColors.ControlTextBrush;
		public static Brush FontLight = new SolidColorBrush(Color.FromArgb(255, 255, 255, 255));

		//Address bar brushes
		public static Brush NormalAddressBg = SystemColors.ControlLightLightBrush; //AddressBar.Background;
        public static Brush FailedAddressBg = new SolidColorBrush(Color.FromRgb(255, 192, 192));

		//Explorer brushes
		public static Brush NormalExplorerBg = NormalAddressBg;
        public static Brush EmptyExplorerBg = new SolidColorBrush(Color.FromRgb(192, 192, 192));

		//Individual file brushes.
        public static Brush NormalFileBg = NormalExplorerBg;
        public static Brush FileCanBeConvertedBg = new SolidColorBrush(Color.FromRgb(192, 192, 255));
        public static Brush FileIsInEngineFormatBg = new SolidColorBrush(Color.FromRgb(192, 255, 192));

		//Log brushes.
		public static Brush LogBg = new SolidColorBrush(Color.FromRgb(16, 24, 32));
		public static Brush LogVerb = new SolidColorBrush(Color.FromRgb(216, 216, 216));
		public static Brush LogInfo = new SolidColorBrush(Color.FromRgb(255, 255, 255));
		public static Brush LogDebug = new SolidColorBrush(Color.FromRgb(0, 255, 0));
		public static Brush LogWarn = new SolidColorBrush(Color.FromRgb(255, 255, 0));
		public static Brush LogErr = new SolidColorBrush(Color.FromRgb(255, 0, 0));
	}
}
