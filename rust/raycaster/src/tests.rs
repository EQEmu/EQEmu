use super::*;
fn mesh() -> Raycaster {
    Raycaster::build(
        vec![[0., 0., 0.], [4., 0., 0.], [0., 4., 0.]],
        vec![0, 1, 2],
    )
    .unwrap()
}
#[test]
fn analytic_and_lifecycle() {
    for _ in 0..64 {
        let mut m = mesh();
        for (a, b, expected) in [
            ([1., 1., 2.], [1., 1., -2.], true),
            ([1., 1., -2.], [1., 1., 2.], true),
            ([5., 5., 2.], [5., 5., -2.], false),
            ([1., 1., 0.], [1., 1., 2.], false),
            ([1., 1., 0.], [1., 1., 0.], false),
            ([1., 1., 1e-6], [1., 1., -1e-6], false),
            ([1., 1., 2.], [1., 1., 0.], false),
        ] {
            let h = m.segment(a, b).unwrap();
            assert_eq!(h.is_some(), expected);
            if let Some(h) = h {
                assert_eq!(h.position, [1., 1., 0.]);
                assert_eq!(h.normal, [0., 0., 1.]);
                assert_eq!(h.distance, 2.);
            }
        }
    }
}
#[test]
fn seam_edges_and_winding() {
    let mut m = Raycaster::build(
        vec![[0., 0., 0.], [4., 0., 0.], [4., 4., 0.], [0., 4., 0.]],
        vec![0, 1, 2, 0, 2, 3],
    )
    .unwrap();
    for x in [
        f32::from_bits(2f32.to_bits() - 1),
        2.,
        f32::from_bits(2f32.to_bits() + 1),
        0.,
        4.,
    ] {
        assert!(m.segment([x, 2., 2.], [x, 2., -2.]).unwrap().is_some());
    }
    assert!(
        m.segment([-f32::EPSILON, 2., 2.], [-f32::EPSILON, 2., -2.])
            .unwrap()
            .is_none()
    );
    let mut rev = Raycaster::build(
        vec![[0., 0., 0.], [4., 0., 0.], [0., 4., 0.]],
        vec![0, 2, 1],
    )
    .unwrap();
    assert_eq!(
        rev.segment([1., 1., 2.], [1., 1., -2.])
            .unwrap()
            .unwrap()
            .normal,
        [0., 0., -1.]
    );
}
#[test]
fn overlap_tie_and_wrap() {
    let mut m = Raycaster::build(
        vec![[0., 0., 0.], [4., 0., 0.], [0., 4., 0.]],
        vec![0, 2, 1, 0, 1, 2],
    )
    .unwrap();
    for _ in 0..3 {
        m.frame = u32::MAX;
        m.visited.fill(1);
        let h = m.segment([1., 1., 2.], [1., 1., -2.]).unwrap().unwrap();
        assert_eq!(h.triangle, 0);
        assert_eq!(h.normal, [0., 0., -1.]);
    }
}
#[test]
fn invalid_and_empty() {
    assert!(Raycaster::build(vec![], vec![0, 0, 0]).is_err());
    assert!(Raycaster::build(vec![[0.; 3]], vec![0]).is_err());
    assert!(Raycaster::build(vec![[f32::NAN; 3]], vec![]).is_err());
    assert!(Raycaster::build(vec![[1e9; 3]], vec![]).is_err());
    let mut empty = Raycaster::build(vec![], vec![]).unwrap();
    assert!(empty.bounds().is_none());
    assert!(empty.segment([0.; 3], [1.; 3]).unwrap().is_none());
    assert!(empty.segment([f32::INFINITY; 3], [0.; 3]).is_err());
    let mut d = Raycaster::build(vec![[0.; 3], [1.; 3], [2.; 3]], vec![0, 1, 2]).unwrap();
    assert!(d.segment([1., 1., 2.], [1., 1., 0.]).unwrap().is_none());
}
// Independently written f64 Moller-Trumbore reference. No production triangle/tree calls.
fn oracle(a: V, b: V, vertices: &[V], indices: &[u32]) -> Option<(usize, f64)> {
    let a = a.map(f64::from);
    let b = b.map(f64::from);
    let d = std::array::from_fn::<_, 3, _>(|k| b[k] - a[k]);
    let sub = |x: [f64; 3], y: [f64; 3]| std::array::from_fn::<_, 3, _>(|k| x[k] - y[k]);
    let dot = |x: [f64; 3], y: [f64; 3]| x[0] * y[0] + x[1] * y[1] + x[2] * y[2];
    let cross = |x: [f64; 3], y: [f64; 3]| {
        [
            x[1] * y[2] - x[2] * y[1],
            x[2] * y[0] - x[0] * y[2],
            x[0] * y[1] - x[1] * y[0],
        ]
    };
    let mut best = None;
    let mut nearest = 1.;
    for (t, ids) in indices.chunks_exact(3).enumerate() {
        let p = vertices[ids[0] as usize].map(f64::from);
        let e = sub(vertices[ids[1] as usize].map(f64::from), p);
        let f = sub(vertices[ids[2] as usize].map(f64::from), p);
        let h = cross(d, f);
        let det = dot(e, h);
        if det.abs() < 1e-12 {
            continue;
        }
        let s = sub(a, p);
        let u = dot(s, h) / det;
        if !(0. ..=1.).contains(&u) {
            continue;
        }
        let q = cross(s, e);
        let v = dot(d, q) / det;
        if v < 0. || u + v > 1. {
            continue;
        }
        let distance = dot(f, q) / det;
        if distance > 0. && (distance < nearest || distance == nearest && best.is_none()) {
            nearest = distance;
            best = Some((t, distance));
        }
    }
    best
}
#[test]
fn seeded_independent_f64_reference() {
    let mut state = 0x12345678u32;
    let mut next = || {
        state ^= state << 13;
        state ^= state >> 17;
        state ^= state << 5;
        (state % 256) as f32
    };
    for _ in 0..12 {
        let mut v = Vec::new();
        let mut i = Vec::new();
        for _ in 0..40 {
            let x = next();
            let y = next();
            let z = next();
            let n = v.len() as u32;
            v.extend([[x, y, z], [x + 8., y, z], [x, y + 8., z]]);
            i.extend([n, n + 1, n + 2]);
        }
        let mut m = Raycaster::build(v.clone(), i.clone()).unwrap();
        for _ in 0..200 {
            let a = [next() + 0.25, next() + 0.25, 300.];
            let b = [a[0], a[1], -10.];
            let expected = oracle(a, b, &v, &i);
            let got = m.segment(a, b).unwrap();
            assert_eq!(got.is_some(), expected.is_some());
            if let (Some(g), Some((id, t))) = (got, expected) {
                assert_eq!(g.triangle, id);
                assert!((f64::from(g.distance) - t * 310.).abs() < 0.0001);
            }
        }
    }
}
#[test]
fn primitive_boundary_and_precision() {
    let tri = [[0., 0., 0.], [4., 0., 0.], [0., 4., 0.]];
    for p in [[0., 0., 2.], [2., 2., 2.], [1., 1., 2.]] {
        assert_eq!(triangle(p, [0., 0., -1.], tri), Some(2.));
        assert_eq!(
            oracle(p, [p[0], p[1], -2.], &tri, &[0, 1, 2]),
            Some((0, 0.5))
        );
    }
    assert!(triangle([1., 1., 2.], [1., 0., -1e-8], tri).is_none());
    assert!(triangle([1., 1., 0.], [0., 0., -1.], tri).is_none());
    // A box containing the origin admits an exact endpoint tie, a planar box at the endpoint rejects it.
    let mut m =
        Raycaster::build(vec![tri[0], tri[1], tri[2], [0., 0., 3.]], vec![0, 1, 2]).unwrap();
    assert!(m.segment([1., 1., 2.], [1., 1., 0.]).unwrap().is_some());
    let mut moved = Raycaster::build(
        tri.map(|mut p| {
            p[2] = 10.;
            p
        })
        .to_vec(),
        vec![0, 1, 2],
    )
    .unwrap();
    assert!(
        moved
            .segment([1., 1., 2.], [1., 1., -2.])
            .unwrap()
            .is_none()
    );
}
#[test]
fn oblique_seeded_f64() {
    let mut seed = 0x854f2026u32;
    let mut next = || {
        seed ^= seed << 13;
        seed ^= seed >> 17;
        seed ^= seed << 5;
        (seed % 10000) as f32 / 100. - 50.
    };
    for _ in 0..8 {
        let mut v = Vec::new();
        let mut i = Vec::new();
        for _ in 0..30 {
            let n = v.len() as u32;
            v.extend([
                [next(), next(), next()],
                [next(), next(), next()],
                [next(), next(), next()],
            ]);
            i.extend([n, n + 1, n + 2]);
        }
        let mut m = Raycaster::build(v.clone(), i.clone()).unwrap();
        for _ in 0..300 {
            let a = [next() * 2., next() * 2., next() * 2.];
            let b = [next() * 2., next() * 2., next() * 2.];
            let h = m.segment(a, b).unwrap();
            let expected = oracle(a, b, &v, &i);
            assert_eq!(h.is_some(), expected.is_some());
            if let (Some(h), Some((id, t))) = (h, expected) {
                assert_eq!(h.triangle, id);
                let length = dot(sub(b, a), sub(b, a)).sqrt() as f64;
                assert!((h.distance as f64 - t * length).abs() < 0.002);
            }
        }
    }
}
#[test]
fn thresholds_and_box_bits() {
    let tri = [[0., 0., 0.], [1., 0., 0.], [0., 1., 0.]];
    let cutoff = 1e-5f32;
    for sign in [-1., 1.] {
        for (bits, hit) in [
            (cutoff.to_bits() - 1, false),
            (cutoff.to_bits(), false),
            (cutoff.to_bits() + 1, true),
        ] {
            let z = f32::from_bits(bits);
            assert_eq!(
                triangle([0.25, 0.25, sign * z], [0., 0., -sign * z], tri).is_some(),
                hit
            );
        }
    }
    let b = Bounds {
        min: [0.; 3],
        max: [1.; 3],
    };
    assert!(!box_hit(b, [0.5; 3], [1., -0., 0.], cutoff));
    assert!(box_hit(
        b,
        [0.5; 3],
        [1., -0., 0.],
        f32::from_bits(cutoff.to_bits() + 1)
    ));
    assert!(box_hit(b, [-1., 1. + cutoff, 0.5], [1., 0., -0.], 3.));
    assert!(!box_hit(
        b,
        [-1., f32::from_bits((1. + cutoff).to_bits() + 1), 0.5],
        [1., 0., 0.],
        3.
    ));
    for y in [
        f32::from_bits(1e-6f32.to_bits() - 1),
        1e-6,
        f32::from_bits(1e-6f32.to_bits() + 1),
    ] {
        let n = normal([[0., 0., 0.], [1., 0., 0.], [1., y, 0.]]);
        if y < 1e-6 {
            assert_eq!(n[2], 0.);
        } else {
            assert!((n[2] - 1.).abs() < 1e-7);
        }
    }
    let mut x: Vec<u8> = Vec::new();
    assert_eq!(reserve(&mut x, usize::MAX), Err(Error("allocation limit")));
}
#[test]
fn split_duplication_and_axis_ties() {
    let mut v = vec![[-2., -2., 0.], [2., -2., 0.], [-2., 2., 0.]];
    v.push([2., 2., 0.]);
    let mut m = Raycaster::build(v, vec![0, 1, 2, 0, 2, 1, 1, 3, 2, 1, 2, 3]).unwrap();
    assert!(m.nodes.len() > 1);
    assert!(m.leaves.len() > 4);
    assert!(m.nodes[0].children[0].is_some());
    let left = m.nodes[0].children[0].unwrap();
    assert_eq!(m.nodes[left].bounds.max[0], 0.);
    for x in [-1., 0., 1.] {
        let h = m.segment([x, 0., 2.], [x, 0., -2.]).unwrap().unwrap();
        assert_eq!(h.triangle, if x <= 0. { 0 } else { 2 });
    }
}

#[test]
fn brute_endpoint_and_tie() {
    let mut m = Raycaster::build(
        vec![[0., 0., 0.], [4., 0., 0.], [0., 4., 0.], [0., 0., 3.]],
        vec![0, 2, 1, 0, 1, 2],
    )
    .unwrap();
    assert!(m.segment([1., 1., 2.], [1., 1., 0.]).unwrap().is_some());
    assert!(
        m.brute_segment([1., 1., 2.], [1., 1., 0.])
            .unwrap()
            .is_none()
    );
    assert_eq!(
        m.brute_segment([1., 1., 2.], [1., 1., -2.])
            .unwrap()
            .unwrap()
            .normal,
        [0., 0., -1.]
    );
}
