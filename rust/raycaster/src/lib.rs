mod core;

#[cxx::bridge(namespace = "eqemu_map")]
mod ffi {
    struct Point {
        x: f32,
        y: f32,
        z: f32,
    }
    struct Bounds {
        present: bool,
        min: Point,
        max: Point,
    }
    struct Hit {
        present: bool,
        position: Point,
        normal: Point,
        distance: f32,
    }
    extern "Rust" {
        type Engine;
        fn create(vertices: &[Point], indices: &[u32]) -> Result<Box<Engine>>;
        fn bounds(self: &Engine) -> Bounds;
        fn brute_segment(self: &Engine, from: Point, to: Point) -> Result<Hit>;
        fn segment(self: &mut Engine, from: Point, to: Point) -> Result<Hit>;
    }
}
pub struct Engine(core::Raycaster);
fn point(v: core::V) -> ffi::Point {
    ffi::Point {
        x: v[0],
        y: v[1],
        z: v[2],
    }
}
fn vector(p: &ffi::Point) -> core::V {
    [p.x, p.y, p.z]
}
fn create(vertices: &[ffi::Point], indices: &[u32]) -> Result<Box<Engine>, core::Error> {
    if vertices.len() > 8_000_000 || indices.len() > 12_000_000 || indices.len() % 3 != 0 {
        return Err(core::Error("invalid geometry counts"));
    }
    let mut v = Vec::new();
    v.try_reserve_exact(vertices.len())
        .map_err(|_| core::Error("allocation limit"))?;
    v.extend(vertices.iter().map(vector));
    let mut i = Vec::new();
    i.try_reserve_exact(indices.len())
        .map_err(|_| core::Error("allocation limit"))?;
    i.extend_from_slice(indices);
    Ok(Box::new(Engine(core::Raycaster::build(v, i)?)))
}
impl Engine {
    fn bounds(&self) -> ffi::Bounds {
        match self.0.bounds() {
            Some(b) => ffi::Bounds {
                present: true,
                min: point(b.min),
                max: point(b.max),
            },
            None => ffi::Bounds {
                present: false,
                min: point([0.; 3]),
                max: point([0.; 3]),
            },
        }
    }
    fn brute_segment(&self, from: ffi::Point, to: ffi::Point) -> Result<ffi::Hit, core::Error> {
        match self.0.brute_segment(vector(&from), vector(&to))? {
            Some(h) => Ok(ffi::Hit {
                present: true,
                position: point(h.position),
                normal: point(h.normal),
                distance: h.distance,
            }),
            None => Ok(ffi::Hit {
                present: false,
                position: point([0.; 3]),
                normal: point([0.; 3]),
                distance: 0.,
            }),
        }
    }
    fn segment(&mut self, from: ffi::Point, to: ffi::Point) -> Result<ffi::Hit, core::Error> {
        match self.0.segment(vector(&from), vector(&to))? {
            Some(h) => Ok(ffi::Hit {
                present: true,
                position: point(h.position),
                normal: point(h.normal),
                distance: h.distance,
            }),
            None => Ok(ffi::Hit {
                present: false,
                position: point([0.; 3]),
                normal: point([0.; 3]),
                distance: 0.,
            }),
        }
    }
}
