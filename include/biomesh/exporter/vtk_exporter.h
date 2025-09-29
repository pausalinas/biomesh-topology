#pragma once

#include <string>
#include <biomesh/mesh_generator/mesh_generator.h>

namespace biomesh {

/**
 * @brief Exporter for VTK format meshes
 */
class VTKExporter {
public:
    /**
     * @brief VTK output format types
     */
    enum class Format {
        ASCII,
        BINARY
    };

    /**
     * @brief Constructor
     */
    VTKExporter();

    /**
     * @brief Destructor
     */
    ~VTKExporter() = default;

    /**
     * @brief Export mesh to VTK format
     * @param mesh Mesh to export
     * @param filename Output filename (with .vtk extension)
     * @return True if export successful
     */
    bool exportMesh(const Mesh& mesh, const std::string& filename);

    /**
     * @brief Set output format
     * @param format ASCII or BINARY format
     */
    void setFormat(Format format) { format_ = format; }

    /**
     * @brief Set precision for coordinate output (ASCII format only)
     * @param precision Number of decimal places
     */
    void setPrecision(int precision) { precision_ = precision; }

    /**
     * @brief Set dataset title
     * @param title Title for the VTK dataset
     */
    void setTitle(const std::string& title) { title_ = title; }

    /**
     * @brief Add scalar data to nodes
     * @param name Name of the scalar field
     * @param data Scalar values for each node
     */
    void addNodeScalarData(const std::string& name, const std::vector<double>& data);

    /**
     * @brief Add scalar data to elements
     * @param name Name of the scalar field
     * @param data Scalar values for each element
     */
    void addElementScalarData(const std::string& name, const std::vector<double>& data);

    /**
     * @brief Clear all additional data
     */
    void clearData();

    /**
     * @brief Get last export error message
     */
    const std::string& getLastError() const { return lastError_; }

private:
    Format format_;
    int precision_;
    std::string title_;
    std::string lastError_;

    // Additional data storage
    struct ScalarData {
        std::string name;
        std::vector<double> values;
    };
    std::vector<ScalarData> nodeScalarData_;
    std::vector<ScalarData> elementScalarData_;

    /**
     * @brief Write VTK header
     * @param file Output file stream
     */
    void writeHeader(std::ofstream& file);

    /**
     * @brief Write points section
     * @param file Output file stream
     * @param mesh Mesh data
     */
    void writePoints(std::ofstream& file, const Mesh& mesh);

    /**
     * @brief Write cells section
     * @param file Output file stream
     * @param mesh Mesh data
     */
    void writeCells(std::ofstream& file, const Mesh& mesh);

    /**
     * @brief Write cell types section
     * @param file Output file stream
     * @param mesh Mesh data
     */
    void writeCellTypes(std::ofstream& file, const Mesh& mesh);

    /**
     * @brief Write point data section
     * @param file Output file stream
     * @param mesh Mesh data
     */
    void writePointData(std::ofstream& file, const Mesh& mesh);

    /**
     * @brief Write cell data section
     * @param file Output file stream
     * @param mesh Mesh data
     */
    void writeCellData(std::ofstream& file, const Mesh& mesh);

    /**
     * @brief Get VTK cell type for element
     * @param element Mesh element
     * @return VTK cell type ID
     */
    int getVTKCellType(const MeshElement& element) const;

    /**
     * @brief Set error message
     * @param error Error message
     */
    void setError(const std::string& error);
};

} // namespace biomesh