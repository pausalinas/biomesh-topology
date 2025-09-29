#pragma once

#include <string>
#include <biomesh/mesh_generator/mesh_generator.h>

namespace biomesh {

/**
 * @brief Exporter for GiD format meshes
 */
class GiDExporter {
public:
    /**
     * @brief Constructor
     */
    GiDExporter();

    /**
     * @brief Destructor
     */
    ~GiDExporter() = default;

    /**
     * @brief Export mesh to GiD format
     * @param mesh Mesh to export
     * @param filename Output filename (without extension)
     * @return True if export successful
     */
    bool exportMesh(const Mesh& mesh, const std::string& filename);

    /**
     * @brief Set precision for coordinate output
     * @param precision Number of decimal places
     */
    void setPrecision(int precision) { precision_ = precision; }

    /**
     * @brief Set whether to include mesh statistics in output
     * @param include If true, include statistics as comments
     */
    void setIncludeStatistics(bool include) { includeStats_ = include; }

    /**
     * @brief Get last export error message
     */
    const std::string& getLastError() const { return lastError_; }

private:
    int precision_;
    bool includeStats_;
    std::string lastError_;

    /**
     * @brief Write mesh file (.msh)
     * @param mesh Mesh data
     * @param filename Base filename
     * @return True if successful
     */
    bool writeMeshFile(const Mesh& mesh, const std::string& filename);

    /**
     * @brief Write coordinates file (.crd)
     * @param mesh Mesh data
     * @param filename Base filename
     * @return True if successful
     */
    bool writeCoordinatesFile(const Mesh& mesh, const std::string& filename);

    /**
     * @brief Write elements file (.ele)
     * @param mesh Mesh data
     * @param filename Base filename
     * @return True if successful
     */
    bool writeElementsFile(const Mesh& mesh, const std::string& filename);

    /**
     * @brief Write GiD header information
     * @param file Output file stream
     * @param mesh Mesh data
     */
    void writeHeader(std::ofstream& file, const Mesh& mesh);

    /**
     * @brief Set error message
     * @param error Error message
     */
    void setError(const std::string& error);
};

} // namespace biomesh