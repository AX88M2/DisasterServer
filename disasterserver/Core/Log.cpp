#include "Log.hpp"

#include <boost/algorithm/string.hpp>
#include <iomanip>
#include <sstream>
#include <syncstream>

#include "Constansts.hpp"

using namespace DisasterServer;

void Logger::write(LogLevel level, std::string message, std::source_location location) {
	using namespace std::chrono;

	std::stringstream ss;

	const time_t now = std::time(nullptr);

	std::tm local{};
#ifdef _WIN32
	localtime_s(&local, &now);
#else
	localtime_r(&now, &local);
#endif

	ss << std::put_time(&local, "%d.%m.%Y %T");
	ss << " " << TerminalColors::light_gray << "[" << std::this_thread::get_id() << "]" << TerminalColors::reset << " ";

	switch (level) {
		case LogLevel::Debug: ss << TerminalColors::cyan << "[Debug]" << TerminalColors::reset; break;
		case LogLevel::Info: ss << TerminalColors::green << "[Info]" << TerminalColors::reset; break;
		case LogLevel::Warning: ss << TerminalColors::yellow << "[Warn]" << TerminalColors::reset; break;
		case LogLevel::Error: ss << TerminalColors::light_red << "[Error]" << TerminalColors::reset; break;
	}
	ss << " " << TerminalColors::light_gray << std::format("({}:{})", location.file_name(), location.line()) << TerminalColors::reset;

	replaceColor(message);

	ss << " " << message << TerminalColors::reset;

	std::osyncstream(std::cout) << ss.str() << std::endl;
}

void Logger::replaceColor(std::string &msg) {
	boost::algorithm::replace_all(msg, CLRCODE_RED, TerminalColors::red);
	boost::algorithm::replace_all(msg, CLRCODE_GRN, TerminalColors::green);
	boost::algorithm::replace_all(msg, CLRCODE_PUR, TerminalColors::light_magenta);
	boost::algorithm::replace_all(msg, CLRCODE_BLU, TerminalColors::light_blue);
	boost::algorithm::replace_all(msg, CLRCODE_GRA, TerminalColors::light_gray);
	boost::algorithm::replace_all(msg, CLRCODE_YLW, TerminalColors::yellow);
	boost::algorithm::replace_all(msg, CLRCODE_ORG, TerminalColors::light_yellow);
	boost::algorithm::replace_all(msg, CLRCODE_RST, TerminalColors::reset);
}
