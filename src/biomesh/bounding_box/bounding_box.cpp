#include <biomesh/bounding_box/bounding_box.h>
#include <limits>
#include <algorithm>

namespace biomesh {

BoundingBox::BoundingBox()
    : minX_(std::numeric_limits<double>::max()),
      minY_(std::numeric_limits<double>::max()),
      minZ_(std::numeric_limits<double>::max()),
      maxX_(std::numeric_limits<double>::lowest()),
      maxY_(std::numeric_limits<double>::lowest()),
      maxZ_(std::numeric_limits<double>::lowest()),
      valid_(false) {}

BoundingBox::BoundingBox(double minX, double minY, double minZ,
                         double maxX, double maxY, double maxZ)
    : minX_(minX), minY_(minY), minZ_(minZ),
      maxX_(maxX), maxY_(maxY), maxZ_(maxZ) {
    updateValidity();
}

BoundingBox BoundingBox::fromAtoms(const std::vector<std::unique_ptr<Atom>>& atoms,
                                   double padding) {
    if (atoms.empty()) {
        return BoundingBox();
    }
    
    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double minZ = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();
    double maxZ = std::numeric_limits<double>::lowest();
    
    for (const auto& atom : atoms) {
        minX = std::min(minX, atom->getX());
        minY = std::min(minY, atom->getY());
        minZ = std::min(minZ, atom->getZ());
        maxX = std::max(maxX, atom->getX());
        maxY = std::max(maxY, atom->getY());
        maxZ = std::max(maxZ, atom->getZ());
    }
    
    return BoundingBox(minX - padding, minY - padding, minZ - padding,
                       maxX + padding, maxY + padding, maxZ + padding);
}

BoundingBox BoundingBox::fromAtomsWithRadii(const std::vector<std::unique_ptr<Atom>>& atoms,
                                             double padding) {
    if (atoms.empty()) {
        return BoundingBox();
    }
    
    double minX = std::numeric_limits<double>::max();
    double minY = std::numeric_limits<double>::max();
    double minZ = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double maxY = std::numeric_limits<double>::lowest();
    double maxZ = std::numeric_limits<double>::lowest();
    
    for (const auto& atom : atoms) {
        double radius = atom->getRadius();
        minX = std::min(minX, atom->getX() - radius);
        minY = std::min(minY, atom->getY() - radius);
        minZ = std::min(minZ, atom->getZ() - radius);
        maxX = std::max(maxX, atom->getX() + radius);
        maxY = std::max(maxY, atom->getY() + radius);
        maxZ = std::max(maxZ, atom->getZ() + radius);
    }
    
    return BoundingBox(minX - padding, minY - padding, minZ - padding,
                       maxX + padding, maxY + padding, maxZ + padding);
}

double BoundingBox::getVolume() const {
    if (!valid_) {
        return 0.0;
    }
    return getWidth() * getHeight() * getDepth();
}

void BoundingBox::getCenter(double& centerX, double& centerY, double& centerZ) const {
    centerX = (minX_ + maxX_) * 0.5;
    centerY = (minY_ + maxY_) * 0.5;
    centerZ = (minZ_ + maxZ_) * 0.5;
}

bool BoundingBox::isValid() const {
    return valid_;
}

bool BoundingBox::contains(double x, double y, double z) const {
    if (!valid_) {
        return false;
    }
    return (x >= minX_ && x <= maxX_ &&
            y >= minY_ && y <= maxY_ &&
            z >= minZ_ && z <= maxZ_);
}

void BoundingBox::expand(double amount) {
    if (!valid_) {
        return;
    }
    minX_ -= amount;
    minY_ -= amount;
    minZ_ -= amount;
    maxX_ += amount;
    maxY_ += amount;
    maxZ_ += amount;
}

void BoundingBox::merge(const BoundingBox& other) {
    if (!other.valid_) {
        return;
    }
    
    if (!valid_) {
        *this = other;
        return;
    }
    
    minX_ = std::min(minX_, other.minX_);
    minY_ = std::min(minY_, other.minY_);
    minZ_ = std::min(minZ_, other.minZ_);
    maxX_ = std::max(maxX_, other.maxX_);
    maxY_ = std::max(maxY_, other.maxY_);
    maxZ_ = std::max(maxZ_, other.maxZ_);
}

void BoundingBox::updateValidity() {
    valid_ = (minX_ <= maxX_ && minY_ <= maxY_ && minZ_ <= maxZ_);
}

} // namespace biomesh