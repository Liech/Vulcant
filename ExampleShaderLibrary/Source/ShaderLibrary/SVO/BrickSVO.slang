module SVO;

import BrickSVONode;

namespace SVO
{
public struct BrickSVO<T>
{
    public uint dimension;
    public StructuredBuffer<BrickSVONode<T>> nodes;

    public static BrickSVO<T> create(uint dimension, StructuredBuffer<BrickSVONode<T>> nodes) {
        BrickSVO<T> tree;
        tree.dimension = dimension;
        tree.nodes = nodes;
        return tree;
    }
};
}