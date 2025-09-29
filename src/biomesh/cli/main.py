"""Main CLI entry point for BiMesh Topology."""

import click
import json
import os
import sys
from pathlib import Path
from typing import List

import biomesh
from ..io.loader import supported_formats


@click.group()
@click.version_option(version=biomesh.__version__)
def main():
    """BiMesh Topology - Biological mesh topology analysis tool."""
    pass


@main.command()
@click.argument('input_file', type=click.Path(exists=True))
@click.option('--output', '-o', type=click.Path(), help='Output file path')
@click.option('--format', '-f', type=click.Choice(['json', 'csv']), default='json',
              help='Output format')
@click.option('--verbose', '-v', is_flag=True, help='Verbose output')
@click.option('--curvature/--no-curvature', default=True, 
              help='Compute curvature analysis')
def analyze(input_file, output, format, verbose, curvature):
    """Analyze a single mesh file."""
    try:
        if verbose:
            click.echo(f"Loading mesh from {input_file}")
        
        # Load mesh
        mesh = biomesh.load_mesh(input_file)
        
        if verbose:
            click.echo(f"Loaded mesh: {mesh.n_vertices} vertices, {mesh.n_faces} faces")
        
        # Perform analysis
        def progress_callback(message, progress):
            if verbose:
                click.echo(f"[{progress:3d}%] {message}")
        
        analyzer = biomesh.TopologyAnalyzer(progress_callback if verbose else None)
        results = analyzer.analyze(mesh, compute_curvature=curvature)
        
        # Output results
        if output:
            if format == 'json':
                with open(output, 'w') as f:
                    json.dump(results, f, indent=2)
            elif format == 'csv':
                _save_results_csv(results, output)
            
            if verbose:
                click.echo(f"Results saved to {output}")
        else:
            # Print to stdout
            if format == 'json':
                click.echo(json.dumps(results, indent=2))
            else:
                _print_results_summary(results)
    
    except Exception as e:
        click.echo(f"Error: {e}", err=True)
        sys.exit(1)


@main.command()
@click.argument('input_dir', type=click.Path(exists=True, file_okay=False))
@click.option('--output-dir', '-o', type=click.Path(), required=True,
              help='Output directory')
@click.option('--pattern', '-p', default='*', help='File pattern to match')
@click.option('--format', '-f', type=click.Choice(['json', 'csv']), default='json',
              help='Output format')
@click.option('--verbose', '-v', is_flag=True, help='Verbose output')
def batch(input_dir, output_dir, pattern, format, verbose):
    """Batch process multiple mesh files."""
    try:
        input_path = Path(input_dir)
        output_path = Path(output_dir)
        output_path.mkdir(parents=True, exist_ok=True)
        
        # Find mesh files
        mesh_files = []
        formats = supported_formats()
        supported_exts = list(formats.keys())
        
        for ext in supported_exts:
            mesh_files.extend(input_path.glob(f"{pattern}{ext}"))
        
        if not mesh_files:
            click.echo("No mesh files found matching the pattern")
            return
        
        if verbose:
            click.echo(f"Found {len(mesh_files)} mesh files to process")
        
        # Process each file
        for i, mesh_file in enumerate(mesh_files):
            if verbose:
                click.echo(f"Processing [{i+1}/{len(mesh_files)}]: {mesh_file.name}")
            
            try:
                # Load and analyze mesh
                mesh = biomesh.load_mesh(str(mesh_file))
                results = biomesh.analyze_topology(mesh)
                
                # Save results
                output_file = output_path / f"{mesh_file.stem}_analysis.{format}"
                if format == 'json':
                    with open(output_file, 'w') as f:
                        json.dump(results, f, indent=2)
                elif format == 'csv':
                    _save_results_csv(results, output_file)
            
            except Exception as e:
                click.echo(f"Error processing {mesh_file.name}: {e}", err=True)
        
        if verbose:
            click.echo("Batch processing complete")
    
    except Exception as e:
        click.echo(f"Error: {e}", err=True)
        sys.exit(1)


@main.command()
@click.argument('mesh_file', type=click.Path(exists=True))
@click.option('--property', '-p', type=click.Choice(['curvature', 'area', 'normal']),
              default='curvature', help='Property to visualize')
@click.option('--output', '-o', type=click.Path(), help='Save visualization to file')
@click.option('--show/--no-show', default=True, help='Show interactive visualization')
def visualize(mesh_file, property, output, show):
    """Visualize mesh properties."""
    try:
        # Load mesh
        mesh = biomesh.load_mesh(mesh_file)
        
        # Create visualization
        figure = biomesh.visualize_mesh(mesh, color_by=property, show=show)
        
        if output:
            figure.save(output)
            click.echo(f"Visualization saved to {output}")
    
    except Exception as e:
        click.echo(f"Error: {e}", err=True)
        sys.exit(1)


@main.command()
@click.argument('mesh1', type=click.Path(exists=True))
@click.argument('mesh2', type=click.Path(exists=True))
@click.option('--metrics', '-m', 
              type=click.Choice(['area', 'volume', 'genus', 'euler', 'all']),
              multiple=True, default=['all'], help='Metrics to compare')
@click.option('--output', '-o', type=click.Path(), help='Output file path')
def compare(mesh1, mesh2, metrics, output):
    """Compare two meshes."""
    try:
        # Load meshes
        mesh_a = biomesh.load_mesh(mesh1)
        mesh_b = biomesh.load_mesh(mesh2)
        
        # Analyze both meshes
        results_a = biomesh.analyze_topology(mesh_a)
        results_b = biomesh.analyze_topology(mesh_b)
        
        # Compare metrics
        comparison = _compare_meshes(results_a, results_b, metrics)
        
        if output:
            with open(output, 'w') as f:
                json.dump(comparison, f, indent=2)
        else:
            click.echo(json.dumps(comparison, indent=2))
    
    except Exception as e:
        click.echo(f"Error: {e}", err=True)
        sys.exit(1)


@main.command()
def formats():
    """List supported file formats."""
    formats_dict = supported_formats()
    
    click.echo("Supported file formats:")
    for ext, description in formats_dict.items():
        click.echo(f"  {ext:<6} {description}")


def _print_results_summary(results):
    """Print a summary of analysis results."""
    metadata = results.get('metadata', {})
    topology = results.get('topology', {})
    geometry = results.get('geometry', {})
    
    click.echo("=== Mesh Analysis Summary ===")
    click.echo(f"Vertices: {metadata.get('mesh_vertices', 'N/A')}")
    click.echo(f"Faces: {metadata.get('mesh_faces', 'N/A')}")
    click.echo(f"Euler characteristic: {topology.get('euler_characteristic', 'N/A')}")
    
    if topology.get('is_closed'):
        click.echo(f"Genus: {topology.get('genus', 'N/A')}")
        click.echo(f"Volume: {geometry.get('volume', 'N/A'):.6f}")
    else:
        click.echo("Mesh is not closed")
        click.echo(f"Boundary edges: {topology.get('boundary_edges', 'N/A')}")
    
    click.echo(f"Surface area: {geometry.get('surface_area', 'N/A'):.6f}")
    click.echo(f"Connected components: {topology.get('connected_components', 'N/A')}")


def _save_results_csv(results, output_file):
    """Save results in CSV format."""
    import csv
    
    # Flatten the results dictionary
    flattened = _flatten_dict(results)
    
    with open(output_file, 'w', newline='') as f:
        writer = csv.writer(f)
        writer.writerow(['Property', 'Value'])
        for key, value in flattened.items():
            writer.writerow([key, value])


def _flatten_dict(d, parent_key='', sep='.'):
    """Flatten a nested dictionary."""
    items = []
    for k, v in d.items():
        new_key = f"{parent_key}{sep}{k}" if parent_key else k
        if isinstance(v, dict):
            items.extend(_flatten_dict(v, new_key, sep=sep).items())
        else:
            items.append((new_key, v))
    return dict(items)


def _compare_meshes(results_a, results_b, metrics):
    """Compare two mesh analysis results."""
    comparison = {
        'mesh_a': results_a['metadata'],
        'mesh_b': results_b['metadata'],
        'differences': {}
    }
    
    if 'all' in metrics:
        metrics = ['area', 'volume', 'genus', 'euler']
    
    for metric in metrics:
        if metric == 'area':
            val_a = results_a['geometry'].get('surface_area')
            val_b = results_b['geometry'].get('surface_area')
        elif metric == 'volume':
            val_a = results_a['geometry'].get('volume')
            val_b = results_b['geometry'].get('volume')
        elif metric == 'genus':
            val_a = results_a['topology'].get('genus')
            val_b = results_b['topology'].get('genus')
        elif metric == 'euler':
            val_a = results_a['topology'].get('euler_characteristic')
            val_b = results_b['topology'].get('euler_characteristic')
        else:
            continue
        
        if val_a is not None and val_b is not None:
            comparison['differences'][metric] = {
                'mesh_a': val_a,
                'mesh_b': val_b,
                'difference': val_b - val_a,
                'relative_difference': (val_b - val_a) / val_a if val_a != 0 else float('inf')
            }
    
    return comparison


if __name__ == '__main__':
    main()