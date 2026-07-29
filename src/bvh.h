#pragma once
#include <numeric>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

const int LEAF_THRESHOLD = 4;

struct Triangle {
    glm::vec3 v0, v1, v2;
};

struct BVHNode {
    glm::vec3 BBMin, BBMax;
    BVHNode *LeftChild, *RightChild;
    int Start, Count;
    bool isLeaf;

    BVHNode() : LeftChild(nullptr), RightChild(nullptr), Start(0), Count(0), isLeaf(false) {}
};

struct FlatNode {
    glm::vec3 BBMin, BBMax;
    int LeftChild_or_Count; //this is really awesome compressed data waow
    int RightChild_or_Start; //if its a leaf it will show count and start if not its gonna show the indexes of left child and right child if you didnt know
};

class BVH {
    private:
    const std::vector<Triangle> &Triangles;
    std::vector<int> TriIndices;
    std::vector<FlatNode> FlatNodes;
    BVHNode *Root;

    
    void DeleteTree(BVHNode* node) {
        if (!node) return;
        DeleteTree(node->LeftChild);
        DeleteTree(node->RightChild);
        delete node;
    }

    void ComputeAABB(int start, int end, glm::vec3& outMin, glm::vec3& outMax) { // Encapsulates all triangles within a certain range in a bounding box
        outMin = glm::vec3(std::numeric_limits<float>::max()); //setting them to their limit so any float can beat them at default
        outMax = glm::vec3(std::numeric_limits<float>::lowest());

        for (int i = start; i < end; i++) {
            int TriID = TriIndices[i];
            const Triangle &tri = Triangles[TriID];

            outMin = std::min(outMin, tri.v0);
            outMin = std::min(outMin, tri.v1);
            outMin = std::min(outMin, tri.v2);

            outMax = std::max(outMax, tri.v0);
            outMax = std::max(outMax, tri.v1);
            outMax = std::max(outMax, tri.v2);
        }
    }

    int ChooseSplitAxis(int start, int end) { //Chooses the axis to split the bounding box. Chooses via longest axis
        if (end - start <= 1) return -1; //if triangle count is 1 (or lower somehow)

        glm::vec3 BBMin = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 BBMax = glm::vec3(std::numeric_limits<float>::min());

        for (int i = start; i < end; i++) {
            int TriID = TriIndices[i];
            const Triangle &tri = Triangles[TriID];

            glm::vec3 Centroid = (tri.v0 + tri.v1 + tri.v2) / 3.0f;

            BBMin = glm::min(BBMin, Centroid);

            BBMax = glm::max(BBMax, Centroid);
        }

        glm::vec3 Spread(BBMax - BBMin);

        if (Spread.x >= Spread.y && Spread.x >= Spread.z) return 0; // 0 = x
        if (Spread.y >= Spread.x && Spread.y >= Spread.z) return 1; // 1 = y
        if (Spread.z >= Spread.x && Spread.z >= Spread.y) return 2; // 2 = z

        return -1; // default
    }
    //1. Set node's start and end
    //2. Create bounding box for node
    //3. Check for leaf thresholds
    // ONWARD ARE NON-LEAF OPERATIONS
    //4. Choose axis to split
    //5. Find all centroids and sort them
    //6. Split them in half and give them to left and right children
    //7. Recursively build the children
    BVHNode *RecursiveBuild(int start, int end) {
        BVHNode *node = new BVHNode();

        //Set basic start and end
        node->Start = start;
        node->Count = start - end;

        //Calculate Bounding Box for node

        ComputeAABB(start, end, node->BBMin, node->BBMax);

        //Leaf check

        if (node->Count <= LEAF_THRESHOLD) {
            node->isLeaf = true;
            return node;
        }

        //Find axis to split

        int axis = ChooseSplitAxis(start, end);
        if (axis == -1) {
            node->isLeaf = true; // if axis is equal to -1 (error thrown) can't split (all centroids at same position)
            return node;
        }

        //Compare centroids on given axis

        //Find centroids on given axis by ChooseSplitAxis() on all triangles

        std::vector<float> Centroids; // float is the centroid's position on an axis given
        Centroids.reserve(end-start);

        for (int i = start ; i < end ; i++) {
            int TriIndex = TriIndices[i];
            const Triangle &tri = Triangles[TriIndex];

            glm::vec3 Centroid = glm::vec3(tri.v0 + tri.v1 + tri.v2) / 3.0f;

            Centroids.push_back(Centroid[axis]); //return only the value on the axis given
        }

        //Find the median centroid as a guideline for splitting the triangles to the children

        std::nth_element(Centroids.begin(), Centroids.begin() + Centroids.size() / 2, Centroids.end());
        float Median = Centroids[Centroids.size() / 2];

        //interator and sort specified part of index
        auto mid = std::partition(
            TriIndices.begin() + start,
            TriIndices.begin() + end,
            [&](int TriID) {
                const Triangle &tri = Triangles[TriID];
                glm::vec3 Centroid = (tri.v0 + tri.v1 + tri.v2) / 3.0f;
                return Centroid[axis] < Median;
            }
        );

        //middle index (convert interator to index)
        int midIndex = std::distance(TriIndices.begin(), mid);

        //Ensure that we don't give children no triangles (unless they are leaf nodes)
        if (midIndex == start || midIndex == end) {
            node->isLeaf = true;
            return node;
        }

        //Recursively build the next children
        node->LeftChild = RecursiveBuild(start, midIndex);
        node->LeftChild = RecursiveBuild(midIndex, end);
        node->isLeaf = false;

        return node;
    }

    int RecursiveFlatten(BVHNode *node) { //Just conversion
        int idx = FlatNodes.size();
        FlatNodes.push_back(FlatNode());

        FlatNodes[idx].BBMin = node->BBMin;
        FlatNodes[idx].BBMax = node->BBMax;

        if (node->isLeaf) {
            FlatNodes[idx].LeftChild_or_Count = -node->Count;
            FlatNodes[idx].RightChild_or_Start = node->Start;
        } else {
            int LeftIndex = RecursiveFlatten(node->LeftChild);
            int RightIndex = RecursiveFlatten(node->RightChild);

            FlatNodes[idx].LeftChild_or_Count = LeftIndex;
            FlatNodes[idx].RightChild_or_Start = RightIndex;
        }

        return idx;
    }


    public:

    BVH(const std::vector<Triangle> &triangles) : 
            Triangles(triangles), Root(nullptr) {
        TriIndices.resize(triangles.size());
        std::iota(TriIndices.begin(), TriIndices.end(), 0);
    }

    ~BVH() {
        DeleteTree(Root);
    }

    void Build() {
        if (Triangles.empty()) return;

        FlatNodes.reserve(2 * Triangles.size() - 1); //Maximum possible amount of nodes

        Root = RecursiveBuild(0, Triangles.size());
    }

    void Flatten() {
        if (!Root) return;

        FlatNodes.clear();
        FlatNodes.reserve(2 * Triangles.size() - 1); //Maximum possible amount of nodes

        RecursiveFlatten(Root);
    }


};