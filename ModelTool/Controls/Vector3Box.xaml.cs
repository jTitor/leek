using ModelTool.Model;
using ModelTool.Src;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Runtime.CompilerServices;
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

namespace ModelTool.Controls
{
	/// <summary>
	/// Interaction logic for Vector3Box.xaml
	/// </summary>
	public partial class Vector3Box : UserControl//, INotifyPropertyChanged
	{
		/*
		//Kind of goofy, but since the class is partial due to the link with the XAML,
		//we need to put the interface implementation in here
		#region INotifyPropertiesChanged Implementation
		//what to do when a property changes
		public event PropertyChangedEventHandler PropertyChanged;

		protected void NotifyPropertyChanged([CallerMemberName] string propertyName = "")
		{
			var handler = PropertyChanged;
			//of course only trigger if we have an event handler.
			if (handler != null)
			{
				handler(this, new PropertyChangedEventArgs(propertyName));
			}
		}
		#endregion
		 */

		public Vector3Box()
		{
			InitializeComponent();
			//xBox.TextChanged += onTextChanged;
			//yBox.TextChanged += onTextChanged;
			//zBox.TextChanged += onTextChanged;
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

		public float X
		{
			get
			{
				return (float)GetValue(XProperty);//tryGetBox(xBox);
			}
			set
			{
				SetValue(XProperty, value);
				xBox.Text = value.ToString();
				//NotifyPropertyChanged();
			}
		}

		public float Y
		{
			get
			{
				//return tryGetBox(yBox);
				return (float)GetValue(YProperty);
			}
			set
			{
				SetValue(YProperty, value);
				yBox.Text = value.ToString();
				//NotifyPropertyChanged();
			}
		}

		public float Z
		{
			get
			{
				//return tryGetBox(zBox);
				return (float)GetValue(ZProperty);
			}
			set
			{
				SetValue(ZProperty, value);
				zBox.Text = value.ToString();
				//NotifyPropertyChanged();
			}
		}

		public Vector3 Values
		{
			get 
			{
				return (Vector3)GetValue(ValuesProperty);
				//return new Vector3(X,Y,Z);
			}
			set
			{
				SetValue(ValuesProperty, value);
				X = value.X;
				Y = value.Y;
				Z = value.Z;
			}
		}

		#region Dependency Property Registrations
		public static readonly DependencyProperty XProperty =
			DependencyProperty.Register("X",
												typeof(float),
												typeof(Vector3Box),
												new FrameworkPropertyMetadata
													(0f,
														FrameworkPropertyMetadataOptions.BindsTwoWayByDefault
													));

		public static readonly DependencyProperty YProperty =
			DependencyProperty.Register("Y",
												typeof(float),
												typeof(Vector3Box),
												new FrameworkPropertyMetadata
													(0f,
														FrameworkPropertyMetadataOptions.BindsTwoWayByDefault
													));

		public static readonly DependencyProperty ZProperty =
			DependencyProperty.Register("Z",
												typeof(float),
												typeof(Vector3Box),
												new FrameworkPropertyMetadata
													(0f,
														FrameworkPropertyMetadataOptions.BindsTwoWayByDefault
													));

		public static readonly DependencyProperty ValuesProperty =
			DependencyProperty.Register("Values",
												typeof(Vector3),
												typeof(Vector3Box),
												new FrameworkPropertyMetadata
													(default(Vector3),
														FrameworkPropertyMetadataOptions.BindsTwoWayByDefault,
														onValuesChanged
													));

		
		private static void onValuesChanged(DependencyObject d, DependencyPropertyChangedEventArgs e)
		{
			if (d.GetType() == typeof(Vector3Box) && e.Property.PropertyType == typeof(Vector3))
			{
				var castVal = (Vector3)e.NewValue;
				//also set X, Y, and Z
				var source = d as Vector3Box;
				source.X = castVal.X;
				source.Y = castVal.Y;
				source.Z = castVal.Z;
			}
			//e.NewValue
		}

		#endregion

		private void onTextChanged(object sender, TextChangedEventArgs e)
		{
			//try to convert to a float; if it doesn't work, reset to 0
			var textBox = (TextBox)sender;
			float newVal = 0;
			if(!float.TryParse(textBox.Text, out newVal))
			{
				textBox.Text = 0.0.ToString();
				return;
			}
			textBox.Text = newVal.ToString();
		}


	}
}
