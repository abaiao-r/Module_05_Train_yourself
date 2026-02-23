/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimDashboard.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/23 01:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SimDashboard.hpp"
#include <cmath>

/* ── Colour palette (matching the dark theme) ──────────────────────────── */
static const char *C_BORDER  = "#06b6d4"; /* cyan-400  – box borders    */
static const char *C_TITLE   = "#e0e0e0"; /* white-ish – title text     */
static const char *C_DIM     = "#94a3b8"; /* slate-400 – dim text       */
static const char *C_GREEN   = "#6ee7b7"; /* emerald-300 – arrived/good */
static const char *C_YELLOW  = "#fbbf24"; /* amber-400 – running        */
static const char *C_RED     = "#f87171"; /* red-400   – stopped/delay  */
static const char *C_CYAN    = "#22d3ee"; /* cyan-300  – progress bar   */
static const char *C_BADGE   = "#334155"; /* slate-700 – badge bg       */

/* ── Constructor ───────────────────────────────────────────────────────── */
SimDashboard::SimDashboard(QWidget *parent) : QTextEdit(parent)
{
	setReadOnly(true);
	setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
	setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	setStyleSheet(
		"QTextEdit {"
		"  background: #0f172a;"
		"  color: #e0e0e0;"
		"  border: 1px solid #334155;"
		"  border-radius: 6px;"
		"  font-family: 'JetBrains Mono', 'Fira Code', 'Courier New', monospace;"
		"  font-size: 11px;"
		"  padding: 4px;"
		"}");
	setMinimumWidth(320);
	clear();
}

/* ── Helpers ───────────────────────────────────────────────────────────── */

QString SimDashboard::fmtTime(double seconds)
{
	int total = static_cast<int>(seconds);
	if (total < 0)
		total = 0;
	int h = total / 3600;
	int m = (total % 3600) / 60;
	int s = total % 60;
	return QString("%1:%2:%3")
		.arg(h, 2, 10, QChar('0'))
		.arg(m, 2, 10, QChar('0'))
		.arg(s, 2, 10, QChar('0'));
}

QString SimDashboard::progressBar(double fraction, int width)
{
	if (fraction < 0.0) fraction = 0.0;
	if (fraction > 1.0) fraction = 1.0;
	int filled = static_cast<int>(fraction * width);
	QString bar;
	for (int i = 0; i < width; i++)
	{
		if (i < filled)
			bar += QChar(0x2588); // █
		else if (i == filled)
			bar += QChar(0x2593); // ▓
		else
			bar += QChar(0x2591); // ░
	}
	return bar;
}

QString SimDashboard::speedStr(double speed_ms)
{
	double kmh = speed_ms * 3.6;
	return QString("%1 km/h").arg(kmh, 6, 'f', 1);
}

QString SimDashboard::hRule(const QString &left, const QString &right, int n)
{
	QString bar;
	for (int i = 0; i < n; i++)
		bar += QChar(0x2550); // ═
	return QString("<span style='color:%1;'>%2%3%4</span><br>")
		.arg(C_BORDER, left, bar, right);
}

/* Count visible characters in an HTML snippet (strip tags & entities). */
static int visLen(const QString &html)
{
	int len = 0;
	bool inTag = false;
	for (int i = 0; i < html.size(); i++)
	{
		QChar c = html[i];
		if (c == '<')          { inTag = true; continue; }
		if (c == '>' && inTag) { inTag = false; continue; }
		if (inTag) continue;
		if (c == '&')
		{
			/* HTML entity like &amp; &lt; etc. → 1 visible char */
			int semi = html.indexOf(';', i + 1);
			if (semi != -1 && semi - i < 10)
				i = semi; /* skip to ';' */
			len++;
			continue;
		}
		len++;
	}
	return len;
}

static constexpr int BOX_INNER = 56; /* visible chars between ║ … ║ */

/* Truncate visible text to maxVis chars, preserving HTML tags. */
static QString visTruncate(const QString &html, int maxVis)
{
	QString out;
	int vis = 0;
	bool inTag = false;
	for (int i = 0; i < html.size(); i++)
	{
		QChar c = html[i];
		if (c == '<') { inTag = true; out += c; continue; }
		if (c == '>' && inTag) { inTag = false; out += c; continue; }
		if (inTag) { out += c; continue; }
		if (c == '&')
		{
			int semi = html.indexOf(';', i + 1);
			if (semi != -1 && semi - i < 10)
			{
				if (vis >= maxVis) break;
				out += html.mid(i, semi - i + 1);
				i = semi;
				vis++;
				continue;
			}
		}
		if (vis >= maxVis) break;
		out += c;
		vis++;
	}
	/* Close any unclosed tags */
	return out;
}

QString SimDashboard::boxRow(const QString &html)
{
	QString content = html;
	int vis = visLen(content);
	/* If content is too wide, truncate it */
	if (vis > BOX_INNER)
		content = visTruncate(content, BOX_INNER);
	vis = visLen(content);
	int pad = BOX_INNER - vis;
	if (pad < 0) pad = 0;
	QString spaces;
	for (int i = 0; i < pad; i++)
		spaces += ' ';
	return QString("<span style='color:%1;'>%2</span>%3%4"
				   "<span style='color:%1;'>%5</span><br>")
		.arg(C_BORDER,
			 QString(QChar(0x2551)), // ║
			 html,
			 spaces,
			 QString(QChar(0x2551)));
}

/* ── Main update ───────────────────────────────────────────────────────── */

void SimDashboard::update(double simTime,
						  const QVector<TrainSnapshot> &snapshots)
{
	QString html;
	html += "<pre style='margin:0; line-height:1.4;'>";

	/* ═══ Header ═══ */
	html += hRule(QString(QChar(0x2554)), QString(QChar(0x2557)), BOX_INNER);
	html += boxRow(QString("  <b style='color:%1;'>Train Simulation "
						   "\xe2\x80\x94 Live View</b>")
					   .arg(C_TITLE));
	html += hRule(QString(QChar(0x2560)), QString(QChar(0x2563)), BOX_INNER);

	/* Clock */
	html += boxRow(QString("  <b style='color:%1;'>Clock:</b> %2")
					   .arg(C_TITLE, fmtTime(simTime)));
	html += hRule(QString(QChar(0x2560)), QString(QChar(0x2563)), BOX_INNER);

	for (int i = 0; i < snapshots.size(); i++)
	{
		const auto &s = snapshots[i];

		/* ── Train header ── */
		QString dot, badge, dotCol;
		if (s.arrived)
		{
			dot = QChar(0x25CF); // ●
			dotCol = C_GREEN;
			badge = QString("<span style='background:%1;color:%2;"
							"padding:1px 4px;border-radius:3px;'>"
							"Arrived</span>")
						.arg(C_BADGE, C_GREEN);
		}
		else if (!s.departed)
		{
			dot = QChar(0x25CB); // ○
			dotCol = C_DIM;
			badge = QString("<span style='background:%1;color:%2;"
							"padding:1px 4px;border-radius:3px;'>"
							"Waiting</span>")
						.arg(C_BADGE, C_DIM);
		}
		else if (s.stopTimer > 0.0)
		{
			dot = QChar(0x25CF);
			dotCol = C_RED;
			badge = QString("<span style='background:%1;color:%2;"
							"padding:1px 4px;border-radius:3px;'>"
							"Stopped</span>")
						.arg(C_BADGE, C_RED);
		}
		else
		{
			dot = QChar(0x25CF);
			dotCol = C_YELLOW;
			badge = QString("<span style='background:%1;color:%2;"
							"padding:1px 4px;border-radius:3px;'>"
							"Running</span>")
						.arg(C_BADGE, C_YELLOW);
		}

		html += boxRow(
			QString("  <span style='color:%1;'>%2</span> "
					"<b>%3%4</b>"
					" <span style='color:%5;'>(%6 → %7)</span>  %8")
				.arg(dotCol, dot, s.name)
				.arg(s.id)
				.arg(C_DIM, s.departure, s.arrival, badge));

		/* ── Detail lines ── */
		if (!s.departed)
		{
			/* Nothing extra — badge says "Waiting" */
		}
		else if (s.arrived)
		{
			html += boxRow(
				QString("    Travel: <span style='color:%1;'>%2</span>"
						"  Delay: <span style='color:%3;'>%4</span>")
					.arg(C_GREEN, fmtTime(s.timeSinceDepart),
						 C_GREEN, "none"));
		}
		else
		{
			/* Segment */
			if (!s.from.isEmpty() && !s.to.isEmpty())
			{
				html += boxRow(
					QString("    <span style='color:%1;'>Seg:</span>"
							" %2 → %3")
						.arg(C_DIM, s.from, s.to));
			}

			/* Speed */
			QString action;
			if (s.stopTimer > 0.0)
				action = "Stopped";
			else if (s.speed_ms < 0.1)
				action = "Starting";
			else
				action = "Moving";
			html += boxRow(
				QString("    <span style='color:%1;'>Spd:</span>"
						" %2  (%3)")
					.arg(C_DIM, speedStr(s.speed_ms), action));

			/* Progress bar */
			double frac = 0.5;
			if (s.pathSize > 1)
			{
				double segCount = static_cast<double>(s.pathSize - 1);
				frac = static_cast<double>(s.segmentIndex) / segCount;
				if (s.segmentIndex + 1 < s.pathSize)
					frac += (1.0 / segCount) * 0.5;
				if (frac > 1.0)
					frac = 1.0;
			}
			int pct = static_cast<int>(frac * 100.0);
			html += boxRow(
				QString("    <span style='color:%1;'>%2</span>"
						" <b>%3%</b>")
					.arg(C_CYAN, progressBar(frac, 30))
					.arg(pct));

			/* Elapsed */
			html += boxRow(
				QString("    <span style='color:%1;'>Time:</span> %2")
					.arg(C_DIM, fmtTime(s.timeSinceDepart)));
		}

		/* Separator */
		if (i + 1 < snapshots.size())
		{
			QString sep;
			for (int j = 0; j < BOX_INNER - 2; j++)
				sep += QChar(0x2500); // ─
			html += boxRow(QString("  <span style='color:%1;'>%2</span>")
							   .arg(C_DIM, sep));
		}
	}

	/* ═══ Footer ═══ */
	html += hRule(QString(QChar(0x255A)), QString(QChar(0x255D)), BOX_INNER);
	html += "</pre>";

	setHtml(html);
}

/* ── Show final results ────────────────────────────────────────────────── */

void SimDashboard::showResults(const QStringList &results)
{
	QString html;
	html += "<pre style='margin:0; line-height:1.4;'>";
	html += hRule(QString(QChar(0x2554)), QString(QChar(0x2557)), BOX_INNER);
	html += boxRow(QString("  <b style='color:%1;'>Simulation Complete "
						   "\xe2\x9c\x93</b>")
					   .arg(C_GREEN));
	html += hRule(QString(QChar(0x2560)), QString(QChar(0x2563)), BOX_INNER);

	for (const auto &r : results)
		html += boxRow("  " + r.toHtmlEscaped());

	html += hRule(QString(QChar(0x255A)), QString(QChar(0x255D)), BOX_INNER);
	html += "</pre>";

	setHtml(html);
}

/* ── Clear / idle ──────────────────────────────────────────────────────── */

void SimDashboard::clear()
{
	QString html;
	html += "<pre style='margin:0; line-height:1.4;'>";
	html += hRule(QString(QChar(0x2554)), QString(QChar(0x2557)), BOX_INNER);
	html += boxRow(QString("  <b style='color:%1;'>Train Simulation "
						   "\xe2\x80\x94 Dashboard</b>")
					   .arg(C_TITLE));
	html += hRule(QString(QChar(0x2560)), QString(QChar(0x2563)), BOX_INNER);
	html += boxRow(QString("  <span style='color:%1;'>"
						   "Run a simulation to see</span>")
					   .arg(C_DIM));
	html += boxRow(QString("  <span style='color:%1;'>"
						   "live train status here.</span>")
					   .arg(C_DIM));
	html += hRule(QString(QChar(0x255A)), QString(QChar(0x255D)), BOX_INNER);
	html += "</pre>";
	setHtml(html);
}
