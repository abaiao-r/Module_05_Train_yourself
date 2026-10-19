/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimDashboard.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 01:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/06 12:55:36 by ctw03933         ###   ########.fr       */
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

	/** Store the static event list, for per-segment risk lookups. */
	void setEventInfo(const QVector<EventInfo> &events);

	/** Show final results after the simulation completes. */
	void showResults(const QStringList &results);

	/** Show aggregated multi-run statistics table. */
	void showMultiRunStats(const QVector<TrainStatRow> &stats,
						   int completedRuns);

	/** Reset to the idle / welcome state. */
	void clear();

  private:
	static QString fmtTime(double seconds);
	static QString progressBar(double fraction, int width);
	static QString speedStr(double speed_ms);

	/** Build an HTML table row with the box-drawing left/right borders. */
	static QString boxRow(const QString &html);
	static QString hRule(const QString &left, const QString &right, int n);
	/** Names of events bound to a from→to segment or its destination
		node - named explicitly rather than a bare number, since a
		vague risk figure with no context isn't actually transparent. */
	QStringList segmentEventNames(const QString &from,
								  const QString &to) const;
	/** Names of events bound to any remaining segment of path,
		starting at fromIndex. */
	QStringList remainingPathEventNames(const QStringList &path,
										size_t fromIndex) const;

	QVector<EventInfo> _events;
};

#endif
