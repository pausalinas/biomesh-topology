#pragma once

#include <string>
#include <memory>

namespace biomesh {

/**
 * @brief Represents an atom with its properties and coordinates
 */
class Atom {
public:
    /**
     * @brief Default constructor
     */
    Atom();

    /**
     * @brief Parameterized constructor
     * @param serial Serial number of the atom
     * @param name Atom name
     * @param element Element symbol
     * @param x X coordinate
     * @param y Y coordinate
     * @param z Z coordinate
     * @param radius Van der Waals radius
     */
    Atom(int serial, const std::string& name, const std::string& element,
         double x, double y, double z, double radius = 1.0);

    /**
     * @brief Copy constructor
     */
    Atom(const Atom& other);

    /**
     * @brief Assignment operator
     */
    Atom& operator=(const Atom& other);

    /**
     * @brief Move constructor
     */
    Atom(Atom&& other) noexcept;

    /**
     * @brief Move assignment operator
     */
    Atom& operator=(Atom&& other) noexcept;

    /**
     * @brief Destructor
     */
    ~Atom() = default;

    // Getters
    int getSerial() const { return serial_; }
    const std::string& getName() const { return name_; }
    const std::string& getElement() const { return element_; }
    double getX() const { return x_; }
    double getY() const { return y_; }
    double getZ() const { return z_; }
    double getRadius() const { return radius_; }

    // Setters
    void setSerial(int serial) { serial_ = serial; }
    void setName(const std::string& name) { name_ = name; }
    void setElement(const std::string& element) { element_ = element; }
    void setPosition(double x, double y, double z);
    void setRadius(double radius) { radius_ = radius; }

    /**
     * @brief Calculate distance to another atom
     * @param other The other atom
     * @return Distance between atoms
     */
    double distanceTo(const Atom& other) const;

private:
    int serial_;
    std::string name_;
    std::string element_;
    double x_, y_, z_;
    double radius_;
};

/**
 * @brief Builder class for creating and enriching Atom objects
 */
class AtomBuilder {
public:
    AtomBuilder();
    ~AtomBuilder() = default;

    AtomBuilder& setSerial(int serial);
    AtomBuilder& setName(const std::string& name);
    AtomBuilder& setElement(const std::string& element);
    AtomBuilder& setPosition(double x, double y, double z);
    AtomBuilder& setRadius(double radius);
    AtomBuilder& enrichWithStandardRadius();

    std::unique_ptr<Atom> build();

private:
    std::unique_ptr<Atom> atom_;
    
    /**
     * @brief Get standard Van der Waals radius for element
     */
    double getStandardRadius(const std::string& element) const;
};

} // namespace biomesh