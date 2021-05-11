from __future__ import annotations
from typing import List, Optional, Union, Iterable, Tuple, Set, Dict, FrozenSet, Any, Callable
from gd2c.bytecode import GDScriptOp, IterateBeginGDScriptOp, IterateGDScriptOp, JumpToDefaultArgumentGDScriptOp, PseudoGDScriptOp, DefineGDScriptOp, ParameterGDScriptOp, JumpGDScriptOp, JumpIfGDScriptOp, JumpIfNotGDScriptOp, JumpToDefaultArgumentGDScriptOp, ReturnGDScriptOp, EndGDScriptOp
from gd2c.address import *
from gd2c.gdscriptclass import GDScriptFunction
from gd2c.variant import VariantType

EXIT_NODE_ADDRESS = 2_147_483_646

BLOCK_TYPE_NORMAL = 0
BLOCK_TYPE_BRANCH = 1
BLOCK_TYPE_RETURN = 2
BLOCK_TYPE_EXIT = 3
BLOCK_TYPE_DEFARGS = 4

class Value:
    # identifies the source address of the value
    name: int
    # identifies the unique definition of the value
    version: int
    # type of value
    vtype: VariantType

    def __init__(self, name: int, version: int):
        self.name = name
        self.version = version

    def __str__(self):
        return f"{self.name}_{self.version}"

class Block:
    def __init__(self, label: str):
        self.edges: Set['Edge'] = set([])
        self._label = label
        self._ops: List[GDScriptOp] = []
        self._locked = False
        self._defs: FrozenSet[int] = frozenset()
        self._uses: FrozenSet[int] = frozenset()
        self._ins: FrozenSet[int] = frozenset([])
        self._outs: FrozenSet[int] = frozenset([])

    @property
    def label(self):
        return self._label

    @property
    def ins(self) -> FrozenSet[int]:
        return self._ins

    @ins.setter
    def ins(self, value: Iterable[int]):
        self._ins = frozenset(value)

    @property
    def outs(self) -> FrozenSet[int]:
        return self._outs

    @outs.setter
    def outs(self, value: Iterable[int]):
        self._outs = frozenset(value)

    @property
    def ops(self) -> Iterable[GDScriptOp]:
        return list(self._ops)

    @property
    def last_op(self) -> Optional[GDScriptOp]:
        if any(self._ops):
            return self._ops[-1]

        return None

    @property
    def first_op(self) -> Optional[GDScriptOp]:
        if any(self._ops):
            return self._ops[0]

        return None

    @property
    def defs(self) -> FrozenSet[int]:
        return self._defs

    @defs.setter
    def defs(self, value: Iterable[int]):
        self._defs = frozenset(value)

    @property
    def uses(self) -> FrozenSet[int]:
        return self._uses

    @uses.setter
    def uses(self, value: Iterable[int]):
        self._uses = frozenset(value)

    @property
    def locked(self):
        """Indicates that no changes that could alter control flow may be added"""
        return self._locked

    def lock(self):
        """Lock the Block so that instructions that may alter control flow cannot be added"""
        self._locked = True

    def append_op(self, op: GDScriptOp):
        """Appends operation to the end of the block. If the block is locked, inserting
        a branch instruction will raise an error."""

        if self._locked and isinstance(op, (JumpGDScriptOp, JumpIfGDScriptOp, JumpIfNotGDScriptOp, ReturnGDScriptOp)):
            raise Exception("Cannot insert instruction that would alter control flow")

        self._ops.append(op)

    def append_ops(self, ops: Iterable[GDScriptOp]):
        """Appends operations to the end of the block. If the block is locked, inserting
        a branch instruction will raise an error."""
        for op in ops:
            self.append_op(op)

    def insert_ops_before(self, insert_before: Union[GDScriptOp], ops: Iterable[GDScriptOp]):
        """Insert a sequence of ops into the block before GDScriptOp insert_before.

        Attempting to insert a branch instruction will raise an exception if the block is locked.
        If a branch operation is to be inserted, you must split the block and insert new
        nodes into the control flow graph.

        Args:
        insert_before (GDScriptOp): Existing op to insert new ops before.
        ops (Iterable[GDScriptOp]): Sequence of ops to insert in order.
        """
        index = self._ops.index(insert_before)
        for op in ops:
            if self._locked and isinstance(op, (JumpGDScriptOp, JumpIfGDScriptOp, JumpIfNotGDScriptOp, ReturnGDScriptOp)):
                raise Exception("Cannot insert instruction that would alter control flow")
            self._ops.insert(index, op)
            index += 1

    def replace_op(self, existing_op: GDScriptOp, replacement_op: GDScriptOp):
        """Replaces the specified op with a new op"""
        self.insert_ops_before(existing_op, [replacement_op])

    def remove_op(self, existing_op: GDScriptOp):
        """Removes the specified op"""
        self._ops.remove(existing_op)

    def replace_branch_address(self, old_addr: int, new_addr: int):
        """Change any instance of old_addr in a branch to new_addr"""
        for op in self._ops:
            if self._locked and isinstance(op, (JumpGDScriptOp, JumpIfGDScriptOp, JumpIfNotGDScriptOp, IterateGDScriptOp, IterateBeginGDScriptOp)):
                if op.fallthrough == old_addr:
                    op.fallthrough = new_addr
                if op.branch == old_addr:
                    op.branch = new_addr
            elif self._locked and isinstance(op, JumpToDefaultArgumentGDScriptOp):
                if op.fallthrough == old_addr:
                    op.fallthrough = new_addr
                for i, ip in enumerate(op.jump_table):
                    if ip == old_addr:
                        op.jump_table[i] = new_addr

    def update_def_use(self):
        defs = set()
        uses = set()

        # defs are any variables that are written to within block
        # uses are any variables that are read prior to being written to within the block
        for op in self._ops:
            uses.update(op.reads - defs)
            defs.update(op.writes)

        self._defs = frozenset(defs)
        self._uses = frozenset(uses)

class Edge:
    def __init__(self, source: Block, dest: Block):
        self._source = source
        self._dest = dest

    @property
    def source(self) -> Block:
        return self._source

    @property
    def dest(self) -> Block:
        return self._dest

    def __hash__(self):
        return hash((self._source, self._dest))

    def __eq__(self, other) -> bool:
        return other and self.source == other.source and self.dest == other.dest

class ControlFlowGraph:
    def __init__(self):
        self._nodes: Dict[str, Block] = {}
        self._edges: Dict[Tuple[Block, Block], Edge] = {}
        self._entry_node: Optional[Block] = None
        self._exit_node: Optional[Block] = None
        self._value_version = 0
        self.is_in_ssa_form = False

    def new_value(self, address: int, vtype: VariantType) -> Value:
        v = Value(address, self._value_version)
        v.vtype = vtype
        self._value_version += 1
        return v

    def add_edge(self, edge: Edge):
        if (edge.source, edge.dest) in self._edges:
            return

        self._edges[(edge.source, edge.dest)] = edge

    def add_node(self, node: Block):
        self._nodes[node.label] = node

    def find_edge(self, source: Block, dest: Block) -> Optional[Edge]:
        return self._edges.get((source, dest), None)

    def preds(self, node: Block) -> Set[Block]:
        return set(map(lambda e: e.source, filter(lambda e: e.dest == node, self._edges.values())))

    def succs(self, node: Block) -> Set[Block]:
        return set(map(lambda e: e.dest, filter(lambda e: e.source == node, self._edges.values())))

    def back_edges(self, node: Block) -> Set[Edge]:
        return set(filter(lambda e: e.source == node, list(self._edges.values())))

    def forward_edges(self, node: Block) -> Set[Edge]:
        return set(filter(lambda e: e.dest == node, list(self._edges.values())))

    def remove_edge(self, edge: Edge):
        del self._edges[(edge.source, edge.dest)]

    def remove_node(self, node: Block):
        if any(self.preds(node)) or any(self.succs(node)):
            raise Exception("Cannot delete node with edges. Remove and resolve edges first.")

        del self._nodes[node.label]

    @property
    def entry_node(self) -> Optional[Block]:
        return self._entry_node
    @entry_node.setter
    def entry_node(self, value: Block):
        self._entry_node = value

    @property
    def exit_node(self) -> Optional[Block]:
        return self._exit_node
    @exit_node.setter
    def exit_node(self, value: Block):
        self._exit_node = value

    def nodes(self) -> Iterable[Block]:
        return self._nodes.values()

    def node(self, label: str) -> Optional[Block]:
        return self._nodes.get(label, None)

    def node_label_from_address(self, original_node_address: int) -> str:
        return f"_{original_node_address}"

    def node_from_address(self, original_node_address: int) -> Block:
        label = self.node_label_from_address(original_node_address)
        return self._nodes[label]

    def visit_nodes(self, visitor: Callable[[Block], None]):
        assert self.entry_node

        worklist: List[Block] = [self.entry_node]
        visited: Set[Block] = set([])
        while any(worklist):
            node = worklist.pop()
            if node in visited:
                continue

            visitor(node)

            visited.add(node)
            worklist.extend(self.succs(node))

    def live_variable_analysis(self):
        class def_use:
            def __init__(self):
                self.ins = set()
                self.outs = set()
                self.defs = set()
                self.uses = set()

        defuses = {}
        for node in self.nodes():
            node.update_def_use()

            du = def_use()
            du.defs = node.defs
            du.ins = set(node.uses)
            defuses[node] = du

        # propagate live variables througout the CFG. May take
        # multiple iterations. We're done when we complete an
        # iteration that makes no changes
        while True:
            worklist: List[ControlFlowGraphNode] = [self.exit_node]
            done = True
            visited: Set[ControlFlowGraphNode] = set()

            while any(worklist):
                node = worklist.pop()
                assert node
                if node in visited:
                    continue

                du = defuses[node]
                pre_ins_count = len(du.ins)
                pre_outs_count = len(du.outs)

                for succ in self.succs(node):
                    du.outs.update(defuses[succ].ins)
                du.ins.update(du.outs - du.defs)

                # see if we made any changes. We only need to check counts on
                # sets because liveness analysis only ever grows the sets.
                if len(du.outs) > pre_outs_count or len(du.ins) > pre_ins_count:
                   done = False

                visited.add(node)
                worklist.extend(self.preds(node))

            if done:
                break

        # update cfg nodes with results of liveness analysis
        for node in self.nodes():
            du = defuses[node]
            node.ins = du.ins
            node.outs = du.outs

    def update_function(self, func: GDScriptFunction):
        assert func
        assert not self.is_in_ssa_form

        ops: List[Tuple[int, GDScriptOp]] = []
        ip = 0
        node_ip: Dict[str, int] = {}

        # First write ops, remembering block addresses
        worklist = [self._entry_node]
        visited: Set[Block] = set()
        while any(worklist):
            node = worklist.pop()
            assert node
            if node in visited:
                continue

            node_ip[node.label] = ip
            visited.add(node)

            for op in node.ops:
                if not isinstance(op, PseudoGDScriptOp):
                    ops.append((ip, op))
                    ip += op.stride

            worklist.extend(self.succs(node))

        # Update jumps with correct block addresses
        for node in visited:
            for op in node.ops:
                if isinstance(op, (JumpGDScriptOp, JumpIfGDScriptOp, JumpIfNotGDScriptOp, IterateGDScriptOp, IterateBeginGDScriptOp)):
                    branch = self.node_label_from_address(op.branch)
                    fallthrough = self.node_label_from_address(op.fallthrough)
                    op.branch = node_ip[branch]
                    op.fallthrough = node_ip[fallthrough]
                elif isinstance(op, JumpToDefaultArgumentGDScriptOp):
                    op.fallthrough = node_ip[self.node_label_from_address(op.fallthrough)]
                    for i, ip in enumerate(op.jump_table):
                        label = self.node_label_from_address(ip)
                        op.jump_table[i] = node_ip[label]

        # Finally update function ops.
        # Must do this in the same order as above loops
        ip = 0
        func.clear_ops()
        for ip, op in ops:
            func.add_op(ip, op)

    def pretty_print(self, print_def_use: bool = True, print_in_out: bool = True):
        worklist = [self._entry_node]
        visited: Set[Block] = set()

        print("-- BEGIN CFG --")

        while any(worklist):
            node = worklist.pop()
            assert node
            if node in visited:
                continue

            visited.add(node)

            if ('0' in self._nodes) and isinstance(node.last_op, (JumpGDScriptOp, JumpIfNotGDScriptOp, IterateGDScriptOp, IterateBeginGDScriptOp)):
                # Here we assume nodes are labeled with the address of the block that created them
                worklist.append(self._nodes[str(node.last_op.branch)])
                worklist.append(self._nodes[str(node.last_op.fallthrough)])
            elif ('0' in self._nodes) and isinstance(node.last_op, JumpIfGDScriptOp):
                # Here we assume nodes are labeled with the address of the block that created them
                worklist.append(self._nodes[str(node.last_op.fallthrough)])
                worklist.append(self._nodes[str(node.last_op.branch)])
            elif ('0' in self._nodes) and isinstance(node.last_op, JumpToDefaultArgumentGDScriptOp):
                worklist.append(self._nodes[str(node.last_op.fallthrough)])
                worklist.extend([self._nodes[str(ip)] for ip in node.last_op.jump_table])
            else:
                worklist.extend(self.succs(node))

            succs = [n.dest.label for n in node.edges]

            print(f"+------------------------------------------------")
            print(f"| Block: {node.label}")
            print(f"| Preds: {', '.join(map(lambda n: n.label, self.preds(node)))} : Succs: {', '.join(succs)}")
            print(f"|------------------------------------------------")
            if print_def_use:
                print(f"| Defs: {', '.join(map(lambda a: str(a), node.defs))}")
                print(f"| Uses: {', '.join(map(lambda a: str(a), node.uses))}")
            if print_in_out:
                print(f"| Ins: {', '.join(map(lambda a: str(a), node.ins))}")
                print(f"| Outs: {', '.join(map(lambda a: str(a), node.outs))}")
            if print_def_use or print_in_out:
                print(f"|------------------------------------------------")

            if any(node.ops):
                for op in node.ops:
                    print(f"|    {str(op)}")
            else:
                print(f"|    EMPTY BLOCK")

            print("")

        print("-- END CFG --")

def build_control_flow_graph(func: GDScriptFunction) -> ControlFlowGraph:
    worklist: List[int] = [0]
    blocks: Dict[int, Block] = {}
    visited: Set[int] = set()
    ops = list(func.ops())

    # Clear reached flag on all ops to be set later.
    # Godot inserts some unreachable instructions which throws off
    # cfg generation. There may be more elegant solutions to the problems
    # created by the unreachable instructions, but this is the easiest way
    # to handle it.
    func.clear_op_reached_flags()

    def get_block(ip: int):
        nonlocal blocks
        if ip in blocks:
            return blocks[ip]
        else:
            block = Block(f'_{ip}')
            blocks[ip] = block
            return block

    while worklist:
        start_ip = worklist.pop()
        if start_ip in visited:
            continue

        visited.add(start_ip)

        block = get_block(start_ip)

        i = 0
        while i < len(ops):
            ip, op = ops[i]
            if ip < start_ip:
                i += 1
                continue

            op.reached = True

            if isinstance(op, (JumpGDScriptOp, JumpIfGDScriptOp, JumpIfNotGDScriptOp, IterateGDScriptOp, IterateBeginGDScriptOp)):
                worklist.append(op.branch)
                worklist.append(op.fallthrough)
                break
            elif isinstance(op, JumpToDefaultArgumentGDScriptOp):
                worklist.extend(op.jump_table)
                worklist.append(op.fallthrough)
                break
            elif isinstance(op, (ReturnGDScriptOp, EndGDScriptOp)):
                worklist.append(EXIT_NODE_ADDRESS)
                break

            i += 1

    # Add ops to blocks
    begin_blocks = sorted(visited)
    for i, begin_ip in enumerate(begin_blocks):
        if i + 1 < len(begin_blocks):
            end_ip = begin_blocks[i + 1]
            block = get_block(begin_ip)
            for op_ip, op in ops:
                if op_ip >= end_ip:
                    break
                elif op_ip >= begin_ip and op.reached:
                    block.append_op(op)

            # Every block must end in a branch instruction unless it is
            # the terminating block
            if block.last_op and not block.last_op.is_branch:
                block.append_op(JumpGDScriptOp(end_ip))

    # Add edges to blocks
    for block in blocks.values():
        for op in block.ops:
            if isinstance(op, (JumpGDScriptOp, JumpIfGDScriptOp, JumpIfNotGDScriptOp, IterateGDScriptOp, IterateBeginGDScriptOp)):
                block.edges.add(Edge(block, get_block(op.branch)))
                block.edges.add(Edge(block, get_block(op.fallthrough)))
            elif isinstance(op, JumpToDefaultArgumentGDScriptOp):
                block.edges.update([Edge(block, get_block(jip)) for jip in op.jump_table])
                block.edges.add(Edge(block, get_block(op.fallthrough)))
            elif isinstance(op, (ReturnGDScriptOp, EndGDScriptOp)):
                block.edges.add(Edge(block, get_block(EXIT_NODE_ADDRESS)))

    # Populate new control flow graph
    cfg = ControlFlowGraph()
    cfg._exit_node = get_block(EXIT_NODE_ADDRESS)
    assert cfg._exit_node
    cfg.add_node(cfg._exit_node)
    for block in blocks.values():
        cfg.add_node(block)
        for edge in block.edges:
            cfg.add_edge(edge)

    # Add synthetic entry node
    cfg._entry_node = Block('_entry')
    cfg.add_node(cfg._entry_node)
    # Add instructions to define used constants/parameters. These ops generally have
    # no executable effect but are required for data flow analysis and SSA form
    parameter_ops = [ParameterGDScriptOp(p) for p in func.parameters()]
    parameter_addresses = set([p.address.address for p in func.parameters()])
    define_ops = [DefineGDScriptOp(c.address.address) for c in func.constants()]
    for p in parameter_ops:
        cfg._entry_node.append_op(p)
    for dd in define_ops:
        cfg._entry_node.append_op(dd)
    # Final up must be branch to block 0
    cfg._entry_node.append_op(JumpGDScriptOp(0))
    entry_edge = Edge(cfg._entry_node, get_block(0))
    cfg._entry_node.edges.add(entry_edge)
    cfg.add_edge(entry_edge)

    for node in blocks.values():
        node.lock()

    return cfg












