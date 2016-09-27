using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Input;

namespace ModelTool.UI
{
	public class AsyncDelegateCommand : ICommand
	{
		private readonly Func<Task> action;
		bool isExecuting;

		public AsyncDelegateCommand(Func<Task> pAction)
		{
			action = pAction;
			isExecuting = false;
		}

		public async void Execute(object parameter)
		{
			// tell the control that we're now executing...
			isExecuting = true;
			OnCanExecuteChanged();
			try
			{
				await action();
			}
			finally
			{
				// tell the control we're done
				isExecuting = false;
				OnCanExecuteChanged();
			}
		}

		public bool CanExecute(object parameter)
		{
			return true;
		}

		protected virtual void OnCanExecuteChanged()
		{
			if (CanExecuteChanged != null)
			{
				CanExecuteChanged(this, new EventArgs());
			}
		}

		//this is *supposed* to only be stored
#pragma warning disable 67
		public event EventHandler CanExecuteChanged;
#pragma warning restore 67
	}

	public class AsyncDelegateCommand<T> : ICommand
	{
		private readonly Func<T, Task<T>> action;
		bool isExecuting;

		public AsyncDelegateCommand(Func<T, Task<T>> pAction)
		{
			action = pAction;
			isExecuting = false;
		}

		public async void Execute(object parameter)
		{
			// tell the control that we're now executing...
			isExecuting = true;
			OnCanExecuteChanged();
			try
			{
				await action(parameter != null ? (T)parameter : default(T));
			}
			finally
			{
				// tell the control we're done
				isExecuting = false;
				OnCanExecuteChanged();
			}
		}

		public bool CanExecute(object parameter)
		{
			return !isExecuting;
		}

		protected virtual void OnCanExecuteChanged()
		{
			if (CanExecuteChanged != null)
			{
				CanExecuteChanged(this, new EventArgs());
			}
		}

		//this is *supposed* to only be stored
#pragma warning disable 67
		public event EventHandler CanExecuteChanged;
#pragma warning restore 67
	}
}
