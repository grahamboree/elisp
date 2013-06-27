
#pragma once 

////////////////////////////////////////////////////////////////////////////////
void die(string message = "An unknown error occured") { cout << message << endl; exit(1); }
void trueOrDie(bool condition, string message) { if (!condition) die(message); }

