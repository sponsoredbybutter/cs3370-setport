#include <iostream>
#include <cstdlib>
#include <sstream>
#include <vector>
#include <tuple>
#include <stdexcept>
#include <regex>
#include <fstream>
#include <wait.h>
#include "setport.hpp"
using namespace std;

struct result
{
	int status;
	string stderr;
	string stdout;
};

string mktmp();
string readFile(string);
void delFile(string);
vector<string> test(string, result);
template <typename T> string fail_msg(string, T, T);

string usage;
string msgs[MSG_COUNT];

// Give a bad return value and talk on cerr if tests don't pass.
int main(int argc, char *args[])
{
	string lang(args[1]);
	regex re("_en");
	usage = readFile(regex_replace(USAGE_FILE, re, "_" + lang));
	ifstream msgstream(regex_replace(MSG_FILE, re, "_" + lang));
	char msgbuf[MSG_LEN + 1];
	for (int i = 0; i < MSG_COUNT; ++i)
	{
		if (msgstream.eof() || msgstream.fail() || msgstream.bad())
		{
			cerr << "cannot read file: " << MSG_FILE << endl;
			return 2;
		}
		msgstream.getline(msgbuf, MSG_LEN);
		msgs[i] = string(msgbuf);
	}
	
	int passed = 0;
	
	array<tuple<string, result>, 18> testCases = {
		// Positive tests
		make_tuple("LANGUAGE="+lang+" ./setport", result{0, "", usage}),
		make_tuple("LANGUAGE="+lang+" ./setport -h", result{0, "", usage}),
		make_tuple("LANGUAGE="+lang+" ./setport --help", result{0, "", usage}),
		make_tuple("LANGUAGE="+lang+" ./setport -p 4040", result{0, "", msgs[LISTENING] + "4040"}),
		make_tuple("LANGUAGE="+lang+" ./setport --port 4040", result{0, "", msgs[LISTENING] + "4040"}),
		
		// Negative tests
		make_tuple("LANGUAGE="+lang+" ./setport help", result{1, msgs[BAD_ARG] + "\n" + usage, ""}),
		make_tuple("LANGUAGE="+lang+" ./setport -help", result{1, msgs[BAD_ARG] + "\n" + usage, ""}),
		make_tuple("LANGUAGE="+lang+" ./setport --h", result{1, msgs[BAD_ARG] + "\n" + usage, ""}),
		make_tuple("LANGUAGE="+lang+" ./setport -h --help", result{1, msgs[BAD_ARG] + "\n" + usage, ""}),
		make_tuple("LANGUAGE="+lang+" ./setport -hs", result{1, msgs[BAD_ARG] + "\n" + usage, ""}),
		make_tuple("LANGUAGE="+lang+" ./setport -p --port 9", result{1, msgs[BAD_PORT] + "\n" + usage, ""}),
		make_tuple("LANGUAGE="+lang+" ./setport -p 77 33", result{1, msgs[BAD_ARG] + "\n" + usage, ""}),
		make_tuple("LANGUAGE="+lang+" ./setport -p -21", result{1, msgs[BAD_PORT] + "\n" + usage, ""}),
		make_tuple("LANGUAGE="+lang+" ./setport -p 0", result{1, msgs[BAD_PORT] + "\n" + usage, ""}),
		make_tuple("LANGUAGE="+lang+" ./setport --port", result{1, msgs[MISSING_PORT] + "\n" + usage, ""}),
		make_tuple("LANGUAGE="+lang+" ./setport -p 90642", result{1, msgs[BAD_PORT] + "\n" + usage, ""}),
		make_tuple("LANGUAGE="+lang+" ./setport -x 45321", result{1, msgs[BAD_ARG] + "\n" + usage, ""}),
		make_tuple("LANGUAGE="+lang+" ./setport -P 714", result{1, msgs[BAD_ARG] + "\n" + usage, ""})
	};
	
	for (int i=0; i<testCases.size(); i++)
	{
		tuple<string, result> tc = testCases[i];
		vector<string> msgs = test(get<0>(tc), get<1>(tc));
		if (msgs.empty()) passed++;
		else
		{
			cerr << "\nTest case " << i << ": " << get<0>(tc) << endl;
			for (string msg : msgs)
			{
				cerr << msg << endl;
			}
		}
	}
	
	cerr << "Passed " << passed << " of " << testCases.size()
			<< " test(s)" << endl;
	return testCases.size() - passed;
}

string mktmp()
{
	char fname[L_tmpnam];
	
	char *p = tmpnam(NULL);
	if (p == NULL) throw new runtime_error("cannot get temp file name");
	FILE *f = fopen(p, "wt");
	if (f == NULL) throw new runtime_error("cannot create temp file");
	if (fclose(f)) throw new runtime_error("cannot close temp file");
	return string(p);
}

string readFile(string fname)
{
	const size_t BUFLEN = 100;
	char buf[BUFLEN];
	const char *cname = fname.c_str();
	FILE *f = fopen(cname, "r");
	if (f == NULL) throw new runtime_error("cannot open temp file");
	
	string data = "";
	do
	{
		size_t n = fread(buf, sizeof(char), BUFLEN, f);
		if (ferror(f)) throw new runtime_error("cannot read temp file");
		data.append(buf, n*sizeof(char));
	}
	while (!feof(f));
	
	return data;
}

void delFile(string fname)
{
	if (remove(fname.c_str()))
	{
		throw new runtime_error("cannot delete temp file");
	}
}

vector<string> test(string cmd, result expected)
{
	string errname = mktmp();
	string outname = mktmp();
	string pipecmd = cmd + " >" + outname + " 2>" + errname;
	const char *ccmd = pipecmd.c_str();
	int status = system(ccmd);
	status = WEXITSTATUS(status);
	
	string err = readFile(errname);
	if (err.back() == '\n') err.pop_back();
	string out = readFile(outname);
	if (out.back() == '\n') out.pop_back();
	
	vector<string> msgs;
	if (status != expected.status)
	{
		msgs.push_back(fail_msg("status", expected.status, status));
	}
	if (err != expected.stderr)
	{
		msgs.push_back(fail_msg("err", expected.stderr, err));
	}
	if (out != expected.stdout)
	{
		msgs.push_back(fail_msg("out", expected.stdout, out));
	}
	
	delFile(errname);
	delFile(outname);
	
	return msgs;
}

template <typename T>
string fail_msg(string label, T expected, T actual)
{
	ostringstream msg;
	msg << "Expected " << label << ":\n" << expected << "\nGot:\n" << actual;
	return msg.str();
}