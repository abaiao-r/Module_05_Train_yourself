/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimDashboard.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 01:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMDASHBOARD_HPP
#define SIMDASHBOARD_HPP

#include <QTextEdit>
#include <QVector>
#include "SimulationWorker.hpp"

/**
 * A styled read-only panel that mirrors the terminal --animate dashboard
 * inside the Qt GUI.  Shows a box-drawing-style live view of all trains
 * with progress bars, speed, segment, and timing info.
 *
 * Usage: call update() from the main-thread simulation tick handler.
 *        call clear() when the simulation ends or is reset.
 */
class SimDashboard : public QTextEdit
{
	Q_OBJECT

  public:
	explicit SimDashboard(QWidget *parent = nullptr);

	/** Render one frame of the dashboard from a tick snapshot. */
	void update(double simTime, const QVector<TrainSnapshot> &snapshots);

	/** Show final results after the simulation completes. */
	void showResults(const QStringList &results);

	/** Reset to the idle / welcome state. */
	void clear();

  private:
	static QString fmtTime(double seconds);
	static QString progressBar(double fraction, int width);
	static QString speedStr(double speed_ms);

	/** Build an HTML table row with the box-drawing left/right borders. */
	static QString boxRow(const QString &html);
	static QString hRule(const QString &left, const QString &right, int n);
};

#endif
