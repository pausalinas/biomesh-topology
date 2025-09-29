#include <biomesh/voxel_grid/voxel_grid.h>
#include <cmath>
#include <algorithm>

namespace biomesh {

VoxelGrid::VoxelGrid(const BoundingBox& boundingBox, double voxelSize)
    : boundingBox_(boundingBox), voxelSize_(voxelSize) {
    
    if (!boundingBox_.isValid() || voxelSize <= 0.0) {
        nx_ = ny_ = nz_ = 0;
        return;
    }
    
    // Calculate grid dimensions
    nx_ = static_cast<int>(std::ceil(boundingBox_.getWidth() / voxelSize_));
    ny_ = static_cast<int>(std::ceil(boundingBox_.getHeight() / voxelSize_));
    nz_ = static_cast<int>(std::ceil(boundingBox_.getDepth() / voxelSize_));
    
    // Ensure minimum grid size
    nx_ = std::max(nx_, 1);
    ny_ = std::max(ny_, 1);
    nz_ = std::max(nz_, 1);
    
    initializeGrid();
}

VoxelGrid::VoxelGrid(const VoxelGrid& other)
    : boundingBox_(other.boundingBox_), voxelSize_(other.voxelSize_),
      nx_(other.nx_), ny_(other.ny_), nz_(other.nz_), grid_(other.grid_) {}

VoxelGrid& VoxelGrid::operator=(const VoxelGrid& other) {
    if (this != &other) {
        boundingBox_ = other.boundingBox_;
        voxelSize_ = other.voxelSize_;
        nx_ = other.nx_;
        ny_ = other.ny_;
        nz_ = other.nz_;
        grid_ = other.grid_;
    }
    return *this;
}

VoxelGrid::VoxelGrid(VoxelGrid&& other) noexcept
    : boundingBox_(std::move(other.boundingBox_)), voxelSize_(other.voxelSize_),
      nx_(other.nx_), ny_(other.ny_), nz_(other.nz_), grid_(std::move(other.grid_)) {
    other.nx_ = other.ny_ = other.nz_ = 0;
    other.voxelSize_ = 0.0;
}

VoxelGrid& VoxelGrid::operator=(VoxelGrid&& other) noexcept {
    if (this != &other) {
        boundingBox_ = std::move(other.boundingBox_);
        voxelSize_ = other.voxelSize_;
        nx_ = other.nx_;
        ny_ = other.ny_;
        nz_ = other.nz_;
        grid_ = std::move(other.grid_);
        
        other.nx_ = other.ny_ = other.nz_ = 0;
        other.voxelSize_ = 0.0;
    }
    return *this;
}

void VoxelGrid::voxelize(const std::vector<std::unique_ptr<Atom>>& atoms) {
    // Reset all voxels
    for (int i = 0; i < nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            for (int k = 0; k < nz_; ++k) {
                grid_[i][j][k].isOccupied = false;
                grid_[i][j][k].density = 0.0;
            }
        }
    }
    
    // Process each atom
    for (const auto& atom : atoms) {
        double atomX = atom->getX();
        double atomY = atom->getY();
        double atomZ = atom->getZ();
        double atomRadius = atom->getRadius();
        
        // Determine range of voxels that might be affected by this atom
        int minI, maxI, minJ, maxJ, minK, maxK;
        worldToGrid(atomX - atomRadius, atomY - atomRadius, atomZ - atomRadius, minI, minJ, minK);
        worldToGrid(atomX + atomRadius, atomY + atomRadius, atomZ + atomRadius, maxI, maxJ, maxK);
        
        // Check each voxel in the range
        for (int i = std::max(0, minI); i <= std::min(nx_ - 1, maxI); ++i) {
            for (int j = std::max(0, minJ); j <= std::min(ny_ - 1, maxJ); ++j) {
                for (int k = std::max(0, minK); k <= std::min(nz_ - 1, maxK); ++k) {
                    Voxel& voxel = grid_[i][j][k];
                    
                    // Calculate distance from voxel center to atom center
                    double dx = voxel.x - atomX;
                    double dy = voxel.y - atomY;
                    double dz = voxel.z - atomZ;
                    double distance = std::sqrt(dx * dx + dy * dy + dz * dz);
                    
                    // If voxel is within atom radius, mark as occupied
                    if (distance <= atomRadius) {
                        voxel.isOccupied = true;
                        // Calculate density based on distance (1.0 at center, 0.0 at radius)
                        voxel.density = std::max(voxel.density, 1.0 - (distance / atomRadius));
                    }
                }
            }
        }
    }
}

const Voxel& VoxelGrid::getVoxel(int i, int j, int k) const {
    if (!isValidIndex(i, j, k)) {
        static const Voxel invalidVoxel;
        return invalidVoxel;
    }
    return grid_[i][j][k];
}

Voxel& VoxelGrid::getVoxel(int i, int j, int k) {
    if (!isValidIndex(i, j, k)) {
        static Voxel invalidVoxel;
        return invalidVoxel;
    }
    return grid_[i][j][k];
}

std::vector<Voxel> VoxelGrid::getOccupiedVoxels() const {
    std::vector<Voxel> occupiedVoxels;
    
    for (int i = 0; i < nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            for (int k = 0; k < nz_; ++k) {
                if (grid_[i][j][k].isOccupied) {
                    occupiedVoxels.push_back(grid_[i][j][k]);
                }
            }
        }
    }
    
    return occupiedVoxels;
}

void VoxelGrid::getDimensions(int& nx, int& ny, int& nz) const {
    nx = nx_;
    ny = ny_;
    nz = nz_;
}

bool VoxelGrid::worldToGrid(double x, double y, double z, int& i, int& j, int& k) const {
    double relX = x - boundingBox_.getMinX();
    double relY = y - boundingBox_.getMinY();
    double relZ = z - boundingBox_.getMinZ();
    
    i = static_cast<int>(relX / voxelSize_);
    j = static_cast<int>(relY / voxelSize_);
    k = static_cast<int>(relZ / voxelSize_);
    
    return isValidIndex(i, j, k);
}

void VoxelGrid::gridToWorld(int i, int j, int k, double& x, double& y, double& z) const {
    x = boundingBox_.getMinX() + (i + 0.5) * voxelSize_;
    y = boundingBox_.getMinY() + (j + 0.5) * voxelSize_;
    z = boundingBox_.getMinZ() + (k + 0.5) * voxelSize_;
}

size_t VoxelGrid::getOccupiedVoxelCount() const {
    size_t count = 0;
    for (int i = 0; i < nx_; ++i) {
        for (int j = 0; j < ny_; ++j) {
            for (int k = 0; k < nz_; ++k) {
                if (grid_[i][j][k].isOccupied) {
                    count++;
                }
            }
        }
    }
    return count;
}

void VoxelGrid::initializeGrid() {
    grid_.resize(nx_);
    for (int i = 0; i < nx_; ++i) {
        grid_[i].resize(ny_);
        for (int j = 0; j < ny_; ++j) {
            grid_[i][j].resize(nz_);
            for (int k = 0; k < nz_; ++k) {
                grid_[i][j][k] = Voxel(i, j, k, 0.0, 0.0, 0.0);
                gridToWorld(i, j, k, grid_[i][j][k].x, grid_[i][j][k].y, grid_[i][j][k].z);
            }
        }
    }
}

bool VoxelGrid::isValidIndex(int i, int j, int k) const {
    return (i >= 0 && i < nx_ && j >= 0 && j < ny_ && k >= 0 && k < nz_);
}

size_t VoxelGrid::getLinearIndex(int i, int j, int k) const {
    return static_cast<size_t>(i) * ny_ * nz_ + static_cast<size_t>(j) * nz_ + static_cast<size_t>(k);
}

} // namespace biomesh