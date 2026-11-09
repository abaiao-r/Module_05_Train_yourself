/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MainWindow.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: abaiao-r <abaiao-r@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/05 19:15:16 by abaiao-r         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QAction>
#include <QCheckBox>
#include <QComboBox>
#include <QGraphicsView>
#include <QLabel>
#include <QListWidget>
#include <QHash>
#include <QMainWindow>
#include <QSet>
#include <QTabWidget>
#include <QTextBrowser>
#include <QPushButton>
#include <QSlider>
#include <QSpinBox>
#include <QSplitter>
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
	void onLoadNetwork();
	void onLoadTrains();
	void onSaveNetwork();
	void onSaveTrains();
	void onImportAll();
	void onExportAll();
	void onExportDot();
	void onLoadPresetNetwork(int index);
	void parseNetworkAuto(const std::string &path, RailNetwork &net,
						  std::vector<Event> &events);
	void onLoadPresetTrains(int index);

	void onAddNode();
	void onAddEdge();
	void onAddTrain();
	void onAddEvent();

	void onDeleteNode();
	void onDeleteEdge();
	void onDeleteTrain();
	void onDeleteEvent();

	void onEditNode();
	void onEditEdge();
	void onEditTrain();
	void onEditEvent();

	void onRunSimulation();
	void updateRunState();
	void updateEditButtons();
	void onStopSimulation();
	void onSimTick(double simTime, QVector<TrainSnapshot> snapshots);
	void onSimFinished(QStringList results);
	void onSimError(const QString &msg);
	void onSpeedChanged(int value);
	void onFitGraph();
	void onRunProgress(int currentRun, int totalRuns);
	void onCompareFinished(QVector<CompareRow> rows,
						   QVector<CompareEventRow> events, int runsEach);
	void onMultiRunFinished(QVector<TrainStatRow> stats, MultiRunTotals totals,
							int completedRuns);
	void onMutationApplied(QString description);
	void onMutationRejected(QString reason);
	void onLiveNodeAdded(QString name);
	void onLiveRailAdded(QString from, QString to, double distanceKm,
						 double speedLimitKmh, int numLines);

  private:
	void buildMenus();
	void buildToolbar();
	void buildCentralWidget();
	void buildSidebar();
	void refreshLists();
	void updateDetails();
	void logInfo(const QString &msg);
	void logError(const QString &msg);
	void logSuccess(const QString &msg);

	QString writeTemporaryNetworkFile() const;
	QString writeTemporaryTrainFile() const;

	void scanPresetFiles();

	bool validateNodeName(const QString &name, QString &errorOut) const;
	bool isSimRunning() const;
	bool guardSimRunning(const QString &action);

	RailNetwork _network;
	std::vector<std::unique_ptr<Train>> _trainDefs;
	std::vector<Event> _eventDefs;
	QString _networkFilePath;
	QString _trainFilePath;
	int _weightMode;  // worker code: 0 Distance, 1 Solo, 2 Adaptive, 3 compare all, 4 Cooperative, 5 CBS
	bool _simRunning;
	double _lastSimTime = 0.0;  // seconds from midnight, updated on each tick

	QSplitter *_hSplit;
	QGraphicsView *_view;
	NetworkScene *_scene;
	QTabWidget *_tabs = nullptr;
	QAction *_showLogAct = nullptr;
	QTextBrowser *_details = nullptr;
	QListWidget *_nodeList;
	QListWidget *_edgeList;
	QListWidget *_trainList;
	QListWidget *_eventList;
	QTextEdit *_logView;
	SimDashboard *_dashboard;

	/* Trains and events currently drawn on the graph (toggled by clicking
	   a train dot or a dashboard row/card). */
	QHash<QString, TrainRunInfo> _runInfo;
	QSet<QString> _selTrains;
	QSet<QString> _selEvents;
	QVector<TrainSnapshot> _lastSnaps;
	QVector<TrainRunInfo> _pendingInfo;
	QVector<EventOutcome> _pendingOutcomes;
	void toggleTrain(const QString &key);
	void toggleEvent(const QString &name);
	void resetSelections();

	static constexpr int DASH_LIVE_W  = 420; /* px – fits BOX_INNER 56 */
	static constexpr int DASH_STATS_W = 540; /* px – fits STAT_BOX 72  */
	QPushButton *_runBtn;
	QLabel *_emptyHint = nullptr;
	QPushButton *_stopBtn;
	QSlider *_speedSlider;
	QLabel *_speedLabel;
	QSpinBox *_runsSpinBox;
	QCheckBox *_animateCheck;
	QCheckBox *_linesCheck;
	QComboBox *_modeCombo;

	QComboBox *_presetNetworkCombo;
	QComboBox *_presetTrainCombo;

	QPushButton *_delNodeBtn;
	QPushButton *_delEdgeBtn;
	QPushButton *_delTrainBtn;
	QPushButton *_delEventBtn;
	QPushButton *_editNodeBtn;
	QPushButton *_editEdgeBtn;
	QPushButton *_editTrainBtn;
	QPushButton *_editEventBtn;

	QThread _simThread;
	SimulationWorker *_worker;
};

#endif
