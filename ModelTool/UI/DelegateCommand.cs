using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace ModelTool.UI
{
	public class DelegateCommand : ICommand
	{
		private readonly Action action;

		public DelegateCommand(Action pAction)
		{
			action = pAction;
		}

		public void Execute(object parameter)
		{
			action();
		}

		public bool CanExecute(object parameter)
		{
			return true;
		}

		//this is *supposed* to only be stored
#pragma warning disable 67
		public event EventHandler CanExecuteChanged;
#pragma warning restore 67
	}

	public class DelegateCommand<T> : ICommand
	{
		private readonly Action<T> action;

		public DelegateCommand(Action<T> pAction)
		{
			action = pAction;
		}

		public void Execute(object parameter)
		{
			action(parameter != null ? (T)parameter : default(T));
		}

		public bool CanExecute(object parameter)
		{
			return true;
		}

		//this is *supposed* to only be stored
#pragma warning disable 67
		public event EventHandler CanExecuteChanged;
#pragma warning restore 67
	}
}
