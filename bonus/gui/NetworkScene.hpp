/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NetworkScene.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by abaiao-r          #+#    #+#             */
/*   Updated: 2026/09/06 12:55:36 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NETWORKSCENE_HPP
#define NETWORKSCENE_HPP

#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QColor>
#include <QMap>
#include <QSet>
#include <QPointF>
#include <QString>
#include <QVector>
#include <memory>
#include <string>

#include "SimulationWorker.hpp"

class RailNetwork;
class Train;
class Node;
class NetworkScene;

/* Notifies the scene on every move so edges and labels track the node. */
class DraggableNode : public QGraphicsEllipseItem
{
  public:
	DraggableNode(const QString &name, double radius,
				  NetworkScene *scene, QGraphicsItem *parent = nullptr);
	QString name() const { return _name; }
	void setName(const QString &name) { _name = name; }

  protected:
	QVariant itemChange(GraphicsItemChange change,
						const QVariant &value) override;
	void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
	void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;

  private:
	QString _name;
	NetworkScene *_netScene;
};

class NetworkScene : public QGraphicsScene
{
	Q_OBJECT

  public:
	explicit NetworkScene(QObject *parent = nullptr);
	~NetworkScene();

	void addNode(const QString &name, const QPointF &pos);
	void addEdge(const QString &from, const QString &to,
				 double distKm, double speedKmh, int numLines = 1,
				 const QVector<double> &lineSpeeds = {});
	void removeNode(const QString &name);
	void removeEdge(const QString &from, const QString &to);
	void renameNode(const QString &oldName, const QString &newName);
	void updateEdge(const QString &from, const QString &to,
					double newDist, double newSpeed, int newLines = -1,
					const QVector<double> &newLineSpeeds = {});
	void clear();

	/** Populate from an existing RailNetwork object. */
	void loadFromNetwork(const RailNetwork &network);

	/** Optional "Station x y" file used instead of the automatic layout
	    when it places every station. Empty path = automatic layout. */
	void setLayoutFile(const QString &path) { _layoutFile = path; }

	/* Called by DraggableNode::itemChange when a node is dragged */
	void nodeMoved(const QString &name, const QPointF &newPos);

	void updateTrains(double simTime,
					  const QVector<TrainSnapshot> &snapshots);
	void clearTrains();

	/** Mark every segment/node that has an event bound to it, by
		appending a small risk indicator to its existing label and a
		tooltip with the details - keeps it correctly positioned when
		nodes are dragged, instead of a separate floating item. */
	void setEventInfo(const QVector<EventInfo> &events);
	/** Briefly flash the segment/node where this event just fired. */
	void flashEvent(const QString &eventName, const QString &trainName);
	/** Show or hide one train's planned route (dashed) and the route it
		actually took (solid), with markers at origin and destination. */
	void showTrainRoute(const QString &key, const QStringList &planned,
						const QStringList &actual, bool on);
	/** Keep an event's location highlighted until toggled off. */
	void showEventMark(const QString &eventName, bool on);
	void clearOverlays();
	/** Briefly mark the node where a train just switched paths. */
	void markReroute(const QString &nodeName, const QString &trainName);

	QStringList nodeNames() const;
	bool hasNode(const QString &name) const;

	/** Draw every rail as lanes (L1 card, colour), as with Multi-line rails. */
	void setLaneMode(bool on);

  signals:
	void nodeClicked(const QString &name);
	/** A train dot was clicked; key is name + id. */
	void trainClicked(const QString &key);

  protected:
	void mousePressEvent(QGraphicsSceneMouseEvent *event) override;

  private:
	struct NodeVis
	{
		DraggableNode *dot;
		QGraphicsTextItem *label;
		QPointF pos;
		/* Comma-joined names of events bound to this node, shown
		   directly in the label - empty means no risk. */
		QString riskNames;
	};
	struct EdgeVis
	{
		QGraphicsPathItem *line; // lane 0
		QGraphicsTextItem *label;
		QString from;
		QString to;
		double distKm;
		double speedKmh;
		QString riskNames;
		int numLines = 1;
		QVector<QGraphicsPathItem *> extraLanes; // lanes 1..N-1
		QVector<double> lineSpeeds;  // per-line km/h, empty = rail speed
		QVector<QGraphicsTextItem *> laneLabels; // "L1 40 km/h ..."
	};
	struct TrainVis
	{
		QGraphicsEllipseItem *dot;
		QGraphicsTextItem *label;
		QPointF prevPos;
		bool hasPrev;
		QString prevFrom;
		QString prevTo;
		double prevFrac = 0.0;
		int prevLine = -1;
	};
	/* Trail breadcrumbs are stored as (edge, fraction) at each endpoint,
	   not raw coordinates, so they can be recomputed when a node moves
	   instead of being left behind at its old position. */
	struct TrailSeg
	{
		QGraphicsLineItem *line;
		QString fromA, toA;
		double fracA;
		QString fromB, toB;
		double fracB;
		int lineA = -1;
		int lineB = -1;
	};

	struct RouteSel
	{
		QStringList planned;
		QStringList actual;
		QColor color;
	};
	QMap<QString, RouteSel> _routeSel;
	QSet<QString> _eventSel;
	QVector<QGraphicsItem *> _overlayItems;
	void rebuildOverlays();
	void stopPulses();
	/** Expanding ring that fades out, 3 times, centred on c; the blink
		lanes flash in the same colour meanwhile. */
	void pulseAt(const QPointF &c, const QColor &col,
				 const QVector<QGraphicsPathItem *> &blink = {});

	QMap<QString, NodeVis> _nodes;
	QVector<EdgeVis> _edges;
	QMap<QString, TrainVis> _trains;
	QVector<TrailSeg> _trailLines;
	QVector<EventInfo> _events;

	void drawEventMarkers();

	void layoutForceDirected();
	bool layoutFromFile();
	QString _layoutFile;
	void repositionEdge(EdgeVis &e);
	QPointF edgePosition(const QString &from, const QString &to,
						 double fraction, int line = -1) const;
	/** Perpendicular shift of a lane so parallel lines never overlap. */
	QPointF laneOffset(const EdgeVis &e, int line, double frac = 0.5) const;
	void setLanePath(QGraphicsPathItem *item, const EdgeVis &e, int line);
	void layoutLanes(EdgeVis &e);
	void clearLanes(EdgeVis &e);
	bool laneMode() const;
	bool _forceLanes = false;
	void refreshEdgeLabels();
	void updateLaneLabels(EdgeVis &e);

	static const QColor PALETTE[];
	static const int PALETTE_SIZE;
};

#endif
