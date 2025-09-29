"""Topology analyzer for comprehensive mesh analysis."""

import numpy as np
from typing import Dict, Any, Optional, List
import time
from ..core.mesh import Mesh


class TopologyAnalyzer:
    """
    Comprehensive topology analyzer for biological meshes.
    
    This class provides methods for analyzing various topological and
    geometric properties of biological mesh structures.
    """
    
    def __init__(self, progress_callback: Optional[callable] = None):
        """
        Initialize the topology analyzer.
        
        Parameters
        ----------
        progress_callback : callable, optional
            Callback function to report analysis progress
        """
        self.progress_callback = progress_callback
        self._analysis_cache = {}
    
    def analyze(self, mesh: Mesh, **kwargs) -> Dict[str, Any]:
        """
        Perform comprehensive topological analysis.
        
        Parameters
        ----------
        mesh : Mesh
            The mesh to analyze
        **kwargs : dict
            Analysis options and parameters
            
        Returns
        -------
        Dict[str, Any]
            Comprehensive analysis results
        """
        start_time = time.time()
        
        # Initialize results dictionary
        results = {
            'metadata': {
                'analysis_timestamp': time.time(),
                'mesh_vertices': mesh.n_vertices,
                'mesh_faces': mesh.n_faces,
                'analysis_options': kwargs
            },
            'topology': {},
            'geometry': {},
            'biological': {},
            'quality': {}
        }
        
        self._report_progress("Starting topology analysis", 0)
        
        # Basic topology analysis
        self._analyze_basic_topology(mesh, results['topology'])
        self._report_progress("Basic topology analysis complete", 25)
        
        # Geometric analysis
        self._analyze_geometry(mesh, results['geometry'])
        self._report_progress("Geometric analysis complete", 50)
        
        # Biological characterization
        self._analyze_biological_properties(mesh, results['biological'], **kwargs)
        self._report_progress("Biological analysis complete", 75)
        
        # Mesh quality assessment
        self._analyze_mesh_quality(mesh, results['quality'])
        self._report_progress("Quality analysis complete", 100)
        
        results['metadata']['analysis_duration'] = time.time() - start_time
        
        return results
    
    def _analyze_basic_topology(self, mesh: Mesh, results: Dict[str, Any]) -> None:
        """Analyze basic topological properties."""
        # Euler characteristic and genus
        euler_char = mesh.compute_euler_characteristic()
        results['euler_characteristic'] = euler_char
        
        # Genus (for closed surfaces)
        if mesh.is_closed():
            results['genus'] = mesh.compute_genus()
            results['is_closed'] = True
        else:
            results['is_closed'] = False
            results['boundary_edges'] = len(mesh.get_boundary_edges())
        
        # Connected components
        components = mesh.get_connected_components()
        results['connected_components'] = len(components)
        results['component_sizes'] = [len(comp) for comp in components]
        
        # Topological invariants
        results['vertices'] = mesh.n_vertices
        results['faces'] = mesh.n_faces
        results['edges'] = mesh.n_vertices + mesh.n_faces - euler_char
    
    def _analyze_geometry(self, mesh: Mesh, results: Dict[str, Any]) -> None:
        """Analyze geometric properties."""
        # Surface area and volume
        results['surface_area'] = mesh.compute_surface_area()
        
        if mesh.is_closed():
            results['volume'] = mesh.compute_volume()
            results['surface_area_to_volume_ratio'] = results['surface_area'] / results['volume']
        
        # Face areas statistics
        face_areas = mesh.compute_face_areas()
        results['face_area_stats'] = {
            'mean': np.mean(face_areas),
            'std': np.std(face_areas),
            'min': np.min(face_areas),
            'max': np.max(face_areas),
            'median': np.median(face_areas)
        }
        
        # Bounding box
        vertices = mesh.vertices
        bbox_min = np.min(vertices, axis=0)
        bbox_max = np.max(vertices, axis=0)
        results['bounding_box'] = {
            'min': bbox_min.tolist(),
            'max': bbox_max.tolist(),
            'size': (bbox_max - bbox_min).tolist()
        }
        
        # Centroid
        results['centroid'] = np.mean(vertices, axis=0).tolist()
    
    def _analyze_biological_properties(self, mesh: Mesh, results: Dict[str, Any], **kwargs) -> None:
        """Analyze biological-specific properties."""
        # Porosity analysis (if applicable)
        if mesh.is_closed():
            results['compactness'] = self._compute_compactness(mesh)
            results['sphericity'] = self._compute_sphericity(mesh)
        
        # Surface roughness
        results['surface_roughness'] = self._compute_surface_roughness(mesh)
        
        # Local curvature analysis
        if kwargs.get('compute_curvature', True):
            curvature_stats = self._analyze_curvature(mesh)
            results['curvature'] = curvature_stats
        
        # Mesh connectivity analysis
        connectivity_stats = self._analyze_connectivity(mesh)
        results['connectivity'] = connectivity_stats
    
    def _analyze_mesh_quality(self, mesh: Mesh, results: Dict[str, Any]) -> None:
        """Analyze mesh quality metrics."""
        # Face quality metrics
        face_areas = mesh.compute_face_areas()
        
        # Detect degenerate faces
        degenerate_faces = np.sum(face_areas < 1e-12)
        results['degenerate_faces'] = int(degenerate_faces)
        
        # Aspect ratio analysis
        aspect_ratios = self._compute_face_aspect_ratios(mesh)
        results['aspect_ratio_stats'] = {
            'mean': np.mean(aspect_ratios),
            'std': np.std(aspect_ratios),
            'min': np.min(aspect_ratios),
            'max': np.max(aspect_ratios)
        }
        
        # Edge length statistics
        edge_lengths = self._compute_edge_lengths(mesh)
        results['edge_length_stats'] = {
            'mean': np.mean(edge_lengths),
            'std': np.std(edge_lengths),
            'min': np.min(edge_lengths),
            'max': np.max(edge_lengths)
        }
    
    def _compute_compactness(self, mesh: Mesh) -> float:
        """Compute mesh compactness (surface area to volume ratio)."""
        surface_area = mesh.compute_surface_area()
        volume = mesh.compute_volume()
        if volume > 0:
            return surface_area / (volume ** (2/3))
        return float('inf')
    
    def _compute_sphericity(self, mesh: Mesh) -> float:
        """Compute sphericity (how sphere-like the mesh is)."""
        volume = mesh.compute_volume()
        surface_area = mesh.compute_surface_area()
        
        # Sphericity = (π^(1/3) * (6V)^(2/3)) / A
        if surface_area > 0:
            return (np.pi ** (1/3) * (6 * volume) ** (2/3)) / surface_area
        return 0.0
    
    def _compute_surface_roughness(self, mesh: Mesh) -> float:
        """Compute surface roughness based on normal variation."""
        normals = mesh.compute_face_normals()
        face_areas = mesh.compute_face_areas()
        
        # Weight normals by face area
        weighted_normals = normals * face_areas[:, np.newaxis]
        mean_normal = np.sum(weighted_normals, axis=0)
        mean_normal /= np.linalg.norm(mean_normal)
        
        # Compute deviation from mean normal
        deviations = 1 - np.dot(normals, mean_normal)
        return np.mean(deviations)
    
    def _analyze_curvature(self, mesh: Mesh) -> Dict[str, Any]:
        """Analyze mesh curvature properties."""
        # Simplified curvature analysis
        # In a full implementation, this would use more sophisticated algorithms
        
        vertices = mesh.vertices
        faces = mesh.faces
        
        # Compute vertex normals (area-weighted)
        vertex_normals = np.zeros_like(vertices)
        vertex_areas = np.zeros(len(vertices))
        
        for face in faces:
            v0, v1, v2 = vertices[face]
            normal = np.cross(v1 - v0, v2 - v0)
            area = 0.5 * np.linalg.norm(normal)
            normal = normal / (2 * area) if area > 0 else normal
            
            for vertex_idx in face:
                vertex_normals[vertex_idx] += area * normal
                vertex_areas[vertex_idx] += area / 3
        
        # Normalize vertex normals
        for i in range(len(vertex_normals)):
            if vertex_areas[i] > 0:
                vertex_normals[i] /= np.linalg.norm(vertex_normals[i])
        
        # Estimate mean curvature from normal variation
        mean_curvatures = []
        for i, vertex in enumerate(vertices):
            # Find neighboring vertices
            neighbors = set()
            for face in faces:
                if i in face:
                    neighbors.update(face)
            neighbors.discard(i)
            
            if neighbors:
                neighbor_normals = vertex_normals[list(neighbors)]
                current_normal = vertex_normals[i]
                
                # Estimate curvature from normal deviation
                curvature = np.mean([np.linalg.norm(current_normal - nn) 
                                   for nn in neighbor_normals])
                mean_curvatures.append(curvature)
        
        return {
            'mean_curvature_stats': {
                'mean': np.mean(mean_curvatures),
                'std': np.std(mean_curvatures),
                'min': np.min(mean_curvatures),
                'max': np.max(mean_curvatures)
            }
        }
    
    def _analyze_connectivity(self, mesh: Mesh) -> Dict[str, Any]:
        """Analyze mesh connectivity properties."""
        # Build vertex-vertex adjacency
        adjacency = [set() for _ in range(mesh.n_vertices)]
        for face in mesh.faces:
            for i in range(3):
                v1, v2 = face[i], face[(i + 1) % 3]
                adjacency[v1].add(v2)
                adjacency[v2].add(v1)
        
        # Compute vertex degree statistics
        degrees = [len(neighbors) for neighbors in adjacency]
        
        return {
            'vertex_degree_stats': {
                'mean': np.mean(degrees),
                'std': np.std(degrees),
                'min': np.min(degrees),
                'max': np.max(degrees)
            },
            'regular_vertices': sum(1 for d in degrees if d == 6),  # Ideal for triangular meshes
            'boundary_vertices': sum(1 for d in degrees if d < 4)
        }
    
    def _compute_face_aspect_ratios(self, mesh: Mesh) -> np.ndarray:
        """Compute aspect ratios for all faces."""
        vertices = mesh.vertices
        faces = mesh.faces
        aspect_ratios = []
        
        for face in faces:
            v0, v1, v2 = vertices[face]
            
            # Compute edge lengths
            edge_lengths = [
                np.linalg.norm(v1 - v0),
                np.linalg.norm(v2 - v1),
                np.linalg.norm(v0 - v2)
            ]
            
            max_edge = max(edge_lengths)
            min_edge = min(edge_lengths)
            
            aspect_ratio = max_edge / min_edge if min_edge > 0 else float('inf')
            aspect_ratios.append(aspect_ratio)
        
        return np.array(aspect_ratios)
    
    def _compute_edge_lengths(self, mesh: Mesh) -> np.ndarray:
        """Compute all edge lengths in the mesh."""
        vertices = mesh.vertices
        faces = mesh.faces
        edge_lengths = []
        
        edges = set()
        for face in faces:
            for i in range(3):
                edge = tuple(sorted([face[i], face[(i + 1) % 3]]))
                edges.add(edge)
        
        for v1, v2 in edges:
            length = np.linalg.norm(vertices[v1] - vertices[v2])
            edge_lengths.append(length)
        
        return np.array(edge_lengths)
    
    def _report_progress(self, message: str, progress: int) -> None:
        """Report analysis progress."""
        if self.progress_callback:
            self.progress_callback(message, progress)