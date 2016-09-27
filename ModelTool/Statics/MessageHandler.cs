using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ModelTool.Statics
{
	static class MessageHandler
	{
		//message type constants
		const int WM_CLOSE = 0x0010;
		const int WM_MOUSEMOVE = 0x0200;
		const int WM_INPUT = 0x00FF;

		static int[] ENGINE_MSGS = null;//{ WM_CLOSE, WM_MOUSEMOVE, WM_INPUT };

		static MessageHandler()
		{
			ENGINE_MSGS = new int[]{ WM_CLOSE, WM_MOUSEMOVE, WM_INPUT };
		}

		public static IntPtr WndProc(IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled)
		{
			for(int i = 0; i < ENGINE_MSGS.Length; ++i)
			{
				if (msg == ENGINE_MSGS[i])
				{
					//if this message is anything the engine would care about, send it
					IntPtr result = NativeFunctions.WndProc(hwnd, msg, wParam, lParam);
					if (result.ToInt64() != 0)
					{
						handled = true;
					}
					else
					{
						handled = false;
					}
					return result;
				}
			}
			//otherwise, we didn't handle this message
			handled = false;
			return IntPtr.Zero;
		}
	}
}
