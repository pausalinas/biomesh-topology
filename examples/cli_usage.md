# BiMesh Topology CLI Usage Examples

This document provides examples of how to use the BiMesh Topology command-line interface.

## Installation

First, install the package:

```bash
pip install -e .
```

## Basic Usage

### Analyze a single mesh file

```bash
# Basic analysis with JSON output
biomesh analyze mesh.stl

# Save results to file
biomesh analyze mesh.stl --output results.json

# Verbose output
biomesh analyze mesh.stl --verbose

# CSV output format
biomesh analyze mesh.stl --format csv --output results.csv
```

### Batch processing

```bash
# Process all STL files in a directory
biomesh batch ./meshes --output-dir ./results

# Process with specific pattern
biomesh batch ./meshes --pattern "*cell*" --output-dir ./results

# CSV output format
biomesh batch ./meshes --output-dir ./results --format csv --verbose
```

### Visualization

```bash
# Basic visualization
biomesh visualize mesh.stl

# Color by curvature
biomesh visualize mesh.stl --property curvature

# Save visualization
biomesh visualize mesh.stl --property area --output mesh_areas.png

# No interactive display, just save
biomesh visualize mesh.stl --no-show --output mesh.png
```

### Compare meshes

```bash
# Compare two meshes
biomesh compare mesh1.stl mesh2.stl

# Compare specific metrics
biomesh compare mesh1.stl mesh2.stl --metrics area,volume,genus

# Save comparison results
biomesh compare mesh1.stl mesh2.stl --output comparison.json
```

### List supported formats

```bash
biomesh formats
```

## Advanced Examples

### Complete workflow for biological mesh analysis

```bash
# 1. Check supported formats
biomesh formats

# 2. Analyze a biological mesh with full options
biomesh analyze cell_membrane.ply \
  --output membrane_analysis.json \
  --format json \
  --verbose

# 3. Batch process multiple tissue samples
biomesh batch ./tissue_samples \
  --output-dir ./analysis_results \
  --pattern "*tissue*" \
  --format csv \
  --verbose

# 4. Visualize curvature distribution
biomesh visualize cell_membrane.ply \
  --property curvature \
  --output membrane_curvature.png

# 5. Compare before and after treatment
biomesh compare before_treatment.stl after_treatment.stl \
  --metrics area,volume,genus,euler \
  --output treatment_comparison.json
```

### Working with different file formats

```bash
# STL files
biomesh analyze scaffold.stl --output scaffold_analysis.json

# PLY files with color information
biomesh analyze colored_mesh.ply --verbose

# OBJ files
biomesh analyze model.obj --format csv --output model_stats.csv

# VTK files (if PyVista is installed)
biomesh analyze simulation_result.vtk --output sim_analysis.json
```

### Quality assessment workflow

```bash
# Analyze mesh quality
biomesh analyze mesh.stl --verbose > quality_report.txt

# The verbose output will include:
# - Degenerate face count
# - Aspect ratio statistics
# - Edge length statistics
# - Connectivity information
# - Mesh validation results
```

### Research workflow example

```bash
#!/bin/bash
# Research analysis pipeline

# Create results directory
mkdir -p ./research_results

# Process all experimental samples
for sample in ./samples/*.stl; do
    echo "Processing $sample..."
    biomesh analyze "$sample" \
      --output "./research_results/$(basename "$sample" .stl)_analysis.json" \
      --verbose
done

# Generate comparison between control and treatment groups
biomesh compare ./samples/control_group.stl ./samples/treatment_group.stl \
  --metrics area,volume,genus,euler \
  --output ./research_results/group_comparison.json

# Create visualizations
for sample in ./samples/*.stl; do
    biomesh visualize "$sample" \
      --property curvature \
      --no-show \
      --output "./research_results/$(basename "$sample" .stl)_curvature.png"
done

echo "Analysis complete! Results in ./research_results/"
```

## Output Formats

### JSON Output
Structured data suitable for further processing:
```json
{
  "metadata": {
    "analysis_timestamp": 1234567890,
    "mesh_vertices": 1000,
    "mesh_faces": 2000
  },
  "topology": {
    "euler_characteristic": 2,
    "genus": 0,
    "connected_components": 1
  },
  "geometry": {
    "surface_area": 15.234,
    "volume": 8.567
  }
}
```

### CSV Output
Tabular format suitable for spreadsheet analysis:
```csv
Property,Value
metadata.mesh_vertices,1000
metadata.mesh_faces,2000
topology.euler_characteristic,2
topology.genus,0
geometry.surface_area,15.234
```

## Error Handling

The CLI provides helpful error messages:

```bash
# File not found
biomesh analyze nonexistent.stl
# Error: File not found: nonexistent.stl

# Unsupported format
biomesh analyze file.xyz
# Error: Unsupported file format: .xyz

# Invalid options
biomesh compare mesh1.stl
# Error: Missing argument 'MESH2'
```

## Integration with Other Tools

### Python scripts
```python
import subprocess
import json

# Run analysis from Python
result = subprocess.run([
    'biomesh', 'analyze', 'mesh.stl', '--format', 'json'
], capture_output=True, text=True)

if result.returncode == 0:
    analysis = json.loads(result.stdout)
    print(f"Surface area: {analysis['geometry']['surface_area']}")
```

### Shell scripting
```bash
#!/bin/bash
# Automated analysis with error handling

MESH_FILE="$1"
OUTPUT_DIR="./results"

if [ ! -f "$MESH_FILE" ]; then
    echo "Error: Mesh file not found: $MESH_FILE"
    exit 1
fi

mkdir -p "$OUTPUT_DIR"

# Run analysis
if biomesh analyze "$MESH_FILE" --output "$OUTPUT_DIR/analysis.json" --verbose; then
    echo "Analysis completed successfully"
    biomesh visualize "$MESH_FILE" --no-show --output "$OUTPUT_DIR/visualization.png"
else
    echo "Analysis failed"
    exit 1
fi
```