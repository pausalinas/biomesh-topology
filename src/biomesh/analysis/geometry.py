"""Geometric analysis functions."""

import numpy as np
from typing import Dict, Any, Optional, Tuple
from ..core.mesh import Mesh


def compute_surface_area(mesh: Mesh) -> float:
    """
    Compute total surface area of the mesh.
    
    Parameters
    ----------
    mesh : Mesh
        Input mesh
        
    Returns
    -------
    float
        Total surface area
    """
    return mesh.compute_surface_area()


def compute_volume(mesh: Mesh) -> float:
    """
    Compute volume enclosed by the mesh (assuming closed mesh).
    
    Parameters
    ----------
    mesh : Mesh
        Input mesh
        
    Returns
    -------
    float
        Enclosed volume
    """
    return mesh.compute_volume()


def analyze_curvature(mesh: Mesh, method: str = 'discrete') -> Dict[str, Any]:
    """
    Analyze mesh curvature properties.
    
    Parameters
    ----------
    mesh : Mesh
        Input mesh
    method : str, default 'discrete'
        Curvature computation method
        
    Returns
    -------
    Dict[str, Any]
        Curvature analysis results
    """
    if method == 'discrete':
        return _compute_discrete_curvature(mesh)
    else:
        raise ValueError(f"Unknown curvature method: {method}")


def _compute_discrete_curvature(mesh: Mesh) -> Dict[str, Any]:
    """Compute discrete curvature measures."""
    vertices = mesh.vertices
    faces = mesh.faces
    
    # Initialize arrays
    mean_curvatures = np.zeros(mesh.n_vertices)
    gaussian_curvatures = np.zeros(mesh.n_vertices)
    vertex_areas = np.zeros(mesh.n_vertices)
    
    # Build vertex-face adjacency
    vertex_faces = [[] for _ in range(mesh.n_vertices)]
    for i, face in enumerate(faces):
        for vertex in face:
            vertex_faces[vertex].append(i)
    
    # Compute curvatures for each vertex
    for i, vertex in enumerate(vertices):
        adjacent_faces = vertex_faces[i]
        if len(adjacent_faces) < 3:
            continue
        
        # Compute local area and angles
        total_area = 0
        angle_sum = 0
        
        for face_idx in adjacent_faces:
            face = faces[face_idx]
            if i not in face:
                continue
            
            # Get vertex position in face
            pos = np.where(face == i)[0][0]
            prev_vertex = face[(pos - 1) % 3]
            next_vertex = face[(pos + 1) % 3]
            
            # Compute vectors
            v1 = vertices[prev_vertex] - vertices[i]
            v2 = vertices[next_vertex] - vertices[i]
            
            # Compute angle at vertex i
            cos_angle = np.dot(v1, v2) / (np.linalg.norm(v1) * np.linalg.norm(v2))
            cos_angle = np.clip(cos_angle, -1, 1)
            angle = np.arccos(cos_angle)
            angle_sum += angle
            
            # Compute face area contribution
            face_area = 0.5 * np.linalg.norm(np.cross(v1, v2))
            total_area += face_area / 3  # Distribute equally among vertices
        
        vertex_areas[i] = total_area
        
        # Gaussian curvature from angle deficit
        if total_area > 0:
            gaussian_curvatures[i] = (2 * np.pi - angle_sum) / total_area
        
        # Mean curvature approximation
        # This is a simplified computation - more sophisticated methods exist
        mean_curvatures[i] = _compute_mean_curvature_at_vertex(mesh, i)
    
    # Compute statistics
    mean_curv_stats = _compute_stats(mean_curvatures)
    gaussian_curv_stats = _compute_stats(gaussian_curvatures)
    
    return {
        'mean_curvature': {
            'values': mean_curvatures,
            'statistics': mean_curv_stats
        },
        'gaussian_curvature': {
            'values': gaussian_curvatures,
            'statistics': gaussian_curv_stats
        },
        'vertex_areas': vertex_areas,
        'total_mean_curvature': np.sum(mean_curvatures * vertex_areas),
        'total_gaussian_curvature': np.sum(gaussian_curvatures * vertex_areas)
    }


def _compute_mean_curvature_at_vertex(mesh: Mesh, vertex_idx: int) -> float:
    """Compute mean curvature at a specific vertex using cotangent weights."""
    vertices = mesh.vertices
    faces = mesh.faces
    
    # Find adjacent vertices
    adjacent_vertices = set()
    for face in faces:
        if vertex_idx in face:
            adjacent_vertices.update(face)
    adjacent_vertices.discard(vertex_idx)
    
    if len(adjacent_vertices) < 2:
        return 0.0
    
    center = vertices[vertex_idx]
    mean_curvature_vector = np.zeros(3)
    total_weight = 0
    
    # Compute cotangent weights
    for adj_vertex in adjacent_vertices:
        # Find faces that contain both vertices
        shared_faces = []
        for face in faces:
            if vertex_idx in face and adj_vertex in face:
                shared_faces.append(face)
        
        if len(shared_faces) != 2:
            continue  # Boundary edge or non-manifold
        
        # Compute cotangent weights from the two adjacent faces
        cot_sum = 0
        for face in shared_faces:
            # Find the third vertex
            third_vertex = None
            for v in face:
                if v != vertex_idx and v != adj_vertex:
                    third_vertex = v
                    break
            
            if third_vertex is None:
                continue
            
            # Compute cotangent of angle at third vertex
            v1 = vertices[vertex_idx] - vertices[third_vertex]
            v2 = vertices[adj_vertex] - vertices[third_vertex]
            
            cos_angle = np.dot(v1, v2) / (np.linalg.norm(v1) * np.linalg.norm(v2))
            cos_angle = np.clip(cos_angle, -1, 1)
            
            if abs(cos_angle) < 0.9999:  # Avoid division by zero
                sin_angle = np.sqrt(1 - cos_angle**2)
                cot_angle = cos_angle / sin_angle
                cot_sum += cot_angle
        
        # Add weighted contribution
        edge_vector = vertices[adj_vertex] - center
        weight = cot_sum / 2
        mean_curvature_vector += weight * edge_vector
        total_weight += weight
    
    if total_weight > 0:
        mean_curvature_vector /= total_weight
        return np.linalg.norm(mean_curvature_vector)
    
    return 0.0


def compute_bounding_box(mesh: Mesh) -> Dict[str, Any]:
    """
    Compute axis-aligned bounding box.
    
    Parameters
    ----------
    mesh : Mesh
        Input mesh
        
    Returns
    -------
    Dict[str, Any]
        Bounding box information
    """
    vertices = mesh.vertices
    min_coords = np.min(vertices, axis=0)
    max_coords = np.max(vertices, axis=0)
    size = max_coords - min_coords
    center = (min_coords + max_coords) / 2
    
    return {
        'min': min_coords.tolist(),
        'max': max_coords.tolist(),
        'size': size.tolist(),
        'center': center.tolist(),
        'volume': np.prod(size),
        'diagonal': np.linalg.norm(size)
    }


def compute_principal_axes(mesh: Mesh) -> Dict[str, Any]:
    """
    Compute principal axes using PCA on vertices.
    
    Parameters
    ----------
    mesh : Mesh
        Input mesh
        
    Returns
    -------
    Dict[str, Any]
        Principal axes information
    """
    vertices = mesh.vertices
    centroid = np.mean(vertices, axis=0)
    centered_vertices = vertices - centroid
    
    # Compute covariance matrix
    cov_matrix = np.cov(centered_vertices.T)
    
    # Compute eigenvalues and eigenvectors
    eigenvalues, eigenvectors = np.linalg.eigh(cov_matrix)
    
    # Sort by eigenvalue (largest first)
    idx = np.argsort(eigenvalues)[::-1]
    eigenvalues = eigenvalues[idx]
    eigenvectors = eigenvectors[:, idx]
    
    return {
        'centroid': centroid.tolist(),
        'eigenvalues': eigenvalues.tolist(),
        'eigenvectors': eigenvectors.tolist(),
        'principal_axes': eigenvectors.T.tolist(),
        'aspect_ratios': [eigenvalues[0] / eigenvalues[1], 
                         eigenvalues[1] / eigenvalues[2]] if eigenvalues[2] > 0 else [0, 0]
    }


def analyze_shape_descriptors(mesh: Mesh) -> Dict[str, Any]:
    """
    Compute various shape descriptors.
    
    Parameters
    ----------
    mesh : Mesh
        Input mesh
        
    Returns
    -------
    Dict[str, Any]
        Shape descriptor results
    """
    surface_area = mesh.compute_surface_area()
    
    results = {
        'surface_area': surface_area,
        'bounding_box': compute_bounding_box(mesh),
        'principal_axes': compute_principal_axes(mesh)
    }
    
    if mesh.is_closed():
        volume = mesh.compute_volume()
        results['volume'] = volume
        results['compactness'] = surface_area / (volume ** (2/3)) if volume > 0 else float('inf')
        results['sphericity'] = (np.pi ** (1/3) * (6 * volume) ** (2/3)) / surface_area if surface_area > 0 else 0
        results['surface_area_to_volume_ratio'] = surface_area / volume if volume > 0 else float('inf')
    
    return results


def _compute_stats(values: np.ndarray) -> Dict[str, float]:
    """Compute basic statistics for an array of values."""
    valid_values = values[np.isfinite(values)]
    if len(valid_values) == 0:
        return {
            'mean': 0.0,
            'std': 0.0,
            'min': 0.0,
            'max': 0.0,
            'median': 0.0,
            'count': 0
        }
    
    return {
        'mean': float(np.mean(valid_values)),
        'std': float(np.std(valid_values)),
        'min': float(np.min(valid_values)),
        'max': float(np.max(valid_values)),
        'median': float(np.median(valid_values)),
        'count': len(valid_values)
    }