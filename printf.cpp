#include <iostream>
#include <cstring>
#include <sstream>

int main(int arg, char *args[])
{
	
}

%

template <typename... Ts>
void printf(const string& fmt, Ts... objs)
{
	ostringstream oss;
	char *cstr = fmt.c_str();
	char *tok = strtok(cstr, "%");
	while (tok != NULL)
	{
		
	}
	string[] parts = split(fmt);
	ostringstream ss;
	for (int i=0; i < parts.size(); i++) {
		ss << parts[i];
		ss << objs[i];
	}
}
