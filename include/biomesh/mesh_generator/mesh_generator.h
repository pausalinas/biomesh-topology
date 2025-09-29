#pragma once

#include <vector>
#include <memory>
#include <biomesh/voxel_grid/voxel_grid.h>

namespace biomesh {

/**
 * @brief Represents a mesh node
 */
struct MeshNode {
    int id;
    double x, y, z;

    MeshNode() : id(0), x(0.0), y(0.0), z(0.0) {}
    MeshNode(int id_, double x_, double y_, double z_) 
        : id(id_), x(x_), y(y_), z(z_) {}
};

/**
 * @brief Represents a mesh element (tetrahedron)
 */
struct MeshElement {
    int id;
    std::vector<int> nodeIds;  // Node IDs that form this element

    MeshElement() : id(0) {}
    MeshElement(int id_, const std::vector<int>& nodes) 
        : id(id_), nodeIds(nodes) {}
};

/**
 * @brief Complete mesh representation
 */
class Mesh {
public:
    /**
     * @brief Default constructor
     */
    Mesh();

    /**
     * @brief Destructor
     */
    ~Mesh() = default;

    /**
     * @brief Copy constructor
     */
    Mesh(const Mesh& other);

    /**
     * @brief Assignment operator
     */
    Mesh& operator=(const Mesh& other);

    /**
     * @brief Move constructor
     */
    Mesh(Mesh&& other) noexcept;

    /**
     * @brief Move assignment operator
     */
    Mesh& operator=(Mesh&& other) noexcept;

    /**
     * @brief Add a node to the mesh
     * @param node Node to add
     */
    void addNode(const MeshNode& node);

    /**
     * @brief Add an element to the mesh
     * @param element Element to add
     */
    void addElement(const MeshElement& element);

    /**
     * @brief Get all nodes
     */
    const std::vector<MeshNode>& getNodes() const { return nodes_; }

    /**
     * @brief Get all elements
     */
    const std::vector<MeshElement>& getElements() const { return elements_; }

    /**
     * @brief Get node count
     */
    size_t getNodeCount() const { return nodes_.size(); }

    /**
     * @brief Get element count
     */
    size_t getElementCount() const { return elements_.size(); }

    /**
     * @brief Clear all mesh data
     */
    void clear();

    /**
     * @brief Validate mesh integrity
     * @return True if mesh is valid
     */
    bool isValid() const;

private:
    std::vector<MeshNode> nodes_;
    std::vector<MeshElement> elements_;
};

/**
 * @brief Generates meshes from voxel grids
 */
class MeshGenerator {
public:
    /**
     * @brief Constructor
     */
    MeshGenerator();

    /**
     * @brief Destructor
     */
    ~MeshGenerator() = default;

    /**
     * @brief Generate mesh from voxel grid
     * @param voxelGrid Input voxel grid
     * @return Generated mesh
     */
    std::unique_ptr<Mesh> generateMesh(const VoxelGrid& voxelGrid);

    /**
     * @brief Generate tetrahedral mesh from voxels
     * @param voxelGrid Input voxel grid
     * @return Generated tetrahedral mesh
     */
    std::unique_ptr<Mesh> generateTetrahedralMesh(const VoxelGrid& voxelGrid);

    /**
     * @brief Set mesh quality parameters
     * @param minAngle Minimum dihedral angle (degrees)
     * @param maxAngle Maximum dihedral angle (degrees)
     */
    void setQualityParameters(double minAngle, double maxAngle);

    /**
     * @brief Set mesh density parameter
     * @param density Relative mesh density (0.1 to 2.0)
     */
    void setMeshDensity(double density);

    /**
     * @brief Get mesh generation statistics
     */
    struct MeshStats {
        size_t inputVoxels = 0;
        size_t generatedNodes = 0;
        size_t generatedElements = 0;
        double minElementQuality = 0.0;
        double avgElementQuality = 0.0;
        double maxElementQuality = 0.0;
    };

    const MeshStats& getMeshStats() const { return stats_; }

private:
    double minAngle_;
    double maxAngle_;
    double meshDensity_;
    MeshStats stats_;

    /**
     * @brief Generate nodes from occupied voxels
     * @param voxelGrid Input voxel grid
     * @param mesh Output mesh
     */
    void generateNodes(const VoxelGrid& voxelGrid, Mesh& mesh);

    /**
     * @brief Generate elements from nodes
     * @param voxelGrid Input voxel grid
     * @param mesh Output mesh
     */
    void generateElements(const VoxelGrid& voxelGrid, Mesh& mesh);

    /**
     * @brief Calculate element quality
     * @param element Element to evaluate
     * @param nodes Mesh nodes
     * @return Quality measure (0.0 to 1.0)
     */
    double calculateElementQuality(const MeshElement& element, 
                                   const std::vector<MeshNode>& nodes) const;

    /**
     * @brief Update mesh statistics
     * @param mesh Generated mesh
     */
    void updateStats(const Mesh& mesh);
};

} // namespace biomesh