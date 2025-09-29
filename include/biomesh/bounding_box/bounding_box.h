#pragma once

#include <vector>
#include <memory>
#include <biomesh/atom/atom.h>

namespace biomesh {

/**
 * @brief Axis-aligned bounding box for spatial calculations
 */
class BoundingBox {
public:
    /**
     * @brief Default constructor - creates invalid bounding box
     */
    BoundingBox();

    /**
     * @brief Constructor with explicit bounds
     * @param minX Minimum X coordinate
     * @param minY Minimum Y coordinate
     * @param minZ Minimum Z coordinate
     * @param maxX Maximum X coordinate
     * @param maxY Maximum Y coordinate
     * @param maxZ Maximum Z coordinate
     */
    BoundingBox(double minX, double minY, double minZ,
                double maxX, double maxY, double maxZ);

    /**
     * @brief Destructor
     */
    ~BoundingBox() = default;

    /**
     * @brief Copy constructor
     */
    BoundingBox(const BoundingBox& other) = default;

    /**
     * @brief Assignment operator
     */
    BoundingBox& operator=(const BoundingBox& other) = default;

    /**
     * @brief Calculate bounding box from atoms
     * @param atoms Vector of atoms
     * @param padding Additional padding around the bounding box
     * @return BoundingBox that encompasses all atoms
     */
    static BoundingBox fromAtoms(const std::vector<std::unique_ptr<Atom>>& atoms,
                                 double padding = 0.0);

    /**
     * @brief Calculate bounding box from atoms including their radii
     * @param atoms Vector of atoms
     * @param padding Additional padding around the bounding box
     * @return BoundingBox that encompasses all atoms with their radii
     */
    static BoundingBox fromAtomsWithRadii(const std::vector<std::unique_ptr<Atom>>& atoms,
                                          double padding = 0.0);

    // Getters
    double getMinX() const { return minX_; }
    double getMinY() const { return minY_; }
    double getMinZ() const { return minZ_; }
    double getMaxX() const { return maxX_; }
    double getMaxY() const { return maxY_; }
    double getMaxZ() const { return maxZ_; }

    /**
     * @brief Get dimensions of the bounding box
     */
    double getWidth() const { return maxX_ - minX_; }
    double getHeight() const { return maxY_ - minY_; }
    double getDepth() const { return maxZ_ - minZ_; }

    /**
     * @brief Get volume of the bounding box
     */
    double getVolume() const;

    /**
     * @brief Get center point of the bounding box
     */
    void getCenter(double& centerX, double& centerY, double& centerZ) const;

    /**
     * @brief Check if bounding box is valid
     */
    bool isValid() const;

    /**
     * @brief Check if point is inside bounding box
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @return True if point is inside
     */
    bool contains(double x, double y, double z) const;

    /**
     * @brief Expand bounding box by given amount
     * @param amount Amount to expand in all directions
     */
    void expand(double amount);

    /**
     * @brief Merge with another bounding box
     * @param other Other bounding box to merge with
     */
    void merge(const BoundingBox& other);

private:
    double minX_, minY_, minZ_;
    double maxX_, maxY_, maxZ_;
    bool valid_;

    /**
     * @brief Update validity flag
     */
    void updateValidity();
};

} // namespace biomesh