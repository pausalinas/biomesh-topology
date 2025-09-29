#include <biomesh/exporter/vtk_exporter.h>
#include <fstream>
#include <iomanip>
#include <sstream>

namespace biomesh {

VTKExporter::VTKExporter() 
    : format_(Format::ASCII), precision_(6), title_("BioMesh Output") {}

bool VTKExporter::exportMesh(const Mesh& mesh, const std::string& filename) {
    lastError_.clear();
    
    if (!mesh.isValid()) {
        setError("Invalid mesh data");
        return false;
    }
    
    if (mesh.getNodeCount() == 0) {
        setError("Mesh has no nodes");
        return false;
    }
    
    std::ofstream file(filename);
    if (!file.is_open()) {
        setError("Cannot create VTK file: " + filename);
        return false;
    }
    
    writeHeader(file);
    writePoints(file, mesh);
    writeCells(file, mesh);
    writeCellTypes(file, mesh);
    
    // Write additional data if available
    if (!nodeScalarData_.empty()) {
        writePointData(file, mesh);
    }
    
    if (!elementScalarData_.empty()) {
        writeCellData(file, mesh);
    }
    
    file.close();
    return true;
}

void VTKExporter::addNodeScalarData(const std::string& name, const std::vector<double>& data) {
    ScalarData scalarData;
    scalarData.name = name;
    scalarData.values = data;
    nodeScalarData_.push_back(scalarData);
}

void VTKExporter::addElementScalarData(const std::string& name, const std::vector<double>& data) {
    ScalarData scalarData;
    scalarData.name = name;
    scalarData.values = data;
    elementScalarData_.push_back(scalarData);
}

void VTKExporter::clearData() {
    nodeScalarData_.clear();
    elementScalarData_.clear();
}

void VTKExporter::writeHeader(std::ofstream& file) {
    file << "# vtk DataFile Version 3.0\n";
    file << title_ << "\n";
    
    if (format_ == Format::ASCII) {
        file << "ASCII\n";
    } else {
        file << "BINARY\n";
    }
    
    file << "DATASET UNSTRUCTURED_GRID\n";
}

void VTKExporter::writePoints(std::ofstream& file, const Mesh& mesh) {
    const auto& nodes = mesh.getNodes();
    file << "POINTS " << nodes.size() << " float\n";
    
    if (format_ == Format::ASCII) {
        for (const auto& node : nodes) {
            file << std::fixed << std::setprecision(precision_) 
                 << node.x << " " << node.y << " " << node.z << "\n";
        }
    } else {
        // Binary format implementation would go here
        setError("Binary format not yet implemented");
    }
}

void VTKExporter::writeCells(std::ofstream& file, const Mesh& mesh) {
    const auto& elements = mesh.getElements();
    
    // Calculate total size needed
    size_t totalSize = 0;
    for (const auto& element : elements) {
        totalSize += 1 + element.nodeIds.size(); // 1 for count + node IDs
    }
    
    file << "CELLS " << elements.size() << " " << totalSize << "\n";
    
    if (format_ == Format::ASCII) {
        for (const auto& element : elements) {
            file << element.nodeIds.size();
            for (int nodeId : element.nodeIds) {
                file << " " << (nodeId - 1); // VTK uses 0-based indexing
            }
            file << "\n";
        }
    }
}

void VTKExporter::writeCellTypes(std::ofstream& file, const Mesh& mesh) {
    const auto& elements = mesh.getElements();
    file << "CELL_TYPES " << elements.size() << "\n";
    
    if (format_ == Format::ASCII) {
        for (const auto& element : elements) {
            int cellType = getVTKCellType(element);
            file << cellType << "\n";
        }
    }
}

void VTKExporter::writePointData(std::ofstream& file, const Mesh& mesh) {
    if (nodeScalarData_.empty()) return;
    
    file << "POINT_DATA " << mesh.getNodeCount() << "\n";
    
    for (const auto& data : nodeScalarData_) {
        file << "SCALARS " << data.name << " float 1\n";
        file << "LOOKUP_TABLE default\n";
        
        if (format_ == Format::ASCII) {
            for (size_t i = 0; i < std::min(data.values.size(), mesh.getNodeCount()); ++i) {
                file << std::fixed << std::setprecision(precision_) 
                     << data.values[i] << "\n";
            }
        }
    }
}

void VTKExporter::writeCellData(std::ofstream& file, const Mesh& mesh) {
    if (elementScalarData_.empty()) return;
    
    file << "CELL_DATA " << mesh.getElementCount() << "\n";
    
    for (const auto& data : elementScalarData_) {
        file << "SCALARS " << data.name << " float 1\n";
        file << "LOOKUP_TABLE default\n";
        
        if (format_ == Format::ASCII) {
            for (size_t i = 0; i < std::min(data.values.size(), mesh.getElementCount()); ++i) {
                file << std::fixed << std::setprecision(precision_) 
                     << data.values[i] << "\n";
            }
        }
    }
}

int VTKExporter::getVTKCellType(const MeshElement& element) const {
    switch (element.nodeIds.size()) {
        case 4:  return 10; // VTK_TETRA
        case 8:  return 12; // VTK_HEXAHEDRON
        case 6:  return 13; // VTK_WEDGE
        case 5:  return 14; // VTK_PYRAMID
        default: return 1;  // VTK_VERTEX (fallback)
    }
}

void VTKExporter::setError(const std::string& error) {
    lastError_ = error;
}

} // namespace biomesh