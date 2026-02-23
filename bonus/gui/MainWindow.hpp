/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MainWindow.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/02/23 10:21:12 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QComboBox>
#include <QGraphicsView>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QPushButton>
#include <QSlider>
#include <QTextEdit>
#include <QThread>
#include <memory>
#include <vector>

#include "Event.hpp"
#include "NetworkScene.hpp"
#include "RailNetwork.hpp"
#include "SimDashboard.hpp"
#include "SimulationWorker.hpp"
#include "Train.hpp"

class QToolButton;

/**
 * Main application window with interactive graph, sidebar lists,
 * preset file browser, and full CRUD for nodes/edges/trains/events.
 */
class MainWindow : public QMainWindow
{
	Q_OBJECT

  public:
	explicit MainWindow(QWidget *parent = nullptr);
	~MainWindow();

  private slots:
	/* ─── File I/O ──────────────────────────────────────────────────── */
	void onLoadNetwork();
	void onLoadTrains();
	void onSaveNetwork();
	void onSaveTrains();
	void onImportAll();
	void onExportAll();
	void onExportDot();
	void onLoadPresetNetwork(int index);
	void onLoadPresetTrains(int index);

	/* ─── Create ────────────────────────────────────────────────────── */
	void onAddNode();
	void onAddEdge();
	void onAddTrain();
	void onAddEvent();

	/* ─── Delete ────────────────────────────────────────────────────── */
	void onDeleteNode();
	void onDeleteEdge();
	void onDeleteTrain();
	void onDeleteEvent();

	/* ─── Edit ──────────────────────────────────────────────────────── */
	void onEditNode();
	void onEditEdge();
	void onEditTrain();
	void onEditEvent();

	/* ─── Simulation ────────────────────────────────────────────────── */
	void onRunSimulation();
	void onStopSimulation();
	void onSimTick(double simTime, QVector<TrainSnapshot> snapshots);
	void onSimFinished(QStringList results);
	void onSimError(const QString &msg);
	void onSpeedChanged(int value);
	void onFitGraph();

  private:
	void buildMenus();
	void buildToolbar();
	void buildCentralWidget();
	void buildSidebar();
	void refreshLists();
	void logInfo(const QString &msg);
	void logError(const QString &msg);
	void logSuccess(const QString &msg);

	QString writeTemporaryNetworkFile() const;
	QString writeTemporaryTrainFile() const;

	void scanPresetFiles();

	/* ─── Validation helpers ───────────────────────────────────────── */
	bool validateNodeName(const QString &name, QString &errorOut) const;
	bool isSimRunning() const;
	bool guardSimRunning(const QString &action);

	/* ─── Model ─────────────────────────────────────────────────────── */
	RailNetwork _network;
	std::vector<std::unique_ptr<Train>> _trainDefs;
	std::vector<Event> _eventDefs;
	QString _networkFilePath;
	QString _trainFilePath;
	bool _useTimeWeight;
	bool _simRunning;

	/* ─── Widgets ───────────────────────────────────────────────────── */
	QGraphicsView *_view;
	NetworkScene *_scene;
	QListWidget *_nodeList;
	QListWidget *_edgeList;
	QListWidget *_trainList;
	QListWidget *_eventList;
	QTextEdit *_logView;
	SimDashboard *_dashboard;
	QPushButton *_runBtn;
	QPushButton *_stopBtn;
	QSlider *_speedSlider;
	QLabel *_speedLabel;

	/* ─── Preset combos ─────────────────────────────────────────────── */
	QComboBox *_presetNetworkCombo;
	QComboBox *_presetTrainCombo;

	/* ─── Delete buttons (one per tab) ──────────────────────────────── */
	QPushButton *_delNodeBtn;
	QPushButton *_delEdgeBtn;
	QPushButton *_delTrainBtn;
	QPushButton *_delEventBtn;
	QPushButton *_editNodeBtn;
	QPushButton *_editEdgeBtn;
	QPushButton *_editTrainBtn;
	QPushButton *_editEventBtn;

	/* ─── Simulation thread ─────────────────────────────────────────── */
	QThread _simThread;
	SimulationWorker *_worker;
};

#endif
