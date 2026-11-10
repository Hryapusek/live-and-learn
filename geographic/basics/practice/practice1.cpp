#include <cmath>
#include <format>
#include <iostream>
#include <numbers>

struct GeographicPosition
{
  double latitude_deg;
  double longitude_deg;
  double height_m;
};

struct Vec3
{
  double x;
  double y;
  double z;
};

// Source - https://stackoverflow.com/a/59914918
// Posted by vitaut, modified by community. See post 'Timeline' for change history
// Retrieved 2026-11-10, License - CC BY-SA 4.0

template <>
struct std::formatter<Vec3> : std::formatter<std::string> {
  auto format(Vec3 p, format_context& ctx) const {
    return formatter<string>::format(
      std::format("[{}, {}, {}]", p.x, p.y, p.z), ctx);
  }
};


constexpr auto eatrh_radius = 6371000.;

double len(Vec3 const& vec) {
  return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z);
}

[[nodiscard]]
auto geographic_to_ecef(GeographicPosition const& position) -> Vec3 {
  auto to_rad = [](auto deg) {
    return std::numbers::pi / 180 * deg;
  };
  return Vec3 {
    .x = (eatrh_radius + position.height_m) * std::cos(to_rad(position.latitude_deg)) * std::cos(to_rad(position.longitude_deg)),
    .y = (eatrh_radius + position.height_m) * std::cos(to_rad(position.latitude_deg)) * std::sin(to_rad(position.longitude_deg)),
    .z = (eatrh_radius + position.height_m) * std::sin(to_rad(position.latitude_deg))
  };
}

int main() {
  std::cout << std::format("Radius is {}", eatrh_radius) << std::endl;
  std::cout << std::format("Calculated point is {}", geographic_to_ecef({
    .latitude_deg = 0,
    .longitude_deg = 0,
    .height_m = 0
  })) << std::endl;
  std::cout << std::format("Calculated point is {}", geographic_to_ecef({
    .latitude_deg = 0,
    .longitude_deg = 90,
    .height_m = 0
  })) << std::endl;
  std::cout << std::format("Calculated point is {}", geographic_to_ecef({
    .latitude_deg = 90,
    .longitude_deg = 0,
    .height_m = 0
  })) << std::endl;
}
