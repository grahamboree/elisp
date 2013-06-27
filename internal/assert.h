/*
 *
 */

#pragma once 

////////////////////////////////////////////////////////////////////////////////
void die(string message)
{
	throw logic_error(message);
}
void trueOrDie(bool condition, string message)
{
	if (!condition)
		die(message);
}

