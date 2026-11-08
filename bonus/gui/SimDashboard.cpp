/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimDashboard.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 01:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/06 18:06:54 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "SimDashboard.hpp"
#include <QHash>
#include <QMouseEvent>
#include <QRegularExpression>
#include <QScrollBar>
#include <QUrl>
#include <algorithm>
#include <cmath>

static const char *C_PANEL  = "#0f172a"; /* panel background           */
static const char *C_CARD   = "#172033"; /* card background            */
static const char *C_TITLE  = "#e2e8f0";
static const char *C_DIM    = "#94a3b8";
static const char *C_GREEN  = "#6ee7b7";
static const char *C_YELLOW = "#fbbf24";
static const char *C_RED    = "#f87171";
static const char *C_CYAN   = "#22d3ee";
static const char *C_TRACK  = "#334155"; /* empty part of a progress bar */

SimDashboard::SimDashboard(QWidget *parent) : QTextEdit(parent)
{
	setReadOnly(true);
	setTextInteractionFlags(Qt::TextSelectableByMouse
							| Qt::LinksAccessibleByMouse);
	viewport()->setMouseTracking(true);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setStyleSheet(
		"QTextEdit {"
		"  background: #0f172a;"
		"  color: #e2e8f0;"
		"  border: 1px solid #334155;"
		"  border-radius: 6px;"
		"  font-size: 12px;"
		"  padding: 6px;"
		"}");
	setMinimumWidth(200);
	clear();
}

QString SimDashboard::fmtTime(double seconds)
{
	int total = static_cast<int>(seconds);
	if (total < 0)
		total = 0;
	return QString("%1:%2:%3")
		.arg(total / 3600, 2, 10, QChar('0'))
		.arg((total % 3600) / 60, 2, 10, QChar('0'))
		.arg(total % 60, 2, 10, QChar('0'));
}

QString SimDashboard::speedStr(double speed_ms)
{
	return QString("%1 km/h").arg(speed_ms * 3.6, 0, 'f', 0);
}

/* Small coloured pill; QTextEdit has no padding on spans, so use nbsp. */
QString SimDashboard::chip(const QString &text, const QString &color)
{
	return QString("<span style='background-color:%1; color:#0f172a; "
				   "font-weight:bold; font-size:11px;'>&nbsp;%2&nbsp;</span>")
		.arg(color, text.toHtmlEscaped());
}

QString SimDashboard::heading(const QString &title, const QString &right)
{
	return QString("<table width='100%' cellspacing='0' cellpadding='4'>"
				   "<tr><td><span style='font-size:15px; font-weight:bold; "
				   "color:%1;'>%2</span></td>"
				   "<td align='right'><span style='color:%3;'>%4</span></td>"
				   "</tr></table>")
		.arg(C_TITLE, title.toHtmlEscaped(), C_DIM, right);
}

/* A card is a one-cell table so it gets its own background. */
QString SimDashboard::card(const QString &inner, const QString &accent)
{
	return QString("<table width='100%' cellspacing='0' cellpadding='0'>"
				   "<tr><td width='4' bgcolor='%1'></td>"
				   "<td bgcolor='%2'><table width='100%' cellspacing='0' "
				   "cellpadding='7'><tr><td>%3</td></tr></table></td></tr>"
				   "</table><div style='font-size:5px;'>&nbsp;</div>")
		.arg(accent, C_CARD, inner);
}

QString SimDashboard::bar(double fraction, const QString &color)
{
	int pct = static_cast<int>(fraction * 100.0 + 0.5);
	if (pct < 0)
		pct = 0;
	if (pct > 100)
		pct = 100;
	QString done = pct > 0 ? QString("<td width='%1%' bgcolor='%2'>"
									 "<span style='font-size:4px;'>&nbsp;"
									 "</span></td>").arg(pct).arg(color)
						   : QString();
	QString left = pct < 100 ? QString("<td width='%1%' bgcolor='%2'>"
									   "<span style='font-size:4px;'>&nbsp;"
									   "</span></td>")
									 .arg(100 - pct)
									 .arg(C_TRACK)
							 : QString();
	return QString("<table width='100%' cellspacing='0' cellpadding='0'>"
				   "<tr>%1%2</tr></table>")
		.arg(done, left);
}

void SimDashboard::setEventInfo(const QVector<EventInfo> &events)
{
	_events = events;
}

QStringList SimDashboard::segmentEventNames(const QString &from,
											const QString &to) const
{
	QStringList names;
	for (const auto &ev : _events)
	{
		bool onSegment = !ev.node2.isEmpty()
			&& ((ev.node1 == from && ev.node2 == to)
				|| (ev.node1 == to && ev.node2 == from));
		bool atDestNode = ev.node2.isEmpty() && ev.node1 == to;
		if (onSegment || atDestNode)
			names << ev.name;
	}
	return names;
}

QStringList SimDashboard::remainingPathEventNames(const QStringList &path,
												  size_t fromIndex) const
{
	QStringList names;
	for (size_t i = fromIndex; i + 1 < static_cast<size_t>(path.size()); i++)
		names << segmentEventNames(path[static_cast<int>(i)],
								   path[static_cast<int>(i) + 1]);
	return names;
}

void SimDashboard::update(double simTime,
						  const QVector<TrainSnapshot> &snapshots)
{
	const QString arrow = QString::fromUtf8("\xE2\x86\x92");
	QString html = QString("<body style='background-color:%1;'>").arg(C_PANEL);
	html += heading("Live view", QString("clock <b style='color:%1;'>%2</b>")
									 .arg(C_CYAN, fmtTime(simTime)));

	if (!_events.isEmpty() && snapshots.size() <= 60)
	{
		QString ev = QString("<b style='color:%1;'>Events</b><br>")
						 .arg(C_TITLE);
		for (const auto &e : _events)
		{
			QString where = e.node2.isEmpty()
				? QString("station %1").arg(e.node1)
				: QString("%1 %2 %3%4").arg(e.node1, arrow, e.node2,
											e.line >= 0
												? QString(", line %1")
													  .arg(e.line + 1)
												: QString());
			ev += QString("<span style='color:%1;'>\xE2\x9A\xA0 %2</span> "
						  "<span style='color:%3;'>%4 &middot; %5% &middot; "
						  "+%6 min</span><br>")
					  .arg(C_YELLOW, e.name.toHtmlEscaped(), C_DIM,
						   where.toHtmlEscaped())
					  .arg(e.probability * 100.0, 0, 'f', 0)
					  .arg(e.durationSec / 60.0, 0, 'f', 1);
		}
		html += card(ev, C_YELLOW);
	}

	/* Thousands of cards make the HTML (and the UI) crawl, so large runs
	   get a summary plus the first MAX_CARDS trains that are in flight. */
	static constexpr int MAX_CARDS = 40;
	const bool crowded = snapshots.size() > 60;
	int nRun = 0, nWait = 0, nDone = 0, nIdle = 0;
	for (const auto &s : snapshots)
	{
		if (s.arrived) ++nDone;
		else if (!s.departed) ++nIdle;
		else if (s.waiting) ++nWait;
		else ++nRun;
	}
	if (crowded)
	{
		html += card(
			QString("<b style='color:%1;'>%2 trains</b><br>"
					"<span style='color:%3;'>running</span> %4 &nbsp; "
					"<span style='color:%3;'>waiting</span> %5 &nbsp; "
					"<span style='color:%3;'>arrived</span> %6 &nbsp; "
					"<span style='color:%3;'>not departed</span> %7<br>"
					"<span style='color:%3;'>Showing the first %8 trains in "
					"flight. %9 events are configured (see the Events "
					"tab).</span>")
				.arg(C_TITLE)
				.arg(snapshots.size())
				.arg(C_DIM)
				.arg(nRun).arg(nWait).arg(nDone).arg(nIdle)
				.arg(MAX_CARDS)
				.arg(_events.size()),
			C_CYAN);
		html += QString("<span style='color:%1;'>High-load view: train dots "
						"only (no labels or motion trails for responsiveness). "
						"After the run, click a train row to compare planned "
						"and travelled routes.</span><br><br>")
					.arg(C_DIM);
	}

	int shown = 0;
	for (const auto &s : snapshots)
	{
		if (crowded)
		{
			if (!s.departed || s.arrived)
				continue;
			if (shown++ >= MAX_CARDS)
				break;
		}
		QString accent, badge;
		if (s.arrived)
		{
			accent = C_GREEN;
			badge = chip("Arrived", C_GREEN);
		}
		else if (!s.departed)
		{
			accent = C_DIM;
			badge = chip("Not departed", C_DIM);
		}
		else if (s.waiting)
		{
			accent = C_YELLOW;
			badge = chip("Waiting for a line", C_YELLOW);
		}
		else if (s.stopTimer > 0.0)
		{
			accent = C_RED;
			badge = chip("Stopped", C_RED);
		}
		else if (s.status == QLatin1String("Delayed"))
		{
			accent = C_RED;
			badge = chip("Delayed", C_RED);
		}
		else
		{
			accent = C_CYAN;
			badge = chip("Running", C_CYAN);
		}

		QString in = QString("<table width='100%' cellspacing='0' "
							 "cellpadding='0'><tr><td><b style='font-size:"
							 "13px; color:%1;'>%2</b> "
							 "<span style='color:%3;'>%4 %5 %6</span></td>"
							 "<td align='right'>%7</td></tr></table>")
						 .arg(C_TITLE, s.name.toHtmlEscaped(), C_DIM,
							  s.departure, arrow, s.arrival, badge);

		auto muted = [&](const QString &k, const QString &v) {
			return QString("<span style='color:%1;'>%2</span> %3&nbsp;&nbsp; ")
				.arg(C_DIM, k, v);
		};

		if (s.arrived)
		{
			in += "<br>" + muted("Travel", fmtTime(s.timeSinceDepart));
		}
		else if (s.departed)
		{
			double frac = 0.5;
			if (s.pathSize > 1)
			{
				double within = s.segmentLength_m > 0.0
					? std::min(1.0, std::max(0.0, s.posOnSegment_m
												  / s.segmentLength_m))
					: 0.0;
				frac = std::min(1.0, (static_cast<double>(s.segmentIndex)
									  + within)
										 / static_cast<double>(s.pathSize - 1));
			}
			in += "<div style='font-size:4px;'>&nbsp;</div>"
				  + bar(frac, accent);
			QString where = s.from.isEmpty() ? QString()
				: QString("%1 %2 %3").arg(s.from, arrow, s.to);
			if (s.line >= 0)
				where += QString("  <b style='color:%1;'>line %2</b>")
							 .arg(C_CYAN)
							 .arg(s.line + 1);
			in += QString("<br>%1%2%3%4")
					  .arg(where.isEmpty() ? QString()
										   : muted("On", where))
					  .arg(muted("Speed", speedStr(s.speed_ms)))
					  .arg(muted("Elapsed", fmtTime(s.timeSinceDepart)))
					  .arg(muted("Done", QString("%1%")
											 .arg(static_cast<int>(
												 frac * 100.0))));
			if (s.expectedTrip_s > 0.0)
			{
				double left = std::max(0.0, s.expectedTrip_s - s.timeSinceDepart);
				in += QString("<br>%1%2")
						  .arg(muted("ETA", "<b>" + fmtTime(simTime + left)
												+ "</b>"))
						  .arg(muted("Left (planned)", fmtTime(left)));
			}

			if (!s.blockedByName.isEmpty())
				in += QString("<br><span style='color:%1;'>Blocked by %2"
							  "</span>")
						  .arg(C_RED, s.blockedByName.toHtmlEscaped());
			if (s.rerouteCount > 0)
				in += QString("<br><span style='color:%1;'>Rerouted %2x"
							  "%3</span>")
						  .arg(C_CYAN)
						  .arg(s.rerouteCount)
						  .arg(s.lastRerouteReason.isEmpty()
								   ? QString()
								   : ": " + s.lastRerouteReason
												.toHtmlEscaped());
			QStringList risk;
			if (!s.from.isEmpty() && !s.to.isEmpty())
				risk << segmentEventNames(s.from, s.to);
			if (!s.path.isEmpty())
				risk << remainingPathEventNames(
					s.path, static_cast<size_t>(s.segmentIndex) + 1);
			if (!risk.isEmpty())
				in += QString("<br><span style='color:%1;'>\xE2\x9A\xA0 risk "
							  "ahead: %2</span>")
						  .arg(C_YELLOW, risk.join(", ").toHtmlEscaped());
			in += QString("<br><span style='color:%1; font-size:11px;'>"
						  "%2 t &middot; accel %3 &middot; decel %4 m/s&sup2;"
						  "</span>")
					  .arg(C_DIM)
					  .arg(s.weightTons, 0, 'f', 0)
					  .arg(s.accelRate_ms2, 0, 'f', 2)
					  .arg(s.decelRate_ms2, 0, 'f', 2);
		}
		html += card(in, accent);
	}
	html += "</body>";
	setHtml(html);
}

void SimDashboard::showAnalysis(const QVector<TrainRunInfo> &trains,
								const QVector<EventOutcome> &outcomes)
{
	_runTrains = trains;
	_outcomes = outcomes;
	_analysisShown = true;
	renderAnalysis();
}

void SimDashboard::setSelection(const QSet<QString> &trains,
								const QSet<QString> &events)
{
	_selTrains = trains;
	_selEvents = events;
	if (_analysisShown)
		renderAnalysis();
}

void SimDashboard::renderAnalysis()
{
	const int scroll = verticalScrollBar()->value();
	const QString arrow = QString::fromUtf8("\xE2\x86\x92");
	QString html = QString("<body style='background-color:%1;'>").arg(C_PANEL);
	html += heading(QString::fromUtf8("Simulation complete \xE2\x9C\x93"),
					QString("%1 train%2")
						.arg(_runTrains.size())
						.arg(_runTrains.size() == 1 ? "" : "s"));
	html += QString("<span style='color:%1;'>Click an event or a train to "
					"draw it on the graph; click again to hide it. Solid = "
					"route travelled, dashed = original plan; green = origin, "
					"red = destination. Events flash pink.</span>"
					"<br>").arg(C_DIM);

	if (!_events.isEmpty())
	{
		html += QString("<p style='margin:6px 0 2px 0;'><b style='color:%1;'>"
						"Events</b></p>").arg(C_TITLE);
		QHash<QString, const EventOutcome *> byName;
		for (const auto &o : _outcomes)
			byName.insert(o.name, &o);
		for (const auto &e : _events)
		{
			const EventOutcome *o = byName.value(e.name, nullptr);
			const bool sel = _selEvents.contains(e.name);
			QString where = e.node2.isEmpty()
				? QString("station %1").arg(e.node1)
				: QString("%1 %2 %3%4").arg(e.node1, arrow, e.node2,
											e.line >= 0
												? QString(", line %1")
													  .arg(e.line + 1)
												: QString());
			QString href = QString("ev:") + QString::fromLatin1(
				QUrl::toPercentEncoding(e.name));
			QString inner = QString(
				"<a href='%1' style='text-decoration:none;'>"
				"<span style='color:%2; font-weight:bold;'>%3 %4</span></a> "
				"%5<br><span style='color:%6;'>%7 &middot; %8% chance "
				"&middot; +%9 min</span>")
				.arg(href, sel ? "#ff4da6" : C_YELLOW,
					 sel ? QString::fromUtf8("\xE2\x97\x8F")
						 : QString::fromUtf8("\xE2\x9A\xA0"),
					 e.name.toHtmlEscaped(),
					 o ? chip(QString("fired %1x").arg(o->hits), C_RED)
					   : chip("never fired", C_DIM),
					 C_DIM, where.toHtmlEscaped())
				.arg(e.probability * 100.0, 0, 'f', 0)
				.arg(e.durationSec / 60.0, 0, 'f', 1);
			if (o)
				inner += QString("<br><span style='color:%1;'>%2 of delay "
								 "added; e.g. %3</span>")
							 .arg(C_DIM, fmtTime(o->totalDelaySec),
								  o->trains.join(", ").toHtmlEscaped());
			html += card(inner, sel ? "#ff4da6" : C_YELLOW);
		}
	}

	const QVector<TrainRunInfo> &rows = _runTrains;
	html += QString("<p style='margin:8px 0 2px 0;'><b style='color:%1;'>"
					"Trains</b></p>").arg(C_TITLE);
	const QString th = QString("<td><b style='color:%1;'>%2</b></td>");
	QString t = "<table width='100%' cellspacing='0' cellpadding='4'><tr>";
	for (const char *h : {"Train", "Route", "Depart", "Arrive", "ETA", "Trip",
						  "Delay", "Cause"})
		t += th.arg(C_CYAN, h);
	t += "</tr>";
	for (const auto &r : rows)
	{
		const bool sel = _selTrains.contains(r.key);
		const bool late = r.delaySec >= 1.0;
		const QString href = QString("tr:") + QString::fromLatin1(
			QUrl::toPercentEncoding(r.key));
		t += QString("<tr bgcolor='%1'>").arg(sel ? "#3b2a52" : C_CARD);
		auto cell = [&t, &href](const QString &value) {
			t += QString("<td><a href='%1' style='color:inherit; "
						 "text-decoration:none;'>%2</a></td>")
					 .arg(href, value.isEmpty() ? QString("&nbsp;") : value);
		};
		cell(QString("<span style='color:%1;'><b>%2%3</b></span>")
				 .arg(sel ? "#ff4da6" : C_TITLE,
					  sel ? QString::fromUtf8("\xE2\x97\x8F ") : QString(),
					  r.name.toHtmlEscaped()));
		cell(QString("%1 %2 %3").arg(r.origin.toHtmlEscaped(), arrow,
									 r.destination.toHtmlEscaped()));
		cell(fmtTime(r.departureSec));
		cell(fmtTime(r.arrivalSec));
		cell(fmtTime(r.departureSec + r.expectedSec));
		cell(fmtTime(r.actualSec));
		cell(QString("<span style='color:%1;'>%2</span>")
				 .arg(late ? C_RED : C_GREEN, fmtTime(r.delaySec)));
		cell(QString("<span style='color:%1;'>%2</span>")
				 .arg(C_YELLOW, r.cause.toHtmlEscaped()));
		t += "</tr>";
	}
	html += t + "</table></body>";
	setHtml(html);
	verticalScrollBar()->setValue(scroll);
}

void SimDashboard::mouseMoveEvent(QMouseEvent *e)
{
	QTextEdit::mouseMoveEvent(e);
	viewport()->setCursor(anchorAt(e->pos()).isEmpty() ? Qt::IBeamCursor
													   : Qt::PointingHandCursor);
}

void SimDashboard::mouseReleaseEvent(QMouseEvent *e)
{
	QTextEdit::mouseReleaseEvent(e);
	if (e->button() != Qt::LeftButton)
		return;
	const QString a = anchorAt(e->pos());
	if (a.startsWith("ev:"))
		emit eventClicked(QUrl::fromPercentEncoding(a.mid(3).toLatin1()));
	else if (a.startsWith("tr:"))
		emit trainClicked(QUrl::fromPercentEncoding(a.mid(3).toLatin1()));
}

void SimDashboard::showResults(const QStringList &results)
{
	_analysisShown = false;
	static const QRegularExpression re(
		R"(^(.+?) : (\S+) \(delay: (\S+)\)(?: \S+ (.+))?$)");
	QString html = QString("<body style='background-color:%1;'>").arg(C_PANEL);
	html += heading(QString::fromUtf8("Simulation complete \xE2\x9C\x93"),
					QString("%1 train%2")
						.arg(results.size())
						.arg(results.size() == 1 ? "" : "s"));
	QString t = QString("<table width='100%' cellspacing='0' cellpadding='5'>"
						"<tr><td><b style='color:%1;'>Train</b></td>"
						"<td><b style='color:%1;'>Trip</b></td>"
						"<td><b style='color:%1;'>Delay</b></td>"
						"<td><b style='color:%1;'>Cause</b></td></tr>")
					.arg(C_CYAN);
	for (const auto &r : results)
	{
		auto m = re.match(r);
		if (!m.hasMatch())
		{
			t += QString("<tr><td colspan='4'>%1</td></tr>")
					 .arg(r.toHtmlEscaped());
			continue;
		}
		bool late = m.captured(3) != QLatin1String("00:00")
					&& m.captured(3) != QLatin1String("00:00:00");
		t += QString("<tr bgcolor='%1'><td><b>%2</b></td><td>%3</td>"
					 "<td><span style='color:%4;'>%5</span></td>"
					 "<td><span style='color:%6;'>%7</span></td></tr>")
				 .arg(C_CARD, m.captured(1).toHtmlEscaped(),
					  m.captured(2), late ? C_RED : C_GREEN,
					  m.captured(3), C_YELLOW,
					  m.captured(4).toHtmlEscaped());
	}
	html += t + "</table></body>";
	setHtml(html);
}

void SimDashboard::showMultiRunStats(const QVector<TrainStatRow> &stats,
									 int completedRuns)
{
	QString html = QString("<body style='background-color:%1;'>").arg(C_PANEL);
	html += heading(QString::fromUtf8("Multi-run statistics \xE2\x9C\x93"),
					QString("%1 runs").arg(completedRuns));
	QString t = QString("<table width='100%' cellspacing='0' cellpadding='5'>"
						"<tr>");
	for (const char *h : {"Train", "Est.", "Avg", "Min", "Max", "Delay"})
		t += QString("<td><b style='color:%1;'>%2</b></td>").arg(C_CYAN, h);
	t += "</tr>";
	for (const auto &s : stats)
	{
		const char *delayCol = s.avgDelay > 600 ? C_RED
							   : s.avgDelay > 60 ? C_YELLOW : C_GREEN;
		t += QString("<tr bgcolor='%1'><td><b>%2</b></td>"
					 "<td style='color:%3;'>%4</td>"
					 "<td style='color:%5;'>%6</td>"
					 "<td style='color:%7;'>%8</td>"
					 "<td style='color:%9;'>%10</td>"
					 "<td style='color:%11;'>%12</td></tr>")
				 .arg(C_CARD, s.name.toHtmlEscaped(), C_DIM,
					  fmtTime(s.estimated), C_YELLOW, fmtTime(s.avgActual))
				 .arg(C_GREEN, fmtTime(s.minActual), C_RED,
					  fmtTime(s.maxActual), delayCol, fmtTime(s.avgDelay));
	}
	/* Sum over all trains (min/max are not summable per train, so omitted). */
	double tEst = 0, tAvg = 0, tDelay = 0;
	for (const auto &s : stats)
	{
		tEst += s.estimated;
		tAvg += s.avgActual;
		tDelay += s.avgDelay;
	}
	t += QString("<tr bgcolor='%1'><td><b>TOTAL</b></td>"
				 "<td style='color:%2;'><b>%3</b></td>"
				 "<td style='color:%4;'><b>%5</b></td>"
				 "<td></td><td></td>"
				 "<td style='color:%2;'><b>%6</b></td></tr>")
			 .arg(C_PANEL, C_DIM, fmtTime(tEst), C_YELLOW, fmtTime(tAvg),
				  fmtTime(tDelay));
	html += t + "</table></body>";
	setHtml(html);
}

void SimDashboard::showCompare(const QVector<CompareRow> &rows, int runsEach)
{
	QString html = QString("<body style='background-color:%1;'>").arg(C_PANEL);
	html += heading(QString::fromUtf8("Mode comparison \xE2\x9C\x93"),
					QString("%1 run%2 each").arg(runsEach)
						.arg(runsEach == 1 ? "" : "s"));
	auto table = [&](const QString &title, bool useDelay) {
		QString t = QString("<p style='color:%1;'><b>%2</b></p>"
							"<table width='100%' cellspacing='0' "
							"cellpadding='5'><tr>").arg(C_DIM, title);
		for (const char *h : {"Train", "Distance", "Solo", "Adaptive", "Cooperative",
						   "CBS"})
			t += QString("<td><b style='color:%1;'>%2</b></td>")
					 .arg(C_CYAN, h);
		t += "</tr>";
		auto row = [&](const QString &label, const double v[5], bool bold) {
			double mn = *std::min_element(v, v + 5);
			QString r = QString("<tr bgcolor='%1'><td>%2%3%4</td>")
							.arg(bold ? C_PANEL : C_CARD, bold ? "<b>" : "",
								 label.toHtmlEscaped(), bold ? "</b>" : "");
			for (int m = 0; m < 5; m++)
			{
				bool best = v[m] <= mn + 0.5;
				r += QString("<td style='color:%1;'>%2%3</td>")
						 .arg(best ? C_GREEN : C_DIM, fmtTime(v[m]),
							  best ? QString::fromUtf8(" \xE2\x98\x85")
								   : QString());
			}
			t += r + "</tr>";
		};
		double total[5] = {0, 0, 0, 0, 0};
		for (const auto &r : rows)
		{
			row(r.name, useDelay ? r.delay : r.avg, false);
			for (int m = 0; m < 5; m++)
				total[m] += useDelay ? r.delay[m] : r.avg[m];
		}
		row("TOTAL", total, true);
		return t + "</table>";
	};
	html += table("Average travel time", false);
	html += table("Average delay (events + waiting for other trains)", true);
	html += "</body>";
	setHtml(html);
}

void SimDashboard::clear()
{
	_analysisShown = false;
	_runTrains.clear();
	_outcomes.clear();
	_selTrains.clear();
	_selEvents.clear();
	QString html = QString("<body style='background-color:%1;'>").arg(C_PANEL);
	html += heading("Dashboard", QString());
	html += card(QString("<span style='color:%1;'>Run a simulation to see "
						 "live train status here.<br>Each train gets a card "
						 "with progress, speed, line and risks.</span>")
					 .arg(C_DIM),
				 C_CYAN);
	html += "</body>";
	setHtml(html);
}
