#include "version.h"

#include <hasPendingDataSTDIN.h>
#include <ParamsAPI2.hpp>
#include <TermAPI.hpp>
#include <envPath.hpp>
#include <str.hpp>

#include <iostream>
#include <string>
#include <algorithm>
#include <regex>

static struct {
	char delim{ '\n' };
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
			<< "OPTIONS:\n"
			<< "  -r <EXPR>  --regex   <EXPR>   Provide a regular expression." << '\n'
			<< "  -R <REPL>  --replace <REPL>   Provide a regular expression replacement string." << '\n'
			<< "  -a         --all              Process all available matches instead of just the first one." << '\n'
			<< "  -n         --no-color         Don't use colorized console output." << '\n'
			<< "  -h         --help             Print this help display to STDOUT, then exit." << '\n'
			<< "  -v         --version          Print the current version number to STDOUT, then exit." << '\n'
			<< "  -q         --quiet            Show only minimal output, useful for scripts." << '\n'
			<< "  -d <CHAR>  --delim <CHAR>     Set the input delimiter used to split inputs. Default is newline '\\n'." << '\n'
			;
	}
};

inline std::vector<std::string> merge_and_split_params(const std::vector<std::string>& params)
{
	std::stringstream buffer;
	for (auto it{ params.begin() }; it != params.end(); ++it) {
		buffer << *it;
		if (std::distance(it, params.end()) > 1)
			buffer << ' ';
	}
	std::vector<std::string> vec;
//	vec.reserve(std::count(buffer.beg(), buffer.end(), Global.delim));
	str::stringify_split(' ', "Hello World How Are You!");
	return vec;
}

int main(const int argc, char** argv)
{
	try {
		std::cout << term::EnableANSI;
		opt::ParamsAPI2 args{ argc, argv, 'r', "regex", 'R', "replace", 'd', "delim" };

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
		if (const auto delim{ args.typegetv_any<opt::Flag, opt::Option>('d', "delim") }; delim.has_value()) {
			if (const auto& val{ delim.value() }; val.size() == 1ull)
				Global.delim = val.front();
			else throw make_exception("\"", val, "\" isn't a valid delimiter, delimiters must be one character in length.");
		}
		// handle blocking arguments
		if (args.check_any<opt::Flag, opt::Option>('h', "help")) {
			std::cout << Help(myNameNoExt);
			return 0;
		}
		else if (args.check_any<opt::Flag, opt::Option>('v', "version")) {
			if (Global.quiet)
				std::cout << STR_VERSION << std::endl;
			else
				std::cout << ORIGINAL_PROGRAM_NAME << " v" << STR_VERSION << std::endl;
			return 0;
		}
		// get input arguments
		auto input{ args.typegetv_all<opt::Parameter>() };
		// check STDIN for more input
		if (hasPendingDataSTDIN()) ///< read piped data
			for (std::string buf; std::getline(std::cin, buf, Global.delim); input.emplace_back(buf)) {}

		if (input.empty())
			throw make_exception("No inputs given!");

		if (const auto regex_arg{ args.typegetv_any<opt::Flag, opt::Option>('r', "regex") }; regex_arg.has_value()) {
			std::basic_regex<char> expr{ regex_arg.value() };
			if (const auto replace_arg{ args.typegetv_any<opt::Flag, opt::Option>('R', "replace") }; replace_arg.has_value()) {
				std::string repl{ replace_arg.value() };
				if (Global.process_all) {
					for (auto& it : input)
						std::cout << std::regex_replace(it, expr, repl) << '\n';
				}
				else std::cout << std::regex_replace(input.front(), expr, repl) << '\n';
			}
			else { // match
				if (Global.process_all) {
					for (auto& it : input)
						std::cout << std::regex_match(it, expr) << '\n';
				}
				else std::cout << std::regex_match(input.front(), expr) << '\n';
			}
		}
		else throw make_exception("No mode specified! Nothing to do.");

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << term::get_error(!Global.no_color) << ex.what() << std::endl;
		return -1;
	} catch (...) {
		std::cerr << term::get_crit(!Global.no_color) << "An undefined exception occurred!" << std::endl;
		return -1;
	}
}
