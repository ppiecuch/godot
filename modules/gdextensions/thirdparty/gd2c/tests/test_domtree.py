"""Tests for dominance tree computation."""
import unittest
from gd2c.controlflow import ControlFlowGraph, Edge, Block
from gd2c.domtree import build_domtree_naive


def build_cfg_1():
    """
                      0
                      |
                      v
              +-----  1  -----+
              |       ^       |
              v       |       |
        +---- 2 ---+  |       |
        |          |  |       |
        v          v  |       v
        3          4  +------ 5
        |          |          |
        +-->  6  <-+          |
              |               |
              +---->  7  <----+
                      |
                      v
                      8
    """
    blocks = [Block(str(i)) for i in range(9)]
    cfg = ControlFlowGraph()
    for b in blocks:
        cfg.add_node(b)
    cfg.entry_node = blocks[0]
    cfg.exit_node = blocks[1]
    cfg.add_edge(Edge(blocks[0], blocks[1]))
    cfg.add_edge(Edge(blocks[1], blocks[2]))
    cfg.add_edge(Edge(blocks[2], blocks[3]))
    cfg.add_edge(Edge(blocks[2], blocks[4]))
    cfg.add_edge(Edge(blocks[3], blocks[6]))
    cfg.add_edge(Edge(blocks[4], blocks[6]))
    cfg.add_edge(Edge(blocks[6], blocks[7]))
    cfg.add_edge(Edge(blocks[7], blocks[8]))
    cfg.add_edge(Edge(blocks[1], blocks[5]))
    cfg.add_edge(Edge(blocks[5], blocks[1]))
    cfg.add_edge(Edge(blocks[5], blocks[7]))
    return cfg


class TestDominanceFrontiers(unittest.TestCase):
    def test_dominance_frontiers_cfg_1(self):
        cfg = build_cfg_1()
        dom = build_domtree_naive(cfg)
        self.assertEqual(dom.frontier('0'), set([]))
        self.assertEqual(dom.frontier('1'), set([cfg.node('1')]))
        self.assertEqual(dom.frontier('2'), set([cfg.node('7')]))
        self.assertEqual(dom.frontier('3'), set([cfg.node('6')]))
        self.assertEqual(dom.frontier('4'), set([cfg.node('6')]))
        self.assertEqual(dom.frontier('5'), set([cfg.node('1'), cfg.node('7')]))
        self.assertEqual(dom.frontier('6'), set([cfg.node('7')]))
        self.assertEqual(dom.frontier('7'), set([]))


class TestDominanceTree(unittest.TestCase):
    def test_frontier_of_entry(self):
        cfg = build_cfg_1()
        dom = build_domtree_naive(cfg)
        # Entry node (0) has empty frontier
        self.assertEqual(dom.frontier('0'), set())

    def test_build_does_not_crash(self):
        """Simple smoke test — building domtree should not raise."""
        blocks = [Block(str(i)) for i in range(3)]
        cfg = ControlFlowGraph()
        for b in blocks:
            cfg.add_node(b)
        cfg.entry_node = blocks[0]
        cfg.exit_node = blocks[2]
        cfg.add_edge(Edge(blocks[0], blocks[1]))
        cfg.add_edge(Edge(blocks[1], blocks[2]))
        dom = build_domtree_naive(cfg)
        self.assertIsNotNone(dom)


if __name__ == "__main__":
    unittest.main()
