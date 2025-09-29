"""Topological analysis functions."""

import numpy as np
from typing import Dict, Any, List, Tuple
from ..core.mesh import Mesh


def compute_euler_characteristic(mesh: Mesh) -> int:
    """
    Compute Euler characteristic of the mesh.
    
    Parameters
    ----------
    mesh : Mesh
        Input mesh
        
    Returns
    -------
    int
        Euler characteristic (V - E + F)
    """
    return mesh.compute_euler_characteristic()


def analyze_connectivity(mesh: Mesh) -> Dict[str, Any]:
    """
    Analyze mesh connectivity properties.
    
    Parameters
    ----------
    mesh : Mesh
        Input mesh
        
    Returns
    -------
    Dict[str, Any]
        Connectivity analysis results
    """
    # Get connected components
    components = mesh.get_connected_components()
    
    # Build vertex adjacency graph
    adjacency = [set() for _ in range(mesh.n_vertices)]
    for face in mesh.faces:
        for i in range(3):
            v1, v2 = face[i], face[(i + 1) % 3]
            adjacency[v1].add(v2)
            adjacency[v2].add(v1)
    
    # Vertex degree statistics
    degrees = [len(neighbors) for neighbors in adjacency]
    
    # Edge connectivity analysis
    edges = set()
    for face in mesh.faces:
        for i in range(3):
            edge = tuple(sorted([face[i], face[(i + 1) % 3]]))
            edges.add(edge)
    
    boundary_edges = mesh.get_boundary_edges()
    
    return {
        'connected_components': len(components),
        'component_sizes': [len(comp) for comp in components],
        'largest_component_size': max(len(comp) for comp in components) if components else 0,
        'vertex_degree_stats': {
            'mean': np.mean(degrees),
            'std': np.std(degrees),
            'min': np.min(degrees),
            'max': np.max(degrees),
            'median': np.median(degrees)
        },
        'total_edges': len(edges),
        'boundary_edges': len(boundary_edges),
        'is_manifold': len(boundary_edges) == 0 or _is_manifold(mesh),
        'euler_characteristic': mesh.compute_euler_characteristic()
    }


def _is_manifold(mesh: Mesh) -> bool:
    """
    Check if mesh is a manifold (each edge is shared by at most 2 faces).
    
    Parameters
    ----------
    mesh : Mesh
        Input mesh
        
    Returns
    -------
    bool
        True if mesh is manifold
    """
    edge_count = {}
    for face in mesh.faces:
        for i in range(3):
            edge = tuple(sorted([face[i], face[(i + 1) % 3]]))
            edge_count[edge] = edge_count.get(edge, 0) + 1
            if edge_count[edge] > 2:
                return False
    return True


def detect_holes(mesh: Mesh) -> List[List[int]]:
    """
    Detect holes in the mesh by finding boundary loops.
    
    Parameters
    ----------
    mesh : Mesh
        Input mesh
        
    Returns
    -------
    List[List[int]]
        List of boundary loops (each loop is a list of vertex indices)
    """
    boundary_edges = mesh.get_boundary_edges()
    if not boundary_edges:
        return []
    
    # Build boundary edge adjacency
    boundary_adjacency = {}
    for v1, v2 in boundary_edges:
        if v1 not in boundary_adjacency:
            boundary_adjacency[v1] = []
        if v2 not in boundary_adjacency:
            boundary_adjacency[v2] = []
        boundary_adjacency[v1].append(v2)
        boundary_adjacency[v2].append(v1)
    
    # Find boundary loops
    visited_edges = set()
    loops = []
    
    for start_vertex in boundary_adjacency:
        if any((start_vertex, neighbor) in visited_edges or (neighbor, start_vertex) in visited_edges 
               for neighbor in boundary_adjacency[start_vertex]):
            continue
        
        # Trace boundary loop starting from this vertex
        loop = []
        current = start_vertex
        prev = None
        
        while True:
            loop.append(current)
            
            # Find next vertex in loop
            neighbors = [v for v in boundary_adjacency[current] if v != prev]
            if not neighbors:
                break
            
            next_vertex = neighbors[0]  # Take any unvisited neighbor
            
            # Mark edge as visited
            visited_edges.add((current, next_vertex))
            visited_edges.add((next_vertex, current))
            
            if next_vertex == start_vertex:
                break  # Completed loop
            
            prev = current
            current = next_vertex
        
        if len(loop) > 2:  # Valid loop
            loops.append(loop)
    
    return loops


def compute_genus_from_euler(euler_characteristic: int, num_components: int, num_boundaries: int) -> int:
    """
    Compute genus from Euler characteristic using the generalized formula.
    
    For a surface with b boundary components and c connected components:
    genus = (2c - χ - b) / 2
    
    Parameters
    ----------
    euler_characteristic : int
        Euler characteristic of the mesh
    num_components : int
        Number of connected components
    num_boundaries : int
        Number of boundary loops
        
    Returns
    -------
    int
        Genus of the surface
    """
    return (2 * num_components - euler_characteristic - num_boundaries) // 2


def analyze_mesh_topology(mesh: Mesh) -> Dict[str, Any]:
    """
    Comprehensive topological analysis of a mesh.
    
    Parameters
    ----------
    mesh : Mesh
        Input mesh
        
    Returns
    -------
    Dict[str, Any]
        Complete topological analysis results
    """
    # Basic topology
    euler_char = mesh.compute_euler_characteristic()
    components = mesh.get_connected_components()
    holes = detect_holes(mesh)
    
    # Connectivity analysis
    connectivity = analyze_connectivity(mesh)
    
    # Genus computation
    if mesh.is_closed():
        genus = mesh.compute_genus()
    else:
        genus = compute_genus_from_euler(euler_char, len(components), len(holes))
    
    return {
        'basic': {
            'vertices': mesh.n_vertices,
            'faces': mesh.n_faces,
            'edges': connectivity['total_edges'],
            'euler_characteristic': euler_char,
            'is_closed': mesh.is_closed(),
            'is_manifold': connectivity['is_manifold']
        },
        'connectivity': connectivity,
        'components': {
            'count': len(components),
            'sizes': [len(comp) for comp in components],
            'largest_size': max(len(comp) for comp in components) if components else 0
        },
        'topology': {
            'genus': genus,
            'holes': {
                'count': len(holes),
                'boundary_loops': holes
            }
        }
    }