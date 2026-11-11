/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/03/01 14:11:37 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <QApplication>
#include <QCommandLineParser>
#include <QStyleFactory>
#include <QTimer>
#include <algorithm>
#include "MainWindow.hpp"

int main(int argc, char *argv[])
{
QApplication app(argc, argv);
app.setApplicationName("Train Simulation");
app.setOrganizationName("42");

/* Use Fusion style on all platforms so the dark stylesheet
   renders identically (macOS native style ignores some properties). */
app.setStyle(QStyleFactory::create("Fusion"));

QCommandLineParser cli;
cli.setApplicationDescription(
	"Train Simulation GUI. The --demo-* options run one scripted simulation, "
	"e.g. to record a video.");
cli.addHelpOption();
const QCommandLineOption net("demo-network", "Network file.", "file");
const QCommandLineOption trains("demo-trains", "Train file.", "file");
const QCommandLineOption mode("demo-mode",
	"distance|solo|adaptive|cooperative|cbs|compare (default solo).", "mode",
	"solo");
const QCommandLineOption lines("demo-lines", "Enable multi-line rails.");
const QCommandLineOption speed("demo-speed",
	"Speed multiplier, as the Speed slider (default 1).", "x", "1");
const QCommandLineOption runs("demo-runs", "Runs (default 1).", "n", "1");
const QCommandLineOption capture("demo-capture",
	"Save one PNG per frame into this directory.", "dir");
const QCommandLineOption target("demo-target",
	"window|view|dashboard|split (default window).", "what", "window");
const QCommandLineOption size("demo-size", "Window size WxH.", "WxH",
	"1200x800");
const QCommandLineOption select("demo-select",
	"Comma-separated train names whose route is highlighted.", "names");
const QCommandLineOption hold("demo-hold",
	"Extra frames of the final result (default 0).", "n", "0");
const QCommandLineOption quit("demo-quit", "Quit when done.");
cli.addOptions({net, trains, mode, lines, speed, runs, capture, target, size,
				select, hold, quit});
cli.process(app);

MainWindow win;
QStringList wh = cli.value(size).split('x');
int w = wh.size() == 2 ? wh[0].toInt() : 0;
int h = wh.size() == 2 ? wh[1].toInt() : 0;
win.resize(w > 0 ? w : 1200, h > 0 ? h : 800);
win.show();

if (cli.isSet(net) || cli.isSet(trains))
{
	if (!cli.isSet(net) || !cli.isSet(trains))
	{
		qCritical("--demo-network and --demo-trains go together");
		return 1;
	}
	DemoOptions opts;
	opts.network = cli.value(net);
	opts.trains = cli.value(trains);
	opts.mode = cli.value(mode);
	opts.lines = cli.isSet(lines);
	opts.speed = std::max(0.01, cli.value(speed).toDouble());
	opts.runs = cli.value(runs).toInt();
	opts.captureDir = cli.value(capture);
	opts.target = cli.value(target);
	if (cli.isSet(select))
		opts.select = cli.value(select).split(',', Qt::SkipEmptyParts);
	opts.holdFrames = std::max(0, cli.value(hold).toInt());
	opts.quit = cli.isSet(quit);
	if (!win.startDemo(opts))
		return 1;
}

return app.exec();
}
