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

Vec3 operator-(Vec3 left, Vec3 right)
{
  return Vec3 {.x = left.x - right.x, .y = left.y - right.y, .z = left.z - right.z};
}

template <>
struct std::formatter<Vec3> : std::formatter<std::string>
{
  auto format(Vec3 p, format_context& ctx) const
  {
    return formatter<string>::format(std::format("[{}, {}, {}]", p.x, p.y, p.z), ctx);
  }
};

constexpr auto eatrh_radius = 6371000.;
constexpr auto deg_to_rad = std::numbers::pi / 180;

double len(Vec3 const& vec) { return std::sqrt(vec.x * vec.x + vec.y * vec.y + vec.z * vec.z); }

[[nodiscard]]
auto geographic_to_ecef(GeographicPosition const& position) -> Vec3
{
  auto to_rad = [](auto deg) { return std::numbers::pi / 180 * deg; };
  return Vec3 {.x = (eatrh_radius + position.height_m) * std::cos(to_rad(position.latitude_deg))
                  * std::cos(to_rad(position.longitude_deg)),
               .y = (eatrh_radius + position.height_m) * std::cos(to_rad(position.latitude_deg))
                  * std::sin(to_rad(position.longitude_deg)),
               .z = (eatrh_radius + position.height_m) * std::sin(to_rad(position.latitude_deg))};
}

[[nodiscard]]
auto chord_distance(GeographicPosition const& a, GeographicPosition const& b) -> double
{
  auto a_vec = geographic_to_ecef(a);
  auto b_vec = geographic_to_ecef(b);

  return len(b_vec - a_vec);
}

// We use earth radius apporximation here and assume that objects are on the sphere for
// simplification.
[[nodiscard]]
auto surface_distance(GeographicPosition const& a, GeographicPosition const& b) -> double
{
  auto projected_a = a;
  projected_a.height_m = 0;

  auto projected_b = b;
  projected_b.height_m = 0;

  auto chord = chord_distance(projected_a, projected_b);
  auto central_angle = 2.0 * std::asin(chord / (2.0 * eatrh_radius));

  return eatrh_radius * central_angle;
}

[[nodiscard]]
auto dot(Vec3 const& a, Vec3 const& b) -> double
{
  return a.x * b.x + a.y * b.y + a.z * b.z;
}

// There are two ways of obtaining the angles between the two vectors. First is through the
// geometry, look at the first implementation. Second is through vectors dot project a * b = |a| *
// |b| * cos(sigma) = x1*x2+y1*y2+z1*z2
[[nodiscard]]
auto surface_distance_dot(GeographicPosition const& a, GeographicPosition const& b) -> double
{
  auto projected_a = a;
  projected_a.height_m = 0;

  auto projected_b = b;
  projected_b.height_m = 0;

  auto vec_a = geographic_to_ecef(projected_a);
  auto vec_b = geographic_to_ecef(projected_b);
  auto dot_product = dot(vec_a, vec_b);
  auto len_dot = len(vec_a) * len(vec_b);

  auto central_angle_rad = std::acos(std::clamp(dot_product / len_dot, -1.0, 1.0));

  return eatrh_radius * central_angle_rad;
}

[[nodiscard]]
auto east_unit_vector(double longitude_deg) -> Vec3
{
  return Vec3 {.x = -std::sin(longitude_deg * deg_to_rad),
               .y = std::cos(longitude_deg * deg_to_rad),
               .z = 0};
}

[[nodiscard]]
auto north_unit_vector(double lat_deg, double longitude_deg) -> Vec3
{
  return Vec3 {.x = -std::sin(lat_deg * deg_to_rad) * std::cos(longitude_deg * deg_to_rad),
               .y = -std::sin(lat_deg * deg_to_rad) * std::sin(longitude_deg * deg_to_rad),
               .z = std::cos(lat_deg * deg_to_rad)};
}

[[nodiscard]]
auto up_unit_vector(double lat_deg, double longitude_deg) -> Vec3
{
  return Vec3 {.x = std::cos(lat_deg * deg_to_rad) * std::cos(longitude_deg * deg_to_rad),
               .y = std::cos(lat_deg * deg_to_rad) * std::sin(longitude_deg * deg_to_rad),
               .z = std::sin(lat_deg * deg_to_rad)};
}

struct EnuVector
{
  double east;
  double north;
  double up;
};

[[nodiscard]]
auto geographic_to_enu(GeographicPosition const& origin, GeographicPosition const& target)
  -> EnuVector
{
  auto vec_origin = geographic_to_ecef(origin);
  auto vec_target = geographic_to_ecef(target);

  auto diff = vec_target - vec_origin;
  return EnuVector {.east = dot(diff, east_unit_vector(origin.longitude_deg)),
                    .north = dot(diff,
                                 north_unit_vector(origin.latitude_deg, origin.longitude_deg)),
                    .up = dot(diff, up_unit_vector(origin.latitude_deg, origin.longitude_deg))};
}

int main()
{
  std::cout << std::format("Radius is {}", eatrh_radius) << std::endl;
  std::cout << std::format("Calculated point is {}",
                           geographic_to_ecef(
                             {.latitude_deg = 0, .longitude_deg = 0, .height_m = 0}))
            << std::endl;
  std::cout << std::format("Calculated point is {}",
                           geographic_to_ecef(
                             {.latitude_deg = 0, .longitude_deg = 90, .height_m = 0}))
            << std::endl;
  std::cout << std::format("Calculated point is {}",
                           geographic_to_ecef(
                             {.latitude_deg = 90, .longitude_deg = 0, .height_m = 0}))
            << std::endl;

  GeographicPosition a {0.0, 0.0, 0.0};
  GeographicPosition b {0.0, 90.0, 0.0};

  std::cout << std::format("Surface arc is {}", chord_distance(a, b)) << std::endl;

  std::cout << std::format(
    "Chord:       {} m\n"
    "Surface:     {} m\n"
    "Surface dot: {} m\n",
    chord_distance(a, b),
    surface_distance(a, b),
    surface_distance_dot(a, b));

  GeographicPosition origin {0.0, 0.0, 0.0};
  GeographicPosition target {0.0, 1.0, 0.0};

  auto enu = geographic_to_enu(origin, target);

  std::cout << std::format(
    "East:  {} m\n"
    "North: {} m\n"
    "Up:    {} m\n",
    enu.east,
    enu.north,
    enu.up);
}
