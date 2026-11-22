//! Maintained raycaster. Numerical operations follow zone/raycast_mesh.cpp.
#![forbid(unsafe_code)]
pub type V = [f32; 3];
#[derive(Debug, Clone, PartialEq)]
pub struct Error(pub &'static str);
impl std::fmt::Display for Error {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        f.write_str(self.0)
    }
}
impl std::error::Error for Error {}
type Result<T> = std::result::Result<T, Error>;
fn reserve<T>(v: &mut Vec<T>, n: usize) -> Result<()> {
    v.try_reserve(n).map_err(|_| Error("allocation limit"))
}
fn push<T>(v: &mut Vec<T>, x: T) -> Result<()> {
    reserve(v, 1)?;
    v.push(x);
    Ok(())
}
pub fn sub(a: V, b: V) -> V {
    [a[0] - b[0], a[1] - b[1], a[2] - b[2]]
}
pub fn dot(a: V, b: V) -> f32 {
    a[0] * b[0] + a[1] * b[1] + a[2] * b[2]
}
pub fn cross(a: V, b: V) -> V {
    [
        a[1] * b[2] - b[1] * a[2],
        a[2] * b[0] - b[2] * a[0],
        a[0] * b[1] - b[0] * a[1],
    ]
}
fn scale(a: V, s: f32) -> V {
    [a[0] * s, a[1] * s, a[2] * s]
}
fn finite(a: V) -> bool {
    a.iter().all(|x| x.is_finite() && x.abs() <= 1e8)
}
#[derive(Clone, Copy, Debug)]
pub struct Bounds {
    pub min: V,
    pub max: V,
}
impl Bounds {
    fn point(p: V) -> Self {
        Self { min: p, max: p }
    }
    fn include(&mut self, p: V) {
        for k in 0..3 {
            if p[k] < self.min[k] {
                self.min[k] = p[k];
            }
            if p[k] > self.max[k] {
                self.max[k] = p[k];
            }
        }
    }
    fn clip(&self, p: V) -> u32 {
        let mut r = 0;
        for k in 0..3 {
            if p[k] < self.min[k] {
                r |= 1 << (2 * k);
            } else if p[k] > self.max[k] {
                r |= 2 << (2 * k);
            }
        }
        r
    }
    fn clamp(&mut self, b: Self) {
        for k in 0..3 {
            if self.min[k] < b.min[k] {
                self.min[k] = b.min[k];
            }
            if self.max[k] > b.max[k] {
                self.max[k] = b.max[k];
            }
        }
    }
}
// Preserve signed-bit tests, epsilon expansion, plane selection and strict segment-box distance.
fn box_hit(b: Bounds, p: V, d: V, dist: f32) -> bool {
    if dist <= 1e-5 {
        return false;
    }
    let mut inside = true;
    let mut mt = [-1.; 3];
    let mut coord = [0.; 3];
    for k in 0..3 {
        if p[k] < b.min[k] {
            coord[k] = b.min[k];
            inside = false;
            if d[k].to_bits() != 0 {
                mt[k] = (b.min[k] - p[k]) / d[k];
            }
        } else if p[k] > b.max[k] {
            coord[k] = b.max[k];
            inside = false;
            if d[k].to_bits() != 0 {
                mt[k] = (b.max[k] - p[k]) / d[k];
            }
        }
    }
    if inside {
        return 0. < dist * dist;
    }
    let mut plane = 0;
    if mt[1] > mt[plane] {
        plane = 1;
    }
    if mt[2] > mt[plane] {
        plane = 2;
    }
    if mt[plane].to_bits() & 0x80000000 != 0 {
        return false;
    }
    for k in 0..3 {
        if k != plane {
            coord[k] = p[k] + mt[plane] * d[k];
            if coord[k] < b.min[k] - 1e-5 || coord[k] > b.max[k] + 1e-5 {
                return false;
            }
        }
    }
    let delta = sub(p, coord);
    dot(delta, delta) < dist * dist
}
fn triangle(p: V, d: V, [a, b, c]: [V; 3]) -> Option<f32> {
    let e1 = sub(b, a);
    let e2 = sub(c, a);
    let h = cross(d, e2);
    let det = dot(e1, h);
    // C++ unsuffixed decimal threshold is double, even though det is float.
    if (det as f64) > -0.00001 && (det as f64) < 0.00001 {
        return None;
    }
    let f = 1. / det;
    let s = sub(p, a);
    let u = f * dot(s, h);
    if u < 0. || u > 1. {
        return None;
    }
    let q = cross(s, e1);
    let v = f * dot(d, q);
    if v < 0. || u + v > 1. {
        return None;
    }
    let t = f * dot(e2, q);
    if t > 0. { Some(t) } else { None }
}
fn normal([a, b, c]: [V; 3]) -> V {
    let n = cross(sub(b, a), sub(c, b));
    // computePlane uses sqrt(double) after the float sum, then narrows to float.
    let mag = (dot(n, n) as f64).sqrt() as f32;
    scale(n, if mag < 1e-6 { 0. } else { 1. / mag })
}
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Hit {
    pub position: V,
    pub normal: V,
    pub distance: f32,
    pub triangle: usize,
}
struct Node {
    bounds: Bounds,
    children: [Option<usize>; 2],
    start: usize,
    count: usize,
}
pub struct Raycaster {
    vertices: Vec<V>,
    indices: Vec<u32>,
    nodes: Vec<Node>,
    leaves: Vec<usize>,
    visited: Vec<u32>,
    frame: u32,
}
impl Raycaster {
    /// Empty arrays build an empty engine. Coordinates and queries are bounded to +/-1e8.
    pub fn build(vertices: Vec<V>, indices: Vec<u32>) -> Result<Self> {
        Self::build_admitted(vertices, indices, 8_000_000, 12_000_000)
    }
    // Count limits bound owned geometry to 144 MB before tree/workspace overhead.
    fn build_admitted(vertices: Vec<V>, indices: Vec<u32>, nv: usize, ni: usize) -> Result<Self> {
        if vertices.len() > nv || indices.len() > ni || indices.len() % 3 != 0 {
            return Err(Error("invalid geometry counts"));
        }
        if vertices.iter().any(|&v| !finite(v)) {
            return Err(Error("nonfinite or out-of-bound vertex"));
        }
        if indices.iter().any(|&i| i as usize >= vertices.len()) {
            return Err(Error("index out of bounds"));
        }
        let nt = indices.len() / 3;
        let mut visited = Vec::new();
        reserve(&mut visited, nt)?;
        visited.resize(nt, 0);
        let mut s = Self {
            vertices,
            indices,
            nodes: Vec::new(),
            leaves: Vec::new(),
            visited,
            frame: 0,
        };
        if nt > 0 {
            let mut b = Bounds::point(s.vertices[0]);
            for &p in &s.vertices {
                b.include(p);
            }
            let mut ids = Vec::new();
            reserve(&mut ids, nt)?;
            ids.extend(0..nt);
            s.split(b, ids, 0)?;
        }
        Ok(s)
    }
    pub fn bounds(&self) -> Option<Bounds> {
        self.nodes.first().map(|n| n.bounds)
    }
    fn tri(&self, t: usize) -> [V; 3] {
        let i = &self.indices[3 * t..3 * t + 3];
        [
            self.vertices[i[0] as usize],
            self.vertices[i[1] as usize],
            self.vertices[i[2] as usize],
        ]
    }
    fn split(&mut self, b: Bounds, ids: Vec<usize>, depth: usize) -> Result<usize> {
        if self.nodes.len() >= 131071 {
            return Err(Error("node limit"));
        }
        let index = self.nodes.len();
        push(
            &mut self.nodes,
            Node {
                bounds: b,
                children: [None; 2],
                start: 0,
                count: 0,
            },
        )?;
        let dim = sub(b.max, b.min);
        let mut axis = 0;
        if dim[1] > dim[0] {
            axis = 1;
        }
        if dim[2] > dim[0] && dim[2] > dim[1] {
            axis = 2;
        }
        if ids.len() < 4 || depth >= 15 || dim[axis] < 0.01 {
            if ids.len() > 16_000_000usize.saturating_sub(self.leaves.len()) {
                return Err(Error("leaf reference limit"));
            }
            self.nodes[index].start = self.leaves.len();
            self.nodes[index].count = ids.len();
            reserve(&mut self.leaves, ids.len())?;
            self.leaves.extend(ids);
            return Ok(index);
        }
        let mid = (b.min[axis] + b.max[axis]) * 0.5;
        let mut halves = [b, b];
        halves[0].max[axis] = mid;
        halves[1].min[axis] = mid;
        let mut lists = [Vec::new(), Vec::new()];
        let mut bounds = [None, None];
        for t in ids {
            let points = self.tri(t);
            let mut codes = [0; 3];
            for k in 0..3 {
                codes[k] = halves[0].clip(points[k]);
            }
            for side in 0..2 {
                if side == 1 && codes.iter().fold(0, |a, c| a | c) == 0 {
                    continue;
                }
                let c = points.map(|p| halves[side].clip(p));
                if c[0] & c[1] & c[2] != 0 {
                    continue;
                }
                let bb = bounds[side].get_or_insert(Bounds::point(points[0]));
                for p in points {
                    bb.include(p);
                }
                push(&mut lists[side], t)?;
            }
        }
        for (side, list) in lists.into_iter().enumerate() {
            if let Some(mut bb) = bounds[side] {
                bb.clamp(halves[side]);
                self.nodes[index].children[side] = Some(self.split(bb, list, depth + 1)?);
            }
        }
        Ok(index)
    }
    /// Mutable access owns only visited stamps. Wrap clears all stamps before reusing frame 1.
    pub fn segment(&mut self, from: V, to: V) -> Result<Option<Hit>> {
        if !finite(from) || !finite(to) {
            return Err(Error("nonfinite or out-of-bound query"));
        }
        let delta = sub(to, from);
        let mut distance = dot(delta, delta).sqrt();
        if distance < 1e-10 || self.nodes.is_empty() {
            return Ok(None);
        }
        let dir = scale(delta, 1. / distance);
        if self.frame == u32::MAX {
            self.visited.fill(0);
            self.frame = 0;
        }
        self.frame += 1;
        let mut best = None;
        self.walk(0, from, dir, &mut distance, &mut best);
        Ok(best)
    }
    /// Diagnostic linear traversal preserves native strict endpoint and first-index ties.
    pub fn brute_segment(&self, from: V, to: V) -> Result<Option<Hit>> {
        if !finite(from) || !finite(to) {
            return Err(Error("nonfinite or out-of-bound query"));
        }
        let delta = sub(to, from);
        let mut nearest = dot(delta, delta).sqrt();
        if nearest < 1e-10 {
            return Ok(None);
        }
        let dir = scale(delta, 1. / nearest);
        let mut best = None;
        for t in 0..self.indices.len() / 3 {
            if let Some(dist) = triangle(from, dir, self.tri(t)) {
                if dist < nearest {
                    nearest = dist;
                    best = Some(Hit {
                        position: [
                            from[0] + dir[0] * dist,
                            from[1] + dir[1] * dist,
                            from[2] + dir[2] * dist,
                        ],
                        normal: normal(self.tri(t)),
                        distance: dist,
                        triangle: t,
                    });
                }
            }
        }
        Ok(best)
    }
    fn walk(&mut self, index: usize, p: V, d: V, nearest: &mut f32, best: &mut Option<Hit>) {
        let n = &self.nodes[index];
        if !box_hit(n.bounds, p, d, *nearest) {
            return;
        }
        let (children, start, count) = (n.children, n.start, n.count);
        if children == [None, None] {
            for i in start..start + count {
                let t = self.leaves[i];
                if self.visited[t] == self.frame {
                    continue;
                }
                self.visited[t] = self.frame;
                if let Some(dist) = triangle(p, d, self.tri(t)) {
                    if dist < *nearest
                        || (dist == *nearest && best.as_ref().map_or(true, |h| t < h.triangle))
                    {
                        *nearest = dist;
                        *best = Some(Hit {
                            position: [p[0] + d[0] * dist, p[1] + d[1] * dist, p[2] + d[2] * dist],
                            normal: normal(self.tri(t)),
                            distance: dist,
                            triangle: t,
                        });
                    }
                }
            }
        } else {
            for child in children.into_iter().flatten() {
                self.walk(child, p, d, nearest, best);
            }
        }
    }
}

#[cfg(test)]
#[path = "tests.rs"]
mod tests;
