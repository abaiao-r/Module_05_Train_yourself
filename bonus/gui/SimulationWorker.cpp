/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimulationWorker.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 23:25:46 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "SimulationWorker.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QStringList>
#include <QThread>

#include "DijkstraPathfinding.hpp"
#include "InputHandler.hpp"
#include "Node.hpp"
#include "Simulation.hpp"
#include "TrainFactory.hpp"

#include <map>

SimulationWorker::SimulationWorker(QObject *parent)
	: QObject(parent) {}

SimulationWorker::~SimulationWorker() {}

void SimulationWorker::setSpeedMultiplier(double mult)
{
	_speedMult.store(mult, std::memory_order_relaxed);
}

double SimulationWorker::speedMultiplier() const
{
	return _speedMult.load(std::memory_order_relaxed);
}

void SimulationWorker::requestStop()
{
	_stopRequested.store(true, std::memory_order_relaxed);
}

bool SimulationWorker::isStopRequested() const
{
	return _stopRequested.load(std::memory_order_relaxed);
}

/* Exception used to break out of the simulation loop cleanly. */
struct SimulationStopException : std::exception
{
	const char *what() const noexcept override { return "Simulation stopped by user"; }
};

void SimulationWorker::runSimulation(const QString &networkFile,
									 const QString &trainFile,
									 bool useTimeWeight)
{
	_stopRequested.store(false, std::memory_order_relaxed);

	try
	{
		/* Ensure CWD is the project root so FileOutputObserver can
		   write to the relative "output/results/" directory.
		   QFileDialog may have changed the CWD in the main thread. */
		QDir projectRoot(QCoreApplication::applicationDirPath());
		projectRoot.cdUp(); // MacOS -> Contents
		projectRoot.cdUp(); // Contents -> TrainGUI.app
		projectRoot.cdUp(); // TrainGUI.app -> bin
		projectRoot.cdUp(); // bin -> project root
		QDir::setCurrent(projectRoot.absolutePath());
		QDir().mkpath("output/results");

		auto data = InputHandler::loadData(networkFile.toStdString(),
										   trainFile.toStdString());
		auto pathfinder = std::make_unique<DijkstraPathfinding>();
		PathWeightMode mode = useTimeWeight ? PathWeightMode::Time
											: PathWeightMode::Distance;

		Simulation sim(std::move(data.network), std::move(data.trains),
					   std::move(data.events), std::move(pathfinder),
					   mode);
		sim.setQuiet(true);

		/* Install a per-tick callback that emits snapshots.
		   Throttle to ~20 FPS real-time: emit every FRAME_INTERVAL
		   sim-seconds and sleep FRAME_DELAY_US between frames.
		   This gives a visible animation at ~600x speedup. */
		static constexpr double BASE_INTERVAL = 30.0; // sim seconds per frame
		static constexpr unsigned FRAME_DELAY_US = 50000; // 50 ms constant
		double lastEmit = -BASE_INTERVAL;

		sim.setAnimCallback(
			[this, &lastEmit, &sim](double simTime,
				   const std::vector<TrainState> &states) {
				if (_stopRequested.load(std::memory_order_relaxed))
					throw SimulationStopException();
				double spd = _speedMult.load(std::memory_order_relaxed);
				if (spd < 0.05) spd = 0.05;
				double interval = BASE_INTERVAL * spd;
				if (interval < 1.0) interval = 1.0;
				if (simTime - lastEmit < interval)
					return;
				lastEmit = simTime;

				QVector<TrainSnapshot> snaps;
				snaps.reserve(static_cast<int>(states.size()));
				for (const auto &s : states)
				{
					TrainSnapshot snap;
					snap.name = QString::fromStdString(
						s.train->getName());
					snap.id = s.train->getId();
					snap.departure = QString::fromStdString(
						s.train->getDepartureStation());
					snap.arrival = QString::fromStdString(
						s.train->getArrivalStation());
					auto &path = s.train->getPath();
					if (path.size() >= 2
						&& s.segmentIndex + 1 < path.size())
					{
						snap.from = QString::fromStdString(
							path[s.segmentIndex]->getName());
						snap.to = QString::fromStdString(
							path[s.segmentIndex + 1]->getName());
					}
					snap.posOnSegment_m = s.posOnSegment_m;
					snap.speed_ms = s.speed_ms;
					snap.timeSinceDepart = s.timeSinceDepart;
					snap.stopTimer = s.stopTimer;
					snap.departed = s.departed;
					snap.arrived = s.arrived;
					snap.segmentIndex = s.segmentIndex;
					snap.pathSize = path.size();
					snap.segmentLength_m = 0.0;
					if (path.size() >= 2
						&& s.segmentIndex + 1 < path.size())
					{
						try {
							const auto &edges = sim.getNetwork()
								.getNeighbours(path[s.segmentIndex]->getName());
							for (const auto &e : edges)
							{
								if (e.destination->getName()
									== path[s.segmentIndex + 1]->getName())
								{
									snap.segmentLength_m = e.distance * 1000.0;
									break;
								}
							}
						} catch (...) {}
					}
					snaps.push_back(snap);
				}
				emit tick(simTime, snaps);
				QThread::usleep(FRAME_DELAY_US); // constant ~20 FPS
			});

		sim.run();

		/* Collect results */
		QStringList results;
		for (const auto &r : sim.getResults())
		{
			int h = static_cast<int>(r.actualTime) / 3600;
			int m = (static_cast<int>(r.actualTime) % 3600) / 60;
			results << QString("%1%2 : %3h%4m (delay: %5s)")
						   .arg(QString::fromStdString(r.name))
						   .arg(r.id)
						   .arg(h, 2, 10, QChar('0'))
						   .arg(m, 2, 10, QChar('0'))
						   .arg(static_cast<int>(r.totalDelay));
		}
		emit finished(results);
	}
	catch (const SimulationStopException &)
	{
		emit finished(QStringList() << "--- Simulation stopped by user ---");
	}
	catch (const std::exception &e)
	{
		emit error(QString::fromStdString(e.what()));
	}
}

/* ================================================================== */
/*  runMulti — Monte-Carlo multi-run with optional first-run anim     */
/* ================================================================== */

void SimulationWorker::runMulti(const QString &networkFile,
								const QString &trainFile,
								bool useTimeWeight,
								int numRuns,
								bool animateFirst)
{
	_stopRequested.store(false, std::memory_order_relaxed);

	try
	{
		/* Ensure CWD is the project root */
		QDir projectRoot(QCoreApplication::applicationDirPath());
		projectRoot.cdUp();
		projectRoot.cdUp();
		projectRoot.cdUp();
		projectRoot.cdUp();
		QDir::setCurrent(projectRoot.absolutePath());
		QDir().mkpath("output/results");

		PathWeightMode mode = useTimeWeight ? PathWeightMode::Time
											: PathWeightMode::Distance;

		/* Stats accumulators: key = trainName */
		std::map<std::string, std::vector<double>> statActual;
		std::map<std::string, std::vector<double>> statDelay;
		std::map<std::string, double> statEstimated;

		int completedRuns = 0;

		for (int run = 0; run < numRuns; ++run)
		{
			if (_stopRequested.load(std::memory_order_relaxed))
				break;

			TrainFactory::resetIdCounter();
			auto data = InputHandler::loadData(networkFile.toStdString(),
											   trainFile.toStdString());
			auto pathfinder = std::make_unique<DijkstraPathfinding>();

			Simulation sim(std::move(data.network), std::move(data.trains),
						   std::move(data.events), std::move(pathfinder),
						   mode);
			sim.setQuiet(true);

			/* Animate only the first run if requested */
			if (animateFirst && run == 0)
			{
				static constexpr double BASE_INTERVAL = 30.0;
				static constexpr unsigned FRAME_DELAY_US = 50000;
				double lastEmit = -BASE_INTERVAL;

				sim.setAnimCallback(
					[this, &lastEmit, &sim](double simTime,
									  const std::vector<TrainState> &states) {
						if (_stopRequested.load(std::memory_order_relaxed))
							throw SimulationStopException();
						double spd = _speedMult.load(std::memory_order_relaxed);
						if (spd < 0.05) spd = 0.05;
						double interval = BASE_INTERVAL * spd;
						if (interval < 1.0) interval = 1.0;
						if (simTime - lastEmit < interval)
							return;
						lastEmit = simTime;

						QVector<TrainSnapshot> snaps;
						snaps.reserve(static_cast<int>(states.size()));
						for (const auto &s : states)
						{
							TrainSnapshot snap;
							snap.name = QString::fromStdString(s.train->getName());
							snap.id = s.train->getId();
							snap.departure = QString::fromStdString(
								s.train->getDepartureStation());
							snap.arrival = QString::fromStdString(
								s.train->getArrivalStation());
							auto &path = s.train->getPath();
							if (path.size() >= 2
								&& s.segmentIndex + 1 < path.size())
							{
								snap.from = QString::fromStdString(
									path[s.segmentIndex]->getName());
								snap.to = QString::fromStdString(
									path[s.segmentIndex + 1]->getName());
							}
							snap.posOnSegment_m = s.posOnSegment_m;
							snap.speed_ms = s.speed_ms;
							snap.timeSinceDepart = s.timeSinceDepart;
							snap.stopTimer = s.stopTimer;
							snap.departed = s.departed;
							snap.arrived = s.arrived;
							snap.segmentIndex = s.segmentIndex;
							snap.pathSize = path.size();
							snap.segmentLength_m = 0.0;
							if (path.size() >= 2
								&& s.segmentIndex + 1 < path.size())
							{
								try {
									const auto &edges = sim.getNetwork()
										.getNeighbours(path[s.segmentIndex]->getName());
									for (const auto &e : edges)
									{
										if (e.destination->getName()
											== path[s.segmentIndex + 1]->getName())
										{
											snap.segmentLength_m = e.distance * 1000.0;
											break;
										}
									}
								} catch (...) {}
							}
							snaps.push_back(snap);
						}
						emit tick(simTime, snaps);
						QThread::usleep(FRAME_DELAY_US);
					});
			}
			else
			{
				/* Non-animated runs: just check for stop requests */
				sim.setAnimCallback(
					[this](double, const std::vector<TrainState> &) {
						if (_stopRequested.load(std::memory_order_relaxed))
							throw SimulationStopException();
					});
			}

			sim.run();

			/* Collect results from this run */
			for (const auto &r : sim.getResults())
			{
				statActual[r.name].push_back(r.actualTime);
				statDelay[r.name].push_back(r.totalDelay);
				statEstimated[r.name] = r.estimatedTime;
			}

			++completedRuns;
			emit runProgress(completedRuns, numRuns);
		}

		/* Build aggregated stats */
		QVector<TrainStatRow> stats;
		for (const auto &pair : statActual)
		{
			const std::string &name = pair.first;
			const auto &times = pair.second;
			const auto &delays = statDelay[name];

			double sum = 0.0, mn = times[0], mx = times[0];
			for (double t : times)
			{
				sum += t;
				if (t < mn) mn = t;
				if (t > mx) mx = t;
			}
			double avg = sum / static_cast<double>(times.size());

			double dsum = 0.0;
			for (double d : delays)
				dsum += d;
			double avgDelay = dsum / static_cast<double>(delays.size());

			TrainStatRow row;
			row.name = QString::fromStdString(name);
			row.estimated = statEstimated[name];
			row.avgActual = avg;
			row.minActual = mn;
			row.maxActual = mx;
			row.avgDelay = avgDelay;
			row.runs = completedRuns;
			stats.push_back(row);
		}

		emit multiRunFinished(stats, completedRuns);
	}
	catch (const SimulationStopException &)
	{
		/* Stopped mid-run — still emit partial stats if we have any */
		emit multiRunFinished(QVector<TrainStatRow>(), 0);
	}
	catch (const std::exception &e)
	{
		emit error(QString::fromStdString(e.what()));
	}
}
