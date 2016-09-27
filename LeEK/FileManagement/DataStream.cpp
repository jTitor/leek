#include "DataStream.h"

using namespace LeEK;

void DefaultAsyncCallback::OnStreamOpen()
{
}

void DefaultAsyncCallback::OnStreamClose()
{
}

void DefaultAsyncCallback::OnChunkRead(FileSz bytesRead)
{
}

void DefaultAsyncCallback::OnChunkWritten(FileSz bytesRead)
{
}

void DefaultAsyncCallback::OnReadEnd()
{
}

void DefaultAsyncCallback::OnWriteEnd()
{
}

void DefaultAsyncCallback::OnError()
{
}

bool DataStream::Open(const char* path)
{
	return Open(path, StreamFlags::ReadWrite);
}

bool DataStream::OpenText(const char* path)
{
	return Open(path,
		StreamFlags::ReadWrite | StreamFlags::Text);
}

bool DataStream::OpenReadOnly(const char* path)
{
	return Open(path,
				StreamFlags::Read);
}

bool DataStream::AsyncRead(IAsyncCallback* asyncCallback)
{
	return DoAsync(AsyncRequest::AsyncRead, NULL, 0, asyncCallback);
}

bool DataStream::AsyncWrite(char* buffer, FileSz bufSz, IAsyncCallback* asyncCallback)
{
	if(!buffer || bufSz <= 0)
	{
		return false;
	}
	return DoAsync(AsyncRequest::AsyncWrite, buffer, bufSz, asyncCallback);
}