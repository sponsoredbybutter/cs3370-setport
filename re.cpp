#include <iostream>
#include <string>
#include <regex>
using namespace std;

int main(int argc, char *args[])
{
	string res(R"EOF(([a-z]{2})(_[A-Z]{2})?(\.UTF-8)?)EOF");
	cmatch m;
	cout << res << endl;
	regex re(res, regex::extended);
	bool valid = regex_match(args[1], m, re);
	cout << valid << endl;
	if (valid) cout << m[1] << endl;
	return 0;
}