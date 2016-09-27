#pragma once
#include "Datatypes.h"
#include <cstdio>

namespace LeEK
{
	typedef U32 StreamFlag;
	/**
	Contains flags describing how a stream should behave.
	*/
	namespace StreamFlags
	{
		/**
		The stream allows reading.
		*/
		StreamFlag Read = 1 << 0;
		/**
		The stream allows writing.
		*/
		StreamFlag Write = 1 << 1;
		/**
		The stream allows reading and writing.
		*/
		StreamFlag ReadWrite = Read | Write;
		/**
		The stream is a text mode stream.
		*/
		StreamFlag Text = 1 << 3;
	}

	enum AsyncRequest
	{
		AsyncRead,
		AsyncWrite
	};

#define ASYNC_CHUNK_SZ 65536
#define	ASYNC_CHUNK_COUNT 3

	/**
	Responder class for an asynchronous read.
	*/
	class IAsyncCallback
	{
	public:
		/**
		Called when the stream is first opened.
		*/
		virtual void OnStreamOpen() = 0;
		/**
		Called when the stream is closed.
		*/
		virtual void OnStreamClose() = 0;
		/**
		Called when a chunk of data has been read.
		*/
		virtual void OnChunkRead(FileSz bytesRead) = 0;
		/**
		Called when a chunk of data has been written.
		*/
		virtual void OnChunkWritten(FileSz bytesRead) = 0;
		/**
		Called when a read request has been completed.
		*/
		virtual void OnReadEnd() = 0;
		/**
		Called when a write request has been completed.
		*/
		virtual void OnWriteEnd() = 0;
		/**
		Called when the DataStream has an error that stops the current
		read/write request.
		*/
		virtual void OnError() = 0;
	};

	class DefaultAsyncCallback : public IAsyncCallback
	{
		public:
			/**
			Called when the stream is first opened.
			*/
			void OnStreamOpen();
			/**
			Called when the stream is closed.
			*/
			void OnStreamClose();
			/**
			Called when a chunk of data has been read.
			*/
			void OnChunkRead(FileSz bytesRead);
			/**
			Called when a chunk of data has been written.
			*/
			void OnChunkWritten(FileSz bytesRead);
			/**
			Called when a read request has been completed.
			*/
			void OnReadEnd();
			/**
			Called when a write request has been completed.
			*/
			void OnWriteEnd();
			/**
			Called when the DataStream has an error that stops the current
			read/write request.
			*/
			void OnError();
	};

	class AsyncBuffer
	{
	public:
		AsyncBuffer(FileSz pBufSz, FileSz pBufCount);
		~AsyncBuffer();
		char* GetCurrBuffer();
		char* GetNextBuffer();
		void RotateBuffers();
	};

	//TODO: Refactor to IDataStream
	class DataStream
	{
	public:
		virtual ~DataStream(void) {}
		//virtual bool IsAsync() = 0;
		/**
		Called when a stream is opened. Implementor must use [flags]
		to determine what mode to open the stream in.
		[asyncCallback] is optional.
		*/
		virtual bool Open(const char* path, StreamFlag flags) = 0;
		/**
		Asynchronously reads or writes [bufSz] bytes to/from the given buffer.
		*/
		virtual bool DoAsync(AsyncRequest asyncType, char* buffer, FileSz bufSz, IAsyncCallback* asyncCallback) = 0;
		bool AsyncRead(IAsyncCallback* asyncCallback);
		bool AsyncWrite(char* buffer, FileSz bufSz, IAsyncCallback* asyncCallback);

		bool Open(const char* path);
		bool OpenText(const char* path);
		bool OpenReadOnly(const char* path);
		virtual bool Close() = 0;
		virtual FileSz FileSize() = 0;
		virtual FileSz Write(const char* buffer, FileSz size) = 0;
		virtual FileSz Write(const char* buffer) = 0;
		virtual FileSz WriteLine(const char* line = "") = 0;
		virtual FileSz Read(char* buffer, FileSz size) = 0;
		virtual FileSz WritePos() = 0;
		virtual FileSz ReadPos() = 0;
		virtual char* ReadAll() = 0;
		virtual FileSz Seek(FileSz position) = 0; //move to (position) bytes from beginning of file
		virtual FileSz FileEnd() { return FileSize() - 1; }
		virtual FileSz SeekToEnd() { return Seek(FileSize()-1); }
		virtual FileSz SeekFromEnd(FileSz negPosition) { return Seek(FileSize() - negPosition); }
		//virtual FileSz WriteAsync(const U8* buffer, FileSz offset, FileSz count) = 0;
		//virtual FileSz ReadAsync(U8* buffer, FileSz offset, FileSz count) = 0;
		//virtual FileSz ReadByte() = 0;
	};
}