using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ModelTool.Statics
{
	static class MathHelpers
	{
		public static T Clamp<T>(T val, T min, T max) where T : IComparable
		{
			return val.CompareTo(max) > 0 ? max : val.CompareTo(min) < 0 ? min : val;
		}

		public static float Clamp(float val)
		{
			return Clamp(val, 0, 1);
		}
	}
}
