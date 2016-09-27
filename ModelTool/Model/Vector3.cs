using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Windows;

using ModelTool.Src;

namespace ModelTool.Model
{
    public class Vector3 : DependencyObject
    {
        private static int size = 3;
        public static Vector3 Zero = new Vector3(0, 0, 0);

        public float x, y, z;

		public float X
		{
			get { return x; }
			set { x = value; }
		}
		public static DependencyProperty XProperty =
			DependencyProperty.RegisterAttached("X",
												typeof(float),
												typeof(Vector3));

		public float Y
		{
			get { return y; }
			set { y = value; }
		}
		public static DependencyProperty YProperty =
			DependencyProperty.RegisterAttached("Y",
												typeof(float),
												typeof(Vector3));

		public float Z
		{
			get { return z; }
			set { z = value; }
		}
		public static DependencyProperty ZProperty =
			DependencyProperty.RegisterAttached("Z",
												typeof(float),
												typeof(Vector3));

        public Vector3(float pX, float pY, float pZ)
        {
            x = pX;
            y = pY;
            z = pZ;
        }

		public Vector3()
			: this(0, 0, 0)
		{ }

        public static Vector3 FromFloatArray(float[] val)
        {
            if (val.Length < size)
            {
                return Vector3.Zero;
            }
            return new Vector3(val[0], val[1], val[2]);
        }

        public static Vector3 FromNativeArray(IntPtr val)
        {
            //Unfortunately, you won't know size data for this array.
            //If the pointer's invalid, you'll just end up with corrupted data.
            float[] converted = new float[size];
            Marshal.Copy(val, converted, 0, size);
            return FromFloatArray(converted);
        }
    }
}
