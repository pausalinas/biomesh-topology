#include <biomesh/mesh_generator/mesh_generator.h>
#include <algorithm>
#include <cmath>

namespace biomesh {

// Mesh implementation
Mesh::Mesh() {}

Mesh::Mesh(const Mesh& other) : nodes_(other.nodes_), elements_(other.elements_) {}

Mesh& Mesh::operator=(const Mesh& other) {
    if (this != &other) {
        nodes_ = other.nodes_;
        elements_ = other.elements_;
    }
    return *this;
}

Mesh::Mesh(Mesh&& other) noexcept 
    : nodes_(std::move(other.nodes_)), elements_(std::move(other.elements_)) {}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        nodes_ = std::move(other.nodes_);
        elements_ = std::move(other.elements_);
    }
    return *this;
}

void Mesh::addNode(const MeshNode& node) {
    nodes_.push_back(node);
}

void Mesh::addElement(const MeshElement& element) {
    elements_.push_back(element);
}

void Mesh::clear() {
    nodes_.clear();
    elements_.clear();
}

bool Mesh::isValid() const {
    // Check that all element nodes reference valid node IDs
    for (const auto& element : elements_) {
        for (int nodeId : element.nodeIds) {
            if (nodeId < 1 || nodeId > static_cast<int>(nodes_.size())) {
                return false;
            }
        }
    }
    return true;
}

// MeshGenerator implementation
MeshGenerator::MeshGenerator() 
    : minAngle_(10.0), maxAngle_(170.0), meshDensity_(1.0) {
    stats_ = MeshStats{};
}

std::unique_ptr<Mesh> MeshGenerator::generateMesh(const VoxelGrid& voxelGrid) {
    return generateTetrahedralMesh(voxelGrid);
}

std::unique_ptr<Mesh> MeshGenerator::generateTetrahedralMesh(const VoxelGrid& voxelGrid) {
    auto mesh = std::make_unique<Mesh>();
    stats_ = MeshStats{};
    
    auto occupiedVoxels = voxelGrid.getOccupiedVoxels();
    stats_.inputVoxels = occupiedVoxels.size();
    
    if (occupiedVoxels.empty()) {
        return mesh;
    }
    
    generateNodes(voxelGrid, *mesh);
    generateElements(voxelGrid, *mesh);
    updateStats(*mesh);
    
    return mesh;
}

void MeshGenerator::setQualityParameters(double minAngle, double maxAngle) {
    minAngle_ = std::max(0.0, std::min(minAngle, 60.0));
    maxAngle_ = std::max(120.0, std::min(maxAngle, 180.0));
}

void MeshGenerator::setMeshDensity(double density) {
    meshDensity_ = std::max(0.1, std::min(density, 2.0));
}

void MeshGenerator::generateNodes(const VoxelGrid& voxelGrid, Mesh& mesh) {
    auto occupiedVoxels = voxelGrid.getOccupiedVoxels();
    
    // Simple approach: create a node at each occupied voxel center
    int nodeId = 1;
    for (const auto& voxel : occupiedVoxels) {
        MeshNode node(nodeId++, voxel.x, voxel.y, voxel.z);
        mesh.addNode(node);
    }
    
    stats_.generatedNodes = mesh.getNodeCount();
}

void MeshGenerator::generateElements(const VoxelGrid& voxelGrid, Mesh& mesh) {
    const auto& nodes = mesh.getNodes();
    if (nodes.size() < 4) {
        return; // Need at least 4 nodes for tetrahedron
    }
    
    // Simple approach: Create tetrahedra from nearby nodes
    // This is a simplified algorithm - real implementation would use 
    // Delaunay triangulation or similar advanced techniques
    
    int elementId = 1;
    double voxelSize = voxelGrid.getVoxelSize();
    double searchRadius = voxelSize * std::sqrt(3.0); // Diagonal of voxel
    
    for (size_t i = 0; i < nodes.size(); ++i) {
        std::vector<int> nearbyNodes;
        
        // Find nearby nodes
        for (size_t j = 0; j < nodes.size(); ++j) {
            if (i == j) continue;
            
            double dx = nodes[i].x - nodes[j].x;
            double dy = nodes[i].y - nodes[j].y;
            double dz = nodes[i].z - nodes[j].z;
            double distance = std::sqrt(dx*dx + dy*dy + dz*dz);
            
            if (distance <= searchRadius) {
                nearbyNodes.push_back(nodes[j].id);
            }
        }
        
        // Create tetrahedra with nearby nodes (simplified)
        if (nearbyNodes.size() >= 3) {
            for (size_t j = 0; j < nearbyNodes.size() - 2; ++j) {
                for (size_t k = j + 1; k < nearbyNodes.size() - 1; ++k) {
                    for (size_t l = k + 1; l < nearbyNodes.size(); ++l) {
                        std::vector<int> elementNodes = {
                            nodes[i].id, nearbyNodes[j], nearbyNodes[k], nearbyNodes[l]
                        };
                        
                        MeshElement element(elementId++, elementNodes);
                        mesh.addElement(element);
                        
                        // Limit number of elements to prevent explosion
                        if (mesh.getElementCount() > stats_.inputVoxels * 6) {
                            stats_.generatedElements = mesh.getElementCount();
                            return;
                        }
                    }
                }
            }
        }
    }
    
    stats_.generatedElements = mesh.getElementCount();
}

double MeshGenerator::calculateElementQuality(const MeshElement& element, 
                                              const std::vector<MeshNode>& nodes) const {
    if (element.nodeIds.size() != 4) {
        return 0.0; // Only tetrahedral elements supported
    }
    
    // Find the nodes
    std::vector<const MeshNode*> elemNodes(4);
    for (size_t i = 0; i < 4; ++i) {
        auto it = std::find_if(nodes.begin(), nodes.end(),
                               [&](const MeshNode& n) { return n.id == element.nodeIds[i]; });
        if (it == nodes.end()) {
            return 0.0;
        }
        elemNodes[i] = &(*it);
    }
    
    // Simple quality measure: ratio of inscribed to circumscribed sphere radii
    // This is a simplified calculation - real implementation would be more sophisticated
    
    // Calculate edge lengths
    std::vector<double> edgeLengths;
    for (size_t i = 0; i < 4; ++i) {
        for (size_t j = i + 1; j < 4; ++j) {
            double dx = elemNodes[i]->x - elemNodes[j]->x;
            double dy = elemNodes[i]->y - elemNodes[j]->y;
            double dz = elemNodes[i]->z - elemNodes[j]->z;
            edgeLengths.push_back(std::sqrt(dx*dx + dy*dy + dz*dz));
        }
    }
    
    double minEdge = *std::min_element(edgeLengths.begin(), edgeLengths.end());
    double maxEdge = *std::max_element(edgeLengths.begin(), edgeLengths.end());
    
    if (maxEdge == 0.0) return 0.0;
    return minEdge / maxEdge; // Quality metric between 0 and 1
}

void MeshGenerator::updateStats(const Mesh& mesh) {
    const auto& nodes = mesh.getNodes();
    const auto& elements = mesh.getElements();
    
    if (elements.empty()) {
        stats_.minElementQuality = 0.0;
        stats_.avgElementQuality = 0.0;
        stats_.maxElementQuality = 0.0;
        return;
    }
    
    double sumQuality = 0.0;
    stats_.minElementQuality = 1.0;
    stats_.maxElementQuality = 0.0;
    
    for (const auto& element : elements) {
        double quality = calculateElementQuality(element, nodes);
        sumQuality += quality;
        stats_.minElementQuality = std::min(stats_.minElementQuality, quality);
        stats_.maxElementQuality = std::max(stats_.maxElementQuality, quality);
    }
    
    stats_.avgElementQuality = sumQuality / elements.size();
}

} // namespace biomesh