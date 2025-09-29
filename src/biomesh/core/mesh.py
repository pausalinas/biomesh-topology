"""Core mesh data structure for biological mesh topology analysis."""

import numpy as np
from typing import Optional, Dict, Any, List, Tuple
import warnings


class Mesh:
    """
    Core mesh data structure for biological topology analysis.
    
    This class represents a 3D triangular mesh with additional metadata
    and analysis capabilities specific to biological structures.
    """
    
    def __init__(
        self,
        vertices: np.ndarray,
        faces: np.ndarray,
        metadata: Optional[Dict[str, Any]] = None,
        validate: bool = True
    ):
        """
        Initialize a Mesh object.
        
        Parameters
        ----------
        vertices : np.ndarray
            Array of shape (n_vertices, 3) containing vertex coordinates
        faces : np.ndarray  
            Array of shape (n_faces, 3) containing face indices
        metadata : dict, optional
            Additional metadata about the mesh
        validate : bool, default True
            Whether to validate mesh integrity upon creation
        """
        self._vertices = np.asarray(vertices, dtype=np.float64)
        self._faces = np.asarray(faces, dtype=np.int32)
        self._metadata = metadata or {}
        
        # Cached properties
        self._normals = None
        self._areas = None
        self._volumes = None
        self._euler_characteristic = None
        
        if validate:
            self._validate()
    
    @property
    def vertices(self) -> np.ndarray:
        """Get mesh vertices."""
        return self._vertices
    
    @property
    def faces(self) -> np.ndarray:
        """Get mesh faces."""
        return self._faces
    
    @property
    def n_vertices(self) -> int:
        """Number of vertices in the mesh."""
        return len(self._vertices)
    
    @property
    def n_faces(self) -> int:
        """Number of faces in the mesh."""
        return len(self._faces)
    
    @property
    def metadata(self) -> Dict[str, Any]:
        """Get mesh metadata."""
        return self._metadata
    
    def _validate(self) -> None:
        """Validate mesh integrity."""
        if self._vertices.shape[1] != 3:
            raise ValueError("Vertices must have shape (n, 3)")
        
        if self._faces.shape[1] != 3:
            raise ValueError("Faces must have shape (n, 3)")
        
        if np.any(self._faces < 0) or np.any(self._faces >= self.n_vertices):
            raise ValueError("Face indices must be valid vertex indices")
    
    def compute_face_normals(self) -> np.ndarray:
        """
        Compute face normal vectors.
        
        Returns
        -------
        np.ndarray
            Array of shape (n_faces, 3) containing face normals
        """
        if self._normals is None:
            v0 = self._vertices[self._faces[:, 0]]
            v1 = self._vertices[self._faces[:, 1]]
            v2 = self._vertices[self._faces[:, 2]]
            
            # Compute cross product for normals
            normals = np.cross(v1 - v0, v2 - v0)
            
            # Normalize
            norms = np.linalg.norm(normals, axis=1, keepdims=True)
            norms[norms == 0] = 1  # Avoid division by zero
            self._normals = normals / norms
        
        return self._normals
    
    def compute_face_areas(self) -> np.ndarray:
        """
        Compute face areas.
        
        Returns
        -------
        np.ndarray
            Array of face areas
        """
        if self._areas is None:
            v0 = self._vertices[self._faces[:, 0]]
            v1 = self._vertices[self._faces[:, 1]]
            v2 = self._vertices[self._faces[:, 2]]
            
            # Compute areas using cross product magnitude
            cross = np.cross(v1 - v0, v2 - v0)
            self._areas = 0.5 * np.linalg.norm(cross, axis=1)
        
        return self._areas
    
    def compute_surface_area(self) -> float:
        """
        Compute total surface area.
        
        Returns
        -------
        float
            Total surface area
        """
        return np.sum(self.compute_face_areas())
    
    def compute_volume(self) -> float:
        """
        Compute mesh volume (assuming closed mesh).
        
        Returns
        -------
        float
            Mesh volume
        """
        if not self.is_closed():
            warnings.warn("Volume computation on non-closed mesh may be inaccurate")
        
        # Use divergence theorem
        v0 = self._vertices[self._faces[:, 0]]
        v1 = self._vertices[self._faces[:, 1]] 
        v2 = self._vertices[self._faces[:, 2]]
        
        # Compute signed volume contribution of each tetrahedron
        volumes = np.sum(v0 * np.cross(v1, v2), axis=1) / 6.0
        return abs(np.sum(volumes))
    
    def is_closed(self) -> bool:
        """
        Check if mesh is closed (no boundary edges).
        
        Returns
        -------
        bool
            True if mesh is closed
        """
        # Create edge list
        edges = set()
        for face in self._faces:
            for i in range(3):
                edge = tuple(sorted([face[i], face[(i + 1) % 3]]))
                if edge in edges:
                    edges.remove(edge)
                else:
                    edges.add(edge)
        
        return len(edges) == 0
    
    def get_boundary_edges(self) -> List[Tuple[int, int]]:
        """
        Get boundary edges (edges that belong to only one face).
        
        Returns
        -------
        List[Tuple[int, int]]
            List of boundary edge tuples
        """
        edge_count = {}
        for face in self._faces:
            for i in range(3):
                edge = tuple(sorted([face[i], face[(i + 1) % 3]]))
                edge_count[edge] = edge_count.get(edge, 0) + 1
        
        return [edge for edge, count in edge_count.items() if count == 1]
    
    def compute_euler_characteristic(self) -> int:
        """
        Compute Euler characteristic (V - E + F).
        
        Returns
        -------
        int
            Euler characteristic
        """
        if self._euler_characteristic is None:
            # Count edges
            edges = set()
            for face in self._faces:
                for i in range(3):
                    edge = tuple(sorted([face[i], face[(i + 1) % 3]]))
                    edges.add(edge)
            
            n_edges = len(edges)
            self._euler_characteristic = self.n_vertices - n_edges + self.n_faces
        
        return self._euler_characteristic
    
    def compute_genus(self) -> int:
        """
        Compute genus from Euler characteristic (for closed surfaces).
        
        Returns
        -------
        int
            Genus of the surface
        """
        if not self.is_closed():
            raise ValueError("Genus is only defined for closed surfaces")
        
        euler_char = self.compute_euler_characteristic()
        # For closed surfaces: genus = (2 - χ) / 2
        return (2 - euler_char) // 2
    
    def get_connected_components(self) -> List[List[int]]:
        """
        Find connected components of vertices.
        
        Returns
        -------
        List[List[int]]
            List of connected components (each component is a list of vertex indices)
        """
        # Build adjacency graph
        adjacency = [set() for _ in range(self.n_vertices)]
        for face in self._faces:
            for i in range(3):
                v1, v2 = face[i], face[(i + 1) % 3]
                adjacency[v1].add(v2)
                adjacency[v2].add(v1)
        
        # Find connected components using DFS
        visited = [False] * self.n_vertices
        components = []
        
        def dfs(vertex, component):
            visited[vertex] = True
            component.append(vertex)
            for neighbor in adjacency[vertex]:
                if not visited[neighbor]:
                    dfs(neighbor, component)
        
        for vertex in range(self.n_vertices):
            if not visited[vertex]:
                component = []
                dfs(vertex, component)
                components.append(component)
        
        return components
    
    def copy(self) -> 'Mesh':
        """Create a copy of the mesh."""
        return Mesh(
            vertices=self._vertices.copy(),
            faces=self._faces.copy(),
            metadata=self._metadata.copy(),
            validate=False
        )
    
    def __repr__(self) -> str:
        return f"Mesh(vertices={self.n_vertices}, faces={self.n_faces})"