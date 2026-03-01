/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   MainWindow.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/03/01 18:28:59 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "MainWindow.hpp"

#include <QAction>
#include <QComboBox>
#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <cmath>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGraphicsDropShadowEffect>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMenuBar>
#include <QMessageBox>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSpinBox>
#include <QSplitter>
#include <QStatusBar>
#include <QTabWidget>
#include <QTemporaryFile>
#include <QTextStream>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "DijkstraPathfinding.hpp"
#include "GraphExporter.hpp"
#include "InputHandler.hpp"
#include "Node.hpp"
#include "TrainFactory.hpp"

/* ================================================================== */
/*  Helpers                                                           */
/* ================================================================== */

static QString quoteIfNeeded(const std::string &s)
{
	QString q = QString::fromStdString(s);
	if (q.contains(' '))
		return '"' + q + '"';
	return q;
}

/* ================================================================== */
/*  ZoomableView – wheel zoom + smart pan / node-drag                 */
/* ================================================================== */

#include <QWheelEvent>

class ZoomableView : public QGraphicsView
{
public:
	explicit ZoomableView(QGraphicsScene *scene, QWidget *parent = nullptr)
		: QGraphicsView(scene, parent)
	{
		setRenderHint(QPainter::Antialiasing);
		setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
		setResizeAnchor(QGraphicsView::AnchorUnderMouse);
		setDragMode(QGraphicsView::ScrollHandDrag);
		setInteractive(true);
	}

protected:
	void wheelEvent(QWheelEvent *event) override
	{
		const double factor = 1.15;
		if (event->angleDelta().y() > 0)
			scale(factor, factor);
		else
			scale(1.0 / factor, 1.0 / factor);
		event->accept();
	}

	void mousePressEvent(QMouseEvent *event) override
	{
		/* If clicking on a movable item, disable ScrollHandDrag so the
		   item can be dragged instead of panning the view. */
		if (event->button() == Qt::LeftButton)
		{
			QGraphicsItem *item = itemAt(event->pos());
			if (item && (item->flags() & QGraphicsItem::ItemIsMovable))
			{
				setDragMode(QGraphicsView::NoDrag);
				QGraphicsView::mousePressEvent(event);
				return;
			}
		}
		setDragMode(QGraphicsView::ScrollHandDrag);
		QGraphicsView::mousePressEvent(event);
	}

	void mouseReleaseEvent(QMouseEvent *event) override
	{
		QGraphicsView::mouseReleaseEvent(event);
		setDragMode(QGraphicsView::ScrollHandDrag);
	}
};

/* ================================================================== */
/*  Style sheet                                                       */
/* ================================================================== */

static const char *STYLESHEET = R"(
QMainWindow { background: #1a1a2e; }
QMenuBar { background: #16213e; color: #e0e0e0; font-size: 13px; padding: 2px; }
QMenuBar::item { padding: 6px 12px; border-radius: 4px; }
QMenuBar::item:selected { background: #0f3460; }
QMenu { background: #16213e; color: #e0e0e0; border: 1px solid #0f3460;
        border-radius: 4px; padding: 4px; }
QMenu::item { padding: 6px 24px; border-radius: 3px; }
QMenu::item:selected { background: #533483; }
QMenu::separator { background: #0f3460; height: 1px; margin: 4px 8px; }
QToolBar { background: #16213e; border: none; spacing: 4px; padding: 4px; }
QToolBar QToolButton { color: #e0e0e0; padding: 6px 10px; border-radius: 4px;
                       font-size: 12px; }
QToolBar QToolButton:hover { background: #533483; }
QPushButton { background: #533483; color: white; border: none;
              padding: 7px 16px; border-radius: 5px; font-weight: bold;
              font-size: 12px; }
QPushButton:hover { background: #7c3aed; }
QPushButton:pressed { background: #4c1d95; }
QPushButton:disabled { background: #374151; color: #6b7280; }
QPushButton[danger="true"] { background: #dc2626; }
QPushButton[danger="true"]:hover { background: #ef4444; }
QListWidget { background: #1e293b; color: #e0e0e0; border: 1px solid #334155;
              border-radius: 6px; padding: 4px; font-size: 12px;
              selection-background-color: #533483; outline: none; }
QListWidget::item { padding: 5px 8px; border-radius: 3px; }
QListWidget::item:hover { background: #334155; }
QListWidget::item:selected { background: #533483; color: white; }
QTextEdit { background: #0f172a; color: #6ee7b7; border: 1px solid #334155;
            border-radius: 6px; font-family: 'JetBrains Mono', 'Fira Code',
            'Courier New', monospace; font-size: 11px; padding: 6px; }
QLabel { color: #e0e0e0; }
QGroupBox { color: #e0e0e0; font-weight: bold; border: 1px solid #334155;
            border-radius: 6px; margin-top: 10px; padding-top: 14px; }
QGroupBox::title { subcontrol-origin: margin; left: 12px; padding: 0 6px; }
QTabWidget::pane { border: 1px solid #334155; border-radius: 0 0 6px 6px;
                   background: #1e293b; }
QTabBar::tab { background: #1e293b; color: #94a3b8; padding: 8px 16px;
               border-top-left-radius: 6px; border-top-right-radius: 6px;
               font-weight: bold; font-size: 11px; min-width: 60px; }
QTabBar::tab:selected { background: #533483; color: white; }
QTabBar::tab:hover:!selected { background: #334155; }
QComboBox { background: #1e293b; color: #e0e0e0; border: 1px solid #334155;
            border-radius: 5px; padding: 5px 8px; font-size: 12px; }
QComboBox:hover { border-color: #533483; }
QComboBox::drop-down { border: none; width: 20px; }
QComboBox QAbstractItemView { background: #1e293b; color: #e0e0e0;
                              border: 1px solid #334155;
                              selection-background-color: #533483; }
QSpinBox, QDoubleSpinBox { background: #1e293b; color: #e0e0e0;
                           border: 1px solid #334155; border-radius: 4px;
                           padding: 4px; }
QSpinBox:focus, QDoubleSpinBox:focus { border-color: #533483; }
QLineEdit { background: #1e293b; color: #e0e0e0; border: 1px solid #334155;
            border-radius: 4px; padding: 5px 8px; }
QLineEdit:focus { border-color: #533483; }
QDockWidget { color: #e0e0e0; font-weight: bold; titlebar-close-icon: none; }
QDockWidget::title { background: #16213e; padding: 8px 12px;
                     border-bottom: 2px solid #533483; }
QSplitter::handle { background: #334155; height: 3px; }
QStatusBar { background: #16213e; color: #94a3b8; font-size: 11px;
             border-top: 1px solid #334155; }
QScrollBar:vertical { background: #1e293b; width: 8px; border-radius: 4px; }
QScrollBar::handle:vertical { background: #475569; border-radius: 4px;
                              min-height: 20px; }
QScrollBar::handle:vertical:hover { background: #533483; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QGraphicsView { border: 1px solid #334155; border-radius: 6px; }
)";

/* ================================================================== */
/*  Constructor / Destructor                                          */
/* ================================================================== */

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent), _weightMode(0), _simRunning(false),
	  _view(nullptr),
	  _scene(nullptr), _nodeList(nullptr), _edgeList(nullptr),
	  _trainList(nullptr), _eventList(nullptr), _logView(nullptr),
	  _runBtn(nullptr), _presetNetworkCombo(nullptr),
	  _presetTrainCombo(nullptr), _delNodeBtn(nullptr),
	  _delEdgeBtn(nullptr), _delTrainBtn(nullptr), _delEventBtn(nullptr),
	  _editTrainBtn(nullptr), _editEventBtn(nullptr), _worker(nullptr)
{
	setWindowTitle("Train Simulation");
	setStyleSheet(STYLESHEET);

	_scene = new NetworkScene(this);

	buildMenus();
	buildToolbar();
	buildCentralWidget();
	buildSidebar();
	scanPresetFiles();

	/* Worker thread setup */
	_worker = new SimulationWorker;
	_worker->moveToThread(&_simThread);
	qRegisterMetaType<QVector<TrainSnapshot>>("QVector<TrainSnapshot>");
	qRegisterMetaType<QVector<TrainStatRow>>("QVector<TrainStatRow>");

	connect(&_simThread, &QThread::finished, _worker, &QObject::deleteLater);
	connect(_worker, &SimulationWorker::tick,
			this, &MainWindow::onSimTick);
	connect(_worker, &SimulationWorker::finished,
			this, &MainWindow::onSimFinished);
	connect(_worker, &SimulationWorker::error,
			this, &MainWindow::onSimError);
	connect(_worker, &SimulationWorker::runProgress,
			this, &MainWindow::onRunProgress);
	connect(_worker, &SimulationWorker::multiRunFinished,
			this, &MainWindow::onMultiRunFinished);
	_simThread.start();

	/* ── Sensible initial window size ── */
	resize(1400, 900);

	logInfo("Welcome! Load a network preset or create nodes to get started.");
	statusBar()->showMessage("Ready");
}

MainWindow::~MainWindow()
{
	_simThread.quit();
	_simThread.wait();
	_worker = nullptr; /* deleted by deleteLater on thread finish */

	/* Clean up temporary simulation files */
	QFile::remove(QDir::tempPath() + "/trainsim_network.txt");
	QFile::remove(QDir::tempPath() + "/trainsim_trains.txt");
}

/* ================================================================== */
/*  Logging helpers                                                   */
/* ================================================================== */

void MainWindow::logInfo(const QString &msg)
{
	QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
	_logView->append(
		QString("<span style='color:#94a3b8;'>[%1]</span> %2")
			.arg(ts, msg));
}

void MainWindow::logError(const QString &msg)
{
	QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
	_logView->append(
		QString("<span style='color:#94a3b8;'>[%1]</span> "
				"<span style='color:#f87171;'>ERROR:</span> "
				"<span style='color:#fca5a5;'>%2</span>")
			.arg(ts, msg));
}

void MainWindow::logSuccess(const QString &msg)
{
	QString ts = QDateTime::currentDateTime().toString("hh:mm:ss");
	_logView->append(
		QString("<span style='color:#94a3b8;'>[%1]</span> "
				"<span style='color:#6ee7b7;'>%2</span>")
			.arg(ts, msg.toHtmlEscaped()));
}

/* ================================================================== */
/*  Validation helpers                                                */
/* ================================================================== */

bool MainWindow::validateNodeName(const QString &name,
								  QString &errorOut) const
{
	if (name.isEmpty())
	{
		errorOut = "Node name cannot be empty.";
		return false;
	}
	if (name.length() > 64)
	{
		errorOut = "Node name is too long (max 64 characters).";
		return false;
	}
	if (name.contains(' ') || name.contains('\t'))
	{
		errorOut = "Node name cannot contain spaces or tabs "
				  "(the file format uses spaces as delimiters).";
		return false;
	}
	/* Reject characters that could break parsing */
	static const QRegularExpression badChars("[<>|\\\"]");
	if (name.contains(badChars))
	{
		errorOut = "Node name contains invalid characters (<, >, |, \\\", \\).";
		return false;
	}
	return true;
}

bool MainWindow::isSimRunning() const
{
	return _simRunning;
}

bool MainWindow::guardSimRunning(const QString &action)
{
	if (!_simRunning)
		return false;
	logError("Cannot " + action + " while simulation is running.");
	QMessageBox::warning(this, "Simulation Running",
		"Cannot " + action + " while the simulation is in progress.\n"
		"Click Stop to abort the simulation first.");
	return true;
}

/* ================================================================== */
/*  UI construction                                                   */
/* ==================================================================  */

void MainWindow::buildMenus()
{
	QMenu *file = menuBar()->addMenu("&File");
	file->addAction("Import All...", QKeySequence("Ctrl+I"),
					this, &MainWindow::onImportAll);
	file->addSeparator();
	file->addAction("Load Network...", QKeySequence("Ctrl+O"),
					this, &MainWindow::onLoadNetwork);
	file->addAction("Load Trains...", QKeySequence("Ctrl+T"),
					this, &MainWindow::onLoadTrains);
	file->addSeparator();
	file->addAction("Save Network...", this, &MainWindow::onSaveNetwork);
	file->addAction("Save Trains...", this, &MainWindow::onSaveTrains);
	file->addSeparator();
	file->addAction("Export All...", QKeySequence("Ctrl+E"),
					this, &MainWindow::onExportAll);
	file->addAction("Export as DOT...", QKeySequence("Ctrl+G"),
					this, &MainWindow::onExportDot);
	file->addSeparator();
	file->addAction("Quit", QKeySequence("Ctrl+Q"), this, &QWidget::close);

	QMenu *edit = menuBar()->addMenu("&Edit");
	edit->addAction("Add Node...", this, &MainWindow::onAddNode);
	edit->addAction("Add Edge...", this, &MainWindow::onAddEdge);
	edit->addAction("Add Train...", this, &MainWindow::onAddTrain);
	edit->addAction("Add Event...", this, &MainWindow::onAddEvent);
	edit->addSeparator();
	edit->addAction("Edit Selected Node...", this, &MainWindow::onEditNode);
	edit->addAction("Edit Selected Edge...", this, &MainWindow::onEditEdge);
	edit->addAction("Edit Selected Train...", this, &MainWindow::onEditTrain);
	edit->addAction("Edit Selected Event...", this, &MainWindow::onEditEvent);
	edit->addSeparator();
	edit->addAction("Delete Selected Node", this, &MainWindow::onDeleteNode);
	edit->addAction("Delete Selected Edge", this, &MainWindow::onDeleteEdge);
	edit->addAction("Delete Selected Train", this, &MainWindow::onDeleteTrain);
	edit->addAction("Delete Selected Event", this, &MainWindow::onDeleteEvent);

	QMenu *sim = menuBar()->addMenu("&Simulation");
	sim->addAction("Run", QKeySequence("Ctrl+R"),
				   this, &MainWindow::onRunSimulation);

	QMenu *help = menuBar()->addMenu("&Help");
	help->addAction("Controls && Shortcuts...", QKeySequence("F1"),
					this, [this]() {
		QMessageBox box(this);
		box.setWindowTitle("Controls & Shortcuts");
		box.setIcon(QMessageBox::Information);
		box.setTextFormat(Qt::RichText);
		box.setText(
			"<h3>Keyboard Shortcuts</h3>"
			"<table cellpadding='4'>"
			"<tr><td><b>Ctrl+I</b></td><td>Import All (network + trains)</td></tr>"
			"<tr><td><b>Ctrl+O</b></td><td>Load Network file</td></tr>"
			"<tr><td><b>Ctrl+T</b></td><td>Load Train file</td></tr>"
			"<tr><td><b>Ctrl+E</b></td><td>Export All to directory</td></tr>"
			"<tr><td><b>Ctrl+G</b></td><td>Export as Graphviz DOT</td></tr>"
			"<tr><td><b>Ctrl+R</b></td><td>Run simulation</td></tr>"
			"<tr><td><b>Ctrl+Q</b></td><td>Quit</td></tr>"
			"<tr><td><b>F1</b></td><td>This help dialog</td></tr>"
			"</table>"
			"<h3>Graph Controls</h3>"
			"<ul>"
			"<li><b>Drag nodes</b> — click and drag any node to reposition it</li>"
			"<li><b>Pan</b> — click and drag on empty space to scroll the view</li>"
			"<li><b>Zoom</b> — mouse wheel to zoom in/out</li>"
			"</ul>"
			"<h3>Workflow</h3>"
			"<ol>"
			"<li>Load or create a <b>network</b> (nodes + edges)</li>"
			"<li>Load or create <b>trains</b> (with valid routes)</li>"
			"<li>Optionally add <b>events</b> (delays, disruptions)</li>"
			"<li>Press <b>Run Simulation</b> (Ctrl+R)</li>"
			"<li>Watch the animated trains and check results in the log</li>"
			"</ol>"
			"<h3>Sidebar</h3>"
			"<p>Use the <b>tabs</b> (Nodes, Edges, Trains, Events) to view, add, "
			"edit, and delete elements. Select an item and click Edit or Delete.</p>"
			"<p>Use <b>Quick Load Presets</b> to load example networks and trains.</p>"
		);
		box.exec();
	});
	help->addAction("About...", this, [this]() {
		QMessageBox::about(this, "About Train Simulation",
			"<h3>Train Simulation GUI</h3>"
			"<p>Module 05 — Train yourself</p>"
			"<p>42 School Project</p>"
			"<p>Built with Qt 6 and C++17</p>");
	});
}

void MainWindow::buildToolbar()
{
	QToolBar *tb = addToolBar("Main");
	tb->setMovable(false);
	tb->setIconSize(QSize(16, 16));

	_runBtn = new QPushButton("  Run Simulation  ");
	_runBtn->setStyleSheet(
		"QPushButton { background: #059669; font-size: 13px; padding: 8px 20px; }"
		"QPushButton:hover { background: #10b981; }"
		"QPushButton:disabled { background: #374151; color: #6b7280; }");
	connect(_runBtn, &QPushButton::clicked, this, &MainWindow::onRunSimulation);
	tb->addWidget(_runBtn);

	_stopBtn = new QPushButton("  Stop  ");
	_stopBtn->setStyleSheet(
		"QPushButton { background: #dc2626; font-size: 13px; padding: 8px 16px; }"
		"QPushButton:hover { background: #ef4444; }"
		"QPushButton:disabled { background: #374151; color: #6b7280; }");
	_stopBtn->setEnabled(false);
	connect(_stopBtn, &QPushButton::clicked, this, &MainWindow::onStopSimulation);
	tb->addWidget(_stopBtn);

	tb->addSeparator();

	/* ── Runs spinbox ── */
	auto *runsCaption = new QLabel("  Runs: ");
	runsCaption->setStyleSheet("QLabel { color: #94a3b8; font-size: 12px; }");
	tb->addWidget(runsCaption);

	_runsSpinBox = new QSpinBox;
	_runsSpinBox->setRange(1, 100);
	_runsSpinBox->setValue(1);
	_runsSpinBox->setToolTip("Number of simulation runs (multi-run Monte Carlo)");
	_runsSpinBox->setFixedWidth(60);
	_runsSpinBox->setStyleSheet(
		"QSpinBox { background: #1e293b; color: #e0e0e0; "
		"border: 1px solid #334155; border-radius: 4px; padding: 4px; "
		"font-size: 12px; }"
		"QSpinBox:focus { border-color: #533483; }");
	tb->addWidget(_runsSpinBox);

	/* ── Animate checkbox ── */
	_animateCheck = new QCheckBox("Animate");
	_animateCheck->setChecked(true);
	_animateCheck->setToolTip(
		"When multi-run: animate the first run, then fast-run the rest.\n"
		"Uncheck for maximum speed (no animation at all).");
	_animateCheck->setStyleSheet(
		"QCheckBox { color: #94a3b8; font-size: 12px; spacing: 4px; }"
		"QCheckBox::indicator { width: 14px; height: 14px; "
		"border-radius: 3px; border: 1px solid #475569; }"
		"QCheckBox::indicator:unchecked { background: #1e293b; }"
		"QCheckBox::indicator:checked { background: #059669; "
		"border-color: #059669; }");
	tb->addWidget(_animateCheck);

	tb->addSeparator();

	/* ── Path weight mode ── */
	auto *pathCaption = new QLabel("  Path: ");
	pathCaption->setStyleSheet("QLabel { color: #94a3b8; font-size: 12px; }");
	tb->addWidget(pathCaption);

	_pathWeightCombo = new QComboBox;
	_pathWeightCombo->addItem("Distance");
	_pathWeightCombo->addItem("Time");
	_pathWeightCombo->addItem("Congestion");
	_pathWeightCombo->setCurrentIndex(0);
	_pathWeightCombo->setToolTip(
		"Pathfinding weight:\n"
		"  Distance   — shortest path by km\n"
		"  Time       — fastest path (accounts for speed limits)\n"
		"  Congestion — dynamic re-routing around occupied segments");
	_pathWeightCombo->setFixedWidth(100);
	_pathWeightCombo->setStyleSheet(
		"QComboBox { background: #1e293b; color: #e0e0e0; "
		"border: 1px solid #334155; border-radius: 4px; padding: 4px 8px; "
		"font-size: 12px; }"
		"QComboBox:focus { border-color: #533483; }"
		"QComboBox::drop-down { border: none; }"
		"QComboBox::down-arrow { image: none; }"
		"QComboBox QAbstractItemView { background: #1e293b; color: #e0e0e0; "
		"selection-background-color: #533483; border: 1px solid #334155; }");
	connect(_pathWeightCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
			this, [this](int idx) { _weightMode = idx; });
	tb->addWidget(_pathWeightCombo);

	tb->addSeparator();

	/* ── Speed slider ── */
	auto *spdCaption = new QLabel("  Speed: ");
	spdCaption->setStyleSheet("QLabel { color: #94a3b8; font-size: 12px; }");
	tb->addWidget(spdCaption);

	_speedSlider = new QSlider(Qt::Horizontal);
	_speedSlider->setRange(0, 20);   // logarithmic 0.1× … 10×
	_speedSlider->setValue(10);       // center = 1.0×
	_speedSlider->setTickPosition(QSlider::TicksBelow);
	_speedSlider->setTickInterval(5);
	_speedSlider->setFixedWidth(200);
	_speedSlider->setStyleSheet(
		"QSlider::groove:horizontal {"
		"  background: #334155; height: 6px; border-radius: 3px; }"
		"QSlider::handle:horizontal {"
		"  background: #06b6d4; width: 14px; margin: -4px 0;"
		"  border-radius: 7px; }"
		"QSlider::sub-page:horizontal {"
		"  background: #0891b2; border-radius: 3px; }");
	connect(_speedSlider, &QSlider::valueChanged,
			this, &MainWindow::onSpeedChanged);
	tb->addWidget(_speedSlider);

	_speedLabel = new QLabel(" 1.0×");
	_speedLabel->setFixedWidth(48);
	_speedLabel->setStyleSheet(
		"QLabel { color: #22d3ee; font-weight: bold; font-size: 12px; }");
	tb->addWidget(_speedLabel);

	tb->addSeparator();

	/* ── Fit-graph button ── */
	auto *fitBtn = new QPushButton("  Fit Graph  ");
	fitBtn->setStyleSheet(
		"QPushButton { background: #3b82f6; font-size: 13px; padding: 8px 16px; }"
		"QPushButton:hover { background: #60a5fa; }");
	connect(fitBtn, &QPushButton::clicked, this, &MainWindow::onFitGraph);
	tb->addWidget(fitBtn);
}

void MainWindow::buildCentralWidget()
{
	auto *vSplit = new QSplitter(Qt::Vertical, this);

	/* ── Top: graph + live dashboard side by side ── */
	_hSplit = new QSplitter(Qt::Horizontal);

	_view = new ZoomableView(_scene);
	_view->setStyleSheet(
		"QGraphicsView { background: #1a1a2e; border: 1px solid #334155;"
		"  border-radius: 6px; }");
	_hSplit->addWidget(_view);

	_dashboard = new SimDashboard;
	_hSplit->addWidget(_dashboard);

	_hSplit->setStretchFactor(0, 5);   /* graph gets lion's share  */
	_hSplit->setStretchFactor(1, 0);   /* dashboard keeps its size */
	_hSplit->setSizes({1200, DASH_LIVE_W}); /* initial preset      */
	vSplit->addWidget(_hSplit);

	/* ── Bottom: log view ── */
	_logView = new QTextEdit;
	_logView->setReadOnly(true);
	_logView->setMaximumHeight(180);
	vSplit->addWidget(_logView);

	vSplit->setStretchFactor(0, 4);
	vSplit->setStretchFactor(1, 1);
	setCentralWidget(vSplit);
}

void MainWindow::buildSidebar()
{
	auto *dock = new QDockWidget("Network Elements", this);
	dock->setAllowedAreas(Qt::RightDockWidgetArea);
	dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
	dock->setMinimumWidth(280);

	auto *container = new QWidget;
	auto *mainLay = new QVBoxLayout(container);
	mainLay->setContentsMargins(8, 8, 8, 8);
	mainLay->setSpacing(6);

	/* ── Preset combos ── */
	auto *presetGroup = new QGroupBox("Quick Load Presets");
	auto *presetLay = new QVBoxLayout(presetGroup);

	auto *netLabel = new QLabel("Network:");
	netLabel->setStyleSheet("font-size: 11px; color: #94a3b8;");
	_presetNetworkCombo = new QComboBox;
	_presetNetworkCombo->addItem("-- select preset --");
	connect(_presetNetworkCombo, QOverload<int>::of(&QComboBox::activated),
			this, &MainWindow::onLoadPresetNetwork);

	auto *trainLabel = new QLabel("Trains:");
	trainLabel->setStyleSheet("font-size: 11px; color: #94a3b8;");
	_presetTrainCombo = new QComboBox;
	_presetTrainCombo->addItem("-- select preset --");
	connect(_presetTrainCombo, QOverload<int>::of(&QComboBox::activated),
			this, &MainWindow::onLoadPresetTrains);

	presetLay->addWidget(netLabel);
	presetLay->addWidget(_presetNetworkCombo);
	presetLay->addWidget(trainLabel);
	presetLay->addWidget(_presetTrainCombo);
	mainLay->addWidget(presetGroup);

	/* ── Tab widget for the 4 element lists ── */
	auto *tabs = new QTabWidget;

	/* --- Nodes tab --- */
	auto *nodeTab = new QWidget;
	auto *nodeLay = new QVBoxLayout(nodeTab);
	nodeLay->setContentsMargins(4, 4, 4, 4);
	_nodeList = new QListWidget;
	nodeLay->addWidget(_nodeList);
	auto *nodeActions = new QHBoxLayout;
	auto *addNodeBtn = new QPushButton("+ Add");
	connect(addNodeBtn, &QPushButton::clicked, this, &MainWindow::onAddNode);
	_editNodeBtn = new QPushButton("Edit");
	connect(_editNodeBtn, &QPushButton::clicked, this, &MainWindow::onEditNode);
	_delNodeBtn = new QPushButton("Delete");
	_delNodeBtn->setProperty("danger", true);
	connect(_delNodeBtn, &QPushButton::clicked, this, &MainWindow::onDeleteNode);
	nodeActions->addWidget(addNodeBtn);
	nodeActions->addWidget(_editNodeBtn);
	nodeActions->addWidget(_delNodeBtn);
	nodeLay->addLayout(nodeActions);
	tabs->addTab(nodeTab, "Nodes");

	/* --- Edges tab --- */
	auto *edgeTab = new QWidget;
	auto *edgeLay = new QVBoxLayout(edgeTab);
	edgeLay->setContentsMargins(4, 4, 4, 4);
	_edgeList = new QListWidget;
	edgeLay->addWidget(_edgeList);
	auto *edgeActions = new QHBoxLayout;
	auto *addEdgeBtn = new QPushButton("+ Add");
	connect(addEdgeBtn, &QPushButton::clicked, this, &MainWindow::onAddEdge);
	_editEdgeBtn = new QPushButton("Edit");
	connect(_editEdgeBtn, &QPushButton::clicked, this, &MainWindow::onEditEdge);
	_delEdgeBtn = new QPushButton("Delete");
	_delEdgeBtn->setProperty("danger", true);
	connect(_delEdgeBtn, &QPushButton::clicked, this, &MainWindow::onDeleteEdge);
	edgeActions->addWidget(addEdgeBtn);
	edgeActions->addWidget(_editEdgeBtn);
	edgeActions->addWidget(_delEdgeBtn);
	edgeLay->addLayout(edgeActions);
	tabs->addTab(edgeTab, "Edges");

	/* --- Trains tab --- */
	auto *trainTab = new QWidget;
	auto *trainLay = new QVBoxLayout(trainTab);
	trainLay->setContentsMargins(4, 4, 4, 4);
	_trainList = new QListWidget;
	trainLay->addWidget(_trainList);
	auto *trainActions = new QHBoxLayout;
	auto *addTrainBtn = new QPushButton("+ Add");
	connect(addTrainBtn, &QPushButton::clicked, this, &MainWindow::onAddTrain);
	_editTrainBtn = new QPushButton("Edit");
	connect(_editTrainBtn, &QPushButton::clicked, this, &MainWindow::onEditTrain);
	_delTrainBtn = new QPushButton("Delete");
	_delTrainBtn->setProperty("danger", true);
	connect(_delTrainBtn, &QPushButton::clicked, this, &MainWindow::onDeleteTrain);
	trainActions->addWidget(addTrainBtn);
	trainActions->addWidget(_editTrainBtn);
	trainActions->addWidget(_delTrainBtn);
	trainLay->addLayout(trainActions);
	tabs->addTab(trainTab, "Trains");

	/* --- Events tab --- */
	auto *eventTab = new QWidget;
	auto *eventLay = new QVBoxLayout(eventTab);
	eventLay->setContentsMargins(4, 4, 4, 4);
	_eventList = new QListWidget;
	eventLay->addWidget(_eventList);
	auto *eventActions = new QHBoxLayout;
	auto *addEventBtn = new QPushButton("+ Add");
	connect(addEventBtn, &QPushButton::clicked, this, &MainWindow::onAddEvent);
	_editEventBtn = new QPushButton("Edit");
	connect(_editEventBtn, &QPushButton::clicked, this, &MainWindow::onEditEvent);
	_delEventBtn = new QPushButton("Delete");
	_delEventBtn->setProperty("danger", true);
	connect(_delEventBtn, &QPushButton::clicked, this, &MainWindow::onDeleteEvent);
	eventActions->addWidget(addEventBtn);
	eventActions->addWidget(_editEventBtn);
	eventActions->addWidget(_delEventBtn);
	eventLay->addLayout(eventActions);
	tabs->addTab(eventTab, "Events");

	mainLay->addWidget(tabs, 1);
	dock->setWidget(container);
	addDockWidget(Qt::RightDockWidgetArea, dock);
}

/* ================================================================== */
/*  Preset scanning                                                   */
/* ================================================================== */

void MainWindow::scanPresetFiles()
{
	/* Resolve project root from the binary location so that preset
	   scanning works regardless of how the app was launched (open,
	   double-click, terminal, etc.).
	   On macOS the binary lives inside a .app bundle:
	       bin/TrainGUI.app/Contents/MacOS/TrainGUI  →  4 cdUp()
	   On Linux the binary is a plain ELF:
	       bin/TrainGUI                               →  1 cdUp()  */
	QDir root(QCoreApplication::applicationDirPath());
#ifdef Q_OS_MACOS
	root.cdUp(); // MacOS -> Contents
	root.cdUp(); // Contents -> TrainGUI.app
	root.cdUp(); // TrainGUI.app -> bin
#endif
	root.cdUp(); // bin -> project root

	/* Scan network presets (skip files with known-bad names) */
	QDir netDir(root.filePath("input/railNetworkPrintFolder"));
	if (netDir.exists())
	{
		auto files = netDir.entryList({"*.txt"}, QDir::Files, QDir::Name);
		for (const auto &f : files)
		{
			/* Only show "good" or region-based files as presets */
			if (f.contains("Bad") || f.contains("Disconnected")
				|| f.contains("Duplicate") || f.contains("Empty")
				|| f.contains("Missing") || f.contains("Negative")
				|| f.contains("Quoted") || f.contains("SelfLoop")
				|| f.contains("Unknown") || f.contains("Zero"))
				continue;
			QString display = f;
			display.remove("railNetwork").remove(".txt");
			_presetNetworkCombo->addItem(display, netDir.filePath(f));
		}
	}

	/* Scan train presets */
	QDir trainDir(root.filePath("input/trainPrintFolder"));
	if (trainDir.exists())
	{
		auto files = trainDir.entryList({"*.txt"}, QDir::Files, QDir::Name);
		for (const auto &f : files)
		{
			if (f.contains("Bad") || f.contains("Empty")
				|| f.contains("Incomplete") || f.contains("Unreachable"))
				continue;
			QString display = f;
			display.remove("trainPrint").remove(".txt");
			_presetTrainCombo->addItem(display, trainDir.filePath(f));
		}
	}
}

/* ================================================================== */
/*  Refresh sidebar lists                                             */
/* ================================================================== */

void MainWindow::refreshLists()
{
	_nodeList->clear();
	for (const auto &name : _network.getNodeNames())
		_nodeList->addItem(QString::fromStdString(name));

	_edgeList->clear();
	for (const auto &name : _network.getNodeNames())
	{
		for (const auto &e : _network.getNeighbours(name))
		{
			QString from = QString::fromStdString(name);
			QString to = QString::fromStdString(e.destination->getName());
			if (from < to)
				_edgeList->addItem(
					QString("%1 <-> %2  (%3 km, %4 km/h)")
						.arg(from, to)
						.arg(e.distance, 0, 'f', 1)
						.arg(e.speedLimit, 0, 'f', 0));
		}
	}

	_trainList->clear();
	for (size_t i = 0; i < _trainDefs.size(); ++i)
	{
		const auto &t = _trainDefs[i];
		int h = static_cast<int>(t->getDepartureTime()) / 3600;
		int m = (static_cast<int>(t->getDepartureTime()) % 3600) / 60;
		_trainList->addItem(
			QString("[%1] %2 : %3 -> %4 @ %5h%6")
				.arg(i)
				.arg(QString::fromStdString(t->getName()),
					 QString::fromStdString(t->getDepartureStation()),
					 QString::fromStdString(t->getArrivalStation()))
				.arg(h, 2, 10, QChar('0'))
				.arg(m, 2, 10, QChar('0')));
	}

	_eventList->clear();
	for (size_t i = 0; i < _eventDefs.size(); ++i)
	{
		const auto &ev = _eventDefs[i];
		_eventList->addItem(
			QString("[%1] %2 (p=%3, %4min) at %5")
				.arg(i)
				.arg(QString::fromStdString(ev.getName()))
				.arg(ev.getProbability(), 0, 'f', 2)
				.arg(ev.getDuration() / 60.0, 0, 'f', 1)
				.arg(QString::fromStdString(ev.getNodeName())));
	}

	statusBar()->showMessage(
		QString("Nodes: %1  |  Edges: %2  |  Trains: %3  |  Events: %4")
			.arg(_nodeList->count())
			.arg(_edgeList->count())
			.arg(_trainList->count())
			.arg(_eventList->count()));
}

/* ================================================================== */
/*  File loading / saving                                             */
/* ================================================================== */

void MainWindow::onLoadNetwork()
{
	if (guardSimRunning("load a network"))
		return;

	QString path = QFileDialog::getOpenFileName(
		this, "Load Network File", "input/railNetworkPrintFolder",
		"Text files (*.txt);;All files (*)");
	if (path.isEmpty())
		return;

	try
	{
		RailNetwork net;
		std::vector<Event> events;
		InputHandler::parseNetworkFile(path.toStdString(), net, events);
		_network = std::move(net);
		_eventDefs = std::move(events);
		_networkFilePath = path;

		_scene->loadFromNetwork(_network);
		_view->fitInView(_scene->sceneRect().adjusted(-60, -60, 60, 60),
						 Qt::KeepAspectRatio);
		refreshLists();
		logSuccess("Loaded network: " + QFileInfo(path).fileName());
	}
	catch (const std::exception &e)
	{
		logError("Failed to load network: " + QString::fromStdString(e.what()));
		QMessageBox::critical(this, "Network Load Error",
			QString("Could not load network file.\n\n"
					"File: %1\n"
					"Reason: %2\n\n"
					"Make sure the file follows the correct format:\n"
					"  Node <name>\n"
					"  Rail <from> <to> <distance> <speed>\n"
					"  Event <name> <probability> <duration> <node>")
				.arg(QFileInfo(path).fileName(),
					 QString::fromStdString(e.what())));
	}
}

void MainWindow::onLoadTrains()
{
	if (guardSimRunning("load trains"))
		return;

	QString path = QFileDialog::getOpenFileName(
		this, "Load Train File", "input/trainPrintFolder",
		"Text files (*.txt);;All files (*)");
	if (path.isEmpty())
		return;

	try
	{
		auto trains = InputHandler::parseTrainFile(path.toStdString());
		_trainDefs = std::move(trains);
		_trainFilePath = path;
		refreshLists();
		logSuccess("Loaded " + QString::number(_trainDefs.size())
				   + " train(s) from " + QFileInfo(path).fileName());
		if (_network.getNodeNames().empty())
			logInfo("Note: no network loaded yet — load one before running.");
	}
	catch (const std::exception &e)
	{
		logError("Failed to load trains: " + QString::fromStdString(e.what()));
		QMessageBox::critical(this, "Train Load Error",
			QString("Could not load train file.\n\n"
					"File: %1\n"
					"Reason: %2\n\n"
					"Make sure the file format is:\n"
					"  <name> <weight> <friction> <accel> <brake> "
					"<from> <to> <departure> <stopDuration>")
				.arg(QFileInfo(path).fileName(),
					 QString::fromStdString(e.what())));
	}
}

void MainWindow::onLoadPresetNetwork(int index)
{
	if (guardSimRunning("load a preset"))
		return;

	if (index <= 0)
		return;
	QString path = _presetNetworkCombo->itemData(index).toString();
	if (path.isEmpty())
		return;

	try
	{
		RailNetwork net;
		std::vector<Event> events;
		InputHandler::parseNetworkFile(path.toStdString(), net, events);
		_network = std::move(net);
		_eventDefs = std::move(events);
		_networkFilePath = path;

		_scene->loadFromNetwork(_network);
		_view->fitInView(_scene->sceneRect().adjusted(-60, -60, 60, 60),
						 Qt::KeepAspectRatio);
		refreshLists();
		logSuccess("Loaded preset network: "
				   + _presetNetworkCombo->itemText(index));
	}
	catch (const std::exception &e)
	{
		logError("Preset load failed: " + QString::fromStdString(e.what()));
	}
}

void MainWindow::onLoadPresetTrains(int index)
{
	if (guardSimRunning("load a preset"))
		return;

	if (index <= 0)
		return;

	QString path = _presetTrainCombo->itemData(index).toString();
	if (path.isEmpty())
		return;

	try
	{
		auto trains = InputHandler::parseTrainFile(path.toStdString());
		_trainDefs = std::move(trains);
		_trainFilePath = path;
		refreshLists();
		logSuccess("Loaded preset trains: "
				   + _presetTrainCombo->itemText(index)
				   + " (" + QString::number(_trainDefs.size()) + " trains)");
		if (_network.getNodeNames().empty())
			logInfo("Note: no network loaded yet — load one before running.");
	}
	catch (const std::exception &e)
	{
		logError("Preset load failed: " + QString::fromStdString(e.what()));
		QMessageBox::warning(this, "Preset Error",
			QString("Could not parse this train preset.\n\n"
					"Reason: %1")
				.arg(QString::fromStdString(e.what())));
	}
}

void MainWindow::onSaveNetwork()
{
	if (_network.getNodeNames().empty())
	{
		logError("Nothing to save — the network is empty.");
		return;
	}

	QString path = QFileDialog::getSaveFileName(
		this, "Save Network File", "input/railNetworkPrintFolder",
		"Text files (*.txt)");
	if (path.isEmpty())
		return;

	QFile f(path);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		logError("Could not open file for writing: " + path);
		return;
	}
	QTextStream out(&f);
	for (const auto &name : _network.getNodeNames())
		out << "Node " << quoteIfNeeded(name) << "\n";
	for (const auto &name : _network.getNodeNames())
	{
		for (const auto &e : _network.getNeighbours(name))
		{
			QString from = quoteIfNeeded(name);
			QString to = quoteIfNeeded(e.destination->getName());
			if (from < to)
				out << "Rail " << from << " " << to << " "
					<< e.distance << " " << e.speedLimit << "\n";
		}
	}
	for (const auto &ev : _eventDefs)
	{
		out << "Event " << quoteIfNeeded(ev.getName())
			<< " " << ev.getProbability() << " "
			<< ev.getDuration() / 60.0 << "m "
			<< quoteIfNeeded(ev.getNodeName());
		if (ev.isRailEvent())
			out << " " << quoteIfNeeded(ev.getNodeName2());
		out << "\n";
	}
	logSuccess("Saved network to " + QFileInfo(path).fileName());
}

void MainWindow::onSaveTrains()
{
	if (_trainDefs.empty())
	{
		logError("Nothing to save — no trains defined.");
		return;
	}

	QString path = QFileDialog::getSaveFileName(
		this, "Save Train File", "input/trainPrintFolder",
		"Text files (*.txt)");
	if (path.isEmpty())
		return;

	QFile f(path);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		logError("Could not open file for writing: " + path);
		return;
	}
	QTextStream out(&f);
	for (const auto &t : _trainDefs)
	{
		int h = static_cast<int>(t->getDepartureTime()) / 3600;
		int m = (static_cast<int>(t->getDepartureTime()) % 3600) / 60;
		int sh = static_cast<int>(t->getStopDuration()) / 3600;
		int sm = (static_cast<int>(t->getStopDuration()) % 3600) / 60;
		out << quoteIfNeeded(t->getName()) << " "
			<< t->getWeight() << " "
			<< t->getFrictionCoefficient() << " "
			<< t->getMaxAccelForce() << " "
			<< t->getMaxBrakeForce() << " "
			<< quoteIfNeeded(t->getDepartureStation()) << " "
			<< quoteIfNeeded(t->getArrivalStation()) << " "
			<< QString("%1h%2").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0'))
			<< " "
			<< QString("%1h%2").arg(sh, 2, 10, QChar('0')).arg(sm, 2, 10, QChar('0'))
			<< "\n";
	}
	logSuccess("Saved " + QString::number(_trainDefs.size())
			   + " train(s) to " + QFileInfo(path).fileName());
}

/* ================================================================== */
/*  Import / Export All                                               */
/* ================================================================== */

void MainWindow::onImportAll()
{
	if (guardSimRunning("import files"))
		return;

	QDialog dlg(this);
	dlg.setWindowTitle("Import Network & Trains");
	dlg.setStyleSheet(styleSheet());
	dlg.setMinimumWidth(500);
	auto *form = new QFormLayout(&dlg);

	auto *netRow = new QHBoxLayout;
	auto *netEdit = new QLineEdit;
	netEdit->setPlaceholderText("Select a network file...");
	auto *netBrowse = new QPushButton("Browse...");
	netRow->addWidget(netEdit, 1);
	netRow->addWidget(netBrowse);
	form->addRow("Network file:", netRow);

	auto *trainRow = new QHBoxLayout;
	auto *trainEdit = new QLineEdit;
	trainEdit->setPlaceholderText("(optional) Select a train file...");
	auto *trainBrowse = new QPushButton("Browse...");
	trainRow->addWidget(trainEdit, 1);
	trainRow->addWidget(trainBrowse);
	form->addRow("Train file:", trainRow);

	auto *btns = new QHBoxLayout;
	auto *okBtn = new QPushButton("Import");
	auto *cancelBtn = new QPushButton("Cancel");
	cancelBtn->setStyleSheet("background: #475569;");
	btns->addWidget(cancelBtn);
	btns->addWidget(okBtn);
	form->addRow(btns);

	connect(netBrowse, &QPushButton::clicked, [&]() {
		QString p = QFileDialog::getOpenFileName(
			&dlg, "Select Network File", "input/railNetworkPrintFolder",
			"Text files (*.txt);;All files (*)");
		if (!p.isEmpty())
			netEdit->setText(p);
	});
	connect(trainBrowse, &QPushButton::clicked, [&]() {
		QString p = QFileDialog::getOpenFileName(
			&dlg, "Select Train File", "input/trainPrintFolder",
			"Text files (*.txt);;All files (*)");
		if (!p.isEmpty())
			trainEdit->setText(p);
	});
	connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
	connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

	if (dlg.exec() != QDialog::Accepted)
		return;

	QString netPath = netEdit->text().trimmed();
	QString trainPath = trainEdit->text().trimmed();

	if (netPath.isEmpty())
	{
		logError("A network file is required to import.");
		QMessageBox::warning(this, "Missing Network File",
			"You must select at least a network file to import.");
		return;
	}

	try
	{
		QString tmpTrain = trainPath.isEmpty()
								? writeTemporaryTrainFile()
								: trainPath;
		auto data = InputHandler::loadData(netPath.toStdString(),
										   tmpTrain.toStdString());
		_network = std::move(data.network);
		_eventDefs = std::move(data.events);
		_networkFilePath = netPath;

		if (!trainPath.isEmpty())
		{
			_trainDefs = std::move(data.trains);
			_trainFilePath = trainPath;
		}

		_scene->loadFromNetwork(_network);
		_view->fitInView(_scene->sceneRect().adjusted(-60, -60, 60, 60),
						 Qt::KeepAspectRatio);
		refreshLists();
		logSuccess("Imported network: " + QFileInfo(netPath).fileName());
		if (!trainPath.isEmpty())
			logSuccess("Imported " + QString::number(_trainDefs.size())
					   + " train(s) from " + QFileInfo(trainPath).fileName());
	}
	catch (const std::exception &e)
	{
		logError("Import failed: " + QString::fromStdString(e.what()));
		QMessageBox::critical(this, "Import Error",
			QString("Could not import the selected files.\n\n"
					"Reason: %1")
				.arg(QString::fromStdString(e.what())));
	}
}

void MainWindow::onExportAll()
{
	if (_network.getNodeNames().empty())
	{
		logError("Nothing to export — the network is empty.");
		QMessageBox::warning(this, "Nothing to Export",
			"Create or load a network before exporting.");
		return;
	}

	QString dir = QFileDialog::getExistingDirectory(
		this, "Choose Export Directory", "output",
		QFileDialog::ShowDirsOnly);
	if (dir.isEmpty())
		return;

	/* ---- Export network file ---- */
	QString netPath = dir + "/network.txt";
	{
		QFile f(netPath);
		if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			logError("Could not write: " + netPath);
			return;
		}
		QTextStream out(&f);
		for (const auto &name : _network.getNodeNames())
			out << "Node " << quoteIfNeeded(name) << "\n";
		for (const auto &name : _network.getNodeNames())
		{
			for (const auto &e : _network.getNeighbours(name))
			{
				QString from = quoteIfNeeded(name);
				QString to = quoteIfNeeded(
					e.destination->getName());
				if (from < to)
					out << "Rail " << from << " " << to << " "
						<< e.distance << " " << e.speedLimit << "\n";
			}
		}
		for (const auto &ev : _eventDefs)
		{
			out << "Event " << quoteIfNeeded(ev.getName())
				<< " " << ev.getProbability() << " "
				<< ev.getDuration() / 60.0 << "m "
				<< quoteIfNeeded(ev.getNodeName());
			if (ev.isRailEvent())
				out << " " << quoteIfNeeded(ev.getNodeName2());
			out << "\n";
		}
	}
	logSuccess("Exported network to " + netPath);

	/* ---- Export trains file ---- */
	if (!_trainDefs.empty())
	{
		QString trainPath = dir + "/trains.txt";
		QFile f(trainPath);
		if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
		{
			logError("Could not write: " + trainPath);
			return;
		}
		QTextStream out(&f);
		for (const auto &t : _trainDefs)
		{
			int h = static_cast<int>(t->getDepartureTime()) / 3600;
			int m = (static_cast<int>(t->getDepartureTime()) % 3600) / 60;
			int sh = static_cast<int>(t->getStopDuration()) / 3600;
			int sm = (static_cast<int>(t->getStopDuration()) % 3600) / 60;
			out << quoteIfNeeded(t->getName()) << " "
				<< t->getWeight() << " "
				<< t->getFrictionCoefficient() << " "
				<< t->getMaxAccelForce() << " "
				<< t->getMaxBrakeForce() << " "
				<< quoteIfNeeded(t->getDepartureStation()) << " "
				<< quoteIfNeeded(t->getArrivalStation()) << " "
				<< QString("%1h%2")
					   .arg(h, 2, 10, QChar('0'))
					   .arg(m, 2, 10, QChar('0'))
				<< " "
				<< QString("%1h%2")
					   .arg(sh, 2, 10, QChar('0'))
					   .arg(sm, 2, 10, QChar('0'))
				<< "\n";
		}
		logSuccess("Exported " + QString::number(_trainDefs.size())
				   + " train(s) to " + trainPath);
	}

	QMessageBox::information(this, "Export Complete",
		QString("Files exported to:\n  %1").arg(dir));
}

void MainWindow::onExportDot()
{
	if (_network.getNodeNames().empty())
	{
		logError("Nothing to export — the network is empty.");
		return;
	}

	QString path = QFileDialog::getSaveFileName(
		this, "Export Graphviz DOT File", "output/network.dot",
		"DOT files (*.dot);;All files (*)");
	if (path.isEmpty())
		return;

	try
	{
		GraphExporter::exportDot(path.toStdString(),
								 _network, _trainDefs);
		logSuccess("Exported DOT graph to " + QFileInfo(path).fileName());
	}
	catch (const std::exception &e)
	{
		logError("DOT export failed: " + QString::fromStdString(e.what()));
	}
}

/* ================================================================== */
/*  Create dialogs                                                    */
/* ================================================================== */

void MainWindow::onAddNode()
{
	if (guardSimRunning("add a node"))
		return;

	bool ok;
	QString name = QInputDialog::getText(
		this, "Add Node", "Enter node name (city / station):\n"
		"(no spaces, max 64 characters)",
		QLineEdit::Normal, "", &ok);
	if (!ok || name.trimmed().isEmpty())
		return;
	name = name.trimmed();

	QString nameError;
	if (!validateNodeName(name, nameError))
	{
		logError(nameError);
		QMessageBox::warning(this, "Invalid Node Name", nameError);
		return;
	}

	if (_scene->hasNode(name))
	{
		logError("Node '" + name + "' already exists.");
		QMessageBox::warning(this, "Duplicate Node",
			"A node with the name '" + name + "' already exists.\n"
			"Each node must have a unique name.");
		return;
	}

	try
	{
		_network.addNode(name.toStdString());
		_scene->addNode(name, QPointF(std::rand() % 400 - 200,
									  std::rand() % 400 - 200));
		refreshLists();
		logSuccess("Added node: " + name);
	}
	catch (const std::exception &e)
	{
		logError(QString::fromStdString(e.what()));
	}
}

void MainWindow::onAddEdge()
{
	if (guardSimRunning("add an edge"))
		return;

	QStringList names = _scene->nodeNames();
	if (names.size() < 2)
	{
		logError("Need at least 2 nodes to create an edge.");
		QMessageBox::information(this, "Cannot Add Edge",
			"You need at least 2 nodes before creating an edge.\n"
			"Add nodes first using '+ Node' or load a network preset.");
		return;
	}

	QDialog dlg(this);
	dlg.setWindowTitle("Add Edge (Rail Connection)");
	dlg.setStyleSheet(styleSheet());
	auto *form = new QFormLayout(&dlg);

	auto *fromBox = new QComboBox;
	fromBox->addItems(names);
	auto *toBox = new QComboBox;
	toBox->addItems(names);
	if (names.size() > 1)
		toBox->setCurrentIndex(1);
	auto *distSpin = new QDoubleSpinBox;
	distSpin->setRange(0.1, 99999.0);
	distSpin->setValue(10.0);
	distSpin->setSuffix(" km");
	auto *speedSpin = new QDoubleSpinBox;
	speedSpin->setRange(1.0, 999.0);
	speedSpin->setValue(200.0);
	speedSpin->setSuffix(" km/h");

	auto *btns = new QHBoxLayout;
	auto *okBtn = new QPushButton("Add Edge");
	auto *cancelBtn = new QPushButton("Cancel");
	cancelBtn->setStyleSheet("background: #475569;");
	btns->addWidget(cancelBtn);
	btns->addWidget(okBtn);

	form->addRow("From station:", fromBox);
	form->addRow("To station:", toBox);
	form->addRow("Distance:", distSpin);
	form->addRow("Speed limit:", speedSpin);
	form->addRow(btns);

	connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
	connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

	if (dlg.exec() != QDialog::Accepted)
		return;

	QString from = fromBox->currentText();
	QString to = toBox->currentText();
	if (from == to)
	{
		logError("Cannot connect a node to itself.");
		QMessageBox::warning(this, "Invalid Edge",
			"A rail connection must link two different stations.");
		return;
	}

	/* Check for duplicate edge */
	for (int i = 0; i < _edgeList->count(); ++i)
	{
		QString txt = _edgeList->item(i)->text();
		if ((txt.startsWith(from + " <-> " + to))
			|| (txt.startsWith(to + " <-> " + from)))
		{
			logError("An edge between '" + from + "' and '" + to
					 + "' already exists.");
			QMessageBox::warning(this, "Duplicate Edge",
				QString("A rail connection between '%1' and '%2' "
						"already exists.")
					.arg(from, to));
			return;
		}
	}

	try
	{
		_network.addConnection(from.toStdString(), to.toStdString(),
							   distSpin->value(), speedSpin->value());
		_scene->addEdge(from, to, distSpin->value(), speedSpin->value());
		refreshLists();
		logSuccess(QString("Added edge: %1 <-> %2 (%3 km, %4 km/h)")
					   .arg(from, to)
					   .arg(distSpin->value())
					   .arg(speedSpin->value()));
	}
	catch (const std::exception &e)
	{
		logError(QString::fromStdString(e.what()));
		QMessageBox::warning(this, "Edge Error",
			QString::fromStdString(e.what()));
	}
}

void MainWindow::onAddTrain()
{
	if (guardSimRunning("add a train"))
		return;

	QStringList names = _scene->nodeNames();
	if (names.size() < 2)
	{
		logError("Need at least 2 nodes to define a train route.");
		QMessageBox::information(this, "Cannot Add Train",
			"You need at least 2 nodes before creating a train.\n"
			"Create a network first.");
		return;
	}

	QDialog dlg(this);
	dlg.setWindowTitle("Add Train");
	dlg.setStyleSheet(styleSheet());
	auto *form = new QFormLayout(&dlg);

	auto *nameEdit = new QLineEdit("MyTrain");
	auto *weightSpin = new QDoubleSpinBox;
	weightSpin->setRange(1, 99999);
	weightSpin->setValue(80);
	weightSpin->setSuffix(" t");
	auto *fricSpin = new QDoubleSpinBox;
	fricSpin->setRange(0.001, 1.0);
	fricSpin->setDecimals(3);
	fricSpin->setValue(0.05);
	auto *accelSpin = new QDoubleSpinBox;
	accelSpin->setRange(1, 99999);
	accelSpin->setValue(356.0);
	accelSpin->setSuffix(" kN");
	auto *brakeSpin = new QDoubleSpinBox;
	brakeSpin->setRange(1, 99999);
	brakeSpin->setValue(30.0);
	brakeSpin->setSuffix(" kN");
	auto *fromBox = new QComboBox;
	fromBox->addItems(names);
	auto *toBox = new QComboBox;
	toBox->addItems(names);
	if (names.size() > 1)
		toBox->setCurrentIndex(1);
	auto *hourSpin = new QSpinBox;
	hourSpin->setRange(0, 23);
	hourSpin->setValue(14);
	auto *minSpin = new QSpinBox;
	minSpin->setRange(0, 59);
	minSpin->setValue(10);
	auto *stopSpin = new QSpinBox;
	stopSpin->setRange(0, 120);
	stopSpin->setValue(10);
	stopSpin->setSuffix(" min");

	auto *btns = new QHBoxLayout;
	auto *okBtn = new QPushButton("Add Train");
	auto *cancelBtn = new QPushButton("Cancel");
	cancelBtn->setStyleSheet("background: #475569;");
	btns->addWidget(cancelBtn);
	btns->addWidget(okBtn);

	form->addRow("Name:", nameEdit);
	form->addRow("Weight:", weightSpin);
	form->addRow("Friction coeff:", fricSpin);
	form->addRow("Accel force:", accelSpin);
	form->addRow("Brake force:", brakeSpin);
	form->addRow("Departure:", fromBox);
	form->addRow("Arrival:", toBox);
	form->addRow("Dep. hour:", hourSpin);
	form->addRow("Dep. minute:", minSpin);
	form->addRow("Stop duration:", stopSpin);
	form->addRow(btns);

	connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
	connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

	if (dlg.exec() != QDialog::Accepted)
		return;

	/* Validate train name */
	QString trainName = nameEdit->text().trimmed();
	if (trainName.isEmpty())
	{
		logError("Train name cannot be empty.");
		QMessageBox::warning(this, "Invalid Train Name",
			"Please provide a non-empty name for the train.");
		return;
	}
	if (trainName.contains(' ') || trainName.contains('\t'))
	{
		logError("Train name cannot contain spaces or tabs.");
		QMessageBox::warning(this, "Invalid Train Name",
			"Train names cannot contain spaces or tabs "
			"(used as delimiter in file format).");
		return;
	}

	if (fromBox->currentText() == toBox->currentText())
	{
		logError("Departure and arrival stations must be different.");
		QMessageBox::warning(this, "Invalid Train",
			"A train's departure and arrival stations must be different.");
		return;
	}

	/* Check route reachability */
	try
	{
		DijkstraPathfinding pathfinder;
		auto path = pathfinder.findPath(
			fromBox->currentText().toStdString(),
			toBox->currentText().toStdString(), _network);
		if (path.empty())
		{
			logError("No route found between '" + fromBox->currentText()
					 + "' and '" + toBox->currentText() + "'.");
			QMessageBox::warning(this, "Unreachable Route",
				QString("There is no path between '%1' and '%2'.\n\n"
						"Make sure the stations are connected by rail edges.")
					.arg(fromBox->currentText(), toBox->currentText()));
			return;
		}
	}
	catch (const std::exception &e)
	{
		logError("Route validation failed: "
				 + QString::fromStdString(e.what()));
		QMessageBox::warning(this, "Unreachable Route",
			QString("Cannot find a path: %1\n\n"
					"Make sure both stations exist and are connected.")
				.arg(QString::fromStdString(e.what())));
		return;
	}

	double depTime = hourSpin->value() * 3600.0 + minSpin->value() * 60.0;
	double stopDur = stopSpin->value() * 60.0;

	auto train = TrainFactory::createTrain(
		trainName.toStdString(),
		weightSpin->value(), fricSpin->value(),
		accelSpin->value(), brakeSpin->value(),
		fromBox->currentText().toStdString(),
		toBox->currentText().toStdString(),
		depTime, stopDur);
	_trainDefs.push_back(std::move(train));
	refreshLists();
	logSuccess("Added train: " + trainName);
}

void MainWindow::onAddEvent()
{
	if (guardSimRunning("add an event"))
		return;

	QStringList names = _scene->nodeNames();
	if (names.isEmpty())
	{
		logError("Need at least 1 node to define an event.");
		QMessageBox::information(this, "Cannot Add Event",
			"Create a network with nodes before adding events.");
		return;
	}

	QDialog dlg(this);
	dlg.setWindowTitle("Add Event");
	dlg.setStyleSheet(styleSheet());
	auto *form = new QFormLayout(&dlg);

	auto *nameEdit = new QLineEdit("Delay");
	auto *probSpin = new QDoubleSpinBox;
	probSpin->setRange(0.01, 1.0);
	probSpin->setDecimals(2);
	probSpin->setValue(0.3);
	auto *durSpin = new QDoubleSpinBox;
	durSpin->setRange(1, 9999);
	durSpin->setValue(30);
	durSpin->setSuffix(" min");
	auto *node1Box = new QComboBox;
	node1Box->addItems(names);
	auto *node2Box = new QComboBox;
	node2Box->addItem("(none - city event)");
	node2Box->addItems(names);

	auto *btns = new QHBoxLayout;
	auto *okBtn = new QPushButton("Add Event");
	auto *cancelBtn = new QPushButton("Cancel");
	cancelBtn->setStyleSheet("background: #475569;");
	btns->addWidget(cancelBtn);
	btns->addWidget(okBtn);

	form->addRow("Event name:", nameEdit);
	form->addRow("Probability:", probSpin);
	form->addRow("Duration:", durSpin);
	form->addRow("Node 1:", node1Box);
	form->addRow("Node 2 (rail):", node2Box);
	form->addRow(btns);

	connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
	connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

	if (dlg.exec() != QDialog::Accepted)
		return;

	/* Validate event name */
	QString evName = nameEdit->text().trimmed();
	if (evName.isEmpty())
	{
		logError("Event name cannot be empty.");
		QMessageBox::warning(this, "Invalid Event Name",
			"Please provide a non-empty name for the event.");
		return;
	}

	std::string n1 = node1Box->currentText().toStdString();
	std::string n2;
	if (node2Box->currentIndex() > 0)
	{
		n2 = node2Box->currentText().toStdString();
		if (n2 == n1)
		{
			logError("Node 2 cannot be the same as Node 1 for a rail event.");
			QMessageBox::warning(this, "Invalid Rail Event",
				"For a rail segment event, Node 1 and Node 2 must be "
				"different stations.");
			return;
		}
	}

	_eventDefs.emplace_back(evName.toStdString(),
							probSpin->value(),
							durSpin->value() * 60.0, n1, n2);
	refreshLists();
	logSuccess("Added event: " + evName);
}

/* ================================================================== */
/*  Delete operations                                                 */
/* ================================================================== */

void MainWindow::onDeleteNode()
{
	if (guardSimRunning("delete a node"))
		return;

	auto *item = _nodeList->currentItem();
	if (!item)
	{
		logError("Select a node in the Nodes tab to delete it.");
		return;
	}

	QString name = item->text();

	/* Count trains that will be invalidated */
	int affectedTrains = 0;
	for (const auto &t : _trainDefs)
		if (t->getDepartureStation() == name.toStdString()
			|| t->getArrivalStation() == name.toStdString())
			++affectedTrains;

	QString warning = QString("Delete node '%1' and all its connected edges?").arg(name);
	if (affectedTrains > 0)
		warning += QString("\n\nThis will also remove %1 train(s) that "
						   "use this station.").arg(affectedTrains);

	int ret = QMessageBox::question(this, "Delete Node", warning,
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (ret != QMessageBox::Yes)
		return;

	/* Remove from scene */
	_scene->removeNode(name);

	/* Rebuild RailNetwork from scene (simplest approach) */
	RailNetwork newNet;
	for (const auto &n : _scene->nodeNames())
		newNet.addNode(n.toStdString());

	/* We lost edge info from the old network, but scene still has it visually.
	   Rebuild by reading the edge list text (or just re-load from original). */
	/* Actually, the simplest: reload the network from scene's node names and
	   the old network's edges (minus the removed node). */
	auto oldNames = _network.getNodeNames();
	for (const auto &n : oldNames)
	{
		if (n == name.toStdString())
			continue;
		for (const auto &e : _network.getNeighbours(n))
		{
			QString from = QString::fromStdString(n);
			QString to = QString::fromStdString(e.destination->getName());
			if (to == name)
				continue;
			if (from < to) // avoid duplicates
			{
				try
				{
					newNet.addConnection(n, e.destination->getName(),
										 e.distance, e.speedLimit);
				}
				catch (...)
				{
				}
			}
		}
	}
	_network = std::move(newNet);

	/* Remove events referencing deleted node */
	_eventDefs.erase(
		std::remove_if(_eventDefs.begin(), _eventDefs.end(),
					   [&name](const Event &ev) {
						   return ev.getNodeName() == name.toStdString()
								  || ev.getNodeName2() == name.toStdString();
					   }),
		_eventDefs.end());

	/* Remove trains that reference deleted node */
	_trainDefs.erase(
		std::remove_if(_trainDefs.begin(), _trainDefs.end(),
					   [&name](const std::unique_ptr<Train> &t) {
						   return t->getDepartureStation() == name.toStdString()
								  || t->getArrivalStation() == name.toStdString();
					   }),
		_trainDefs.end());

	refreshLists();
	logSuccess("Deleted node: " + name);
}

void MainWindow::onDeleteEdge()
{
	if (guardSimRunning("delete an edge"))
		return;

	auto *item = _edgeList->currentItem();
	if (!item)
	{
		logError("Select an edge in the Edges tab to delete it.");
		return;
	}

	/* Parse "From <-> To  (... km, ... km/h)" */
	QString text = item->text();
	QStringList parts = text.split(" <-> ");
	if (parts.size() < 2)
		return;
	QString from = parts[0].trimmed();
	QString to = parts[1].split("  (").first().trimmed();

	int ret = QMessageBox::question(this, "Delete Edge",
		QString("Remove the rail connection between '%1' and '%2'?")
			.arg(from, to),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (ret != QMessageBox::Yes)
		return;

	_scene->removeEdge(from, to);

	/* Rebuild network without this edge */
	RailNetwork newNet;
	for (const auto &n : _network.getNodeNames())
		newNet.addNode(n);
	for (const auto &n : _network.getNodeNames())
	{
		for (const auto &e : _network.getNeighbours(n))
		{
			QString fn = QString::fromStdString(n);
			QString tn = QString::fromStdString(e.destination->getName());
			if (fn < tn)
			{
				if ((fn == from && tn == to) || (fn == to && tn == from))
					continue;
				try
				{
					newNet.addConnection(n, e.destination->getName(),
										 e.distance, e.speedLimit);
				}
				catch (...)
				{
				}
			}
		}
	}
	_network = std::move(newNet);
	refreshLists();
	logSuccess("Deleted edge: " + from + " <-> " + to);
}

void MainWindow::onDeleteTrain()
{
	if (guardSimRunning("delete a train"))
		return;

	int row = _trainList->currentRow();
	if (row < 0 || row >= static_cast<int>(_trainDefs.size()))
	{
		logError("Select a train in the Trains tab to delete it.");
		return;
	}

	QString name = QString::fromStdString(_trainDefs[row]->getName());
	int ret = QMessageBox::question(this, "Delete Train",
		QString("Remove train '%1'?").arg(name),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (ret != QMessageBox::Yes)
		return;

	_trainDefs.erase(_trainDefs.begin() + row);
	refreshLists();
	logSuccess("Deleted train: " + name);
}

void MainWindow::onDeleteEvent()
{
	if (guardSimRunning("delete an event"))
		return;

	int row = _eventList->currentRow();
	if (row < 0 || row >= static_cast<int>(_eventDefs.size()))
	{
		logError("Select an event in the Events tab to delete it.");
		return;
	}

	QString name = QString::fromStdString(_eventDefs[row].getName());
	int ret = QMessageBox::question(this, "Delete Event",
		QString("Remove event '%1'?").arg(name),
		QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
	if (ret != QMessageBox::Yes)
		return;

	_eventDefs.erase(_eventDefs.begin() + row);
	refreshLists();
	logSuccess("Deleted event: " + name);
}

/* ================================================================== */
/*  Edit operations                                                   */
/* ================================================================== */

void MainWindow::onEditNode()
{
	if (guardSimRunning("edit a node"))
		return;

	auto *item = _nodeList->currentItem();
	if (!item)
	{
		logError("Select a node in the Nodes tab to edit it.");
		return;
	}

	QString oldName = item->text();

	bool ok;
	QString newName = QInputDialog::getText(
		this, "Rename Node",
		QString("Rename node '%1' to:\n(no spaces, max 64 characters)")
			.arg(oldName),
		QLineEdit::Normal, oldName, &ok);
	if (!ok || newName.trimmed().isEmpty())
		return;
	newName = newName.trimmed();

	if (newName == oldName)
		return;

	QString nameError;
	if (!validateNodeName(newName, nameError))
	{
		logError(nameError);
		QMessageBox::warning(this, "Invalid Node Name", nameError);
		return;
	}

	if (_scene->hasNode(newName))
	{
		logError("Node '" + newName + "' already exists.");
		QMessageBox::warning(this, "Duplicate Node",
			"A node with the name '" + newName + "' already exists.\n"
			"Each node must have a unique name.");
		return;
	}

	/* Rebuild RailNetwork with the renamed node */
	RailNetwork newNet;
	for (const auto &n : _network.getNodeNames())
	{
		std::string nodeName = (n == oldName.toStdString())
								   ? newName.toStdString()
								   : n;
		newNet.addNode(nodeName);
	}
	for (const auto &n : _network.getNodeNames())
	{
		for (const auto &e : _network.getNeighbours(n))
		{
			QString from = QString::fromStdString(n);
			QString to = QString::fromStdString(e.destination->getName());
			if (from < to)
			{
				std::string f = (from == oldName) ? newName.toStdString()
												  : from.toStdString();
				std::string t = (to == oldName) ? newName.toStdString()
												: to.toStdString();
				try
				{
					newNet.addConnection(f, t, e.distance, e.speedLimit);
				}
				catch (...)
				{
				}
			}
		}
	}
	_network = std::move(newNet);

	/* Update scene visual */
	_scene->renameNode(oldName, newName);

	/* Update trains referencing old name */
	for (auto &t : _trainDefs)
	{
		bool changed = false;
		std::string dep = t->getDepartureStation();
		std::string arr = t->getArrivalStation();
		if (dep == oldName.toStdString())
		{
			dep = newName.toStdString();
			changed = true;
		}
		if (arr == oldName.toStdString())
		{
			arr = newName.toStdString();
			changed = true;
		}
		if (changed)
		{
			auto replacement = TrainFactory::createTrain(
				t->getName(), t->getWeight(), t->getFrictionCoefficient(),
				t->getMaxAccelForce(), t->getMaxBrakeForce(),
				dep, arr, t->getDepartureTime(), t->getStopDuration());
			t = std::move(replacement);
		}
	}

	/* Update events referencing old name */
	for (size_t i = 0; i < _eventDefs.size(); ++i)
	{
		auto &ev = _eventDefs[i];
		std::string n1 = ev.getNodeName();
		std::string n2 = ev.getNodeName2();
		bool changed = false;
		if (n1 == oldName.toStdString())
		{
			n1 = newName.toStdString();
			changed = true;
		}
		if (n2 == oldName.toStdString())
		{
			n2 = newName.toStdString();
			changed = true;
		}
		if (changed)
			_eventDefs[i] = Event(ev.getName(), ev.getProbability(),
								  ev.getDuration(), n1, n2);
	}

	refreshLists();
	logSuccess("Renamed node: " + oldName + " -> " + newName);
}

void MainWindow::onEditEdge()
{
	if (guardSimRunning("edit an edge"))
		return;

	auto *item = _edgeList->currentItem();
	if (!item)
	{
		logError("Select an edge in the Edges tab to edit it.");
		return;
	}

	/* Parse "From <-> To  (X.X km, Y km/h)" */
	QString text = item->text();
	QStringList parts = text.split(" <-> ");
	if (parts.size() < 2)
		return;
	QString from = parts[0].trimmed();
	QString remainder = parts[1];
	QString to = remainder.split("  (").first().trimmed();

	/* Find current values */
	double curDist = 10.0;
	double curSpeed = 200.0;
	for (const auto &e : _network.getNeighbours(from.toStdString()))
	{
		if (e.destination->getName() == to.toStdString())
		{
			curDist = e.distance;
			curSpeed = e.speedLimit;
			break;
		}
	}

	QDialog dlg(this);
	dlg.setWindowTitle("Edit Edge");
	dlg.setStyleSheet(styleSheet());
	auto *form = new QFormLayout(&dlg);

	auto *fromLabel = new QLabel(from);
	fromLabel->setStyleSheet("font-weight: bold;");
	auto *toLabel = new QLabel(to);
	toLabel->setStyleSheet("font-weight: bold;");
	auto *distSpin = new QDoubleSpinBox;
	distSpin->setRange(0.1, 99999.0);
	distSpin->setValue(curDist);
	distSpin->setSuffix(" km");
	auto *speedSpin = new QDoubleSpinBox;
	speedSpin->setRange(1.0, 999.0);
	speedSpin->setValue(curSpeed);
	speedSpin->setSuffix(" km/h");

	auto *btns = new QHBoxLayout;
	auto *okBtn = new QPushButton("Save Changes");
	auto *cancelBtn = new QPushButton("Cancel");
	cancelBtn->setStyleSheet("background: #475569;");
	btns->addWidget(cancelBtn);
	btns->addWidget(okBtn);

	form->addRow("From:", fromLabel);
	form->addRow("To:", toLabel);
	form->addRow("Distance:", distSpin);
	form->addRow("Speed limit:", speedSpin);
	form->addRow(btns);

	connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
	connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

	if (dlg.exec() != QDialog::Accepted)
		return;

	double newDist = distSpin->value();
	double newSpeed = speedSpin->value();

	/* Rebuild RailNetwork with updated edge */
	RailNetwork newNet;
	for (const auto &n : _network.getNodeNames())
		newNet.addNode(n);
	for (const auto &n : _network.getNodeNames())
	{
		for (const auto &e : _network.getNeighbours(n))
		{
			QString fn = QString::fromStdString(n);
			QString tn = QString::fromStdString(e.destination->getName());
			if (fn < tn)
			{
				double d = e.distance;
				double s = e.speedLimit;
				if ((fn == from && tn == to) || (fn == to && tn == from))
				{
					d = newDist;
					s = newSpeed;
				}
				try
				{
					newNet.addConnection(n, e.destination->getName(), d, s);
				}
				catch (...)
				{
				}
			}
		}
	}
	_network = std::move(newNet);

	/* Update scene visual */
	_scene->updateEdge(from, to, newDist, newSpeed);

	refreshLists();
	logSuccess(QString("Updated edge: %1 <-> %2 (%3 km, %4 km/h)")
				   .arg(from, to)
				   .arg(newDist, 0, 'f', 1)
				   .arg(newSpeed, 0, 'f', 0));
}

void MainWindow::onEditTrain()
{
	if (guardSimRunning("edit a train"))
		return;

	int row = _trainList->currentRow();
	if (row < 0 || row >= static_cast<int>(_trainDefs.size()))
	{
		logError("Select a train in the Trains tab to edit it.");
		return;
	}

	const auto &t = _trainDefs[row];
	QStringList names = _scene->nodeNames();

	QDialog dlg(this);
	dlg.setWindowTitle("Edit Train");
	dlg.setStyleSheet(styleSheet());
	auto *form = new QFormLayout(&dlg);

	auto *nameEdit = new QLineEdit(QString::fromStdString(t->getName()));
	auto *weightSpin = new QDoubleSpinBox;
	weightSpin->setRange(1, 99999);
	weightSpin->setValue(t->getWeight());
	weightSpin->setSuffix(" t");
	auto *fricSpin = new QDoubleSpinBox;
	fricSpin->setRange(0.001, 1.0);
	fricSpin->setDecimals(3);
	fricSpin->setValue(t->getFrictionCoefficient());
	auto *accelSpin = new QDoubleSpinBox;
	accelSpin->setRange(1, 99999);
	accelSpin->setValue(t->getMaxAccelForce());
	accelSpin->setSuffix(" kN");
	auto *brakeSpin = new QDoubleSpinBox;
	brakeSpin->setRange(1, 99999);
	brakeSpin->setValue(t->getMaxBrakeForce());
	brakeSpin->setSuffix(" kN");
	auto *fromBox = new QComboBox;
	fromBox->addItems(names);
	fromBox->setCurrentText(QString::fromStdString(t->getDepartureStation()));
	auto *toBox = new QComboBox;
	toBox->addItems(names);
	toBox->setCurrentText(QString::fromStdString(t->getArrivalStation()));
	auto *hourSpin = new QSpinBox;
	hourSpin->setRange(0, 23);
	hourSpin->setValue(static_cast<int>(t->getDepartureTime()) / 3600);
	auto *minSpin = new QSpinBox;
	minSpin->setRange(0, 59);
	minSpin->setValue((static_cast<int>(t->getDepartureTime()) % 3600) / 60);
	auto *stopSpin = new QSpinBox;
	stopSpin->setRange(0, 120);
	stopSpin->setValue(static_cast<int>(t->getStopDuration()) / 60);
	stopSpin->setSuffix(" min");

	auto *btns = new QHBoxLayout;
	auto *okBtn = new QPushButton("Save Changes");
	auto *cancelBtn = new QPushButton("Cancel");
	cancelBtn->setStyleSheet("background: #475569;");
	btns->addWidget(cancelBtn);
	btns->addWidget(okBtn);

	form->addRow("Name:", nameEdit);
	form->addRow("Weight:", weightSpin);
	form->addRow("Friction coeff:", fricSpin);
	form->addRow("Accel force:", accelSpin);
	form->addRow("Brake force:", brakeSpin);
	form->addRow("Departure:", fromBox);
	form->addRow("Arrival:", toBox);
	form->addRow("Dep. hour:", hourSpin);
	form->addRow("Dep. minute:", minSpin);
	form->addRow("Stop duration:", stopSpin);
	form->addRow(btns);

	connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
	connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

	if (dlg.exec() != QDialog::Accepted)
		return;

	/* Validate edited train */
	QString editTrainName = nameEdit->text().trimmed();
	if (editTrainName.isEmpty())
	{
		logError("Train name cannot be empty.");
		QMessageBox::warning(this, "Invalid Train Name",
			"Please provide a non-empty name for the train.");
		return;
	}
	if (editTrainName.contains(' ') || editTrainName.contains('\t'))
	{
		logError("Train name cannot contain spaces or tabs.");
		QMessageBox::warning(this, "Invalid Train Name",
			"Train names cannot contain spaces or tabs.");
		return;
	}
	if (fromBox->currentText() == toBox->currentText())
	{
		logError("Departure and arrival stations must be different.");
		QMessageBox::warning(this, "Invalid Train",
			"A train's departure and arrival must be different stations.");
		return;
	}
	/* Check route reachability */
	try
	{
		DijkstraPathfinding pathfinder;
		auto path = pathfinder.findPath(
			fromBox->currentText().toStdString(),
			toBox->currentText().toStdString(), _network);
		if (path.empty())
		{
			logError("No route between '" + fromBox->currentText()
					 + "' and '" + toBox->currentText() + "'.");
			QMessageBox::warning(this, "Unreachable Route",
				QString("There is no path between '%1' and '%2'.")
					.arg(fromBox->currentText(), toBox->currentText()));
			return;
		}
	}
	catch (const std::exception &e)
	{
		logError("Route check failed: " + QString::fromStdString(e.what()));
		QMessageBox::warning(this, "Unreachable Route",
			QString::fromStdString(e.what()));
		return;
	}

	double depTime = hourSpin->value() * 3600.0 + minSpin->value() * 60.0;
	double stopDur = stopSpin->value() * 60.0;

	auto newTrain = TrainFactory::createTrain(
		editTrainName.toStdString(),
		weightSpin->value(), fricSpin->value(),
		accelSpin->value(), brakeSpin->value(),
		fromBox->currentText().toStdString(),
		toBox->currentText().toStdString(),
		depTime, stopDur);
	_trainDefs[row] = std::move(newTrain);
	refreshLists();
	logSuccess("Updated train: " + editTrainName);
}

void MainWindow::onEditEvent()
{
	if (guardSimRunning("edit an event"))
		return;

	int row = _eventList->currentRow();
	if (row < 0 || row >= static_cast<int>(_eventDefs.size()))
	{
		logError("Select an event in the Events tab to edit it.");
		return;
	}

	const auto &ev = _eventDefs[row];
	QStringList names = _scene->nodeNames();

	QDialog dlg(this);
	dlg.setWindowTitle("Edit Event");
	dlg.setStyleSheet(styleSheet());
	auto *form = new QFormLayout(&dlg);

	auto *nameEdit = new QLineEdit(QString::fromStdString(ev.getName()));
	auto *probSpin = new QDoubleSpinBox;
	probSpin->setRange(0.01, 1.0);
	probSpin->setDecimals(2);
	probSpin->setValue(ev.getProbability());
	auto *durSpin = new QDoubleSpinBox;
	durSpin->setRange(1, 9999);
	durSpin->setValue(ev.getDuration() / 60.0);
	durSpin->setSuffix(" min");
	auto *node1Box = new QComboBox;
	node1Box->addItems(names);
	node1Box->setCurrentText(QString::fromStdString(ev.getNodeName()));
	auto *node2Box = new QComboBox;
	node2Box->addItem("(none - city event)");
	node2Box->addItems(names);
	if (ev.isRailEvent())
		node2Box->setCurrentText(QString::fromStdString(ev.getNodeName2()));

	auto *btns = new QHBoxLayout;
	auto *okBtn = new QPushButton("Save Changes");
	auto *cancelBtn = new QPushButton("Cancel");
	cancelBtn->setStyleSheet("background: #475569;");
	btns->addWidget(cancelBtn);
	btns->addWidget(okBtn);

	form->addRow("Event name:", nameEdit);
	form->addRow("Probability:", probSpin);
	form->addRow("Duration:", durSpin);
	form->addRow("Node 1:", node1Box);
	form->addRow("Node 2 (rail):", node2Box);
	form->addRow(btns);

	connect(okBtn, &QPushButton::clicked, &dlg, &QDialog::accept);
	connect(cancelBtn, &QPushButton::clicked, &dlg, &QDialog::reject);

	if (dlg.exec() != QDialog::Accepted)
		return;

	/* Validate event name */
	QString editEvName = nameEdit->text().trimmed();
	if (editEvName.isEmpty())
	{
		logError("Event name cannot be empty.");
		QMessageBox::warning(this, "Invalid Event Name",
			"Please provide a non-empty name for the event.");
		return;
	}

	std::string n1 = node1Box->currentText().toStdString();
	std::string n2;
	if (node2Box->currentIndex() > 0)
	{
		n2 = node2Box->currentText().toStdString();
		if (n2 == n1)
		{
			logError("Node 2 cannot be the same as Node 1 for a rail event.");
			QMessageBox::warning(this, "Invalid Rail Event",
				"For a rail segment event, Node 1 and Node 2 must be "
				"different stations.");
			return;
		}
	}

	_eventDefs[row] = Event(editEvName.toStdString(),
							probSpin->value(),
							durSpin->value() * 60.0, n1, n2);
	refreshLists();
	logSuccess("Updated event: " + editEvName);
}

/* ================================================================== */
/*  Simulation                                                        */
/* ================================================================== */

void MainWindow::onRunSimulation()
{
	if (_simRunning)
	{
		logError("A simulation is already running.");
		return;
	}

	if (_network.getNodeNames().empty())
	{
		logError("Cannot run simulation: no network loaded.");
		QMessageBox::warning(this, "No Network",
			"Load or create a network before running the simulation.\n\n"
			"Use the 'Quick Load Presets' in the sidebar or File > Import All.");
		return;
	}
	if (_trainDefs.empty())
	{
		logError("Cannot run simulation: no trains defined.");
		QMessageBox::warning(this, "No Trains",
			"Load or create at least one train before running.\n\n"
			"Use the 'Quick Load Presets' or the + Add button in the Trains tab.");
		return;
	}

	/* Pre-flight: validate every train route is reachable */
	logInfo("Pre-flight check: validating " + QString::number(_trainDefs.size())
			+ " train(s)...");
	DijkstraPathfinding preflight;
	for (size_t i = 0; i < _trainDefs.size(); ++i)
	{
		const auto &t = _trainDefs[i];
		QString tName = QString::fromStdString(t->getName());
		QString dep = QString::fromStdString(t->getDepartureStation());
		QString arr = QString::fromStdString(t->getArrivalStation());

		/* Check stations exist */
		bool depExists = false;
		bool arrExists = false;
		for (const auto &n : _network.getNodeNames())
		{
			if (n == dep.toStdString()) depExists = true;
			if (n == arr.toStdString()) arrExists = true;
		}
		if (!depExists)
		{
			logError(QString("Train '%1': departure station '%2' does not "
							 "exist in the network.").arg(tName, dep));
			QMessageBox::critical(this, "Invalid Train",
				QString("Train '%1' departs from '%2', but that node "
						"does not exist in the current network.\n\n"
						"Fix: edit the train or add the missing node.")
					.arg(tName, dep));
			return;
		}
		if (!arrExists)
		{
			logError(QString("Train '%1': arrival station '%2' does not "
							 "exist in the network.").arg(tName, arr));
			QMessageBox::critical(this, "Invalid Train",
				QString("Train '%1' arrives at '%2', but that node "
						"does not exist in the current network.\n\n"
						"Fix: edit the train or add the missing node.")
					.arg(tName, arr));
			return;
		}

		/* Check route reachability */
		try
		{
			auto path = preflight.findPath(
				dep.toStdString(), arr.toStdString(), _network);
			if (path.empty())
			{
				logError(QString("Train '%1': no route from '%2' to '%3'.")
							 .arg(tName, dep, arr));
				QMessageBox::critical(this, "Unreachable Route",
					QString("Train '%1' cannot travel from '%2' to '%3' "
							"— no connected path exists.\n\n"
							"Fix: add rail edges to connect these stations, "
							"or change the train's route.")
						.arg(tName, dep, arr));
				return;
			}
		}
		catch (const std::exception &e)
		{
			logError(QString("Train '%1': route check error — %2")
						 .arg(tName, QString::fromStdString(e.what())));
			QMessageBox::critical(this, "Route Check Failed",
				QString("Error validating route for train '%1' "
						"(%2 → %3):\n\n%4")
					.arg(tName, dep, arr,
						 QString::fromStdString(e.what())));
			return;
		}
	}
	logSuccess("Pre-flight OK: all " + QString::number(_trainDefs.size())
			   + " train route(s) are valid.");

	_runBtn->setEnabled(false);
	_runBtn->setText("  Running...  ");
	_stopBtn->setEnabled(true);
	_simRunning = true;
	_scene->clearTrains();

	/* Always write temp files from current in-memory state so that
	   any GUI modifications (add/edit/delete) are included. */
	QString netFile = writeTemporaryNetworkFile();
	QString trainFile = writeTemporaryTrainFile();

	logInfo(QString("  Network: %1 nodes, %2 edges")
				.arg(_nodeList->count()).arg(_edgeList->count()));
	logInfo(QString("  Trains: %1, Events: %2")
				.arg(_trainDefs.size()).arg(_eventDefs.size()));

	int numRuns = _runsSpinBox->value();
	bool animate = _animateCheck->isChecked();

	if (numRuns > 1)
	{
		/* Multi-run Monte Carlo mode */
		logInfo(QString("--- Starting multi-run (%1 runs, animate first: %2) ---")
					.arg(numRuns).arg(animate ? "yes" : "no"));
		_runBtn->setText(QString("  Run 0/%1...  ").arg(numRuns));
		QMetaObject::invokeMethod(
			_worker, "runMulti", Qt::QueuedConnection,
			Q_ARG(QString, netFile), Q_ARG(QString, trainFile),
			Q_ARG(int, _weightMode),
			Q_ARG(int, numRuns),
			Q_ARG(bool, animate));
	}
	else
	{
		/* Single run — always animated */
		logInfo("--- Starting simulation ---");
		QMetaObject::invokeMethod(
			_worker, "runSimulation", Qt::QueuedConnection,
			Q_ARG(QString, netFile), Q_ARG(QString, trainFile),
			Q_ARG(int, _weightMode));
	}
}

void MainWindow::onSimTick(double simTime,
						   QVector<TrainSnapshot> snapshots)
{
	_scene->updateTrains(simTime, snapshots);
	_dashboard->update(simTime, snapshots);

	int h = static_cast<int>(simTime) / 3600;
	int m = (static_cast<int>(simTime) % 3600) / 60;
	int s = static_cast<int>(simTime) % 60;
	statusBar()->showMessage(
		QString("Simulation time: %1:%2:%3  |  %4 trains active")
			.arg(h, 2, 10, QChar('0'))
			.arg(m, 2, 10, QChar('0'))
			.arg(s, 2, 10, QChar('0'))
			.arg(snapshots.size()));
}

void MainWindow::onSimFinished(QStringList results)
{
	_runBtn->setEnabled(true);
	_runBtn->setText("  Run Simulation  ");
	_stopBtn->setEnabled(false);
	_stopBtn->setText("  Stop  ");
	_simRunning = false;
	logSuccess("--- Simulation complete ---");
	for (const auto &r : results)
		logInfo("  " + r);
	_dashboard->showResults(results);
	statusBar()->showMessage("Simulation finished successfully");
}

void MainWindow::onSimError(const QString &msg)
{
	_runBtn->setEnabled(true);
	_runBtn->setText("  Run Simulation  ");
	_stopBtn->setEnabled(false);
	_stopBtn->setText("  Stop  ");
	_simRunning = false;

	/* Try to give a more specific diagnosis */
	QString detail = msg;
	QString hint;
	if (msg.contains("not found", Qt::CaseInsensitive)
		|| msg.contains("NodeNotFound", Qt::CaseInsensitive))
		hint = "A train references a station that doesn't exist in "
			   "the network.\nCheck the Trains tab for mismatched names.";
	else if (msg.contains("path", Qt::CaseInsensitive)
			 || msg.contains("route", Qt::CaseInsensitive)
			 || msg.contains("unreachable", Qt::CaseInsensitive))
		hint = "A train's departure or arrival station is not "
			   "reachable.\nMake sure the stations are connected by edges.";
	else if (msg.contains("file", Qt::CaseInsensitive)
			 || msg.contains("open", Qt::CaseInsensitive))
		hint = "Could not read a temporary simulation file.\n"
			   "Check disk permissions and available space.";
	else
		hint = "Try re-loading the network and trains, or check \n"
			   "the log panel for more details.";

	logError("Simulation failed: " + detail);
	logError("Hint: " + hint);
	QMessageBox::critical(this, "Simulation Error",
		QString("The simulation encountered an error:\n\n"
				"%1\n\n"
				"Diagnosis:\n%2")
			.arg(detail, hint));
}

void MainWindow::onFitGraph()
{
	if (_scene->items().isEmpty())
		return;
	_view->fitInView(_scene->sceneRect().adjusted(-60, -60, 60, 60),
					 Qt::KeepAspectRatio);
}

void MainWindow::onRunProgress(int currentRun, int totalRuns)
{
	_runBtn->setText(QString("  Run %1/%2...  ").arg(currentRun).arg(totalRuns));
	statusBar()->showMessage(
		QString("Multi-run: completed %1 of %2 runs")
			.arg(currentRun).arg(totalRuns));
}

void MainWindow::onMultiRunFinished(QVector<TrainStatRow> stats,
									int completedRuns)
{
	_runBtn->setEnabled(true);
	_runBtn->setText("  Run Simulation  ");
	_stopBtn->setEnabled(false);
	_stopBtn->setText("  Stop  ");
	_simRunning = false;

	if (stats.isEmpty())
	{
		logInfo("--- Multi-run stopped (no complete runs) ---");
		_dashboard->clear();
		statusBar()->showMessage("Multi-run stopped by user");
		return;
	}

	logSuccess(QString("--- Multi-run complete: %1 runs ---").arg(completedRuns));
	for (const auto &s : stats)
	{
		logInfo(QString("  %1: avg=%2s  min=%3s  max=%4s  avgDelay=%5s")
					.arg(s.name)
					.arg(static_cast<int>(s.avgActual))
					.arg(static_cast<int>(s.minActual))
					.arg(static_cast<int>(s.maxActual))
					.arg(static_cast<int>(s.avgDelay)));
	}
	_dashboard->showMultiRunStats(stats, completedRuns);

	/* Widen dashboard to fit the wider stats table */
	QList<int> sizes = _hSplit->sizes();
	if (sizes.size() == 2 && sizes[1] < DASH_STATS_W)
	{
		int total = sizes[0] + sizes[1];
		_hSplit->setSizes({total - DASH_STATS_W, DASH_STATS_W});
	}

	statusBar()->showMessage(
		QString("Multi-run finished: %1 runs completed").arg(completedRuns));
}

void MainWindow::onSpeedChanged(int value)
{
	/* Slider range 0–20, logarithmic:
	   0 → 0.1×, 10 → 1.0×, 20 → 10.0×
	   Formula: mult = 10^((value - 10) / 10)  = pow(10, (v-10)/10) */
	double mult = std::pow(10.0, (value - 10) / 10.0);

	/* Snap to clean values near standard points */
	if (std::abs(mult - 1.0) < 0.05) mult = 1.0;

	if (_worker)
		_worker->setSpeedMultiplier(mult);

	if (mult < 1.0)
		_speedLabel->setText(QString(" %1×").arg(mult, 0, 'f', 2));
	else
		_speedLabel->setText(QString(" %1×").arg(mult, 0, 'f', 1));
}

void MainWindow::onStopSimulation()
{
	if (!_simRunning || !_worker)
		return;
	logInfo("Requesting simulation stop...");
	_stopBtn->setEnabled(false);
	_stopBtn->setText("  Stopping...  ");
	_worker->requestStop();
}

/* ================================================================== */
/*  Temporary file helpers                                            */
/* ================================================================== */

QString MainWindow::writeTemporaryNetworkFile() const
{
	static QString tmpPath;
	tmpPath = QDir::tempPath() + "/trainsim_network.txt";
	QFile f(tmpPath);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
		return tmpPath;
	QTextStream out(&f);

	for (const auto &name : _network.getNodeNames())
		out << "Node " << quoteIfNeeded(name) << "\n";
	for (const auto &name : _network.getNodeNames())
	{
		for (const auto &e : _network.getNeighbours(name))
		{
			QString from = quoteIfNeeded(name);
			QString to = quoteIfNeeded(e.destination->getName());
			if (from < to)
				out << "Rail " << from << " " << to << " "
					<< e.distance << " " << e.speedLimit << "\n";
		}
	}
	for (const auto &ev : _eventDefs)
	{
		out << "Event " << quoteIfNeeded(ev.getName())
			<< " " << ev.getProbability() << " "
			<< ev.getDuration() / 60.0 << "m "
			<< quoteIfNeeded(ev.getNodeName());
		if (ev.isRailEvent())
			out << " " << quoteIfNeeded(ev.getNodeName2());
		out << "\n";
	}
	return tmpPath;
}

QString MainWindow::writeTemporaryTrainFile() const
{
	static QString tmpPath;
	tmpPath = QDir::tempPath() + "/trainsim_trains.txt";
	QFile f(tmpPath);
	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
		return tmpPath;
	QTextStream out(&f);

	for (const auto &t : _trainDefs)
	{
		int h = static_cast<int>(t->getDepartureTime()) / 3600;
		int m = (static_cast<int>(t->getDepartureTime()) % 3600) / 60;
		int sh = static_cast<int>(t->getStopDuration()) / 3600;
		int sm = (static_cast<int>(t->getStopDuration()) % 3600) / 60;
		out << quoteIfNeeded(t->getName()) << " "
			<< t->getWeight() << " "
			<< t->getFrictionCoefficient() << " "
			<< t->getMaxAccelForce() << " "
			<< t->getMaxBrakeForce() << " "
			<< quoteIfNeeded(t->getDepartureStation()) << " "
			<< quoteIfNeeded(t->getArrivalStation()) << " "
			<< QString("%1h%2").arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0'))
			<< " "
			<< QString("%1h%2").arg(sh, 2, 10, QChar('0')).arg(sm, 2, 10, QChar('0'))
			<< "\n";
	}
	return tmpPath;
}
