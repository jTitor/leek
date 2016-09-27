using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Input;

namespace ModelTool.UI.Behaviors
{
	/**
	 * An attached behavior that allows tagged elements to respond
	 * to double MouseLeftButtonUp events.
	 */
	class MouseLeftButtonUp
	{
		//You register commands on instantiation;
		//Since these are dependency properties,
		//they update automatically
		public static DependencyProperty CommandProperty =
		DependencyProperty.RegisterAttached("Command",
											typeof(ICommand),
											typeof(MouseLeftButtonUp),
											new FrameworkPropertyMetadata(null,CommandChanged));

		public static DependencyProperty CommandParameterProperty =
			DependencyProperty.RegisterAttached("CommandParameter",
												typeof(object),
												typeof(MouseLeftButtonUp),
												new UIPropertyMetadata(null));

		public static void SetCommand(DependencyObject target, ICommand value)
		{
			target.SetValue(CommandProperty, value);
		}
		public static ICommand GetCommand(DependencyObject target)
		{
			return (ICommand)target.GetValue(CommandProperty);
		}

		public static void SetCommandParameter(DependencyObject target, object value)
		{
			target.SetValue(CommandParameterProperty, value);
		}
		public static object GetCommandParameter(DependencyObject target)
		{
			return target.GetValue(CommandParameterProperty);
		}

		private static void CommandChanged(DependencyObject target, DependencyPropertyChangedEventArgs e)
		{
			Control control = target as Control;
			//swap out mouse event handlers as needed.
			if (control != null)
			{
				if ((e.NewValue != null) && (e.OldValue == null))
				{
					control.MouseLeftButtonUp += OnMouseLeftButtonUp;
				}
				else if ((e.NewValue == null) && (e.OldValue != null))
				{
					control.MouseLeftButtonUp -= OnMouseLeftButtonUp;
				}
			}
		}

		//Trip whatever command we have set.
		private static void OnMouseLeftButtonUp(object sender, RoutedEventArgs e)
		{
			Control control = sender as Control;
			bool isTreeView = e.Source is TreeViewItem;
			if ((isTreeView &&
				(e.Source as TreeViewItem).IsSelected) || !isTreeView)
			{
				ICommand command = (ICommand)control.GetValue(CommandProperty);
				object commandParameter = control.GetValue(CommandParameterProperty);
				command.Execute(commandParameter);
			}
		}
	}
}
