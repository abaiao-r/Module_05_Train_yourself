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

#include <QSet>
#include <QTextEdit>
#include <QVector>
#include "SimulationWorker.hpp"

/**
 * A styled read-only panel that shows a live card per train
 * (status chip, progress bar, speed, line, risks) plus result tables.
 *
 * Usage: call update() from the main-thread simulation tick handler.
 *        call clear() when the simulation ends or is reset.
 */
class SimDashboard : public QTextEdit
{
	Q_OBJECT

  public:
	explicit SimDashboard(QWidget *parent = nullptr);

	void update(double simTime, const QVector<TrainSnapshot> &snapshots);

	/** Store the static event list, for per-segment risk lookups. */
	void setEventInfo(const QVector<EventInfo> &events);

	void showResults(const QStringList &results);

	/** Post-run table (origin/destination, depart, arrive, expected,
		trip, delay, cause) and one clickable card per event. */
	void showAnalysis(const QVector<TrainRunInfo> &trains,
					  const QVector<EventOutcome> &outcomes);
	/** Which trains/events are currently drawn on the graph. */
	void setSelection(const QSet<QString> &trains,
					  const QSet<QString> &events);
	/** Show aggregated multi-run statistics table. */
	void showCompare(const QVector<CompareRow> &rows,
					 const QVector<CompareEventRow> &events, int runsEach);
	void showMultiRunStats(const QVector<TrainStatRow> &stats,
						   const MultiRunTotals &totals, int completedRuns);
	void clear();

  signals:
	void trainClicked(const QString &key);
	void eventClicked(const QString &name);

  protected:
	void mouseReleaseEvent(QMouseEvent *e) override;
	void mouseMoveEvent(QMouseEvent *e) override;

  private:
	static QString fmtTime(double seconds);
	static QString speedStr(double speed_ms);
	static QString chip(const QString &text, const QString &color);
	static QString heading(const QString &title, const QString &right);
	static QString card(const QString &inner, const QString &accent);
	static QString bar(double fraction, const QString &color);
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
	QVector<TrainRunInfo> _runTrains;
	QVector<EventOutcome> _outcomes;
	QSet<QString> _selTrains;
	QSet<QString> _selEvents;
	bool _analysisShown = false;
	void renderAnalysis();
};

#endif
