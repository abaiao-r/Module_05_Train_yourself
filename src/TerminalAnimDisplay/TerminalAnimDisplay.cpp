/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TerminalAnimDisplay.cpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:00:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 12:26:07 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "TerminalAnimDisplay.hpp"
#include "Simulation.hpp"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>

/* ---- ANSI helpers ---- */
static const char *CSI       = "\033[";
static const char *CLEAR     = "\033[2J";
static const char *HOME      = "\033[H";
static const char *HIDE_CUR  = "\033[?25l";
static const char *SHOW_CUR  = "\033[?25h";
static const char *BOLD      = "\033[1m";
static const char *RESET     = "\033[0m";
static const char *GREEN     = "\033[32m";
static const char *YELLOW    = "\033[33m";
static const char *CYAN      = "\033[36m";
static const char *RED       = "\033[31m";
static const char *DIM       = "\033[2m";
static const char *WHITE     = "\033[37m";

/* ---- Construction / Destruction ---- */
TerminalAnimDisplay::TerminalAnimDisplay()
	: _firstFrame(true), _lastLineCount(0) {}

TerminalAnimDisplay::~TerminalAnimDisplay() { cleanup(); }

void TerminalAnimDisplay::cleanup()
{
	std::cout << SHOW_CUR << std::flush;
}

/* ---- Helpers ---- */
std::string TerminalAnimDisplay::fmtTime(double seconds)
{
	int total = static_cast<int>(seconds);
	if (total < 0)
		total = 0;
	int h = total / 3600;
	int m = (total % 3600) / 60;
	int s = total % 60;
	std::ostringstream oss;
	oss << std::setfill('0') << std::setw(2) << h << ":"
		<< std::setw(2) << m << ":"
		<< std::setw(2) << s;
	return oss.str();
}

std::string TerminalAnimDisplay::progressBar(double fraction, int width)
{
	if (fraction < 0.0)
		fraction = 0.0;
	if (fraction > 1.0)
		fraction = 1.0;
	int filled = static_cast<int>(fraction * width);
	std::string bar;
	bar.reserve(width + 2);
	for (int i = 0; i < width; i++)
	{
		if (i < filled)
			bar += "\xe2\x96\x88"; // █ (UTF-8 full block)
		else if (i == filled)
			bar += "\xe2\x96\x93"; // ▓
		else
			bar += "\xe2\x96\x91"; // ░
	}
	return bar;
}

std::string TerminalAnimDisplay::speedStr(double speed_ms)
{
	double kmh = speed_ms * 3.6;
	std::ostringstream oss;
	oss << std::fixed << std::setprecision(1)
		<< std::setw(6) << kmh << " km/h";
	return oss.str();
}

/* ---- Main render ---- */

/* Box inner width: 62 visible characters between the two ║ borders. */
static const int BOX_W = 62;

/**
 * Compute the visible (terminal-column) width of a string by
 * stripping ANSI CSI escape sequences and counting UTF-8 codepoints.
 */
static int visLen(const std::string &s)
{
	int len = 0;
	size_t i = 0;
	while (i < s.size())
	{
		if (s[i] == '\033')
		{
			i++;
			if (i < s.size() && s[i] == '[')
			{
				i++;
				while (i < s.size()
					   && !((s[i] >= 'A' && s[i] <= 'Z')
							|| (s[i] >= 'a' && s[i] <= 'z')))
					i++;
				if (i < s.size())
					i++;
			}
			continue;
		}
		unsigned char c = static_cast<unsigned char>(s[i]);
		if ((c & 0x80) == 0)
			i += 1;
		else if ((c & 0xE0) == 0xC0)
			i += 2;
		else if ((c & 0xF0) == 0xE0)
			i += 3;
		else
			i += 4;
		len++;
	}
	return len;
}

/** Pad content to BOX_W visible chars, wrap with ║ borders. */
static std::string boxLine(const std::string &content)
{
	int pad = BOX_W - visLen(content);
	if (pad < 0)
		pad = 0;
	return " \xe2\x95\x91" + content + std::string(pad, ' ')
		 + "\xe2\x95\x91\n";
}

/** Generate a horizontal rule of `n` ═ characters. */
static std::string hRule(char left, char right, int n)
{
	// left/right are ignored — we build from raw UTF-8.
	(void)left;
	(void)right;
	std::string s = " ";
	if (left == 'T')
		s += "\xe2\x95\x94"; // ╔
	else if (left == 'M')
		s += "\xe2\x95\xa0"; // ╠
	else
		s += "\xe2\x95\x9a"; // ╚
	for (int i = 0; i < n; i++)
		s += "\xe2\x95\x90"; // ═
	if (right == 'T')
		s += "\xe2\x95\x97"; // ╗
	else if (right == 'M')
		s += "\xe2\x95\xa3"; // ╣
	else
		s += "\xe2\x95\x9d"; // ╝
	s += "\n";
	return s;
}

void TerminalAnimDisplay::render(double simTime,
								 const std::vector<TrainState> &states)
{
	if (_firstFrame)
	{
		std::cout << HIDE_CUR << CLEAR;
		_firstFrame = false;
	}

	std::cout << HOME;
	std::ostringstream out;

	out << BOLD << CYAN << hRule('T', 'T', BOX_W) << RESET;
	out << boxLine(std::string(BOLD) + CYAN
		+ "      " + WHITE + "Train Simulation \xe2\x80\x94 Live View"
		+ RESET);
	out << BOLD << CYAN << hRule('M', 'M', BOX_W) << RESET;
	out << boxLine(std::string("  ") + BOLD + WHITE + "Clock: "
		+ fmtTime(simTime) + RESET);
	out << BOLD << CYAN << hRule('M', 'M', BOX_W) << RESET;

	int lineCount = 5;

	for (size_t i = 0; i < states.size(); i++)
	{
		const auto &s = states[i];
		const auto &path = s.train->getPath();

		/* ── Train header line ── */
		{
			std::ostringstream ln;
			ln << "  ";
			if (s.arrived)
				ln << GREEN << "\xe2\x97\x8f ";
			else if (!s.departed)
				ln << DIM << "\xe2\x97\x8b ";
			else
				ln << YELLOW << "\xe2\x97\x8f ";

			ln << BOLD << s.train->getName()
			   << s.train->getId() << RESET;
			ln << DIM << "  (" << s.train->getDepartureStation()
			   << " \xe2\x86\x92 " << s.train->getArrivalStation()
			   << ")" << RESET;

			if (s.arrived)
				ln << "  " << GREEN << "[Arrived]" << RESET;
			else if (!s.departed)
				ln << "  " << DIM << "[Waiting]" << RESET;
			else if (s.stopTimer > 0.0)
				ln << "  " << RED << "[Stopped]" << RESET;
			else
				ln << "  " << YELLOW << "[Running]" << RESET;

			out << boxLine(ln.str());
			lineCount++;
		}

		if (!s.departed)
		{
			out << boxLine(std::string("  ") + DIM
				+ "      Departs at "
				+ fmtTime(s.train->getDepartureTime()) + RESET);
			lineCount++;
		}
		else if (s.arrived)
		{
			std::ostringstream ln;
			ln << "      Travel time: " << GREEN
			   << fmtTime(s.timeSinceDepart) << RESET << "  Delay: ";
			if (s.train->getTotalDelay() > 0.0)
				ln << RED << fmtTime(s.train->getTotalDelay()) << RESET;
			else
				ln << GREEN << "none" << RESET;
			out << boxLine(std::string("  ") + ln.str());
			lineCount++;
		}
		else
		{
			if (path.size() >= 2 && s.segmentIndex + 1 < path.size())
			{
				out << boxLine(std::string("      ") + DIM
					+ "Segment: " + RESET
					+ path[s.segmentIndex]->getName()
					+ " \xe2\x86\x92 "
					+ path[s.segmentIndex + 1]->getName());
				lineCount++;
			}

			std::string action;
			if (s.stopTimer > 0.0)
				action = "Stopped";
			else if (s.speed_ms < 0.1)
				action = "Starting";
			else
				action = "Moving";
			out << boxLine(std::string("      ") + DIM + "Speed:   "
				+ RESET + speedStr(s.speed_ms)
				+ "  (" + action + ")");
			lineCount++;

			double segCount = (path.size() > 1)
				? static_cast<double>(path.size() - 1) : 1.0;
			double frac = static_cast<double>(s.segmentIndex) / segCount;
			if (path.size() >= 2 && s.segmentIndex + 1 < path.size())
				frac += (1.0 / segCount) * 0.5;
			if (frac > 1.0)
				frac = 1.0;
			int pct = static_cast<int>(frac * 100.0);
			std::ostringstream pctOss;
			pctOss << std::setw(3) << pct << "%";
			out << boxLine(std::string("      ") + CYAN
				+ progressBar(frac, 40) + RESET + " "
				+ BOLD + pctOss.str() + RESET);
			lineCount++;

			out << boxLine(std::string("      ") + DIM + "Elapsed: "
				+ fmtTime(s.timeSinceDepart) + RESET);
			lineCount++;
		}

		if (i + 1 < states.size())
		{
			std::string line;
			for (int j = 0; j < 57; j++)
				line += "\xe2\x94\x80";
			out << boxLine(std::string("  ") + DIM + line + RESET);
			lineCount++;
		}
	}

	out << BOLD << CYAN << hRule('B', 'B', BOX_W) << RESET;
	lineCount++;

	for (int i = lineCount; i < _lastLineCount; i++)
		out << CSI << "2K\n";
	_lastLineCount = lineCount;

	std::cout << out.str() << std::flush;
	std::this_thread::sleep_for(std::chrono::microseconds(50000));
}
