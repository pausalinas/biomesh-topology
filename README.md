# BioMesh - Biomolecular Mesh Generation Tool

A modern C++ library for generating volumetric meshes of biomolecules from PDB files.

## Overview

BioMesh is a comprehensive toolkit for converting molecular structures (from PDB files) into high-quality volumetric meshes suitable for computational simulations. The library follows modern C++ practices and provides a modular architecture for processing biomolecular data.

## Features

- **PDB Parser**: Parse Protein Data Bank (PDB) files to extract atomic coordinates and properties
- **Atomic Representation**: Rich atom data structure with Van der Waals radii and builder pattern
- **Spatial Processing**: Bounding box calculation and spatial domain management
- **Voxelization**: Convert molecular structures into 3D voxel grids
- **Mesh Generation**: Create tetrahedral meshes from voxelized molecular data
- **Multiple Export Formats**: Support for GiD and VTK mesh formats
- **Modern C++**: Uses smart pointers, RAII, and follows SOLID principles

## Project Structure

```
biomesh-topology/
├── include/biomesh/          # Header files
│   ├── atom/                 # Atom representation and builder
│   ├── pdb_parser/           # PDB file parsing
│   ├── bounding_box/         # Spatial domain calculations
│   ├── voxel_grid/           # Voxelization algorithms
│   ├── mesh_generator/       # Mesh generation from voxels
│   └── exporter/             # Mesh export (GiD, VTK)
├── src/biomesh/              # Implementation files
├── examples/                 # Example applications
├── tests/                    # Unit tests (GoogleTest)
└── CMakeLists.txt           # Build configuration
```

## Dependencies

- **Required**: 
  - CMake 3.16 or higher
  - C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2019+)
  
- **Optional**:
  - GoogleTest (for unit tests)

## Building

### Quick Start

```bash
# Clone the repository
git clone https://github.com/pausalinas/biomesh-topology.git
cd biomesh-topology

# Create build directory
mkdir build && cd build

# Configure with CMake
cmake ..

# Build the project
make -j$(nproc)

# Run the example
./biomesh_app
```

### Build Options

```bash
# Debug build
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Release build (optimized)
cmake -DCMAKE_BUILD_TYPE=Release ..

# With tests (requires GoogleTest)
sudo apt-get install libgtest-dev  # Ubuntu/Debian
cmake ..
make
```

## Usage

### Basic Example

```cpp
#include <biomesh/pdb_parser/pdb_parser.h>
#include <biomesh/bounding_box/bounding_box.h>
#include <biomesh/voxel_grid/voxel_grid.h>
#include <biomesh/mesh_generator/mesh_generator.h>
#include <biomesh/exporter/vtk_exporter.h>

using namespace biomesh;

int main() {
    // Parse PDB file
    PDBParser parser;
    auto atoms = parser.parseFile("molecule.pdb");
    
    // Calculate bounding box
    BoundingBox bbox = BoundingBox::fromAtomsWithRadii(atoms, 2.0);
    
    // Create voxel grid
    VoxelGrid grid(bbox, 0.5); // 0.5 Angstrom voxels
    grid.voxelize(atoms);
    
    // Generate mesh
    MeshGenerator meshGen;
    auto mesh = meshGen.generateMesh(grid);
    
    // Export to VTK
    VTKExporter exporter;
    exporter.exportMesh(*mesh, "output.vtk");
    
    return 0;
}
```

### Command Line Usage

```bash
# Process a PDB file
./biomesh_app path/to/molecule.pdb

# Use built-in example molecule
./biomesh_app
```

## API Documentation

### Core Classes

#### `PDBParser`
Parses PDB files and extracts atomic data.

```cpp
PDBParser parser;
parser.setIncludeHetAtoms(true);
parser.setIncludeHydrogens(false);
auto atoms = parser.parseFile("protein.pdb");
```

#### `AtomBuilder`
Fluent interface for creating and enriching atom objects.

```cpp
auto atom = AtomBuilder()
    .setSerial(1)
    .setName("CA")
    .setElement("C")
    .setPosition(1.0, 2.0, 3.0)
    .enrichWithStandardRadius()
    .build();
```

#### `VoxelGrid`
Performs molecular voxelization.

```cpp
VoxelGrid grid(boundingBox, voxelSize);
grid.voxelize(atoms);
auto occupiedVoxels = grid.getOccupiedVoxels();
```

#### `MeshGenerator`
Generates tetrahedral meshes from voxel data.

```cpp
MeshGenerator generator;
generator.setMeshDensity(1.0);
generator.setQualityParameters(15.0, 160.0);
auto mesh = generator.generateMesh(voxelGrid);
```

## Output Formats

### GiD Format
- `.msh` - Main mesh file with coordinates and connectivity
- `.crd` - Coordinates file
- `.ele` - Elements file

### VTK Format
- `.vtk` - Unstructured grid format compatible with ParaView, VisIt

## Testing

Unit tests are available when GoogleTest is installed:

```bash
# Install GoogleTest (Ubuntu/Debian)
sudo apt-get install libgtest-dev

# Configure with tests
cmake ..
make

# Run tests
./biomesh_tests
```

## Performance Considerations

- **Voxel Size**: Smaller voxel sizes increase accuracy but require more memory and computation time
- **Mesh Density**: Higher density produces more elements but better quality meshes
- **Memory Usage**: Large molecules may require significant RAM for voxelization

## Examples and Use Cases

- **Molecular Dynamics**: Generate meshes for finite element simulations
- **Drug Design**: Create volumetric representations for binding site analysis
- **Protein Visualization**: Export meshes for advanced visualization tools
- **Biophysics Research**: Support computational studies of molecular systems

## Contributing

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Protein Data Bank (PDB) format specification
- VTK file format documentation
- Modern C++ best practices community