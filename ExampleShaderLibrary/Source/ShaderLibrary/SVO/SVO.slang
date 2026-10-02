module SVO;

import SVONode;

namespace SVO
{
public struct SVO<T>
{
    public uint dimension;
    public StructuredBuffer<SVONode<T>> nodes;

    public static SVO<T> create(uint dimension, StructuredBuffer<SVONode<T>> nodes) {
        SVO<T> tree;
        tree.dimension = dimension;
        tree.nodes = nodes;
        return tree;
    }
};
}