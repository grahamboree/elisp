/*
 *
 */

#pragma once 

////////////////////////////////////////////////////////////////////////////////
void die(string message = "An unknown error occured")
{
	throw runtime_error(message);
}
void trueOrDie(bool condition, string message)
{
	if (!condition)
		die(message);
}

