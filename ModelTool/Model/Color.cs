using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;
using System.Windows.Media;

using ModelTool.Statics;

namespace ModelTool.Model
{
    /**
     *  Stores colors in normalized RGBA format.
     */
    public class LColor
    {
        private static int size = 4;
        public static LColor Black = new LColor(0, 0, 0, 0);

        public float R, G, B, A;

        public LColor(float pR, float pG, float pB, float pA)
        {
            R = MathHelpers.Clamp(pR);
            G = MathHelpers.Clamp(pG);
            B = MathHelpers.Clamp(pB);
            A = MathHelpers.Clamp(pA);
        }

		public LColor()
			: this(0, 0, 0, 255)
		{}

        public static LColor FromFloatArray(float[] val)
        {
            if (val.Length < size)
            {
                return LColor.Black;
            }
            return new LColor(val[0], val[1], val[2], val[3]);
        }

        public static LColor FromNativeArray(IntPtr val)
        {
            //Unfortunately, you won't know size data for this array.
            //If the pointer's invalid, you'll just end up with corrupted data.
            float[] converted = new float[size];
            Marshal.Copy(val, converted, 0, size);
            return FromFloatArray(converted);
        }

		public Color AsMsftColor
		{
			get 
			{
				byte cR, cG, cB, cA;
				cR = (byte)(255 * R);
				cG = (byte)(255 * G);
				cB = (byte)(255 * B);
				cA = (byte)(255 * A);

				return Color.FromArgb(cA, cR, cG, cB);
			}
		}
    }
}
