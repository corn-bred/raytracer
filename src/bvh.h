#pragma once
#include <numeric>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>

const int LEAF_THRESHOLD = 8;

struct Vertex {
    glm::vec3 Position; // +12, 0 /= 16
    float padding0; // +4, 16
    glm::vec3 Normal; // +12, 16 /= 16
    float padding1; // +4, 32
    glm::vec2 TexCoords; // +8, 32 /= 16
    float padding2[2]; // +8, 48
}; // TOTAL: 48 /= 16

struct Triangle {
    Vertex v0; // +48, 0 /= 16
    Vertex v1; // +48, 48 /= 16
    Vertex v2; // +48, 96 /= 16
}; // TOTAL: 144 /= 16

struct Object {
    Triangle triangle;  // +144, 0 /= 16

    float roughness;    // +4 = 144 /= 4
    int roughnessTextureIdx; // +4 148 /= 4

    float padding0[2];  // +8, 152

    glm::vec3 albedo;   // +12, 160 /= 16
    int albedoTextureIdx; // +4, 172 /= 4

    int dielectric;     // +4, 176 /= 4

    float ior;          // +4, 180 /= 4

    int emissive;       // +4, 184 /= 4

    float padding1;     // +4, 192

    float padding2[4];  // +16, 208
};                      //TOTAL: 208 /= 16

struct BVHNode {
    glm::vec3 BBMin, BBMax;
    BVHNode *LeftChild, *RightChild;
    int Start, Count;
    bool isLeaf;

    BVHNode() : LeftChild(nullptr), RightChild(nullptr), Start(0), Count(0), isLeaf(false) {}
};

struct FlatNode {
    glm::vec3 BBMin; // +12,  0 /= 16
    float padding0; // +4
    int LeftChild_or_Count; // +4, 16 /= 16//this is really awesome compressed data waow
    float padding1[3]; // +12
    glm::vec3 BBMax; // +12, 32 /= 16
    float padding2; // +4, 
    int RightChild_or_Start; //if its a leaf it will show count and start if not its gonna show the indexes of left child and right child if you didnt know
    float padding3[3]; // +12
}; //TOTAL: 64 /= 16

class BVH {
    private:
    std::vector<Object> Triangles;
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
            const Triangle &tri = Triangles[TriID].triangle;

            outMin = glm::min(outMin, tri.v0.Position);
            outMin = glm::min(outMin, tri.v1.Position);
            outMin = glm::min(outMin, tri.v2.Position);

            outMax = glm::max(outMax, tri.v0.Position);
            outMax = glm::max(outMax, tri.v1.Position);
            outMax = glm::max(outMax, tri.v2.Position);
        }
        outMin -= glm::vec3(0.001);
        outMax += glm::vec3(0.001);
    }

    int ChooseSplitAxis(int start, int end) { //Chooses the axis to split the bounding box. Chooses via longest axis
        if (end - start <= 1) return -1; //if triangle count is 1 (or lower somehow)

        glm::vec3 BBMin = glm::vec3(std::numeric_limits<float>::max());
        glm::vec3 BBMax = glm::vec3(std::numeric_limits<float>::min());

        for (int i = start; i < end; i++) {
            int TriID = TriIndices[i];
            const Triangle &tri = Triangles[TriID].triangle;

            glm::vec3 Centroid = (tri.v0.Position + tri.v1.Position + tri.v2.Position) / 3.0f;

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
    //5. Find all centroids and sort them via the SAH
    //6. Split them in half and give them to left and right children
    //7. Recursively build the children
    BVHNode *RecursiveBuild(int start, int end) {
        BVHNode *node = new BVHNode();

        //Set basic start and end
        node->Start = start;
        node->Count = end - start;

        //Calculate Bounding Box for node

        ComputeAABB(start, end, node->BBMin, node->BBMax);

        //Leaf check

        if (node->Count <= LEAF_THRESHOLD) {
            node->isLeaf = true;
            return node;
        }

        //SAH time

        float BestCost = std::numeric_limits<float>::max();
        int axis = 0;
        int split = start;

        //Test each axis to find best split
        for (int iAxis = 0; iAxis < 3; iAxis++) {
            //First, sort for the axis (lowest value to highest value on axis)
            std::sort(TriIndices.begin() + start, TriIndices.begin() + end, 
                [&](int a, int b) {
                    const Triangle &triA = Triangles[a].triangle;
                    const Triangle &triB = Triangles[b].triangle;

                    glm::vec3 CentroidA = (triA.v0.Position + triA.v1.Position + triA.v2.Position) / 3.0f;
                    glm::vec3 CentroidB = (triB.v0.Position + triB.v1.Position + triB.v2.Position) / 3.0f;

                    return CentroidA[iAxis] < CentroidB[iAxis];
                }
            );
            
            int Count = end - start;

            //Prebuild AABBs for each triangle (Count tests in total)

            std::vector<glm::vec3> aMin(Count), aMax(Count), bMin(Count), bMax(Count);

            // Box A
            glm::vec3 minVal = glm::vec3(std::numeric_limits<float>::max());
            glm::vec3 maxVal = glm::vec3(std::numeric_limits<float>::lowest());
            for (int i = 0; i < Count; i++) {
                const Triangle &tri = Triangles[TriIndices[start + i]].triangle;

                minVal = glm::min(minVal, tri.v0.Position);
                minVal = glm::min(minVal, tri.v1.Position);
                minVal = glm::min(minVal, tri.v2.Position);

                maxVal = glm::max(maxVal, tri.v0.Position);
                maxVal = glm::max(maxVal, tri.v1.Position);
                maxVal = glm::max(maxVal, tri.v2.Position);

                aMin[i] = minVal;
                aMax[i] = maxVal;
            }

            // Box B
            minVal = glm::vec3(std::numeric_limits<float>::max());
            maxVal = glm::vec3(std::numeric_limits<float>::lowest());
            for (int i = Count - 1; i >= 0; i--) { //this time, backwards
                const Triangle &tri = Triangles[TriIndices[start + i]].triangle;

                minVal = glm::min(minVal, tri.v0.Position);
                minVal = glm::min(minVal, tri.v1.Position);
                minVal = glm::min(minVal, tri.v2.Position);

                maxVal = glm::max(maxVal, tri.v0.Position);
                maxVal = glm::max(maxVal, tri.v1.Position);
                maxVal = glm::max(maxVal, tri.v2.Position);

                bMin[i] = minVal;
                bMax[i] = maxVal;
            }

            // time to evaluate every single possible group of triangles each group can have :)
            for (int i = 0; i < Count - 1; i++) {
                glm::vec3 aSize = aMax[i] - aMin[i];
                glm::vec3 bSize = bMax[i + 1] - bMin[i + 1];

                float aArea = 2.0f * (aSize.x * aSize.y + aSize.y * aSize.z + aSize.z * aSize.x);
                float bArea = 2.0f * (bSize.x * bSize.y + bSize.y * bSize.z + bSize.z * bSize.x);

                float Cost = (i + 1) * aArea + (Count - i - 1) * bArea;

                if (Cost < BestCost) {
                    BestCost = Cost;
                    axis = iAxis;
                    split = start + i + 1;
                }
            }
        }

        //dont make 0 triangle nodes
        if (split == start || split == end) {
            node->isLeaf = true;
            return node;
        }

        std::nth_element(TriIndices.begin() + start, TriIndices.begin() + split, TriIndices.begin() + end, 
            [&](int a, int b) {
                const Triangle &triA = Triangles[a].triangle;
                const Triangle &triB = Triangles[b].triangle;
                glm::vec3 CentroidA = (triA.v0.Position + triA.v1.Position + triA.v2.Position) / 3.0f;
                glm::vec3 CentroidB = (triB.v0.Position + triB.v1.Position + triB.v2.Position) / 3.0f;
                return CentroidA[axis] < CentroidB[axis];
            }
        );

        //Recursively build the next children
        node->LeftChild = RecursiveBuild(start, split);
        node->RightChild = RecursiveBuild(split, end);
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

    BVH(const std::vector<Object> &triangles) : 
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

    std::vector<Object> &GetData() {
        return Triangles;
    }

    std::vector<FlatNode> &GetBVH() {
        return FlatNodes;
    }

    std::vector<int> &GetTriIndices() {
        return TriIndices;
    }
};

static_assert(sizeof(Vertex) == 48, "Vertex size mismatch!");
static_assert(sizeof(Triangle) == 144, "Triangle size mismatch!");
static_assert(sizeof(Object) == 208, "Object size mismatch!");
static_assert(sizeof(FlatNode) == 64, "FlatNode size mismatch!");