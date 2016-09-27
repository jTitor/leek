using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;
using System.IO;

using ModelTool.Src;
using ModelTool.UI;
using System.Windows.Threading;
using System.Timers;
using System.Runtime.InteropServices;
using System.Collections.Specialized;
using System.Windows.Interop;
using ModelTool.Statics;

namespace ModelTool
{
    /// <summary>
    /// Interaction logic for MainWindow.xaml
    /// </summary>
    public partial class MainWindow : Window
    {
		public MainWindow()
		{
			InitializeComponent();
		}

		protected override void OnSourceInitialized(EventArgs e)
		{
			base.OnSourceInitialized(e);
			//Now we also need to handle message loops; hook us into a Win32 window.
			HwndSource source = PresentationSource.FromVisual(this) as HwndSource;
			//Notify that we're interested in raw input events
			NativeFunctions.SetInputTargetHWND(source.Handle);
			//And set the window proc for this window.
			source.AddHook(new HwndSourceHook(MessageHandler.WndProc));
		}
    }
}
