# BiMesh Topology

A computational biology software package for analyzing and visualizing the topological properties of biological mesh structures.

## Overview

BiMesh Topology provides robust tools for analyzing 3D biological mesh topologies, with a focus on cellular membranes, tissue scaffolds, and biomaterial networks. The package offers both programmatic APIs and command-line interfaces for comprehensive mesh analysis.

## Features

- **Comprehensive Mesh Analysis**: Compute topological properties (Euler characteristic, genus, connectivity)
- **Geometric Analysis**: Surface area, volume, curvature analysis, and shape descriptors
- **Biological Characterization**: Specialized metrics for biological structures
- **Multiple File Formats**: Support for STL, PLY, OBJ, VTK, and other common mesh formats
- **Visualization**: 3D interactive visualization with property mapping
- **Quality Assessment**: Mesh validation and repair tools
- **CLI and Python API**: Both command-line and programmatic interfaces
- **Report Generation**: Automated analysis reports in JSON, CSV, HTML, and Markdown formats

## Installation

### From Source

```bash
git clone https://github.com/pausalinas/biomesh-topology.git
cd biomesh-topology
pip install -e .
```

### Dependencies

Required dependencies:
- numpy >= 1.20.0
- scipy >= 1.7.0
- matplotlib >= 3.5.0

Optional dependencies for enhanced functionality:
- pyvista >= 0.37.0 (3D visualization and VTK support)
- trimesh >= 3.15.0 (additional mesh processing)
- pandas >= 1.3.0 (data analysis)

Install with all optional dependencies:
```bash
pip install -e ".[dev,docs,jupyter]"
```

## Quick Start

### Python API

```python
import biomesh

# Load and analyze a mesh
mesh = biomesh.load_mesh('sample.stl')
results = biomesh.analyze_topology(mesh)

# Print basic properties
print(f"Surface area: {results['geometry']['surface_area']:.3f}")
print(f"Volume: {results['geometry']['volume']:.3f}")
print(f"Genus: {results['topology']['genus']}")

# Generate and save report
report = biomesh.generate_report(results, format='json')
biomesh.save_report(report, 'analysis_report.json')

# Visualize with curvature coloring
biomesh.visualize_mesh(mesh, color_by='curvature')
```

### Command Line Interface

```bash
# Analyze a single mesh
biomesh analyze mesh.stl --output results.json --verbose

# Batch process multiple files
biomesh batch ./meshes --output-dir ./results --format csv

# Visualize mesh properties
biomesh visualize mesh.stl --property curvature --output curvature_plot.png

# Compare two meshes
biomesh compare mesh1.stl mesh2.stl --metrics area,volume,genus

# List supported formats
biomesh formats
```

## Documentation

- **[API Documentation](docs/)**: Complete API reference
- **[User Guide](docs/user_guide.md)**: Detailed usage examples
- **[CLI Usage](examples/cli_usage.md)**: Command-line interface examples
- **[Examples](examples/)**: Python examples and tutorials

## Supported File Formats

| Format | Extension | Description | Support |
|--------|-----------|-------------|---------|
| STL | .stl | STereoLithography | Full |
| PLY | .ply | Stanford Polygon Format | Full |
| OBJ | .obj | Wavefront OBJ | Full |
| VTK | .vtk | Visualization Toolkit | With PyVista |
| OFF | .off | Object File Format | With Trimesh |

## Analysis Capabilities

### Topological Analysis
- Euler characteristic computation
- Genus calculation
- Connected component analysis
- Boundary detection and hole identification
- Non-manifold edge detection

### Geometric Analysis
- Surface area and volume calculations
- Mean and Gaussian curvature analysis
- Shape descriptors (compactness, sphericity)
- Bounding box and principal axes
- Face and edge statistics

### Biological Characterization
- Mesh quality assessment
- Surface roughness quantification
- Connectivity analysis
- Porosity measurements (for applicable structures)

### Visualization
- Interactive 3D mesh rendering
- Property-based color mapping
- Cross-sectional views
- Publication-ready figure generation

## Development

### Setting up development environment

```bash
git clone https://github.com/pausalinas/biomesh-topology.git
cd biomesh-topology
pip install -e ".[dev]"
pre-commit install
```

### Running tests

```bash
pytest tests/
```

### Code formatting

```bash
black src/ tests/
isort src/ tests/
```

### Type checking

```bash
mypy src/
```

## Contributing

We welcome contributions! Please see [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

1. Fork the repository
2. Create a feature branch
3. Make your changes with tests
4. Run the test suite
5. Submit a pull request

## License

This project is licensed under the GNU General Public License v3.0 - see the [LICENSE](LICENSE) file for details.

## Citation

If you use BiMesh Topology in your research, please cite:

```bibtex
@software{biomesh_topology,
  title={BiMesh Topology: Computational Biology Software for Mesh Analysis},
  author={BiMesh Topology Contributors},
  url={https://github.com/pausalinas/biomesh-topology},
  year={2024}
}
```

## Support

- **Documentation**: [Read the Docs](https://biomesh-topology.readthedocs.io)
- **Issues**: [GitHub Issues](https://github.com/pausalinas/biomesh-topology/issues)
- **Discussions**: [GitHub Discussions](https://github.com/pausalinas/biomesh-topology/discussions)

## Acknowledgments

This project builds upon the excellent work of:
- [PyVista](https://pyvista.org/) for 3D visualization
- [Trimesh](https://trimsh.org/) for mesh processing
- [NumPy](https://numpy.org/) and [SciPy](https://scipy.org/) for numerical computing