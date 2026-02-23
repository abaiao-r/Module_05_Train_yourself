/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NetworkScene.hpp                                   :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ctw03933 <ctw03933@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2026/02/22 18:30:00 by ctw03933          #+#    #+#             */
/*   Updated: 2026/02/23 00:53:56 by ctw03933         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NETWORKSCENE_HPP
#define NETWORKSCENE_HPP

#include <QGraphicsEllipseItem>
#include <QGraphicsLineItem>
#include <QGraphicsPathItem>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QMap>
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

/* ────────────────────────────────────────────────────────────────────────── */
/*  DraggableNode                                                            */
/*  Custom QGraphicsEllipseItem that notifies the scene when it moves,       */
/*  so edges and labels track the node in real time.                         */
/* ────────────────────────────────────────────────────────────────────────── */
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

/* ────────────────────────────────────────────────────────────────────────── */
/*  NetworkScene                                                             */
/* ────────────────────────────────────────────────────────────────────────── */
class NetworkScene : public QGraphicsScene
{
	Q_OBJECT

  public:
	explicit NetworkScene(QObject *parent = nullptr);
	~NetworkScene();

	/* ─── Network building ──────────────────────────────────────────── */
	void addNode(const QString &name, const QPointF &pos);
	void addEdge(const QString &from, const QString &to,
				 double distKm, double speedKmh);
	void removeNode(const QString &name);
	void removeEdge(const QString &from, const QString &to);
	void renameNode(const QString &oldName, const QString &newName);
	void updateEdge(const QString &from, const QString &to,
					double newDist, double newSpeed);
	void clear();

	/** Populate from an existing RailNetwork object. */
	void loadFromNetwork(const RailNetwork &network);

	/* Called by DraggableNode::itemChange when a node is dragged */
	void nodeMoved(const QString &name, const QPointF &newPos);

	/* ─── Train visualisation ──────────────────────────────────────── */
	void updateTrains(double simTime,
					  const QVector<TrainSnapshot> &snapshots);
	void clearTrains();

	/* ─── Accessors ────────────────────────────────────────────────── */
	QStringList nodeNames() const;
	bool hasNode(const QString &name) const;

  signals:
	void nodeClicked(const QString &name);

  private:
	struct NodeVis
	{
		DraggableNode *dot;
		QGraphicsTextItem *label;
		QPointF pos;
	};
	struct EdgeVis
	{
		QGraphicsLineItem *line;
		QGraphicsTextItem *label;
		QString from;
		QString to;
		double distKm;
		double speedKmh;
	};
	struct TrainVis
	{
		QGraphicsEllipseItem *dot;
		QGraphicsTextItem *label;
		QPointF prevPos;
		bool hasPrev;
	};

	QMap<QString, NodeVis> _nodes;
	QVector<EdgeVis> _edges;
	QMap<QString, TrainVis> _trains;
	QVector<QGraphicsLineItem *> _trailLines;

	void layoutForceDirected();
	void repositionEdge(EdgeVis &e);
	QPointF edgePosition(const QString &from, const QString &to,
						 double fraction) const;

	static const QColor PALETTE[];
	static const int PALETTE_SIZE;
};

#endif
