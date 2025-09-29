"""Tests for core mesh functionality."""

import numpy as np
import pytest
from biomesh.core.mesh import Mesh
from biomesh.core.analyzer import TopologyAnalyzer


class TestMesh:
    """Test the Mesh class."""
    
    def test_mesh_creation(self):
        """Test basic mesh creation."""
        # Create a simple triangle
        vertices = np.array([
            [0.0, 0.0, 0.0],
            [1.0, 0.0, 0.0],
            [0.0, 1.0, 0.0]
        ])
        faces = np.array([[0, 1, 2]])
        
        mesh = Mesh(vertices, faces)
        
        assert mesh.n_vertices == 3
        assert mesh.n_faces == 1
        assert np.allclose(mesh.vertices, vertices)
        assert np.array_equal(mesh.faces, faces)
    
    def test_mesh_validation(self):
        """Test mesh validation."""
        vertices = np.array([
            [0.0, 0.0, 0.0],
            [1.0, 0.0, 0.0],
            [0.0, 1.0, 0.0]
        ])
        
        # Valid mesh
        faces = np.array([[0, 1, 2]])
        mesh = Mesh(vertices, faces)
        assert mesh.n_vertices == 3
        
        # Invalid faces (out of range indices)
        with pytest.raises(ValueError):
            invalid_faces = np.array([[0, 1, 3]])  # Index 3 doesn't exist
            Mesh(vertices, invalid_faces)
    
    def test_surface_area_calculation(self):
        """Test surface area calculation."""
        # Create a triangle with known area
        vertices = np.array([
            [0.0, 0.0, 0.0],
            [1.0, 0.0, 0.0],
            [0.0, 1.0, 0.0]
        ])
        faces = np.array([[0, 1, 2]])
        
        mesh = Mesh(vertices, faces)
        area = mesh.compute_surface_area()
        
        # Area of triangle with vertices at (0,0), (1,0), (0,1) is 0.5
        assert abs(area - 0.5) < 1e-10
    
    def test_euler_characteristic(self):
        """Test Euler characteristic calculation."""
        # Create a tetrahedron (4 vertices, 4 faces, 6 edges)
        vertices = np.array([
            [0.0, 0.0, 0.0],
            [1.0, 0.0, 0.0],
            [0.0, 1.0, 0.0],
            [0.0, 0.0, 1.0]
        ])
        faces = np.array([
            [0, 1, 2],
            [0, 1, 3],
            [0, 2, 3],
            [1, 2, 3]
        ])
        
        mesh = Mesh(vertices, faces)
        euler_char = mesh.compute_euler_characteristic()
        
        # For a tetrahedron: V - E + F = 4 - 6 + 4 = 2
        assert euler_char == 2
    
    def test_connected_components(self):
        """Test connected components detection."""
        # Create two separate triangles
        vertices = np.array([
            [0.0, 0.0, 0.0],  # Triangle 1
            [1.0, 0.0, 0.0],
            [0.0, 1.0, 0.0],
            [2.0, 0.0, 0.0],  # Triangle 2 (separate)
            [3.0, 0.0, 0.0],
            [2.0, 1.0, 0.0]
        ])
        faces = np.array([
            [0, 1, 2],  # Triangle 1
            [3, 4, 5]   # Triangle 2
        ])
        
        mesh = Mesh(vertices, faces)
        components = mesh.get_connected_components()
        
        assert len(components) == 2
        assert len(components[0]) == 3  # First triangle has 3 vertices
        assert len(components[1]) == 3  # Second triangle has 3 vertices


class TestTopologyAnalyzer:
    """Test the TopologyAnalyzer class."""
    
    def test_basic_analysis(self):
        """Test basic topology analysis."""
        # Create a simple mesh
        vertices = np.array([
            [0.0, 0.0, 0.0],
            [1.0, 0.0, 0.0],
            [0.0, 1.0, 0.0],
            [0.0, 0.0, 1.0]
        ])
        faces = np.array([
            [0, 1, 2],
            [0, 1, 3],
            [0, 2, 3],
            [1, 2, 3]
        ])
        
        mesh = Mesh(vertices, faces)
        analyzer = TopologyAnalyzer()
        results = analyzer.analyze(mesh)
        
        # Check that all main sections are present
        assert 'metadata' in results
        assert 'topology' in results
        assert 'geometry' in results
        assert 'biological' in results
        assert 'quality' in results
        
        # Check basic topology
        assert results['topology']['vertices'] == 4
        assert results['topology']['faces'] == 4
        assert results['topology']['euler_characteristic'] == 2
        assert results['topology']['connected_components'] == 1


def test_mesh_copy():
    """Test mesh copying."""
    vertices = np.array([
        [0.0, 0.0, 0.0],
        [1.0, 0.0, 0.0],
        [0.0, 1.0, 0.0]
    ])
    faces = np.array([[0, 1, 2]])
    metadata = {'test': 'value'}
    
    original_mesh = Mesh(vertices, faces, metadata)
    copied_mesh = original_mesh.copy()
    
    # Modify original
    original_mesh.vertices[0, 0] = 99.0
    original_mesh.metadata['test'] = 'modified'
    
    # Check that copy is unaffected
    assert copied_mesh.vertices[0, 0] == 0.0
    assert copied_mesh.metadata['test'] == 'value'


if __name__ == "__main__":
    pytest.main([__file__])