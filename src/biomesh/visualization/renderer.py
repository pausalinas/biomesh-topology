"""3D mesh visualization and rendering."""

import numpy as np
from typing import Optional, Dict, Any, Union
import warnings

try:
    import pyvista as pv
    import matplotlib.pyplot as plt
    from matplotlib.colors import Normalize
    import matplotlib.cm as cm
    VISUALIZATION_AVAILABLE = True
except ImportError:
    VISUALIZATION_AVAILABLE = False
    warnings.warn("Visualization dependencies not available. Install pyvista and matplotlib.")

from ..core.mesh import Mesh


def visualize_mesh(
    mesh: Mesh,
    color_by: str = 'default',
    colormap: str = 'viridis',
    show_edges: bool = False,
    show_normals: bool = False,
    opacity: float = 1.0,
    show: bool = True,
    window_size: tuple = (800, 600),
    **kwargs
) -> Optional[Any]:
    """
    Visualize a 3D mesh with various rendering options.
    
    Parameters
    ----------
    mesh : Mesh
        The mesh to visualize
    color_by : str, default 'default'
        Property to color the mesh by ('default', 'curvature', 'area', 'normal')
    colormap : str, default 'viridis'
        Matplotlib colormap name
    show_edges : bool, default False
        Whether to show mesh edges
    show_normals : bool, default False
        Whether to show face normals
    opacity : float, default 1.0
        Mesh opacity (0.0 to 1.0)
    show : bool, default True
        Whether to show the interactive window
    window_size : tuple, default (800, 600)
        Window size for the visualization
    **kwargs : dict
        Additional visualization parameters
        
    Returns
    -------
    pyvista.Plotter or None
        The plotter object if visualization is available, None otherwise
    """
    if not VISUALIZATION_AVAILABLE:
        warnings.warn("Visualization not available. Install pyvista and matplotlib.")
        return None
    
    # Create PyVista mesh
    pv_mesh = _mesh_to_pyvista(mesh)
    
    # Compute colors based on the specified property
    if color_by != 'default':
        colors = _compute_mesh_colors(mesh, color_by)
        if colors is not None:
            pv_mesh.point_data[color_by] = colors
    
    # Create plotter
    plotter = pv.Plotter(window_size=window_size)
    
    # Add mesh to plotter
    mesh_actor = plotter.add_mesh(
        pv_mesh,
        scalars=color_by if color_by != 'default' else None,
        cmap=colormap,
        show_edges=show_edges,
        opacity=opacity,
        **kwargs
    )
    
    # Add normals if requested
    if show_normals:
        normals = pv_mesh.compute_normals(point_normals=False)
        plotter.add_mesh(normals.arrows, color='red', opacity=0.7)
    
    # Set camera and lighting
    plotter.camera_position = 'isometric'
    plotter.add_axes()
    
    # Show colorbar if coloring is applied
    if color_by != 'default':
        plotter.add_scalar_bar(color_by, vertical=True)
    
    if show:
        plotter.show()
    
    return plotter


def create_figure(
    mesh: Mesh,
    color_by: str = 'default',
    colormap: str = 'viridis',
    figsize: tuple = (10, 8),
    save_path: Optional[str] = None,
    **kwargs
) -> Optional[Any]:
    """
    Create a matplotlib figure of the mesh.
    
    Parameters
    ----------
    mesh : Mesh
        The mesh to visualize
    color_by : str, default 'default'
        Property to color the mesh by
    colormap : str, default 'viridis'
        Matplotlib colormap name
    figsize : tuple, default (10, 8)
        Figure size in inches
    save_path : str, optional
        Path to save the figure
    **kwargs : dict
        Additional matplotlib parameters
        
    Returns
    -------
    matplotlib.figure.Figure or None
        The figure object if matplotlib is available, None otherwise
    """
    if not VISUALIZATION_AVAILABLE:
        warnings.warn("Visualization not available. Install matplotlib.")
        return None
    
    fig = plt.figure(figsize=figsize)
    ax = fig.add_subplot(111, projection='3d')
    
    # Get mesh data
    vertices = mesh.vertices
    faces = mesh.faces
    
    # Compute colors if specified
    if color_by != 'default':
        colors = _compute_mesh_colors(mesh, color_by)
        if colors is not None:
            norm = Normalize(vmin=np.min(colors), vmax=np.max(colors))
            colormap_obj = cm.get_cmap(colormap)
            face_colors = colormap_obj(norm(colors))
        else:
            face_colors = 'lightblue'
    else:
        face_colors = 'lightblue'
    
    # Plot triangles
    for i, face in enumerate(faces):
        triangle = vertices[face]
        if isinstance(face_colors, np.ndarray):
            color = face_colors[i] if len(face_colors) > i else 'lightblue'
        else:
            color = face_colors
        
        # Create triangle patch
        xs, ys, zs = triangle.T
        ax.plot_trisurf(xs, ys, zs, color=color, alpha=0.8, **kwargs)
    
    # Set equal aspect ratio
    ax.set_xlabel('X')
    ax.set_ylabel('Y')
    ax.set_zlabel('Z')
    
    # Set equal scaling
    max_range = np.max(np.ptp(vertices, axis=0)) / 2.0
    mid_x = np.mean(vertices[:, 0])
    mid_y = np.mean(vertices[:, 1])
    mid_z = np.mean(vertices[:, 2])
    ax.set_xlim(mid_x - max_range, mid_x + max_range)
    ax.set_ylim(mid_y - max_range, mid_y + max_range)
    ax.set_zlim(mid_z - max_range, mid_z + max_range)
    
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight')
    
    return fig


def _mesh_to_pyvista(mesh: Mesh) -> 'pv.PolyData':
    """Convert BiMesh Mesh to PyVista PolyData."""
    # Create faces array in PyVista format
    faces_flat = []
    for face in mesh.faces:
        faces_flat.extend([3] + list(face))
    
    faces_array = np.array(faces_flat)
    
    # Create PyVista mesh
    pv_mesh = pv.PolyData(mesh.vertices, faces_array)
    
    return pv_mesh


def _compute_mesh_colors(mesh: Mesh, color_by: str) -> Optional[np.ndarray]:
    """Compute colors for mesh visualization based on specified property."""
    if color_by == 'curvature':
        return _compute_curvature_colors(mesh)
    elif color_by == 'area':
        return _compute_area_colors(mesh)
    elif color_by == 'normal':
        return _compute_normal_colors(mesh)
    else:
        warnings.warn(f"Unknown color property: {color_by}")
        return None


def _compute_curvature_colors(mesh: Mesh) -> np.ndarray:
    """Compute vertex colors based on mean curvature."""
    from ..analysis.geometry import analyze_curvature
    
    try:
        curvature_analysis = analyze_curvature(mesh)
        mean_curvatures = curvature_analysis['mean_curvature']['values']
        return mean_curvatures
    except Exception as e:
        warnings.warn(f"Could not compute curvature colors: {e}")
        return np.ones(mesh.n_vertices)


def _compute_area_colors(mesh: Mesh) -> np.ndarray:
    """Compute face colors based on face areas."""
    face_areas = mesh.compute_face_areas()
    
    # Map face areas to vertices (average over adjacent faces)
    vertex_areas = np.zeros(mesh.n_vertices)
    vertex_counts = np.zeros(mesh.n_vertices)
    
    for i, face in enumerate(mesh.faces):
        for vertex in face:
            vertex_areas[vertex] += face_areas[i]
            vertex_counts[vertex] += 1
    
    # Avoid division by zero
    vertex_counts[vertex_counts == 0] = 1
    vertex_areas = vertex_areas / vertex_counts
    
    return vertex_areas


def _compute_normal_colors(mesh: Mesh) -> np.ndarray:
    """Compute vertex colors based on normal direction."""
    # Compute vertex normals (area-weighted average of adjacent face normals)
    vertex_normals = np.zeros_like(mesh.vertices)
    vertex_areas = np.zeros(mesh.n_vertices)
    
    face_normals = mesh.compute_face_normals()
    face_areas = mesh.compute_face_areas()
    
    for i, face in enumerate(mesh.faces):
        for vertex in face:
            vertex_normals[vertex] += face_normals[i] * face_areas[i]
            vertex_areas[vertex] += face_areas[i]
    
    # Normalize
    for i in range(mesh.n_vertices):
        if vertex_areas[i] > 0:
            vertex_normals[i] /= vertex_areas[i]
            vertex_normals[i] /= np.linalg.norm(vertex_normals[i])
    
    # Use Z-component of normal as color
    return vertex_normals[:, 2]


def create_comparison_plot(
    meshes: list,
    labels: list,
    property_name: str = 'surface_area',
    figsize: tuple = (10, 6),
    save_path: Optional[str] = None
) -> Optional[Any]:
    """
    Create a comparison plot for multiple meshes.
    
    Parameters
    ----------
    meshes : list of Mesh
        List of meshes to compare
    labels : list of str
        Labels for each mesh
    property_name : str, default 'surface_area'
        Property to compare
    figsize : tuple, default (10, 6)
        Figure size
    save_path : str, optional
        Path to save the figure
        
    Returns
    -------
    matplotlib.figure.Figure or None
        The comparison figure
    """
    if not VISUALIZATION_AVAILABLE:
        warnings.warn("Visualization not available. Install matplotlib.")
        return None
    
    # Analyze all meshes
    results = []
    for mesh in meshes:
        from ..core.analyzer import TopologyAnalyzer
        analyzer = TopologyAnalyzer()
        result = analyzer.analyze(mesh)
        results.append(result)
    
    # Extract property values
    values = []
    for result in results:
        if property_name in result.get('geometry', {}):
            values.append(result['geometry'][property_name])
        elif property_name in result.get('topology', {}):
            values.append(result['topology'][property_name])
        else:
            values.append(0)
    
    # Create bar plot
    fig, ax = plt.subplots(figsize=figsize)
    bars = ax.bar(labels, values)
    
    ax.set_xlabel('Mesh')
    ax.set_ylabel(property_name.replace('_', ' ').title())
    ax.set_title(f'Comparison of {property_name.replace("_", " ").title()}')
    
    # Add value labels on bars
    for bar, value in zip(bars, values):
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height,
                f'{value:.3f}', ha='center', va='bottom')
    
    plt.xticks(rotation=45)
    plt.tight_layout()
    
    if save_path:
        plt.savefig(save_path, dpi=300, bbox_inches='tight')
    
    return fig