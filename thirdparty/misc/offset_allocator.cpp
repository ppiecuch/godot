// (C) Sebastian Aaltonen 2023
// MIT License (see file: LICENSE)

#include "offset_allocator.h"

#include "core/error_macros.h"
#include "core/variant.h"

#ifdef _MSC_VER
#include <intrin.h>
#endif

#include <cstring>

namespace offset_allocator
{
    inline uint32 lzcnt_nonzero(uint32 v)
    {
#ifdef _MSC_VER
        unsigned long retVal;
        _BitScanReverse(&retVal, v);
        return 31 - retVal;
#else
        return __builtin_clz(v);
#endif
    }

    inline uint32 tzcnt_nonzero(uint32 v)
    {
#ifdef _MSC_VER
        unsigned long retVal;
        _BitScanForward(&retVal, v);
        return retVal;
#else
        return __builtin_ctz(v);
#endif
    }

    namespace SmallFloat
    {
        static constexpr uint32 MANTISSA_BITS = 3;
        static constexpr uint32 MANTISSA_VALUE = 1 << MANTISSA_BITS;
        static constexpr uint32 MANTISSA_MASK = MANTISSA_VALUE - 1;
    
        // Bin sizes follow floating point (exponent + mantissa) distribution (piecewise linear log approx)
        // This ensures that for each size class, the average overhead percentage stays the same
        uint32 uintToFloatRoundUp(uint32 size)
        {
            uint32 exp = 0;
            uint32 mantissa = 0;
            
            if (size < MANTISSA_VALUE)
            {
                // Denorm: 0..(MANTISSA_VALUE-1)
                mantissa = size;
            } else {
                // Normalized: Hidden high bit always 1. Not stored. Just like float.
                uint32 leadingZeros = lzcnt_nonzero(size);
                uint32 highestSetBit = 31 - leadingZeros;
                
                uint32 mantissaStartBit = highestSetBit - MANTISSA_BITS;
                exp = mantissaStartBit + 1;
                mantissa = (size >> mantissaStartBit) & MANTISSA_MASK;
                
                uint32 lowBitsMask = (1 << mantissaStartBit) - 1;
                
                // Round up!
                if ((size & lowBitsMask) != 0)
                    mantissa++;
            }
            
            return (exp << MANTISSA_BITS) + mantissa; // + allows mantissa->exp overflow for round up
        }

        uint32 uintToFloatRoundDown(uint32 size)
        {
            uint32 exp = 0;
            uint32 mantissa = 0;
            
            if (size < MANTISSA_VALUE) {
                mantissa = size; // Denorm: 0..(MANTISSA_VALUE-1)
            } else {
                // Normalized: Hidden high bit always 1. Not stored. Just like float.
                uint32 leadingZeros = lzcnt_nonzero(size);
                uint32 highestSetBit = 31 - leadingZeros;
                
                uint32 mantissaStartBit = highestSetBit - MANTISSA_BITS;
                exp = mantissaStartBit + 1;
                mantissa = (size >> mantissaStartBit) & MANTISSA_MASK;
            }
            
            return (exp << MANTISSA_BITS) | mantissa;
        }
    
        uint32 floatToUint(uint32 floatValue)
        {
            uint32 exponent = floatValue >> MANTISSA_BITS;
            uint32 mantissa = floatValue & MANTISSA_MASK;
            if (exponent == 0) {
                return mantissa; // Denorms
            } else {
                return (mantissa | MANTISSA_VALUE) << (exponent - 1);
            }
        }
    }

    // Utility functions
    uint32 findLowestSetBitAfter(uint32 bitMask, uint32 startBitIndex)
    {
        uint32 maskBeforeStartIndex = (1 << startBitIndex) - 1;
        uint32 maskAfterStartIndex = ~maskBeforeStartIndex;
        uint32 bitsAfter = bitMask & maskAfterStartIndex;
        if (bitsAfter == 0) return Allocation::NO_SPACE;
        return tzcnt_nonzero(bitsAfter);
    }

    // Allocator...
    Allocator::Allocator(uint32 size, uint32 allocs) :
        allocSize(size),
        maxAllocs(allocs),
        nodes(nullptr),
        freeNodes(nullptr)
    {
        if (sizeof(NodeIndex) == 2)
        {
#ifdef USE_16_BIT_NODE_INDICES
            DEV_ASSERT(allocs <= 32768);
#else
            DEV_ASSERT(allocs <= 65536);
#endif
        }
        reset();
    }

    Allocator::Allocator(Allocator &&other) :
        allocSize(other.allocSize),
        maxAllocs(other.maxAllocs),
        freeStorage(other.freeStorage),
        usedBinsTop(other.usedBinsTop),
        nodes(other.nodes),
        freeNodes(other.freeNodes),
        freeOffset(other.freeOffset)
    {
        memcpy(usedBins, other.usedBins, sizeof(uint8) * NUM_TOP_BINS);
        memcpy(binIndices, other.binIndices, sizeof(NodeIndex) * NUM_LEAF_BINS);

        other.nodes = nullptr;
        other.freeNodes = nullptr;
        other.freeOffset = 0;
        other.maxAllocs = 0;
        other.usedBinsTop = 0;
    }

    void Allocator::reset()
    {
        freeStorage = 0;
        usedBinsTop = 0;
        freeOffset = maxAllocs - 1;

        for (uint32 i = 0 ; i < NUM_TOP_BINS; i++)
            usedBins[i] = 0;
        
        for (uint32 i = 0 ; i < NUM_LEAF_BINS; i++)
            binIndices[i] = Node::unused;
        
        if (nodes) delete[] nodes;
        if (freeNodes) delete[] freeNodes;

        nodes = new Node[maxAllocs];
        freeNodes = new NodeIndex[maxAllocs];
        
        // Freelist is a stack. Nodes in inverse order so that [0] pops first.
        for (uint32 i = 0; i < maxAllocs; i++)
            freeNodes[i] = maxAllocs - i - 1;
        
        // Start state: Whole storage as one big node
        // Algorithm will split remainders and push them back as smaller nodes
        insertNodeIntoBin(allocSize, 0);
    }

    Allocator::~Allocator()
    {
        delete[] nodes;
        delete[] freeNodes;
    }
    
    Allocation Allocator::allocate(uint32 size)
    {
        // Out of allocations?
        if (freeOffset == 0)
        {
            return {.offset = Allocation::NO_SPACE, .metadata = Allocation::NO_SPACE};
        }
        
        // Round up to bin index to ensure that alloc >= bin
        // Gives us min bin index that fits the size
        uint32 minBinIndex = SmallFloat::uintToFloatRoundUp(size);
        
        uint32 minTopBinIndex = minBinIndex >> TOP_BINS_INDEX_SHIFT;
        uint32 minLeafBinIndex = minBinIndex & LEAF_BINS_INDEX_MASK;
        
        uint32 topBinIndex = minTopBinIndex;
        uint32 leafBinIndex = Allocation::NO_SPACE;

        // If top bin exists, scan its leaf bin. This can fail (NO_SPACE).
        if (usedBinsTop & (1 << topBinIndex))
        {
            leafBinIndex = findLowestSetBitAfter(usedBins[topBinIndex], minLeafBinIndex);
        }
    
        // If we didn't find space in top bin, we search top bin from +1
        if (leafBinIndex == Allocation::NO_SPACE)
        {
            topBinIndex = findLowestSetBitAfter(usedBinsTop, minTopBinIndex + 1);
            
            // Out of space?
            if (topBinIndex == Allocation::NO_SPACE)
            {
                return {.offset = Allocation::NO_SPACE, .metadata = Allocation::NO_SPACE};
            }

            // All leaf bins here fit the alloc, since the top bin was rounded up. Start leaf search from bit 0.
            // NOTE: This search can't fail since at least one leaf bit was set because the top bit was set.
            leafBinIndex = tzcnt_nonzero(usedBins[topBinIndex]);
        }
                
        uint32 binIndex = (topBinIndex << TOP_BINS_INDEX_SHIFT) | leafBinIndex;
        
        // Pop the top node of the bin. Bin top = node.next.
        uint32 nodeIndex = binIndices[binIndex];
        Node& node = nodes[nodeIndex];
        uint32 nodeTotalSize = node.dataSize;
        node.dataSize = size;
        node.used = true;
        binIndices[binIndex] = node.binListNext;
        if (node.binListNext != Node::unused) nodes[node.binListNext].binListPrev = Node::unused;
        freeStorage -= nodeTotalSize;
#ifdef DEBUG_ENABLED
        print_verbose(vformat("Free storage: %u (-%u) (allocate)\n", freeStorage, nodeTotalSize));
#endif

        // Bin empty?
        if (binIndices[binIndex] == Node::unused)
        {
            // Remove a leaf bin mask bit
            usedBins[topBinIndex] &= ~(1 << leafBinIndex);
            
            // All leaf bins empty?
            if (usedBins[topBinIndex] == 0)
            {
                // Remove a top bin mask bit
                usedBinsTop &= ~(1 << topBinIndex);
            }
        }
        
        // Push back reminder N elements to a lower bin
        uint32 reminderSize = nodeTotalSize - size;
        if (reminderSize > 0)
        {
            uint32 newNodeIndex = insertNodeIntoBin(reminderSize, node.dataOffset + size);
            
            // Link nodes next to each other so that we can merge them later if both are free
            // And update the old next neighbor to point to the new node (in middle)
            if (node.neighborNext != Node::unused) nodes[node.neighborNext].neighborPrev = newNodeIndex;
            nodes[newNodeIndex].neighborPrev = nodeIndex;
            nodes[newNodeIndex].neighborNext = node.neighborNext;
            node.neighborNext = newNodeIndex;
        }
        
        return {.offset = node.dataOffset, .metadata = nodeIndex};
    }
    
    void Allocator::free(Allocation allocation)
    {
        DEV_ASSERT(allocation.metadata != Allocation::NO_SPACE);
        if (!nodes) return;
        
        uint32 nodeIndex = allocation.metadata;
        Node& node = nodes[nodeIndex];
        
        // Double delete check
        DEV_ASSERT(node.used == true);
        
        // Merge with neighbors...
        uint32 offset = node.dataOffset;
        uint32 size = node.dataSize;
        
        if ((node.neighborPrev != Node::unused) && (nodes[node.neighborPrev].used == false))
        {
            // Previous (contiguous) free node: Change offset to previous node offset. Sum sizes
            Node& prevNode = nodes[node.neighborPrev];
            offset = prevNode.dataOffset;
            size += prevNode.dataSize;
            
            // Remove node from the bin linked list and put it in the freelist
            removeNodeFromBin(node.neighborPrev);
            
            DEV_ASSERT(prevNode.neighborNext == nodeIndex);
            node.neighborPrev = prevNode.neighborPrev;
        }
        
        if ((node.neighborNext != Node::unused) && (nodes[node.neighborNext].used == false))
        {
            // Next (contiguous) free node: Offset remains the same. Sum sizes.
            Node& nextNode = nodes[node.neighborNext];
            size += nextNode.dataSize;
            
            // Remove node from the bin linked list and put it in the freelist
            removeNodeFromBin(node.neighborNext);
            
            DEV_ASSERT(nextNode.neighborPrev == nodeIndex);
            node.neighborNext = nextNode.neighborNext;
        }

        uint32 neighborNext = node.neighborNext;
        uint32 neighborPrev = node.neighborPrev;
        
        // Insert the removed node to freelist
#ifdef DEBUG_ENABLED
        print_verbose(vformat("Putting node %u into freelist[%u] (free)\n", nodeIndex, freeOffset + 1));
#endif
        freeNodes[++freeOffset] = nodeIndex;

        // Insert the (combined) free node to bin
        uint32 combinedNodeIndex = insertNodeIntoBin(size, offset);

        // Connect neighbors with the new combined node
        if (neighborNext != Node::unused)
        {
            nodes[combinedNodeIndex].neighborNext = neighborNext;
            nodes[neighborNext].neighborPrev = combinedNodeIndex;
        }
        if (neighborPrev != Node::unused)
        {
            nodes[combinedNodeIndex].neighborPrev = neighborPrev;
            nodes[neighborPrev].neighborNext = combinedNodeIndex;
        }
    }

    uint32 Allocator::insertNodeIntoBin(uint32 size, uint32 dataOffset)
    {
        // Round down to bin index to ensure that bin >= alloc
        uint32 binIndex = SmallFloat::uintToFloatRoundDown(size);
        
        uint32 topBinIndex = binIndex >> TOP_BINS_INDEX_SHIFT;
        uint32 leafBinIndex = binIndex & LEAF_BINS_INDEX_MASK;
        
        // Bin was empty before?
        if (binIndices[binIndex] == Node::unused)
        {
            // Set bin mask bits
            usedBins[topBinIndex] |= 1 << leafBinIndex;
            usedBinsTop |= 1 << topBinIndex;
        }
        
        // Take a freelist node and insert on top of the bin linked list (next = old top)
        uint32 topNodeIndex = binIndices[binIndex];
        uint32 nodeIndex = freeNodes[freeOffset--];
#ifdef DEBUG_ENABLED
        print_verbose(vformat("Getting node %u from freelist[%u]\n", nodeIndex, freeOffset + 1));
#endif
        nodes[nodeIndex] = {.dataOffset = dataOffset, .dataSize = size, .binListNext = topNodeIndex};
        if (topNodeIndex != Node::unused) nodes[topNodeIndex].binListPrev = nodeIndex;
        binIndices[binIndex] = nodeIndex;
        
        freeStorage += size;
#ifdef DEBUG_ENABLED
        print_verbose(vformat("Free storage: %u (+%u) (insertNodeIntoBin)\n", freeStorage, size));
#endif

        return nodeIndex;
    }
    
    void Allocator::removeNodeFromBin(uint32 nodeIndex)
    {
        Node &node = nodes[nodeIndex];
        
        if (node.binListPrev != Node::unused) {
            // Easy case: We have previous node. Just remove this node from the middle of the list.
            nodes[node.binListPrev].binListNext = node.binListNext;
            if (node.binListNext != Node::unused) nodes[node.binListNext].binListPrev = node.binListPrev;
        } else {
            // Hard case: We are the first node in a bin. Find the bin.
            
            // Round down to bin index to ensure that bin >= alloc
            uint32 binIndex = SmallFloat::uintToFloatRoundDown(node.dataSize);
            
            uint32 topBinIndex = binIndex >> TOP_BINS_INDEX_SHIFT;
            uint32 leafBinIndex = binIndex & LEAF_BINS_INDEX_MASK;
            
            binIndices[binIndex] = node.binListNext;
            if (node.binListNext != Node::unused) nodes[node.binListNext].binListPrev = Node::unused;

            // Bin empty?
            if (binIndices[binIndex] == Node::unused)
            {
                // Remove a leaf bin mask bit
                usedBins[topBinIndex] &= ~(1 << leafBinIndex);
                
                // All leaf bins empty?
                if (usedBins[topBinIndex] == 0)
                {
                    // Remove a top bin mask bit
                    usedBinsTop &= ~(1 << topBinIndex);
                }
            }
        }
        
        // Insert the node to freelist
#ifdef DEBUG_ENABLED
        print_verbose(vformat("Putting node %u into freelist[%u] (removeNodeFromBin)\n", nodeIndex, freeOffset + 1));
#endif
        freeNodes[++freeOffset] = nodeIndex;

        freeStorage -= node.dataSize;
#ifdef DEBUG_ENABLED
        print_verbose(vformat("Free storage: %u (-%u) (removeNodeFromBin)\n", freeStorage, node.dataSize));
#endif
    }

    uint32 Allocator::allocationSize(Allocation allocation) const
    {
        if (allocation.metadata == Allocation::NO_SPACE) return 0;
        if (!nodes) return 0;
        
        return nodes[allocation.metadata].dataSize;
    }

    StorageReport Allocator::storageReport() const
    {
        uint32 largestFreeRegion = 0;
        uint32 freeStorage = 0;
        
        // Out of allocations? -> Zero free space
        if (freeOffset > 0)
        {
            freeStorage = freeStorage;
            if (usedBinsTop)
            {
                uint32 topBinIndex = 31 - lzcnt_nonzero(usedBinsTop);
                uint32 leafBinIndex = 31 - lzcnt_nonzero(usedBins[topBinIndex]);
                largestFreeRegion = SmallFloat::floatToUint((topBinIndex << TOP_BINS_INDEX_SHIFT) | leafBinIndex);
                DEV_ASSERT(freeStorage >= largestFreeRegion);
            }
        }

        return {.totalFreeSpace = freeStorage, .largestFreeRegion = largestFreeRegion};
    }

    StorageReportFull Allocator::storageReportFull() const
    {
        StorageReportFull report;
        for (uint32 i = 0; i < NUM_LEAF_BINS; i++)
        {
            uint32 count = 0;
            uint32 nodeIndex = binIndices[i];
            while (nodeIndex != Node::unused)
            {
                nodeIndex = nodes[nodeIndex].binListNext;
                count++;
            }
            report.freeRegions[i] = { .size = SmallFloat::floatToUint(i), .count = count };
        }
        return report;
    }
}
