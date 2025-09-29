#!/usr/bin/env python3
"""
Basic usage example for BiMesh Topology.

This example demonstrates how to:
1. Create a simple mesh
2. Perform topology analysis
3. Generate reports
4. Visualize results
"""

import numpy as np
import biomesh


def create_sample_mesh():
    """Create a simple tetrahedron mesh for demonstration."""
    vertices = np.array([
        [0.0, 0.0, 0.0],    # vertex 0
        [1.0, 0.0, 0.0],    # vertex 1  
        [0.5, 1.0, 0.0],    # vertex 2
        [0.5, 0.5, 1.0]     # vertex 3
    ])
    
    faces = np.array([
        [0, 1, 2],  # bottom face
        [0, 1, 3],  # front face
        [0, 2, 3],  # left face
        [1, 2, 3]   # right face
    ])
    
    return biomesh.Mesh(vertices, faces, metadata={'name': 'sample_tetrahedron'})


def main():
    """Main demonstration function."""
    print("BiMesh Topology - Basic Usage Example")
    print("=" * 40)
    
    # 1. Create a sample mesh
    print("\n1. Creating sample tetrahedron mesh...")
    mesh = create_sample_mesh()
    print(f"   Created mesh: {mesh}")
    print(f"   Vertices: {mesh.n_vertices}, Faces: {mesh.n_faces}")
    
    # 2. Perform comprehensive analysis
    print("\n2. Performing topology analysis...")
    results = biomesh.analyze_topology(mesh)
    
    # 3. Display basic results
    print("\n3. Analysis Results:")
    print("   Topological Properties:")
    print(f"     Vertices: {results['topology']['vertices']}")
    print(f"     Faces: {results['topology']['faces']}")
    print(f"     Edges: {results['topology']['edges']}")
    print(f"     Euler Characteristic: {results['topology']['euler_characteristic']}")
    print(f"     Genus: {results['topology']['genus']}")
    print(f"     Is Closed: {results['topology']['is_closed']}")
    print(f"     Connected Components: {results['topology']['connected_components']}")
    
    print("\n   Geometric Properties:")
    print(f"     Surface Area: {results['geometry']['surface_area']:.6f}")
    print(f"     Volume: {results['geometry']['volume']:.6f}")
    print(f"     Surface/Volume Ratio: {results['geometry']['surface_area_to_volume_ratio']:.6f}")
    
    # 4. Generate different report formats
    print("\n4. Generating reports...")
    
    # JSON report
    json_report = biomesh.generate_report(results, 'json')
    print(f"   JSON report generated ({len(json_report)} characters)")
    
    # Save reports to files
    biomesh.save_report(json_report, 'tetrahedron_analysis.json')
    print("   Saved JSON report to 'tetrahedron_analysis.json'")
    
    # 5. Mesh validation
    print("\n5. Validating mesh quality...")
    validation_results = biomesh.validate_mesh(mesh, verbose=False)
    print(f"   Mesh is valid: {validation_results['is_valid']}")
    
    if validation_results['issues']:
        print("   Issues found:")
        for issue in validation_results['issues']:
            print(f"     - {issue}")
    
    if validation_results['warnings']:
        print("   Warnings:")
        for warning in validation_results['warnings']:
            print(f"     - {warning}")
    
    # 6. Advanced analysis
    print("\n6. Computing detailed geometric properties...")
    
    # Face areas
    face_areas = mesh.compute_face_areas()
    print(f"   Face areas: {face_areas}")
    print(f"   Mean face area: {np.mean(face_areas):.6f}")
    
    # Face normals
    normals = mesh.compute_face_normals()
    print(f"   Face normals shape: {normals.shape}")
    
    # 7. Demonstrate repair functionality
    print("\n7. Demonstrating mesh repair...")
    
    # Create a mesh with some issues for demonstration
    vertices_with_duplicates = np.vstack([mesh.vertices, mesh.vertices[0:1]])  # Add duplicate vertex
    faces_with_issues = mesh.faces.copy()
    
    problematic_mesh = biomesh.Mesh(vertices_with_duplicates, faces_with_issues, validate=False)
    print(f"   Created problematic mesh with {problematic_mesh.n_vertices} vertices")
    
    # Validate the problematic mesh
    validation = biomesh.validate_mesh(problematic_mesh, verbose=False)
    print(f"   Problematic mesh is valid: {validation['is_valid']}")
    
    # Repair the mesh
    repaired_mesh = biomesh.repair_mesh(problematic_mesh)
    print(f"   Repaired mesh has {repaired_mesh.n_vertices} vertices")
    
    print("\n8. Analysis complete!")
    print("   Check 'tetrahedron_analysis.json' for detailed results.")


if __name__ == "__main__":
    main()