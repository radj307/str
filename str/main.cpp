#include "version.h"

#include <hasPendingDataSTDIN.h>
#include <ParamsAPI2.hpp>
#include <TermAPI.hpp>
#include <envPath.hpp>

#include <iostream>
#include <string>

static struct {
	bool process_all{ false };
	bool no_color{ false };
	bool quiet{ false };
} Global;

#define ORIGINAL_PROGRAM_NAME "str"

struct Help {
	const std::string program_name;
	Help(const std::string& nameNoExt) : program_name{ nameNoExt } {}
	friend std::ostream& operator<<(std::ostream& os, const Help& help)
	{
		return os
			<< ORIGINAL_PROGRAM_NAME << " v" << STR_VERSION << '\n'
			<< "  Commandline string manipulation utility.\n"
			<< '\n'
			<< "USAGE:\n"
			<< "  " << help.program_name << " <MODE> [OPTIONS] [INPUT]...\n"
			<< '\n'
			<< "MODES:\n"
			<< "  -m  --match            Regex match mode" << '\n'
			<< "  -r  --replace          Regex replace mode" << '\n'
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -a  --all              Process all available matches instead of just the first one." << '\n'
			<< "  -n  --no-color         Don't use colorized console output." << '\n'
			<< "  -h  --help             Print this help display to STDOUT, then exit." << '\n'
			<< "  -v  --version          Print the current version number to STDOUT, then exit." << '\n'
			<< "  -q  --quiet            Hide most output, show the minimum amount of information possible. Useful for scripts." << '\n'
			;
	}
};

int main(const int argc, char** argv)
{
	try {
		std::cout << term::EnableANSI;
		opt::ParamsAPI2 args{ argc, argv };

		const auto& [myPath, myName] { env::PATH{}.resolve_split(argv[0]) };

		const std::string myNameNoExt{ [&myName]() -> std::string {
			std::string str{myName.generic_string()};
			if (const auto& pos{ str.find('.') }; pos < str.size())
				return str.substr(0, pos);
			return str;
		}() };

		// handle arguments
		Global.no_color = args.check_any<opt::Flag, opt::Option>('n', "no-color");
		Global.quiet = args.check_any<opt::Flag, opt::Option>('q', "quiet");
		Global.process_all = args.check_any<opt::Flag, opt::Option>('a', "all");

		if (args.check_any<opt::Flag, opt::Option>('h', "help")) {
			std::cout << Help(myNameNoExt);
		}
		if (args.check_any<opt::Flag, opt::Option>('v', "version")) {
			if (Global.quiet)
				std::cout << STR_VERSION << std::endl;
			else 
				std::cout << ORIGINAL_PROGRAM_NAME << " v" << STR_VERSION << std::endl;
		}

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << term::get_error(!Global.no_color) << ex.what() << std::endl;
		return -1;
	} catch (...) {
		std::cerr << term::get_crit(!Global.no_color) << "An undefined exception occurred!" << std::endl;
		return -1;
	}
}
