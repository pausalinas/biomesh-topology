#include <iostream>
#include <vector>
#include <memory>

// Include all BioMesh modules
#include <biomesh/pdb_parser/pdb_parser.h>
#include <biomesh/atom/atom.h>
#include <biomesh/bounding_box/bounding_box.h>
#include <biomesh/voxel_grid/voxel_grid.h>
#include <biomesh/mesh_generator/mesh_generator.h>
#include <biomesh/exporter/gid_exporter.h>
#include <biomesh/exporter/vtk_exporter.h>

using namespace biomesh;

/**
 * @brief Create a simple example molecule for testing
 */
std::vector<std::unique_ptr<Atom>> createExampleMolecule() {
    std::vector<std::unique_ptr<Atom>> atoms;
    
    // Create a simple water molecule
    AtomBuilder builder;
    
    // Oxygen atom
    auto oxygen = builder.setSerial(1)
                        .setName("O")
                        .setElement("O")
                        .setPosition(0.0, 0.0, 0.0)
                        .enrichWithStandardRadius()
                        .build();
    atoms.push_back(std::move(oxygen));
    
    // Hydrogen atom 1
    auto hydrogen1 = AtomBuilder()
                           .setSerial(2)
                           .setName("H1")
                           .setElement("H")
                           .setPosition(0.96, 0.0, 0.0)
                           .enrichWithStandardRadius()
                           .build();
    atoms.push_back(std::move(hydrogen1));
    
    // Hydrogen atom 2
    auto hydrogen2 = AtomBuilder()
                           .setSerial(3)
                           .setName("H2")
                           .setElement("H")
                           .setPosition(-0.24, 0.93, 0.0)
                           .enrichWithStandardRadius()
                           .build();
    atoms.push_back(std::move(hydrogen2));
    
    return atoms;
}

int main(int argc, char* argv[]) {
    std::cout << "BioMesh - Biomolecular Mesh Generation Tool\n";
    std::cout << "==========================================\n\n";
    
    try {
        std::vector<std::unique_ptr<Atom>> atoms;
        
        // Check if PDB file is provided as argument
        if (argc > 1) {
            std::cout << "Parsing PDB file: " << argv[1] << "\n";
            
            PDBParser parser;
            parser.setIncludeHetAtoms(false);
            parser.setIncludeHydrogens(true);
            
            atoms = parser.parseFile(argv[1]);
            
            const auto& stats = parser.getParseStats();
            std::cout << "Parsing complete:\n";
            std::cout << "  Total lines: " << stats.totalLines << "\n";
            std::cout << "  ATOM records: " << stats.atomRecords << "\n";
            std::cout << "  HETATM records: " << stats.hetatmRecords << "\n";
            std::cout << "  Atoms parsed: " << atoms.size() << "\n\n";
        } else {
            std::cout << "No PDB file provided. Using example water molecule.\n\n";
            atoms = createExampleMolecule();
            std::cout << "Created example molecule with " << atoms.size() << " atoms.\n\n";
        }
        
        if (atoms.empty()) {
            std::cerr << "No atoms to process. Exiting.\n";
            return 1;
        }
        
        // Calculate bounding box
        std::cout << "Calculating bounding box...\n";
        BoundingBox bbox = BoundingBox::fromAtomsWithRadii(atoms, 2.0);
        
        if (!bbox.isValid()) {
            std::cerr << "Invalid bounding box. Exiting.\n";
            return 1;
        }
        
        double centerX, centerY, centerZ;
        bbox.getCenter(centerX, centerY, centerZ);
        
        std::cout << "Bounding box:\n";
        std::cout << "  Min: (" << bbox.getMinX() << ", " << bbox.getMinY() << ", " << bbox.getMinZ() << ")\n";
        std::cout << "  Max: (" << bbox.getMaxX() << ", " << bbox.getMaxY() << ", " << bbox.getMaxZ() << ")\n";
        std::cout << "  Center: (" << centerX << ", " << centerY << ", " << centerZ << ")\n";
        std::cout << "  Dimensions: " << bbox.getWidth() << " x " << bbox.getHeight() << " x " << bbox.getDepth() << "\n";
        std::cout << "  Volume: " << bbox.getVolume() << "\n\n";
        
        // Create voxel grid
        std::cout << "Creating voxel grid...\n";
        double voxelSize = 0.5; // 0.5 Angstrom voxels
        VoxelGrid voxelGrid(bbox, voxelSize);
        
        int nx, ny, nz;
        voxelGrid.getDimensions(nx, ny, nz);
        std::cout << "Grid dimensions: " << nx << " x " << ny << " x " << nz << "\n";
        std::cout << "Total voxels: " << voxelGrid.getTotalVoxels() << "\n";
        
        // Perform voxelization
        std::cout << "Performing voxelization...\n";
        voxelGrid.voxelize(atoms);
        
        size_t occupiedVoxels = voxelGrid.getOccupiedVoxelCount();
        std::cout << "Occupied voxels: " << occupiedVoxels << "\n";
        std::cout << "Occupancy ratio: " << (100.0 * occupiedVoxels / voxelGrid.getTotalVoxels()) << "%\n\n";
        
        // Generate mesh
        std::cout << "Generating mesh...\n";
        MeshGenerator meshGen;
        meshGen.setMeshDensity(1.0);
        meshGen.setQualityParameters(15.0, 160.0);
        
        auto mesh = meshGen.generateMesh(voxelGrid);
        
        const auto& meshStats = meshGen.getMeshStats();
        std::cout << "Mesh generation complete:\n";
        std::cout << "  Input voxels: " << meshStats.inputVoxels << "\n";
        std::cout << "  Generated nodes: " << meshStats.generatedNodes << "\n";
        std::cout << "  Generated elements: " << meshStats.generatedElements << "\n";
        std::cout << "  Element quality (min/avg/max): " 
                  << meshStats.minElementQuality << "/" 
                  << meshStats.avgElementQuality << "/" 
                  << meshStats.maxElementQuality << "\n\n";
        
        // Export mesh
        std::cout << "Exporting mesh...\n";
        
        // Export to GiD format
        GiDExporter gidExporter;
        gidExporter.setPrecision(6);
        gidExporter.setIncludeStatistics(true);
        
        if (gidExporter.exportMesh(*mesh, "biomesh_output")) {
            std::cout << "  GiD export successful: biomesh_output.msh, .crd, .ele\n";
        } else {
            std::cerr << "  GiD export failed: " << gidExporter.getLastError() << "\n";
        }
        
        // Export to VTK format
        VTKExporter vtkExporter;
        vtkExporter.setFormat(VTKExporter::Format::ASCII);
        vtkExporter.setPrecision(6);
        vtkExporter.setTitle("BioMesh Generated Volumetric Mesh");
        
        if (vtkExporter.exportMesh(*mesh, "biomesh_output.vtk")) {
            std::cout << "  VTK export successful: biomesh_output.vtk\n";
        } else {
            std::cerr << "  VTK export failed: " << vtkExporter.getLastError() << "\n";
        }
        
        std::cout << "\nBioMesh processing complete!\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}