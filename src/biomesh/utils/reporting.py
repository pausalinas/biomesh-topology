"""Report generation utilities."""

import json
import csv
import os
from typing import Dict, Any, Optional
from datetime import datetime
import warnings
import numpy as np

try:
    import pandas as pd
    PANDAS_AVAILABLE = True
except ImportError:
    PANDAS_AVAILABLE = False


class ReportGenerator:
    """Generate analysis reports in various formats."""
    
    def __init__(self):
        self.supported_formats = ['json', 'csv', 'html', 'markdown']
    
    def generate(self, analysis_results: Dict[str, Any], format: str = 'json') -> str:
        """
        Generate a report from analysis results.
        
        Parameters
        ----------
        analysis_results : Dict[str, Any]
            Results from mesh analysis
        format : str, default 'json'
            Output format ('json', 'csv', 'html', 'markdown')
            
        Returns
        -------
        str
            Generated report content
        """
        if format not in self.supported_formats:
            raise ValueError(f"Unsupported format: {format}. Supported: {self.supported_formats}")
        
        if format == 'json':
            return self._generate_json_report(analysis_results)
        elif format == 'csv':
            return self._generate_csv_report(analysis_results)
        elif format == 'html':
            return self._generate_html_report(analysis_results)
        elif format == 'markdown':
            return self._generate_markdown_report(analysis_results)
    
    def _generate_json_report(self, results: Dict[str, Any]) -> str:
        """Generate JSON report."""
        # Convert numpy types to native Python types for JSON serialization
        serializable_results = self._convert_numpy_types(results)
        return json.dumps(serializable_results, indent=2)
    
    def _generate_csv_report(self, results: Dict[str, Any]) -> str:
        """Generate CSV report."""
        # Flatten the results dictionary
        flattened = self._flatten_dict(results)
        
        # Create CSV content
        lines = ["Property,Value"]
        for key, value in flattened.items():
            # Handle special characters in CSV
            if isinstance(value, str) and (',' in value or '"' in value):
                value = f'"{value.replace('"', '""')}"'
            lines.append(f"{key},{value}")
        
        return '\n'.join(lines)
    
    def _generate_html_report(self, results: Dict[str, Any]) -> str:
        """Generate HTML report."""
        metadata = results.get('metadata', {})
        topology = results.get('topology', {})
        geometry = results.get('geometry', {})
        biological = results.get('biological', {})
        quality = results.get('quality', {})
        
        html = f"""
<!DOCTYPE html>
<html>
<head>
    <title>BiMesh Topology Analysis Report</title>
    <style>
        body {{ font-family: Arial, sans-serif; margin: 20px; }}
        .header {{ background-color: #f0f0f0; padding: 10px; border-radius: 5px; }}
        .section {{ margin: 20px 0; }}
        .subsection {{ margin-left: 20px; }}
        table {{ border-collapse: collapse; width: 100%; }}
        th, td {{ border: 1px solid #ddd; padding: 8px; text-align: left; }}
        th {{ background-color: #f2f2f2; }}
        .metric {{ margin: 5px 0; }}
    </style>
</head>
<body>
    <div class="header">
        <h1>BiMesh Topology Analysis Report</h1>
        <p>Generated on: {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}</p>
        <p>Analysis Duration: {metadata.get('analysis_duration', 'N/A'):.3f} seconds</p>
    </div>
    
    <div class="section">
        <h2>Mesh Overview</h2>
        <div class="metric">Vertices: {metadata.get('mesh_vertices', 'N/A')}</div>
        <div class="metric">Faces: {metadata.get('mesh_faces', 'N/A')}</div>
        <div class="metric">Edges: {topology.get('edges', 'N/A')}</div>
    </div>
    
    <div class="section">
        <h2>Topological Properties</h2>
        <div class="metric">Euler Characteristic: {topology.get('euler_characteristic', 'N/A')}</div>
        <div class="metric">Genus: {topology.get('genus', 'N/A')}</div>
        <div class="metric">Connected Components: {topology.get('connected_components', 'N/A')}</div>
        <div class="metric">Is Closed: {topology.get('is_closed', 'N/A')}</div>
        {f'<div class="metric">Boundary Edges: {topology.get("boundary_edges", "N/A")}</div>' if not topology.get('is_closed', True) else ''}
    </div>
    
    <div class="section">
        <h2>Geometric Properties</h2>
        <div class="metric">Surface Area: {geometry.get('surface_area', 'N/A'):.6f}</div>
        {f'<div class="metric">Volume: {geometry.get("volume", "N/A"):.6f}</div>' if 'volume' in geometry else ''}
        {f'<div class="metric">Surface Area to Volume Ratio: {geometry.get("surface_area_to_volume_ratio", "N/A"):.6f}</div>' if 'surface_area_to_volume_ratio' in geometry else ''}
        
        <div class="subsection">
            <h3>Face Area Statistics</h3>
            {self._generate_stats_table(geometry.get('face_area_stats', {}))}
        </div>
        
        <div class="subsection">
            <h3>Bounding Box</h3>
            {self._generate_bounding_box_table(geometry.get('bounding_box', {}))}
        </div>
    </div>
    
    {self._generate_biological_section(biological) if biological else ''}
    
    <div class="section">
        <h2>Mesh Quality</h2>
        <div class="metric">Degenerate Faces: {quality.get('degenerate_faces', 'N/A')}</div>
        
        <div class="subsection">
            <h3>Aspect Ratio Statistics</h3>
            {self._generate_stats_table(quality.get('aspect_ratio_stats', {}))}
        </div>
        
        <div class="subsection">
            <h3>Edge Length Statistics</h3>
            {self._generate_stats_table(quality.get('edge_length_stats', {}))}
        </div>
    </div>
</body>
</html>
"""
        return html
    
    def _generate_markdown_report(self, results: Dict[str, Any]) -> str:
        """Generate Markdown report."""
        metadata = results.get('metadata', {})
        topology = results.get('topology', {})
        geometry = results.get('geometry', {})
        biological = results.get('biological', {})
        quality = results.get('quality', {})
        
        markdown = f"""# BiMesh Topology Analysis Report

**Generated on:** {datetime.now().strftime('%Y-%m-%d %H:%M:%S')}  
**Analysis Duration:** {metadata.get('analysis_duration', 'N/A'):.3f} seconds

## Mesh Overview

- **Vertices:** {metadata.get('mesh_vertices', 'N/A')}
- **Faces:** {metadata.get('mesh_faces', 'N/A')}
- **Edges:** {topology.get('edges', 'N/A')}

## Topological Properties

- **Euler Characteristic:** {topology.get('euler_characteristic', 'N/A')}
- **Genus:** {topology.get('genus', 'N/A')}
- **Connected Components:** {topology.get('connected_components', 'N/A')}
- **Is Closed:** {topology.get('is_closed', 'N/A')}
{f"- **Boundary Edges:** {topology.get('boundary_edges', 'N/A')}" if not topology.get('is_closed', True) else ''}

## Geometric Properties

- **Surface Area:** {geometry.get('surface_area', 'N/A'):.6f}
{f"- **Volume:** {geometry.get('volume', 'N/A'):.6f}" if 'volume' in geometry else ''}
{f"- **Surface Area to Volume Ratio:** {geometry.get('surface_area_to_volume_ratio', 'N/A'):.6f}" if 'surface_area_to_volume_ratio' in geometry else ''}

### Face Area Statistics

{self._generate_stats_markdown_table(geometry.get('face_area_stats', {}))}

### Bounding Box

{self._generate_bounding_box_markdown_table(geometry.get('bounding_box', {}))}

{self._generate_biological_markdown_section(biological) if biological else ''}

## Mesh Quality

- **Degenerate Faces:** {quality.get('degenerate_faces', 'N/A')}

### Aspect Ratio Statistics

{self._generate_stats_markdown_table(quality.get('aspect_ratio_stats', {}))}

### Edge Length Statistics

{self._generate_stats_markdown_table(quality.get('edge_length_stats', {}))}
"""
        return markdown
    
    def _convert_numpy_types(self, obj: Any) -> Any:
        """Convert numpy types to native Python types recursively."""
        if isinstance(obj, np.integer):
            return int(obj)
        elif isinstance(obj, np.floating):
            return float(obj)
        elif isinstance(obj, np.ndarray):
            return obj.tolist()
        elif isinstance(obj, dict):
            return {key: self._convert_numpy_types(value) for key, value in obj.items()}
        elif isinstance(obj, list):
            return [self._convert_numpy_types(item) for item in obj]
        else:
            return obj
    
    def _flatten_dict(self, d: Dict[str, Any], parent_key: str = '', sep: str = '.') -> Dict[str, Any]:
        """Flatten a nested dictionary."""
        items = []
        for k, v in d.items():
            new_key = f"{parent_key}{sep}{k}" if parent_key else k
            if isinstance(v, dict):
                items.extend(self._flatten_dict(v, new_key, sep=sep).items())
            elif isinstance(v, list) and len(v) < 10:  # Avoid very long lists
                items.append((new_key, str(v)))
            else:
                items.append((new_key, v))
        return dict(items)
    
    def _generate_stats_table(self, stats: Dict[str, Any]) -> str:
        """Generate HTML table for statistics."""
        if not stats:
            return "<p>No statistics available</p>"
        
        rows = []
        for key, value in stats.items():
            if isinstance(value, float):
                value = f"{value:.6f}"
            rows.append(f"<tr><td>{key.title()}</td><td>{value}</td></tr>")
        
        return f"""
        <table>
            <tr><th>Statistic</th><th>Value</th></tr>
            {''.join(rows)}
        </table>
        """
    
    def _generate_bounding_box_table(self, bbox: Dict[str, Any]) -> str:
        """Generate HTML table for bounding box."""
        if not bbox:
            return "<p>No bounding box information available</p>"
        
        rows = []
        for key, value in bbox.items():
            if isinstance(value, list):
                value = f"[{', '.join(f'{v:.3f}' for v in value)}]"
            elif isinstance(value, float):
                value = f"{value:.6f}"
            rows.append(f"<tr><td>{key.replace('_', ' ').title()}</td><td>{value}</td></tr>")
        
        return f"""
        <table>
            <tr><th>Property</th><th>Value</th></tr>
            {''.join(rows)}
        </table>
        """
    
    def _generate_biological_section(self, biological: Dict[str, Any]) -> str:
        """Generate HTML for biological properties section."""
        if not biological:
            return ""
        
        section = '<div class="section"><h2>Biological Properties</h2>'
        
        # Add biological-specific metrics
        for key, value in biological.items():
            if isinstance(value, dict):
                section += f'<div class="subsection"><h3>{key.replace("_", " ").title()}</h3>'
                section += self._generate_stats_table(value)
                section += '</div>'
            else:
                if isinstance(value, float):
                    value = f"{value:.6f}"
                section += f'<div class="metric">{key.replace("_", " ").title()}: {value}</div>'
        
        section += '</div>'
        return section
    
    def _generate_stats_markdown_table(self, stats: Dict[str, Any]) -> str:
        """Generate Markdown table for statistics."""
        if not stats:
            return "No statistics available"
        
        rows = ["| Statistic | Value |", "|-----------|-------|"]
        for key, value in stats.items():
            if isinstance(value, float):
                value = f"{value:.6f}"
            rows.append(f"| {key.title()} | {value} |")
        
        return '\n'.join(rows)
    
    def _generate_bounding_box_markdown_table(self, bbox: Dict[str, Any]) -> str:
        """Generate Markdown table for bounding box."""
        if not bbox:
            return "No bounding box information available"
        
        rows = ["| Property | Value |", "|----------|-------|"]
        for key, value in bbox.items():
            if isinstance(value, list):
                value = f"[{', '.join(f'{v:.3f}' for v in value)}]"
            elif isinstance(value, float):
                value = f"{value:.6f}"
            rows.append(f"| {key.replace('_', ' ').title()} | {value} |")
        
        return '\n'.join(rows)
    
    def _generate_biological_markdown_section(self, biological: Dict[str, Any]) -> str:
        """Generate Markdown for biological properties section."""
        if not biological:
            return ""
        
        section = "## Biological Properties\n\n"
        
        for key, value in biological.items():
            if isinstance(value, dict):
                section += f"### {key.replace('_', ' ').title()}\n\n"
                section += self._generate_stats_markdown_table(value) + "\n\n"
            else:
                if isinstance(value, float):
                    value = f"{value:.6f}"
                section += f"- **{key.replace('_', ' ').title()}:** {value}\n"
        
        return section


def save_report_to_file(report_content: str, output_path: str) -> None:
    """
    Save report content to file.
    
    Parameters
    ----------
    report_content : str
        Generated report content
    output_path : str
        Path to save the report
    """
    dir_name = os.path.dirname(output_path)
    if dir_name:  # Only create directory if it's not empty
        os.makedirs(dir_name, exist_ok=True)
    
    with open(output_path, 'w', encoding='utf-8') as f:
        f.write(report_content)


def export_to_dataframe(results: Dict[str, Any]) -> Optional['pd.DataFrame']:
    """
    Export analysis results to pandas DataFrame.
    
    Parameters
    ----------
    results : Dict[str, Any]
        Analysis results
        
    Returns
    -------
    pd.DataFrame or None
        DataFrame with flattened results, None if pandas not available
    """
    if not PANDAS_AVAILABLE:
        warnings.warn("Pandas not available. Cannot export to DataFrame.")
        return None
    
    # Flatten the results
    generator = ReportGenerator()
    flattened = generator._flatten_dict(results)
    
    # Create DataFrame
    df = pd.DataFrame(list(flattened.items()), columns=['Property', 'Value'])
    return df