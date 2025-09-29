"""Mesh validation and repair utilities."""

import numpy as np
from typing import Dict, Any, List, Tuple, Optional
import warnings

from ..core.mesh import Mesh


def validate_mesh(mesh: Mesh, verbose: bool = False) -> Dict[str, Any]:
    """
    Validate mesh integrity and quality.
    
    Parameters
    ----------
    mesh : Mesh
        Mesh to validate
    verbose : bool, default False
        Whether to print detailed validation information
        
    Returns
    -------
    Dict[str, Any]
        Validation results with issues found and recommendations
    """
    issues = []
    warnings_list = []
    stats = {}
    
    # Basic structure validation
    if mesh.n_vertices == 0:
        issues.append("Mesh has no vertices")
    
    if mesh.n_faces == 0:
        issues.append("Mesh has no faces")
    
    # Check for invalid face indices
    if np.any(mesh.faces < 0) or np.any(mesh.faces >= mesh.n_vertices):
        issues.append("Invalid face indices found")
    
    # Check for degenerate faces
    degenerate_faces = _find_degenerate_faces(mesh)
    if len(degenerate_faces) > 0:
        issues.append(f"Found {len(degenerate_faces)} degenerate faces")
        stats['degenerate_faces'] = degenerate_faces
    
    # Check for duplicate vertices
    duplicate_vertices = _find_duplicate_vertices(mesh)
    if len(duplicate_vertices) > 0:
        warnings_list.append(f"Found {len(duplicate_vertices)} duplicate vertices")
        stats['duplicate_vertices'] = duplicate_vertices
    
    # Check for unreferenced vertices
    unreferenced_vertices = _find_unreferenced_vertices(mesh)
    if len(unreferenced_vertices) > 0:
        warnings_list.append(f"Found {len(unreferenced_vertices)} unreferenced vertices")
        stats['unreferenced_vertices'] = unreferenced_vertices
    
    # Check manifoldness
    is_manifold, non_manifold_edges = _check_manifoldness(mesh)
    if not is_manifold:
        issues.append(f"Mesh is not manifold ({len(non_manifold_edges)} non-manifold edges)")
        stats['non_manifold_edges'] = non_manifold_edges
    
    # Check face orientation consistency
    orientation_issues = _check_face_orientation(mesh)
    if orientation_issues > 0:
        warnings_list.append(f"Found {orientation_issues} face orientation inconsistencies")
        stats['orientation_issues'] = orientation_issues
    
    # Compute quality metrics
    quality_metrics = _compute_quality_metrics(mesh)
    stats.update(quality_metrics)
    
    # Generate recommendations
    recommendations = _generate_recommendations(issues, warnings_list, stats)
    
    validation_result = {
        'is_valid': len(issues) == 0,
        'issues': issues,
        'warnings': warnings_list,
        'statistics': stats,
        'recommendations': recommendations
    }
    
    if verbose:
        _print_validation_report(validation_result)
    
    return validation_result


def repair_mesh(mesh: Mesh, repair_options: Optional[Dict[str, bool]] = None) -> Mesh:
    """
    Attempt to repair common mesh issues.
    
    Parameters
    ----------
    mesh : Mesh
        Mesh to repair
    repair_options : Dict[str, bool], optional
        Options for what to repair:
        - 'remove_duplicates': Remove duplicate vertices
        - 'remove_unreferenced': Remove unreferenced vertices  
        - 'remove_degenerate': Remove degenerate faces
        - 'fix_orientation': Fix face orientation
        
    Returns
    -------
    Mesh
        Repaired mesh
    """
    if repair_options is None:
        repair_options = {
            'remove_duplicates': True,
            'remove_unreferenced': True,
            'remove_degenerate': True,
            'fix_orientation': False  # This can be destructive
        }
    
    repaired_mesh = mesh.copy()
    
    # Remove duplicate vertices
    if repair_options.get('remove_duplicates', False):
        repaired_mesh = _remove_duplicate_vertices(repaired_mesh)
    
    # Remove degenerate faces
    if repair_options.get('remove_degenerate', False):
        repaired_mesh = _remove_degenerate_faces(repaired_mesh)
    
    # Remove unreferenced vertices
    if repair_options.get('remove_unreferenced', False):
        repaired_mesh = _remove_unreferenced_vertices(repaired_mesh)
    
    # Fix face orientation (experimental)
    if repair_options.get('fix_orientation', False):
        repaired_mesh = _fix_face_orientation(repaired_mesh)
    
    return repaired_mesh


def _find_degenerate_faces(mesh: Mesh, tolerance: float = 1e-12) -> List[int]:
    """Find faces with zero or near-zero area."""
    face_areas = mesh.compute_face_areas()
    return np.where(face_areas < tolerance)[0].tolist()


def _find_duplicate_vertices(mesh: Mesh, tolerance: float = 1e-9) -> List[Tuple[int, int]]:
    """Find pairs of duplicate vertices."""
    vertices = mesh.vertices
    duplicates = []
    
    for i in range(len(vertices)):
        for j in range(i + 1, len(vertices)):
            if np.linalg.norm(vertices[i] - vertices[j]) < tolerance:
                duplicates.append((i, j))
    
    return duplicates


def _find_unreferenced_vertices(mesh: Mesh) -> List[int]:
    """Find vertices that are not referenced by any face."""
    referenced_vertices = set(mesh.faces.flatten())
    all_vertices = set(range(mesh.n_vertices))
    return list(all_vertices - referenced_vertices)


def _check_manifoldness(mesh: Mesh) -> Tuple[bool, List[Tuple[int, int]]]:
    """Check if mesh is manifold (each edge shared by at most 2 faces)."""
    edge_count = {}
    for face in mesh.faces:
        for i in range(3):
            edge = tuple(sorted([face[i], face[(i + 1) % 3]]))
            edge_count[edge] = edge_count.get(edge, 0) + 1
    
    non_manifold_edges = [edge for edge, count in edge_count.items() if count > 2]
    return len(non_manifold_edges) == 0, non_manifold_edges


def _check_face_orientation(mesh: Mesh) -> int:
    """Check for inconsistent face orientations."""
    # This is a simplified check - a full implementation would be more complex
    edge_orientations = {}
    inconsistencies = 0
    
    for face in mesh.faces:
        for i in range(3):
            v1, v2 = face[i], face[(i + 1) % 3]
            edge = tuple(sorted([v1, v2]))
            orientation = 1 if v1 < v2 else -1
            
            if edge in edge_orientations:
                if edge_orientations[edge] == orientation:
                    inconsistencies += 1
            else:
                edge_orientations[edge] = orientation
    
    return inconsistencies


def _compute_quality_metrics(mesh: Mesh) -> Dict[str, Any]:
    """Compute various mesh quality metrics."""
    vertices = mesh.vertices
    faces = mesh.faces
    
    # Edge length statistics
    edge_lengths = []
    edges = set()
    for face in faces:
        for i in range(3):
            edge = tuple(sorted([face[i], face[(i + 1) % 3]]))
            if edge not in edges:
                edges.add(edge)
                v1, v2 = edge
                length = np.linalg.norm(vertices[v1] - vertices[v2])
                edge_lengths.append(length)
    
    edge_lengths = np.array(edge_lengths)
    
    # Aspect ratios
    aspect_ratios = []
    for face in faces:
        v0, v1, v2 = vertices[face]
        
        # Compute edge lengths
        edge_lens = [
            np.linalg.norm(v1 - v0),
            np.linalg.norm(v2 - v1),
            np.linalg.norm(v0 - v2)
        ]
        
        max_edge = max(edge_lens)
        min_edge = min(edge_lens)
        aspect_ratio = max_edge / min_edge if min_edge > 0 else float('inf')
        aspect_ratios.append(aspect_ratio)
    
    aspect_ratios = np.array(aspect_ratios)
    
    return {
        'edge_length_stats': {
            'mean': float(np.mean(edge_lengths)),
            'std': float(np.std(edge_lengths)),
            'min': float(np.min(edge_lengths)),
            'max': float(np.max(edge_lengths))
        },
        'aspect_ratio_stats': {
            'mean': float(np.mean(aspect_ratios)),
            'std': float(np.std(aspect_ratios)),
            'min': float(np.min(aspect_ratios)),
            'max': float(np.max(aspect_ratios))
        },
        'total_edges': len(edges)
    }


def _generate_recommendations(issues: List[str], warnings: List[str], stats: Dict[str, Any]) -> List[str]:
    """Generate repair recommendations based on validation results."""
    recommendations = []
    
    if 'degenerate_faces' in stats:
        recommendations.append("Remove degenerate faces using repair_mesh with 'remove_degenerate': True")
    
    if 'duplicate_vertices' in stats:
        recommendations.append("Merge duplicate vertices using repair_mesh with 'remove_duplicates': True")
    
    if 'unreferenced_vertices' in stats:
        recommendations.append("Remove unreferenced vertices using repair_mesh with 'remove_unreferenced': True")
    
    if 'non_manifold_edges' in stats:
        recommendations.append("Fix non-manifold edges - this may require manual intervention")
    
    aspect_stats = stats.get('aspect_ratio_stats', {})
    if aspect_stats.get('max', 0) > 10:
        recommendations.append("Consider mesh refinement to improve triangle quality")
    
    if stats.get('orientation_issues', 0) > 0:
        recommendations.append("Consider fixing face orientation inconsistencies")
    
    return recommendations


def _remove_duplicate_vertices(mesh: Mesh, tolerance: float = 1e-9) -> Mesh:
    """Remove duplicate vertices and update face indices."""
    vertices = mesh.vertices
    faces = mesh.faces
    
    # Find unique vertices
    unique_vertices = []
    vertex_mapping = {}
    
    for i, vertex in enumerate(vertices):
        # Check if this vertex is close to any existing unique vertex
        found_duplicate = False
        for j, unique_vertex in enumerate(unique_vertices):
            if np.linalg.norm(vertex - unique_vertex) < tolerance:
                vertex_mapping[i] = j
                found_duplicate = True
                break
        
        if not found_duplicate:
            vertex_mapping[i] = len(unique_vertices)
            unique_vertices.append(vertex)
    
    # Update face indices
    new_faces = []
    for face in faces:
        new_face = [vertex_mapping[vertex_idx] for vertex_idx in face]
        new_faces.append(new_face)
    
    return Mesh(
        vertices=np.array(unique_vertices),
        faces=np.array(new_faces),
        metadata=mesh.metadata.copy()
    )


def _remove_degenerate_faces(mesh: Mesh, tolerance: float = 1e-12) -> Mesh:
    """Remove faces with zero or near-zero area."""
    face_areas = mesh.compute_face_areas()
    valid_faces = mesh.faces[face_areas >= tolerance]
    
    return Mesh(
        vertices=mesh.vertices.copy(),
        faces=valid_faces,
        metadata=mesh.metadata.copy()
    )


def _remove_unreferenced_vertices(mesh: Mesh) -> Mesh:
    """Remove vertices that are not referenced by any face."""
    referenced_vertices = np.unique(mesh.faces.flatten())
    
    # Create mapping from old to new vertex indices
    vertex_mapping = {old_idx: new_idx for new_idx, old_idx in enumerate(referenced_vertices)}
    
    # Extract referenced vertices
    new_vertices = mesh.vertices[referenced_vertices]
    
    # Update face indices
    new_faces = []
    for face in mesh.faces:
        new_face = [vertex_mapping[vertex_idx] for vertex_idx in face]
        new_faces.append(new_face)
    
    return Mesh(
        vertices=new_vertices,
        faces=np.array(new_faces),
        metadata=mesh.metadata.copy()
    )


def _fix_face_orientation(mesh: Mesh) -> Mesh:
    """Attempt to fix face orientation consistency (experimental)."""
    # This is a complex problem - this is a simplified implementation
    warnings.warn("Face orientation fixing is experimental and may not work correctly")
    
    # For now, just return the original mesh
    # A proper implementation would use graph traversal to propagate consistent orientations
    return mesh.copy()


def _print_validation_report(validation_result: Dict[str, Any]) -> None:
    """Print a detailed validation report."""
    print("=== Mesh Validation Report ===")
    print(f"Valid: {'Yes' if validation_result['is_valid'] else 'No'}")
    
    if validation_result['issues']:
        print("\nIssues:")
        for issue in validation_result['issues']:
            print(f"  - {issue}")
    
    if validation_result['warnings']:
        print("\nWarnings:")
        for warning in validation_result['warnings']:
            print(f"  - {warning}")
    
    stats = validation_result['statistics']
    if stats:
        print("\nQuality Statistics:")
        for key, value in stats.items():
            if isinstance(value, dict):
                print(f"  {key}:")
                for subkey, subvalue in value.items():
                    print(f"    {subkey}: {subvalue}")
            else:
                print(f"  {key}: {value}")
    
    if validation_result['recommendations']:
        print("\nRecommendations:")
        for rec in validation_result['recommendations']:
            print(f"  - {rec}")
    
    print("=" * 30)