#pragma once

#include "data/data.h"

class DDLS_Graph {
	unsigned id;

	DDLSGraphNode node;
	DDLSGraphEdge edge;

public:
	unsigned get_id() const { return id; }

	DDLSGraphEdge get edge() const;
	DDLSGraphNode get node() const;
	DDLSGraphNode insert_node();
	void delete_node(DDLSGraphNode p_node);
	DDLSGraphEdge insert_edge(DDLSGraphNode p_from_node, DDLSGraphNode p_to_node);
	void delete_edge(DDLSGraphEdge p_edge);

	DDLSGraph();
	~DDLSGraph();
};
