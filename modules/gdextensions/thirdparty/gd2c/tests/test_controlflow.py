"""Tests for control flow graph construction and analysis."""
import unittest
from gd2c.controlflow import ControlFlowGraph, Edge, Block, Value


def make_linear_cfg(n_blocks):
    """Create a linear CFG: 0 -> 1 -> 2 -> ... -> n-1"""
    blocks = [Block(str(i)) for i in range(n_blocks)]
    cfg = ControlFlowGraph()
    for b in blocks:
        cfg.add_node(b)
    cfg.entry_node = blocks[0]
    cfg.exit_node = blocks[-1]
    for i in range(n_blocks - 1):
        cfg.add_edge(Edge(blocks[i], blocks[i + 1]))
    return cfg, blocks


def make_diamond_cfg():
    """Diamond: 0 -> {1,2} -> 3"""
    blocks = [Block(str(i)) for i in range(4)]
    cfg = ControlFlowGraph()
    for b in blocks:
        cfg.add_node(b)
    cfg.entry_node = blocks[0]
    cfg.exit_node = blocks[3]
    cfg.add_edge(Edge(blocks[0], blocks[1]))
    cfg.add_edge(Edge(blocks[0], blocks[2]))
    cfg.add_edge(Edge(blocks[1], blocks[3]))
    cfg.add_edge(Edge(blocks[2], blocks[3]))
    return cfg, blocks


def make_loop_cfg():
    """Loop: 0 -> 1 -> 2, 2 -> 1 (back edge)"""
    blocks = [Block(str(i)) for i in range(3)]
    cfg = ControlFlowGraph()
    for b in blocks:
        cfg.add_node(b)
    cfg.entry_node = blocks[0]
    cfg.exit_node = blocks[2]
    cfg.add_edge(Edge(blocks[0], blocks[1]))
    cfg.add_edge(Edge(blocks[1], blocks[2]))
    cfg.add_edge(Edge(blocks[2], blocks[1]))
    return cfg, blocks


class TestBlock(unittest.TestCase):
    def test_label(self):
        b = Block("test_label")
        self.assertEqual(b.label, "test_label")

    def test_empty_ops(self):
        b = Block("b")
        self.assertEqual(len(b.ops), 0)

    def test_ins_setter(self):
        b = Block("b")
        b.ins = [1, 2, 3]
        self.assertEqual(b.ins, frozenset([1, 2, 3]))

    def test_outs_setter(self):
        b = Block("b")
        b.outs = [4, 5]
        self.assertEqual(b.outs, frozenset([4, 5]))


class TestValue(unittest.TestCase):
    def test_create(self):
        v = Value(10, 0)
        self.assertEqual(v.name, 10)
        self.assertEqual(v.version, 0)

    def test_str(self):
        v = Value(5, 3)
        self.assertEqual(str(v), "5_3")


class TestEdge(unittest.TestCase):
    def test_create(self):
        b1 = Block("a")
        b2 = Block("b")
        e = Edge(b1, b2)
        self.assertEqual(e.source, b1)
        self.assertEqual(e.dest, b2)

    def test_equality(self):
        b1, b2 = Block("a"), Block("b")
        e1 = Edge(b1, b2)
        e2 = Edge(b1, b2)
        self.assertEqual(e1, e2)


class TestControlFlowGraph(unittest.TestCase):
    def test_linear_cfg(self):
        cfg, blocks = make_linear_cfg(4)
        self.assertEqual(cfg.entry_node, blocks[0])
        self.assertEqual(cfg.exit_node, blocks[3])

    def test_successors(self):
        cfg, blocks = make_linear_cfg(3)
        succs = list(cfg.succs(blocks[0]))
        self.assertEqual(len(succs), 1)
        self.assertEqual(succs[0], blocks[1])

    def test_predecessors(self):
        cfg, blocks = make_linear_cfg(3)
        preds = list(cfg.preds(blocks[2]))
        self.assertEqual(len(preds), 1)
        self.assertEqual(preds[0], blocks[1])

    def test_diamond_successors(self):
        cfg, blocks = make_diamond_cfg()
        succs = set(cfg.succs(blocks[0]))
        self.assertEqual(succs, {blocks[1], blocks[2]})

    def test_diamond_predecessors(self):
        cfg, blocks = make_diamond_cfg()
        preds = set(cfg.preds(blocks[3]))
        self.assertEqual(preds, {blocks[1], blocks[2]})

    def test_loop_back_edge(self):
        cfg, blocks = make_loop_cfg()
        succs = set(cfg.succs(blocks[2]))
        self.assertIn(blocks[1], succs)

    def test_node_lookup(self):
        cfg, blocks = make_linear_cfg(3)
        self.assertEqual(cfg.node("0"), blocks[0])
        self.assertEqual(cfg.node("2"), blocks[2])

    def test_nodes_iteration(self):
        cfg, blocks = make_linear_cfg(5)
        all_nodes = list(cfg.nodes())
        self.assertEqual(len(all_nodes), 5)


if __name__ == "__main__":
    unittest.main()
