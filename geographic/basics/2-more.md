# Geodesy — Part 2

## 1. atan2 and vector directions

Ordinary `atan(y / x)` loses quadrant information because dividing coordinates loses their individual signs.

- `atan(y / x)` → −90° to +90°
- `atan2(y, x)` → −180° to +180°, preserves quadrant

For navigational azimuth (clockwise from north):

$$
A=\operatorname{atan2}(E,N)
$$

Normalize negative angles by adding 360°.

## 2. Orthodromy and loxodromy

- **Orthodromy:** shortest surface path on a sphere (great-circle arc). Azimuth generally changes.
- **Loxodromy:** constant azimuth (rhumb line). Usually longer.

Great-circle distance:

$$
d=R\sigma
$$

Here \(\sigma\) is the central angle between the points, in radians.

## 3. ECEF — Earth-Centered, Earth-Fixed

Cartesian coordinate system with origin at Earth's center.

- X → equator, longitude 0°
- Y → equator, longitude 90° E
- Z → North Pole

For a spherical Earth:

$$
\begin{aligned}
x&=(R+h)\cos\varphi\cos\lambda\\
y&=(R+h)\cos\varphi\sin\lambda\\
z&=(R+h)\sin\varphi
\end{aligned}
$$

Where φ = latitude, λ = longitude, h = height above the reference sphere.

Straight-line distance between two points:

$$
d=\|\vec P_2-\vec P_1\|
$$

This is the **chord distance**, not the distance along Earth's surface.

## 4. ENU and NED — Local coordinates

**ENU:** East, North, Up. Local Cartesian coordinate system attached to a reference location.

**NED:** North, East, Down. Common in aerospace.

ECEF → ENU:

1. Subtract ECEF positions to obtain displacement.
2. Project displacement onto local East, North, Up unit vectors.

The dot product with a unit vector gives the **signed component along that direction**.

For ENU displacement (E, N, U):

$$
\begin{aligned}
d_h&=\sqrt{E^2+N^2}\\
A&=\operatorname{atan2}(E,N)\\
\alpha&=\operatorname{atan2}(U,d_h)\\
d&=\sqrt{E^2+N^2+U^2}
\end{aligned}
$$

Here A is azimuth, α is elevation angle, and d is straight-line distance.

## 5. Earth's shape and altitude

Earth is approximated more accurately by an **oblate ellipsoid**, not a sphere.

**WGS84** reference ellipsoid:
- Equatorial radius: 6,378.137 km
- Polar radius: ≈ 6,356.752 km

Three important surfaces:

- **Ellipsoid:** smooth mathematical reference for coordinates.
- **Geoid:** gravity-based surface approximating global mean sea level.
- **Terrain:** actual ground surface.

Three height concepts:

- **h:** ellipsoidal height.
- **H:** orthometric height (relative to geoid).
- **AGL:** height above ground level.

Geoid undulation N:

$$
h\approx H+N
$$

Approximate height above ground:

$$
AGL\approx H_{\text{aircraft}}-H_{\text{terrain}}
$$

Both elevations must use compatible vertical datums.

## 6. Digital Elevation Model (DEM)

A DEM stores terrain elevation at geographic locations, often as a regular grid.

Used for:
- Terrain elevation queries
- Height above ground
- Line-of-sight and target calculations

**Important:** Always check the DEM's coordinate system, vertical datum, and resolution.

---

**Remember:** Latitude/longitude describe horizontal geographic position; accurate 3D positioning also needs height and its reference. The spherical model is useful for learning, but real geodetic calculations use an ellipsoid.