/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimulationWorker.cpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/06 17:58:05 by ctw03933         ###   ########.fr       */
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
#include <type_traits>

static PathWeightMode intToWeightMode(int idx)
{
	switch (idx)
	{
	case 1:  return PathWeightMode::Solo;
	case 2:  return PathWeightMode::Adaptive;
	default: return PathWeightMode::Distance;
	}
}

static QString fmtHms(double seconds)
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

namespace
{
/* macOS: bin/TrainGUI.app/Contents/MacOS -> four levels up; other
   platforms run bin/TrainGUI directly -> one level up. */
QDir projectRootDir()
{
	QDir dir(QCoreApplication::applicationDirPath());
	int ups = dir.absolutePath().endsWith("/Contents/MacOS") ? 4 : 1;
	while (ups-- > 0)
		dir.cdUp();
	return dir;
}
} // namespace

bool SimulationWorker::isSimulationLive() const
{
	std::lock_guard<std::mutex> lk(_liveSimMutex);
	return _liveSim != nullptr;
}

bool SimulationWorker::enqueueAddNode(const QString &name)
{
	std::lock_guard<std::mutex> lk(_liveSimMutex);
	Simulation *sim = _liveSim;
	if (!sim)
		return false;
	sim->enqueueMutation(AddNodeCommand{name.toStdString()});
	return true;
}

bool SimulationWorker::enqueueAddRail(const QString &from, const QString &to,
									  double distanceKm,
									  double speedLimitKmh)
{
	std::lock_guard<std::mutex> lk(_liveSimMutex);
	Simulation *sim = _liveSim;
	if (!sim)
		return false;
	sim->enqueueMutation(AddRailCommand{from.toStdString(),
									   to.toStdString(), distanceKm,
									   speedLimitKmh});
	return true;
}

bool SimulationWorker::enqueueAddEvent(const QString &name,
									   double probability,
									   double durationSeconds,
									   const QString &node1,
									   const QString &node2)
{
	std::lock_guard<std::mutex> lk(_liveSimMutex);
	Simulation *sim = _liveSim;
	if (!sim)
		return false;
	sim->enqueueMutation(AddEventCommand{
		name.toStdString(), probability, durationSeconds,
		node1.toStdString(), node2.toStdString()});
	return true;
}

bool SimulationWorker::enqueueAddTrain(const QString &name,
									   double weightTons, double friction,
									   double maxAccelKn, double maxBrakeKn,
									   const QString &from,
									   const QString &to,
									   double departureTime,
									   double stopDuration)
{
	std::lock_guard<std::mutex> lk(_liveSimMutex);
	Simulation *sim = _liveSim;
	if (!sim)
		return false;
	sim->enqueueMutation(AddTrainCommand{
		name.toStdString(), weightTons, friction, maxAccelKn, maxBrakeKn,
		from.toStdString(), to.toStdString(), departureTime,
		stopDuration});
	return true;
}

/* Exception used to break out of the simulation loop cleanly. */
struct SimulationStopException : std::exception
{
	const char *what() const noexcept override { return "Simulation stopped by user"; }
};

void SimulationWorker::runSimulation(const QString &networkFile,
									 const QString &trainFile,
									 int weightMode)
{
	_stopRequested.store(false, std::memory_order_relaxed);

	try
	{
		/* Ensure CWD is the project root so FileOutputObserver can
		   write to the relative "output/results/" directory.
		   QFileDialog may have changed the CWD in the main thread. */
		QDir projectRoot = projectRootDir();
		QDir::setCurrent(projectRoot.absolutePath());
		QDir().mkpath("output/results");

		auto data = InputHandler::loadData(networkFile.toStdString(),
										   trainFile.toStdString());
		auto pathfinder = std::make_unique<DijkstraPathfinding>();
		PathWeightMode mode = intToWeightMode(weightMode);

		Simulation sim(std::move(data.network), std::move(data.trains),
					   std::move(data.events), std::move(pathfinder),
					   mode);
		sim.setQuiet(true);

		/* Expose this simulation to enqueueAddX() calls from the GUI
		   thread for the duration of the run; the guard clears it on
		   every exit path (normal, stopped, or exception). */
		{
			std::lock_guard<std::mutex> lk(_liveSimMutex);
			_liveSim = &sim;
		}
		struct LiveSimGuard
		{
			std::mutex &mtx;
			Simulation *&ref;
			/* Taking the lock waits out any in-flight enqueue, so the
			   Simulation is never destroyed under a GUI call. */
			~LiveSimGuard()
			{
				std::lock_guard<std::mutex> lk(mtx);
				ref = nullptr;
			}
		} liveSimGuard{_liveSimMutex, _liveSim};

		sim.setMutationCallbacks(
			[this](const LiveMutation &mutation, const std::string &desc) {
				std::visit(
					[this](const auto &cmd) {
						using T = std::decay_t<decltype(cmd)>;
						if constexpr (std::is_same_v<T, AddNodeCommand>)
							emit nodeAdded(
								QString::fromStdString(cmd.name));
						else if constexpr (std::is_same_v<T, AddRailCommand>)
							emit railAdded(
								QString::fromStdString(cmd.from),
								QString::fromStdString(cmd.to),
								cmd.distanceKm, cmd.speedLimitKmh);
					},
					mutation);
				emit mutationApplied(QString::fromStdString(desc));
			},
			[this](const std::string &reason) {
				emit mutationRejected(QString::fromStdString(reason));
			});

		/* Static event risk, so the GUI can draw markers before
		   anything actually triggers. */
		{
			QVector<EventInfo> evInfos;
			for (const auto &ev : sim.getEvents())
			{
				EventInfo info;
				info.name = QString::fromStdString(ev.getName());
				info.probability = ev.getProbability();
				info.durationSec = ev.getDuration();
				info.node1 = QString::fromStdString(ev.getNodeName());
				info.node2 = QString::fromStdString(ev.getNodeName2());
				evInfos.push_back(info);
			}
			emit eventsAvailable(evInfos);
		}
		QMap<QString, QStringList> triggeredEvents;
		sim.setEventTriggerCallback(
			[this, &triggeredEvents](const std::string &trainName,
				  const std::string &eventName, const std::string &loc,
				  double delaySec) {
				QString tName = QString::fromStdString(trainName);
				triggeredEvents[tName] << QString("%1 (%2)")
					.arg(QString::fromStdString(eventName),
						 fmtHms(delaySec));
				emit eventTriggered(tName,
								   QString::fromStdString(eventName),
								   QString::fromStdString(loc), delaySec);
			});
		sim.setRerouteCallback(
			[this](const std::string &trainName,
				  const std::string &fromNode, const std::string &route) {
				emit rerouteOccurred(QString::fromStdString(trainName),
									QString::fromStdString(fromNode),
									QString::fromStdString(route));
			});

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
					snap.path.clear();
					for (const auto &node : path)
						snap.path << QString::fromStdString(node->getName());
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
					snap.status = QString::fromStdString(
						Train::statusToString(s.train->getStatus()));
					snap.blockedByName = s.blockedByTrain
						? QString::fromStdString(s.blockedByTrain->getName())
						: QString();
					snap.rerouteCount = s.rerouteCount;
					snap.lastRerouteReason =
						QString::fromStdString(s.lastRerouteReason);
					snap.weightTons = s.train->getWeight();
					snap.frictionCoeff = s.train->getFrictionCoefficient();
					snap.maxAccelKn = s.train->getMaxAccelForce();
					snap.maxBrakeKn = s.train->getMaxBrakeForce();
					snap.accelRate_ms2 = s.train->getAccelRate();
					snap.decelRate_ms2 = s.train->getDecelRate();
					snaps.push_back(snap);
				}
				emit tick(simTime, snaps);
				QThread::usleep(FRAME_DELAY_US); // constant ~20 FPS
			});

		sim.run();

		QStringList results;
		for (const auto &r : sim.getResults())
		{
			QString name = QString::fromStdString(r.name);
			QString cause = triggeredEvents.value(name).join(", ");
			QString line = QString("%1%2 : %3 (delay: %4)")
							   .arg(name)
							   .arg(r.id)
							   .arg(fmtHms(r.actualTime),
									fmtHms(r.totalDelay));
			if (!cause.isEmpty())
				line += QString(" \xe2\x80\x94 %1").arg(cause);
			results << line;
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

void SimulationWorker::runMulti(const QString &networkFile,
								const QString &trainFile,
								int weightMode,
								int numRuns,
								bool animateFirst)
{
	_stopRequested.store(false, std::memory_order_relaxed);

	try
	{
		/* Ensure CWD is the project root */
		QDir projectRoot = projectRootDir();
		QDir::setCurrent(projectRoot.absolutePath());
		QDir().mkpath("output/results");

		PathWeightMode mode = intToWeightMode(weightMode);

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

			if (animateFirst && run == 0)
			{
				{
					QVector<EventInfo> evInfos;
					for (const auto &ev : sim.getEvents())
					{
						EventInfo info;
						info.name = QString::fromStdString(ev.getName());
						info.probability = ev.getProbability();
						info.durationSec = ev.getDuration();
						info.node1 = QString::fromStdString(ev.getNodeName());
						info.node2 = QString::fromStdString(ev.getNodeName2());
						evInfos.push_back(info);
					}
					emit eventsAvailable(evInfos);
				}
				sim.setEventTriggerCallback(
					[this](const std::string &trainName,
						  const std::string &eventName,
						  const std::string &loc, double delaySec) {
						emit eventTriggered(
							QString::fromStdString(trainName),
							QString::fromStdString(eventName),
							QString::fromStdString(loc), delaySec);
					});
				sim.setRerouteCallback(
					[this](const std::string &trainName,
						  const std::string &fromNode,
						  const std::string &route) {
						emit rerouteOccurred(
							QString::fromStdString(trainName),
							QString::fromStdString(fromNode),
							QString::fromStdString(route));
					});

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
							snap.path.clear();
							for (const auto &node : path)
								snap.path << QString::fromStdString(node->getName());
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
							snap.status = QString::fromStdString(
								Train::statusToString(s.train->getStatus()));
							snap.blockedByName = s.blockedByTrain
								? QString::fromStdString(s.blockedByTrain->getName())
								: QString();
							snap.rerouteCount = s.rerouteCount;
							snap.lastRerouteReason =
								QString::fromStdString(s.lastRerouteReason);
							snap.weightTons = s.train->getWeight();
							snap.frictionCoeff = s.train->getFrictionCoefficient();
							snap.maxAccelKn = s.train->getMaxAccelForce();
							snap.maxBrakeKn = s.train->getMaxBrakeForce();
							snap.accelRate_ms2 = s.train->getAccelRate();
							snap.decelRate_ms2 = s.train->getDecelRate();
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

			/* Same-named trains (different departures) get separate rows:
			   results keep load order, so the Nth duplicate is stable. */
			std::map<std::string, int> seen;
			for (const auto &r : sim.getResults())
			{
				int n = seen[r.name]++;
				std::string key = n ? r.name + " #" + std::to_string(n + 1)
									: r.name;
				statActual[key].push_back(r.actualTime);
				statDelay[key].push_back(r.totalDelay);
				statEstimated[key] = r.estimatedTime;
			}

			++completedRuns;
			emit runProgress(completedRuns, numRuns);
		}

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
