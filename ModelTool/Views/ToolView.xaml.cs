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

using ModelTool.UI;
using ModelTool.Model;
using ModelTool.Statics;
using System.Drawing;
using System.ComponentModel;
using ModelTool.Log;
using System.Windows.Interop;

namespace ModelTool.Views
{
	/// <summary>
	/// Interaction logic for ToolView.xaml
	/// </summary>
	public partial class ToolView : UserControl
	{
		private float zoomSpeed = 1.0f/60.0f;
		private float rotSpeed = 1.0f/120.0f;
		System.Drawing.Point lastMousePos;
		float lastZoom = float.NegativeInfinity;

		private void rebuildViewPort()
		{
			//see if the new datacontext is a Presenter;
			//if it is, hook the renderer into the canvas
			if (!(DataContext is Presenter) || PreviewCanvas.Child == null)
			{
				return;
			}
			else
			{
				Presenter dCtx = DataContext as Presenter;
				if (!Double.IsNaN(PreviewCanvas.Width) &&
					!Double.IsNaN(PreviewCanvas.Height))
				{
					PreviewCanvas.Child.Size = new System.Drawing.Size((int)PreviewCanvas.Width,
																		(int)PreviewCanvas.Height);
				}
				dCtx.SetRendererTarget(PreviewCanvas.Child.Handle);
				//PreviewCanvas.Handle
			}
		}

		public ToolView()
		{
			if (DesignerProperties.GetIsInDesignMode(this))
			{
				DataContext = new UI.Presenter();
			}

			InitializeComponent();
			LogList.Background = ThemeBrushes.LogBg;
			LogList.ItemContainerStyle.RegisterName("Font", new Font(System.Drawing.FontFamily.GenericMonospace,
																	12.0f,
																	System.Drawing.FontStyle.Regular));
			//set log filter to the lowest possible level
			LogFilterBox.SelectedIndex = LogFilterBox.Items.Count - 1;
			
			DataContextChanged += ToolView_DataContextChanged;
			ComponentDispatcher.ThreadIdle += ComponentDispatcher_ThreadIdle;
			PreviewCanvas.SizeChanged += PreviewCanvas_SizeChanged;
			PreviewCanvas.Child.MouseWheel += PreviewCanvas_MouseWheel;
			PreviewCanvas.Child.MouseMove += PreviewCanvas_MouseMove;
		}

		private void PreviewCanvas_MouseMove(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			if (e.Button == System.Windows.Forms.MouseButtons.Left)
			{
				var currPos = e.Location;
				//...
				if (lastMousePos.X != int.MinValue &&
					lastMousePos.Y != int.MinValue)
				{
					float xDelta = currPos.X - lastMousePos.X;
					float yDelta = currPos.Y - lastMousePos.Y;
					NativeFunctions.SpinCamera(	rotSpeed * yDelta,
												-rotSpeed * xDelta);
				}
				lastMousePos = currPos;
			}
			else if (e.Button == System.Windows.Forms.MouseButtons.Right)
			{
				float currZoom = e.Y;
				if (lastZoom != float.NegativeInfinity)
				{
					NativeFunctions.ZoomCamera((currZoom - lastZoom) * zoomSpeed);
				}
				lastZoom = currZoom;
			}
			else
			{
				lastMousePos.X = int.MinValue;
				lastMousePos.Y = int.MinValue;
				lastZoom = float.NegativeInfinity;
			}
		}

		private void PreviewCanvas_MouseWheel(object sender, System.Windows.Forms.MouseEventArgs e)
		{
			//NativeFunctions.ZoomCamera(((float)e.Delta) * zoomSpeed);
		}

		void PreviewCanvas_SizeChanged(object sender, SizeChangedEventArgs e)
		{
			rebuildViewPort();
		}

		private void ComponentDispatcher_ThreadIdle(object sender, EventArgs e)
		{
			if (PreviewCanvas.IsVisible)
			{
				NativeFunctions.Draw();
			}
		}

		void ToolView_DataContextChanged(object sender, DependencyPropertyChangedEventArgs e)
		{
			rebuildViewPort();
		}
	}
}
