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
	bool no_color{ false };
	bool quiet{ false };
} Global;

using regex_t = std::basic_regex<char>;
using syntax_t = std::regex_constants::syntax_option_type;

struct regex {
private:
	std::string _expr;
	syntax_t _mode;
	regex_t _regex;

	void update_regex()
	{
		_regex = regex_t{ _expr, _mode };
	}

public:
	regex(const syntax_t& mode) : _mode{ mode } { update_regex(); }
	regex(const std::string& expr, const syntax_t& mode) : _expr{ expr }, _mode{ mode } { update_regex(); }

	const std::string& expr() const
	{
		return _expr;
	}
	const syntax_t& mode() const
	{
		return _mode;
	}
	const regex_t& regexpr() const
	{
		return _regex;
	}

	void reset(const std::string& expr)
	{
		_expr = expr;
		update_regex();
	}
	void reset(const std::string& expr, const syntax_t& mode)
	{
		_expr = expr;
		_mode = mode;
		update_regex();
	}

	operator regex_t() const { return _regex; }
};


#define ORIGINAL_PROGRAM_NAME "str"

inline syntax_t getMode(std::vector<std::string> mode_args)
{
	if (mode_args.empty()) // use defaults:
		return regex_t::ECMAScript | regex_t::optimize | regex_t::collate;
	// else parse modes and ignore defaults
	syntax_t out_mode{ 0 };
	for (auto& mode_arg : mode_args) {
		const auto mode{ str::tolower(mode_arg) };
		if (mode == "basic")
			out_mode |= regex_t::basic;
		else if (mode == "extended")
			out_mode |= regex_t::extended;
		else if (mode == "ecmascript")
			out_mode |= regex_t::ECMAScript;
		else if (mode == "grep")
			out_mode |= regex_t::grep;
		else if (mode == "egrep")
			out_mode |= regex_t::egrep;
		else if (mode == "awk")
			out_mode |= regex_t::awk;
		else if (mode == "icase" || mode == "ignore-case")
			out_mode |= regex_t::icase;
		else if (mode == "nosubs")
			out_mode |= regex_t::nosubs;
		else if (mode == "optimize")
			out_mode |= regex_t::optimize;
		else if (mode == "collate")
			out_mode |= regex_t::collate;
		else
			throw make_exception("Failed to recognize syntax mode \"", mode_arg, "\"");
	}
	return out_mode;
}

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
			<< "  -S <MODE>  --syntax  <MODE>   Specify an alternative regular expression syntax, see-" << '\n'
			<< "             --strip   <CHARS>  Remove all characters listed in \"<CHARS>\" from the output." << '\n'
			<< "             --mode    <MODE>   -the \"MODES\" section for a list of possible modes." << '\n'
			<< "  -n         --no-color         Don't use colorized console output." << '\n'
			<< "  -h         --help             Print this help display to STDOUT, then exit." << '\n'
			<< "  -v         --version          Print the current version number to STDOUT, then exit." << '\n'
			<< "  -q         --quiet            Show only minimal output, useful for scripts." << '\n'
			<< "  -d <CHAR>  --delim <CHAR>     Set the input delimiter used to split inputs. Default is newline '\\n'." << '\n'
			<< '\n'
			<< "MODES:\n"
			<< "  You can set as many modes as you want by using multiple arguments. If no modes are specified, the modes marked-" << '\n'
			<< "  -with \"(Default)\" are used. Specifying any modes will disable all defaults." << '\n'
			<< "  Some modes may not work well together." << '\n'
			<< "  - basic                         Basic regular expression syntax that requires escaping special characters." << '\n'
			<< "  - extended                      Same as basic except special characters aren't escaped." << '\n'
			<< "  - ECMAScript       (Default)    Modified ECMAScript regular expression grammar." << '\n'
			<< "  - awk                           Same syntax as the 'awk' POSIX utility." << '\n'
			<< "  - grep                          Same syntax as the 'grep' POSIX utility." << '\n'
			<< "  - egrep                         Same syntax as the 'grep' POSIX utility with the '-E' flag." << '\n'
			<< "  - icase                         Ignores alphabetic case when performing matches." << '\n'
			<< "  - nosubs                        Treats all sub-expressions \"()\" as non-marking \"(?:)\" sub-expressions." << '\n'
			<< "  - collate          (Default)    Character ranges \"[a-z]\" are locale-sensitive." << '\n'
			<< "  - optimize         (Default)    Optimize regular expressions for speed at the cost of increased construction time." << '\n'
			;
	}
};

int main(const int argc, char** argv)
{
	try {
		std::cout << term::EnableANSI;
		opt::ParamsAPI2 args{ argc, argv, 'r', "regex", 'R', "replace", 'd', "delim", 'S', "syntax", "mode", "strip" };

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
		//Global.process_all = args.check_any<opt::Flag, opt::Option>('a', "all");
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
		auto inputs{ str::stringify_split(' ', args.typegetv_all<opt::Parameter>()) };
		// check STDIN for more input
		if (hasPendingDataSTDIN()) ///< read piped data
			for (std::string buf; std::getline(std::cin, buf, Global.delim); inputs.emplace_back(buf)) {}
		// throw if no inputs were found
		if (inputs.empty())
			throw make_exception("No inputs given!");

		// handle arguments:
		// get regex match expression:
		regex match{ "", getMode(args.typegetv_all<opt::Flag, opt::Option>('S', "syntax", "mode")) };
		if (const auto regex_arg{ args.typegetv_any<opt::Flag, opt::Option>('r', "regex") }; regex_arg.has_value())
			match.reset(regex_arg.value());

		// get regex replacement expression:
		std::optional<std::string> repl{ args.typegetv_any<opt::Flag, opt::Option>('R', "replace") };

		// get strip characters
		const std::string strip_chars{ [](const std::vector<std::string>& strip_args)-> std::string {
				std::string chars;
				if (!strip_args.empty())
					for (auto& it : strip_args)
						chars += it;
				return chars;
			}(args.typegetv_all<opt::Option>("strip"))
		};

		bool
			do_match{ !match.expr().empty() },
			do_repl{ repl.has_value() },
			do_strip{ !strip_chars.empty() };

		if (!do_match)
			throw make_exception("No operations specified!");

		// handle inputs:
		for (auto it{ inputs.begin() }; it != inputs.end(); ++it) {
			std::string out;

			// regex:
			if (do_match) {
				// regex replace:
				if (do_repl)
					out = std::regex_replace(*it, match.regexpr(), repl.value());
				// regex match:
				else
					out = std::regex_match(*it, match.regexpr());
			}

			// strip chars:
			if (do_strip)
				out = str::strip(out, strip_chars);

			// print result
			if (!out.empty())
				std::cout << out << std::endl;
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
