using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ModelTool.UI
{
	public class ViewModel : INotifyPropertyChanged
	{
		//what to do when a property changes
		public event PropertyChangedEventHandler PropertyChanged;

		protected void NotifyPropertyChanged(string propertyName)
		{
			var handler = PropertyChanged;
			//of course only trigger if we have an event handler.
			if (handler != null)
			{
				handler(this, new PropertyChangedEventArgs(propertyName));
			}
		}
	}
}
