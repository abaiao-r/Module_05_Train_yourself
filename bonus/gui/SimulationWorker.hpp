/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimulationWorker.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/06 12:30:48 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMULATIONWORKER_HPP
#define SIMULATIONWORKER_HPP

#include <QObject>
#include <QMutex>
#include <QString>
#include <QStringList>
#include <QVector>
#include <atomic>
#include <mutex>
#include <memory>
#include <string>
#include <vector>

struct TrainState;
class Simulation;

/**
 * Snapshot of one train's state, safe to send across threads via signal.
 */
struct TrainSnapshot
{
	QString name;
	int id;
	QString from;
	QString to;
	QString departure;
	QString arrival;
	double posOnSegment_m;
	double speed_ms;
	double timeSinceDepart;
	double expectedTrip_s = 0.0; // planned trip time (no random events)
	double stopTimer;
	bool departed;
	bool arrived;
	size_t segmentIndex;
	size_t pathSize;
	double segmentLength_m;
	int line = -1;        // line id ridden (lines mode), -1 if none
	bool waiting = false; // standing at a station awaiting a free line
	/* Full route (node names in order), so displays can look ahead for
	   risk/congestion beyond just the current segment - "from"/"to"
	   alone only cover where the train is right now. */
	QStringList path;
	/* Status/blocking/reroute info, invisible in the raw physics data
	   above but useful for a user trying to understand WHY a train is
	   behaving a certain way. */
	QString status;          // "Waiting"/"Running"/"Delayed"/"Arrived"
	QString blockedByName;   // empty if not currently blocked
	int rerouteCount;
	QString lastRerouteReason; // empty if never rerouted
	/* Train physics, otherwise only visible in the input file. */
	double weightTons;
	double frictionCoeff;
	double maxAccelKn;
	double maxBrakeKn;
	double accelRate_ms2;  // derived: (accelForce - friction) / mass
	double decelRate_ms2;  // derived: (brakeForce + friction) / mass
};

Q_DECLARE_METATYPE(QVector<TrainSnapshot>)

/**
 * Static event info (name/probability/duration/location), for displays
 * that want to show event risk on the graph before it materializes.
 */
struct EventInfo
{
	QString name;
	double probability;
	double durationSec;
	QString node1;
	QString node2; // empty → station event, not a rail-segment event
	int line = -1; // 0-based line of a single-line event, -1 = whole rail
};

Q_DECLARE_METATYPE(QVector<EventInfo>)

/** Post-run facts about one train, for the analysis table and the
	planned-vs-taken route overlay. */
struct TrainRunInfo
{
	QString key;  // name + id, the same key the scene uses
	QString name;
	QString origin;
	QString destination;
	double departureSec = 0.0;
	double arrivalSec = 0.0;
	double expectedSec = 0.0;
	double actualSec = 0.0;
	double delaySec = 0.0;
	QString cause;
	QStringList plannedPath; // route when the run started
	QStringList actualPath;  // stations actually traversed
	QVector<int> actualLines; // line used for each traversed segment
};

Q_DECLARE_METATYPE(QVector<TrainRunInfo>)

/** How often one event actually fired during the run. */
struct EventOutcome
{
	QString name;
	int hits = 0;
	double totalDelaySec = 0.0;
	QStringList trains; // first few affected trains
};

Q_DECLARE_METATYPE(QVector<EventOutcome>)

/**
 * Aggregated statistics for one train across multiple simulation runs.
 */
struct TrainStatRow
{
	QString name;
	double estimated;
	double avgActual;
	double minActual;
	double maxActual;
	double avgDelay;
	int runs;
};

Q_DECLARE_METATYPE(QVector<TrainStatRow>)

/** Average travel time of one train under Distance / Solo / Adaptive. */
struct CompareRow
{
	QString name;
	double avg[5]; // Distance, Solo, Adaptive, Cooperative, CBS
	double delay[5];
};

Q_DECLARE_METATYPE(QVector<CompareRow>)

/**
 * Runs a simulation in a background thread, emitting per-tick snapshots
 * via a Qt signal so the GUI can update without blocking.
 *
 * This class follows the Worker Object pattern (QObject moved to QThread).
 */
class SimulationWorker : public QObject
{
	Q_OBJECT

  public:
	explicit SimulationWorker(QObject *parent = nullptr);
	~SimulationWorker();

	/** Thread-safe speed multiplier (1.0 = default, <1 = slower, >1 = faster). */
	void setSpeedMultiplier(double mult);
	double speedMultiplier() const;

	/** Request the simulation to stop at the next frame. */
	void requestStop();
	bool isStopRequested() const;

	/** Thread-safe: the GUI calls this once it has drawn a tick, so the
	    worker only sends a new frame when the previous one is done. */
	void frameConsumed();

	/**
	 * Thread-safe: queue an addition into the currently-running
	 * simulation. Returns false (no-op) if no simulation is running.
	 * Applied at the start of the next physics tick; rejections are
	 * reported asynchronously via mutationRejected().
	 */
	bool enqueueAddNode(const QString &name);
	bool enqueueAddRail(const QString &from, const QString &to,
						double distanceKm, double speedLimitKmh,
						int numLines = 1);
	bool enqueueAddEvent(const QString &name, double probability,
						 double durationSeconds, const QString &node1,
						 const QString &node2, int line = -1);
	bool enqueueAddTrain(const QString &name, double weightTons,
						 double friction, double maxAccelKn,
						 double maxBrakeKn, const QString &from,
						 const QString &to, double departureTime,
						 double stopDuration);
	bool isSimulationLive() const;

  public slots:
	void runSimulation(const QString &networkFile,
					   const QString &trainFile,
					   int weightMode,
					   bool multiLines = false);

	/** Run N simulations, optionally animating the first run.
	    Emits runProgress() after each run, then multiRunFinished(). */
	void runMulti(const QString &networkFile,
				  const QString &trainFile,
				  int weightMode,
				  int numRuns,
				  bool animateFirst,
				  bool multiLines = false);

	/** Run every path mode numRuns times each (quietly) and emit
	    compareFinished() with the average travel time per train. */
	void runCompare(const QString &networkFile,
					const QString &trainFile,
					int numRuns,
					bool multiLines = false);

  signals:
	void compareFinished(QVector<CompareRow> rows, int runsEach);
	void tick(double simTime, QVector<TrainSnapshot> snapshots);
	void finished(QStringList results);
	/** Emitted just before finished() with the data behind the results. */
	void analysisReady(QVector<TrainRunInfo> trains,
					   QVector<EventOutcome> events);
	void error(const QString &message);
	void runProgress(int currentRun, int totalRuns);
	void multiRunFinished(QVector<TrainStatRow> stats, int completedRuns);
	/** A live-added node/rail was accepted — the GUI should draw it. */
	void nodeAdded(QString name);
	void railAdded(QString from, QString to, double distanceKm,
				   double speedLimitKmh, int numLines);
	/** A live-added event/train was accepted (no visual element to draw;
		trains appear automatically via tick() once they depart). */
	void mutationApplied(QString description);
	void mutationRejected(QString reason);
	/** Static event list, emitted once near the start of a run so the
		GUI can draw risk markers before anything actually triggers. */
	void eventsAvailable(QVector<EventInfo> events);
	/** A random event actually triggered on a train this tick. */
	void eventTriggered(QString trainName, QString eventName,
					   QString location, double delaySec);
	/** A train's path was changed by the adaptive reroute logic. */
	void rerouteOccurred(QString trainName, QString fromNode,
						QString newRoute);

  private:
	std::atomic<double> _speedMult{1.0};
	std::atomic<bool> _stopRequested{false};
	std::atomic<int> _pendingFrames{0};
	mutable std::mutex _liveSimMutex;
	Simulation *_liveSim = nullptr;
};

#endif
