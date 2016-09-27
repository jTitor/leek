using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ModelTool.Log
{
	public class LogElement
	{
		public enum LogVerbosity
		{
			Error	= 0,
			Warning	= 1,
			Info	= 2,
			Debug	= 3,
			Verbose	= 4
		};

		public static IEnumerable<LogVerbosity> LogVerbosityValues
		{
			get { return Enum.GetValues(typeof(LogVerbosity)).Cast<LogVerbosity>(); }
		}

		private Tuple<string, LogVerbosity> data;

		public LogElement(string text, LogVerbosity verbLevel)
		{
			data = new Tuple<string, LogVerbosity>(text, verbLevel);
		}

		public string Text
		{
			get { return data.Item1; }
		}

		public LogVerbosity Verbosity
		{
			get { return data.Item2; }
		}
	}
}
