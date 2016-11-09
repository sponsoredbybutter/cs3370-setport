#include <iostream>
#include <fstream>
#include <stdexcept>
#include <string>
#include <queue>
#include <regex>
#include "setport.hpp"
using namespace std;

string usage;
string license;

string msgs[MSG_COUNT];

const int PORT_MIN = 1;
const int PORT_MAX = 65535;
const string DEFAULT_ENV = "PORT";
const string ENGLISH_639 = "en";

enum localestatus
{
	NO_LOCALE,
	BAD_LOCALE,
	GOOD_LOCALE
};


struct parserState{ parserState (*fn)(queue<string>&, int&); };

parserState getFlag(queue<string>&, int&);
parserState getPort(queue<string>&, int&);
parserState showHelp(queue<string>&, int&);
parserState showPort(queue<string>&, int&);
parserState showAbout(queue<string>&, int&);
parserState showVersion(queue<string>&, int&);
parserState getEnvName(queue<string>&, int&);
const parserState STOP = { NULL };

string readFile(string);
tuple<localestatus, string> getLanguage(string);
void translate(string);

const string localeVars[] = {
	"LANGUAGE",
	"LC_ALL",
	"LC_MESSAGES",
	"LANG"
};

int port;

int main(int argc, char *args[])
{
	string lang = ENGLISH_639;
	for (const string var : localeVars)
	{
		tuple<localestatus, string> langresult = getLanguage(var);
		
		if (get<0>(langresult) == BAD_LOCALE)
		{
			// TODO this needs to set the status to 1
			string val = get<1>(langresult);
			cerr << "Bad language code in environment variable ";
			cerr << var << "=" << val << ". Using English." << endl;
			break;
		}
		else if (get<0>(langresult) == GOOD_LOCALE)
		{
			lang = get<1>(langresult);
			break;
		}
	}
	
	try
	{
		translate(lang);
	}
	catch (const runtime_error& e1)
	{
		cerr << "Missing " << lang << " translation files.";
		cerr << " Using English." << endl;
		try
		{
			translate(ENGLISH_639);
		}
		catch (runtime_error& e2)
		{
			cerr << e2.what() << endl;
			return 2;
		}
	}
	
	int status = 0;
	queue<string> argq;
	for (int i=1; i < argc; ++i)
	{
		argq.push(string(args[i]));
	}
	
	// Long, deeply nested if-else chains are ugly and flagile.
	// Long live the FSM.
	// The FSM is an enemy to all mankind
	// May the FSM be covered in meaty marinara and fed to the hounds
	parserState state = { getFlag };
	while (state.fn != NULL)
	{
		state = state.fn(argq, status);
	}
	
	return status;
}

parserState getFlag(queue<string>& args, int& status)
{
	if (args.empty())
	{
		return { showHelp };
	}
	
	string arg = args.front();
	args.pop();
	
	if (arg == "-p" || arg == "--port")
	{
		return { getPort };
	}
	else if (arg == "-h" || arg == "--help" || arg == "-?")
	{
		return { showHelp };
	}
	else if (arg == "-!" || arg == "--about")
	{
		return { showAbout };
	}
	else if (arg == "-v" || arg == "--version")
	{
		return { showVersion };
	}
	else if (arg == "-e" || arg == "--environment")
	{
		return { getEnvName };
	}
	else
	{
		cerr << msgs[BAD_ARG] << endl << usage << endl;
		status = 1;
		return STOP;
	}
}

parserState getPort(queue<string>& args, int& status)
{
	if (args.empty())
	{
		cerr << msgs[MISSING_PORT] << endl << usage << endl;
		status = 1;
		return STOP;
	}
	
	string arg = args.front();
	args.pop();
	
	try
	{
		// TODO factor this out and reuse it in getEnv or whatever
		size_t i;
		port = stoi(arg, &i);
		if (port < PORT_MIN || port > PORT_MAX) throw out_of_range("");
		if (i < arg.size()) throw invalid_argument("");
		return { showPort };
	}
	catch (invalid_argument& e)
	{
		// Use case is -p -e
		if (arg == "-e" || arg == "--environment") return { getEnvName };
		
		cerr << msgs[BAD_PORT] << endl << usage << endl;
		status = 1;
		return STOP;
	}
	catch (out_of_range& e)
	{
		cerr << msgs[BAD_PORT] << endl << usage << endl;
		status = 1;
		return STOP;
	}
}

parserState showHelp(queue<string>& args, int& status)
{
	if (!args.empty())
	{
		cerr << msgs[BAD_ARG] << endl << usage << endl;
		status = 1;
	}
	else
	{
		cout << usage << endl;
	}
	return STOP;
}

parserState showAbout(queue<string>& args, int& status)
{
	if (!args.empty())
	{
		cerr << msgs[BAD_ARG] << endl << usage << endl;
		status = 1;
	}
	else
	{
		cout << VERSION << endl << license << endl;
	}
	return STOP;
}

parserState showVersion(queue<string>& args, int& status)
{
	if (!args.empty())
	{
		cerr << msgs[BAD_ARG] << endl << usage << endl;
		status = 1;
	}
	else
	{
		cout << VERSION << endl;
	}
	return STOP;
}

parserState getEnvName(queue<string>&args, int& status)
{
	string varName;
	if (args.empty()) varName = DEFAULT_ENV;
	else
	{
		varName = args.front();
		args.pop();
	}
	
	char *val = getenv(varName.c_str());
	if (val == NULL)
	{
		cerr << msgs[BAD_ENV] << endl << usage << endl;
		status = 1;
		return STOP;
	}
	
	try
	{
		port = stoi(string(val));
		if (port < PORT_MIN || port > PORT_MAX) throw out_of_range("");
	}
	catch (invalid_argument& e)
	{
		cerr << msgs[BAD_PORT] << endl << usage << endl;
		status = 1;
		return STOP;
	}
	catch (out_of_range& e)
	{
		cerr << msgs[BAD_PORT] << endl << usage << endl;
		status = 1;
		return STOP;
	}
	return { showPort };
}

parserState showPort(queue<string>& args, int& status)
{
	if (!args.empty())
	{
		cerr << msgs[BAD_ARG] << endl << usage << endl;
		status = 1;
	}
	else
	{
		cout << msgs[LISTENING] << port << endl;
	}
	return STOP;
}

string readFile(string fname)
{
	const size_t BUFLEN = 100;
	char buf[BUFLEN];
	const char *cname = fname.c_str();
	FILE *f = fopen(cname, "r");
	if (f == NULL) throw runtime_error("cannot read file: " + fname);
	
	string data = "";
	do
	{
		size_t n = fread(buf, sizeof(char), BUFLEN, f);
		if (ferror(f)) throw runtime_error("cannot read file: " + fname);
		data.append(buf, n*sizeof(char));
	}
	while (!feof(f));
	
	return data;
}

tuple<localestatus, string> getLanguage(string localeVar)
{
	char *env = getenv(localeVar.c_str());
	if (env == NULL) return make_tuple(NO_LOCALE, string(""));
	string senv(env);
	if (senv == "C" || senv == "C.UTF-8" || senv == "")
	{
		return make_tuple(NO_LOCALE, "");
	}
	regex re(R"EOF(([a-z]{2})(_[A-Z]{2})?(\.UTF-8)?)EOF");
	smatch m;
	if (!regex_match(senv, m, re)) return make_tuple(BAD_LOCALE, string(""));
	return make_tuple(GOOD_LOCALE, m[1]);
}

void translate(string lang)
{
	regex re("_" + ENGLISH_639);
	usage = readFile(regex_replace(USAGE_FILE, re, "_" + lang));
	license = readFile(regex_replace(ABOUT_FILE, re, "_" + lang));
	
	ifstream msgstream(regex_replace(MSG_FILE, re, "_" + lang));
	char msgbuf[MSG_LEN + 1];
	for (int i = 0; i < MSG_COUNT; ++i)
	{
		if (msgstream.eof() || msgstream.fail() || msgstream.bad())
		{
			throw runtime_error("error reading messages");
		}
		msgstream.getline(msgbuf, MSG_LEN);
		msgs[i] = string(msgbuf);
	}
}
