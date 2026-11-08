/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NetworkScene.cpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/06 18:06:54 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "NetworkScene.hpp"

#include <QApplication>
#include <QBrush>
#include <QCursor>
#include <QFile>
#include <QFont>
#include <QGraphicsSceneHoverEvent>
#include <QPen>
#include <QTimer>
#include <QtMath>
#include <cmath>
#include <cstdlib>

#include "Node.hpp"
#include "RailNetwork.hpp"

DraggableNode::DraggableNode(const QString &name, double radius,
							 NetworkScene *scene, QGraphicsItem *parent)
	: QGraphicsEllipseItem(-radius, -radius, 2 * radius, 2 * radius, parent),
	  _name(name), _netScene(scene)
{
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsSelectable);
	setAcceptHoverEvents(true);
	setCursor(Qt::OpenHandCursor);
}

QVariant DraggableNode::itemChange(GraphicsItemChange change,
								   const QVariant &value)
{
	if (change == ItemPositionHasChanged && _netScene)
		_netScene->nodeMoved(_name, value.toPointF());
	return QGraphicsEllipseItem::itemChange(change, value);
}

void DraggableNode::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
	setPen(QPen(QColor(26, 188, 156), 3));          /* teal glow */
	setBrush(QBrush(QColor(44, 62, 80).lighter(140)));
	setCursor(Qt::ClosedHandCursor);
}

void DraggableNode::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
	setPen(QPen(Qt::white, 2));
	setBrush(QBrush(QColor(52, 73, 94)));
	setCursor(Qt::OpenHandCursor);
}

const QColor NetworkScene::PALETTE[] = {
	QColor(231, 76, 60),   QColor(46, 204, 113),  QColor(52, 152, 219),
	QColor(241, 196, 15),  QColor(155, 89, 182),  QColor(230, 126, 34),
	QColor(26, 188, 156),  QColor(236, 64, 122),
};
const int NetworkScene::PALETTE_SIZE =
	static_cast<int>(sizeof(PALETTE) / sizeof(PALETTE[0]));

NetworkScene::NetworkScene(QObject *parent) : QGraphicsScene(parent)
{
	setBackgroundBrush(QColor(44, 62, 80));
}

NetworkScene::~NetworkScene() {}

/* Node label text, with any risky event names appended directly (not
   just an icon) so it's readable at a glance, without needing to
   hover. */
static QString nodeLabelText(const QString &name, const QString &riskNames)
{
	return riskNames.isEmpty()
		? name
		: name + QString::fromUtf8(" \xE2\x9A\xA0 ") + riskNames;
}

void NetworkScene::addNode(const QString &name, const QPointF &pos)
{
	if (_nodes.contains(name))
		return;

	const double R = 16.0;
	auto *dot = new DraggableNode(name, R, this);
	dot->setPen(QPen(Qt::white, 2));
	dot->setBrush(QBrush(QColor(52, 73, 94)));
	dot->setPos(pos);
	dot->setZValue(10);
	addItem(dot);

	auto *label = addText(nodeLabelText(name, QString()),
						  QFont("Segoe UI", 10, QFont::Bold));
	label->setDefaultTextColor(Qt::white);
	label->setPos(pos.x() + R + 4, pos.y() - 10);
	label->setZValue(11);

	_nodes[name] = {dot, label, pos, QString()};
}

/* Edge label text, with any risky event names appended directly (see
   nodeLabelText) so it stays correctly positioned (part of the same
   tracked label item) instead of a separate floating marker. */
static QString edgeLabelText(double distKm, double speedKmh,
							 const QString &riskNames, bool laneMode = false)
{
	/* Multi-line rails show speed per line on the lane cards instead. */
	QString txt = laneMode
		? QString("%1 km").arg(distKm, 0, 'f', 1)
		: QString("%1 km | %2 km/h").arg(distKm, 0, 'f', 1)
							.arg(speedKmh, 0, 'f', 0);
	if (!riskNames.isEmpty() && !laneMode)
		txt += QString::fromUtf8(" \xE2\x9A\xA0 ") + riskNames;
	return txt;
}

static const double LANE_GAP = 18.0;

/* Lanes mode (Multi-line rails ticked, or any rail with 2+ lines) shows every
   rail as lanes, single-line rails included (one L1 card). */
void NetworkScene::setLaneMode(bool on)
{
	if (_forceLanes == on)
		return;
	_forceLanes = on;
	refreshEdgeLabels();
}

bool NetworkScene::laneMode() const
{
	if (_forceLanes)
		return true;
	for (const auto &e : _edges)
		if (e.numLines > 1)
			return true;
	return false;
}

void NetworkScene::refreshEdgeLabels()
{
	bool lm = laneMode();
	for (auto &e : _edges)
	{
		e.label->setPlainText(
			edgeLabelText(e.distKm, e.speedKmh, e.riskNames, lm));
		repositionEdge(e);
	}
}

void NetworkScene::addEdge(const QString &from, const QString &to,
						   double distKm, double speedKmh, int numLines,
						   const QVector<double> &lineSpeeds)
{
	if (!_nodes.contains(from) || !_nodes.contains(to))
		return;
	for (const auto &e : _edges)
		if ((e.from == from && e.to == to) || (e.from == to && e.to == from))
			return;

	QPointF p1 = _nodes[from].pos;
	QPointF p2 = _nodes[to].pos;

	const QPen lanePen(QColor(149, 165, 166), 2.5, Qt::SolidLine,
					   Qt::RoundCap);
	auto *line = addPath(QPainterPath(), lanePen);
	line->setZValue(1);
	QVector<QGraphicsPathItem *> extra;
	for (int i = 1; i < numLines; i++)
	{
		auto *l = addPath(QPainterPath(), lanePen);
		l->setZValue(1);
		extra.push_back(l);
	}

	QPointF mid = (p1 + p2) / 2.0;
	auto *label = addText(edgeLabelText(distKm, speedKmh, QString(), numLines > 1),
						  QFont("Segoe UI", 7));
	label->setDefaultTextColor(QColor(189, 195, 199));
	label->setPos(mid.x() + 4, mid.y() - label->boundingRect().height());
	label->setZValue(2);

	const bool laneBefore = laneMode();
	_edges.push_back({line, label, from, to, distKm, speedKmh, QString(),
					  numLines, extra, lineSpeeds, {}});
	/* Re-laying out every edge per add is O(E^2) while loading a big
	   network; only the lane mode flipping needs a full refresh. */
	if (laneMode() != laneBefore)
		refreshEdgeLabels();
	else
	{
		auto &added = _edges.last();
		added.label->setPlainText(edgeLabelText(
			added.distKm, added.speedKmh, added.riskNames, laneBefore));
		repositionEdge(added);
	}
}

/* Each line of a multi-line rail has its own colour, used for the stroke
   and for its card, so a card is matched to its lane by colour. */
static QColor laneColor(int line)
{
	static const QColor palette[] = {
		QColor(52, 152, 219), QColor(230, 126, 34), QColor(46, 204, 113),
		QColor(155, 89, 182), QColor(241, 196, 15), QColor(231, 76, 60)};
	return palette[line % 6];
}

/* Lanes fan out from the node centres and run parallel in the middle, so
   every lane touches both nodes however many there are. */
QPointF NetworkScene::laneOffset(const EdgeVis &e, int line,
								 double frac) const
{
	if (e.numLines <= 1 || line < 0 || !_nodes.contains(e.from)
		|| !_nodes.contains(e.to))
		return QPointF(0, 0);
	QPointF d = _nodes[e.to].pos - _nodes[e.from].pos;
	double len = std::sqrt(d.x() * d.x() + d.y() * d.y());
	if (len < 1e-6)
		return QPointF(0, 0);
	QPointF normal(-d.y() / len, d.x() / len);
	double ramp = std::min(0.5, 90.0 / len);
	double t = std::min(frac, 1.0 - frac) / ramp;
	t = t >= 1.0 ? 1.0 : (t <= 0.0 ? 0.0 : t * t * (3.0 - 2.0 * t));
	return normal * ((line - (e.numLines - 1) / 2.0) * LANE_GAP * t);
}

void NetworkScene::setLanePath(QGraphicsPathItem *item, const EdgeVis &e,
							   int line)
{
	QPointF p1 = _nodes[e.from].pos;
	QPointF p2 = _nodes[e.to].pos;
	QPainterPath path;
	const int STEPS = 40;
	for (int k = 0; k <= STEPS; k++)
	{
		double f = double(k) / STEPS;
		QPointF pt = p1 + (p2 - p1) * f + laneOffset(e, line, f);
		if (k == 0)
			path.moveTo(pt);
		else
			path.lineTo(pt);
	}
	item->setPath(path);
}

void NetworkScene::clearLanes(EdgeVis &e)
{
	removeItem(e.line);
	delete e.line;
	for (auto *l : e.extraLanes)
	{
		removeItem(l);
		delete l;
	}
	e.extraLanes.clear();
	for (auto *t : e.laneLabels)
	{
		removeItem(t);
		delete t;
	}
	e.laneLabels.clear();
}

/* One label per lane (only on multi-line rails) with its number, its
   speed and the events that hit that lane only. Labels are staggered
   along the rail so neighbouring lanes never print on top of each other. */
void NetworkScene::updateLaneLabels(EdgeVis &e)
{
	for (auto *t : e.laneLabels)
	{
		removeItem(t);
		delete t;
	}
	e.laneLabels.clear();
	if (!laneMode() || !_nodes.contains(e.from)
		|| !_nodes.contains(e.to))
		return;

	QPointF p1 = _nodes[e.from].pos;
	QPointF p2 = _nodes[e.to].pos;
	QPointF d = p2 - p1;
	double len = std::hypot(d.x(), d.y());
	if (len <= 1e-6)
		return;

	/* One dispatcher card per lane, in the lane's own colour and sitting on
	   it: "number | speed limit | events". Cards are spread along the rail
	   so neighbouring cards never overlap. */
	double ramp = std::min(0.4, 90.0 / len);
	for (int i = 0; i < e.numLines; i++)
	{
		double kmh = (i < e.lineSpeeds.size() && e.lineSpeeds[i] > 0.0)
						 ? e.lineSpeeds[i] : e.speedKmh;
		QStringList evs;
		for (const auto &ev : _events)
			if ((ev.line == i || ev.line < 0) && !ev.node2.isEmpty()
				&& ((ev.node1 == e.from && ev.node2 == e.to)
					|| (ev.node1 == e.to && ev.node2 == e.from)))
				evs << ev.name;
		QString card = QString("<span style=\"background-color:%1;"
							   "color:#ffffff;\">&nbsp;<b>L%2</b> &middot; "
							   "%3 km/h")
						   .arg(laneColor(i).name()).arg(i + 1)
						   .arg(kmh, 0, 'f', 0);
		if (!evs.isEmpty())
			card += QString::fromUtf8(" &middot; \xE2\x9A\xA0 ")
					+ evs.join(", ").toHtmlEscaped();
		card += "&nbsp;</span>";
		/* Off-centre for a single lane: crossing rails meet at the middle. */
		double t = e.numLines > 1 ? double(i) / (e.numLines - 1) : 0.3;
		double f = ramp + t * (1.0 - 2.0 * ramp);
		auto *lt = addText("", QFont("Segoe UI", 7));
		lt->setHtml(card);
		QRectF bb = lt->boundingRect();
		QPointF bp = edgePosition(e.from, e.to, f, i);
		lt->setPos(bp.x() - bb.width() / 2, bp.y() - bb.height() / 2);
		/* Run the card along its lane so it stays inside the lane's own
		   channel instead of covering the neighbours; keep text upright. */
		QPointF dir = edgePosition(e.from, e.to, std::min(1.0, f + 0.02), i)
					  - edgePosition(e.from, e.to, std::max(0.0, f - 0.02), i);
		double ang = std::atan2(dir.y(), dir.x()) * 180.0 / M_PI;
		if (ang > 90.0)
			ang -= 180.0;
		else if (ang <= -90.0)
			ang += 180.0;
		lt->setTransformOriginPoint(bb.center());
		lt->setRotation(ang);
		lt->setZValue(6); // above train trails (5), below trains (20)
		e.laneLabels.push_back(lt);
	}
}

void NetworkScene::removeNode(const QString &name)
{
	if (!_nodes.contains(name))
		return;

	for (int i = _edges.size() - 1; i >= 0; --i)
	{
		if (_edges[i].from == name || _edges[i].to == name)
		{
			clearLanes(_edges[i]);
			removeItem(_edges[i].label);
			delete _edges[i].label;
			_edges.removeAt(i);
		}
	}

	auto &nv = _nodes[name];
	removeItem(nv.dot);
	removeItem(nv.label);
	delete nv.dot;
	delete nv.label;
	_nodes.remove(name);
}

void NetworkScene::removeEdge(const QString &from, const QString &to)
{
	for (int i = 0; i < _edges.size(); ++i)
	{
		if (((_edges[i].from == from && _edges[i].to == to)
			 || (_edges[i].from == to && _edges[i].to == from)))
		{
			clearLanes(_edges[i]);
			removeItem(_edges[i].label);
			delete _edges[i].label;
			_edges.removeAt(i);
			return;
		}
	}
}

void NetworkScene::renameNode(const QString &oldName, const QString &newName)
{
	if (!_nodes.contains(oldName) || _nodes.contains(newName))
		return;

	NodeVis nv = _nodes.take(oldName);
	nv.dot->setName(newName);
	nv.label->setPlainText(nodeLabelText(newName, nv.riskNames));
	_nodes[newName] = nv;

	for (auto &e : _edges)
	{
		if (e.from == oldName)
			e.from = newName;
		if (e.to == oldName)
			e.to = newName;
	}
}

void NetworkScene::updateEdge(const QString &from, const QString &to,
							  double newDist, double newSpeed, int newLines,
							  const QVector<double> &newLineSpeeds)
{
	for (auto &e : _edges)
	{
		if ((e.from == from && e.to == to)
			|| (e.from == to && e.to == from))
		{
			if (newLines > 0 && newLines != e.numLines)
			{
				const QString f = e.from, t = e.to, risk = e.riskNames;
				removeEdge(f, t);
				addEdge(f, t, newDist, newSpeed, newLines, newLineSpeeds);
				for (auto &n : _edges)
					if (n.from == f && n.to == t)
					{
						n.riskNames = risk;
						n.label->setPlainText(edgeLabelText(
							newDist, newSpeed, risk, laneMode()));
						repositionEdge(n);
					}
				return;
			}
			e.distKm = newDist;
			e.speedKmh = newSpeed;
			e.lineSpeeds = newLineSpeeds;
			e.label->setPlainText(
				edgeLabelText(newDist, newSpeed, e.riskNames, laneMode()));
			updateLaneLabels(e);
			return;
		}
	}
}

void NetworkScene::clear()
{
	QGraphicsScene::clear();
	_nodes.clear();
	_edges.clear();
	_trains.clear();
	_trailLines.clear();
	_events.clear();
}

void NetworkScene::nodeMoved(const QString &name, const QPointF &newPos)
{
	if (!_nodes.contains(name))
		return;

	_nodes[name].pos = newPos;
	_nodes[name].label->setPos(newPos.x() + 20, newPos.y() - 10);

	for (auto &e : _edges)
	{
		if (e.from == name || e.to == name)
			repositionEdge(e);
	}

	/* Moving the layout invalidates drawn trails and train positions, so
	   drop them; trains reappear on the next simulation update. */
	clearTrains();
}

void NetworkScene::repositionEdge(EdgeVis &e)
{
	if (!_nodes.contains(e.from) || !_nodes.contains(e.to))
		return;
	QPointF p1 = _nodes[e.from].pos;
	QPointF p2 = _nodes[e.to].pos;
	for (int i = 0; i < e.numLines; i++)
	{
		QGraphicsPathItem *lane = i == 0 ? e.line : e.extraLanes[i - 1];
		setLanePath(lane, e, i);
	}
	updateLaneLabels(e);
	QPointF mid = (p1 + p2) / 2.0;
	/* Keep the label clear of the outermost lane: push it out along the
	   normal on the upper side, centred when the edge is flat-ish. */
	QPointF d = p2 - p1;
	double len = std::hypot(d.x(), d.y());
	QRectF lb = e.label->boundingRect();
	if (!laneMode() || len < 1e-6)
	{
		e.label->setPos(mid.x() + 4, mid.y() - lb.height());
		return;
	}
	QPointF n(-d.y() / len, d.x() / len);
	if (n.y() > 0 || (n.y() == 0 && n.x() < 0))
		n = -n;
	/* Distance so the label rectangle clears the outer lane (support
	   function of the rectangle along the normal). */
	double out = (e.numLines - 1) / 2.0 * LANE_GAP + 16.0
				 + std::fabs(n.x()) * lb.width() / 2.0
				 + std::fabs(n.y()) * lb.height() / 2.0;
	QPointF c = mid + n * out;
	e.label->setPos(c.x() - lb.width() / 2, c.y() - lb.height() / 2);
}

void NetworkScene::loadFromNetwork(const RailNetwork &network)
{
	clear();

	auto names = network.getNodeNames();
	for (const auto &name : names)
		addNode(QString::fromStdString(name), QPointF(0, 0));

	for (const auto &name : names)
	{
		const auto &edges = network.getNeighbours(name);
		for (const auto &e : edges)
		{
			QString from = QString::fromStdString(name);
			QString to = QString::fromStdString(e.destination->getName());
			addEdge(from, to, e.distance, e.speedLimit, e.numLines,
				QVector<double>(e.lineSpeeds.begin(), e.lineSpeeds.end()));
		}
	}

	if (!layoutFromFile())
		layoutForceDirected();
}

/* "Station x y" per line ('#' comments). Applied only if it places every
   station, otherwise the automatic layout is used. */
bool NetworkScene::layoutFromFile()
{
	if (_layoutFile.isEmpty())
		return false;
	QFile f(_layoutFile);
	if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;
	QMap<QString, QPointF> pts;
	while (!f.atEnd())
	{
		QString line = QString::fromUtf8(f.readLine()).trimmed();
		if (line.isEmpty() || line.startsWith('#'))
			continue;
		QStringList p = line.split(' ', Qt::SkipEmptyParts);
		bool okx = false, oky = false;
		if (p.size() != 3)
			return false;
		double x = p[1].toDouble(&okx), y = p[2].toDouble(&oky);
		if (!okx || !oky)
			return false;
		pts[p[0]] = QPointF(x, y);
	}
	for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
		if (!pts.contains(it.key()))
			return false;
	for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
	{
		it.value().pos = pts[it.key()];
		it.value().dot->setPos(it.value().pos);
	}
	return true;
}

void NetworkScene::layoutForceDirected()
{
	if (_nodes.size() < 2)
		return;

	double spread = 120.0 * std::sqrt(_nodes.size());
	std::srand(42);
	for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
	{
		double x = (std::rand() % 1000 - 500) * spread / 500.0;
		double y = (std::rand() % 1000 - 500) * spread / 500.0;
		it.value().pos = QPointF(x, y);
	}

	const int ITERS = 300;
	const double REPEL = 80000.0;
	const double ATTRACT = 0.005;
	const double DAMPING = 0.85;

	/* Index arrays instead of string-keyed maps: same maths and the same
	   (sorted-key) order, so positions are unchanged, but far faster. */
	const QStringList keys = _nodes.keys();
	const int N = keys.size();
	QHash<QString, int> idx;
	idx.reserve(N);
	QVector<QPointF> pos(N), vel(N, QPointF(0, 0));
	for (int i = 0; i < N; i++)
	{
		idx.insert(keys[i], i);
		pos[i] = _nodes[keys[i]].pos;
	}
	QVector<QPair<int, int>> links;
	for (const auto &e : _edges)
		if (idx.contains(e.from) && idx.contains(e.to))
			links.push_back({idx[e.from], idx[e.to]});

	for (int iter = 0; iter < ITERS; iter++)
	{
		QVector<QPointF> force(N, QPointF(0, 0));

		for (int i = 0; i < N; i++)
		{
			for (int j = i + 1; j < N; j++)
			{
				QPointF d = pos[i] - pos[j];
				double dist = std::sqrt(d.x() * d.x() + d.y() * d.y());
				if (dist < 1.0) dist = 1.0;
				double f = REPEL / (dist * dist);
				QPointF fn = QPointF(d.x() / dist * f, d.y() / dist * f);
				force[i] += fn;
				force[j] -= fn;
			}
		}

		for (const auto &l : links)
		{
			QPointF d = pos[l.second] - pos[l.first];
			double dist = std::sqrt(d.x() * d.x() + d.y() * d.y());
			if (dist < 1.0) dist = 1.0;
			double f = ATTRACT * dist;
			QPointF fn = QPointF(d.x() / dist * f, d.y() / dist * f);
			force[l.first] += fn;
			force[l.second] -= fn;
		}

		for (int i = 0; i < N; i++)
		{
			vel[i] = (vel[i] + force[i]) * DAMPING;
			pos[i] += vel[i];
		}
	}

	for (int i = 0; i < N; i++)
		_nodes[keys[i]].pos = pos[i];

	/* Apply final positions — go through setPos which triggers itemChange
	   → nodeMoved, so edges & labels auto-reposition. */
	for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
		it.value().dot->setPos(it.value().pos);
}

QPointF NetworkScene::edgePosition(const QString &from, const QString &to,
								   double fraction, int line) const
{
	if (!_nodes.contains(from) || !_nodes.contains(to))
		return QPointF(0, 0);
	QPointF p1 = _nodes[from].pos;
	QPointF p2 = _nodes[to].pos;
	QPointF pos = p1 + (p2 - p1) * fraction;
	if (line >= 0)
		for (const auto &e : _edges)
			if ((e.from == from && e.to == to)
				|| (e.from == to && e.to == from))
				return pos + laneOffset(e, line, fraction);
	return pos;
}

void NetworkScene::updateTrains(double /*simTime*/,
								const QVector<TrainSnapshot> &snapshots)
{
	/* With hundreds of trains, per-train text labels, trails and tooltips
	   are what freezes the GUI; draw plain dots for trains in flight only. */
	const bool large = snapshots.size() > 150;
	static constexpr int MAX_TRAIL_SEGS = 4000;

	QHash<QString, const EdgeVis *> edgeAt;
	edgeAt.reserve(_edges.size() * 2);
	for (const auto &e : _edges)
	{
		edgeAt.insert(e.from + QLatin1Char('|') + e.to, &e);
		edgeAt.insert(e.to + QLatin1Char('|') + e.from, &e);
	}

	for (int i = 0; i < snapshots.size(); i++)
	{
		const auto &s = snapshots[i];
		QString key = s.name + QString::number(s.id);
		QColor col = PALETTE[i % PALETTE_SIZE];

		if (large && (!s.departed || s.arrived))
		{
			auto hidden = _trains.find(key);
			if (hidden != _trains.end())
			{
				hidden->dot->setVisible(false);
				if (hidden->label)
					hidden->label->setVisible(false);
				hidden->hasPrev = false;
			}
			continue;
		}

		QPointF pos(0, 0);
		QString curFrom, curTo;
		double curFrac = 0.0;
		if (s.departed && !s.arrived && !s.from.isEmpty()
			&& !s.to.isEmpty())
		{
			if (_nodes.contains(s.from) && _nodes.contains(s.to))
			{
				QPointF p1 = _nodes[s.from].pos;
				QPointF p2 = _nodes[s.to].pos;
				double frac = 0.5;
				const EdgeVis *ev = edgeAt.value(s.from + QLatin1Char('|')
												 + s.to, nullptr);
				if (ev)
				{
					double edgeM = ev->distKm * 1000.0;
					if (edgeM > 0)
						frac = s.posOnSegment_m / edgeM;
				}
				if (frac > 1.0) frac = 1.0;
				if (frac < 0.0) frac = 0.0;
				pos = p1 + (p2 - p1) * frac;
				if (s.line >= 0 && ev)
					pos += laneOffset(*ev, s.line, frac);
				curFrom = s.from;
				curTo = s.to;
				curFrac = frac;
			}
		}
		else if (s.arrived && _nodes.contains(s.arrival))
		{
			pos = _nodes[s.arrival].pos;
			curFrom = curTo = s.arrival;
		}
		else if (!s.departed && _nodes.contains(s.departure))
		{
			pos = _nodes[s.departure].pos;
			curFrom = curTo = s.departure;
		}

		if (!_trains.contains(key))
		{
			const double R = large ? 5.0 : 8.0;
			auto *dot = addEllipse(-R, -R, 2 * R, 2 * R,
								   QPen(Qt::white, 1.5), QBrush(col));
			dot->setZValue(20);
			QGraphicsTextItem *label = nullptr;
			if (!large)
			{
				label = addText(s.name, QFont("Segoe UI", 8, QFont::Bold));
				label->setDefaultTextColor(col);
				label->setZValue(21);
			}
			_trains[key] = {dot, label, pos, false, curFrom, curTo, curFrac,
							s.line};
		}
		auto &vis = _trains[key];
		vis.dot->setVisible(true);
		if (!large)
		{
			if (!vis.label)
			{
				vis.label = addText(s.name,
									QFont("Segoe UI", 8, QFont::Bold));
				vis.label->setDefaultTextColor(col);
				vis.label->setZValue(21);
			}
			vis.label->setVisible(true);
		}
		if (large)
		{
			vis.dot->setPos(pos);
			vis.prevPos = pos;
			vis.hasPrev = true;
			bool isDelayed = (s.status == QLatin1String("Delayed"));
			vis.dot->setPen(QPen(isDelayed ? QColor(231, 76, 60) : Qt::white,
								 isDelayed ? 2.0 : 1.0));
			continue;
		}

		if (vis.hasPrev && s.departed && !s.arrived
			&& _trailLines.size() < MAX_TRAIL_SEGS)
		{
			double dx = pos.x() - vis.prevPos.x();
			double dy = pos.y() - vis.prevPos.y();
			if (dx * dx + dy * dy > 1.0) /* skip if barely moved */
			{
				QColor trailCol = col;
				trailCol.setAlpha(120);
				auto *seg = addLine(
					vis.prevPos.x(), vis.prevPos.y(),
					pos.x(), pos.y(),
					QPen(trailCol, 3.5, Qt::SolidLine, Qt::RoundCap));
				seg->setZValue(5);
				_trailLines.push_back({seg, vis.prevFrom, vis.prevTo,
									   vis.prevFrac, curFrom, curTo,
									   curFrac, vis.prevLine, s.line});
			}
		}
		vis.prevPos = pos;
		vis.hasPrev = s.departed;
		vis.prevFrom = curFrom;
		vis.prevTo = curTo;
		vis.prevFrac = curFrac;
		vis.prevLine = s.line;

		vis.dot->setPos(pos);
		/* Arrived trains sit on top of the node, so drop the label below
		   it (instead of the usual upper-right slot) to avoid colliding
		   with the node's own name label, and fade it like the dot. */
		vis.label->setPos(pos.x() + 12,
						   s.arrived ? pos.y() + 14 : pos.y() - 16);
		vis.dot->setOpacity(s.arrived ? 0.35 : 1.0);
		vis.label->setOpacity(s.arrived ? 0.35 : 1.0);

		/* Blocked/Delayed trains get a red ring so the state is visible
		   at a glance, not just in the dashboard text. */
		bool isDelayed = (s.status == QLatin1String("Delayed"));
		vis.dot->setPen(QPen(isDelayed ? QColor(231, 76, 60) : Qt::white,
							 isDelayed ? 2.5 : 1.5));

		QString tip = s.name;
		tip += QString("\nStatus: %1").arg(s.status);
		QString lineTxt;
		if (s.line >= 0)
			lineTxt = QString(" L%1").arg(s.line + 1);
		else if (s.waiting)
			lineTxt = QString(" (waiting)");
		vis.label->setPlainText(s.name + lineTxt);
		if (!lineTxt.isEmpty())
			tip += QString("\nLine:%1").arg(lineTxt);
		if (!s.blockedByName.isEmpty())
			tip += QString("\nBlocked by: %1").arg(s.blockedByName);
		if (s.rerouteCount > 0)
		{
			tip += QString("\nRerouted %1x").arg(s.rerouteCount);
			if (!s.lastRerouteReason.isEmpty())
				tip += QString(" (%1)").arg(s.lastRerouteReason);
		}
		tip += QString("\nWeight: %1 t | Friction: %2")
				   .arg(s.weightTons, 0, 'f', 0)
				   .arg(s.frictionCoeff, 0, 'f', 2);
		tip += QString("\nAccel: %1 kN (%2 m/s\xC2\xB2) | Brake: %3 kN (%4 m/s\xC2\xB2)")
				   .arg(s.maxAccelKn, 0, 'f', 0)
				   .arg(s.accelRate_ms2, 0, 'f', 2)
				   .arg(s.maxBrakeKn, 0, 'f', 0)
				   .arg(s.decelRate_ms2, 0, 'f', 2);
		vis.dot->setToolTip(tip);
	}
}

void NetworkScene::clearTrains()
{
	for (auto &seg : _trailLines)
	{
		if (seg.line->scene() == this)
			removeItem(seg.line);
		delete seg.line;
	}
	_trailLines.clear();

	for (auto it = _trains.begin(); it != _trains.end(); ++it)
	{
		if (it.value().dot->scene() == this)
			removeItem(it.value().dot);
		if (it.value().label && it.value().label->scene() == this)
			removeItem(it.value().label);
		delete it.value().dot;
		delete it.value().label;
	}
	_trains.clear();
}

void NetworkScene::setEventInfo(const QVector<EventInfo> &events)
{
	_events = events;
	drawEventMarkers();
}

void NetworkScene::drawEventMarkers()
{
	/* Reset any previous risk marking before reapplying, in case this
	   is called again with a different event list. */
	for (auto &n : _nodes)
	{
		if (!n.riskNames.isEmpty())
		{
			n.riskNames.clear();
			n.label->setPlainText(nodeLabelText(n.dot->name(), QString()));
		}
	}
	for (auto &e : _edges)
	{
		if (!e.riskNames.isEmpty())
		{
			e.riskNames.clear();
			e.label->setPlainText(
				edgeLabelText(e.distKm, e.speedKmh, QString(), laneMode()));
		}
	}

	/* Multiple events can share one location - accumulate names for
	   all of them before touching the label, so nothing gets silently
	   overwritten by a later event at the same spot. */
	QMap<QString, QStringList> nodeNames;
	QMap<QString, QStringList> edgeNames;

	for (const auto &ev : _events)
	{
		if (!ev.node2.isEmpty())
		{
			for (auto &e : _edges)
			{
				if ((e.from == ev.node1 && e.to == ev.node2)
					|| (e.from == ev.node2 && e.to == ev.node1))
					edgeNames[e.from + "->" + e.to]
						<< (ev.line >= 0
								? QString("%1 (L%2)").arg(ev.name).arg(ev.line + 1)
								: ev.name);
			}
		}
		else if (_nodes.contains(ev.node1))
		{
			nodeNames[ev.node1] << ev.name;
		}
	}

	for (auto &e : _edges)
	{
		QString key = e.from + "->" + e.to;
		updateLaneLabels(e);
		if (!edgeNames.contains(key))
			continue;
		e.riskNames = edgeNames[key].join(", ");
		e.label->setPlainText(
			edgeLabelText(e.distKm, e.speedKmh, e.riskNames, laneMode()));
	}
	for (auto it = nodeNames.begin(); it != nodeNames.end(); ++it)
	{
		if (!_nodes.contains(it.key()))
			continue;
		auto &n = _nodes[it.key()];
		n.riskNames = it.value().join(", ");
		n.label->setPlainText(nodeLabelText(n.dot->name(), n.riskNames));
	}
}

void NetworkScene::flashEvent(const QString &eventName,
							  const QString &trainName)
{
	Q_UNUSED(trainName);
	const EventInfo *found = nullptr;
	for (const auto &ev : _events)
		if (ev.name == eventName) { found = &ev; break; }
	if (!found)
		return;

	if (!found->node2.isEmpty())
	{
		for (auto &e : _edges)
		{
			if ((e.from == found->node1 && e.to == found->node2)
				|| (e.from == found->node2 && e.to == found->node1))
			{
				QVector<QGraphicsPathItem *> lanes = e.extraLanes;
				lanes.push_front(e.line);
				for (int li = 0; li < lanes.size(); li++)
				{
					if (found->line >= 0 && li != found->line)
						continue;
					auto *line = lanes[li];
					QPen original = line->pen();
					line->setPen(QPen(QColor(231, 76, 60), 4.5,
									  Qt::SolidLine, Qt::RoundCap));
					QTimer::singleShot(900, this, [this, line, original]() {
						if (line->scene() == this)
							line->setPen(original);
					});
				}
			}
		}
	}
	else if (_nodes.contains(found->node1))
	{
		auto *dot = _nodes[found->node1].dot;
		QBrush original = dot->brush();
		dot->setBrush(QBrush(QColor(231, 76, 60)));
		QTimer::singleShot(900, this, [this, dot, original]() {
			if (dot->scene() == this)
				dot->setBrush(original);
		});
	}
}

void NetworkScene::markReroute(const QString &nodeName,
							   const QString &trainName)
{
	Q_UNUSED(trainName);
	if (!_nodes.contains(nodeName))
		return;
	auto *dot = _nodes[nodeName].dot;
	QPen original = dot->pen();
	dot->setPen(QPen(QColor(52, 152, 219), 4));
	QTimer::singleShot(900, this, [this, dot, original]() {
		if (dot->scene() == this)
			dot->setPen(original);
	});
}


QStringList NetworkScene::nodeNames() const
{
	QStringList names;
	for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
		names << it.key();
	names.sort();
	return names;
}

bool NetworkScene::hasNode(const QString &name) const
{
	return _nodes.contains(name);
}
