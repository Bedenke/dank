const std = @import("std");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    // Standard target options allows the person running `zig build` to choose
    // what target to build for. Here we do not override the defaults, which
    // means any target is allowed, and the default is native. Other options
    // for restricting supported target set are available.
    const target = b.standardTargetOptions(.{});

    // Standard optimization options allow the person running `zig build` to select
    // between Debug, ReleaseSafe, ReleaseFast, and ReleaseSmall. Here we do not
    // set a preferred release mode, allowing the user to decide how to optimize.
    const optimize = b.standardOptimizeOption(.{});

    const time_in_sec = std.time.timestamp();
    const fmt = "{s}_{}";
    var buf: [100]u8 = undefined;
    const name = std.fmt.bufPrint(&buf, fmt, .{ "dank", time_in_sec }) catch "engine";

    const lib = b.addSharedLibrary(.{
        .name = name,
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    lib.addIncludePath(b.path("src/"));
    lib.installHeader(b.path("src/os/apple/AppleOS.hpp"), "AppleOS.hpp");
    lib.installHeader(b.path("src/os/apple/Metal.hpp"), "Metal.hpp");
    lib.linkLibCpp();
    lib.linkFramework("Foundation");
    lib.linkFramework("Metal");
    lib.addCSourceFiles(.{
        .root = b.path("src"),
        .files = &.{
            "modules/engine/Console.cpp",
            "modules/engine/Engine.cpp",
            "modules/scene/Scene.cpp",
            "os/apple/AppleOS.cpp",
            "os/apple/renderer/AppleRenderer.cpp",
        },
    });

    // This declares intent for the library to be installed into the standard
    // location when the user invokes the "install" step (the default step when
    // running `zig build`).
    b.installArtifact(lib);
}
