#include "StringUtils.h"

using namespace LeEK;

//TODO: fix for Unicode?
//Attempts to match a wildcard against a given string.
//Returns true if the ENTIRETY of the string matches the wildcard,
//false otherwise.
//Possible Glitches:
//"**" matches against stars in input string?
bool StringUtils::WildcardMatch(const char* str, const char* wildcard)
{
	//iterate through wildcard.
	const char* strPos = str;
	const char* wcPos = wildcard;
	//if the wildcard's character is a *, need to check the char ahead of it.
	while(*strPos != 0)
	{
		switch(*wcPos)
		{
		//if the wildcard's character is a *, need to check the char ahead of it.
		case '*':
			//"*" matches against everything
			if(*(wcPos+1) == 0)
			{
				return true;
			}
			//if the char ahead matches the current char in the string,
			//skip 2 chars ahead in the wildcard
			else if(*strPos == *(wcPos+1))
			{
				wcPos += 2;
			}
			break;
		//? matches against current character
		case '?':
			++wcPos;
			break;
		//if we're at the end of the wildcard before the end of the string,
		//the wildcard's malformed (empty string doesn't imply "*");
		//report no match
		case 0:
			return true;
		//anything that's not a wildcard,
		//do a direct comparison
		default:
			if(*strPos != *wcPos)
			{
				return false;
			}
			++wcPos;
			break;
		}
		//and advance the string
		++strPos;
	}
	//if we're at the end of the string, but not the wildcard,
	//the match failed
	if(*wcPos != 0)
	{
		return false;
	}
	//otherwise, we're at the end of both; report a match
	return true;
}