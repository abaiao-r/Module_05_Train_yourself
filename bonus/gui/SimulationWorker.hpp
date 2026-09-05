/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimulationWorker.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/05 14:09:06 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SIMULATIONWORKER_HPP
#define SIMULATIONWORKER_HPP

#include <QObject>
#include <QMutex>
#include <QString>
#include <QVector>
#include <atomic>
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
	double stopTimer;
	bool departed;
	bool arrived;
	size_t segmentIndex;
	size_t pathSize;
	double segmentLength_m;
};

Q_DECLARE_METATYPE(QVector<TrainSnapshot>)

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

	/**
	 * Thread-safe: queue an addition into the currently-running
	 * simulation. Returns false (no-op) if no simulation is running.
	 * Applied at the start of the next physics tick; rejections are
	 * reported asynchronously via mutationRejected().
	 */
	bool enqueueAddNode(const QString &name);
	bool enqueueAddRail(const QString &from, const QString &to,
						double distanceKm, double speedLimitKmh);
	bool enqueueAddEvent(const QString &name, double probability,
						 double durationSeconds, const QString &node1,
						 const QString &node2);
	bool enqueueAddTrain(const QString &name, double weightTons,
						 double friction, double maxAccelKn,
						 double maxBrakeKn, const QString &from,
						 const QString &to, double departureTime,
						 double stopDuration);
	bool isSimulationLive() const;

  public slots:
	void runSimulation(const QString &networkFile,
					   const QString &trainFile,
					   bool useTimeWeight);

	/** Run N simulations, optionally animating the first run.
	    Emits runProgress() after each run, then multiRunFinished(). */
	void runMulti(const QString &networkFile,
				  const QString &trainFile,
				  bool useTimeWeight,
				  int numRuns,
				  bool animateFirst);

  signals:
	void tick(double simTime, QVector<TrainSnapshot> snapshots);
	void finished(QStringList results);
	void error(const QString &message);
	void runProgress(int currentRun, int totalRuns);
	void multiRunFinished(QVector<TrainStatRow> stats, int completedRuns);
	/** A live-added node/rail was accepted — the GUI should draw it. */
	void nodeAdded(QString name);
	void railAdded(QString from, QString to, double distanceKm,
				   double speedLimitKmh);
	/** A live-added event/train was accepted (no visual element to draw;
		trains appear automatically via tick() once they depart). */
	void mutationApplied(QString description);
	void mutationRejected(QString reason);

  private:
	std::atomic<double> _speedMult{1.0};
	std::atomic<bool> _stopRequested{false};
	std::atomic<Simulation *> _liveSim{nullptr};
};

#endif
