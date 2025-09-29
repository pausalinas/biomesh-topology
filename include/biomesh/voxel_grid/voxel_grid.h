#pragma once

#include <vector>
#include <memory>
#include <biomesh/atom/atom.h>
#include <biomesh/bounding_box/bounding_box.h>

namespace biomesh {

/**
 * @brief Represents a single voxel in the grid
 */
struct Voxel {
    int i, j, k;        // Grid indices
    double x, y, z;     // World coordinates
    bool isOccupied;    // Whether voxel contains molecular material
    double density;     // Density value (0.0 to 1.0)

    Voxel() : i(0), j(0), k(0), x(0.0), y(0.0), z(0.0), isOccupied(false), density(0.0) {}
    Voxel(int i_, int j_, int k_, double x_, double y_, double z_) 
        : i(i_), j(j_), k(k_), x(x_), y(y_), z(z_), isOccupied(false), density(0.0) {}
};

/**
 * @brief 3D voxel grid for molecular voxelization
 */
class VoxelGrid {
public:
    /**
     * @brief Constructor
     * @param boundingBox Spatial domain for voxelization
     * @param voxelSize Size of each voxel
     */
    VoxelGrid(const BoundingBox& boundingBox, double voxelSize);

    /**
     * @brief Destructor
     */
    ~VoxelGrid() = default;

    /**
     * @brief Copy constructor
     */
    VoxelGrid(const VoxelGrid& other);

    /**
     * @brief Assignment operator
     */
    VoxelGrid& operator=(const VoxelGrid& other);

    /**
     * @brief Move constructor
     */
    VoxelGrid(VoxelGrid&& other) noexcept;

    /**
     * @brief Move assignment operator
     */
    VoxelGrid& operator=(VoxelGrid&& other) noexcept;

    /**
     * @brief Perform voxelization of atoms
     * @param atoms Vector of atoms to voxelize
     */
    void voxelize(const std::vector<std::unique_ptr<Atom>>& atoms);

    /**
     * @brief Get voxel at grid indices
     * @param i Grid index in X direction
     * @param j Grid index in Y direction
     * @param k Grid index in Z direction
     * @return Reference to voxel
     */
    const Voxel& getVoxel(int i, int j, int k) const;

    /**
     * @brief Get voxel at grid indices (non-const)
     * @param i Grid index in X direction
     * @param j Grid index in Y direction
     * @param k Grid index in Z direction
     * @return Reference to voxel
     */
    Voxel& getVoxel(int i, int j, int k);

    /**
     * @brief Get all occupied voxels
     * @return Vector of occupied voxels
     */
    std::vector<Voxel> getOccupiedVoxels() const;

    /**
     * @brief Get grid dimensions
     */
    void getDimensions(int& nx, int& ny, int& nz) const;

    /**
     * @brief Get voxel size
     */
    double getVoxelSize() const { return voxelSize_; }

    /**
     * @brief Get bounding box
     */
    const BoundingBox& getBoundingBox() const { return boundingBox_; }

    /**
     * @brief Convert world coordinates to grid indices
     * @param x World X coordinate
     * @param y World Y coordinate
     * @param z World Z coordinate
     * @param i Output grid index X
     * @param j Output grid index Y
     * @param k Output grid index Z
     * @return True if coordinates are within grid bounds
     */
    bool worldToGrid(double x, double y, double z, int& i, int& j, int& k) const;

    /**
     * @brief Convert grid indices to world coordinates
     * @param i Grid index X
     * @param j Grid index Y
     * @param k Grid index Z
     * @param x Output world X coordinate
     * @param y Output world Y coordinate
     * @param z Output world Z coordinate
     */
    void gridToWorld(int i, int j, int k, double& x, double& y, double& z) const;

    /**
     * @brief Get total number of voxels
     */
    size_t getTotalVoxels() const { return nx_ * ny_ * nz_; }

    /**
     * @brief Get number of occupied voxels
     */
    size_t getOccupiedVoxelCount() const;

private:
    BoundingBox boundingBox_;
    double voxelSize_;
    int nx_, ny_, nz_;
    std::vector<std::vector<std::vector<Voxel>>> grid_;

    /**
     * @brief Initialize grid structure
     */
    void initializeGrid();

    /**
     * @brief Check if grid indices are valid
     */
    bool isValidIndex(int i, int j, int k) const;

    /**
     * @brief Calculate linear index from 3D indices
     */
    size_t getLinearIndex(int i, int j, int k) const;
};

} // namespace biomesh