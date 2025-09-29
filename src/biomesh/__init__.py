"""
BiMesh Topology - Computational biology software for analyzing biological mesh topologies.

This package provides tools for analyzing and visualizing 3D biological mesh structures,
particularly focusing on cellular membranes, tissue scaffolds, and biomaterial networks.
"""

__version__ = "0.1.0"
__author__ = "BiMesh Topology Contributors"
__email__ = "biomesh@example.com"
__license__ = "GPL-3.0"

# Core functionality imports
from .core.mesh import Mesh
from .core.analyzer import TopologyAnalyzer
from .io.loader import load_mesh, save_mesh
from .analysis.topology import compute_euler_characteristic, analyze_connectivity
from .analysis.geometry import compute_surface_area, compute_volume, analyze_curvature
from .visualization.renderer import visualize_mesh, create_figure
from .utils.validation import validate_mesh, repair_mesh

# Main analysis function
def analyze_topology(mesh_path_or_obj, **kwargs):
    """
    Perform comprehensive topological analysis on a biological mesh.
    
    Parameters
    ----------
    mesh_path_or_obj : str or Mesh
        Path to mesh file or Mesh object
    **kwargs : dict
        Additional analysis parameters
        
    Returns
    -------
    dict
        Analysis results containing topological and geometric properties
    """
    if isinstance(mesh_path_or_obj, str):
        mesh = load_mesh(mesh_path_or_obj)
    else:
        mesh = mesh_path_or_obj
    
    analyzer = TopologyAnalyzer()
    return analyzer.analyze(mesh, **kwargs)

# Convenience functions
def generate_report(analysis_results, output_format='json'):
    """Generate analysis report in specified format."""
    from .utils.reporting import ReportGenerator
    generator = ReportGenerator()
    return generator.generate(analysis_results, output_format)

def save_report(report, output_path):
    """Save generated report to file."""
    from .utils.reporting import save_report_to_file
    save_report_to_file(report, output_path)

# Package metadata
__all__ = [
    # Core classes
    'Mesh',
    'TopologyAnalyzer',
    
    # I/O functions
    'load_mesh',
    'save_mesh',
    
    # Analysis functions
    'analyze_topology',
    'compute_euler_characteristic',
    'analyze_connectivity',
    'compute_surface_area',
    'compute_volume',
    'analyze_curvature',
    
    # Visualization
    'visualize_mesh',
    'create_figure',
    
    # Utilities
    'validate_mesh',
    'repair_mesh',
    'generate_report',
    'save_report',
]