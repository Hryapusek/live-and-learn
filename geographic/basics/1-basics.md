# Geodesy — Basics

## Geographic coordinates
- **Latitude (φ):** north/south of the equator, −90° to +90°.
- **Longitude (λ):** east/west of the prime meridian, −180° to +180°.
- **Parallels:** east–west, constant latitude.
- **Meridians:** north–south, constant longitude.
- Coordinates are **angles, not distances**.

## Approximate distances
For small displacements on Earth:

- 1° latitude ≈ 111.2 km.
- 1° longitude ≈ 111.32 × cos(latitude) km.
- At 60° latitude, 1° longitude ≈ 55.66 km.

Convert angular differences into local distances:

$$
N \approx 111.2 \cdot \Delta\varphi
$$

$$
E \approx 111.32 \cdot \cos(\varphi)\cdot\Delta\lambda
$$

$$
d \approx \sqrt{N^2+E^2}
$$

Here, angles are in degrees and distances in kilometers. Use these approximations for short distances.

## Navigation angles
- **Azimuth:** clockwise angle from north, 0–360°.
- **Bearing:** direction from one point to another.
- **Heading:** direction the vehicle's nose points.
- **Track:** direction the vehicle actually moves over the ground.

Azimuth directions: N = 0°, E = 90°, S = 180°, W = 270°.

For simplified horizontal camera rotation:

$$
A_{\text{camera}}=(A_{\text{heading}}+A_{\text{pan}})\bmod360^\circ
$$

**Important:** Earth is curved. Latitude/longitude cannot be treated directly as Cartesian X/Y coordinates. Flat-plane approximations work locally; longer distances require geodesic calculations.