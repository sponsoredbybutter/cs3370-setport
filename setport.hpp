// requires <string>

enum msgidx: int
{
	BAD_ARG,
	MISSING_PORT,
	MISSING_ENV,
	BAD_PORT,
	BAD_ENV,
	LISTENING,
	MSG_COUNT // Not actually a message index
};

const int MSG_LEN = 256;

const std::string VERSION = "setport 1.2.1";

const std::string USAGE_FILE = "setport.usage_en.txt";
const std::string ABOUT_FILE = "setport.about_en.txt";
const std::string MSG_FILE = "setport.messages_en.txt";
