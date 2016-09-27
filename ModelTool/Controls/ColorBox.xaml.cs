using ModelTool.Model;
using ModelTool.Src;
using ModelTool.Statics;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
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
using System.Windows.Threading;

namespace ModelTool.Controls
{
	/// <summary>
	/// Interaction logic for Vector3Box.xaml
	/// </summary>
	public partial class ColorBox : UserControl
	{
		//Dispatcher uiThread = Dispatcher.CurrentDispatcher;

		SolidColorBrush previewBrush = new SolidColorBrush(Colors.Black);
		public ColorBox()
		{
			InitializeComponent();
			//uiThread = ;
			preview.Fill = previewBrush;
		}

		private float tryGetBox(TextBox box)
		{
			float result = 0.0f;
			if (!float.TryParse(box.Text, out result))
			{
				return 0.0f;
			}
			return result;
		}

		void updatePreview()
		{
			previewBrush.Color = Values.AsMsftColor;
		}

		void requestUpdatePreview()
		{
			//uiThread.Invoke(updatePreview);
		}

		/**
		 * Clamps value to the range 
		 */
		float clampValue(float val)
		{
			return MathHelpers.Clamp(val);
		}

		public float R
		{
			get
			{
				return (float)GetValue(RProperty);
			}
			set
			{
				rBox.Text = clampValue(value).ToString();
				SetValue(RProperty, value);
			}
		}

		public float G
		{
			get
			{
				return (float)GetValue(GProperty);
			}
			set
			{
				gBox.Text = clampValue(value).ToString();
				SetValue(GProperty, value);
			}
		}

		public float B
		{
			get
			{
				return (float)GetValue(BProperty);
			}
			set
			{
				bBox.Text = clampValue(value).ToString();
				SetValue(BProperty, value);
			}
		}

		public float A
		{
			get
			{
				return (float)GetValue(AProperty);
			}
			set
			{
				aBox.Text = clampValue(value).ToString();
				SetValue(RProperty, value);
			}
		}

		public LColor Values
		{
			get { return (LColor)GetValue(ValuesProperty); }
			set
			{
				SetValue(ValuesProperty, value);
				A = value.A;
				R = value.R;
				G = value.G;
				B = value.B;
				//requestUpdatePreview();
			}
		}

		#region Dependency Property Registrations
		public static readonly DependencyProperty RProperty =
			DependencyProperty.RegisterAttached("R",
												typeof(float),
												typeof(ColorBox),
												new FrameworkPropertyMetadata
													(0f,
														FrameworkPropertyMetadataOptions.BindsTwoWayByDefault
													));

		public static readonly DependencyProperty GProperty =
			DependencyProperty.RegisterAttached("G",
												typeof(float),
												typeof(ColorBox),
												new FrameworkPropertyMetadata
													(0f,
														FrameworkPropertyMetadataOptions.BindsTwoWayByDefault
													));

		public static readonly DependencyProperty BProperty =
			DependencyProperty.RegisterAttached("B",
												typeof(float),
												typeof(ColorBox),
												new FrameworkPropertyMetadata
													(0f,
														FrameworkPropertyMetadataOptions.BindsTwoWayByDefault
													));

		public static readonly DependencyProperty AProperty =
			DependencyProperty.RegisterAttached("A",
												typeof(float),
												typeof(ColorBox),
												new FrameworkPropertyMetadata
													(0f,
													 FrameworkPropertyMetadataOptions.BindsTwoWayByDefault
													));

		public static readonly DependencyProperty ValuesProperty =
			DependencyProperty.RegisterAttached("Values",
												typeof(LColor),
												typeof(ColorBox),
												new FrameworkPropertyMetadata
													(default(LColor),
													 FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
													 onValuesChanged
													));

		private static void onValuesChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
		{
			if (d.GetType() == typeof(ColorBox) && e.Property.PropertyType == typeof(LColor))
			{
				var castVal = (LColor)e.NewValue;
				//also set X, Y, and Z
				var source = d as ColorBox;
				source.R = castVal.R;
				source.G = castVal.G;
				source.B = castVal.B;
				source.A = castVal.A;

				source.updatePreview();
			}
		}
		#endregion

		private void onTextChanged(object sender, TextChangedEventArgs e)
		{
			//try to convert to a float; if it doesn't work, reset to 0
			var textBox = (TextBox)sender;
			float newVal = 0;
			if (!float.TryParse(textBox.Text, out newVal))
			{
				textBox.Text = 0.0.ToString();
				return;
			}
			textBox.Text = newVal.ToString();
			requestUpdatePreview();
		}
	}
}
