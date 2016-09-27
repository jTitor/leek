using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Controls;
using System.Windows.Input;
//using System.Drawing;
using ModelTool.Src;
using System.Windows;
using System.Windows.Media.Imaging;
using ModelTool.Statics;

namespace ModelTool.UI
{
	/**
	 * Represents an element within a directory.
	 */
    class ExplorerElement : TreeViewItem
    {
		private static int iconSize = 16;
		private static Thickness padding = new Thickness(1);
		private const int MIN_ICON_SIZE = 8;
		private const double fontScale = 0.6;

        private FileSystemInfo info;

		private void initHeader()
		{
			//Whatever's in the Header is displayed, 
			//so to display icons by the name, you can do a StackView
			//starting with the icon, then label.
			StackPanel panel = new StackPanel();
			panel.Orientation = Orientation.Horizontal;
			panel.Margin = padding;
			//load the element's shell icon
			Image elemIcon = new Image();
			elemIcon.Margin = padding;
			try
			{
				System.Drawing.Icon iconRes = FileSystemOps.GetIcon(info.FullName);
				//now convert that into a WPF source and connect it to the image
				elemIcon.Source = System.Windows.Interop.Imaging.CreateBitmapSourceFromHIcon(iconRes.Handle,
					Int32Rect.Empty,
					BitmapSizeOptions.FromWidthAndHeight(iconSize, iconSize));
			}
			catch
			{
				//set to no source, and default icon size
				elemIcon.Source = null;
				elemIcon.Width = iconSize;
				elemIcon.Height = iconSize;
			}
			//Set the element's text label too
			TextBlock text = new TextBlock();
			text.Text = info.Name;
			text.VerticalAlignment = System.Windows.VerticalAlignment.Center;
			//text.FontSize = ((double)iconSize) * fontScale;
			//Put those in the stackpanel
			panel.Children.Add(elemIcon);
			panel.Children.Add(text);

			//And set THAT as header.
			Header = panel;
		}

        /**
         * Creates a new explorer element.
         * @param fsElem filesystem info that this element represents.
         * @param dblClickHandler a delegate triggered when this element is double clicked.
         */
        public ExplorerElement(FileSystemInfo fsElem, MouseButtonEventHandler dblClickHandler)
        {
			info = fsElem;
			initHeader();            
            MouseDoubleClick += dblClickHandler;
        }

		public static int IconSize
		{
			get { return iconSize; }
			set { iconSize = Math.Max(value, MIN_ICON_SIZE); }
		}

		public static double Padding
		{
			get { return padding.Left; }
			set { padding = new Thickness(Math.Max(value, 0)); }
		}

        /**
         * Allows access to the file system info.
         */
        public FileSystemInfo Info
        {
            get { return info; }
        }

        /**
         * If true, this element represents a directory.
         */
        public bool IsDirectory
        {
            get { return (Info.Attributes & FileAttributes.Directory) == FileAttributes.Directory; }
        }
    }
}
