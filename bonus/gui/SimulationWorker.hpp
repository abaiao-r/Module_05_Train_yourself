/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   SimulationWorker.hpp                               :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by ctw03933         ###   ########.fr       */
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
};

Q_DECLARE_METATYPE(QVector<TrainSnapshot>)

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

  public slots:
	void runSimulation(const QString &networkFile,
					   const QString &trainFile,
					   bool useTimeWeight);

  signals:
	void tick(double simTime, QVector<TrainSnapshot> snapshots);
	void finished(QStringList results);
	void error(const QString &message);

  private:
	std::atomic<double> _speedMult{1.0};
	std::atomic<bool> _stopRequested{false};
};

#endif
