const std = @import("std");
const zcc = @import("compile_commands");

// Although this function looks imperative, note that its job is to
// declaratively construct a build graph that will be executed by an external
// runner.
pub fn build(b: *std.Build) void {
    var targets = std.ArrayList(*std.Build.Step.Compile).init(b.allocator);
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

    const cflags = [_][]const u8{ "-std=c++17", "-fno-sanitize=undefined" };

    const lib = b.addSharedLibrary(.{
        .name = name,
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    lib.addIncludePath(b.path("src/"));
    lib.installHeader(b.path("src/os/apple/DankView.h"), "DankView.h");
    lib.linkLibCpp();
    lib.linkFramework("Foundation");
    lib.linkFramework("Metal");

    lib.addCSourceFiles(.{
        .root = b.path("src"),
        .files = &.{
            "modules/engine/Console.cpp",
            "modules/engine/Engine.cpp",
            "modules/scene/Scene.cpp",
            "modules/scene/Camera.cpp",
            "modules/os/Thread.cpp",
            "modules/input/Input.cpp",
            "os/apple/AppleOS.cpp",
            "os/apple/renderer/AppleRenderer.cpp",
        },
        .flags = &cflags,
    });

    const objCFlags = [_][]const u8{ "-std=c++17", "-fno-sanitize=undefined", "-fobjc-arc" };

    const libApple = b.addStaticLibrary(.{
        .name = "DankApple",
        .target = target,
        .optimize = optimize,
        .link_libc = true,
    });
    libApple.addIncludePath(b.path("src/"));
    libApple.addCSourceFiles(.{
        .root = b.path("src"),
        .files = &.{
            "os/apple/support/CaptureStreamOutput.mm",
            "os/apple/support/CaptureEngine.mm",
            "os/apple/DankView.mm",
        },
        .flags = &objCFlags,
    });
    libApple.linkLibCpp();
    libApple.linkFramework("Foundation");
    libApple.linkFramework("CoreFoundation");
    libApple.linkFramework("Metal");
    libApple.linkFramework("MetalKit");
    libApple.linkFramework("ScreenCaptureKit");

    const HelperFunctions = struct {
        fn clearLibDir(_: *std.Build.Step, _: std.Progress.Node) anyerror!void {
            const cwd = std.fs.cwd();
            try cwd.deleteTree("zig-out/lib/");
            try cwd.makeDir("zig-out/lib/");
        }
    };

    var clean_lib_step = std.Build.Step.init(.{
        .id = .custom,
        .name = "clean-lib-directory",
        .owner = b,
        .makeFn = HelperFunctions.clearLibDir,
    });

    const tmp = b.makeTempPath();

    //xcrun -sdk macosx metal -o Shadow.ir  -c Shadow.metal
    const compiled_shader = b.fmt("{s}/{s}", .{ tmp, "StaticMesh.ir" });
    const compile_shader = b.addSystemCommand(&.{"xcrun"});
    compile_shader.addArgs(&.{
        "-sdk",
        "macosx",
        "metal",
        "-frecord-sources",
        "-gline-tables-only",
        "-o",
        compiled_shader,
        "-c",
    });
    compile_shader.addFileArg(b.path("src/os/apple/renderer/shaders/StaticMesh.metal"));
    compile_shader.expectExitCode(0);

    const pack_name = std.fmt.bufPrint(&buf, "{s}_{}.{s}", .{ "Dank", time_in_sec, "metallib" }) catch "metal";

    const pack_shaders = b.addSystemCommand(&.{"xcrun"});
    pack_shaders.addArgs(&.{
        "-sdk",
        "macosx",
        "metallib",
        "-o",
    });
    const p = b.pathJoin(&.{
        ".",
        "zig-out",
        "lib",
        pack_name,
    });
    // const p = b.pathJoin(&.{
    //     "..",
    //     "DankApple",
    //     "DankApple Shared",
    //     "Resources",
    //     pack_name,
    // });
    pack_shaders.addArg(p);
    pack_shaders.addArg(b.pathFromRoot(compiled_shader));
    pack_shaders.expectExitCode(0);

    compile_shader.step.dependOn(&clean_lib_step);
    pack_shaders.step.dependOn(&compile_shader.step);

    targets.append(lib) catch |err| {
        std.debug.print("Failed {s}", .{@errorName(err)});
    };

    b.getInstallStep().dependOn(&pack_shaders.step);

    // This declares intent for the library to be installed into the standard
    // location when the user invokes the "install" step (the default step when
    // running `zig build`).
    b.installArtifact(lib);
    b.installArtifact(libApple);

    zcc.createStep(b, "cdb", targets.toOwnedSlice() catch @panic("OOM"));
}
