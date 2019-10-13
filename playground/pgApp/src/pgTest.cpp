#pragma warning(disable : 4244)
#include "argparse.h"

#include <windows.h>
//#include <shellapi.h>

#include <string>
#include <sstream>
#include <iostream>

static std::vector<char *> cmdLineToArgv(const char* cmd) {
	std::vector<char *> args;
	std::istringstream iss(cmd);

	std::string token;
	while (iss >> token) {
		char *arg = new char[token.size() + 1];
		copy(token.begin(), token.end(), arg);
		arg[token.size()] = '\0';
		args.push_back(arg);
	}
	
	return args;
}


void argparse_do() {
		//const auto* cmdLine = GetCommandLineW();
		//int nArgs;
		//LPWSTR* szArglist = CommandLineToArgvW(cmdLine, &nArgs);

		const auto* cmdLine = GetCommandLineA();
		std::vector<char *> args = cmdLineToArgv(cmdLine);

		//run as : [program name] "0 -c" abc - a 1 - sdfl --flag - v 1 2.7 3 4 9 8.12 87
		ArgumentParser parser("Argument parser example");

		parser.add_argument("-a", "an integer");
		parser.add_argument("-s", "an combined flag", true);
		parser.add_argument("-d", "an combined flag", true);
		parser.add_argument("-f", "an combined flag", true);
		parser.add_argument("--flag", "a flag");
		parser.add_argument("-v", "a vector", true);
		parser.add_argument("-l", "--long", "a long argument", false);

		int argc = (int)args.size();
		char** argv = args.data();

		try {
			parser.parse(argc, argv);
		}
		catch (const ArgumentParser::ArgumentNotFound& ex) {
			std::cout << ex.what() << std::endl;
			return;
		}

		if (parser.is_help()) {
			return;
		}
		std::cout << "a: " << parser.get<int>("a") << std::endl;
		std::cout << "flag: " << std::boolalpha << parser.get<bool>("flag")
			<< std::endl;
		std::cout << "d: " << std::boolalpha << parser.get<bool>("d") << std::endl;
		std::cout << "long flag: " << std::boolalpha << parser.get<bool>("l")
			<< std::endl;
		auto v = parser.getv<double>("v");
		std::cout << "v: ";

		// release
		for (std::vector<char*>::iterator it = args.begin(); it != args.end(); ++it) {
			char* p = *it;

			delete p;
		}

		args.clear();
	}