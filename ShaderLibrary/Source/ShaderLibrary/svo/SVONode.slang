module SVONode;

namespace SVO
{
public struct SVONode<T>
{
    public bool isLeaf()
    {
        return this.childPointer == 0xFFFFFFFF;
    }

    // 0xFFFFFFFF / uint(-1) means no child
    public uint childPointer = 0xFFFFFFFF;
    public T value;
};
}