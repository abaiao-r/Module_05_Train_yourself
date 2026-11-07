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
							 const QString &riskNames, int numLines = 1)
{
	QString txt = QString("%1 km | %2 km/h")
					  .arg(distKm, 0, 'f', 1)
					  .arg(speedKmh, 0, 'f', 0);
	if (numLines > 1)
		txt += QString(" | %1 lines").arg(numLines);
	if (!riskNames.isEmpty())
		txt += QString::fromUtf8(" \xE2\x9A\xA0 ") + riskNames;
	return txt;
}

static const double LANE_GAP = 14.0;

void NetworkScene::addEdge(const QString &from, const QString &to,
						   double distKm, double speedKmh, int numLines)
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
	auto *label = addText(edgeLabelText(distKm, speedKmh, QString(), numLines),
						  QFont("Segoe UI", 7));
	label->setDefaultTextColor(QColor(189, 195, 199));
	label->setPos(mid.x() + 4, mid.y() - label->boundingRect().height());
	label->setZValue(2);

	_edges.push_back({line, label, from, to, distKm, speedKmh, QString(),
					  numLines, extra});
	repositionEdge(_edges.last());
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
							  double newDist, double newSpeed, int newLines)
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
				addEdge(f, t, newDist, newSpeed, newLines);
				for (auto &n : _edges)
					if (n.from == f && n.to == t)
					{
						n.riskNames = risk;
						n.label->setPlainText(edgeLabelText(
							newDist, newSpeed, risk, newLines));
						repositionEdge(n);
					}
				return;
			}
			e.distKm = newDist;
			e.speedKmh = newSpeed;
			e.label->setPlainText(
				edgeLabelText(newDist, newSpeed, e.riskNames, e.numLines));
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
		setLanePath(i == 0 ? e.line : e.extraLanes[i - 1], e, i);
	}
	QPointF mid = (p1 + p2) / 2.0;
	/* Keep the label clear of the outermost lane: push it out along the
	   normal on the upper side, centred when the edge is flat-ish. */
	QPointF d = p2 - p1;
	double len = std::hypot(d.x(), d.y());
	QRectF lb = e.label->boundingRect();
	if (e.numLines <= 1 || len < 1e-6)
	{
		e.label->setPos(mid.x() + 4, mid.y() - lb.height());
		return;
	}
	QPointF n(-d.y() / len, d.x() / len);
	if (n.y() > 0 || (n.y() == 0 && n.x() < 0))
		n = -n;
	/* Distance so the label rectangle clears the outer lane (support
	   function of the rectangle along the normal). */
	double out = (e.numLines - 1) / 2.0 * LANE_GAP + 6.0
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
			addEdge(from, to, e.distance, e.speedLimit, e.numLines);
		}
	}

	layoutForceDirected();
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

	QMap<QString, QPointF> vel;
	for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
		vel[it.key()] = QPointF(0, 0);

	for (int iter = 0; iter < ITERS; iter++)
	{
		QMap<QString, QPointF> force;
		for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
			force[it.key()] = QPointF(0, 0);

		auto keys = _nodes.keys();
		for (int i = 0; i < keys.size(); i++)
		{
			for (int j = i + 1; j < keys.size(); j++)
			{
				QPointF d = _nodes[keys[i]].pos - _nodes[keys[j]].pos;
				double dist = std::sqrt(d.x() * d.x() + d.y() * d.y());
				if (dist < 1.0) dist = 1.0;
				double f = REPEL / (dist * dist);
				QPointF fn = QPointF(d.x() / dist * f, d.y() / dist * f);
				force[keys[i]] += fn;
				force[keys[j]] -= fn;
			}
		}

		for (const auto &e : _edges)
		{
			if (!_nodes.contains(e.from) || !_nodes.contains(e.to))
				continue;
			QPointF d = _nodes[e.to].pos - _nodes[e.from].pos;
			double dist = std::sqrt(d.x() * d.x() + d.y() * d.y());
			if (dist < 1.0) dist = 1.0;
			double f = ATTRACT * dist;
			QPointF fn = QPointF(d.x() / dist * f, d.y() / dist * f);
			force[e.from] += fn;
			force[e.to] -= fn;
		}

		for (auto it = _nodes.begin(); it != _nodes.end(); ++it)
		{
			vel[it.key()] = (vel[it.key()] + force[it.key()]) * DAMPING;
			it.value().pos += vel[it.key()];
		}
	}

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
	for (int i = 0; i < snapshots.size(); i++)
	{
		const auto &s = snapshots[i];
		QString key = s.name + QString::number(s.id);
		QColor col = PALETTE[i % PALETTE_SIZE];

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
				for (const auto &e : _edges)
				{
					if ((e.from == s.from && e.to == s.to)
						|| (e.from == s.to && e.to == s.from))
					{
						double edgeM = e.distKm * 1000.0;
						if (edgeM > 0)
							frac = s.posOnSegment_m / edgeM;
						break;
					}
				}
				if (frac > 1.0) frac = 1.0;
				if (frac < 0.0) frac = 0.0;
				pos = p1 + (p2 - p1) * frac;
				if (s.line >= 0)
					for (const auto &e : _edges)
						if ((e.from == s.from && e.to == s.to)
							|| (e.from == s.to && e.to == s.from))
						{
							pos += laneOffset(e, s.line, frac);
							break;
						}
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
			const double R = 8.0;
			auto *dot = addEllipse(-R, -R, 2 * R, 2 * R,
								   QPen(Qt::white, 1.5), QBrush(col));
			dot->setZValue(20);
			auto *label = addText(s.name, QFont("Segoe UI", 8, QFont::Bold));
			label->setDefaultTextColor(col);
			label->setZValue(21);
			_trains[key] = {dot, label, pos, false, curFrom, curTo, curFrac,
							s.line};
		}
		auto &vis = _trains[key];

		if (vis.hasPrev && s.departed && !s.arrived)
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
		if (it.value().label->scene() == this)
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
				edgeLabelText(e.distKm, e.speedKmh, QString(), e.numLines));
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
		if (!edgeNames.contains(key))
			continue;
		e.riskNames = edgeNames[key].join(", ");
		e.label->setPlainText(
			edgeLabelText(e.distKm, e.speedKmh, e.riskNames, e.numLines));
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
