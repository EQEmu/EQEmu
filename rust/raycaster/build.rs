fn main() {
    let mut build = cxx_build::bridge("src/lib.rs");
    build.std("c++20");
    // Generated CXX is native code. Rust itself is not sanitizer-instrumented.
    if std::env::var_os("EQEMU_MAP_SANITIZERS").is_some() {
        build
            .flag("-fsanitize=address,undefined")
            .flag("-fno-sanitize-recover=all");
    }
    build.compile("map-raycaster");
    println!("cargo:rerun-if-changed=src/lib.rs");
    println!("cargo:rerun-if-env-changed=EQEMU_MAP_SANITIZERS");
}
