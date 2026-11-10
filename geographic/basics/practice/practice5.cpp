#include <cmath>
#include <format>
#include <iostream>
#include <numbers>

#include <GeographicLib/LocalCartesian.hpp>

struct GeodeticPosition
{
  double latitude_deg;
  double longitude_deg;
  double ellipsoidal_height_m;
};

struct TargetMeasurements
{
  double horizontal_distance_m;
  double slant_distance_m;
  double azimuth_deg;
  double elevation_deg;
};

[[nodiscard]]
auto calculate_target_measurements(GeodeticPosition const& heli, GeodeticPosition const& target)
  -> TargetMeasurements
{
  GeographicLib::LocalCartesian local(heli.latitude_deg,
                                      heli.longitude_deg,
                                      heli.ellipsoidal_height_m);

  double east, north, up;

  local.Forward(target.latitude_deg,
                target.longitude_deg,
                target.ellipsoidal_height_m,
                east,
                north,
                up);

  TargetMeasurements measurements;
  measurements.elevation_deg = local.HeightOrigin();
  // measurements.azimuth_deg = 
}

int main() {}
