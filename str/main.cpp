

#include <hasPendingDataSTDIN.h>

#include <iostream>
#include <string>

inline std::string get_stdin()
{
	if (hasPendingDataSTDIN()) {
		std::string str;
		for (std::string buf{}; std::cin >> buf; str += buf) {}
		return str;
	}
	return{};
}

int main(const int argc, char** argv)
{

	return 0;
}
