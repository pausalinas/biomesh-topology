"""Mesh loading and saving functionality."""

import os
import numpy as np
from typing import Dict, Any, Optional
import warnings

try:
    import trimesh
    TRIMESH_AVAILABLE = True
except ImportError:
    TRIMESH_AVAILABLE = False
    warnings.warn("Trimesh not available. Some file formats may not be supported.")

try:
    import pyvista as pv
    PYVISTA_AVAILABLE = True
except ImportError:
    PYVISTA_AVAILABLE = False
    warnings.warn("PyVista not available. VTK formats may not be supported.")

from ..core.mesh import Mesh


def load_mesh(filepath: str, **kwargs) -> Mesh:
    """
    Load a mesh from file.
    
    Parameters
    ----------
    filepath : str
        Path to the mesh file
    **kwargs : dict
        Additional loading parameters
        
    Returns
    -------
    Mesh
        Loaded mesh object
        
    Raises
    ------
    FileNotFoundError
        If the file doesn't exist
    ValueError
        If the file format is not supported
    """
    if not os.path.exists(filepath):
        raise FileNotFoundError(f"File not found: {filepath}")
    
    ext = os.path.splitext(filepath)[1].lower()
    
    # Try to load using trimesh first (supports most formats)
    if TRIMESH_AVAILABLE and ext in ['.stl', '.ply', '.obj', '.off']:
        return _load_with_trimesh(filepath, **kwargs)
    
    # Try PyVista for VTK formats
    elif PYVISTA_AVAILABLE and ext in ['.vtk', '.vtp', '.vtu']:
        return _load_with_pyvista(filepath, **kwargs)
    
    # Custom loaders for specific formats
    elif ext == '.stl':
        return _load_stl(filepath, **kwargs)
    elif ext == '.ply':
        return _load_ply(filepath, **kwargs)
    elif ext == '.obj':
        return _load_obj(filepath, **kwargs)
    else:
        raise ValueError(f"Unsupported file format: {ext}")


def save_mesh(mesh: Mesh, filepath: str, **kwargs) -> None:
    """
    Save a mesh to file.
    
    Parameters
    ----------
    mesh : Mesh
        Mesh object to save
    filepath : str
        Output file path
    **kwargs : dict
        Additional saving parameters
    """
    ext = os.path.splitext(filepath)[1].lower()
    
    # Try to save using trimesh first
    if TRIMESH_AVAILABLE and ext in ['.stl', '.ply', '.obj', '.off']:
        _save_with_trimesh(mesh, filepath, **kwargs)
    
    # Try PyVista for VTK formats
    elif PYVISTA_AVAILABLE and ext in ['.vtk', '.vtp']:
        _save_with_pyvista(mesh, filepath, **kwargs)
    
    # Custom savers
    elif ext == '.stl':
        _save_stl(mesh, filepath, **kwargs)
    elif ext == '.ply':
        _save_ply(mesh, filepath, **kwargs)
    elif ext == '.obj':
        _save_obj(mesh, filepath, **kwargs)
    else:
        raise ValueError(f"Unsupported output format: {ext}")


def _load_with_trimesh(filepath: str, **kwargs) -> Mesh:
    """Load mesh using trimesh library."""
    tm_mesh = trimesh.load(filepath, **kwargs)
    
    # Handle case where trimesh returns a Scene instead of Mesh
    if hasattr(tm_mesh, 'geometry'):
        # It's a Scene, get the first geometry
        geometries = list(tm_mesh.geometry.values())
        if not geometries:
            raise ValueError("No geometry found in file")
        tm_mesh = geometries[0]
    
    # Extract vertices and faces
    vertices = np.array(tm_mesh.vertices)
    faces = np.array(tm_mesh.faces)
    
    # Extract metadata
    metadata = {
        'source_file': filepath,
        'loader': 'trimesh'
    }
    
    return Mesh(vertices, faces, metadata)


def _save_with_trimesh(mesh: Mesh, filepath: str, **kwargs) -> None:
    """Save mesh using trimesh library."""
    tm_mesh = trimesh.Trimesh(vertices=mesh.vertices, faces=mesh.faces)
    tm_mesh.export(filepath, **kwargs)


def _load_with_pyvista(filepath: str, **kwargs) -> Mesh:
    """Load mesh using PyVista library."""
    pv_mesh = pv.read(filepath, **kwargs)
    
    # Convert to triangular mesh if needed
    if hasattr(pv_mesh, 'triangulate'):
        pv_mesh = pv_mesh.triangulate()
    
    # Extract vertices and faces
    vertices = np.array(pv_mesh.points)
    
    # Extract faces (PyVista format: [n_points, p0, p1, p2, ...])
    faces_flat = pv_mesh.faces
    if len(faces_flat) > 0:
        faces = []
        i = 0
        while i < len(faces_flat):
            n_points = faces_flat[i]
            if n_points == 3:  # Triangle
                faces.append(faces_flat[i+1:i+4])
                i += 4
            else:
                # Skip non-triangular faces for now
                i += n_points + 1
        
        faces = np.array(faces)
    else:
        faces = np.array([]).reshape(0, 3)
    
    metadata = {
        'source_file': filepath,
        'loader': 'pyvista'
    }
    
    return Mesh(vertices, faces, metadata)


def _save_with_pyvista(mesh: Mesh, filepath: str, **kwargs) -> None:
    """Save mesh using PyVista library."""
    # Create PyVista mesh
    faces_flat = []
    for face in mesh.faces:
        faces_flat.extend([3] + list(face))
    
    pv_mesh = pv.PolyData(mesh.vertices, faces_flat)
    pv_mesh.save(filepath, **kwargs)


def _load_stl(filepath: str, **kwargs) -> Mesh:
    """Load STL file using custom parser."""
    # This is a simplified STL loader
    # In practice, you'd want a more robust implementation
    raise NotImplementedError("Custom STL loader not implemented. Install trimesh for STL support.")


def _save_stl(mesh: Mesh, filepath: str, **kwargs) -> None:
    """Save STL file using custom writer."""
    # ASCII STL format
    with open(filepath, 'w') as f:
        f.write(f"solid {kwargs.get('name', 'biomesh')}\n")
        
        for face in mesh.faces:
            v0, v1, v2 = mesh.vertices[face]
            
            # Compute normal
            normal = np.cross(v1 - v0, v2 - v0)
            normal = normal / np.linalg.norm(normal)
            
            f.write(f"  facet normal {normal[0]} {normal[1]} {normal[2]}\n")
            f.write("    outer loop\n")
            f.write(f"      vertex {v0[0]} {v0[1]} {v0[2]}\n")
            f.write(f"      vertex {v1[0]} {v1[1]} {v1[2]}\n")
            f.write(f"      vertex {v2[0]} {v2[1]} {v2[2]}\n")
            f.write("    endloop\n")
            f.write("  endfacet\n")
        
        f.write("endsolid\n")


def _load_ply(filepath: str, **kwargs) -> Mesh:
    """Load PLY file using custom parser."""
    raise NotImplementedError("Custom PLY loader not implemented. Install trimesh for PLY support.")


def _save_ply(mesh: Mesh, filepath: str, **kwargs) -> None:
    """Save PLY file using custom writer."""
    with open(filepath, 'w') as f:
        f.write("ply\n")
        f.write("format ascii 1.0\n")
        f.write(f"element vertex {mesh.n_vertices}\n")
        f.write("property float x\n")
        f.write("property float y\n")
        f.write("property float z\n")
        f.write(f"element face {mesh.n_faces}\n")
        f.write("property list uchar int vertex_indices\n")
        f.write("end_header\n")
        
        # Write vertices
        for vertex in mesh.vertices:
            f.write(f"{vertex[0]} {vertex[1]} {vertex[2]}\n")
        
        # Write faces
        for face in mesh.faces:
            f.write(f"3 {face[0]} {face[1]} {face[2]}\n")


def _load_obj(filepath: str, **kwargs) -> Mesh:
    """Load OBJ file using custom parser."""
    vertices = []
    faces = []
    
    with open(filepath, 'r') as f:
        for line in f:
            line = line.strip()
            if line.startswith('v '):
                # Vertex
                coords = list(map(float, line.split()[1:4]))
                vertices.append(coords)
            elif line.startswith('f '):
                # Face
                face_data = line.split()[1:]
                face_indices = []
                for vertex_data in face_data:
                    # Handle v/vt/vn format
                    vertex_index = int(vertex_data.split('/')[0]) - 1  # OBJ is 1-indexed
                    face_indices.append(vertex_index)
                
                if len(face_indices) >= 3:
                    faces.append(face_indices[:3])  # Take only first 3 vertices for triangles
    
    vertices = np.array(vertices)
    faces = np.array(faces)
    
    metadata = {
        'source_file': filepath,
        'loader': 'custom_obj'
    }
    
    return Mesh(vertices, faces, metadata)


def _save_obj(mesh: Mesh, filepath: str, **kwargs) -> None:
    """Save OBJ file using custom writer."""
    with open(filepath, 'w') as f:
        f.write("# OBJ file generated by BiMesh Topology\n")
        
        # Write vertices
        for vertex in mesh.vertices:
            f.write(f"v {vertex[0]} {vertex[1]} {vertex[2]}\n")
        
        # Write faces (OBJ is 1-indexed)
        for face in mesh.faces:
            f.write(f"f {face[0]+1} {face[1]+1} {face[2]+1}\n")


def supported_formats() -> Dict[str, str]:
    """
    Get supported file formats.
    
    Returns
    -------
    Dict[str, str]
        Dictionary mapping extensions to format descriptions
    """
    formats = {}
    
    if TRIMESH_AVAILABLE:
        formats.update({
            '.stl': 'STL (STereoLithography)',
            '.ply': 'PLY (Stanford Polygon Format)',
            '.obj': 'OBJ (Wavefront)',
            '.off': 'OFF (Object File Format)'
        })
    else:
        formats.update({
            '.obj': 'OBJ (Wavefront) - limited support',
            '.stl': 'STL - save only',
            '.ply': 'PLY - save only'
        })
    
    if PYVISTA_AVAILABLE:
        formats.update({
            '.vtk': 'VTK (Visualization Toolkit)',
            '.vtp': 'VTP (VTK PolyData)',
            '.vtu': 'VTU (VTK Unstructured Grid)'
        })
    
    return formats