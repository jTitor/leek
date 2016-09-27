using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.IO;
using System.Runtime.InteropServices;
using System.ComponentModel;

namespace ModelTool.Statics
{
	static class ImporterOps
	{
		//private static Progress<int> fileProgress;
		private static int ioBufferSize = 4096;
		private static bool shouldCancel = false;

		static ImporterOps()
		{
			//fileLoader = new BackgroundWorker();
			//fileLoader.DoWork += fileLoader_DoWork;
			//fileLoader.RunWorkerCompleted += fileLoader_RunWorkerCompleted;
			//fileLoader.WorkerReportsProgress = true;
			//fileProgress = new Progress<int>();
		}

		/*
		public static Progress<int> FileProgress
		{
			get { return fileProgress; }
		}
		*/

		private static int importModelFromBuffer(byte[] data)
		{
			IntPtr nativeBuf = Marshal.AllocCoTaskMem(data.Length);
			Marshal.Copy(data, 0, nativeBuf, data.Length);
			int result = NativeFunctions.ImportFromBuffer(nativeBuf, data.Length);
			Marshal.FreeCoTaskMem(nativeBuf);

			return result;
		}

		private static byte[] exportModelToBuffer(int mdlIdx)
		{
			int bufSz = NativeFunctions.GetModelExportedSize(mdlIdx);
			IntPtr nativeBuf = Marshal.AllocHGlobal(bufSz);
			NativeFunctions.ExportModel(mdlIdx, nativeBuf, bufSz);
			byte[] data = new byte[bufSz];
			Marshal.Copy(nativeBuf, data, 0, bufSz);
			Marshal.FreeHGlobal(nativeBuf);

			return data;
		}

		//right now, only Assimp is available.
		public static string[] GetImportFormats()
		{
			return FileFilters.AssimpCompatible;
		}

		public static int ImportModel(string path)
		{
			byte[] data = File.ReadAllBytes(path);

			return importModelFromBuffer(data);
		}

		public static async Task<int> ImportModelAsync(string path, IProgress<float> fileProgress)
		{
			if (!FileSystemOps.PathIsValid(path))
			{
				return -1;
			}
			FileStream file = new FileStream(
										path,
										FileMode.Open,
										FileAccess.Read,
										FileShare.Read,	//important - 
				//we want other programs to be able to read while we're reading
										ioBufferSize,
										true);			//actually make stream an async stream!

			//fileLoader.RunWorkerAsync(path);
			byte[] data = new byte[file.Length];
			int dataHead = 0;
			//int bytesRead = 0;
			//read file into memory
			//var readFileTask = Task.Run(async () =>
				//{
					while (!shouldCancel)
					{
						if (dataHead >= data.Length)
						{
							break;
						}
						int maxBytesRead = Math.Min(ioBufferSize, data.Length - dataHead);
						int bytesRead = await file.ReadAsync(data, dataHead, maxBytesRead);
						dataHead += bytesRead;
						dataHead = MathHelpers.Clamp(dataHead, 0, data.Length);
						if (fileProgress != null)
						{
							fileProgress.Report(((float)bytesRead / data.Length) * 100f);
						}
					}
				//});
			if (shouldCancel)
			{
				//operation canceled; report something and reset the canceler
				shouldCancel = false;
				return -1;
			}
			//await readFileTask;
			//now proceed as normal
			int result = importModelFromBuffer(data);

			return result;
		}

		public static void ExportModel(string path, int mdlIdx)
		{
			var data = exportModelToBuffer(mdlIdx);
			File.WriteAllBytes(path, data);
		}

		public async static Task ExportModelAsync(string path, int mdlIdx)
		{
			if (!FileSystemOps.PathIsValid(path))
			{
				return;
			}
			FileStream file = new FileStream(
										path,
										FileMode.Open,
										FileAccess.ReadWrite,
										FileShare.Read,	//important - 
				//we want other programs to be able to read while we're reading
										ioBufferSize,
										true);			//actually make stream an async stream!
			//file.Seek(0, SeekOrigin.

			var data = exportModelToBuffer(mdlIdx);

			await file.WriteAsync(data, 0, data.Length);
		}
	}
}
