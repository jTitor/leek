#pragma once

namespace LeEK
{
	//From http://randomascii.wordpress.com/2013/04/03/stop-using-strncpy-already/
	template <size_t charCount>
	void strcpySafe(char (&output)[charCount], const char* pSrc)
	{
		//YourCopyNFunction(output, pSrc, charCount);
		// Copy the string — don’t copy too many bytes.
		strncpy(output, pSrc, charCount);
		// Ensure null-termination.
		output[charCount - 1] = 0;
	}
}