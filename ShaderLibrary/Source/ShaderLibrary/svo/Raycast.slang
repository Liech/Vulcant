module Caster;

import Raycast.Ray;
import Raycast.VolumeAccumulator;
import Raycast.Intersect;
import SVO.SVO;
import SVO.SVONode;

namespace SVO
{

struct Todo {
    uint   currentNode;
    float3 nodePosition;
    float  nodeSize;
    uint   treeDepth;
};
 
public void raycast<DATA, ACC : Raycast.VolumeAccumulator<DATA>>(Raycast.Ray ray, SVO<DATA> svo, inout ACC accumulator)
{
    static const int MAX_TODO_SIZE = 64;
    static const int MAX_DEPTH = 15;
    accumulator.initialize();

    uint childSortMask = 0;
    if (ray.direction.x < 0) childSortMask |= 1;
    if (ray.direction.y < 0) childSortMask |= 2;
    if (ray.direction.z < 0) childSortMask |= 4;

    Todo todo[MAX_TODO_SIZE];

    todo[0].currentNode      = 0;
    todo[0].nodePosition     = float3(0, 0, 0);
    todo[0].nodeSize         = svo.dimension;
    todo[0].treeDepth        = 0;

    int currentTodo = 0;

    [loop]
    while (currentTodo >= 0)
    {
        float3 currentPos = todo[currentTodo].nodePosition;
        float currentSize = todo[currentTodo].nodeSize;
        Raycast.RayHit hit = Raycast.intersectAABB(ray, currentPos, currentPos + currentSize);

        SVONode<DATA> node = svo.nodes[todo[currentTodo].currentNode];

        if (node.isLeaf() || todo[currentTodo].treeDepth >= MAX_DEPTH)
        {
            if (todo[currentTodo].treeDepth >= MAX_DEPTH)
            {
                accumulator.error(Raycast.AccumulatorError.MaxDepthReached);
            }
            if (accumulator.accumulate(node.value, hit.tNear, hit.tFar))
            {
                break;
            }
            currentTodo--;
        }
        else
        {
            if (currentTodo + 8 >= MAX_TODO_SIZE)
            {
                accumulator.error(Raycast.AccumulatorError.StackOverflow);
                break;
            }

            float halfSize = currentSize * 0.5f;
            uint nextDepth = todo[currentTodo].treeDepth + 1;
            currentTodo--;

            for (uint i = 0; i < 8; i++)
            {
                uint childIndex = 7-(i ^ childSortMask);
                SVONode<DATA> sub = svo.nodes[node.childPointer + childIndex];
                float3 childPos = currentPos + float3((childIndex & 1) != 0 ? halfSize : 0, (childIndex & 2) != 0 ? halfSize : 0, (childIndex & 4) != 0 ? halfSize : 0);

                Raycast.RayHit hit = Raycast.intersectAABB(ray, childPos, childPos + halfSize);
                if (hit.hit)
                {
                    currentTodo++;
                    todo[currentTodo].currentNode = node.childPointer + childIndex;
                    todo[currentTodo].nodePosition = childPos;
                    todo[currentTodo].nodeSize = halfSize;
                    todo[currentTodo].treeDepth = nextDepth;
                }
            }
        }
    }
    accumulator.finalize();
}
}