#include <biomesh/atom/atom.h>
#include <cmath>
#include <unordered_map>

namespace biomesh {

// Atom implementation
Atom::Atom() : serial_(0), name_(""), element_(""), x_(0.0), y_(0.0), z_(0.0), radius_(1.0) {}

Atom::Atom(int serial, const std::string& name, const std::string& element,
           double x, double y, double z, double radius)
    : serial_(serial), name_(name), element_(element), x_(x), y_(y), z_(z), radius_(radius) {}

Atom::Atom(const Atom& other)
    : serial_(other.serial_), name_(other.name_), element_(other.element_),
      x_(other.x_), y_(other.y_), z_(other.z_), radius_(other.radius_) {}

Atom& Atom::operator=(const Atom& other) {
    if (this != &other) {
        serial_ = other.serial_;
        name_ = other.name_;
        element_ = other.element_;
        x_ = other.x_;
        y_ = other.y_;
        z_ = other.z_;
        radius_ = other.radius_;
    }
    return *this;
}

Atom::Atom(Atom&& other) noexcept
    : serial_(other.serial_), name_(std::move(other.name_)), element_(std::move(other.element_)),
      x_(other.x_), y_(other.y_), z_(other.z_), radius_(other.radius_) {
    other.serial_ = 0;
    other.x_ = other.y_ = other.z_ = other.radius_ = 0.0;
}

Atom& Atom::operator=(Atom&& other) noexcept {
    if (this != &other) {
        serial_ = other.serial_;
        name_ = std::move(other.name_);
        element_ = std::move(other.element_);
        x_ = other.x_;
        y_ = other.y_;
        z_ = other.z_;
        radius_ = other.radius_;
        
        other.serial_ = 0;
        other.x_ = other.y_ = other.z_ = other.radius_ = 0.0;
    }
    return *this;
}

void Atom::setPosition(double x, double y, double z) {
    x_ = x;
    y_ = y;
    z_ = z;
}

double Atom::distanceTo(const Atom& other) const {
    double dx = x_ - other.x_;
    double dy = y_ - other.y_;
    double dz = z_ - other.z_;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// AtomBuilder implementation
AtomBuilder::AtomBuilder() : atom_(std::make_unique<Atom>()) {}

AtomBuilder& AtomBuilder::setSerial(int serial) {
    atom_->setSerial(serial);
    return *this;
}

AtomBuilder& AtomBuilder::setName(const std::string& name) {
    atom_->setName(name);
    return *this;
}

AtomBuilder& AtomBuilder::setElement(const std::string& element) {
    atom_->setElement(element);
    return *this;
}

AtomBuilder& AtomBuilder::setPosition(double x, double y, double z) {
    atom_->setPosition(x, y, z);
    return *this;
}

AtomBuilder& AtomBuilder::setRadius(double radius) {
    atom_->setRadius(radius);
    return *this;
}

AtomBuilder& AtomBuilder::enrichWithStandardRadius() {
    double standardRadius = getStandardRadius(atom_->getElement());
    atom_->setRadius(standardRadius);
    return *this;
}

std::unique_ptr<Atom> AtomBuilder::build() {
    return std::move(atom_);
}

double AtomBuilder::getStandardRadius(const std::string& element) const {
    // Van der Waals radii in Angstroms
    static const std::unordered_map<std::string, double> vdwRadii = {
        {"H", 1.20}, {"C", 1.70}, {"N", 1.55}, {"O", 1.52}, {"P", 1.80},
        {"S", 1.80}, {"F", 1.47}, {"Cl", 1.75}, {"Br", 1.85}, {"I", 1.98},
        {"Na", 2.27}, {"Mg", 1.73}, {"K", 2.75}, {"Ca", 2.31}, {"Fe", 2.04},
        {"Zn", 1.39}, {"Cu", 1.40}, {"Mn", 2.05}
    };
    
    auto it = vdwRadii.find(element);
    return (it != vdwRadii.end()) ? it->second : 1.70; // Default to carbon radius
}

} // namespace biomesh