module SVONode;

namespace SVO
{
public struct BrickSVONode<T>
{
    public bool isLeaf()
    {
        return this.childPointer == 0xFFFFFFFF;
    }

    // 0xFFFFFFFF / uint(-1) means no child
    public uint childPointer = 0xFFFFFFFF;
    // 0xFFFFFFFF / uint(-1) means no brick, use value
    public uint brickPointer = 0xFFFFFFFF;
    public T value;
};
}