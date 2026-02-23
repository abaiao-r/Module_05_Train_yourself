/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   TerminalAnimDisplay.hpp                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:00:00 by ctw03933          #+#    #+#             */
/*   Updated: 2026/02/22 18:19:38 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef TERMINALANIMDISPLAY_HPP
#define TERMINALANIMDISPLAY_HPP

#include <string>
#include <vector>

struct TrainState;

/**
 * Renders a live-updating terminal display of all trains using
 * ANSI escape codes.  Intended to be used as a per-tick callback
 * via Simulation::setAnimCallback().
 *
 * Design note: This is a pure rendering utility with no dependency
 * on the Simulation class itself — it only reads TrainState data,
 * keeping it fully decoupled and testable.
 */
class TerminalAnimDisplay
{
  public:
	TerminalAnimDisplay();
	~TerminalAnimDisplay();
	TerminalAnimDisplay(const TerminalAnimDisplay &) = delete;
	TerminalAnimDisplay &operator=(const TerminalAnimDisplay &) = delete;

	/** Render one frame.  Called once per simulation tick. */
	void render(double simTime, const std::vector<TrainState> &states);

	/** Restore terminal state (show cursor, scroll region). */
	void cleanup();

  private:
	bool _firstFrame;
	int _lastLineCount;

	static std::string fmtTime(double seconds);
	static std::string progressBar(double fraction, int width);
	static std::string speedStr(double speed_ms);
};

#endif
